#include"search_framework.h"
#include <cstdio>
#include <cstdlib>
#include <utility>
extern clock_t find_best_time;
extern clock_t find_bks_time;
extern int find_best_run;
extern int find_best_gen;
extern int find_bks_run;
extern int find_bks_gen;
extern bool find_better;
extern long call_count_move_eval;
extern long mean_duration_move_eval;
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

void update_best_solution(Solution &s, Solution &best_s, clock_t used, int run, int gen, Data &data)
{
    if (s.cost - best_s.cost < -PRECISION)
    {
        best_s = s;
        printf("Best solution update: %.4f\n", best_s.cost);
        find_best_time = used;
        find_best_run = run;
        find_best_gen = gen;
        if (!find_better && (std::abs(best_s.cost - data.bks) < PRECISION ||
                             (best_s.cost - data.bks < -PRECISION)))
        {
            find_better = true;
            find_bks_time = used;
            find_bks_run = run;
            find_bks_gen = gen;
        }
    }
}

/*
*************************** Main Adapt-CMSA-SETCOV Function ***************************  
    a self-adapted CMSA algorithm for EVRP-TW-SPD 
    CMSA: construct, merge, solve & adapt
*/
void Adapt_CMSA_SETCOV(Data &data, Solution &best_s){
    // TODO
    double cost_all_run = 0.0; 
    double time_all_run = 0.0;
    vector<double> solutions;
    vector<double> times;

    double t_prop = data.t_prop; // a proportion
    double t_ILP = data.t_ILP; // the CPU time limit for the application of CPLEX to solve the ILP model  
    double alpha_LB = data.alpha_LB; // a lower bound for alpha_bsf
    double alpha_UB = data.alpha_UB; // an upper bound for alpha_bsf
    double alpha_red = data.alpha_red; // the step size (learning/self-adaptive rate) for the reduction of alpha_bsf
    // double alpha_bsf = data.alpha_UB; 
    double d_rate = data.d_rate; // the probability of performing a step during solution construction deterministically. 
    double h_rate = data.h_rate; // the probability to choose - the C&W savings heuristic (h_rate) - the insertion heuristic (1 - h_rate) as solution construction mechanism. 
    //int n_a = data.n_a; // a number of n_a solutions are probabilistically constructed
    //int l_size = data.l_size; // controls the number of considered options (perturb operators?) at each solution construction step.

    // int numNodes = data.node_num ; // add "1" more since CPLEX need return depot 

    /* --- main body --- */
    bool time_exhausted = false;
    int run = 1;
    for (; run <= data.runs; run++)
    {
        printf("---------------------------------Run %d---------------------------\n", run);
        clock_t stime = clock();
        clock_t used = 0;
        double used_sec = 0.0;
        int no_improve = 0;
        int iter = 0;
        
        Solution s_bsf(data);
        /* ------------------------------ */
        std::vector<std::vector<int>> adjMatrix;  // C
        adjMatrix.resize(data.node_num, std::vector<int>(data.node_num, 0)); 

        data.alpha_bsf = data.alpha_UB;
        Initialize(data);

        GenerateGreedySolution(s_bsf, adjMatrix, data);
        used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
        printf("cost %.2lf, already consumed %.2lf sec\n", s_bsf.cost, used_sec); 

        // exit(0);

        double cost_in_this_run = s_bsf.cost;
        double time_bks = used_sec; // time to find/generate the best solution
        /* ------------------------------ */

        // no_improve = data.g_1 + 1;

        while (!termination(no_improve, data))
        {
            iter++;
            no_improve++;
            /* ------------------------------ */
            // store merged arc information in adjMatrix, update in each iteration
            adjMatrix.assign(data.node_num, std::vector<int>(data.node_num, 0));
            s_bsf.adjMatrixRepresentation(adjMatrix);

            // store adjacency matrix of s_bsf in adjMatrix_tmp
            std::vector<std::vector<int>> adjMatrix_tmp;
            adjMatrix_tmp = adjMatrix;

            // init adjacency matrix of s in adjMatrix_s
            std::vector<std::vector<int>> adjMatrix_s;
            adjMatrix_s.resize(data.node_num, std::vector<int>(data.node_num, 0)); 

            Solution tour_set(data);
            InitializeTour(data);
            Merge(tour_set, s_bsf, data);

            for (int i = 0; i < data.n_a; ++i) {
                Solution s(data);
                ProbabilisticSolutionConstruction(s, adjMatrix_tmp, data);

                // prevent from running out of time
                used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
                if (data.tmax == NO_LIMIT || used_sec < clock_t(data.tmax)) {
                    LocalSearch(s, data, stime, 1);
                }

                adjMatrix_s.assign(data.node_num, std::vector<int>(data.node_num, 0));
                s.adjMatrixRepresentation(adjMatrix_s);
                Merge(adjMatrix, adjMatrix_s, data.node_num);
                Merge(tour_set, s, data);                

                if (s.cost < s_bsf.cost) {
                    s_bsf = s;
                    cost_in_this_run = s_bsf.cost;
                    time_bks = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
                }

                // prevent from running out of time
                used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
                if (data.tmax != NO_LIMIT && used_sec > clock_t(data.tmax))
                {
                    time_exhausted = true;
                    break;
                }

            }
            
            Solution s_cplex(data);
            double t_solve = 0.0;
            
            // prevent from running out of time
            used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
            if (data.tmax != NO_LIMIT && clock_t(data.tmax) - used_sec < t_ILP){
                s_cplex = s_bsf;
            }
            else {
                // SolveSubinstance(s_cplex, t_solve, adjMatrix, t_ILP, data);
                 SolveSubinstance(s_cplex, t_solve, tour_set, t_ILP, data);
                // s_cplex is a solution obtained by solving the ILP_SETCOV model, may includes duplicate customers 
                // handle exception: empty s_cplex when CPLEX fails to find a solution
                if (s_cplex.cost == double(INFINITY)) {
                    s_cplex = s_bsf;
                }
                else{
                    RemoveDuplicates(s_cplex, data);
                    // if RemoveDuplicates(s_cplex, data) fails to find a solution, i.e., s_cplex violates the constraints (battery, time windows), 
                    // then s_cplex = s_bsf
                    if (s_cplex.cost == double(INFINITY)) {
                        s_cplex = s_bsf;
                    }
                }
            }
            
            // prevent from running out of time
            used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
            if (data.tmax == NO_LIMIT || used_sec < clock_t(data.tmax)){
                LocalSearch(s_cplex, data, stime, 2);
            }

            /* Adapt: alpha_bsf, n_a, l_size */
            if (t_solve < t_prop * t_ILP && data.alpha_bsf > alpha_LB) {
                data.alpha_bsf -= alpha_red;
            }

            if (s_cplex.cost < s_bsf.cost) {      // solution in CPLEX is strictly better 
                s_bsf = s_cplex;
                Initialize(data);          // reset
                used = (clock() - stime) / CLOCKS_PER_SEC;
                update_best_solution(s_bsf, best_s, used, run, iter, data);
            } else {
                if (s_cplex.cost > s_bsf.cost) {  // solution in CPLEX is strictly worse
                    if (data.n_a == data.init_n_a) {
                        data.alpha_bsf = std::min(data.alpha_bsf + alpha_red / 10.0, alpha_UB);  // enlarge alpha_bsf
                    } 
                    else {
                        Initialize(data);  // reset
                    }
                } 
                else {                                                   // solution in CPLEX is exactly the same
                    Increment(data);       // increment
                }
            }
                
            /* ------------------------------ */
            if (s_bsf.cost - cost_in_this_run < -PRECISION)
            {
                    no_improve = 0;
                    cost_in_this_run = s_bsf.cost;
                    time_bks = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
            }
            used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
            used = (clock() - stime) / CLOCKS_PER_SEC;

            if (iter % OUTPUT_PER_GENS == 0)
            {
                // output(pop, pop_fit, pop_argrank, data);
                printf("Iter %d done, no improvement for %d iters, already consumed %.2lf sec\n", iter, no_improve, used_sec);
            }

            if (data.tmax != NO_LIMIT && used_sec > clock_t(data.tmax))
            {
                time_exhausted = true;
                break;
            }        
        } 

        used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
        used = (clock() - stime) / CLOCKS_PER_SEC;

        update_best_solution(s_bsf, best_s, used, run, iter, data);

        printf("Run %d finishes\n", run);
        cost_all_run += s_bsf.cost;
        // time_all_run += used_sec;
        time_all_run += time_bks;

        solutions.push_back(s_bsf.cost);
        // times.push_back(used_sec);
        times.push_back(time_bks);

        data.rng.seed(data.seed + run);

    }

    // TODO: print solutions (to be fixed)
    printf("------------Summary-----------\n");
    best_s.output(data);
    if (!best_s.check(data)) {
        printf("\n Error: the best solution is not feasible \n");
    }
    // check if feasible, then save best solution and run time in file
    printf("Total %d runs, total consumed %.2lf sec\n", run-1, time_all_run);
    
    write_solution_to_file(data, best_s, run, solutions, times, cost_all_run, time_all_run);
    // exit(0);

}
/*
    initializes the best-so-far solution S_bsf to a feasible solution obtained utilizing an insertion heuristic
*/

