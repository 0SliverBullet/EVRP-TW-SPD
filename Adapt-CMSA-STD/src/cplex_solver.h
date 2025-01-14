#pragma once
#include <cstdlib>
#include <ilcplex/ilocplex.h>
#include <cmath>
#include <vector>
#include <iostream>
#include "data.h"
ILOSTLBEGIN

void ILPmodel(Data& data, double time_limit, const std::vector<std::vector<int>>& adjMatrix = {});
