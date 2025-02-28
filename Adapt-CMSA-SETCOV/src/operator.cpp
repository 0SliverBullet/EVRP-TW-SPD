#include "operator.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ostream>
#include <string>
#include <vector>

// LocalSearch1: intra-route operators; LocalSearch2: inter-route operators
//*************************************************************************************
// for preventing memory allocation
Move tmp_move;
// map from string to opt;
std::map<std::string, std::function<void(int, int, Solution&, Data&, Move&, double&)>>
small_opt_map = {
    {"relocation", relocation}, 
    {"swap", exchange_swap},
    {"2opt", two_opt},
    {"exchange_1_1", exchange_1_1},
    {"shift_1_0", shift_1_0}
    };

bool iscustomerlist(const std::vector<int>& s1, const std::vector<int>& s2) {

    if (s1.empty()) return true;

    size_t j = 0; 

    for (size_t i = 0; i < s2.size(); ++i) {
        if (s2[i] == s1[j]) {
            ++j; 
        }
        if (j == s1.size()) {
            return true; 
        }
    }
    return false; 
}

void snippet(int r1, int r2, std::string &opt, Solution &s, Data &data, Move &target, double &base_cost)
{
    auto &m = data.get_mem(opt, r1, r2);
    small_opt_map[opt](r1, r2, s, data, m, base_cost);
    if (m.delta_cost - target.delta_cost < -PRECISION)
        target = m;
}

void find_local_optima(Solution &s, Data &data, clock_t stime, double base_cost, int id)
{
    // delta_value: EVRP-TW-SPD
    
    if (int(data.small_opts[id].size() == 0))
    {
        printf("No small stepsize operator used, directly return.\n");
        return;
    }

    // record the best solution in the neighborhood of each small opt

    std::vector<Move> move_list(int(data.small_opts[id].size()));

    // find the best move for all sub-neighbors of each opt
    int len = int(s.len());
    for (int i = 0; i < int(move_list.size()); i++)
    {
        move_list[i].delta_cost = double(INFINITY);
        auto &opt = data.small_opts[id][i];
        
        // std::cout<<opt<<std::endl;
        
        if (opt == "relocation" || opt == "swap" || opt == "2opt")
        {
            for (int r = 0; r < len; r++)
            {
                snippet(r, -1, opt, s, data, move_list[i], base_cost);
            }
        }
        else if (opt == "exchange_1_1" || opt == "shift_1_0")
        {
            for (int r1 = 0; r1 < len; r1++)
            {
                for (int r2 = r1 + 1; r2 < len; r2++)
                {
                    snippet(r1, r2, opt, s, data, move_list[i], base_cost);
                }
            }
        }
        else
        {
            std::cout << "Unknown opt: " << opt;
            exit(-1);
        }
    }

    // double acc_delta_cost = 0;
    std::vector<int> tour_id_array;
    //std::vector<int> no_use;
    double used_sec = 0.0;

    while (true)
    {
        int best_index = -1;
        double min_delta_cost = double(INFINITY);

        //std::cout<<move_list.size()<<std::endl;

        for (int i = 0; i < int(move_list.size()); i++)
        {

            if (move_list[i].delta_cost - min_delta_cost < -PRECISION)
            {
                
                best_index = i;
                min_delta_cost = move_list[i].delta_cost;
            }
        }

        // prevent from running out of time
        used_sec = (clock() - stime) / (CLOCKS_PER_SEC*1.0);
        
        if ((data.tmax == NO_LIMIT || used_sec < clock_t(data.tmax)) && min_delta_cost < -PRECISION)
        {
            // apply move
            std::unordered_set<int> set = {0, 1};
            tour_id_array = apply_move(s, move_list[best_index], data);
            for (int j = 0; j < tour_id_array.size(); j++){
                if (tour_id_array[j] >= s.len()) continue;
                // s.get(tour_id_array[j]).customer_list = move_list[best_index].list[j];
                // s.get(tour_id_array[j]).total_cost = move_list[best_index].total_cost[j];
                for (int k = 0; k < 2; k++){  
                    if (set.find(k) == set.end()) continue; 
                    // if (std::find(move_list[best_index].list[k].begin(), move_list[best_index].list[k].end(), s.get(tour_id_array[j]).node_list[1]) != move_list[best_index].list[k].end())
                    if (iscustomerlist(s.get(tour_id_array[j]).node_list, move_list[best_index].list[k]))
                    {
                        set.erase(k);
                        s.get(tour_id_array[j]).customer_list = move_list[best_index].list[k];
                        s.get(tour_id_array[j]).total_cost = move_list[best_index].total_cost[k];
                        break;
                    }
                }

            }
            s.cost += min_delta_cost;
            
            // printf("%.2lf\n", s.cost);        
            base_cost = s.cost;

            // update move_list
            int len = s.len();

            // for (int node = data.customer_num+1; node <= data.customer_num + data.station_num; node++){
            //     s.idle[node] = true;
            // }

            // printf("solution: \n");
            // for (int i = 0; i < len; i++){
            //     Route& r = s.get(i);
            //     for (int node: r.node_list){
            //         printf("%d ", node);
            //     }
            //     printf("\n");
            // }
            


            for (int i = 0; i < int(move_list.size()); i++)
            {
                move_list[i].delta_cost = double(INFINITY);
                auto &opt = data.small_opts[id][i];

                //std::cout<<opt<<std::endl;

                if (opt == "relocation" || opt == "swap" || opt == "2opt")
                {
                    for (auto &r : tour_id_array)
                    {
                        if (r >= len) continue;
                        snippet(r, -1, opt, s, data, move_list[i], base_cost);
                    }
                    for (int r = 0; r < len; r++)
                    {
                        if (data.get_mem(opt, r, -1).delta_cost - move_list[i].delta_cost < -PRECISION)
                            move_list[i] = data.get_mem(opt, r, -1);
                    }
                }
                else if (opt == "exchange_1_1" || opt == "shift_1_0")
                {
                    for (auto &r : tour_id_array)
                    {
                        if (r >= len) continue;
                        for (int r1 = 0; r1 < r; r1++)
                        {
                            snippet(r1, r, opt, s, data, move_list[i], base_cost);
                        }
                        for (int r1 = r+1; r1 < len; r1++)
                        {
                            snippet(r, r1, opt, s, data, move_list[i], base_cost);
                        }
                    }
                    for (int r1 = 0; r1 < len; r1++)
                    {
                        for (int r2 = r1 + 1; r2 < len; r2++)
                        {
                            if (data.get_mem(opt, r1, r2).delta_cost - move_list[i].delta_cost < -PRECISION)
                                move_list[i] = data.get_mem(opt, r1, r2);
                        }
                    }
                }
                else
                {
                    std::cout << "Unknown opt: " << opt;
                    exit(-1);
                }
            }
        }
        else break;
    }
}

