// #include "common.h"

// #include <iostream>
// #include <fstream>
// #include <filesystem>
// #include <vector>
// #include <cstdio>
// #include <chrono>
// #include <thread>

// #include <cerrno>
// #include <stdio.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h> 

// struct FifoHandler {
//     std::string path;

//     void Clear() {
//         unlink(path); 
//     }

//     bool Create() {
//         if((mkfifo("/tmp/fifo0001.1", O_RDWR)) == -1) {
//             std::cerr << "Error during creating fifo\n" 
//                       << errno << " " << std::strerror(errno) << std::endl;
//             return false
//         } 
//         return true;
//     }
// };


// bool DoWork(const std::filesystem::path operations_table_path, const std::filesystem::path operations_queue_path) {
//     const auto operations_file_content = ReadFile(operations_table_path);
//     if (!operations_file_content) {
//         return false;
//     }
//     const auto opertions_table = FileContentToOperationsTable(*operations_file_content);
//     const auto operations_queue_opt = ReadFile(operations_queue_path);

//     for (const auto& [operation, count] : *operations_queue_opt) {
//         const auto time_it = opertions_table.find(operation);
//         if (time_it == opertions_table.end()) {
//             std::cerr << "Can not find operation with name '" << operation << "' " << std::endl;
//             return false;
//         }
//         const size_t time = time_it->second;

//         for (size_t i = 0; i < count; i++) {
//             std::cout << "[Sender] started operation '" << operation << "'. Takes " << time << " seconds." << std::endl;
//             std::this_thread::sleep_for(std::chrono::seconds{time});
//             std::cout << "[Sender] finished operation '" << operation << "'. Sending to receiver" << std::endl;
//         }
//     }   

//     return true;
// }

// int main(int argc, char** argv) {
//     std::filesystem::path executable_path{argv[0]};
//     std::filesystem::path dir_path{executable_path.parent_path()};

//     DoWork(dir_path / "op1.txt", dir_path / "queue.txt");
//     return 0; 
// }

int main() {}