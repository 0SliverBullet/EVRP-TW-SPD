#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <numeric>
#include <unordered_set>
#include <cstdio>    // fprintf 
#include <string>    // std::string 
#include <map>
#include "stdio.h"
#include "data.h"
#include "solution.h"
#include "time.h"
#include "operator.h"
#include "config.h"
#include "cplex_solver.h"
using namespace std;

void Adapt_CMSA_STD(Data &data, Solution &s);
void GenerateGreedySolution(Solution &s);
void ProbabilisticSolutionConstruction(Solution &s, Solution &s_bsf, double alpha_bsf, int l_size, Data &data);
void Merge(std::vector<std::vector<int>>& adjMatrix1, std::vector<std::vector<int>>& adjMatrix2, int numNodes);
void SolveSubinstance(Solution &s_cplex, double &t_solve, std::vector<std::vector<int>>&adjMatrix, double t_ILP, Data &data);
void Initialize(int& n_a, int& l_size, Data& data);
void Increment(int& n_a, int& l_size, Data& data);
void LocalSearch(Solution &s, int id);

static inline bool termination(int no_improve, Data &data)
{
    return (no_improve > data.g_1);
}