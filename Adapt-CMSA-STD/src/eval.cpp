#include "eval.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

void chk_nl_node_pos_O_n(std::vector<int> &nl, int inserted_node, int pos, Data &data, int &flag, double &cost)
{   // suppose insert into the pos
    // also check battery constrain
    int len = int(nl.size());
    double capacity = data.vehicle.capacity;
    double distance = 0.0;
    double time = data.start_time;
    double load = 0.0;
    for (auto node : nl)
    {
        load += data.node[node].delivery;
    }
    load += data.node[inserted_node].delivery;

    if (load > capacity)
    {
        flag = 2;
        return;
    }

    int pre_node = nl[0];
    double arr_remain_dist = data.max_distance_reachable;
    double dep_remain_dist = data.max_distance_reachable;

    bool checked = false;
    for (int i = 1; i < len; i++)
    {    
        //printf("%d\n", i);
        int node = nl[i];
        if (i == pos && !checked)
        {
            node = inserted_node;
            i--;
            checked = true;  //insert once
        }

        load = load - data.node[node].delivery + data.node[node].pickup;
        if (load > capacity)
        {
            flag = 2;
            return;
        }
        time += data.time[pre_node][node];

        if (data.node[node].type != 2){   //customer or depot

        if (time > data.node[node].end)
        {
            flag = 3;
            return;
        }
        time = std::max(time, data.node[node].start) + data.node[node].s_time;
        arr_remain_dist = dep_remain_dist - data.dist[pre_node][node];
        if (arr_remain_dist < -PRECISION) {
            flag = 4;
            return;
        }
        dep_remain_dist = arr_remain_dist;
        }
        else{
        arr_remain_dist = dep_remain_dist - data.dist[pre_node][node];
        if (arr_remain_dist < -PRECISION) {
            flag = 4;
            return;
        }
        double f_f0_dist=0;
        int j = i;
        bool chk=checked;
        bool if_inserted = false;
        do{
            j++;
            if (j == pos && chk == false) {
               f_f0_dist += data.dist[nl[j-1]][inserted_node];
               j--;
               chk = true;
               if_inserted = true;
            }
            else if (j == pos && chk == true){
               f_f0_dist += data.dist[inserted_node][nl[j]];
               if_inserted = false;
            }
            else{
                f_f0_dist += data.dist[nl[j-1]][nl[j]];
                if_inserted = false;
            }
        } while ((if_inserted == true && data.node[inserted_node].type == 1)||(if_inserted == false && data.node[nl[j]].type == 1)); 
        
        dep_remain_dist = std::max(f_f0_dist, arr_remain_dist);  

        dep_remain_dist = std::min(dep_remain_dist,data.max_distance_reachable);
   
        double max_recharge_time = (dep_remain_dist - arr_remain_dist) * data.vehicle.consumption_rate * data.vehicle.recharging_rate;
        
        double min_remain_time = double(INFINITY);
        double move_time = time + max_recharge_time;
        j = i;
        chk=checked;
        if_inserted = false;
        do{
            j++;
            if (j == pos && chk == false) {
                move_time += data.time[nl[j-1]][inserted_node];

                if (move_time - data.node[inserted_node].start < -PRECISION){
                        double additional_charge_time = std::min(min_remain_time, data.node[inserted_node].start - move_time);
                        max_recharge_time += additional_charge_time;
                        move_time += additional_charge_time;
                        min_remain_time -= additional_charge_time;
                }             
                if (data.node[inserted_node].end - move_time < -PRECISION){
                flag = 3; return;
                }      
                min_remain_time = std::min (min_remain_time, data.node[inserted_node].end-move_time);
                if (min_remain_time == 0) break;
                move_time = std::max(move_time, data.node[inserted_node].start) + data.node[inserted_node].s_time;  
                j--;
                chk = true;
                if_inserted = true;
            }
            else if (j == pos && chk == true){
                move_time += data.time[inserted_node][nl[j]];
                if (move_time - data.node[nl[j]].start < -PRECISION){
                        double additional_charge_time = std::min(min_remain_time, data.node[nl[j]].start - move_time);
                        max_recharge_time += additional_charge_time;
                        move_time += additional_charge_time;
                        min_remain_time -= additional_charge_time;
                }       
                if (data.node[nl[j]].end - move_time < -PRECISION){
                flag = 3; return;
                }            
                min_remain_time = std::min (min_remain_time, data.node[nl[j]].end-move_time);
                if (min_remain_time == 0) break;
                move_time = std::max(move_time, data.node[nl[j]].start) + data.node[nl[j]].s_time;  
                if_inserted = false;
            }
            else{
                move_time += data.time[nl[j-1]][nl[j]];
                if (move_time - data.node[nl[j]].start < -PRECISION){
                        double additional_charge_time = std::min(min_remain_time, data.node[nl[j]].start - move_time);
                        max_recharge_time += additional_charge_time;
                        move_time += additional_charge_time;
                        min_remain_time -= additional_charge_time;
                }            
                if (data.node[nl[j]].end - move_time < -PRECISION){
                    flag = 3; return;
                }       
                min_remain_time = std::min (min_remain_time, data.node[nl[j]].end-move_time);
                if (min_remain_time == 0) break;  //扫描结束时 min_remain_time 可能 > 0
                move_time = std::max(move_time, data.node[nl[j]].start) + data.node[nl[j]].s_time;  
                if_inserted = false;
            }         
        } 
        while ((if_inserted == true && data.node[inserted_node].type == 1)||(if_inserted == false && data.node[nl[j]].type == 1)); 

        dep_remain_dist = std::min(max_recharge_time / data.vehicle.recharging_rate / data.vehicle.consumption_rate + arr_remain_dist, data.max_distance_reachable);

        time += (dep_remain_dist - arr_remain_dist) * data.vehicle.consumption_rate * data.vehicle.recharging_rate;
        }

        distance += data.dist[pre_node][node];
        pre_node = node;
    }

    flag = 1;
    cost = data.vehicle.d_cost + distance * data.vehicle.unit_cost;
}

