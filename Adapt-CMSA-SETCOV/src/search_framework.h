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
#include <cstdlib>
#include "stdio.h"
#include "data.h"
#include "solution.h"
#include "time.h"
#include "operator.h"
#include "config.h"
#include "cplex_solver.h"

void Adapt_CMSA_SETCOV(Data &data, Solution &s);
void GenerateGreedySolution(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data);
void ProbabilisticSolutionConstruction(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data);
void Merge(std::vector<std::vector<int>>& adjMatrix1, std::vector<std::vector<int>>& adjMatrix2, int numNodes);
void Merge(Solution &tour_set, Solution &s_bsf, Data& data);
void SolveSubinstance(Solution &s_cplex, double &t_solve, Solution& tour_set, double t_ILP, Data &data);
void Initialize(Data& data);
void Increment(Data& data);
void LocalSearch(Solution &s, Data& data, clock_t stime, int id);
void InitializeTour(Data& data);
void write_solution_to_file(Data &data, Solution &s_bsf, int run, const std::vector<double> &solutions, const std::vector<double> &times, double cost_all_run, double time_all_run);

static inline bool termination(int no_improve, Data &data)
{
    return (no_improve > data.g_1);
}