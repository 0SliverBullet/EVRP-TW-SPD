#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <algorithm>
#include <random>
#include <cmath>
#include <iostream>  
#include <limits>    

std::vector<std::vector<int>> ProbabilisticClarkWrightSavings(const std::vector<std::vector<int>>& s_bsf, bool& isInfeasibie);
void ProbabilisticInsertion(std::vector<std::vector<int>>& s, const std::vector<std::vector<int>>& s_bsf, int& l_size, double& d_rate, bool& isInfeasible);