## 调试方式

qjs 会将 js 源码编译为其内部的 opcode（operation code） 然后解释执行 opcode。因此学习 opcode 有助于了解引擎内部的执行方式

学习 opcode 最直观的着手方式是，将 js 源码和它们对应的字节码放置到一起对比学习理解。为了达到这个目的，需要看下 qjs 如何可以输出源码对应的 opcode

通过查看源码，发现打印 opcode 的方法是 `js_dump_function_bytecode`，进而追踪该方法的调用位置发现，只可以通过设置预处理变量 `DUMP_BYTECODE` 编译出打印 opcode 的 qjs 可执行文件，换句话 qjs 当前的实现中是没有方法可以通过命令来让其打印 opcode 的

通过将 `DUMP_BYTECODE` 设置为以下内容后重新编译，可以获得打印 opcode 的 qjs 可执行文件：

```c
/* dump the bytecode of the compiled functions: combination of bits
   1: dump pass 3 final byte code
   2: dump pass 2 code
   4: dump pass 1 code
   8: dump stdlib functions
  16: dump bytecode in hex
  32: dump line number table
 */
#define DUMP_BYTECODE  (1 | 16)
```

`DUMP_BYTECODE` 上方的注释也说明了数值 `1` 和 `16` 的含义

*我们暂时不去关注 `pass 1`、`pass 2` 这些，目前只需知道 opcode 的生成不是一蹴而就的，中间经过了 3 个阶段，而我们当前只关注最终的结果 opcode 即可*

通过下面的命令重新编译 qjs：

```bash
make qjs-debug
```

编写测试脚本：

```js
// test.js
function calc(a, b) {
  const c = a + b + 1;
  return c;
}

print(calc(1, 2));
```

使用编译后的 qjs 运行测试脚本：

```bash
./qjs-debug test.js > test.bytecode
```

`test.bytecode` 文件中得到的输出内容类似：

```
test.js:1: function: calc
  args: a b
  locals:
    0: const c [level:1 next:-1]
  stack_size: 2
  opcodes:
;; function calc(a, b) {

    0  61 00 00                   set_loc_uninitialized 0: c

;;   const c = a + b + 1;

    3  D1                         get_arg0 0: a
    4  D2                         get_arg1 1: b
    5  9D                         add
    6  B6                         push_1 1
    7  9D                         add
    8  C9                         put_loc0 0: c

;;   return c;

    9  62 00 00                   get_loc_check 0: c
   12  28                         return

;; }
```

## 指令定义

指令统一定义在 [quickjs-opcode.h](../quickjs-opcode.h) 文件中

指令主要分两种：

- 一种是使用 `DEF` 定义的最终指令，最终的含义是这些指令会出现在最终生产的字节码序列中
- 另一种是使用 `def` 定义的临时指令，临时指令会在中间的优化过程中被优化掉，不会出现在最终生成的指令序列中

`DEF` 指令不到 246 个，`def` 指令 16 个

*因为指令数是可能会在未来升级中变更的，所以无需精确统计它们的个数*

## 指令执行

