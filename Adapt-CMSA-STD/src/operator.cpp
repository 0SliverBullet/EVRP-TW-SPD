#include "operator.h"
#include <cstdio>
#include <cstdlib>

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


void direct_routes(Solution &s, Data &data, int initial_node)  
{
    // generate a set of direct routes, each route contains only one customer
    // TODO: should be fixed

    double unrouted_d = data.all_delivery;
    double unrouted_p = data.all_pickup;

    int num_cus = data.customer_num;
    // find all unrouted customers
    // index points to the last position
    std::vector<int> record(num_cus + 1, 0);
    find_unrouted(s, record);

    // first dim: node, second dim: position
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
        double partial_cost = 0.0, new_cost = 0.0;

        std::vector<std::pair<int,int>> station_insert_pos;
        station_insert_pos.reserve(MAX_NODE_IN_ROUTE);
        if (index == data.customer_num) // the first time into this loop
        {
            first_node = initial_node;
            for (int i = 0; i < index; i++)
            {
                if (std::get<0>(unrouted[i]) == first_node)
                {
                    selected = i;
                    break;
                }
            }
        }
        else  // not the first time into this loop
        {
            selected = randint(0, index-1, data.rng);
            first_node = std::get<0>(unrouted[selected]);
        }

        r.temp_node_list = r.node_list;
        r.temp_node_list.insert(r.temp_node_list.begin() + 1, first_node); 
        flag = 0, new_cost = 0.0, index_negtive_first = -1;
        update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  
        station_insert_pos.clear();  //need to be optimized, not eligant
        if (flag == 0 || flag == 2 || flag == 3) {  // "flag ==3 -> no solution" holds in the Euclidean plane
            printf("No solution.");
            exit(0);
        }
        if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost)) continue;
        maintain_unrouted(selected, first_node, index, unrouted, unrouted_d, unrouted_p, data);
        r.node_list.insert(r.node_list.begin() + 1, first_node);  
        for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
            r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
        }

        partial_cost = new_cost;

        s.append(r);
        s.cost += partial_cost;
    }
}


void mergeRoute(){

}

std::tuple<int, int, int, double> select_turple_from_Lr(const std::vector<std::tuple<int, int, int, double>>& Lr, double total_q) {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double rand_prob = dist(rng) * total_q;  

    double cumulative_prob = 0.0;
    for (const auto& element : Lr) {
        cumulative_prob += std::get<3>(element);
        if (cumulative_prob >= rand_prob) {
            return element;
        }
    }
    return Lr.back();
}

std::pair<int, int> select_pair_from_Lr(const std::vector<std::pair<std::pair<int, int>, double>>& Lr, double total_qi_j) {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double rand_prob = dist(rng) * total_qi_j;  

    double cumulative_prob = 0.0;
    for (const auto& element : Lr) {
        cumulative_prob += element.second;
        if (cumulative_prob >= rand_prob) {
            return element.first;
        }
    }
    return Lr.back().first;
}

void add_element_to_L(std::vector<int>& nl1, std::vector<int>& nl2, double alpha_bsf, 
                       std::vector<std::vector<int>>& adjMatrix, Data& data, 
                       std::vector<std::pair<std::pair<int, int>, double>>& L) {
    int len1 = nl1.size(), len2 = nl2.size();
    
    auto create_element = [&](int a, int b) {
        return std::make_pair(std::make_pair(a, b), cal_additional_savings_value(a, b, alpha_bsf, adjMatrix, data));
    };

    L.push_back(create_element(nl1[1], nl2[1]));
    L.push_back(create_element(nl1[1], nl2[len2 - 2]));
    L.push_back(create_element(nl1[len1 - 2], nl2[1]));
    L.push_back(create_element(nl1[len1 - 2], nl2[len2 - 2]));
}


bool compare_elements(const std::pair<std::pair<int, int>, double>& a,
                      const std::pair<std::pair<int, int>, double>& b) {
    return a.second >= b.second;  
}

void ProbabilisticClarkWrightSavings(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data) {

    int initial_node = randint(1, data.customer_num, data.rng);
    direct_routes(s, data, initial_node);  // generate a set of direct routes, each route contains only one customer

    std::vector<std::pair<std::pair<int, int>, double>> L;

    int len = s.len();

    for (int i = 0; i < len; i++) {
        Route &r1 = s.get(i);
        for (int j = 0; j < len; j++) {
            if (i != j) {
                Route &r2 = s.get(j);
                add_element_to_L(r1.node_list, r2.node_list, data.alpha_bsf, adjMatrix, data, L);
            }
        }
    }

    std::sort(L.begin(), L.end(), compare_elements);

    std::vector<std::pair<std::pair<int, int>, double>> Lr(L.begin(), L.begin() + std::min(data.l_size, (int)L.size()));

    double total_qi_j = 0.0;
    for (const auto& element : Lr) {
        total_qi_j += element.second;
    }

    std::pair<int, int> selected_pair = select_pair_from_Lr(Lr, total_qi_j);

    mergeRoute();

}

