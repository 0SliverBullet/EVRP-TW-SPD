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
using namespace std;

void Adapt_CMSA_STD(Data &data, Solution &s);
void AdaptCMSA(
    int dimension, 
    double t_prop, 
    double t_ILP, 
    double alpha_LB, 
    double alpha_UB, 
    double alpha_red, 
    double d_rate, 
    double h_rate, 
    int n_a = 1, 
    int l_size = 100, 
    double gamma = 0.1, 
    int delta_n = 1, 
    int delta_l_size = 20, 
    double delta_gamma = 0.1, 
    double infeasible_rate = 0.22, 
    double cpu_time_limit = 150
);
std::vector<std::vector<int>> GenerateGreedySolution();
std::vector<std::vector<int>> ProbabilisticSolutionConstruction(const std::vector<std::vector<int>>& s_bsf, double alpha_bsf, int l_size, double d_rate, double h_rate, double infeasible_rate);
std::vector<std::vector<int>> Merge(const std::vector<std::vector<int>>& C, const std::vector<std::vector<int>>& S);
void AddRandomEdges(std::vector<std::vector<int>>& C, double gamma);
std::pair<std::vector<std::vector<int>>, double> SolveSubinstance(const std::vector<std::vector<int>>& C, double t_ILP);
void Initialize(int& n_a, int& l_size, double& gamma);
void Increment(int& n_a, int& l_size, double& gamma);
void LocalSearch(std::vector<std::vector<int>>& s, int id);