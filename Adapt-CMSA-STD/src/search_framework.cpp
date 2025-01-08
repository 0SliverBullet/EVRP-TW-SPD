#include"search_framework.h"

// grobal varivales
int dim;
int n_init;
int n_inc;          
int l_size_init;
int l_size_inc;
double gamma_init;
double gamma_inc;

void AdaptCMSASTD(Data &data, Solution &s){
    // TODO
}

/*
*************************** Main Adapt-CMSA-STD function ***************************  
    a self-adapted CMSA algorithm for EVRP-TW-SPD (CMSA: construct, merge, solve & adapt)

    construct:
        ProbabilisticSolutionConstruction()
    merge:
        Merge() 
        AddRandomEdges()
    solve:
        SolveSubinstance()
    adapt:
        the dynamic change of parameters alpha_bsf, n_a, l_size, gamma 

    Parameters
        - dimsension:
            solution dimemsion, i.e., s_bsf is a (dim, dim) matrix.
        - t_prop: 
            \in [0.1, 0.8], default: 0.44
            a proportion, \in (0, 1)
        - t_ILP: 
            \in {5, 7, 10, 15, 20, 25, 30, 35, 40}, default: 20
            the maximally possible computation time in CPLEX
            i.e., the CPU time limit for the application of CPLEX to solve the ILP model  
        - alpha_LB: 
            \in [0.6, 0.99], default: 0.62
            a lower bound for alpha_bsf
        - alpha_UB: 
            \in [0.6, 0.99], default: 0.76
            an upper bound for alpha_bsf
        - alpha_red: 
            \in [0.01, 0.1], default: 0.06
            the step size (learning/self-adaptive rate) for the reduction of alpha_bsf
        - [ ] d_rate:
            \in [0, 1], default: 0.17
            the probability of performing a step during solution construction deterministically. 
            i.e., determinism rate for solution construction
        - h_rate: 
            \in [0, 1], default: 0.2
            the probability to choose 
                - the C&W savings heuristic (h_rate)
                - the insertion heuristic (1 - h_rate)
            as solution construction mechanism. 
        - n_a:
            \in {1, 2, 3, 4, 5}, default: 1 
            a number of n_a solutions are probabilistically constructed in ProbabilisticSolutionConstruction()
        - [ ] l_size:
            \in {10, 15, 20, 50, 100, 200}, default: 100
            controls the number of considered options (perturb operators?) at each solution construction step.
            a higher value of l_size results in more diverse solutions which, in turn, leads to a larger sub-instance
            i.e., initial list size value
        - gamma:
            \in (0, 1), default: 0.1
            randomly add gamma·|V'_{0,N+1}| additional edges/options to the current sub-instance in AddRandomEdges(C, gamma)
        - delta_n:
            default: 1
        - delta_l_size:
            \in {10, 15, 20, 50, 100, 200}, default: 20
            list size increment
        - delta_gamma:
            default: 0.1
        - infeasible_rate:
            \in [0, 1], default: 0.22
            probability to choose infeasible construction
        - cpu_time_limit:
            \in {150, 900}, default: 150
            CPU time limit for Adapt-CMSA-STD to solve a certain instance 
                - small-scale instance: 150 seconds
                - medium-scale instance: 900 seconds 
                - large-scale instance:
                    - small time limit: 1800 etc.
                    - large time limit: 3600 etc.
            (**Note that in HMA, cpu_time_limit for small- and medium-scale  \in {105, 630}**)
*/
void AdaptCMSA(
    int dimension, 
    double t_prop, 
    double t_ILP, 
    double alpha_LB, 
    double alpha_UB, 
    double alpha_red, 
    double d_rate, 
    double h_rate, 
    int n_a, 
    int l_size, 
    double gamma, 
    int delta_n, 
    int delta_l_size, 
    double delta_gamma, 
    double infeasible_rate, 
    double cpu_time_limit
) {
    
    dim = dimension;
    n_init = n_a;
    l_size_init = l_size;
    gamma = gamma_init;
    n_inc = delta_n;
    l_size_inc = delta_l_size;
    gamma_inc = delta_gamma;

    auto s_bsf = GenerateGreedySolution();
    double alpha_bsf = alpha_UB; // solutions constructed with a high value of alpha_bsf will be rather similar to s_bsf
    Initialize(n_a, l_size, gamma);

    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(current_time - start_time).count();
        if (elapsed_time >= cpu_time_limit) break;

        auto C = s_bsf;

        for (int i = 0; i < n_a; ++i) {
            auto S = ProbabilisticSolutionConstruction(s_bsf, alpha_bsf, l_size, d_rate, h_rate, infeasible_rate);
            LocalSearch(S, 1);
            C = Merge(C, S);
        }

        AddRandomEdges(C, gamma);
        // it seems that this function should be removed

        auto [s_cplex, t_solve] = SolveSubinstance(C, t_ILP);

        /*
        t_solve: the required computation time in CPLEX
        t_prop: a proportion, \in (0, 1)
        */
        LocalSearch(s_cplex, 2);

        if (t_solve < t_prop * t_ILP && alpha_bsf > alpha_LB) {
            alpha_bsf -= alpha_red;
        }
        /*
        If sub-instance can be solved in a computation time 
            - t_solve < t_prop \times t_ILP, 
            - alpha_bsf > alpha_LB
        alphe_bsf is reduced by alpha_red 
            - solutions constructed with a lower value of alpha_bsf will be rather different to s_bsf
        */

        if ( true /* fitness of s_cplex < fitness of s_bsf */) {      // solution in CPLEX is strictly better 
            s_bsf = s_cplex;
            Initialize(n_a, l_size, gamma);          // reset
        } else {
            if ( true /* fitness of s_cplex >  fitness of s_bsf */) {  // solution in CPLEX is strictly worse
                if (n_a == n_init) {
                    alpha_bsf = std::min(alpha_bsf + alpha_red / 10.0, alpha_UB);  // enlarge alpha_bsf
                } 
                else {
                    Initialize(n_a, l_size, gamma);  // reset
                }
            } 
            else {                                                   // solution in CPLEX is exactly the same
                Increment(n_a, l_size, gamma);       // increment
            }
        }
    }

    // return s_bsf;
    // TODO: printf()
}

