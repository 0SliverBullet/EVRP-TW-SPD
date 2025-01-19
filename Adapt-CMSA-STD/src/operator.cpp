#include "operator.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

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
            s.idle[station_insert_pos[i].first] = false;
        }

        s.append(r);
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
    std::vector<int> redundant_station;
    for (int k = 1; k < nl.size() - 1; k++) {
        if (data.node[nl[k]].type == 2) {
            redundant_station.push_back(nl[k]);
            s.idle[nl[k]] = true;
        }
    }

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
            s.idle[station_insert_pos[k].first] = false;
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

        for (auto node : redundant_station) {
            s.idle[node] = false;
        }
        // printf("(%d, %d, %d, %d, %.2lf) ", i, dir1, j, dir2, std::get<4>(selected_tuple));
        saving_list.erase(selected_tuple);
        // printf("%zu ", saving_list.size());
        saving_list.insert(std::make_tuple(i, dir1, j, dir2, - double(INFINITY)));
        // printf("%zu ", saving_list.size());
        saving_cache[generate_cache_key(i, j, concatenate)] = - double(INFINITY);
    }


}



/********** Probabilistic C&W savings algorithm **********/
void ProbabilisticClarkWrightSavings(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data) {

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
}



/********** Probabilistic Insertion Algorithm **********/
void ProbabilisticInsertion(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data){

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
            exit(0);
        } // 第一个点就出现时间窗不满足，容量不满足，直接退出
        // printf("index_negtive_first: %d\n", index_negtive_first);
        if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s)) {
            printf("%d, ", flag);
            printf("No solution.\n");
            exit(0);            
        }
        // 电量不足，尝试插入充电站
        // 经过上面的操作，r.temp_node_list 已经插入了第一个customer，且满足容量、时间窗、电量约束

        maintain_unrouted(selected, first_node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
        unrouted_flag[first_node] = false;  // 标记已经插入的 customer
        r.node_list.insert(r.node_list.begin() + 1, first_node);    // 将 node 真的插入到 route 中

        for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
            r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
            s.idle[station_insert_pos[i].first] = false;
        } // 将缓存的充电站插入信息 应用到 route 中


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
                    for (int pos = 1; pos < r_len; pos++){
                        r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
                        r.temp_node_list.insert(r.temp_node_list.begin() + pos, node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入
                        flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
                        update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量
                        station_insert_pos.clear(); 

                        if (flag == 1 || (flag == 4 && sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s))) {
                            cost_list.push_back(std::make_tuple(i, j, pos, criterion_customer(r, data, node, pos, adjMatrix)));
                        }
                    }
                }
            }

            if (cost_list.size() == 0) {
                next_selected = -1;
                break;
            }

            std::sort(cost_list.begin(), cost_list.end(), [](const std::tuple<int, int, int, double>& a, const std::tuple<int, int, int, double>& b){
                return std::get<3>(a) > std::get<3>(b);
            });

            std::vector<std::tuple<int, int, int, double>> cost_list_r(cost_list.begin(), cost_list.begin() + std::min(data.l_size, (int)cost_list.size()));
            
            double total_q = 0.0;
            for (const auto& element : cost_list_r){ 
                total_q += std::get<3>(element);
            }

            auto selected_tuple = select_tuple_from_Lr<std::tuple<int, int, int, double>, 3>(cost_list_r, total_q, data);

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
            if (flag == 0 || flag == 2 || flag == 3) {  
                next_selected = i;
                break;
            } 

            if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost, s)) {
                next_selected = i;
                break;
            }  

            // 经过上面的操作，r.temp_node_list 可以插入node，且满足容量、时间窗、电量约束

            maintain_unrouted(i, node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
            unrouted_flag[node] = false;
            r.node_list.insert(r.node_list.begin() + pos, node);    // 将 node 真的插入到 route 中

            for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
                r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
                s.idle[station_insert_pos[i].first] = false;
            } // 将缓存的充电站插入信息 应用到 route 中
        
        }

    }

    s.update(data);
    s.cal_cost(data);
    // s.output(data);
}