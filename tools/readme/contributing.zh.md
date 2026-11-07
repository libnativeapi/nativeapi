## 参与贡献

开发在 [nativeapi](https://github.com/libnativeapi/nativeapi) 中进行，它包含所有绑定和代码生成器，并以 submodule 的形式检出核心库：

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
```

标有 `AUTO-GENERATED. DO NOT EDIT.` 的文件由 [nativeapi](https://github.com/libnativeapi/nativeapi-core) 的 C++ 头文件生成。如需修改 API，请向该仓库提交 PR，绑定由维护者重新生成。

所有问题反馈和功能需求统一提交到 [nativeapi issues](https://github.com/libnativeapi/nativeapi/issues)。请注明涉及的包或绑定、版本、操作系统，并提供最小复现示例。
