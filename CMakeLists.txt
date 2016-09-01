# %Tag(FULLTEXT)%
cmake_minimum_required(VERSION 2.8.3)
project(ir_server)

## Find catkin and any catkin packages
find_package(catkin REQUIRED COMPONENTS roscpp rospy std_msgs message_generation) #genmsg)

#add_message_files(FILES)

## Generate added messages and services
generate_messages(DEPENDENCIES std_msgs)

## Declare a catkin package
catkin_package( CATKIN_DEPENDS message_runtime)


## Build talker and listener
include_directories(include ${catkin_INCLUDE_DIRS})

add_subdirectory(src)

# %EndTag(FULLTEXT)%

