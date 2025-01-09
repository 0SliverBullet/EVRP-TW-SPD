#include "operator.h"

std::vector<std::vector<int>> ProbabilisticClarkWrightSavings(const std::vector<std::vector<int>>& s_bsf, bool& isInfeasible){
    return s_bsf;
    // TODO: use referrence type for the constructed solution
}

void ProbabilisticInsertion(std::vector<std::vector<int>>& s, const std::vector<std::vector<int>>& s_bsf, int& l_size, double& d_rate, bool& isInfeasible) {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    int dim = s.size();
}


// void ProbabilisticClarkWrightSavings(Solution &s, Solution &s_bsf, double alpha_bsf, Data &data, bool &isInfeasible) {
//     // Step 1: Initialize the savings list
//     int N = data.node_num;  // Number of customers (excluding depot)
//     std::vector<std::pair<int, int>> savingsList;
//     std::unordered_map<std::pair<int, int>, double, pair_hash> savingsMap;
    
//     // Parameters for savings computation
//     double lambda = data.lambda; // Route shape parameter
//     double mu = data.mu;         // Asymmetry scaling parameter

//     for (int i = 1; i <= N; ++i) {
//         for (int j = i + 1; j <= N; ++j) {
//             double savings = data.dist[0][i] + data.dist[0][j] - lambda * data.dist[i][j] + mu * fabs(data.dist[0][i] - data.dist[0][j]);
//             savingsList.emplace_back(i, j);
//             savingsMap[{i, j}] = savings;
//         }
//     }

//     // Step 2: Sort savings list by adjusted savings (q_ij)
//     for (auto &entry : savingsList) {
//         int i = entry.first;
//         int j = entry.second;
//         double adjustment = (s_bsf.containsArc(i, j)) ? alpha_bsf : (1 - alpha_bsf);
//         savingsMap[entry] = (savingsMap[entry] + 1) * adjustment;
//     }
//     std::sort(savingsList.begin(), savingsList.end(), [&](const std::pair<int, int> &a, const std::pair<int, int> &b) {
//         return savingsMap[a] > savingsMap[b];
//     });

//     // Reduce the list to the first `l_size` entries
//     int l_size = data.l_size;
//     if (savingsList.size() > l_size) {
//         savingsList.resize(l_size);
//     }

//     // Step 3: Construct tours
//     std::unordered_map<int, std::vector<int>> routes; // Node -> Route mapping
//     for (int i = 1; i <= N; ++i) {
//         routes[i] = {0, i, N + 1}; // Direct route for each node
//     }

//     for (auto &entry : savingsList) {
//         int i = entry.first;
//         int j = entry.second;
        
//         if (routes[i][1] == i && routes[j][1] == j) {
//             // Check feasibility of merging
//             std::vector<int> &route_i = routes[i];
//             std::vector<int> &route_j = routes[j];
            
//             bool feasible = true;
//             std::vector<int> mergedRoute;

//             // Select merge strategy based on the connection of i and j
//             if (route_i.back() == i && route_j.front() == j) {
//                 mergedRoute = route_i;
//                 mergedRoute.insert(mergedRoute.end(), route_j.begin() + 1, route_j.end());
//             } else if (route_i.front() == i && route_j.back() == j) {
//                 mergedRoute = route_j;
//                 mergedRoute.insert(mergedRoute.end(), route_i.begin() + 1, route_i.end());
//             } else if (route_i.back() == i && route_j.back() == j) {
//                 mergedRoute = route_i;
//                 std::reverse(route_j.begin(), route_j.end());
//                 mergedRoute.insert(mergedRoute.end(), route_j.begin() + 1, route_j.end());
//             } else if (route_i.front() == i && route_j.front() == j) {
//                 mergedRoute = route_j;
//                 std::reverse(route_i.begin(), route_i.end());
//                 mergedRoute.insert(mergedRoute.end(), route_i.begin() + 1, route_i.end());
//             } else {
//                 feasible = false;
//             }

//             // Check feasibility of merged route
//             if (feasible && isRouteFeasible(mergedRoute, data)) {
//                 routes[i] = mergedRoute;
//                 routes[j] = mergedRoute;
//             } else {
//                 isInfeasible = true;
//             }
//         }
//     }

//     // Step 4: Convert routes into solution components
//     s.clear();
//     for (const auto &route : routes) {
//         s.addRoute(route.second);
//     }
// }

// bool isRouteFeasible(const std::vector<int> &route, const Data &data) {
//     // Check capacity constraints
//     int load = 0;
//     for (int node : route) {
//         load += data.node[node].demand;
//         if (load > data.vehicle_capacity) {
//             return false;
//         }
//     }

//     // Check time window constraints
//     double time = 0.0;
//     for (size_t i = 1; i < route.size(); ++i) {
//         int from = route[i - 1];
//         int to = route[i];
//         time += data.dist[from][to];
//         if (time > data.node[to].latest) {
//             return false;
//         }
//         time = std::max(time, data.node[to].earliest);
//     }

//     // Check battery constraints
//     double battery = data.vehicle_battery;
//     for (size_t i = 1; i < route.size(); ++i) {
//         int from = route[i - 1];
//         int to = route[i];
//         battery -= data.energy[from][to];
//         if (battery < 0) {
//             return false;
//         }
//     }

//     return true;
// }
