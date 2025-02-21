#pragma once
#include <iostream>
#include <cstdio>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "data.h"
/* attribute definition */
struct status
{
    double arr_time;  // arrival time 
    double dep_time;  // departure time
    double arr_RD;    // when arrival, the maximum remaining distance that EV can reach
    double dep_RD;    // when departure, the maximum remaining distance that EV can reach
};
struct Attr
{
    int num_cus;
    double dist;
    int s; // start point
    int e; // end point
    /* time window related */
    double T_D; // minimum duration time
    double T_E; // arriving at [T_E,T_L] could achive minimum duration time
    double T_L;
    /* capacity related*/
    double C_E; // initial capacity demand
    double C_H; // the highest capacity demand
    double C_L; // the final capacity demand
};

void attr_for_one_node(Data &data, int node, Attr &a);
Attr attr_for_one_node(Data &data, int node);

// connect tmp_a and tmp_b, return value
Attr connect(const Attr &tmp_a, const Attr &tmp_b, double dist_ij, double t_ij);

// connect tmp_a and tmp_b, write to merged_attr
void connect(const Attr &tmp_a, const Attr &tmp_b, Attr &merged_attr, double dist_ij, double t_ij);
void connect(Attr &merged_attr, const Attr &tmp_b, double dist_ij, double t_ij);

std::vector<int> make_tmp_nl(Data &data);

bool equal_attr(const Attr &a, const Attr &b);

/* route class definition */
class Route
{
public:
    std::vector<int> node_list;   //id
    std::vector<int> customer_list;   //id
    std::vector<int> temp_node_list;   //id
    std::vector<status> status_list;   //arr_time, dep_time, arr_RD, dep_RD
    double dep_time, ret_time, transcost, total_cost, x, y;
    int node_num, customer_num, cluster;  //the customer number in route
    std::vector<Attr> attr;
    Attr self;
    
    Route() {}

    Route(Data &data)
    {
        this->attr.reserve(MAX_NODE_IN_ROUTE * MAX_NODE_IN_ROUTE);
        this->node_list.reserve(MAX_NODE_IN_ROUTE);
        this->customer_list.reserve(MAX_NODE_IN_ROUTE);
        this->temp_node_list.reserve(MAX_NODE_IN_ROUTE);
        this->status_list.reserve(MAX_NODE_IN_ROUTE);

        this->node_list.push_back(data.DC);
        this->node_list.push_back(data.DC);
        
        this->status_list.resize(MAX_NODE_IN_ROUTE);
        status start{data.start_time, data.start_time, data.max_distance_reachable, data.max_distance_reachable};
        this->status_list[0] = start;

        this->update(data);
        this->total_cost = 0.0;
    }

    // attribute
    Attr &gat(int i, int j)
    {
        int nl_len = int(node_list.size());
        return this->attr[i * nl_len + j];
    }

    void cal_attr(Data &data) //calculate complete attr matrix O(n^2)
    {
        int nl_len = int(node_list.size());
        int end_index = nl_len - 1;
        this->attr.resize(nl_len * nl_len);
        // 1. attribute for each node
        for (int i = 0; i <= end_index; i++)
        {
            attr_for_one_node(data, node_list[i], gat(i,i));
        }
        // 2. attribute for sub sequence with start point i, and end point j
        for (int i = 0; i < end_index; i++)
        {
            for (int j = i + 1; j <= end_index; j++)
            {
                connect(gat(i, j-1), gat(j, j), gat(i, j),\
                        data.dist[node_list[j-1]][node_list[j]],\
                        data.time[node_list[j-1]][node_list[j]]);
            }
        }
        this->self = this->gat(0, end_index);
        // 3. attribute for inverse sub sequence with length 2, need to check TW constraint
        for (int i = end_index - 1; i > 0; i--)
        {
            bool feasible = true;
            for (int j = i - 1; j > 0 && (i - j + 1) <= 2; j--)
            {
                if (!feasible)
                {
                    gat(i, j).num_cus = INFEASIBLE;
                    continue;
                }
                // check TW constraint
                if (gat(i, j+1).T_E + gat(i, j+1).T_D + data.time[node_list[j+1]][node_list[j]] - gat(j, j).T_L > 0)
                {
                    gat(i, j).num_cus = INFEASIBLE;
                    feasible = false;
                }
                else
                {
                    connect(gat(i, j+1), gat(j, j), gat(i, j),\
                            data.dist[node_list[j+1]][node_list[j]],
                            data.time[node_list[j+1]][node_list[j]]);
                }
            }
        }
    }