void GenerateGreedySolution(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data){
    Solution s_tmp(data);
    const int max_attempts = 100;
    int attempts = 0;

    while (!ProbabilisticInsertion(s_tmp, adjMatrix, data) && attempts < max_attempts) {
        s_tmp = Solution(data);
        ++attempts;
    }

    if (attempts < max_attempts) {
        s = std::move(s_tmp);
    } else {
        std::cerr << "Warning: ProbabilisticInsertion exceeded max attempts." << std::endl;
    }

    // Solution s_tmp(data);
    // while (!ProbabilisticInsertion(s_tmp, adjMatrix, data)) {
    //     s_tmp = Solution(data);
    // }
    // s = s_tmp;
    // ProbabilisticInsertion(s, adjMatrix, data);
    // ProbabilisticClarkWrightSavings(s, adjMatrix, data);
}

void ProbabilisticSolutionConstruction(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data){
    Solution s_tmp(data);
    const int max_attempts = 100;
    int attempts = 0;
    bool isCWsavings = (rand(0.0, 1.0, data.rng) <= data.h_rate);
    if (isCWsavings) {
        // printf("ProbabilisticClarkWrightSavings\n");
        while (!ProbabilisticClarkWrightSavings(s_tmp, adjMatrix, data) && attempts < max_attempts) {
            s_tmp = Solution(data);
            ++attempts;
        }

        if (attempts < max_attempts) {
            s = std::move(s_tmp);
        } else {
            std::cerr << "Warning: ProbabilisticClarkWrightSavings exceeded max attempts." << std::endl;
        }
        // ProbabilisticClarkWrightSavings(s, adjMatrix, data);
        /* 
            the C&W savings heuristic
                Clarke, G., & Wright, J. W. (1964). Scheduling of vehicles from a central depot to a number of delivery points. 
                Operations research, 12(4), 568-581. https://doi.org/10.1287/opre.12.4.568
        */
    } 
    else {
        // printf("ProbabilisticInsertion\n");
        while (!ProbabilisticInsertion(s_tmp, adjMatrix, data) && attempts < max_attempts) {
            s_tmp = Solution(data);
            ++attempts;
        }

        if (attempts < max_attempts) {
            s = std::move(s_tmp);
        } else {
            std::cerr << "Warning: ProbabilisticInsertion exceeded max attempts." << std::endl;
        }
        //ProbabilisticInsertion(s, adjMatrix, data);
    } 
}

