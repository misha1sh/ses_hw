#include <algorithm>
#include <atomic>
#include <optional>
#include <exception>
#include <string>
#include <iostream>
#include <thread>


std::optional<size_t> ReadArguments(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Expected one argument <N>. Got " << argc << " arguments" << std::endl;
        return std::nullopt;
    }

    try {
        size_t res = std::stoi(argv[1]);
        if (res < 0) {
            std::cerr << "N cannot be negative. Got " << res << std::endl;
            return std::nullopt;
        }
        return res;
    } catch (std::invalid_argument) {
        std::cerr << "Invalid N" << std::endl;
    } catch (std::out_of_range) {
        std::cerr << "N is too big" << std::endl;
    }
    return std::nullopt;
}

void RunTasks(size_t max_tasks_count) {
    std::atomic<size_t> current_tasks_count{0};
    std::string command;
    while (std::getline(std::cin, command)) {
        if (current_tasks_count.load() >= max_tasks_count) {
            std::cerr << "Too many threads are running. Please wait" << std::endl;
            continue;
        }
        current_tasks_count++;

        std::thread thread{[&current_tasks_count, command = std::move(command)]() {
            std::system(command.c_str());
            current_tasks_count--;
        }};
        thread.detach();
    }
}

int main(int argc, char** argv) {
    const auto max_tasks_count = ReadArguments(argc, argv);
    if (!max_tasks_count) {
        return -1;
    }
    RunTasks(*max_tasks_count);

    return 0;
}