/*
    initializes the best-so-far solution S_bsf to a feasible solution obtained utilizing an insertion heuristic
*/
std::vector<std::vector<int>> GenerateGreedySolution() {
    // TODO: Generate an initial greedy solution (to be implemented based on specific problem requirements)
    return {};
}

std::vector<std::vector<int>> ProbabilisticSolutionConstruction(const std::vector<std::vector<int>>& s_bsf, double alpha_bsf, int l_size, double d_rate, double h_rate, double infeasible_rate) {
    // Initialize random number generator
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    /* 
    First decision: choose construction mechanism
        - isCWsavings == true: 
        the C&W savings heuristic
            Clarke, G., & Wright, J. W. (1964). Scheduling of vehicles from a central depot to a number of delivery points. 
            Operations research, 12(4), 568-581. https://doi.org/10.1287/opre.12.4.568
    */
    
    double r1 = dist(rng);
    bool isCWsavings = (r1 <= h_rate);

    /*
    Second decision: choose algorithm variant
        - isInfeasible == true: 
        allows for 
            - battery, 
            - time window
        infeasibilities by simply accepting infeasible solution construction steps
    */ 
    double r2 = dist(rng);
    bool isInfeasibie = (r2 <= infeasible_rate);
    
    // all elements in s are initialized to 0
    std::vector<std::vector<int>> s(dim, std::vector<int>(dim, 0)); 

    if (isCWsavings) {
        s = ProbabilisticClarkWrightSavings(s_bsf, isInfeasibie);
    } 
    else {
        ProbabilisticInsertion(s, s_bsf, l_size, d_rate, isInfeasibie);
    }

    return s;
}



std::vector<std::vector<int>> Merge(const std::vector<std::vector<int>>& C, const std::vector<std::vector<int>>& S) {
    // Merge two solutions C and S (to be implemented based on specific problem requirements)
    return {};
}

void AddRandomEdges(std::vector<std::vector<int>>& C, double gamma) {
    // Add random edges to the solution (to be implemented based on specific problem requirements)
}

std::pair<std::vector<std::vector<int>>, double> SolveSubinstance(const std::vector<std::vector<int>>& C, double t_ILP) {
    // Solve the sub-instance (to be implemented based on specific problem requirements)
    // Return a pair containing the solution and the computation time
    return {{}, 0.0};
}

void Initialize(int& n_a, int& l_size, double& gamma) {
    // TODO: Initialize parameters (to be implemented based on specific problem requirements)
    n_a = n_init; 
    l_size = l_size_init;
    gamma = gamma_init; 
    // DONE.
}

void Increment(int& n_a, int& l_size, double& gamma) {
    // TODO: Increment parameters (to be implemented based on specific problem requirements)
    n_a += n_inc;
    l_size += l_size_inc;
    gamma += gamma_inc;
    // DONE. 
}

void LocalSearch(std::vector<std::vector<int>>& s, int id){
    // TODO
}