    // method used when applying move operator
    void update(Data &data)
    {
        // update this route, including attribute (if necessary), dep_time and ret_time
        this->cal_attr(data);
        // set dep_time and ret_time
        this->dep_time = this->self.T_E;
        this->ret_time = this->dep_time + this->self.T_D;
    }

    void set_node_list(const std::vector<int> &nl) 
    { 
        this->node_list = nl; 
    }

    // cost
    double cal_cost(Data &data)
    {
        this->transcost = this->self.dist * data.vehicle.unit_cost;
        double dispatchcost = 0.0;
        if (!this->isempty())
            dispatchcost = data.vehicle.d_cost;
        return this->transcost + dispatchcost;
    }

    // others
    bool isempty()
    {
        return this->self.num_cus == 0;
    }

    std::vector<int> check(Data &data, bool &st_re_DC, bool &smaller_ca, bool &earlier_tw, bool &positive_ba, double &cost)
    {
        /*
        flag == 0 route error
        flag == 1 feasible
        flag == 2 capacity violation
        flag == 3 capacity Ok, but time window violation
        flag == 4 capacity & time window Ok, but battery violation only
        */
        /* time complexity O(n) */
        std::vector<int> nodes;
        std::vector<int> &nl = this->node_list;
        std::vector<status> &sl = this->status_list;
        int len = int(nl.size());

        // start and end at DC
        if (nl[0] != data.DC || nl[len-1] != data.DC) {
            printf("Not starting/ending at DC\n");
            st_re_DC = false;
            return nodes;
        }

        double capacity = data.vehicle.capacity;
        double distance = 0.0;
        double time = data.start_time;
        double load = 0.0;

        for (int i = 1; i < len - 1; i++)
        {
            if (data.node[nl[i]].type == 1) nodes.push_back(nl[i]);
            load += data.node[nl[i]].delivery;
        }

        if (load > capacity)
        {
            smaller_ca = false;
            printf("Whole delivery > cpacity\n");
            return nodes;
        }

        int pre_node = nl[0];
        for (int i = 1; i < len; i++)
        {
            int node = nl[i];
            load = load - data.node[node].delivery + data.node[node].pickup;
            if (load > capacity)
            {
                smaller_ca = false;
                printf("Load %f > capacity %f at %d th node: %d, with delivery %f and pickup %f\n",\
                        load, capacity, i, node, data.node[node].delivery, data.node[node].pickup);
                return nodes;
            }   //capacity constrain violation


            if (data.node[node].type != 2){  //customer or depot        
            time += data.time[pre_node][node]; 
            sl[i].arr_time = time;
            if (time > data.node[node].end)             
            {
                earlier_tw = false;
                printf("Arrive at %d th node: %d at time %f > tw end %f",\
                        i, node, time, data.node[node].end);
                return nodes;
            }  //time constrain violation
            time = std::max(time, data.node[node].start) + data.node[node].s_time;
            sl[i].dep_time = time;

            sl[i].arr_RD = sl[i-1].dep_RD - data.dist[pre_node][node];   
            if (sl[i].arr_RD < -PRECISION) 
            {
                positive_ba = false;
                printf("Arrive at %d th node: %d with remaining distance %f < 0",\
                        i, node, sl[i].arr_RD);
                return nodes;
            }  //battery constrain violation
            sl[i].dep_RD = sl[i].arr_RD;          
            }

            else{  //station
            sl[i].arr_RD = sl[i-1].dep_RD - data.dist[pre_node][node];   
            if (sl[i].arr_RD < -PRECISION) {
                int flag = 4;
                for (int k=1;k<data.station_range;k++) {
                    node = data.optimal_staion[pre_node][nl[i+1]][k];
                    sl[i].arr_RD = sl[i-1].dep_RD - data.dist[pre_node][node];
                    if (sl[i].arr_RD > -PRECISION){
                        nl[i]=node;
                        flag = 1;
                        break;
                    }
                }           
                if (flag == 4) 
                {
                    positive_ba = false;
                    printf("Arrive at %d th node: %d with remaining distance %f < 0",\
                            i, node, sl[i].arr_RD);
                    return nodes;
                }
                }
            //update sl[i].dep_RD:
            double f_f0_dist=0;
            int j = i;
            do{
                j++;
                f_f0_dist += data.dist[nl[j-1]][nl[j]];
            } while (data.node[nl[j]].type == 1);
            sl[i].dep_RD = std::max(f_f0_dist,sl[i].arr_RD); 
            
            if (data.max_distance_reachable - sl[i].dep_RD < -PRECISION) { 
                int flag = 4;
                for (int k=1;k<data.station_range;k++) {
                    node = data.optimal_staion[pre_node][nl[i+1]][k];
                    sl[i].arr_RD = sl[i-1].dep_RD - data.dist[pre_node][node];
                    if (sl[i].arr_RD > -PRECISION){  
                            nl[i]=node;
                            f_f0_dist=0;
                            j = i;
                            do{
                                j++;
                                f_f0_dist += data.dist[nl[j-1]][nl[j]];
                            } while (data.node[nl[j]].type == 1);  
                            sl[i].dep_RD = std::max(f_f0_dist,sl[i].arr_RD);  
                            if (data.max_distance_reachable - sl[i].dep_RD > -PRECISION){
                            flag = 1;
                            break;
                            }
                    }
                }
                if (flag == 4) 
                {
                    positive_ba = false;
                    printf("Leave from %d th node: %d with remaining distance %f > max distance %f",\
                            i, node, sl[i].dep_RD, data.max_distance_reachable);
                    return nodes;
                }
            }
            
            double max_recharge_time = (sl[i].dep_RD - sl[i].arr_RD) * data.vehicle.consumption_rate * data.vehicle.recharging_rate;
            time += data.time[pre_node][node]; 
            sl[i].arr_time = time;
            //--------------------------------------------------------------------------------
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
                        earlier_tw = false;
                        printf("Arrive at %d th node: %d at time %f > tw end %f",\
                                j, nl[j], move_time, data.node[nl[j]].end);
                        return nodes;
                }                
                min_remain_time = std::min (min_remain_time, data.node[nl[j]].end-move_time);
                if (min_remain_time == 0) break;
                move_time = std::max(move_time, data.node[nl[j]].start) + data.node[nl[j]].s_time;
            } while (data.node[nl[j]].type == 1);

            //-------------------------------------------------------------------------------------------------------------------

            sl[i].dep_RD = std::min(max_recharge_time / data.vehicle.recharging_rate / data.vehicle.consumption_rate + sl[i].arr_RD, data.max_distance_reachable);

            time += (sl[i].dep_RD - sl[i].arr_RD) * data.vehicle.consumption_rate * data.vehicle.recharging_rate;

            sl[i].dep_time = time;
            }
            
