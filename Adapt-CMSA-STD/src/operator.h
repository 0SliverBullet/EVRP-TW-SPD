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

void ProbabilisticClarkWrightSavings(Solution &s, std::vector<std::vector<int>>& adjMatrix,Data &data);
void ProbabilisticInsertion(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data);