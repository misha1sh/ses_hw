#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <thread>

std::optional<std::filesystem::path> ReadArguments(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Expected one argument <filePath>. Got " << argc << " arguments" << std::endl;
        return std::nullopt;
    }

    return argv[1];
}

struct Task {
    std::chrono::duration<double> time;
    std::string program_name;
};

std::optional<std::vector<Task>> ReadFile(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << file_path << std::endl;
        return std::nullopt;
    }

    std::vector<Task> tasks;
    while (file) {
        double time;
        file >> time;
        std::string program_name;
        std::getline(file, program_name);
        std::cerr << "`" << program_name << "`" << std::endl;
        tasks.push_back(Task{std::chrono::duration<double>(time), program_name});
    }
    return tasks;
}

void RunTasks(const std::chrono::high_resolution_clock::time_point start_time,
              std::vector<Task> tasks) {
    using namespace std::chrono_literals;

    std::sort(tasks.begin(), tasks.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.time < rhs.time;
    });

    std::vector<std::thread> threads;
    for (const auto& task : tasks) {
        const auto current_time = std::chrono::high_resolution_clock::now();
        const auto time_passed = current_time - start_time;
        const auto time_to_wait = task.time - time_passed;

        if (time_to_wait > 0s) {
            std::this_thread::sleep_for(time_to_wait);
        }

        threads.emplace_back([&](){
            std::system(task.program_name.c_str());
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}


int main(int argc, char** argv) {
    const auto start_time = std::chrono::high_resolution_clock::now();

    const auto file_path_opt = ReadArguments(argc, argv);
    if (!file_path_opt) {
        return -1;
    }

    auto tasks_opt = ReadFile(*file_path_opt);
    if (!tasks_opt) {
        return -1;
    }

    RunTasks(start_time, std::move(*tasks_opt));

    return 0;
}
