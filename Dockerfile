# ARM64 ROS 2 编译基础镜像
# 用法：
#   docker build --platform linux/arm64 --build-arg ROS_DISTRO=foxy -t ros:foxy-dev .
#   docker build --platform linux/arm64 --build-arg ROS_DISTRO=humble -t ros:humble-dev .
#
# 由 build_in_docker.sh 自动调用

ARG ROS_DISTRO=foxy
FROM ros:${ROS_DISTRO}-ros-base

ENV DEBIAN_FRONTEND=noninteractive TZ=Asia/Shanghai

WORKDIR /work
