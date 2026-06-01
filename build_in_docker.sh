#!/bin/bash
# build_in_docker.sh — 在 ARM64 ROS 2 镜像中编译
# 用法: ./build_in_docker.sh [u22]   (默认 u20)
#
# 要求：
#   - 当前目录下有 CMakeLists.txt
#   - 当前目录下有 drdds-ros2-msgs deb 文件
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DIR_NAME="$(basename "${SCRIPT_DIR}")"

# ---- 版本选择 ----
if [ "$1" = "u22" ]; then
  ROS_DISTRO="humble"
  DEB_SUFFIX="_u22.deb"
  IMAGE_TAG="ros:humble-dev"
else
  ROS_DISTRO="foxy"
  DEB_SUFFIX="_u20.deb"
  IMAGE_TAG="ros:foxy-dev"
fi

# deb 文件名
DEB_NAME="drdds-ros2-msgs_1.0.10_arm64${DEB_SUFFIX}"
DEB_PATH="${SCRIPT_DIR}/${DEB_NAME}"
if [ ! -f "${DEB_PATH}" ]; then
  echo "ERROR: ${DEB_PATH} not found"
  exit 1
fi

# ---- qemu 检查 ----
if ! docker run --rm --platform linux/arm64 hello-world >/dev/null 2>&1; then
  docker run --rm --privileged multiarch/qemu-user-static --reset -p yes >/dev/null 2>&1 || true
fi

# ---- 构建镜像（已存在则跳过） ----
if docker image inspect "${IMAGE_TAG}" >/dev/null 2>&1; then
  echo "[SKIP] Image ${IMAGE_TAG} already exists"
else
  echo "Building ${IMAGE_TAG} ..."
  docker build --platform linux/arm64 \
    --build-arg ROS_DISTRO="${ROS_DISTRO}" \
    -t "${IMAGE_TAG}" \
    -f "${SCRIPT_DIR}/Dockerfile" \
    "${SCRIPT_DIR}"
fi

# ---- 容器内编译 ----
mkdir -p "${SCRIPT_DIR}/build"
sudo rm -rf "${SCRIPT_DIR}/build/"*

docker run --platform linux/arm64 --rm \
  -v "${PROJECT_ROOT}":/work \
  -v "${SCRIPT_DIR}/build":/work/${DIR_NAME}/build \
  -e ROS_DISTRO="${ROS_DISTRO}" \
  "${IMAGE_TAG}" \
  bash -c "
    set -e
    source /opt/ros/\${ROS_DISTRO}/setup.bash
    dpkg -i /work/${DIR_NAME}/${DEB_NAME} 2>/dev/null || apt-get install -y -f
    cd /work/${DIR_NAME}/build
    cmake /work/${DIR_NAME}
    make -j\$(nproc)
  "

echo "Done. Build output in: ${SCRIPT_DIR}/build/"
