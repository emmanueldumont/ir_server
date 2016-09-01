cmake_minimum_required(VERSION 2.8.3)


add_executable(irServer ir_server.cpp)
target_link_libraries(irServer ${catkin_LIBRARIES})
add_dependencies(irServer irServer__generate_messages_cpp)
