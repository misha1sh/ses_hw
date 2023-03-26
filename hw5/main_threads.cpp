#include <big_number.h>

#include <algorithm>
#include <atomic>
#include <optional>
#include <exception>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>



std::optional<size_t> ReadArguments(int argc, char** argv) {
    if (argc != 2) {
        return std::thread::hardware_concurrency() + 1;
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

void RunTasks(size_t max_threads_count) {
    std::atomic<size_t> free_threads_count{0};
    std::vector<std::thread> thread_pool{};

    bool need_stop = false;

    std::mutex queue_mutex;
    std::condition_variable cv;
    std::queue<std::string> queue_tasks{};

    std::string command;
    while (std::getline(std::cin, command)) {
        // Add new thread if all are busy
        if (free_threads_count.load() == 0 && thread_pool.size() < max_threads_count) {
            thread_pool.emplace_back([&]() {
                while (!need_stop) {
                    std::string thread_command;
                    {
                        free_threads_count++;
                        std::unique_lock lk{queue_mutex};
                        cv.wait(lk, [&]{ return queue_tasks.size() > 0 || need_stop; });
                        if (need_stop) {
                            return;
                        }
                        free_threads_count--;


                        thread_command = queue_tasks.front();
                        queue_tasks.pop();
                    }
                    
                    std::cout << "Started task '" << thread_command << "' " << std::endl;
                    try {
                        big_num::BigNumber factorial_bound{thread_command};

                        big_num::BigNumber result{1};
                        for (big_num::BigNumber i = 1; i <= factorial_bound; i = i + big_num::BigNumber{1}) {
                            result = result * i;
                        }
                        std::cout << factorial_bound << "! = " << result << std::endl;
    
                    } catch(std::invalid_argument) {
                        std::cerr << "Number " << thread_command << " is invalid" << std::endl;
                    } catch(std::out_of_range) {
                        std::cerr << "Number " << thread_command << " is out of range" << std::endl;
                    }

                }
            });
        }

        {
            std::unique_lock lk{queue_mutex};
            queue_tasks.emplace(command);
            cv.notify_one();
        }
    }

    need_stop = true;
    cv.notify_all();
    for (auto& thread : thread_pool) {
        thread.join();
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
