#pragma once
#include <cstdlib>
#include <ilcplex/ilocplex.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <vector>
#include <cstdio>
#include "solution.h"
#include "data.h"
ILOSTLBEGIN

void ILPmodel(Solution& s, double& t_solve, Data& data, double time_limit, const std::vector<std::vector<int>>& adjMatrix, bool& optimal);
