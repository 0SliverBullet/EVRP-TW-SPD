#pragma once
#include "solution.h"
#include "data.h"
#include "move.h"
#include <chrono>

void update_route_status(std::vector<int> &nl, std::vector<status> &sl, Data &data, int &flag, double &cost, int &index_negtive_first);
bool cal_score_customer(std::vector<int> &feasible_pos, std::vector<std::tuple<int, int>> &unrouted, std::vector<double> &score, std::vector<int> &unrouted_index, \
                int &score_len, int index, Route &r, double unrouted_d, double unrouted_p, std::vector<std::vector<int>>& adjMatrix, Data &data);
bool cal_score_station(bool type, std::vector<int> &feasible_pos, std::vector<int> &station_pos, std::vector<double> &score, Route &r,Data &data,int index_last_f0, int index_negtive_first);
double cal_additional_savings_value(int pos1, int pos2, double alpha_bsf, std::vector<std::vector<int>>& adjMatrix, Data &data);

double criterion_customer(Route &r, Data &data, int node, int pos, std::vector<std::vector<int>>& adjMatrix);
double criterion_station(Route &r, Data &data, int node, int pos);

bool sequential_station_insertion(int &flag, int &index_negtive_first, Route &r, Data &data, std::vector<std::pair<int,int>> &station_insert_pos, double &heuristic_cost);