void update_route_status(std::vector<int> &nl, std::vector<status> &sl, Data &data, int &flag, double &cost, int &index_negtive_first)
{
    /*
    flag == 0 route error
    flag == 1 feasible
    flag == 2 capacity violation
    flag == 3 capacity Ok, but time window violation
    flag == 4 capacity & time window Ok, but battery violation only
    */
    int len = int(nl.size());

    // start and end at DC
    if (nl[0] != data.DC || nl[len-1] != data.DC) {flag = 0; return;}

    if (len == 2)
    {
        flag = 1;
        cost = 0.0;
        return;
    }

    double capacity = data.vehicle.capacity;
    double distance = 0.0;
    double time = data.start_time;
    double load = 0.0;
    for (auto node : nl) {load += data.node[node].delivery;}
    if (load > capacity) {flag = 2; return;}

    int pre_node = nl[0];
    for (int i = 1; i < len; i++)
    {
        int node = nl[i];
        load = load - data.node[node].delivery + data.node[node].pickup;
        if (load > capacity) {flag = 2; return;}   //capacity constrain violation
        time += data.time[pre_node][node]; 
        sl[i].arr_time = time;

        if (data.node[node].type != 2){  //customer or depot
        if (time > data.node[node].end) {flag = 3; return;}  //time window violation
        time = std::max(time, data.node[node].start) + data.node[node].s_time;
        sl[i].dep_time = time;

        sl[i].arr_RD = sl[i-1].dep_RD - data.dist[pre_node][node];   
        if (sl[i].arr_RD < -PRECISION) {flag = 4; index_negtive_first = i; return;} 
        sl[i].dep_RD = sl[i].arr_RD;          
        }

        else{  
        sl[i].arr_RD = sl[i-1].dep_RD - data.dist[pre_node][node];   
        if (sl[i].arr_RD < -PRECISION) {flag = 4; index_negtive_first = i; return;}

        double f_f0_dist=0;
        int j = i;
        do{
            j++;
            f_f0_dist += data.dist[nl[j-1]][nl[j]];
        } while (data.node[nl[j]].type == 1);
   
        sl[i].dep_RD = std::max(f_f0_dist,sl[i].arr_RD);  
        
        sl[i].dep_RD = std::min(sl[i].dep_RD,data.max_distance_reachable); 

        double max_recharge_time = (sl[i].dep_RD - sl[i].arr_RD) * data.vehicle.consumption_rate * data.vehicle.recharging_rate;

        double min_remain_time = double(INFINITY);
        double move_time = sl[i].arr_time + max_recharge_time; 
        j = i;
        do{
            j++;
            move_time += data.time[nl[j-1]][nl[j]];
            if (move_time - data.node[nl[j]].start < -PRECISION){
                    double additional_charge_time = std::min(min_remain_time, data.node[nl[j]].start - move_time);
                    max_recharge_time += additional_charge_time;
                    move_time += additional_charge_time;
                    min_remain_time -= additional_charge_time;
            }
            if (data.node[nl[j]].end - move_time < -PRECISION){
                flag = 3; return;
            }             
            min_remain_time = std::min (min_remain_time, data.node[nl[j]].end-move_time);
            if (min_remain_time == 0 ) break;
            move_time = std::max(move_time, data.node[nl[j]].start) + data.node[nl[j]].s_time;
        } while (data.node[nl[j]].type == 1);
        
        sl[i].dep_RD = std::min(max_recharge_time / data.vehicle.recharging_rate / data.vehicle.consumption_rate + sl[i].arr_RD, data.max_distance_reachable);

        time += (sl[i].dep_RD - sl[i].arr_RD) * data.vehicle.consumption_rate * data.vehicle.recharging_rate;

        sl[i].dep_time = time;
        }
        
        distance += data.dist[pre_node][node];  
        pre_node = node;
    }

    flag = 1;
    cost = data.vehicle.d_cost + distance * data.vehicle.unit_cost;

}


