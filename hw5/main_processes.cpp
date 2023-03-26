#include <big_number.h>

#include <algorithm>
#include <atomic>
#include <optional>
#include <exception>
#include <string>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cerrno>
#include <cstring>

#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <sys/wait.h>
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <thread>
#include <unistd.h>

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

void ClearSem(int* p, int shmid, sem_t *sem) {
    shmdt (p);
    shmctl (shmid, IPC_RMID, 0);

    sem_unlink ("pSem");   
    sem_close(sem);  
}

void RunTask(size_t max_proc_count) {
    // https://stackoverflow.com/questions/16400820/how-to-use-posix-semaphores-on-forked-processes-in-c
    key_t shmkey = ftok ("/dev/null", 5);       /* valid directory name and a number */
    int shmid = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
    if (shmid < 0){                           /* shared memory error check */
        std::cerr << "Error during shmget " << shmid << "\n" 
                        << errno << " " << std::strerror(errno) << std::endl;
        return;
    }

    int* p = (int *) shmat (shmid, NULL, 0);   /* attach p to shared memory */
    *p = 0;
    sem_t *sem = sem_open ("pSem", O_CREAT | O_EXCL, 0644, max_proc_count); \
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            sem_unlink ("pSem");  
            sem = sem_open ("pSem", O_CREAT | O_EXCL, 0644, max_proc_count); \
        }
        if (sem == SEM_FAILED) {
            std::cerr << "Error during sem_open n\n"
                            << errno << " " << std::strerror(errno) << std::endl;
            return;
        }
    }

    std::vector<pid_t> children{};

    std::string command;
    while (std::getline(std::cin, command)) {
        sem_wait(sem);
        pid_t fork_id = fork();
        
        if (fork_id < 0 ) {
            ClearSem(p, shmid, sem);
            std::cerr << "Error during creating proccess using fork\n" 
                    << errno << " " << std::strerror(errno) << std::endl;
            return;
        } else if (fork_id == 0) {
            // we are in children.
            std::cout << "Started task '" << command << "' " << std::endl;
            try {
                big_num::BigNumber factorial_bound{command};

                big_num::BigNumber result{1};
                for (big_num::BigNumber i = 1; i <= factorial_bound; i = i + big_num::BigNumber{1}) {
                    result = result * i;
                }
                std::cout << factorial_bound << "! = " << result << std::endl;
                sem_post(sem);
                return;

            } catch(std::invalid_argument) {
                std::cerr << "Number " << command << " is invalid" << std::endl;
            } catch(std::out_of_range) {
                std::cerr << "Number " << command << " is out of range" << std::endl;
            }
        } else {
            // we are in parent thread
            children.push_back(fork_id);
        }
    }

    while (pid_t pid = waitpid (-1, NULL, 0)){
        if (errno == ECHILD) {
            break;
        }
    }

    ClearSem(p, shmid, sem);
}

int main(int argc, char** argv) {
    const auto max_proc_count = ReadArguments(argc, argv);
    if (!max_proc_count) {
        return -1;
    }

    RunTask(*max_proc_count);

    return 0;
} 