void Merge(std::vector<std::vector<int>>& adjMatrix1, std::vector<std::vector<int>>& adjMatrix2, int numNodes){
    for (int i = 0; i < numNodes; i++) {
        for (int j = 0; j < numNodes; j++) {
            adjMatrix1[i][j] |= adjMatrix2[i][j]; 
        }
    }    
}

void Merge(Solution &tour_set, Solution &s, Data& data){
    std::vector<IloInt> indexArray(data.cplex_data.numCustomers);
    for (int i = 0; i < s.len(); i++){
        Route &r = s.get(i);
        tour_set.append(r);
        data.cplex_data.transcost.push_back(r.transcost);
        for (auto node: r.node_list){
            if (data.node[node].type == 1){
                indexArray[node-1] = i + data.cplex_data.numRoutes;
            }
        }
    }
    data.cplex_data.routesIndex.push_back(indexArray);
    data.cplex_data.numSolutions++;
    data.cplex_data.numRoutes+= s.len();
}

void SolveSubinstance(Solution &s_cplex, double &t_solve, Solution& tour_set, double t_ILP, Data &data){
    ILPSETCOVmodel(s_cplex, t_solve, data, t_ILP, tour_set); // solve the ILPSETCOV model to get the optimal solution
}

void Initialize(Data& data){
    data.n_a = data.init_n_a;
    data.l_size = data.init_l_size;
}