bool sequential_station_insertion(int &flag, int &index_negtive_first, Route &r, Data &data, std::vector<std::pair<int,int>> &station_insert_pos,double &heuristic_cost, Solution &s){
    double new_cost = 0.0;
    int index_last_f0 = index_negtive_first - 1;
    std::vector<int>& nl = r.temp_node_list;

    do {
        // 枚举插入位点
        index_last_f0 = std::max(index_last_f0 - 1, 0); ;
        std::vector<int> station_pos(data.station_num);
        int node = -1;
        int pos = index_last_f0 + 1;

        int feasible_station_1 = -1;
        int feasible_station_4 = -1;

        for (int j = 0; j <data.station_range; j++){
            if (s.idle[data.optimal_staion[nl[pos-1]][nl[pos]][j]]){
                node = data.optimal_staion[nl[pos-1]][nl[pos]][j];
                if ((data.node[nl[pos-1]].type == 2 && ((nl[pos-1] - data.customer_num) % data.station_cardinality) == ((node -data.customer_num) % data.station_cardinality)) \
                || (data.node[nl[pos]].type == 2 && ((nl[pos] - data.customer_num) % data.station_cardinality) == ((node - data.customer_num) % data.station_cardinality)) \
                || (pos - 2 >=0 && data.node[nl[pos-1]].type == 2 && data.node[nl[pos-2]].type == 2 && ((nl[pos-2] - data.customer_num) % data.station_cardinality == (node - data.customer_num) % data.station_cardinality) ) \
                || (pos + 1 <= nl.size()-1 && data.node[nl[pos]].type == 2 && data.node[nl[pos+1]].type == 2 && ((nl[pos+1] - data.customer_num) % data.station_cardinality == (node - data.customer_num) % data.station_cardinality)) \
                || (nl[pos-1] == data.DC && data.dist[nl[pos-1]][node] == PRECISION) \
                || (nl[pos] == data.DC && data.dist[node][nl[pos]] < PRECISION)) {
                    node = -1;
                    continue;    //剪枝：删去了(1) -f1-f1-, (2) -f1-f2-f1-, (3) depot 就地插入 的异常情况
                } 
                else{

                    double cost = 0.0;
                    int flag_check = 0;
                    chk_nl_node_pos_O_n(nl, node, pos, data, flag_check, cost);  //检查capacity、time window、battery是否满足约束

                    if (feasible_station_1 == -1 && flag_check == 1){
                        feasible_station_1 = node;
                        break;
                    }
                    
                    if (feasible_station_4 == -1 && flag_check == 4){
                        feasible_station_4 = node;
                    }

                }
                // break;
            }                    
        }

        if (feasible_station_1 != -1 || feasible_station_4 != -1){   
            if (feasible_station_1 != -1){
                node = feasible_station_1;
            }
            else{
                node = feasible_station_4;
            }
            station_insert_pos.push_back({node,pos});
            nl.insert(nl.begin() + pos, node);
            s.idle[node] = false;
            flag = 0, new_cost = 0.0, index_negtive_first = -1;
            update_route_status(nl, r.status_list, data,flag,new_cost,index_negtive_first);
            heuristic_cost = new_cost;
   

            // FILE* file = fopen("output.txt", "a"); // 打开文件，写入模式
            // for (int vertex : r. temp_node_list) {
            //     fprintf(file, "%d ", vertex); // 将数据写入文件
            // }
            // fprintf(file, "\n");

            // fclose(file); // 关闭文件

            if (flag != 4) {
                break;
            }

        }
    } 
    // while (data.node[nl[index_last_f0]].type == 1);
    while (flag == 4 && index_last_f0 > 0);

    for (int i = 0; i < station_insert_pos.size(); i++){  
        int node = station_insert_pos[i].first;
        s.idle[node] = true;
    }   
    // 撤销 idle 标记   
        
    if (flag == 1) {
        return true;
    }
    else{
        heuristic_cost = double(INFINITY); 
        return false;
    }
}


