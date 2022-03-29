## 源码调试

为了更好的分析引擎的执行过程，首先配置如何对引擎进行断点调试

先将源码克隆到本地：

```bash
git clone https://github.com/hsiaosiyuan0/quickjs
```

切换到 learn 分支

```bash
git checkout learn
```

构建包含调试信息的二进制版本：

```bash
make qjs-debug
```

安装 VSCode 插件 [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)

在文件 [qjs.c#L309](../qjs.c#L309) 处增加断点，点击调试按钮，如果在断点处暂停，则说明配置成功：

![](https://p5.music.126.net/obj/wo3DlcOGw6DClTvDisK1/13699899369/8c26/2d99/def4/e5fbaa5cc8471763bdd90673eb5ebe40.gif)

## 预读内容

quickjs 官方的说明 [QuickJS documentation](https://bellard.org/quickjs/quickjs.html) 是重点推荐的预读内容

由于其篇幅很少，建议跳过一些中文翻译版，直接阅读英文原版。对 quickjs 的功能特性有一个概览性的了解即可