            distance += data.dist[pre_node][node];  
            pre_node = node;
        }

        cost = data.vehicle.d_cost + distance * data.vehicle.unit_cost;
        return nodes;
    }
};

/* solution class definition*/
class Solution
{
private:
    std::vector<Route> route_list;

public:
    std::vector<bool> idle;
    double cost = 0.0;

    Solution(){}

    Solution(const Data &data)
        : route_list(), idle(data.node_num, true), cost(0.0)
    {
        route_list.reserve(data.vehicle.max_num);
    }
    // Function to merge another Solution into this one
    void merge(const Solution& other)
    {
        // Append routes from other to this
        route_list.insert(route_list.end(), other.route_list.begin(), other.route_list.end());
        cost += other.cost;
    }
    void reserve(Data &data)
    {
        this->route_list.reserve(data.vehicle.max_num);
    }

    void append(Route &r)
    {
        this->route_list.push_back(r);
    }

    void del(int index)
    {
        this->route_list.erase(this->route_list.begin() + index);
    }

    Route &get(int index)
    {
        return this->route_list[index];
    }
    Route &get_tail()
    { 
        return *(route_list.end() - 1);
    }

    int len()
    {
        return int(this->route_list.size());
    }

    void update(Data &data)
    {
        // update all routes, and delete empty routes
        int len = this->len();
        for (int index = 0; index < len;)
        {
            Route &route = this->get(index);
            route.update(data);
            route.total_cost = route.cal_cost(data);
            if (route.isempty())
            {
                // delete empty route
                this->del(index);
                len--;
            }
            else
                index++;
        }
    }

