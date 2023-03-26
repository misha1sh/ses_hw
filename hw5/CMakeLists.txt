set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)



add_executable(hw5_threads main_threads.cpp)
target_include_directories(hw5_threads PUBLIC "../libs/big-number/src/big_number")
target_link_libraries(hw5_threads big_number_lib)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(hw5_threads PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(hw5_threads "${CMAKE_THREAD_LIBS_INIT}")
endif()



add_executable(hw5_processes main_processes.cpp)
target_include_directories(hw5_processes PUBLIC "../libs/big-number/src/big_number")
target_link_libraries(hw5_processes big_number_lib)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(hw5_processes PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(hw5_processes "${CMAKE_THREAD_LIBS_INIT}")
endif()