void relocation(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost)
{
    m.delta_cost = double(INFINITY);
    Route &r = s.get(r1);
    auto &n_l = r.node_list;
    int len = int(n_l.size());
    if (len < 4) return;
    for (int start = 1; start <= len - 2; start++)
    {
        // data.or_opt = 1 by default
        for (int seq_len = 1; seq_len <= data.relocation_len; seq_len++)
        {
            int end = start + seq_len - 1;
            if (end >= len - 1) continue;
            if (data.pruning &&
                (!data.pm[n_l[start-1]][n_l[end+1]]))
                continue;
            // relocate to the same route
            for (int pos = 1; pos <= start - 1; pos++)
            {
                if (data.pruning &&
                    (!data.pm[n_l[pos-1]][n_l[start]] ||\
                        !data.pm[n_l[end]][n_l[pos]]))
                        continue;
                tmp_move.r_indice[0] = r1;
                tmp_move.r_indice[1] = -2;
                tmp_move.len_1 = 4;
                tmp_move.seqList_1[0] = {r1, 0, pos-1};
                tmp_move.seqList_1[1] = {r1, start, end};
                tmp_move.seqList_1[2] = {r1, pos, start-1};
                tmp_move.seqList_1[3] = {r1, end+1, len-1};
                tmp_move.len_2 = 0;
                if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
                {
                    m = tmp_move;
                }
            }
            for (int pos = end + 2; pos <= len - 1; pos++)
            {
                if (data.pruning &&
                    (!data.pm[n_l[pos-1]][n_l[start]] ||
                        !data.pm[n_l[end]][n_l[pos]]))
                    continue;
                tmp_move.r_indice[0] = r1;
                tmp_move.r_indice[1] = -2;
                tmp_move.len_1 = 4;
                tmp_move.seqList_1[0] = {r1, 0, start-1};
                tmp_move.seqList_1[1] = {r1, end+1, pos-1};
                tmp_move.seqList_1[2] = {r1, start, end};
                tmp_move.seqList_1[3] = {r1, pos, len-1};
                tmp_move.len_2 = 0;
                if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
                {
                    m = tmp_move;
                }
            }
            // // relocate to a new route
            // tmp_move.r_indice[0] = r1;
            // tmp_move.r_indice[1] = -1;
            // tmp_move.len_1 = 2;
            // tmp_move.seqList_1[0] = {r1, 0, start - 1};
            // tmp_move.seqList_1[1] = {r1, end + 1, len - 1};
            // tmp_move.len_2 = 3;
            // tmp_move.seqList_2[0] = {-1, data.DC, data.DC};
            // tmp_move.seqList_2[1] = {r1, start, end};
            // tmp_move.seqList_2[2] = {-1, data.DC, data.DC};
            // if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
            // {
            //     m = tmp_move;
            // }
        }
    }
}
void exchange_swap(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost){
    m.delta_cost = double(INFINITY);
    Route &r = s.get(r1);
    auto &n_l = r.node_list;
    int len = int(n_l.size());
    if (len < 4) return;
    for (int start1 = 1; start1 <= len -3; start1++){
        for (int start2 = start1 + 1; start2 <= len -2; start2++){
            if (start1 >= len -1 || start2 >= len -1) continue;
            if (data.pruning &&
                (!data.pm[n_l[start1-1]][n_l[start2]]||\
                 !data.pm[n_l[start2]][n_l[start1+1]]||\
                 !data.pm[n_l[start2-1]][n_l[start1]]||\
                 !data.pm[n_l[start1]][n_l[start2+1]]))
                continue;
            if (start2 - start1 > 1){
                tmp_move.r_indice[0] = r1;
                tmp_move.r_indice[1] = -2;
                tmp_move.len_1 = 5;
                tmp_move.seqList_1[0] = {r1, 0, start1-1};
                tmp_move.seqList_1[1] = {r1, start2, start2};
                tmp_move.seqList_1[2] = {r1, start1+1, start2-1};
                tmp_move.seqList_1[3] = {r1, start1, start1};
                tmp_move.seqList_1[4] = {r1, start2+1, len-1};
                tmp_move.len_2 = 0;
                if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
                {
                    m = tmp_move;
                }
            }
            else{
                tmp_move.r_indice[0] = r1;
                tmp_move.r_indice[1] = -2;
                tmp_move.len_1 = 4;
                tmp_move.seqList_1[0] = {r1, 0, start1-1};
                tmp_move.seqList_1[1] = {r1, start2, start2};
                tmp_move.seqList_1[2] = {r1, start1, start1};
                tmp_move.seqList_1[3] = {r1, start2+1, len-1};
                tmp_move.len_2 = 0;
                if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
                {
                    m = tmp_move;
                }

            }

            
        }
    }
}

void two_opt(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost)
{
    // inverse a 2-sequence in a route
    m.delta_cost = double(INFINITY);

    Route &r = s.get(r1);
    auto &n_l = r.node_list; // customer
    int len = int(n_l.size());
    if (len < 5) return;
    for (int start = 1; start <= len - 4; start++)
    {
        for (int seq_len = 3; seq_len <= len - 2; seq_len++)
        {
            // choose two non-adjacent nodes, avoid equivalent operation to `swap` when `seq_len = 2` 
            int end = start + seq_len - 1;
            if (end >= len - 1) continue;

            bool valid = true;
            if (data.pruning) {
                if (!data.pm[n_l[start-1]][n_l[end]]) {
                    valid = false;
                }
                for (int i = end; i > start; --i) {
                    if (!data.pm[n_l[i]][n_l[i-1]]) {
                        valid = false;
                        break;
                    }
                }
                if (valid && (!data.pm[n_l[start]][n_l[end+1]])) {
                    valid = false;
                }
                if (!valid) {
                    continue;
                }
            }

            for (int i = end; i > start; --i) {
                if (r.gat(i, i-1).num_cus == INFEASIBLE) {
                    valid = false;
                    break;
                }
            }

            if (!valid) {
                continue;
            }

        tmp_move.r_indice[0] = r1;
        tmp_move.r_indice[1] = -2;  // -2 表示没有使用 r2
        tmp_move.len_1 = 3;
        tmp_move.seqList_1[0] = {r1, 0, start-1};
        tmp_move.seqList_1[1] = {r1, end, start};
        tmp_move.seqList_1[2] = {r1, end+1, len-1};
        tmp_move.len_2 = 0;
        if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
        {
            m = tmp_move;
        }      
        }
    }
}

void exchange_1_1(int r1, int r2, Solution &s, Data &data, Move &m, double &base_cost){
    m.delta_cost = double(INFINITY);
    // exchange two sequences with seqs
    // printf("%d, %d\n", r1, r2);
    Route &r_1 = s.get(r1);
    auto &n_l_1 = r_1.node_list;
    int len_1 = int(n_l_1.size());

    Route &r_2 = s.get(r2);
    auto &n_l_2 = r_2.node_list;
    int len_2 = int(n_l_2.size());
    for (int start_1 = 1; start_1 <= len_1 - 2; start_1++)
    {
        for (int seq_len_1 = 1; seq_len_1 <= data.exchange_1_1_len; seq_len_1++)
        {
            int end_1 = start_1 + seq_len_1 - 1;
            if (end_1 >= len_1 - 1) continue;
            for (int start_2 = 1; start_2 <= len_2 - 2; start_2++)
            {
                for (int seq_len_2 = 1; seq_len_2 <= data.exchange_1_1_len; seq_len_2++)
                {
                    int end_2 = start_2 + seq_len_2 - 1;
                    if (end_2 >= len_2 - 1) continue;
                    // printf("index: %d, %d; ", start_1-1, start_2);
                    // printf("len: %d, %d; ", int(n_l_1.size()), int(n_l_2.size()));
                    // printf("customer: %d\n", n_l_1[start_1-1]);
                    // for (int node: n_l_1){
                    //     printf("%d ", node);
                    // }
                    // printf("\n");
                    // for (int node: n_l_2){
                    //     printf("%d ", node);
                    // }
                    // printf("\n");
                    if (data.pruning &&
                        (!data.pm[n_l_1[start_1-1]][n_l_2[start_2]] ||\
                            !data.pm[n_l_2[end_2]][n_l_1[end_1+1]] ||\
                            !data.pm[n_l_2[start_2-1]][n_l_1[start_1]] ||\
                            !data.pm[n_l_1[end_1]][n_l_2[end_2+1]]))
                        continue;
                    tmp_move.r_indice[0] = r1;
                    tmp_move.r_indice[1] = r2;
                    tmp_move.len_1 = 3;
                    tmp_move.seqList_1[0] = {r1, 0, start_1-1};
                    tmp_move.seqList_1[1] = {r2, start_2, end_2};
                    tmp_move.seqList_1[2] = {r1, end_1+1, len_1-1};
                    tmp_move.len_2 = 3;
                    tmp_move.seqList_2[0] = {r2, 0, start_2-1};
                    tmp_move.seqList_2[1] = {r1, start_1, end_1};
                    tmp_move.seqList_2[2] = {r2, end_2+1, len_2-1};
                    if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
                    {
                        m = tmp_move;
                    }
                }
            }
        }
    }
}

