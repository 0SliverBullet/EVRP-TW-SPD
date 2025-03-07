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
std::vector<int> cores = {0, 
                            1, 2, 3, 4,29,30,
                          31,32,33,34,35,36,37,38,39,40,
                          41,42,43,44,45,46,47};
// std::vector<int> cores = {0, 
//                             1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
//                           11,12,13,14,15,16,17,18,19,20,
//                           21,22,23};
// std::vector<int> cores = {         24,25,26,27,28,29,30,
//                           31,32,33,34,35,36,37,38,39,40,
//                           41,42,43,44,45,46,47          };
// std::vector<int> cores = {30,
//                           31,32,33,34,35,36,37,38,39,40,
//                           41,42,43,44,45,46,47,48,49, 50,
//                           51, 52, 53};
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


void add_to_threads(std::vector<std::thread>& threads, const std::string& baseCommand, const std::vector<std::tuple<std::string, std::string>>& options_and_prefixes, int start, int end) {
    int option_index = 0;
    for (const auto& [options, prefix] : options_and_prefixes) {
        for (int i = start; i <= end; ++i) {
            std::string problemFile = prefix + std::to_string(i) + ".txt";
            std::string command = baseCommand + problemFile + options;
            int core_id = cores[(option_index * (end - start + 1) + (i - start)) % num_cores];
            threads.emplace_back(execute_command, command, core_id);
        }
        option_index++;
    }
    // int option_index = 0;
    // for (const auto& [options, prefix] : options_and_prefixes) {
    //         std::string problemFile = prefix +".txt";
    //         std::string command = baseCommand + problemFile + options;
    //         int core_id = cores[option_index % num_cores];
    //         threads.emplace_back(execute_command, command, core_id);
    //         option_index++;
    // }
}

int main() {
    std::string baseCommand = "./build/evrp-tw-spd --problem ./data/jd_instances/";

    std::vector<std::tuple<std::string, std::string>> options_and_prefixes = {
        // {" --pruning --output ./solution/v0.0.4/jd/small_timelimit/ --time 1800 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd200_"},
        // {" --pruning --output ./solution/v0.0.4/jd/small_timelimit/ --time 5400 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd400_"},
        // {" --pruning --output ./solution/v0.0.4/jd/small_timelimit/ --time 9000 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd600_"},
        // {" --pruning --output ./solution/v0.0.4/jd/small_timelimit/ --time 12600 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd800_"},
        // {" --pruning --output ./solution/v0.0.4/jd/small_timelimit/ --time 16200 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd1000_"},
        // {" --pruning --output ./solution/v0.0.4/jd/large_timelimit/ --time 3600 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd200_"},
        // {" --pruning --output ./solution/v0.0.4/jd/large_timelimit/ --time 10800 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd400_"},
        // {" --pruning --output ./solution/v0.0.4/jd/large_timelimit/ --time 18000 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd600_"},
        {" --pruning --output ./solution/v0.0.4/jd/large_timelimit/ --time 25200 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd800_"},
        {" --pruning --output ./solution/v0.0.4/jd/large_timelimit/ --time 32400 --runs 10 --g_1 20 --O_1_eval --relocation 1 --swap --two_opt --exchange_1_1 1 --shift_1_0 1 --dummy_stations 2", "jd1000_"},
    };

    std::vector<std::thread> threads;

    add_to_threads(threads, baseCommand, options_and_prefixes, 1, 4);

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    std::cout << "All commands executed successfully." << std::endl;
    return 0;
}