#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <map>
#include <cmath>
#include "config.h"
#include "argparse.h"
#include "util.h"
#include "move.h"

struct Point
{
    short id; // DC_ID: depot is 0, otherwise: customers or stations (not used in EVRP-TW-SPD)
    short type;  // depot = 0, customer = 1, station = 2
    double x; // x-axis coordinate
    double y; // y-axis coordinate
    double delivery; // delivery demand
    double pickup; // piuk_up demand  
    double start; // start of time window
    double end; // end of time window
    double s_time;   //service time 
    bool idle; // specify the dummy station is available or not
};

struct Vehicle
{
    short max_num;    // for EVRP-TW-SPD, it is not necessary    
    double d_cost; // dispatching cost
    double unit_cost; // unit cost for transportation
    double capacity;  // loading capacity
    double battery;  // battery capacity
    double consumption_rate; // the battery energy consumption rate of an electric vehicle per unit distance travelled
    double recharging_rate; //  it takes time recharging_rate to charge per unit of battery energy
    
};


class Data
{
public:
    // problem definitions
    std::string problem_name;
    std::vector<Point> node;
    int node_num; // include dummy stations
    int node_cardinality; // unique node
    int customer_num; // shouldn't be larger than MAX_POINT
    int station_num; // shouldn't be larger than MAX_STATION_POINT, dummy stations
    int station_cardinality; // unique stations
    int station_range;  // the number of charging stations considered, i.e., sr $\in$ (0, 1], the selection range parameter.
    std::vector<std::vector<double>> dist;
    std::vector<std::vector<double>> time;
    std::vector<std::vector<double>> rm;
    std::vector<std::vector<int>> rm_argrank;
    std::vector<std::vector<bool>> pm;
    std::vector<std::vector<std::vector<int>>> optimal_staion;  // preprocess the charging stations to rank them for insertion between each pair of nodes
    std::vector<std::vector<std::vector<int>>> hyperarc; // record information of "the shortest path in terms of travel time between every pair of nodes" to hyperarc

    Vehicle vehicle;
    double max_dist = 0; // max value in dist
    double min_dist = double(INFINITY); // min value in dist
    double all_pickup; // all pick-up demands
    double all_delivery; // all delivery demands
    double all_dist; // all distance
    double all_time; // all time
    double start_time; // the earliest time of servicing
    double end_time;   // the latest time of returning to depot
    double max_distance_reachable; // given the current battery capacity, the maximum distance that EV can reach
    

    int DC; // ID of depot, 0 by default
     
    // parameters
    bool pruning = DEFAULT_PRUNING; // whether do pruning
    bool O_1_evl = DEFAULT_O_1_EVAL; // whether do O(1) evaluation
    bool no_crossover = DEFAULT_NO_CROSSOVER; // whether do crossover
    bool if_output = DEFAULT_IF_OUTPUT; // whether output to file
    std::string output = " "; // output file path
    int tmax = NO_LIMIT; // max running time (s), -1 means no limit
    int g_1 = G_1; // terminate MA when no improving in g_1 gens
    int g_2 = G_2; // terminate DECOMPOSITION when no improving in g_2 gens
    int runs = RUNS; // restarts MA for runs times
    int p_size = P_SIZE; // population size
    int k_init = K; //the number of generated solutions for producing a solution during initialization
    int k_crossover = K; //the number of generated solutions for producing a solution during crossover
    int ksize = 0;
    int seed = DEFAULT_SEED; // random seed to initialize rng
    double bks = -1.0;       // best known solution
    std::mt19937 rng; // random number generator
    std::string init = DEFAULT_INIT; // initialization method
    std::string cross_repair = DEFAULT_CROSSOVER; // insertion used in crossover
    std::tuple<double, double> lambda_gamma = std::make_tuple(0.5,0.5); // params of RCRS method
    std::vector<std::tuple<double, double>> latin;
    std::string n_insert = ""; //indicate which type-1 insertion should be used, RCRS or TD

    std::string selection = DEFAULT_SEL;           //parent selection method
    std::string replacement = DEFAULT_REPLACEMENT; //replacement strategy

    double ls_prob = DEFAULT_LS_PROB; //local search probability
    bool skip_finding_lo = DEFAULT_SKIP_FINDING_LO; // if skip finding_local_optima
    std::map<std::string, std::vector<Move>> mem;
    bool two_opt = DEFAULT_2_OPT; // 2-opt
    std::vector<Move> mem_2opt;
    bool two_opt_star = DEFAULT_2_OPT_STAR; //2-opt*
    std::vector<Move> mem_2optstar;
    bool or_opt = DEFAULT_OR_OPT; // or-opt
    std::vector<Move> mem_oropt_single;
    std::vector<Move> mem_oropt_double;
    bool two_exchange = DEFAULT_2_EX; // 2-exchange
    std::vector<Move> mem_2ex;

    int or_opt_len = DEFAULT_OR_OPT_LEN; //max length of seqs relocated by oropt
    int exchange_len = DEFAUTL_EX_LEN;   //max length of seqs exchanged
    int subproblem_range = DEFAULT_SUBPROBLEM; // // BCD decomposition. `subproblem = 1` means no decomposition; otherwise, it indicates decomposition.
    
    int escape_local_optima = DEFAULT_ELO;            // number of times of escaping local optima
    double destroy_ratio_l = DEFAULT_DESTROY_RATIO_L; //customers to be delete in recombination
    double destroy_ratio_u = DEFAULT_DESTROY_RATIO_U;
    bool random_removal = DEFAULT_RD_REMOVAL;
    bool related_removal = DEFAULT_RT_REMOVAL;
    bool greedy_insertion = DEFAULT_GD_INSERTION;
    bool regret_insertion = DEFAULT_RG_INSERTION;
    double alpha = DEFAULT_ALPHA;
    double r = 0.0;
    bool rd_removal_insertion = DEFAULT_RD_R_I;
    bool individual_search = DEFAULT_INDIVIDUAL_SEARCH; // large neighborhood search
    bool population_search = DEFAULT_POPULATION_SEARCH; // memetic search
    bool parallel_insertion = DEFAULT_PARALLEL_STATION_INSERTION; // PSI (SSI is used in CDNS by default)
    bool conservative_local_search = DEFAULT_CONSERVATIVE_LOCAL_SEARCH; // CLS
    bool aggressive_local_search = DEFAULT_AGGRESSIVE_LOCAL_SEARCH; // ALS (however, at least one local search should use)

    /* Adapt-CMSA-STD Parameters*/
    double t_prop = DEFAULT_T_PROP;
    double t_ILP = DEFAULT_T_ILP;
    double alpha_LB = DEFAULT_ALPHA_LB; 
    double alpha_UB = DEFAULT_ALPHA_UB; 
    double alpha_red = DEFAULT_ALPHA_RED; 
    double d_rate = DEFAULT_D_RATE; 
    double h_rate = DEFAULT_H_RATE; 
    int n_a = DEFAULT_N_A; 
    int l_size = DEFAULT_L_SIZE; 
    //double gamma; 
    int delta_n = DEFAULT_DELTA_N; 
    int delta_l_size = DEFAULT_DELTA_L_SIZE; 
    //double delta_gamma; 
    //double infeasible_rate; 
    int dummy_stations = DEFAULT_DUMMY_STATIONS;    


    std::vector<std::string> small_opts;
    std::vector<std::string> destroy_opts;
    std::vector<std::string> repair_opts;
    Data(ArgumentParser &parser); // read problem files, set parameters
    void pre_processing();
    void clear_mem();
    Move& get_mem(std::string &opt, const int &r1, const int &r2);
};