double criterion_station(Route &r, Data &data, int node, int pos)
{
    std::vector<int> &nl = r.temp_node_list;
    double alpha=1.0, beta=1.0;
    // TD
    int pre = nl[pos-1];
    int suc = nl[pos];
    double td = data.dist[pre][node] + data.dist[node][suc] - data.dist[pre][suc];
    
    return td;
    // //RD
    // double rd = r.status_list[pos-1].dep_RD  - data.dist[pre][node];

    // //PD
    // double pd = std::min((data.max_distance_reachable- r.status_list[pos-1].dep_RD + data.dist[pre][node]), (std::max(0.0, data.node[suc].start-(r.status_list[pos-1].dep_time+data.time[pre][node] + data.time[node][suc])))/data.vehicle.recharging_rate / data.vehicle.consumption_rate);

    // //rdpd
    // double rdpd = td + alpha * rd - beta * pd ;
    // //printf("%.4lf, %.4lf, %.4lf\n", td, rd, pd);
    
    // return rdpd;
}



bool sequential_station_insertion(int &flag, int &index_negtive_first, Route &r, Data &data, std::vector<std::pair<int,int>> &station_insert_pos,double &heuristic_cost){
    std::vector<double> score(MAX_STATION_POINT);
    std::vector<int> score_argrank(MAX_STATION_POINT);
    std::vector<int> ties(MAX_STATION_POINT);
    std::vector<int> feasible_pos(MAX_NODE_IN_ROUTE*MAX_POINT, 0);
    int station_pos_num = 0;
    bool station_pos_type = false;  
    //false 表示从station选最好的插入；
    //true 表示pos选最好的station插入；
    double new_cost = 0.0;
    while (flag == 4){
            int index_last_f0 = index_negtive_first;
            do {
              index_last_f0--;
            }while (data.node[r.temp_node_list[index_last_f0]].type == 1);
            // -----------------------------------
            int path_len = index_negtive_first - index_last_f0;
            if (data.station_num <= path_len){
                station_pos_num = data.station_num;
                station_pos_type = false; 
            }
            else{
                station_pos_num = path_len;
                station_pos_type = true;
            }
            std::vector<int> station_pos(station_pos_num);
            // ----------------------------------
            if (cal_score_station(station_pos_type, feasible_pos,station_pos,score,r,data,index_last_f0,index_negtive_first))
            {
            argsort(score, score_argrank, station_pos_num);
            double best_score = score[score_argrank[0]];
            ties[0] = score_argrank[0];

            int selected;
            int i = 1;
            for (; i < station_pos_num; i++)
            {
                if (std::abs(best_score - score[score_argrank[i]]) < -PRECISION)
                    ties[i] = score_argrank[i];
                else
                    break;
            } //consider the same score
            if (i > 1) selected = ties[randint(0, i - 1, data.rng)];
            else selected = ties[0];

            // insert station 弥补电量，同时capacity、time window的约束也满足
            
            int node; 
            int pos;
            if (station_pos_type == false){
                    node = selected + data.customer_num+1;
                    pos = station_pos[selected];                
            }
            else{
                    pos = selected + index_last_f0 + 1;
                    node = station_pos[selected];               
            }
            // printf("充电站 %d, 位置 %d\n",node,pos);
            if (r.temp_node_list[pos-1]==node || r.temp_node_list[pos] == node \
            || (pos - 2 >=0 && r.temp_node_list[pos-2]==node && data.node[r.temp_node_list[pos-1]].type == 2) \
            || (pos + 1 <= r.temp_node_list.size()-1 && r.temp_node_list[pos+1]==node && data.node[r.temp_node_list[pos]].type == 2)) {
                break;    //剪枝：删去了-f1-f1-, -f1-f2-f1-的插入情况
            }
            station_insert_pos.push_back({node,pos});
            r.temp_node_list.insert(r.temp_node_list.begin() + pos, node);
            flag = 0;
            new_cost = 0.0;
            index_negtive_first = -1;
            update_route_status(r.temp_node_list, r.status_list, data,flag,new_cost,index_negtive_first);
            heuristic_cost = new_cost;
            // printf("flag: %d\n",flag);
            // for (int i=0; i<r.temp_node_list.size(); i++){
            //    printf("%d: <%.4lf, %.4lf, %.4lf, %.4lf>, ", r.temp_node_list[i], r.status_list[i].arr_time, r.status_list[i].dep_time, r.status_list[i].arr_RD, r.status_list[i].dep_RD);
            // }
            // printf("\n");
            }
            else{ //没有可行的插入充电站解决，回溯撤销添加customer, 对于first customer不可能出现
            break;
            }
        }
        
        if (flag==1) {
        return true;
        }
        else{
        heuristic_cost = double(INFINITY);
        return false;
        }
}