void shift_1_0(int r_index_1, int r_index_2, Solution &s, Data &data, Move &m, double &base_cost)
{
    m.delta_cost = double(INFINITY);
    // relocate a sequence
    for (int i = 0; i < 2; i++)
    {
        int r1, r2;
        if (i == 0)
        {
            r1 = r_index_1;
            r2 = r_index_2;
        }
        else if (i == 1)
        {
            r1 = r_index_2;
            r2 = r_index_1;
        }
        Route &r = s.get(r1);
        auto &n_l = r.node_list;
        int len = int(n_l.size());
        for (int start = 1; start <= len - 2; start++)
        {
            for (int seq_len = 1; seq_len <= data.shift_1_0_len; seq_len++)
            {
                int end = start + seq_len - 1;
                if (end >= len - 1)
                    continue;
                if (data.pruning &&
                    (!data.pm[n_l[start - 1]][n_l[end + 1]]))
                    continue;
                // relocate to other route

                if (r1 == r2)
                    continue;
                Route &r_2 = s.get(r2);
                auto &n_l_2 = r_2.node_list;
                int len_2 = int(n_l_2.size());
                for (int pos = 1; pos <= len_2 - 1; pos++)
                {
                    if (data.pruning &&
                        (!data.pm[n_l_2[pos - 1]][n_l[start]] ||
                         !data.pm[n_l[end]][n_l_2[pos]]))
                        continue;
                    tmp_move.r_indice[0] = r1;
                    tmp_move.r_indice[1] = r2;
                    tmp_move.len_1 = 2;
                    tmp_move.seqList_1[0] = {r1, 0, start - 1};
                    tmp_move.seqList_1[1] = {r1, end + 1, len - 1};
                    tmp_move.len_2 = 3;
                    tmp_move.seqList_2[0] = {r2, 0, pos - 1};
                    tmp_move.seqList_2[1] = {r1, start, end};
                    tmp_move.seqList_2[2] = {r2, pos, len_2 - 1};
                    if (eval_move(s, tmp_move, data, base_cost) && tmp_move.delta_cost < m.delta_cost)
                    {
                        m = tmp_move;
                    }
                }
            }
        }
    }
}


void RemoveDuplicates(Solution &s, Data& data){

    vector<int> unique(data.customer_num + 1, 0);
    int len = s.len();
    for (int i = 0; i < len; i++){
        Route &r = s.get(i);
        std::vector<int> &nl = r.node_list;
        for (int node : nl){
            if (data.node[node].type == 1){
                unique[node]++;
            }
        }
    }

    int select = -1, r_index = -1, pos = -1;
    double benefit = -double(INFINITY);

    // Loop to find and remove duplicates
    while (true) {
        // Find the next node to remove
        select = -1;
        benefit = -double(INFINITY);
        
        for (int i = 0; i < len; i++) {
            Route &r = s.get(i);
            std::vector<int> &nl = r.node_list;
            
            for (int j = 1; j < nl.size() - 1; j++) {  // Avoid first and last node
                int node = nl[j];
                
                if (data.node[node].type == 1 && unique[node] > 1) {
                    int prev = nl[j - 1], next = nl[j + 1];
                    double TD = data.dist[prev][node] + data.dist[node][next] - data.dist[prev][next];
                    
                    if (TD > benefit) {
                        select = node;
                        r_index = i;
                        pos = j;
                        benefit = TD;
                    }
                }
            }
        }

        // If no more duplicates are found, break the loop
        if (select == -1) {
            break;
        }

        // Remove the selected node
        Route &r = s.get(r_index);
        std::vector<int> &nl = r.node_list;
        nl.erase(nl.begin() + pos);  // Remove the node at the specified position

        // Decrement the unique count for the selected node
        unique[select]--;
    }

    s.update(data);

    len = s.len();
    for (int i = 0; i < len; i++) {
        Route &r = s.get(i);
        std::vector<int> &nl = r.node_list;
        nl.erase(std::remove_if(nl.begin(), nl.end(), 
                [&data](int node) { return data.node[node].type == 2; }), nl.end());
        r.temp_node_list = nl;
        int flag = 0,  index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
        double new_cost = 0.0;
        update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量
        std::vector<std::pair<int,int>> station_insert_pos;
        station_insert_pos.clear(); 
        if (flag == 1 || (flag == 4 && sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s))) {
            for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
                r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
            }
        }
        else{
            s.cost = double(INFINITY);
            return;
        }
    }

    s.update(data);
    s.cal_cost(data); 
}


// heuristic construction
// ************************************************************************************
// i是unrouted中的下标，node是要从unrouted移除的节点， index是unrouted的长度
void maintain_unrouted(int i, int node, int &index, std::vector<std::tuple<int, int>> &unrouted, double &unrouted_d, double &unrouted_p, Data &data)
{
    unrouted[i] = unrouted[index-1];  // move the last one to the ith position, and then index-- (effectively remove the ith one)
    index--;
    unrouted_d -= data.node[node].delivery;
    unrouted_p -= data.node[node].pickup;
} 


void find_unrouted(Solution &s, std::vector<int> &record)
{
    int len = s.len();
    for (int i = 0; i < len; i++)
    {
        Route &r = s.get(i);
        for (auto node : r.node_list)
        {    
            if (node < record.size()) record[node] = 1;
        }
    }
}

// generate a set of direct routes, each route contains only one customer
void direct_routes_init(Solution &s, Data &data)  
{
    double unrouted_d = data.all_delivery;
    double unrouted_p = data.all_pickup;

    int num_cus = data.customer_num;
    std::vector<int> record(num_cus + 1, 0);
    find_unrouted(s, record);
    std::vector<std::tuple<int, int>> unrouted(data.customer_num);
    std::vector<int> station_pos(data.station_num);
    
    int index = 0;
    for (int i = 0; i < num_cus + 1; i++)
    {
        if (i != data.DC && record[i] == 0)
        {
            std::get<0>(unrouted[index]) = i;
            index++;
        }
        else if (i != data.DC && record[i] == 1)
        {
            unrouted_d -= data.node[i].delivery;
            unrouted_p -= data.node[i].pickup;
        }
    }

    while (index > 0)   //# unrouted customers > 0
    {   
        Route r(data);
        int selected = -1;
        int first_node = -1;
        int flag = 0, index_negtive_first = -1;
        double new_cost = 0.0;

        std::vector<std::pair<int,int>> station_insert_pos;
        station_insert_pos.reserve(MAX_NODE_IN_ROUTE);


        selected = randint(0, index-1, data.rng);
        first_node = std::get<0>(unrouted[selected]);

        r.temp_node_list = r.node_list;
        r.temp_node_list.insert(r.temp_node_list.begin() + 1, first_node); 
        flag = 0, new_cost = 0.0, index_negtive_first = -1;
        update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  
        station_insert_pos.clear();  //need to be optimized, not eligant
        if (flag == 0 || flag == 2 || flag == 3) {  // "flag ==3 -> no solution" holds in the Euclidean plane
            printf("%d, ", flag);
            printf("No solution.\n");
            exit(0);
        }
        if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s)) {
            printf("%d, ", flag);
            printf("No solution.\n");
            exit(0);            
        }
        maintain_unrouted(selected, first_node, index, unrouted, unrouted_d, unrouted_p, data);
        r.node_list.insert(r.node_list.begin() + 1, first_node);  

        for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
            r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
            // s.idle[station_insert_pos[i].first] = false;
        }

        s.append(r);
        // for (int node: r.node_list){
        //     printf("%d, ", node);
        // }
        // printf("\n");
    }
    s.update(data);
    s.cal_cost(data);
}

