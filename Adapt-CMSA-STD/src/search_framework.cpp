#include"search_framework.h"
#include <cstdlib>
extern clock_t find_best_time;
extern clock_t find_bks_time;
extern int find_best_run;
extern int find_best_gen;
extern int find_bks_run;
extern int find_bks_gen;
extern bool find_better;
extern long call_count_move_eval;
extern long mean_duration_move_eval;
// grobal varivales
int dim;
int n_init;
int n_inc;          
int l_size_init;
int l_size_inc;
double gamma_init;
double gamma_inc;

/*
*************************** Main Adapt-CMSA-STD Function ***************************  
    a self-adapted CMSA algorithm for EVRP-TW-SPD 
    CMSA: construct, merge, solve & adapt
*/
void Adapt_CMSA_STD(Data &data, Solution &s_bsf){
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
    double alpha_bsf = data.alpha_UB; 
    double d_rate = data.d_rate; // the probability of performing a step during solution construction deterministically. 
    double h_rate = data.h_rate; // the probability to choose - the C&W savings heuristic (h_rate) - the insertion heuristic (1 - h_rate) as solution construction mechanism. 
    int n_a = data.n_a; // a number of n_a solutions are probabilistically constructed
    int l_size = data.l_size; // controls the number of considered options (perturb operators?) at each solution construction step.

    int numNodes = data.node_num + 1; // add "1" more since CPLEX need return depot 
    std::vector<std::vector<int>> adjMatrix;
    adjMatrix.resize(numNodes, std::vector<int>(numNodes, 0)); 
    std::vector<std::vector<int>> adjMatrix_s;
    adjMatrix_s.resize(numNodes, std::vector<int>(numNodes, 0)); 

    /* --- main body --- */

    ILPmodel(data, 100); // solve the ILP model to get the optimal solution

    exit(0);

    bool time_exhausted = false;
    int run = 1;
    exit(0);
    for (; run <= data.runs; run++)
    {
        
        /* ------------------------------ */

        GenerateGreedySolution(s_bsf);
        alpha_bsf = data.alpha_UB;
        Initialize(n_a, l_size, data);

        /* ------------------------------ */

        clock_t stime = clock();
        clock_t used = 0;
        double used_sec = 0.0;
        int no_improve = 0;
        int iter = 0;

        while (!termination(no_improve, data))
        {
            iter++;
            no_improve++;

            /* ------------------------------ */
            adjMatrix.assign(numNodes, std::vector<int>(numNodes, 0));
            s_bsf.adjMatrixRepresentation(adjMatrix, numNodes);

            for (int i = 0; i < n_a; ++i) {
                Solution s;
                ProbabilisticSolutionConstruction(s, s_bsf, alpha_bsf, l_size, data);

                LocalSearch(s, 1);

                adjMatrix_s.assign(numNodes, std::vector<int>(numNodes, 0));
                s.adjMatrixRepresentation(adjMatrix_s, numNodes);
                Merge(adjMatrix, adjMatrix_s, numNodes);
            }
            
            Solution s_cplex;
            double t_solve = 0.0;

            SolveSubinstance(s_cplex, t_solve, adjMatrix, t_ILP, data); 

            LocalSearch(s_cplex, 2);

            /* Adapt: alpha_bsf, n_a, l_size */
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
                Initialize(n_a, l_size, data);          // reset
            } else {
                if ( true /* fitness of s_cplex >  fitness of s_bsf */) {  // solution in CPLEX is strictly worse
                    if (n_a == n_init) {
                        alpha_bsf = std::min(alpha_bsf + alpha_red / 10.0, alpha_UB);  // enlarge alpha_bsf
                    } 
                    else {
                        Initialize(n_a, l_size, data);  // reset
                    }
                } 
                else {                                                   // solution in CPLEX is exactly the same
                    Increment(n_a, l_size, data);       // increment
                }
            }
                
            /* ------------------------------ */
            used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
            used = (clock() - stime) / CLOCKS_PER_SEC;

            if (iter % OUTPUT_PER_GENS == 0)
            {
                printf("Iter: %d. ", iter);
                // output(pop, pop_fit, pop_argrank, data);
                printf("Iter %d done, no improvement for %d iters, already consumed %.2lf sec\n", iter, no_improve, used_sec);
            }

            if (data.tmax != NO_LIMIT && used_sec > clock_t(data.tmax))
            {
                time_exhausted = true;
                break;
            }        
        }    
    }

    // TODO: print solutions (to be fixed)
    printf("------------Summary-----------\n");
    s_bsf.output(data);
    if (!s_bsf.check(data)) exit(0);  // check if feasible, then save best solution and run time in file
    printf("Total %d runs, total consumed %.2lf sec\n", run-1, time_all_run);
    
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
        // update_route_status(nl, r.status_list, data, flag, new_cost, index_negtive_first); 
        int pre = -1;            
        for (int j = 0; j < nl.size(); j++)
        {  
            int node = nl[j];
            if (pre != -1){
                for (int sub_node: data.hyperarc[pre][node]){
                        output_s += ' ' + std::to_string(sub_node);
                }
            }
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
            pre = node;
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
/*
    initializes the best-so-far solution S_bsf to a feasible solution obtained utilizing an insertion heuristic
*/

void GenerateGreedySolution(Solution &s){

}

void ProbabilisticSolutionConstruction(Solution &s, Solution &s_bsf, double alpha_bsf, int l_size, Data &data){
    double h_rate = data.h_rate;
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

    if (isCWsavings) {

    } 
    else {
    }
}

void Merge(std::vector<std::vector<int>>& adjMatrix1, std::vector<std::vector<int>>& adjMatrix2, int numNodes){
    for (int i = 0; i < numNodes; i++) {
        for (int j = 0; j < numNodes; j++) {
            adjMatrix1[i][j] |= adjMatrix2[i][j]; 
        }
    }    
}

void SolveSubinstance(Solution &s_cplex, double &t_solve, std::vector<std::vector<int>>&adjMatrix, double t_ILP, Data &data){
    

}

void Initialize(int& n_a, int& l_size, Data& data){
    n_a = data.n_a;
    l_size = data.l_size;
}

void Increment(int& n_a, int& l_size, Data& data){
    n_a += data.delta_n;
    l_size += data.delta_l_size;
}

void LocalSearch(Solution &s, int id){

}