#pragma once
#include "solution.h"
#include "data.h"
#include "move.h"
#include <chrono>

void update_route_status(std::vector<int> &nl, std::vector<status> &sl, Data &data, int &flag, double &cost, int &index_negtive_first);
double cal_additional_savings_value(int pos1, int pos2, std::vector<std::vector<int>>& adjMatrix, Data &data);
double criterion_customer(Route &r, Data &data, int node, int pos, std::vector<std::vector<int>>& adjMatrix);
bool sequential_station_insertion(int &flag, int &index_negtive_first, Route &r, Data &data, std::vector<std::pair<int,int>> &station_insert_pos, double &heuristic_cost, Solution &s);