void ProbabilisticInsertion(Solution &s, std::vector<std::vector<int>>& adjMatrix, Data &data){

    // customer with either the greatest distance from the depot or the earliest deadline.
    int initial_node = 0;
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r1 = dist(rng);
    bool isDistance = (r1 <= 0.5);
    if (isDistance) {
        int selected = randint(0, data.greatest_distance_customers.size() - 1, data.rng);
        initial_node = data.greatest_distance_customers[selected];
    }
    else {
        int selected = randint(0, data.earliest_deadline_customers.size() - 1, data.rng);
        initial_node = data.earliest_deadline_customers[selected];
    }
    
    

    std::vector<double> score(MAX_POINT);
    std::vector<int> score_argrank(MAX_POINT);
    std::vector<int> unrouted_index(MAX_POINT);
    std::vector<int> feasible_pos(MAX_NODE_IN_ROUTE * MAX_POINT, 0);

    double unrouted_d = data.all_delivery;
    double unrouted_p = data.all_pickup;

    int num_cus = data.customer_num;
    std::vector<int> record(num_cus + 1, 0);
    find_unrouted(s, record);
    std::vector<std::tuple<int, int>> unrouted(data.customer_num);
    std::vector<int> station_pos(data.station_num);

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

    while (index > 0)   //# unrouted customers > 0
    {   
        Route r(data);
        int selected = -1;
        int first_node = -1;
        int flag = 0, index_negtive_first = -1;
        double partial_cost = 0.0, new_cost = 0.0;

        std::vector<std::pair<int,int>> station_insert_pos;
        station_insert_pos.reserve(MAX_NODE_IN_ROUTE);  // 一条route的充电站备选插入位置

        if (index == data.customer_num) // the first time into this loop
        {
            first_node = initial_node;
            for (int i = 0; i < index; i++)
            {
                if (std::get<0>(unrouted[i]) == first_node)
                {
                    selected = i;
                    break;
                }
            }
        }
        else  // not the first time into this loop
        {
            // includes only the respective customer.
            selected = next_selected;
            first_node = std::get<0>(unrouted[selected]);
        }

        r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
        r.temp_node_list.insert(r.temp_node_list.begin() + 1, first_node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入
        flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
        update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量

        station_insert_pos.clear();  //need to be optimized, not eligant
        if (flag == 0 || flag == 2 || flag == 3) {  // "flag ==3 -> no solution" holds in the Euclidean plane
            printf("No solution.");
            exit(0);
        } // 第一个点就出现时间窗不满足，容量不满足，直接退出

        if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost)) continue;  // 电量不足，尝试插入充电站
        // 经过上面的操作，r.temp_node_list 已经插入了第一个customer，且满足容量、时间窗、电量约束

        maintain_unrouted(selected, first_node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
        r.node_list.insert(r.node_list.begin() + 1, first_node);    // 将 node 真的插入到 route 中

        for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
            r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
        } // 将缓存的充电站插入信息 应用到 route 中


        s.append(r);  // 将当前 route 加入到 solution 中
        
        int len = s.len();
        
        while (index > 0)   //# unrouted customers > 0
        {
            std::vector<std::tuple<int, int, int, double>> cost_list;
            for (int i = 0; i < index; i++){
                int node = std::get<0>(unrouted[i]);
                for (int j = 0; j < len; j++){
                    Route &route = s.get(j);
                    int r_len = int(r.node_list.size());
                    for (int pos = 1; pos < r_len; pos++){
                        cost_list.push_back(std::make_tuple(i, j, pos, criterion_customer(route, data, node, pos, adjMatrix)));
                    }
                }
            }

            std::sort(cost_list.begin(), cost_list.end(), [](const std::tuple<int, int, int, double>& a, const std::tuple<int, int, int, double>& b){
                return std::get<3>(a) > std::get<3>(b);
            });

            std::vector<std::tuple<int, int, int, double>> cost_list_r(cost_list.begin(), cost_list.begin() + std::min(data.l_size, (int)cost_list.size()));
            
            double total_q = 0.0;
            for (const auto& element : cost_list_r){ 
                total_q += std::get<3>(element);
            }
            std::tuple<int, int, int, double> selected_tuple = select_turple_from_Lr(cost_list_r, total_q);
            int i = std::get<0>(selected_tuple);
            int j = std::get<1>(selected_tuple);
            int pos = std::get<2>(selected_tuple);
            Route &r = s.get(j);
            int node = std::get<0>(unrouted[i]);

            r.temp_node_list = r.node_list;  // 当前 route 的 node list 备份
            r.temp_node_list.insert(r.temp_node_list.begin() + pos, node);  // 尝试在备份插入第一个customer, 从第二个位置开始插入
            flag = 0, new_cost = 0.0, index_negtive_first = -1;  // flag: 0->infeasible, 1->feasible, 2->capacity infeasible, 3->time window infeasible, 4->electricity infeasible
            update_route_status(r.temp_node_list, r.status_list, data, flag, new_cost, index_negtive_first);  // 更新 route 的 status list, 计算到达和离开每个点的时间、电量

            station_insert_pos.clear(); 
            if (flag == 0 || flag == 2 || flag == 3) {  
                next_selected = i;
                break;
            } // 出现时间窗不满足，容量不满足，直接退出

            if (flag == 4 && !sequential_station_insertion(flag, index_negtive_first, r, data, station_insert_pos, new_cost)) {
                next_selected = i;
                break;
            }  
            // 电量不足，尝试插入充电站

            // 经过上面的操作，r.temp_node_list 可以插入node，且满足容量、时间窗、电量约束

            maintain_unrouted(i, node, index, unrouted, unrouted_d, unrouted_p, data);  // 从 unrouted 中移除即将插入的 customer
            r.node_list.insert(r.node_list.begin() + pos, node);    // 将 node 真的插入到 route 中

            for (int i = 0; i < station_insert_pos.size(); i++){  // the same insertion order
                r.node_list.insert(r.node_list.begin()+ station_insert_pos[i].second, station_insert_pos[i].first); 
            } // 将缓存的充电站插入信息 应用到 route 中
        
        }
      


    }

    s.update(data);
    s.cal_cost(data);
    // s.output(data);
}