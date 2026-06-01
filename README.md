# Docker 编译

## 使用方法

1. 复制 `Dockerfile` 和 `build_in_docker.sh` 到目标工程目录下
2. 放入对应版本的 deb 文件（`drdds-ros2-msgs_1.0.10_arm64_u20.deb` 或 `_u22.deb`）
3. 编写 `CMakeLists.txt` 定义编译目标
4. 运行编译：

```bash
./build_in_docker.sh        # Ubuntu 20.04 (Foxy)
./build_in_docker.sh u22    # Ubuntu 22.04 (Humble)
```

编译产物在 `build/` 目录下。