template <typename Tuple, size_t ProbIndex>
Tuple select_tuple_from_Lr(const std::vector<Tuple>& Lr, double total_q, Data& data) {
    double rand_prob = rand(0.0, 1.0, data.rng) * total_q;  
    double cumulative_prob = 0.0;
    for (const auto& element : Lr) {
        cumulative_prob += std::get<ProbIndex>(element);
        if (cumulative_prob >= rand_prob) {
            return element;
        }
    }
    return Lr.back();
}

template <typename Tuple, size_t ProbIndex>
size_t select_tuple_index_from_Lr(const std::vector<Tuple>& Lr, double total_q, Data& data) {
    double rand_prob = rand(0.0, 1.0, data.rng) * total_q;
    double cumulative_prob = 0.0;
    for (size_t i = 0; i < Lr.size(); ++i) {
        cumulative_prob += std::get<ProbIndex>(Lr[i]);
        if (cumulative_prob >= rand_prob) {
            return i;  
        }
    }
    return Lr.size() - 1; 
}


struct CompareTuple {
    bool operator()(const std::tuple<int, int, int, int, double>& a,
                    const std::tuple<int, int, int, int, double>& b) const {
        if (std::get<4>(a) == std::get<4>(b)) {
            if (std::get<0>(a) != std::get<0>(b)) {
                return std::get<0>(a) < std::get<0>(b);  
            }
            if (std::get<1>(a) != std::get<1>(b)) {
                return std::get<1>(a) < std::get<1>(b);  
            }
            if (std::get<2>(a) != std::get<2>(b)) {
                return std::get<2>(a) < std::get<2>(b);  
            }
            if (std::get<3>(a) != std::get<3>(b)) {
                return std::get<3>(a) < std::get<3>(b);  
            }
        }
        return std::get<4>(a) > std::get<4>(b); 
    }
};



