[quickjs-emscripten](../../packages.md) • **quickjs-emscripten-core** • [Readme](../README.md) \| [Exports](../exports.md)

***

[quickjs-emscripten](../../packages.md) / [quickjs-emscripten-core](../exports.md) / EmscriptenModule

# Interface: EmscriptenModule

Typings for the features we use to interface with our Emscripten build of
QuickJS.

## Contents

- [Extended By](EmscriptenModule.md#extended-by)
- [Properties](EmscriptenModule.md#properties)
  - [FAST\_MEMORY](EmscriptenModule.md#fast-memory)
  - [HEAP16](EmscriptenModule.md#heap16)
  - [HEAP32](EmscriptenModule.md#heap32)
  - [HEAP8](EmscriptenModule.md#heap8)
  - [HEAPF32](EmscriptenModule.md#heapf32)
  - [HEAPF64](EmscriptenModule.md#heapf64)
  - [HEAPU16](EmscriptenModule.md#heapu16)
  - [HEAPU32](EmscriptenModule.md#heapu32)
  - [HEAPU8](EmscriptenModule.md#heapu8)
  - [TOTAL\_MEMORY](EmscriptenModule.md#total-memory)
  - [TOTAL\_STACK](EmscriptenModule.md#total-stack)
- [Methods](EmscriptenModule.md#methods)
  - [UTF8ToString()](EmscriptenModule.md#utf8tostring)
  - [\_free()](EmscriptenModule.md#free)
  - [\_malloc()](EmscriptenModule.md#malloc)
  - [cwrap()](EmscriptenModule.md#cwrap)
  - [lengthBytesUTF8()](EmscriptenModule.md#lengthbytesutf8)
  - [stringToUTF8()](EmscriptenModule.md#stringtoutf8)

## Extended By

- [`QuickJSAsyncEmscriptenModule`](QuickJSAsyncEmscriptenModule.md)
- [`QuickJSEmscriptenModule`](QuickJSEmscriptenModule.md)

## Properties

### FAST\_MEMORY

> **FAST\_MEMORY**: `number`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:163

***

### HEAP16

> **HEAP16**: `Int16Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:154

***

### HEAP32

> **HEAP32**: `Int32Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:155

***

### HEAP8

> **HEAP8**: `Int8Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:153

***

### HEAPF32

> **HEAPF32**: `Float32Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:159

***

### HEAPF64

> **HEAPF64**: `Float64Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:160

***

### HEAPU16

> **HEAPU16**: `Uint16Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:157

***

### HEAPU32

> **HEAPU32**: `Uint32Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:158

***

### HEAPU8

> **HEAPU8**: `Uint8Array`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:156

***

### TOTAL\_MEMORY

> **TOTAL\_MEMORY**: `number`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:162

***

### TOTAL\_STACK

> **TOTAL\_STACK**: `number`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:161

## Methods

### UTF8ToString()

> **UTF8ToString**(`ptr`, `maxBytesToRead`?): `string`

HeapChar to JS string.
https://emscripten.org/docs/api_reference/preamble.js.html#UTF8ToString

#### Parameters

• **ptr**: [`BorrowedHeapCharPointer`](../exports.md#borrowedheapcharpointer)

• **maxBytesToRead?**: `number`

#### Returns

`string`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:148

***

### \_free()

> **\_free**(`ptr`): `void`

#### Parameters

• **ptr**: `number`

#### Returns

`void`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:151

***

### \_malloc()

> **\_malloc**(`size`): `number`

#### Parameters

• **size**: `number`

#### Returns

`number`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:150

***

### cwrap()

> **cwrap**(`ident`, `returnType`, `argTypes`, `opts`?): (...`args`) => `any`

#### Parameters

• **ident**: `string`

• **returnType**: `null` \| `ValueType`

• **argTypes**: `ValueType`[]

• **opts?**: `CCallOpts`

#### Returns

`Function`

> ##### Parameters
>
> • ...**args**: `any`[]
>
> ##### Returns
>
> `any`
>

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:152

***

### lengthBytesUTF8()

> **lengthBytesUTF8**(`str`): `number`

#### Parameters

• **str**: `string`

#### Returns

`number`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:149

***

### stringToUTF8()

> **stringToUTF8**(`str`, `outPtr`, `maxBytesToRead`?): `void`

Write JS `str` to HeapChar pointer.
https://emscripten.org/docs/api_reference/preamble.js.html#stringToUTF8

#### Parameters

• **str**: `string`

• **outPtr**: [`OwnedHeapCharPointer`](../exports.md#ownedheapcharpointer)

• **maxBytesToRead?**: `number`

#### Returns

`void`

#### Source

packages/quickjs-ffi-types/dist/index.d.ts:143

***

Generated using [typedoc-plugin-markdown](https://www.npmjs.com/package/typedoc-plugin-markdown) and [TypeDoc](https://typedoc.org/)