cmake_minimum_required(VERSION 3.10)

project(test_pub_nav_cmd)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认构建类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# 警告忽略和编译选项
add_definitions(-w)
add_compile_options(-fPIC)

# 查找依赖包
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(drdds REQUIRED)

add_executable(pub_nav_cmd pub_nav_cmd.cpp)

ament_target_dependencies(pub_nav_cmd
  rclcpp
  drdds
)

target_link_libraries(pub_nav_cmd
  -lpthread
  -lm
  -lrt
  -ldl
  -lstdc++
)

install(TARGETS pub_nav_cmd
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()