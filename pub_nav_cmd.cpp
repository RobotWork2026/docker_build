// pub_nav_cmd — 用 ROS 2 发布 /NAV_CMD 控制狗的速度
//
// 用法：
//   ./pub_nav_cmd <x_vel> <y_vel> <yaw_vel> [duration_sec]
//     x_vel       前进/后退速度 (m/s，>0 前进)
//     y_vel       侧向速度    (m/s，>0 左)
//     yaw_vel     转向角速度  (rad/s，>0 左转)
//     duration_sec 持续发送时长 (秒，默认 1.0)
//
// 行为：
//   以 50Hz 持续发送速度命令，duration 到时间发一帧零速让狗停下再退出。
//   Ctrl+C 也会先发一帧零速度再退出，避免狗失控。
//
// 示例：
//   ./pub_nav_cmd 0.3 0 0          前进 0.3 m/s 持续 1 秒后停
//   ./pub_nav_cmd 0   0 0.5 2.0    原地左转 0.5 rad/s 持续 2 秒
//   ./pub_nav_cmd -0.2 0 0 0.5     后退 0.2 m/s 持续 0.5 秒

#include "rclcpp/rclcpp.hpp"
#include "drdds/msg/nav_cmd.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {
std::atomic<bool> g_stop{false};
void HandleSigint(int) { g_stop.store(true); }
}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cout << "Usage: " << argv[0]
              << " <x_vel m/s> <y_vel m/s> <yaw_vel rad/s> [duration_sec=1.0]\n"
              << "Examples:\n"
              << "  " << argv[0] << " 0.3 0 0\n"
              << "  " << argv[0] << " 0 0 0.5 2.0\n";
    return 1;
  }

  const float x_vel   = std::strtof(argv[1], nullptr);
  const float y_vel   = std::strtof(argv[2], nullptr);
  const float yaw_vel = std::strtof(argv[3], nullptr);
  const float duration_sec = (argc >= 5) ? std::strtof(argv[4], nullptr) : 1.0f;

  if (duration_sec <= 0.f) {
    std::cerr << "duration_sec must be > 0\n";
    return 1;
  }

  std::signal(SIGINT, HandleSigint);

  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("pub_nav_cmd");

  auto nav_pub = node->create_publisher<drdds::msg::NavCmd>("/NAV_CMD", 10);

  // 等 discovery 匹配
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::cout << "[pub] /NAV_CMD x_vel=" << x_vel
            << " y_vel=" << y_vel
            << " yaw_vel=" << yaw_vel
            << " duration=" << duration_sec << "s @50Hz, matched_subs="
            << nav_pub->get_subscription_count() << "\n";

  constexpr int kRateHz = 50;
  const auto period = std::chrono::milliseconds(1000 / kRateHz);
  const auto t_end = std::chrono::steady_clock::now()
                   + std::chrono::milliseconds(static_cast<int>(duration_sec * 1000.f));

  uint64_t frame_id = 0;
  auto fill_and_write = [&](float vx, float vy, float vyaw) {
    drdds::msg::NavCmd msg;
    msg.data.x_vel = vx;
    msg.data.y_vel = vy;
    msg.data.yaw_vel = vyaw;
    msg.header.frame_id = frame_id++;
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto sec  = std::chrono::duration_cast<std::chrono::seconds>(now);
    const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);
    msg.header.stamp.sec  = static_cast<int32_t>(sec.count());
    msg.header.stamp.nanosec = static_cast<uint32_t>(nsec.count());
    nav_pub->publish(msg);
  };

  while (!g_stop.load() && rclcpp::ok() && std::chrono::steady_clock::now() < t_end) {
    fill_and_write(x_vel, y_vel, yaw_vel);
    std::this_thread::sleep_for(period);
  }

  // 收尾：连续发几帧零速度，确保看门狗那边稳定接到 stop
  std::cout << "[pub] sending zero velocity to stop...\n";
  for (int i = 0; i < 5; ++i) {
    fill_and_write(0.f, 0.f, 0.f);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  // 留时间把包送出去再退出
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  rclcpp::shutdown();
  return 0;
}
