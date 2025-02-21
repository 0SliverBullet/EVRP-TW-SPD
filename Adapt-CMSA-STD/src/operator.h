#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <limits>  
#include <functional>
#include <map>
#include <numeric>
#include <utility>
#include <tuple>
#include <unordered_set>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <string>
#include <set>
#include "solution.h"
#include "data.h"
#include "config.h"
#include "solution.h"
#include "eval.h"

bool ProbabilisticClarkWrightSavings(Solution &s, std::vector<std::vector<int>>& adjMatrix,Data &data);
bool ProbabilisticInsertion(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data);
void find_local_optima(Solution &s, Data &data, clock_t stime, double base_cost, int id);

void two_opt(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void two_opt_star(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void or_opt_single(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void or_opt_double(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void two_exchange(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);

void relocation(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void exchange_swap(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
// void two_opt(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void exchange_1_1(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);
void shift_1_0(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost);