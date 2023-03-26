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
#include <variant>
#include <memory>
#include <random>

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

template <typename T, typename Comparator>
void Merge(std::vector<T>& input, std::vector<T>& output, Comparator comp, 
               const size_t left, const size_t right, const size_t end) {
    size_t i = left;
    size_t j = right;

    for (size_t k = left; k < end; k++) {
        if (i < right && (j >= end || input[i] <= input[j])) {
            output[k] = input[i];
            i++;
        } else {
            output[k] = input[j];
            j++;
        }
    } 
    for (size_t k = left; k < end; k++) {
        input[k] = output[k];
    }
}

// reference implementation
template <typename T, typename Comparator>
void MergeSort(std::vector<T>& array, Comparator comp, size_t begin, size_t end, std::vector<T>& output) {
    if (end - begin <= 1) {
        return;
    }
    size_t middle = (begin + end) / 2;

    MergeSort(array, comp, begin, middle, output);
    MergeSort(array, comp, middle, end, output);

    Merge(output, array, comp, begin, middle, end);
}

template <typename T>
struct MergeSortDownTask {
    std::vector<T>& array;
    size_t begin;
    size_t end;
    std::vector<T>& output;
};

template <typename T>
struct MergeSortUpTask {
    std::vector<T>& input;
    std::vector<T>& output;
    size_t left;
    size_t right;
    size_t end;
};

template <typename T>
struct MergeTask {
    std::variant<MergeSortDownTask<T>, MergeSortUpTask<T>> task;

    size_t completed_children_tasks{0};
    std::shared_ptr<MergeTask> parent_task{};

    bool queued{false};
};



template <typename T, typename Comparator>
void MergeSortParallel(std::vector<T>& array, Comparator comp, size_t max_threads_count) {
    std::vector<std::thread> thread_pool{};
    std::atomic<size_t> free_threads_count{0};

    std::mutex host_notify_mutex;
    bool host_notify{false};
    std::condition_variable host_notify_cv;

    bool need_stop = false;

    bool done = false;

    std::mutex queue_mutex;
    std::condition_variable cv;
    std::queue<std::shared_ptr<MergeTask<T>>> queue_tasks{};

    const auto task_completed = [&](const auto& task) {
        {
            std::unique_lock lk{queue_mutex};
            if (auto& parent_task = task->parent_task) {
                parent_task->completed_children_tasks++;
                if (parent_task->completed_children_tasks == 2 && !parent_task->queued) {
                    parent_task->queued = true;
                    queue_tasks.emplace(parent_task);
                }
                cv.notify_one();
            } else {
                done = true;
                std::unique_lock{host_notify_mutex};
                host_notify = true;
                host_notify_cv.notify_one();
            }
        }
    };

    std::vector<T> array_copy{array};
    queue_tasks.emplace(std::make_shared<MergeTask<T>>(
            MergeTask<T> {
                MergeSortDownTask<T> { array_copy, 0, array.size(), array}
            }
        ));



    while (!done) {
        if (free_threads_count.load() == 0 && thread_pool.size() < max_threads_count) {
            thread_pool.emplace_back([&]() {
                while (!need_stop) {
                    std::shared_ptr<MergeTask<T>> task{nullptr};
                    {
                        free_threads_count++;
                        std::unique_lock lk{queue_mutex};
                        cv.wait(lk, [&]{ return queue_tasks.size() > 0 || need_stop; });
                        if (need_stop) {
                            return;
                        }
                        free_threads_count--;
                        task = queue_tasks.front();
                        queue_tasks.pop();
                        if (free_threads_count.load() == 0) {
                            std::unique_lock{host_notify_mutex};
                            host_notify = true;
                            host_notify_cv.notify_one();
                        }
                    }
                    
                    if (const auto* task_type = std::get_if<MergeSortDownTask<T>>(&task->task)) {
                        if (task_type->end - task_type->begin <= 1) {
                            task_completed(task);
                            continue;
                        }

                        size_t middle = (task_type->begin + task_type->end) / 2;
                        auto task1 = std::make_shared<MergeTask<T>>(
                            MergeTask<T> {
                                MergeSortDownTask<T> { task_type->array, task_type->begin, middle, task_type->output },
                            }
                        );
                        auto task2 = std::make_shared<MergeTask<T>>(
                            MergeTask<T> {
                                MergeSortDownTask<T> { task_type->array, middle, task_type->end, task_type->output }
                            }
                        );
                        auto task_merge = std::make_shared<MergeTask<T>>(
                            MergeTask<T> {
                                MergeSortUpTask<T> { task_type->output, task_type->array, task_type->begin, middle, task_type->end }
                            }
                        );
                        
                        task1->parent_task = task_merge;
                        task2->parent_task = task_merge;
                        task_merge->parent_task = task->parent_task;
                        {
                            std::unique_lock lk{queue_mutex};
                            queue_tasks.emplace(task1);
                            task1->queued = true;
                        }
                        cv.notify_one();
                        {
                            std::unique_lock lk{queue_mutex};
                            queue_tasks.emplace(task2);
                            task2->queued = true;
                        }
                        cv.notify_one();
                    } else if (const auto* task_type = std::get_if<MergeSortUpTask<T>>(&task->task)) {
                        Merge(task_type->input, task_type->output, comp, 
                              task_type->left, task_type->right, task_type->end);
                        task_completed(task);
                    }
                }
            });
        }

        std::unique_lock lk{host_notify_mutex};
        host_notify_cv.wait(lk, [&]{ return host_notify; });
        host_notify = false;
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

    // std::vector<int> arr{10, 100, 1123, 9, 1234, 8, 7, 6, 141241, 5, -123, 4, 123123, 3, 2, 1};

    std::mt19937 rng(0);
    std::uniform_int_distribution<int> dist(1,1000000);
    std::vector<int> arr;
    for (size_t i = 0; i < 10000000; i++) {
        arr.emplace_back(dist(rng));
    }
    MergeSortParallel(arr, std::less<int>{}, *max_tasks_count);
    for (size_t i = 0; i < arr.size(); i++) {
        std::cout << arr[i] << " ";
    }
    return 0;
}
