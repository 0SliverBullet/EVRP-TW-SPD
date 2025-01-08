#include "operator.h"

std::vector<std::vector<int>> ProbabilisticClarkWrightSavings(const std::vector<std::vector<int>>& s_bsf, bool& isInfeasible){
    return s_bsf;
    // TODO: use referrence type for the constructed solution
}

void ProbabilisticInsertion(std::vector<std::vector<int>>& s, const std::vector<std::vector<int>>& s_bsf, int& l_size, double& d_rate, bool& isInfeasible) {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    int dim = s.size();
 
    // // Unvisited customers
    // std::vector<int> unvisited_customers;
    // for (int i = 1; i < dim - 1; ++i) {
    //     unvisited_customers.push_back(i);
    // }

    // // Initialize the first route
    // std::vector<int> route;
    // int first_customer = *std::max_element(unvisited_customers.begin(), unvisited_customers.end(), [](int a, int b) {
    //     return /* use distance to depot or deadline comparison */ a < b;
    // });

    // route.push_back(0); // Start at depot
    // route.push_back(first_customer);
    // route.push_back(dim - 1); // End at depot
    // unvisited_customers.erase(std::remove(unvisited_customers.begin(), unvisited_customers.end(), first_customer), unvisited_customers.end());

    // while (!unvisited_customers.empty()) {
    //     // Generate cost list
    //     struct InsertionCost {
    //         int customer;
    //         int position;
    //         double cost;
    //     };

    //     std::vector<InsertionCost> cost_list;

    //     for (int customer : unvisited_customers) {
    //         for (size_t pos = 1; pos < route.size(); ++pos) {
    //             int j = route[pos - 1];
    //             int k = route[pos];
    //             // double cost = /* c(j,i,k) */ + /* dji + dik - djk */;
    //             double cost = 0;
    //             cost_list.push_back({customer, static_cast<int>(pos), cost});
    //         }
    //     }

    //     // Sort by insertion cost
    //     std::sort(cost_list.begin(), cost_list.end(), [](const InsertionCost& a, const InsertionCost& b) {
    //         return a.cost < b.cost;
    //     });

    //     // Choose customer and position
    //     double r = dist(rng);
    //     InsertionCost selected;

    //     if (r <= d_rate) {
    //         // Deterministic: Choose the minimum cost insertion
    //         selected = cost_list.front();
    //     } else {
    //         // Probabilistic: Random selection from the top l_size options
    //         size_t limit = std::min(static_cast<size_t>(l_size), cost_list.size());
    //         std::uniform_int_distribution<size_t> index_dist(0, limit - 1);
    //         selected = cost_list[index_dist(rng)];
    //     }

    //     // Check feasibility
    //     bool feasible = true;

    //     if (!isInfeasible) {
    //         // Check vehicle capacity
    //         // ... (implement vehicle load feasibility checks)

    //         // Check time windows
    //         // ... (implement time window feasibility checks)

    //         // Check battery constraints
    //         // ... (add charging station if needed)
    //     }

    //     if (feasible) {
    //         // Insert into route
    //         route.insert(route.begin() + selected.position, selected.customer);
    //         unvisited_customers.erase(std::remove(unvisited_customers.begin(), unvisited_customers.end(), selected.customer), unvisited_customers.end());
    //     } else {
    //         // Start a new route
    //         route = {0, selected.customer, dim - 1};
    //         unvisited_customers.erase(std::remove(unvisited_customers.begin(), unvisited_customers.end(), selected.customer), unvisited_customers.end());
    //     }
    // }

    // // Transform route into matrix form
    // for (size_t i = 0; i < route.size() - 1; ++i) {
    //     s[route[i]][route[i + 1]] = 1;
    // }
}