void Increment(Data& data){
    data.n_a += data.delta_n;
    data.l_size += data.delta_l_size;
}

void LocalSearch(Solution &s, Data& data, clock_t stime, int id){
    // printf("Before LocalSearch%d: %.2lf\n", id, s.cost);
    double base_cost = s.cost;

    for (int j = 0; j< s.len(); j++ ){
        Route &r = s.get(j);
        r.customer_list = r.node_list;
        auto &nodes = r.customer_list;
        nodes.erase(std::remove_if(nodes.begin(), nodes.end(), 
                [&data](int node) { return data.node[node].type == 2; }), nodes.end());
        r.node_list.swap(r.customer_list);
        r.update(data);    
        //no need to update total_cost                      
    }   

    find_local_optima(s, data, stime, base_cost, id-1);

    for (int j = 0; j< s.len(); j++ ){
        Route &r = s.get(j);
        std::swap(r.node_list, r.customer_list);
        r.update(data);                          
    } 
    s.cal_cost(data);

    //printf("After LocalSearch%d: %.2lf\n", id, s.cost);
}

void InitializeTour(Data& data){
    data.cplex_data.numSolutions = 0;
    data.cplex_data.numRoutes = 0;
    data.cplex_data.transcost.clear();
    data.cplex_data.routesIndex.clear();
}

void write_solution_to_file(Data &data, Solution &s_bsf, int run, const std::vector<double> &solutions, const std::vector<double> &times, double cost_all_run, double time_all_run) {

    std::string timelimit = std::to_string(data.tmax);
    std::string subproblem_range = std::to_string(data.subproblem_range);
    std::string filename = data.output + data.problem_name + "_timelimit=" + timelimit + "_subproblem=" +  subproblem_range + ".txt";
    FILE *file_solution = fopen(filename.c_str(), "a");
    if (file_solution == nullptr) {
        perror("Failed to open file");
    }
    std::string output_s = "Details of the solution:\n";
    int len = s_bsf.len();
    for (int i = 0; i < len; i++)
    {
        Route &r=s_bsf.get(i);
        int flag = 0;
        double new_cost = 0.0;
        int index_negtive_first = -1;
        std::vector<int> &nl = r.node_list;
        output_s += "route " + std::to_string(i) +
                    ", node_num " + std::to_string(nl.size()) +
                    ", cost " + std::to_string(r.transcost) +
                    ", nodes:";
        update_route_status(nl, r.status_list, data, flag, new_cost, index_negtive_first);         
        for (int j = 0; j < nl.size(); j++)
        {  
            int node = nl[j];
            output_s += ' ' + std::to_string(node);
            if (data.node[node].type != 1){
                std::ostringstream stream1;
                std::ostringstream stream2;
                stream1 << std::fixed << std::setprecision(2) << r.status_list[j].arr_RD;
                stream2 << std::fixed << std::setprecision(2) << r.status_list[j].dep_RD;
                std::string Str1 = stream1.str();
                std::string Str2 = stream2.str();
                output_s += "(" + Str1 + ", "+ Str2 + ")";

            }
        }
        output_s += '\n';
    }
    output_s += "vehicle (route) number: " + std::to_string(len) + '\n';

    std::ostringstream stream;
    
    stream << std::fixed << std::setprecision(2) << s_bsf.cost;
    std::string costStr = stream.str();

    output_s += "Total cost: " + costStr + '\n';
    const char* c_output_s = output_s.c_str();
    fprintf(file_solution, "%s", c_output_s);
    for (int i = 0; i < run - 1; i++){
        fprintf(file_solution, "%.2lf, %.2lf\n", solutions[i], times[i]);
    }
    fclose(file_solution);
    
    std::string filename_output = data.output +"output_1.txt";
    FILE *file = fopen(filename_output.c_str(), "a");
    if (file == nullptr) {
        perror("Failed to open file");
    }
    const char* c_str = data.problem_name.c_str();
    fprintf(file, "%s: %d, %.2lf, %.2lf, %.2lf, subproblem = %d, timelimit = %d\n", c_str, s_bsf.len(), s_bsf.cost, cost_all_run / (run-1), time_all_run / (run-1), data.subproblem_range, data.tmax);
    fclose(file);
}