    void local_update(std::vector<int> &route_indice)
    {
        // delete empty routes in route_indice, and maintain route_list
        // note route_indice would be altered to record all routes

        int len = this->len();
        int empty_id = -1; // -1 means no empty route exists
        bool last_id_in = false; // true means last route changed
        for (int &item : route_indice)
        {
            // there are at most 2 route index in the array
            // and at most 1 route is emppty
            if (this->get(item).isempty())
                empty_id = item;
            if (item == len - 1)
                last_id_in = true;
        }
        if (empty_id != -1)
        {
            if (empty_id == len - 1)
            {
                this->route_list.pop_back();
            }
            else
            {
                this->route_list[empty_id] = *(this->route_list.end() - 1);
                this->route_list.pop_back();
                if (!last_id_in)
                    route_indice.push_back(len - 1);
            }
        }
    }

    void clear(Data &data)
    {
        // clear route_list
        this->route_list.clear();
        this->route_list.reserve(data.vehicle.max_num);
        this->cost = 0.0;
    }

    double cal_cost(Data &data)
    {
        this->cost = 0.0;
        for (auto route = this->route_list.begin(); route < this->route_list.end(); route++)
        {
            this->cost += route->cal_cost(data);
        }
        return this->cost;
    }

    std::string build_output_str()
    {
        std::string output_s = "Details of the solution:\n";
        int len = this->len();
        for (int i = 0; i < len; i++)
        {
            std::vector<int> &nl = this->route_list[i].node_list;
            output_s += "route " + std::to_string(i) +
                        ", node_num " + std::to_string(nl.size()) +
                        ", cost " + std::to_string(this->route_list[i].transcost) +
                        ", nodes:";
            for (int node : nl)
            {
                output_s += ' ' + std::to_string(node);
            }
            output_s += '\n';
        }
        output_s += "vehicle (route) number: " + std::to_string(len) + '\n';

        std::ostringstream stream;
        
        stream << std::fixed << std::setprecision(2) << this->cost;
        std::string costStr = stream.str();

        output_s += "Total cost: " + costStr + '\n';
        return output_s;
    }

    void output(Data &data)
    {
        std::string output_s = this->build_output_str();
        if (!data.if_output) std::cout << output_s;
        else
        {
            std::ofstream out(data.output.c_str());
            out << output_s;
        }
    }

    bool check(Data &data)
    {
        double total_cost = 0.0;
        int len = this->len();
        std::unordered_set<int> record;
        record.reserve(data.customer_num);
        for (int i = 0; i < len; i++)
        {
            // printf("Check route %d\n", i);
            Route &r = this->get(i);
            bool st_re_DC = true;
            bool smaller_ca = true;
            bool earlier_tw = true;
            bool positive_ba = true;
            double cost = 0.0;
            std::vector<int>nodes = r.check(data, st_re_DC, smaller_ca, earlier_tw, positive_ba, cost);
            if (!st_re_DC || !smaller_ca || !earlier_tw || !positive_ba)
                return false;
            total_cost += cost;
            for (auto node : nodes)
            {
                if (record.count(node) == 0)
                    record.insert(node);
                else if (record.count(node) == 1)
                {
                    printf("Duplicate node: %d\n", node);
                    return false;
                }
            }
        }
        for (int i = 0; i < data.customer_num + 1; i++)
        {
            if (i == data.DC) continue;
            if (record.count(i) != 1)
            {
                printf("Misssing customer: %d\n", i);
                return false;
            }
        }

        printf("This cost %f, check total cost %f, diff %f\n", this->cost, total_cost, total_cost-this->cost);
        if (std::abs(total_cost-this->cost) > PRECISION) {
            printf("Objective value is wrong!\n");
            return false;
        }
        return true;
    }

