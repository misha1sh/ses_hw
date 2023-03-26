set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)
add_executable(hw4_fifo_sender fifo_sender.cpp)
add_executable(hw4_fifo_receiver fifo_receiver.cpp)

configure_file(op1.txt op1.txt COPYONLY)
configure_file(op2.txt op2.txt COPYONLY)
configure_file(queue.txt queue.txt COPYONLY)
