# Bindgen Example

示例目录结构：

```text
tools/bindgen/example/
  bindgen/
    config.yaml
    template/
      example.txt.j2
  README.md
```

运行命令：

```bash
cd tools/bindgen/example
PYTHONPATH=../.. python3 -m bindgen \
  --config bindgen/config.yaml \
  --dump-ir out/ir.json \
  --out out
```

模板会自动从 `bindgen/config.yaml` 同目录下的 `template/` 读取。