    // if we do not record adjMatrix, a List to record c_ij is also OK
    void adjMatrixRepresentation(std::vector<std::vector<int>>& adjMatrix){
        int len = this->len();
        for (int i = 0; i < len; i++)
        {
            std::vector<int> &nl = this->route_list[i].node_list;
            int nl_size = nl.size();
            for (int j = 0; j < nl_size - 1; j++) {
                int from = nl[j];     
                int to = nl[j + 1];   
                adjMatrix[from][to] = 1; 
            }
            // adjMatrix[nl[nl_size-2]][depot] = 1;
        }

    }

    void routeListRepresentation(const std::vector<std::vector<int>>& solMatrix, std::vector<int> &next, Data &data){ 
        int numNodes = solMatrix.size();
        for (int i = 0; i < numNodes; ++i) {
            if (solMatrix[0][i] == 0) continue;
            std::vector<int> nl;
            nl.push_back(0);
            int current = i;
            while (current != numNodes - 1) {
                nl.push_back(current);
                current = next[current];
            }
            nl.push_back(0);
            Route r(data);
            r.node_list = nl;
            this->append(r);
        }

        this->update(data);
        this->cal_cost(data);
        // printf("%.2f\n", this->cost);
    }


    double distance(const Route& p1, const Route& p2) {
            return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }  

    bool centroidsChanged(const std::vector<Route>& centroids, const std::vector<Route>& oldCentroids) {
        const double EPSILON = 1e-4;
        for (size_t i = 0; i < centroids.size(); ++i) {
            if (distance(centroids[i], oldCentroids[i]) > EPSILON) {
                return true;
            }
        }
        return false;
    }

    void balancedCluster(int k, int Max_customer_num_per_subproblem, std::vector<int> &clusterSize, int maxIterations = 1000) {
        int n = this->len();
        int maxClusterSize = Max_customer_num_per_subproblem;
        std::vector<Route> centroids(k);
        std::vector<Route> oldCentroids(k);

        for (int i = 0; i < k; ++i) {
           centroids[i] = this->get(i);
        }

        for (int iter = 0; iter < maxIterations; ++iter) {
            std::vector<int> clusterSizes(k, 0);
            std::vector<int> assignCheck(n, 0);
            std::vector<double> dist(k*n,0);
            std::vector<int> dist_argrank(k*n, 0);
            int cnt = 0;
            for (int i = 0; i < k; ++i) {
                    for (int j = 0; j< n; j++) {
                            dist[cnt] = distance(this->get(j), centroids[i]);
                            cnt ++;
                    }
            }
            argsort(dist, dist_argrank, k*n);
            for (int index: dist_argrank){
                int i = index / n;
                int j = index % n;
                if  (assignCheck[j] == 0 && clusterSizes[i] < maxClusterSize){
                        this->get(j).cluster = i;
                        clusterSizes[i] += this->get(j).customer_num;
                        assignCheck [j] = 1;
                }
            }

            oldCentroids = centroids;
            std::vector<int> counts(k, 0);
            std::vector<double> sumX(k, 0.0), sumY(k, 0.0);
            
            for (int j = 0; j< n; j++) {
                int cluster = this->get(j).cluster;
                sumX[cluster] += this->get(j).x * this->get(j).node_num;
                sumY[cluster] += this->get(j).y * this->get(j).node_num;
                counts[cluster] += this->get(j).node_num;
            }

            for (int i = 0; i < k; ++i) {
                if (clusterSizes[i] > 0) {
                    centroids[i].x = sumX[i] / counts[i];
                    centroids[i].y = sumY[i] / counts[i];
                }
                clusterSize[i] = clusterSizes[i];
            }

            if (!centroidsChanged(centroids, oldCentroids)) {
                break;
            }
        }    
    }
};