指令的执行是通过 [JS_CallInternal](../quickjs.c#L16198) 完成的，JS_CallInternal 内部使用了手动构造的 Jump Table

先不着急了解 Jump Table 是什么，源码中的 C 实现可以通过下面的伪代码表示：

```ts
const dispatch_table: Array<PointerOfLabel> = [AddressOfOP0 ... AddressOfOP255]

OP0:
  internal operations of OP0
OP1:
  internal operations of OP1

// ...

OP255:
  internal operations of OP255
```

`OP0:` 表示 C 语言中的一条 Labelled Statement

`internal operations of OP0` 表示 OP0 对应的一系列操作，假设 OP0 表示加法指令，那么一系列操作可能就是从操作数栈弹出操作数，执行加法运算，并将执行结果再压入操作数栈

C 语言（或者是编译器、这里就不深究是不是语言标准的一部分了）是支持获取 Labelled Statement 起始指令的内存地址的，通过在标签名上使用 `&&` 操作符

所以伪代码中 `AddressOfOP0` 其实就可以通过 `&& OP0 获得`，加上我们知道 C 语言中有 `goto` 语句，那在知道目标地址的前提下，就能用 `goto` 语句进行跳转了，Jump Table 的名字就是这么来的

在上面伪代码的基础上，继续理解下面的源码实现：

```c
    static const void * const dispatch_table[256] = {
#define DEF(id, size, n_pop, n_push, f) && case_OP_ ## id,
#if SHORT_OPCODES
#define def(id, size, n_pop, n_push, f)
#else
#define def(id, size, n_pop, n_push, f) && case_default,
#endif
#include "quickjs-opcode.h"
        [ OP_COUNT ... 255 ] = &&case_default
    };
#define SWITCH(pc)      goto *dispatch_table[opcode = *pc++];
#define CASE(op)        case_ ## op
#define DEFAULT         case_default
#define BREAK           SWITCH(pc)

SWITCH(pc) {
CASE(OP_push_i32):
    *sp++ = JS_NewInt32(ctx, get_u32(pc));
    pc += 4;
    BREAK;
CASE(OP_push_const):
    *sp++ = JS_DupValue(ctx, b->cpool[get_u32(pc)]);
    pc += 4;
    BREAK;
}
```

> `##` 是预处理指令，用于在预处理阶段进行字符串连接

上面的代码展开后类似：

```c
static const void * const dispatch_table[256] = {
  && case_OP_push_i32,
  && case_OP_push_const,
  // ...
  // ...

  // 上文也提到了，目前指令个数大致在 245 个，dispatch_table 长度是 256
  // 多余的部分统一跳转到 case_default 的位置，查看下方源码也可以看到 case_default
  // 对应的位置是抛出一个「未知指令」的异常
  [ OP_COUNT ... 255 ] = &&case_default
};

#define SWITCH(pc)      goto *dispatch_table[opcode = *pc++];
#define CASE(op)        case_ ## op
#define DEFAULT         case_default
#define BREAK           SWITCH(pc)

goto *dispatch_table[opcode = *pc++];

{

case_OP_push_i32:
    *sp++ = JS_NewInt32(ctx, get_u32(pc));
    pc += 4;
    goto *dispatch_table[opcode = *pc++];

case_OP_push_const:
    *sp++ = JS_DupValue(ctx, b->cpool[get_u32(pc)]);
    pc += 4;
    goto *dispatch_table[opcode = *pc++];

}
```

对于标签语句 `case_OP_push_i32:` 而言，使用 `&& case_OP_push_i32` 获取的就是其第一条语句，比如 `*sp++ = JS_NewInt32(ctx, get_u32(pc));` 编译后的第一条指令的内存地址

展开后的代码我们也可以感受到 `goto` 语句在代码组织和维护时并不方便，所以才通过 `SWITCH`，`CASE` 之类的宏来提供遍历

既然使用了 `SWITCH` 宏，为什么不直接使用 Switch 语句呢?

就目前掌握的信息来看，现代的编译器，会对 `switch` 语句做足够智能的优化 - 编译成 if-else 或者 jmp-table，见 [StackOverflow - c-switch-and-jump-tables](https://stackoverflow.com/questions/17061967/c-switch-and-jump-tables)

这里手动构造 jmp-table 有可能是作者在对比了自动生成的结果和手动构造结果的性能之后有意为之（相当随意的猜测）

## 指令生成方式

在上文为了打印字节码而设置预处理变量 `DUMP_BYTECODE` 时，以及源码中：

```c
def(    enter_scope, 3, 0, 0, u16)  /* emitted in phase 1, removed in phase 2 */
```

都可以看到指令的生成似乎包含几个阶段，因此在这一节中，将主要了解这些阶段的发生位置和区别

## 指令分析
