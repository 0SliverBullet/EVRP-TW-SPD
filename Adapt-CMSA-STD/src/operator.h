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
#include <unordered_set>
#include <iomanip>
#include <ctime>
#include <unordered_map>
// #include <windows.h>
// #include <psapi.h>
#include "solution.h"
#include "data.h"
#include "config.h"

std::vector<std::vector<int>> ProbabilisticClarkWrightSavings(const std::vector<std::vector<int>>& s_bsf, bool& isInfeasibie);
// void ProbabilisticClarkWrightSavings(Solution &s, Solution &s_bsf, double alpha_bsf, Data &data, bool &isInfeasible);
void ProbabilisticInsertion(std::vector<std::vector<int>>& s, const std::vector<std::vector<int>>& s_bsf, int& l_size, double& d_rate, bool& isInfeasible);