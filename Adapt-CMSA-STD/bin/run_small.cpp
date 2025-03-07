  /*
g++ -std=c++17 -pthread -o ../bin/run_commands -O3 run_commands.cpp
*/
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <tuple>

std::mutex mtx;
std::condition_variable cv;
int active_threads = 0;
// std::vector<int> cores = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
//                           11,12,13,14,15,16,17,18,19,20,
//                           21,22,23,24,25,26,27,28,29,30,
//                           31,32,33,34,35,36,37,38,39,40,
//                           41,42,43,44,45,46,47,48,49,50,
//                           51,52,53,54,55};
std::vector<int> cores = {30,
                          31,32,33,34,35,36,37,38,39,40,
                          41,42,43,44,45,46,47,48,49, 50,
                          51, 52, 53};
int num_cores = cores.size(); // Number of specific cores we want to use

void execute_command(const std::string& command, int core_id) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return active_threads < num_cores; });
        ++active_threads;
    }

    // Print the command and the core it's bound to
    std::cout << "Executing on core " << core_id << ": " << command << std::endl;

    // Use taskset to bind the command to a specific core
    std::string taskset_command = "taskset -c " + std::to_string(core_id) + " " + command;
    int result = system(taskset_command.c_str());
    if (result == -1) {
        std::cerr << "Failed to execute command: " << taskset_command << std::endl;
    }

    {
        std::unique_lock<std::mutex> lock(mtx);
        --active_threads;
    }
    cv.notify_one();
}

void add_to_threads(std::vector<std::thread>& threads, const std::string& baseCommand, const std::vector<std::tuple<std::string, std::string>>& options_and_prefixes) {
    std::vector<std::string> small_instances = {
                                                // "c101C5","c103C5","c206C5","c208C5","r104C5","r105C5",\
                                                // "r202C5","r203C5","rc105C5","rc108C5","rc204C5","rc208C5"
                                                // "c101C10","c104C10","c202C10","c205C10","r102C10","r103C10",\
                                                // "r201C10","r203C10","rc102C10","rc108C10","rc201C10","rc205C10"
                                                "c103C15","c106C15","c202C15","c208C15","r102C15","r105C15",\
                                                "r202C15","r209C15","rc103C15","rc108C15","rc202C15","rc204C15"
                                                };

    // std::vector<std::string> medium_instances;

    // for (int i = 101; i <= 109; ++i) {
    //     medium_instances.push_back("c" + std::to_string(i) + "_21");
    // }

    // for (int i = 201; i <= 208; ++i) {
    //     medium_instances.push_back("c" + std::to_string(i) + "_21");
    // }

    // for (int i = 101; i <= 112; ++i) {
    //     medium_instances.push_back("r" + std::to_string(i) + "_21");
    // }

    // for (int i = 201; i <= 211; ++i) {
    //     medium_instances.push_back("r" + std::to_string(i) + "_21");
    // }

    // for (int i = 101; i <= 108; ++i) {
    //     medium_instances.push_back("rc" + std::to_string(i) + "_21");
    // }

    // for (int i = 201; i <= 208; ++i) {
    //     medium_instances.push_back("rc" + std::to_string(i) + "_21");
    // }

    for (const auto& [options, prefix] : options_and_prefixes) {
        for (int i = 0; i < small_instances.size(); ++i){
            std::string problemFile = small_instances[i]+".txt";
            std::string command = baseCommand + problemFile + options;
            int core_id = cores[i % num_cores];
            threads.emplace_back(execute_command, command, core_id);
        }
    }

    // for (const auto& [options, prefix] : options_and_prefixes) {
    //     for (int i = 0; i < medium_instances.size(); ++i){
    //         std::string problemFile = medium_instances[i]+".txt";
    //         std::string command = baseCommand + problemFile + options;
    //         int core_id = cores[i % num_cores];
    //         threads.emplace_back(execute_command, command, core_id);
    //     }
    // }
    
}

int main() {
    std::string baseCommand = "./build/evrp-tw-spd --problem ./data/akb_instances/";

    std::vector<std::tuple<std::string, std::string>> options_and_prefixes = {
        {" --pruning --output ./solution/v0.0.1/akb/ --time 105 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 3", "_"}
    };

    std::vector<std::thread> threads;

    add_to_threads(threads, baseCommand, options_and_prefixes);

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    std::cout << "All commands executed successfully." << std::endl;
    return 0;
}