bool cal_score_station(bool type, std::vector<int> &feasible_pos, std::vector<int> &station_pos, std::vector<double> &score, Route &r,Data &data,int index_last_f0, int index_negtive_first)
{
    if (index_negtive_first == -1) return false;
    int r_len = int(r.temp_node_list.size());
    // filter all infeasible positions
    int count1 = 0, count4 = 0, relax = 1;          
    for (int pos=index_last_f0+1; pos <= index_negtive_first; pos++){
            // for (int i = data.customer_num+1; i <= data.customer_num + data.station_num; i++){
            for (int j = 0; j <data.station_range; j++){
                int i = data.optimal_staion[r.temp_node_list[pos-1]][r.temp_node_list[pos]][j];
                int flag = 0;
                double cost = -1.0;
                //过滤掉那些不能走到充电站的点, 不能从同一个充电站走到自己
                //if (r.status_list[pos-1].dep_RD + PRECISION < data.dist[r.temp_node_list[pos-1]][i] ) {
                if (r.status_list[pos-1].dep_RD - data.dist[r.temp_node_list[pos-1]][i] < -PRECISION \
                    || (pos == 1 && data.dist[r.temp_node_list[pos-1]][i] == 0)\
                    || i == r.temp_node_list[pos-1] \
                    || i == r.temp_node_list[pos] \
                    || (data.node[r.temp_node_list[pos]].type == 0 && data.dist[i][r.temp_node_list[pos]] == 0)) {
                    flag = 0;
                }
                else chk_nl_node_pos_O_n(r.temp_node_list, i, pos, data, flag, cost);  //检查capacity、time window、battery是否满足约束
                if (flag == 1) {
                    feasible_pos[i*MAX_NODE_IN_ROUTE+pos] = 1;
                    count1++;
                }else if (flag ==4){
                    feasible_pos[i*MAX_NODE_IN_ROUTE+pos] = 4;
                    count4++;
                }else {
                    feasible_pos[i*MAX_NODE_IN_ROUTE+pos] = 0;
                }            
          }
    }
    if (count1 + count4 ==0) return false;
    if (count1 == 0) relax = 4;
    // insertion criterion RDPD
    if (type ==false) {
            for (int i = data.customer_num+1; i <= data.customer_num + data.station_num; i++)
            {
                double best_score = double(INFINITY);
                int best_pos = -1;
                for (int pos=index_last_f0+1; pos <= index_negtive_first; pos++)
                {
                    if (feasible_pos[i*MAX_NODE_IN_ROUTE+pos] != relax) continue;
                    //if (feasible_pos[i*MAX_NODE_IN_ROUTE+pos] == 0) continue;
                    double utility = criterion_station(r, data, i, pos);
                    if (utility - best_score < -PRECISION)
                    {
                        best_score = utility;
                        best_pos = pos;
                    }
                }
                station_pos[i-data.customer_num-1] = best_pos;
                score[i-data.customer_num-1] = best_score;
            }        
    }
    else{
            for (int pos=index_last_f0+1; pos <= index_negtive_first; pos++)
            {
                double best_score = double(INFINITY);
                int best_station = -1;
                for (int j = 0; j <data.station_range; j++)
                {
                    int i = data.optimal_staion[r.temp_node_list[pos-1]][r.temp_node_list[pos]][j];
                    if (feasible_pos[i*MAX_NODE_IN_ROUTE+pos]!=relax) continue;
                    //if (feasible_pos[i*MAX_NODE_IN_ROUTE+pos] == 0) continue;
                    double utility = criterion_station(r, data, i, pos);
                    if (utility - best_score < -PRECISION)
                    {
                        best_score = utility;
                        best_station = i;
                    }
                }
                station_pos[pos-index_last_f0-1] = best_station;
                score[pos-index_last_f0-1] = best_score;
            } 
    }

    return true;    

}

