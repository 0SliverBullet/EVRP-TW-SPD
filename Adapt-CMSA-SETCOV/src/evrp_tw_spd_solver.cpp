/*
A Reference unofficial C++ reimplementation of multiple variants of the Adapt-CMSA:

- Adapt-CMSA-STD
- Adapt-CMSA-SETCOV

Created on: 2025.01.10
Author: Zubin Zheng

******************** Acknowledgment *********************
Part of codes is from the github repository:
https://github.com/senshineL/VRPenstein
Its MIT License is shown below.
*********************************************************
MIT License

Copyright (c) 2020 Shengcai Liu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*********************************************************

*/
#include "evrp_tw_spd_solver.h"
Solution best_s;
double best_s_cost;
clock_t find_best_time;
int find_best_run;
int find_best_gen;
clock_t find_bks_time;
int find_bks_run;
int find_bks_gen;
bool find_better;
long call_count_move_eval;
double mean_duration_move_eval;
double mean_route_len;


void signalHandler(int signum)
{
    printf("Interrupt signal (%d) received.\n", signum);
    printf("Best cost: %.4f.\n", best_s_cost);
    printf("Time to find this solution: %d.\n", int(find_best_time));
    printf("Time to surpass BKS: %d.\n", int(find_bks_time));
    exit(signum);
}

int main(int argc, char **argv)
{
    best_s.cost = double(INFINITY);
    best_s_cost = -1.0;
    find_best_time = find_bks_time = 0;
    find_best_run = find_bks_run = 0;
    find_best_gen = find_bks_gen = 0;
    find_better = false;
    call_count_move_eval = 0;
    mean_duration_move_eval = 0.0;
    mean_route_len = 0.0;
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    ArgumentParser parser;
    // required arguments
    parser.addArgument("--problem", 1, false);

    // optional
    parser.addArgument("--pruning");
    parser.addArgument("--output", 1);
    parser.addArgument("--time", 1);  // cpu time limit;
    parser.addArgument("--runs", 1);
    parser.addArgument("--g_1", 1);

    parser.addArgument("--t_prop", 1);
    parser.addArgument("--t_ILP", 1);  // cplex time limit
    parser.addArgument("--alpha_LB", 1);
    parser.addArgument("--alpha_UB", 1);
    parser.addArgument("--alpha_red", 1);
    parser.addArgument("--d_rate", 1);
    parser.addArgument("--h_rate", 1);
    parser.addArgument("--init_n_a", 1);
    parser.addArgument("--init_l_size", 1);
    parser.addArgument("--delta_n", 1);
    parser.addArgument("--delta_l_size", 1);
    parser.addArgument("--dummy_stations", 1);

    parser.addArgument("--O_1_eval");

    parser.addArgument("--relocation",1);
    parser.addArgument("--swap");
    parser.addArgument("--two_opt");
    parser.addArgument("--exchange_1_1", 1);
    parser.addArgument("--shift_1_0", 1);

    parser.addArgument("--bks", 1);
    parser.addArgument("--random_seed", 1);

    // parse the command-line arguments - throws if invalid format
    parser.parse(argc, argv);
    // Load data and preprocess
    Data data(parser);
    // Adapt-CMSA-SETCOV 
    Adapt_CMSA_SETCOV(data, best_s); 

    return 0;
}