void mergeRoute(Solution& s, std::tuple<int, int, int, int, double>& selected_tuple, std::vector<std::vector<int>>& adjMatrix, Data& data, \
                std::vector<std::pair<int,int>>& station_insert_pos, \
                std::set<std::tuple<int, int, int, int, double>, CompareTuple>& saving_list, \
                std::unordered_map<std::string, double>& saving_cache) {
    
    int i = std::get<0>(selected_tuple);
    int dir1 = std::get<1>(selected_tuple);
    int j = std::get<2>(selected_tuple);
    int dir2 = std::get<3>(selected_tuple);

    Route &r1 = s.get(i);
    Route &r2 = s.get(j);
    std::vector<int> &nl1 = r1.node_list;
    std::vector<int> &nl2 = r2.node_list;
    int len = s.len(), len1 = nl1.size(), len2 = nl2.size();

    int flag = 0, index_negtive_first = -1;
    double new_cost = 0.0;
    int concatenate = 0;

    Route r(data);
    std::vector<int> nl;   
    nl.push_back(0);
    if (dir1 == -1 && dir2 == 1){
        concatenate = 1;
        for (int k = len1 -2; k >= 1; k--){
            nl.push_back(nl1[k]);
        }
        for (int k = 1; k <= len2 - 2; k++){
            nl.push_back(nl2[k]);
        }                  
    }   
    if (dir1 == -1 && dir2 == -1){
        concatenate = 2;
        for (int k = len1 - 2; k >= 1; k--){
            nl.push_back(nl1[k]);
        }
        for (int k = len2 - 2; k >= 1; k--){
            nl.push_back(nl2[k]);
        }
    } 
    if (dir1 == 1 && dir2 == 1){
        concatenate = 3;
        for (int k = 1; k <= len1 -2; k++){
            nl.push_back(nl1[k]);
        }
        for (int k = 1; k <= len2 - 2; k++){
            nl.push_back(nl2[k]);
        }
    }
    if (dir1 == 1 && dir2 == -1){
        concatenate = 4;
        for (int k = 1; k <= len1 -2; k++){
            nl.push_back(nl1[k]);
        }
        for (int k = len2 - 2; k >= 1; k--){
            nl.push_back(nl2[k]);
        }
    }
    nl.push_back(0);
    
    // 充电站冗余识别
    // std::vector<int> redundant_station;
    // for (int k = 1; k < nl.size() - 1; k++) {
    //     if (data.node[nl[k]].type == 2) {
    //         redundant_station.push_back(nl[k]);
            // s.idle[nl[k]] = true;
    //     }
    // }

    nl.erase(std::remove_if(nl.begin(), nl.end(), 
            [&data](int node) { return data.node[node].type == 2; }), nl.end());
    
    r.node_list = nl;
    r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
    flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
    update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量

    station_insert_pos.clear(); 

    // upate saving_list
    auto generate_cache_key = [](int i, int j, int concatenate) {
        return std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(concatenate);
    };

    if (flag == 1 || (flag == 4 && sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s))) {

        // printf("Route %d and Route %d are merged. ", i, j);

        for (int k = 0; k < station_insert_pos.size(); k++){  // the same insertion order
            r.node_list.insert(r.node_list.begin() + station_insert_pos[k].second, station_insert_pos[k].first); 
            // s.idle[station_insert_pos[k].first] = false;
        } // 将缓存的充电站插入信息 应用到 route 中

        s.get(i) = r;  // 更新 route i
        s.get(j) = s.get(len-1);
        s.del(len-1);  // 删除 route j
        s.update(data);  // 更新 solution
        s.cal_cost(data);  // 计算 solution 的 cost

        len = s.len();

        if (i < s.len() && j < s.len()){
            
            Route &ri = s.get(i);
            std::vector<int> &nli = ri.node_list;


            for (int k = 0; k < len; k++) {

                    saving_list.erase(std::make_tuple(len, -1, k, 1, saving_cache[generate_cache_key(len, k, 1)]));
                    saving_list.erase(std::make_tuple(len, -1, k, -1, saving_cache[generate_cache_key(len, k, 2)]));
                    saving_list.erase(std::make_tuple(len, 1, k, 1, saving_cache[generate_cache_key(len, k, 3)]));
                    saving_list.erase(std::make_tuple(len, 1, k, -1, saving_cache[generate_cache_key(len, k, 4)]));

                    saving_list.erase(std::make_tuple(k, -1, len, 1, saving_cache[generate_cache_key(k, len, 1)]));
                    saving_list.erase(std::make_tuple(k, -1, len, -1, saving_cache[generate_cache_key(k, len, 2)]));
                    saving_list.erase(std::make_tuple(k, 1, len, 1, saving_cache[generate_cache_key(k, len, 3)]));
                    saving_list.erase(std::make_tuple(k, 1, len, -1, saving_cache[generate_cache_key(k, len, 4)]));

                    if (k != i){

                        Route &rk = s.get(k);
                        std::vector<int> &nlk = rk.node_list;    
                        int leni = nli.size(), lenk = nlk.size();       

                        double saving_value = 0.0;

                        // (i, k)

                        saving_list.erase(std::make_tuple(i, -1, k, 1, saving_cache[generate_cache_key(i, k, 1)]));
                        saving_value = cal_additional_savings_value(nli[1], nlk[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(i, -1, k, 1, saving_value));
                        saving_cache[generate_cache_key(i, k, 1)] = saving_value;

                        saving_list.erase(std::make_tuple(i, -1, k, -1, saving_cache[generate_cache_key(i, k, 2)]));
                        saving_value = cal_additional_savings_value(nli[1], nlk[lenk - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(i, -1, k, -1, saving_value));
                        saving_cache[generate_cache_key(i, k, 2)] = saving_value;

                        saving_list.erase(std::make_tuple(i, 1, k, 1, saving_cache[generate_cache_key(i, k, 3)]));
                        saving_value = cal_additional_savings_value(nli[leni - 2], nlk[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(i, 1, k, 1, saving_value));
                        saving_cache[generate_cache_key(i, k, 3)] = saving_value;

                        saving_list.erase(std::make_tuple(i, 1, k, -1, saving_cache[generate_cache_key(i, k, 4)]));
                        saving_value = cal_additional_savings_value(nli[leni - 2], nlk[lenk - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(i, 1, k, -1, saving_value));
                        saving_cache[generate_cache_key(i, k, 4)] = saving_value;

                        // (k, i)
                        saving_list.erase(std::make_tuple(k, -1, i, 1, saving_cache[generate_cache_key(k, i, 1)]));
                        saving_value = cal_additional_savings_value(nlk[1], nli[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, -1, i, 1, saving_value));
                        saving_cache[generate_cache_key(k, i, 1)] = saving_value;

                        saving_list.erase(std::make_tuple(k, -1, i, -1, saving_cache[generate_cache_key(k, i, 2)]));
                        saving_value = cal_additional_savings_value(nlk[1], nli[leni - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, -1, i, -1, saving_value));
                        saving_cache[generate_cache_key(k, i, 2)] = saving_value;

                        saving_list.erase(std::make_tuple(k, 1, i, 1, saving_cache[generate_cache_key(k, i, 3)]));
                        saving_value = cal_additional_savings_value(nlk[lenk - 2], nli[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, 1, i, 1, saving_value));
                        saving_cache[generate_cache_key(k, i, 3)] = saving_value;

                        saving_list.erase(std::make_tuple(k, 1, i, -1, saving_cache[generate_cache_key(k, i, 4)]));
                        saving_value = cal_additional_savings_value(nlk[lenk - 2], nli[leni - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, 1, i, -1, saving_value));
                        saving_cache[generate_cache_key(k, i, 4)] = saving_value;

                    }
            }

            for (int k = 0; k < len; k++) {
                
                if (k !=i && k != j){
                        saving_list.erase(std::make_tuple(j, -1, k, 1, saving_cache[generate_cache_key(j, k, 1)]));
                        saving_list.erase(std::make_tuple(j, -1, k, -1, saving_cache[generate_cache_key(j, k, 2)]));
                        saving_list.erase(std::make_tuple(j, 1, k, 1, saving_cache[generate_cache_key(j, k, 3)]));
                        saving_list.erase(std::make_tuple(j, 1, k, -1, saving_cache[generate_cache_key(j, k, 4)]));

                        saving_list.erase(std::make_tuple(k, -1, j, 1, saving_cache[generate_cache_key(k, j, 1)]));
                        saving_list.erase(std::make_tuple(k, -1, j, -1, saving_cache[generate_cache_key(k, j, 2)]));
                        saving_list.erase(std::make_tuple(k, 1, j, 1, saving_cache[generate_cache_key(k, j, 3)]));
                        saving_list.erase(std::make_tuple(k, 1, j, -1, saving_cache[generate_cache_key(k, j, 4)]));
                }

                for (int l = 1 ; l <= 4;  l++){
                    if (k !=i && k != j){
                        saving_cache[generate_cache_key(j, k, l)] = saving_cache[generate_cache_key(len, k, l)];
                        saving_cache[generate_cache_key(k, j, l)] = saving_cache[generate_cache_key(k, len, l)];               
                    }
                }

                if (k !=i && k != j){
                        saving_list.insert(std::make_tuple(j, -1, k, 1, saving_cache[generate_cache_key(j, k, 1)]));
                        saving_list.insert(std::make_tuple(j, -1, k, -1, saving_cache[generate_cache_key(j, k, 2)]));
                        saving_list.insert(std::make_tuple(j, 1, k, 1, saving_cache[generate_cache_key(j, k, 3)]));
                        saving_list.insert(std::make_tuple(j, 1, k, -1, saving_cache[generate_cache_key(j, k, 4)]));

                        saving_list.insert(std::make_tuple(k, -1, j, 1, saving_cache[generate_cache_key(k, j, 1)]));
                        saving_list.insert(std::make_tuple(k, -1, j, -1, saving_cache[generate_cache_key(k, j, 2)]));
                        saving_list.insert(std::make_tuple(k, 1, j, 1, saving_cache[generate_cache_key(k, j, 3)]));
                        saving_list.insert(std::make_tuple(k, 1, j, -1, saving_cache[generate_cache_key(k, j, 4)]));
                }
            }


        }
        else if (i == s.len() && j < s.len()){

            Route &ri = s.get(j);
            std::vector<int> &nli = ri.node_list;


            for (int k = 0; k < len; k++) {

                    saving_list.erase(std::make_tuple(len, -1, k, 1, saving_cache[generate_cache_key(len, k, 1)]));
                    saving_list.erase(std::make_tuple(len, -1, k, -1, saving_cache[generate_cache_key(len, k, 2)]));
                    saving_list.erase(std::make_tuple(len, 1, k, 1, saving_cache[generate_cache_key(len, k, 3)]));
                    saving_list.erase(std::make_tuple(len, 1, k, -1, saving_cache[generate_cache_key(len, k, 4)]));

                    saving_list.erase(std::make_tuple(k, -1, len, 1, saving_cache[generate_cache_key(k, len, 1)]));
                    saving_list.erase(std::make_tuple(k, -1, len, -1, saving_cache[generate_cache_key(k, len, 2)]));
                    saving_list.erase(std::make_tuple(k, 1, len, 1, saving_cache[generate_cache_key(k, len, 3)]));
                    saving_list.erase(std::make_tuple(k, 1, len, -1, saving_cache[generate_cache_key(k, len, 4)]));

                    if (k != j){

                        Route &rk = s.get(k);
                        std::vector<int> &nlk = rk.node_list;    
                        int leni = nli.size(), lenk = nlk.size();       

                        double saving_value = 0.0;

                        // (j, k)

                        saving_list.erase(std::make_tuple(j, -1, k, 1, saving_cache[generate_cache_key(j, k, 1)]));
                        saving_value = cal_additional_savings_value(nli[1], nlk[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(j, -1, k, 1, saving_value));
                        saving_cache[generate_cache_key(j, k, 1)] = saving_value;

                        saving_list.erase(std::make_tuple(j, -1, k, -1, saving_cache[generate_cache_key(j, k, 2)]));
                        saving_value = cal_additional_savings_value(nli[1], nlk[lenk - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(j, -1, k, -1, saving_value));
                        saving_cache[generate_cache_key(j, k, 2)] = saving_value;

                        saving_list.erase(std::make_tuple(j, 1, k, 1, saving_cache[generate_cache_key(j, k, 3)]));
                        saving_value = cal_additional_savings_value(nli[leni - 2], nlk[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(j, 1, k, 1, saving_value));
                        saving_cache[generate_cache_key(j, k, 3)] = saving_value;

                        saving_list.erase(std::make_tuple(j, 1, k, -1, saving_cache[generate_cache_key(j, k, 4)]));
                        saving_value = cal_additional_savings_value(nli[leni - 2], nlk[lenk - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(j, 1, k, -1, saving_value));
                        saving_cache[generate_cache_key(j, k, 4)] = saving_value;

                        // (k, j)
                        saving_list.erase(std::make_tuple(k, -1, j, 1, saving_cache[generate_cache_key(k, j, 1)]));
                        saving_value = cal_additional_savings_value(nlk[1], nli[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, -1, j, 1, saving_value));
                        saving_cache[generate_cache_key(k, j, 1)] = saving_value;

                        saving_list.erase(std::make_tuple(k, -1, j, -1, saving_cache[generate_cache_key(k, j, 2)]));
                        saving_value = cal_additional_savings_value(nlk[1], nli[leni - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, -1, j, -1, saving_value));
                        saving_cache[generate_cache_key(k, j, 2)] = saving_value;

                        saving_list.erase(std::make_tuple(k, 1, j, 1, saving_cache[generate_cache_key(k, j, 3)]));
                        saving_value = cal_additional_savings_value(nlk[lenk - 2], nli[1], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, 1, j, 1, saving_value));
                        saving_cache[generate_cache_key(k, j, 3)] = saving_value;

                        saving_list.erase(std::make_tuple(k, 1, j, -1, saving_cache[generate_cache_key(k, j, 4)]));
                        saving_value = cal_additional_savings_value(nlk[lenk - 2], nli[leni - 2], adjMatrix, data);
                        saving_list.insert(std::make_tuple(k, 1, j, -1, saving_value));
                        saving_cache[generate_cache_key(k, j, 4)] = saving_value;

                    }
            }


        }
        else if (i < s.len() && j == s.len()){

                Route &ri = s.get(i);
                std::vector<int> &nli = ri.node_list;


                for (int k = 0; k < len; k++) {

                        saving_list.erase(std::make_tuple(len, -1, k, 1, saving_cache[generate_cache_key(len, k, 1)]));
                        saving_list.erase(std::make_tuple(len, -1, k, -1, saving_cache[generate_cache_key(len, k, 2)]));
                        saving_list.erase(std::make_tuple(len, 1, k, 1, saving_cache[generate_cache_key(len, k, 3)]));
                        saving_list.erase(std::make_tuple(len, 1, k, -1, saving_cache[generate_cache_key(len, k, 4)]));

                        saving_list.erase(std::make_tuple(k, -1, len, 1, saving_cache[generate_cache_key(k, len, 1)]));
                        saving_list.erase(std::make_tuple(k, -1, len, -1, saving_cache[generate_cache_key(k, len, 2)]));
                        saving_list.erase(std::make_tuple(k, 1, len, 1, saving_cache[generate_cache_key(k, len, 3)]));
                        saving_list.erase(std::make_tuple(k, 1, len, -1, saving_cache[generate_cache_key(k, len, 4)]));

                        if (k != i){

                            Route &rk = s.get(k);
                            std::vector<int> &nlk = rk.node_list;    
                            int leni = nli.size(), lenk = nlk.size();       

                            double saving_value = 0.0;

                            // (i, k)

                            saving_list.erase(std::make_tuple(i, -1, k, 1, saving_cache[generate_cache_key(i, k, 1)]));
                            saving_value = cal_additional_savings_value(nli[1], nlk[1], adjMatrix, data);
                            saving_list.insert(std::make_tuple(i, -1, k, 1, saving_value));
                            saving_cache[generate_cache_key(i, k, 1)] = saving_value;

                            saving_list.erase(std::make_tuple(i, -1, k, -1, saving_cache[generate_cache_key(i, k, 2)]));
                            saving_value = cal_additional_savings_value(nli[1], nlk[lenk - 2], adjMatrix, data);
                            saving_list.insert(std::make_tuple(i, -1, k, -1, saving_value));
                            saving_cache[generate_cache_key(i, k, 2)] = saving_value;

                            saving_list.erase(std::make_tuple(i, 1, k, 1, saving_cache[generate_cache_key(i, k, 3)]));
                            saving_value = cal_additional_savings_value(nli[leni - 2], nlk[1], adjMatrix, data);
                            saving_list.insert(std::make_tuple(i, 1, k, 1, saving_value));
                            saving_cache[generate_cache_key(i, k, 3)] = saving_value;

                            saving_list.erase(std::make_tuple(i, 1, k, -1, saving_cache[generate_cache_key(i, k, 4)]));
                            saving_value = cal_additional_savings_value(nli[leni - 2], nlk[lenk - 2], adjMatrix, data);
                            saving_list.insert(std::make_tuple(i, 1, k, -1, saving_value));
                            saving_cache[generate_cache_key(i, k, 4)] = saving_value;

                            // (k, i)
                            saving_list.erase(std::make_tuple(k, -1, i, 1, saving_cache[generate_cache_key(k, i, 1)]));
                            saving_value = cal_additional_savings_value(nlk[1], nli[1], adjMatrix, data);
                            saving_list.insert(std::make_tuple(k, -1, i, 1, saving_value));
                            saving_cache[generate_cache_key(k, i, 1)] = saving_value;

                            saving_list.erase(std::make_tuple(k, -1, i, -1, saving_cache[generate_cache_key(k, i, 2)]));
                            saving_value = cal_additional_savings_value(nlk[1], nli[leni - 2], adjMatrix, data);
                            saving_list.insert(std::make_tuple(k, -1, i, -1, saving_value));
                            saving_cache[generate_cache_key(k, i, 2)] = saving_value;

                            saving_list.erase(std::make_tuple(k, 1, i, 1, saving_cache[generate_cache_key(k, i, 3)]));
                            saving_value = cal_additional_savings_value(nlk[lenk - 2], nli[1], adjMatrix, data);
                            saving_list.insert(std::make_tuple(k, 1, i, 1, saving_value));
                            saving_cache[generate_cache_key(k, i, 3)] = saving_value;

                            saving_list.erase(std::make_tuple(k, 1, i, -1, saving_cache[generate_cache_key(k, i, 4)]));
                            saving_value = cal_additional_savings_value(nlk[lenk - 2], nli[leni - 2], adjMatrix, data);
                            saving_list.insert(std::make_tuple(k, 1, i, -1, saving_value));
                            saving_cache[generate_cache_key(k, i, 4)] = saving_value;

                        }
                }

        }


    }
    else{
        
        // printf("Route %d and Route %d are not merged. ", i, j);

        // for (auto node : redundant_station) {
            // s.idle[node] = false;
        // }
        // printf("(%d, %d, %d, %d, %.2lf) ", i, dir1, j, dir2, std::get<4>(selected_tuple));
        saving_list.erase(selected_tuple);
        // printf("%zu ", saving_list.size());
        saving_list.insert(std::make_tuple(i, dir1, j, dir2, - double(INFINITY)));
        // printf("%zu ", saving_list.size());
        saving_cache[generate_cache_key(i, j, concatenate)] = - double(INFINITY);
    }


}



/********** Probabilistic C&W savings algorithm **********/
bool ProbabilisticClarkWrightSavings(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data) {

    direct_routes_init(s, data);

    std::set<std::tuple<int, int, int, int, double>, CompareTuple> saving_list;

    int flag = 0, index_negtive_first = -1;
    double new_cost = 0.0;
    std::vector<std::pair<int,int>> station_insert_pos;
    station_insert_pos.reserve(MAX_NODE_IN_ROUTE);

    std::unordered_map<std::string, double> saving_cache;

    auto generate_cache_key = [](int i, int j, int concatenate) {
        return std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(concatenate);
    };

    int len = s.len();
    
    for (int i = 0; i < len; i++) {
        Route &r1 = s.get(i);
        for (int j = 0; j < len; j++) {

            if (i != j) {
                Route &r2 = s.get(j);
                std::vector<int> &nl1 = r1.node_list;
                std::vector<int> &nl2 = r2.node_list;
                int len1 = nl1.size(), len2 = nl2.size();

                double saving_value = 0.0;

                saving_value = cal_additional_savings_value(nl1[1], nl2[1], adjMatrix, data);
                saving_list.insert(std::make_tuple(i, -1, j, 1, saving_value));
                saving_cache[generate_cache_key(i, j, 1)] = saving_value;

                saving_value = cal_additional_savings_value(nl1[1], nl2[len2 - 2], adjMatrix, data);
                saving_list.insert(std::make_tuple(i, -1, j, -1, saving_value));
                saving_cache[generate_cache_key(i, j, 2)] = saving_value;

                saving_value = cal_additional_savings_value(nl1[len1 - 2], nl2[1], adjMatrix, data);
                saving_list.insert(std::make_tuple(i, 1, j, 1, saving_value));
                saving_cache[generate_cache_key(i, j, 3)] = saving_value;


                saving_value = cal_additional_savings_value(nl1[len1 - 2], nl2[len2 - 2], adjMatrix, data);
                saving_list.insert(std::make_tuple(i, 1, j, -1, saving_value));
                saving_cache[generate_cache_key(i, j, 4)] = saving_value;
            }
        }
    }

    // if (saving_list.size() == 0) break;

    // auto start_time = std::chrono::steady_clock::now();
    // auto end_time = std::chrono::steady_clock::now();

    while (true) {

        // printf("%d, ", s.len());
        // printf("saving_list size: %zu\n", saving_list.size());

        // start_time = std::chrono::steady_clock::now();

        std::vector<std::tuple<int, int, int, int, double>> saving_list_r;

        int l_size = std::min(data.l_size, (int)saving_list.size());
        auto it = saving_list.begin();

        for (int i = 0; i < l_size; ++i, ++it) {
            if (std::get<4>(*it) == - double(INFINITY)) break;
            saving_list_r.push_back(*it);
        }

        // end_time = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        // printf("sorting time: %f\n", elapsed_seconds.count());

        if (saving_list_r.size() == 0) break;
        
        double total_q = 0.0;
        for (const auto& element : saving_list_r) {
            total_q += std::get<4>(element);
        }

        auto selected_tuple = select_tuple_from_Lr<std::tuple<int, int, int, int, double>, 4>(saving_list_r, total_q, data);

        // merge route and update saving_list
        mergeRoute(s, selected_tuple, adjMatrix, data, station_insert_pos, saving_list, saving_cache);
    }

    return true;
}



/********** Probabilistic Insertion Algorithm **********/
bool ProbabilisticInsertion(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data){

    double unrouted_d = data.all_delivery;
    double unrouted_p = data.all_pickup;

    int num_cus = data.customer_num;
    std::vector<int> record(num_cus + 1, 0);
    find_unrouted(s, record);
    std::vector<std::tuple<int, int>> unrouted(data.customer_num);
    std::vector<bool> unrouted_flag(data.customer_num + 1, true);
    // std::vector<int> station_pos(data.station_num);

    int index = 0; // 维护unrouted数组有效长度 [0, index-1]
    for (int i = 0; i < num_cus + 1; i++)
    {
        if (i != data.DC && record[i] == 0)
        {
            std::get<0>(unrouted[index]) = i;
            index++;
        }
        else if (i != data.DC && record[i] == 1)
        {
            unrouted_d -= data.node[i].delivery;
            unrouted_p -= data.node[i].pickup;
        }
    }

    int next_selected = -1;
    int distance_ptr = 0;
    int deadline_ptr = 0;

    while (index > 0)   //# unrouted customers > 0
    {   
        Route r(data);
        int selected = -1;
        int first_node = -1;
        int flag = 0, index_negtive_first = -1;
        double new_cost = 0.0;

        std::vector<std::pair<int,int>> station_insert_pos;
        station_insert_pos.reserve(MAX_NODE_IN_ROUTE);  // 一条route的充电站备选插入位置

        if (next_selected == -1) // the first time into this loop
        {
            // customer with either the greatest distance from the depot or the earliest deadline.
            bool isDistance = (rand(0.0, 1.0, data.rng) <= 0.5);
            const auto& target_list = isDistance ? data.greatest_distance_customers : data.earliest_deadline_customers;
            int& ptr = isDistance ? distance_ptr : deadline_ptr;

            while (selected == -1 && ptr < (int)target_list.size()) {
                int candidate_node = target_list[ptr];
                if (unrouted_flag[candidate_node]){
                    for (int i = 0; i < index; i++)
                    {
                        if (std::get<0>(unrouted[i]) == candidate_node)
                        {
                            selected = i;
                            break;
                        }
                    }
                    first_node = candidate_node;
                }
                ptr++;
            }
            ptr = std::min(ptr, (int)target_list.size() - 1);
        }
        else  // not the first time into this loop
        {
            selected = next_selected;
            first_node = std::get<0>(unrouted[selected]);                
        }

        r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
        r.temp_node_list.insert(r.temp_node_list.begin() + 1, first_node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入


        flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
        update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量

        station_insert_pos.clear();  //need to be optimized, not eligant
        if (flag == 0 || flag == 2 || flag == 3) {  // "flag ==3 -> no solution" holds in the Euclidean plane
            printf("%d, ", flag);
            printf("No solution.\n");
            return false;
            //exit(0);
        } // 第一个点就出现时间窗不满足，容量不满足，直接退出
        // printf("index_negtive_first: %d\n", index_negtive_first);
        if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s)) {
            printf("%d, ", flag);
            printf("No solution.\n");
            return false;
            // exit(0);            
        }
        // 电量不足，尝试插入充电站
        // 经过上面的操作，r.temp_node_list 已经插入了第一个customer，且满足容量、时间窗、电量约束

        // printf("Success\n");
        // exit(0);

        maintain_unrouted(selected, first_node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
        unrouted_flag[first_node] = false;  // 标记已经插入的 customer
        r.node_list.insert(r.node_list.begin() + 1, first_node);    // 将 node 真的插入到 route 中

        for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
            r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
            // s.idle[station_insert_pos[i].first] = false;
        } // 将缓存的充电站插入信息 应用到 route 中


        // to be optimized



        s.append(r);  // 将当前 route 加入到 solution 中
        
        int len = s.len();
        std::vector<std::tuple<int, int, int, double>> cost_list; 
        // a cost list that outlines all possible insertion points for each unvisited customer, along with the associated costs
        // first dim: customer index in unrouted, second dim: route index, third dim: position, fourth dim: cost

        while (index > 0)   //# unrouted customers > 0
        {
            cost_list.clear();
            for (int i = 0; i < index; i++){
                int node = std::get<0>(unrouted[i]);
                for (int j = len-1; j < len; j++){
                    Route &r = s.get(j);
                    int r_len = int(r.node_list.size());                     
                    r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
                    for (int pos = 1; pos < r_len; pos++){
                        // r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
                        // r.temp_node_list.insert(r.temp_node_list.begin() + pos, node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入
                        // flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
                        // update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量
                        // station_insert_pos.clear(); 
                        // if (flag == 1 || (flag == 4 && sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s))) {
                        //     cost_list.push_back(std::make_tuple(i, j, pos, criterion_customer(r, data, node, pos, adjMatrix)));
                        // }

                        flag = 0, new_cost = 0.0;
                        chk_nl_node_pos_O_n(r.temp_node_list, node, pos, data, flag, new_cost);
                        if (flag == 1 || flag == 4){
                            cost_list.push_back(std::make_tuple(i, j, pos, criterion_customer(r, data, node, pos, adjMatrix)));
                        }
                        // else if (flag == 4) {

                        //     r.temp_node_list.insert(r.temp_node_list.begin() + pos, node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入
                        //     flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
                        //     update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量
                        //     station_insert_pos.clear(); 
                        //     if (sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s)){
                        //         cost_list.push_back(std::make_tuple(i, j, pos, criterion_customer(r, data, node, pos, adjMatrix)));
                        //     }

                        // }

                    }
                }
            }

            if (cost_list.size() == 0) {
                next_selected = -1;
                break;
            }

            if (cost_list.size() > data.l_size){
                std::sort(cost_list.begin(), cost_list.end(), [](const std::tuple<int, int, int, double>& a, const std::tuple<int, int, int, double>& b){
                    return std::get<3>(a) > std::get<3>(b);
                });              
            }

            std::vector<std::tuple<int, int, int, double>> cost_list_r(cost_list.begin(), cost_list.begin() + std::min(data.l_size, (int)cost_list.size()));
            
            double total_q = 0.0;
            for (const auto& element : cost_list_r){ 
                total_q += std::get<3>(element);
            }

            // to be optimized:
            // use cost_list_r up 
            while (cost_list_r.size() > 0){

                size_t tuple_index = select_tuple_index_from_Lr<std::tuple<int, int, int, double>, 3>(cost_list_r, total_q, data);
                auto selected_tuple = cost_list_r[tuple_index];
                // auto selected_tuple = select_tuple_from_Lr<std::tuple<int, int, int, double>, 3>(cost_list_r, total_q, data);

                int i = std::get<0>(selected_tuple);
                int node = std::get<0>(unrouted[i]);
                int j = std::get<1>(selected_tuple);
                Route &r = s.get(j);
                int pos = std::get<2>(selected_tuple);

                r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
                r.temp_node_list.insert(r.temp_node_list.begin() + pos, node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入
                flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
                update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量

                station_insert_pos.clear(); 

                if (flag == 1 || (flag == 4 && sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s))) {
                    // 经过上面的操作，r.temp_node_list 可以插入node，且满足容量、时间窗、电量约束
                    maintain_unrouted(i, node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
                    unrouted_flag[node] = false;
                    r.node_list.insert(r.node_list.begin() + pos, node);    // 将 node 真的插入到 route 中

                    for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
                        r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
                        // s.idle[station_insert_pos[i].first] = false;
                    } // 将缓存的充电站插入信息 应用到 route 中
                    break;
                }
                else{
                    // to be optimized
                    size_t index_to_remove = tuple_index; // 需要删除的元素索引

                    total_q -= std::get<3>(selected_tuple);

                    if (index_to_remove < cost_list_r.size()) {
                        // 交换待删除元素与最后一个元素
                        std::swap(cost_list_r[index_to_remove], cost_list_r.back());
                        // 删除最后一个元素
                        cost_list_r.pop_back();
                    }
                    // next_selected = i;
                    // break;

                }

            }

            if (cost_list_r.size() == 0) {
                next_selected = -1;
                break;
            }

            // if (next_selected != -1) break;


            // if (flag == 0 || flag == 2 || flag == 3) {  
            //     next_selected = i;
            //     break;
            // } 

            // if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s)) {
            //     next_selected = i;
            //     break;
            // }  

            // // 经过上面的操作，r.temp_node_list 可以插入node，且满足容量、时间窗、电量约束

            // maintain_unrouted(i, node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
            // unrouted_flag[node] = false;
            // r.node_list.insert(r.node_list.begin() + pos, node);    // 将 node 真的插入到 route 中

            // for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
            //     r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
            //     s.idle[station_insert_pos[i].first] = false;
            // } // 将缓存的充电站插入信息 应用到 route 中
        
        }

        if (true){

            Route& r = s.get(len - 1);
            int j, k, check = 0;
            double previous_cost = 0.0;
            double heuristic_cost = 0.0;
            index_negtive_first = -1;
            update_route_status(r.node_list, r.status_list, data, flag, previous_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量
            std::vector<int> n_l = r.node_list;
            int node1, node2, node3;
            for (j = 0; j < r.node_list.size()-2; j++){
                node1 = r.node_list[j];
                node2 = r.node_list[j+1];
                node3 = r.node_list[j+2];
                if (data.node[node2].type == 2){  
                    if (data.node[node1].type ==2 || data.node[node3].type ==2 ){  
                        for (k=0; k<data.station_range;k++){
                            if (((data.optimal_staion[node1][node3][k] - data.customer_num) % data.station_cardinality) == ((node2 - data.customer_num) % data.station_cardinality)) break;
                            // if (s.idle[data.optimal_staion[node1][node3][k]] == false) continue;
                            n_l[j+1] = data.optimal_staion[node1][node3][k];
                            flag = 0;
                            new_cost = 0.0;
                            index_negtive_first = -1;
                            update_route_status(n_l,r.status_list,data,flag,new_cost,index_negtive_first);
                            // if (flag != 1) continue;
                            if (flag == 0|| flag==2 || flag == 3) break;
                            if (flag == 4) { 
                                double heuristic_cost=double(INFINITY);
                                std::vector<std::pair<int,int>> station_insert_pos;
                                station_insert_pos.clear();       
                                r.temp_node_list = n_l;  

                                // s.idle[node2] = true;
                                // s.idle[data.optimal_staion[node1][node3][k]] = false;

                                if (sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, heuristic_cost, s)){
                                        for (int index=0; index<station_insert_pos.size(); index++){
                                            n_l.insert(n_l.begin()+ station_insert_pos[index].second, station_insert_pos[index].first); 
                                            // s.idle[station_insert_pos[index].first] = false;
                                        }  
                                        new_cost = heuristic_cost;
                                                    
                                }
                                else {
                                    // s.idle[node2] = false;
                                    // s.idle[data.optimal_staion[node1][node3][k]] = true;
                                    break;
                                } 
                            }      
                            if (new_cost-previous_cost<-PRECISION)  {
                                //printf("%d, %d -> %d, %d: %.2lf\n",first_node,node2,data.optimal_staion[first_node][node3][k],node3, new_cost-pre_cost);
                                heuristic_cost = new_cost;
                                // s.idle[node2] = true;
                                // s.idle[data.optimal_staion[node1][node3][k]] = false;
                                r.node_list = n_l;
                                check = 1;
                                break;
                            }    
                            else{
                                n_l = r.node_list;
                            }  

                        }
                        if (check == 1){
                            break;
                        }
                    }
                }
            }
        }




        
        // /*** redundancy removal ***/
        // std::vector<int> redundant_station;
        // Route &r_tmp = s.get(len - 1);
        // std::vector<int> nl = r_tmp.node_list;
        // for (int k = 1; k < nl.size() - 1; k++) {
        //     if (data.node[nl[k]].type == 2) {
        //         redundant_station.push_back(nl[k]);
        //         s.idle[nl[k]] = true;
        //     }
        // }

        // nl.erase(std::remove_if(nl.begin(), nl.end(), 
        // [&data](int node) { return data.node[node].type == 2; }), nl.end());

        // r.node_list = nl;
        // r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
        // flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
        // update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量

        // station_insert_pos.clear(); 

        // if (flag == 1 || (flag == 4 && sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s))){

        //         for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
        //             r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
        //             s.idle[station_insert_pos[i].first] = false;
        //         } // 将缓存的充电站插入信息 应用到 route 中

        //         s.get(len - 1) = r;


        // }
        // else{
        //     for (auto node : redundant_station) {
        //         s.idle[node] = false;
        //     }          
        // }

    }

    s.update(data);
    s.cal_cost(data);
    // s.output(data);

    return true;
}
//*********************************************************************************************