double cal_additional_savings_value(int pos1, int pos2, double alpha_bsf, std::vector<std::vector<int>>& adjMatrix, Data &data){
    // savings value
    double sigma = data.dist[0][pos1] + data.dist[0][pos2] - data.lambda * data.dist[pos1][pos2] + data.mu * std::abs(data.dist[0][pos1] - data.dist[0][pos2]);
    // additional savings value
    double q = 0;
    if (adjMatrix[pos1][pos2] == 1){
        q = (sigma + 1) * alpha_bsf;
    }
    else{
        q = (sigma + 1) * (1 - alpha_bsf);
    }
    return q;
}


double criterion_customer(Route &r, Data &data, int node, int pos, std::vector<std::vector<int>>& adjMatrix)
{
    std::vector<int> &nl = r.node_list;
    // TD
    int pre = nl[pos-1];
    int suc = nl[pos];
    double td = data.dist[pre][node] + data.dist[node][suc] - data.dist[pre][suc];
    
    double alpha_bsf = data.alpha_bsf;
    double q = 0;
    if (adjMatrix[pre][node] == 1 && adjMatrix[node][suc] == 1)
    {
        q = (td + 1) * (1 - alpha_bsf) * (1 - alpha_bsf);
    }
    else if (adjMatrix[pre][node] == 0 && adjMatrix[node][suc] == 0)
    {
        q = (td + 1) * alpha_bsf * alpha_bsf;
    }
    else
    {
        q = (td + 1) * alpha_bsf * (1 - alpha_bsf) ;
    }
    // return q;
    return  1.0 / q;
}


