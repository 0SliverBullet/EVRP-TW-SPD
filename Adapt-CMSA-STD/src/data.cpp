#include "data.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
Data::Data(const Data& data, std::vector<int> &subproblem) // main problem to subproblems mapping
{
    double all_pickup = 0.0;
    double all_delivery = 0.0;
    this->problem_name = data.problem_name;
    this->customer_num = subproblem.size();
    this->station_num = data.station_num;
    this->station_range = data.station_range;
    this->node_num = this->customer_num + this->station_num + 1;
    std::vector<double> tmp_v_1(this->node_num, 0.0);
    std::vector<bool> tmp_v_2(this->node_num, 0.0);
    std::vector<int> tmp_v_3(this->node_num, 0);
    for (int i = 0; i < this->node_num; i++)
    {
                this->node.push_back({0, 0, 0.0, 0.0, 0.0, 0.0, 0.0});
                this->dist.push_back(tmp_v_1);
                this->time.push_back(tmp_v_1);
                this->rm.push_back(tmp_v_1);
                this->rm_argrank.push_back(tmp_v_3);
                this->pm.push_back(tmp_v_2);
    }
    this->vehicle.max_num = data.vehicle.max_num;
    this->vehicle.d_cost = data.vehicle.d_cost;
    this->vehicle.unit_cost = data.vehicle.unit_cost;
    this->vehicle.capacity = data.vehicle.capacity;
    this->vehicle.battery = data.vehicle.battery;
    this->vehicle.consumption_rate = data.vehicle.consumption_rate;
    this->max_distance_reachable = data.max_distance_reachable;
    this->vehicle.recharging_rate = data.vehicle.recharging_rate;
    this->node[0].type = 0;
    this->node[0].x = data.node[0].x;
    this->node[0].y = data.node[0].y;
    this->node[0].delivery = data.node[0].delivery;
    all_delivery += this->node[0].delivery;
    this->node[0].pickup = data.node[0].pickup;
    all_pickup += this->node[0].pickup;
    this->node[0].start = data.node[0].start;
    this->node[0].end = data.node[0].end;
    this->node[0].s_time = data.node[0].s_time;

    for (int i = 1; i<= this->customer_num; i++){
        this->node[i].type = 1;
        int matser_to_sub = subproblem[i-1];
        this->node[i].x = data.node[matser_to_sub].x;
        this->node[i].y = data.node[matser_to_sub].y;
        this->node[i].delivery = data.node[matser_to_sub].delivery;
        all_delivery += this->node[i].delivery;
        this->node[i].pickup = data.node[matser_to_sub].pickup;
        all_pickup += this->node[i].pickup;
        this->node[i].start = data.node[matser_to_sub].start;
        this->node[i].end = data.node[matser_to_sub].end;
        this->node[i].s_time = data.node[matser_to_sub].s_time;
    }
    for (int i = this->customer_num+1; i<= this->customer_num + this->station_num; i++){
        this->node[i].type = 2;
        int matser_to_sub = data.customer_num + i - this->customer_num;
        this->node[i].x = data.node[matser_to_sub].x;
        this->node[i].y = data.node[matser_to_sub].y;        
        this->node[i].delivery = data.node[matser_to_sub].delivery;
        all_delivery += this->node[i].delivery;
        this->node[i].pickup = data.node[matser_to_sub].pickup;
        all_pickup += this->node[i].pickup;
        this->node[i].start = data.node[matser_to_sub].start;
        this->node[i].end = data.node[matser_to_sub].end;
        this->node[i].s_time = data.node[matser_to_sub].s_time;        
    }
    for (int i = 0; i<= this->customer_num + this->station_num; i++){
                int i_matser_to_sub = 0; 
                
                if (this->node[i].type == 0){
                }else if (this->node[i].type ==1){
                    i_matser_to_sub = subproblem[i-1];
                }else if (this->node[i].type == 2){
                    i_matser_to_sub = data.customer_num + i - this->customer_num;
                }
        for (int j = 0; j<= this->customer_num + this->station_num; j++){
                int j_matser_to_sub = 0;

                if (this->node[j].type == 0){
                }else if (this->node[j].type ==1){
                    j_matser_to_sub = subproblem[j-1];
                }else if (this->node[j].type == 2){
                    j_matser_to_sub = data.customer_num + j - this->customer_num;
                }
                    double d = data.dist[i_matser_to_sub][j_matser_to_sub];
                    double t = data.time[i_matser_to_sub][j_matser_to_sub];
                    this->dist[i][j] = d;
                    // all_dist += d;
                    this->time[i][j] = t;
                    // all_time += t;
                    // if (d < this->min_dist) this->min_dist = d;
                    // if (d > this->max_dist) this->max_dist = d;

        }
    }
    this->floydWarshall();
    this->DC = data.DC;
    this->start_time = this->node[this->DC].start;
    this->end_time = this->node[this->DC].end;
    this->all_delivery = all_delivery;
    this->all_pickup = all_pickup;

    std::vector<std::vector<int>> tmp_v_4(this->customer_num+this->station_num+1, std::vector<int>(this->station_num, 0));
    for (int i = 0; i <= this->customer_num + this->station_num; i++)
    {       
        this->optimal_staion.push_back(tmp_v_4);
    }
    std::vector<double> c_f_c_dist(this->station_num);
    std::vector<int> c_f_c_dist_argrank(this->station_num); 
    for (int i = 0; i <= this->customer_num; i++)    // c-f-c
    {       
        for (int j=0; j<= this->customer_num;j++){
              if (i!=j) {
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                if (i != this->DC && j!= this->DC){
                    for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                    }                    
                }
                else if (i == this->DC) {
                    if (this->dist[i][c_f_c_dist_argrank[0]+this->customer_num+1]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                }  
                else if (j == this->DC) {
                    if (this->dist[c_f_c_dist_argrank[0]+this->customer_num+1][j]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                } 
              }  
        }
    }    

    for (int i = 0; i <= this->customer_num; i++)    // c-f-f
    {       
        for (int j=this->customer_num+1; j<= this->customer_num+this->station_num;j++){
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        if (k!=j) {
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];
                        }
                        else{
                        c_f_c_dist[k-this->customer_num-1]=double(INFINITY);    
                        }
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                if (i != this->DC){
                    for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                    }                    
                }
                else {
                    if (this->dist[i][c_f_c_dist_argrank[0]+this->customer_num+1]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                }  
        }
    }    

    for (int i = this->customer_num+1; i <= this->customer_num+this->station_num; i++)    // f-f-c
    {       
        for (int j=0; j<= this->customer_num; j++){
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        if (k!=i) {
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];
                        }
                        else{
                        c_f_c_dist[k-this->customer_num-1]=double(INFINITY);    
                        }
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                if (j != this->DC){
                    for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                    }                    
                }
                else {
                    if (this->dist[c_f_c_dist_argrank[0]+this->customer_num+1][j]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                }                      
        }
    } 

    for (int i = this->customer_num+1; i <= this->customer_num+this->station_num; i++)    // f-f-f
    {       
        for (int j = this->customer_num+1; j <= this->customer_num+this->station_num; j++){
              if (i!=j) {
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        if (i!=k && j!=k){
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];    
                        }
                        else{
                        c_f_c_dist[k-this->customer_num-1]=double(INFINITY); 
                        }
                        
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                }                    
      
              }  
        }
    }       
    this->individual_search = data.individual_search;
    this->population_search = data.population_search;
    this->parallel_insertion = data.parallel_insertion;
    this->conservative_local_search = data.conservative_local_search;
    this->aggressive_local_search = data.aggressive_local_search;
    this->subproblem_range = data.subproblem_range;
    this->seed = data.seed;
    this->rng.seed(this->seed);
    // set parameters
    this->pruning = data.pruning;
    this->if_output = data.if_output;
    this->output = data.output;
    this->tmax = data.tmax;
    this->runs = 1;
    this->g_1 = data.g_1;
    this->p_size = data.p_size;
    
    int sr = int(sqrt(double(this->p_size)));
    if (sr == 1)
        this->latin.push_back(std::make_tuple(0.5, 0.5));
    else
    {
        double step = 1.0 / (sr - 1);
        for (int i = 0; i < sr; i++)
        {
            for (int j = 0; j < sr; j++)
            {
                double lambda = std::min(1.0, step * i);
                double gamma = std::min(1.0, step * j);
                this->latin.push_back(std::make_tuple(lambda, gamma));
            }
        }
        std::shuffle(this->latin.begin(), this->latin.end(), this->rng);
    }

    this->init = PERTURB;  //data.init; 
    this->k_init = data.k_init;
    if (this->k_init == K)
        this->k_init = this->customer_num;

    this->cross_repair = data.cross_repair;
    this->k_crossover = data.k_crossover;
    if (this->k_crossover == K)
        this->k_crossover = this->customer_num;
    this->selection = data.selection;
    this->replacement = data.replacement;
    this->ls_prob = data.ls_prob;
    this->skip_finding_lo = data.skip_finding_lo;
    this->O_1_evl = data.O_1_evl;
    this->no_crossover = data.no_crossover;

    if (data.two_opt)
    {
        this->two_opt = true;
        small_opts.push_back("2opt");
        std::vector<Move> tmp_mem(this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("2opt", tmp_mem));
    }
    if (data.two_opt_star)
    {
        this->two_opt_star = true;
        small_opts.push_back("2opt*");
        std::vector<Move> tmp_mem(this->vehicle.max_num * this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("2opt*", tmp_mem));
    }

    if (data.or_opt)
    {
        this->or_opt = true;
        this->or_opt_len = data.or_opt_len;
        small_opts.push_back("oropt_single");
        small_opts.push_back("oropt_double");
        std::vector<Move> tmp_mem_0(this->vehicle.max_num);
        std::vector<Move> tmp_mem_1(this->vehicle.max_num * this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("oropt_single", tmp_mem_0));
        mem.insert(std::pair<std::string, std::vector<Move>>("oropt_double", tmp_mem_1));
    }

    if (data.two_exchange)
    {
        this->two_exchange = true;
        this->exchange_len = data.exchange_len;
        small_opts.push_back("2exchange");
        std::vector<Move> tmp_mem(this->vehicle.max_num * this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("2exchange", tmp_mem));
    }

    this->escape_local_optima = data.escape_local_optima;

    if (data.random_removal)
    {
        this->random_removal = true;
        destroy_opts.push_back("random_removal");
    }

    if (data.related_removal)
    {
        this->related_removal = true;
        this->alpha = data.alpha;
        this->r = this->alpha * (this->all_dist / this->all_time);
        destroy_opts.push_back("related_removal");
    }


        this->destroy_ratio_l = data.destroy_ratio_l;
        this->destroy_ratio_u = data.destroy_ratio_u; 

    if (data.regret_insertion)
    {
        this->regret_insertion = true;
        repair_opts.push_back("regret_insertion");
    }

    if (data.greedy_insertion)
    {
        this->greedy_insertion = true;
        repair_opts.push_back("greedy_insertion");
    }

    if (data.rd_removal_insertion)
    {
        this->rd_removal_insertion = true;
    }

    this->bks = data.bks;

    int n_num = this->node_num;
    for (int i = 0; i < n_num; i++)
    {
        for (int j = 0; j < n_num; j++)
        {this->pm[i][j] = true;}
    }
    this->pre_processing();
}
Data::Data(ArgumentParser &parser)
{
    // read problem file
    const char* pro_file = parser.retrieve<std::string>("problem").c_str();
    char Buffer[N];
    std::ifstream fp;
    fp.rdbuf()->pubsetbuf(Buffer, N);
    fp.open(pro_file);

    std::string line;
    double all_pickup = 0.0;
    double all_delivery = 0.0;
    bool flag = true;
    while (true)
    {
        if (flag)
        {
            if (!std::getline(fp, line)) break;
        }
        else flag = true;
        trim(line);
        if (line.size() == 0) continue;
        std::vector<std::string> results = split(line, ':');
        trim(results[0]);
        if (results.size() > 1) trim(results[1]);
        if (results[0] == "NAME")
        {
            printf("%s\n", line.c_str());
            this->problem_name = results[1];
        }
        else if (results[0] == "TYPE")
        {
            printf("%s\n", line.c_str());
        }
        else if (results[0] == "DIMENSION")
        {
            printf("%s\n", line.c_str());
            this->node_num = stoi(results[1]);
            std::vector<double> tmp_v_1(this->node_num, 0.0);
            std::vector<bool> tmp_v_2(this->node_num, 0.0);
            std::vector<int> tmp_v_3(this->node_num, 0);
            for (int i = 0; i < this->node_num; i++)
            {
                this->node.push_back({0, 0, 0.0, 0.0, 0.0, 0.0, 0.0});
                this->dist.push_back(tmp_v_1);
                this->time.push_back(tmp_v_1);
                this->rm.push_back(tmp_v_1);
                this->rm_argrank.push_back(tmp_v_3);
                this->pm.push_back(tmp_v_2);
            }
        }
        else if (results[0] == "VEHICLES")
        {
            printf("%s\n", line.c_str());
            this->vehicle.max_num = stoi(results[1]) + V_NUM_RELAX;
        }
        else if (results[0] == "DISPATCHINGCOST")
        {
            printf("%s\n", line.c_str());
            this->vehicle.d_cost = stod(results[1]);
        }
        else if (results[0] == "UNITCOST")
        {
            printf("%s\n", line.c_str());
            this->vehicle.unit_cost = stod(results[1]);
        }
        else if (results[0] == "CAPACITY")
        {
            printf("%s\n", line.c_str());
            this->vehicle.capacity = stod(results[1]);
        }
        else if (results[0] == "ELECTRIC_POWER")
        {
            printf("%s\n", line.c_str());
            this->vehicle.battery = stod(results[1]);
        }
        else if (results[0] == "CONSUMPTION_RATE")
        {
            printf("%s\n", line.c_str());
            this->vehicle.consumption_rate = stod(results[1]);
            this->max_distance_reachable= this->vehicle.battery / vehicle.consumption_rate;
        }
        else if (results[0] == "RECHARGING_RATE")
        {
            printf("%s\n", line.c_str());
            this->vehicle.recharging_rate = stod(results[1]);
        }        
        else if (results[0] == "EDGE_WEIGHT_TYPE")
        {
            printf("%s\n", line.c_str());
            if (results[1] != "EXPLICIT")
            {
                printf("Expect edge weight type: EXPLICIT, while accept type: %s\n", results[1].c_str());
                exit(-1);
            }
        }
        else if (results[0] == "NODE_SECTION")
        {
            getline(fp, line);  //ignore the table header
            bool Cartesian_coordinate_system = true;
            trim(line);
            if (line.size() == 0) continue;
            std::vector<std::string> r = split(line, ',');
            if (r.size() > 1){
                trim(r[2]);
                trim(r[3]);
                if (r[2] == "lng" && r[3] == "lat") Cartesian_coordinate_system = false;
            }            
            this->customer_num=0;
            this->station_num=0;
            while (getline(fp, line))
            {
                trim(line);
                if (line.size() == 0) continue;
                std::vector<std::string> r = split(line, ',');
                if (r.size() > 1)
                {
                    trim(r[0]);
                    int i = stoi(r[0]);
                    //this->node[i].id = i;
                    trim(r[1]);
                    if (r[1]=="d"){
                        this->node[i].type=0;
                    }
                    else if (r[1]=="c"){
                        this->node[i].type=1;
                        this->customer_num++;
                    }else if (r[1]=="f"){
                        this->node[i].type=2;
                        this->station_num++;
                    }
                    trim(r[2]);
                    trim(r[3]);
                    if (Cartesian_coordinate_system) {
                        this->node[i].x = stod(r[2]);
                        this->node[i].y = stod(r[3]);
                    }
                    else{
                        /*
                        How to convert longitude and latitude coordinates into plane Cartesian coordinates?
                        We use Mercator Projection here.
                        More details can be seen in
                        [1] https://en.wikipedia.org/wiki/Mercator_projection
                        [2] Osborne, P. (2013). Mercator. Technical report, Edinburgh.(P26, 32)
                        */
                        double lng = stod(r[2]) * M_PI / 180.0;  
                        double lat = stod(r[3]) * M_PI / 180.0;
                        // convert degree system to radian system
                        this->node[i].x = Radius * lng;   
                        this->node[i].y = Radius * std::log(std::tan(M_PI / 4.0 + lat / 2.0));
                        /*
                        x = R \cdot \lambda
                        y = R \cdot \ln \left( \tan \left( \frac{\pi}{4} + \frac{\phi}{2} \right) \right)
                        */
                    }
                    trim(r[4]);
                    this->node[i].delivery = stod(r[4]);
                    all_delivery += this->node[i].delivery;
                    trim(r[5]);
                    this->node[i].pickup = stod(r[5]);
                    all_pickup += this->node[i].pickup;
                    trim(r[6]);
                    this->node[i].start = stod(r[6]);
                    trim(r[7]);
                    this->node[i].end = stod(r[7]);
                    trim(r[8]);
                    this->node[i].s_time = stod(r[8]);
                }
                else
                {
                    flag = false;
                    break;
                }

            }
        }
        else if (results[0] == "DISTANCETIME_SECTION")
        {   
            getline(fp, line);   //ignore the table header
            while (getline(fp, line))
            {
                trim(line);
                if (line.size() == 0)
                    continue;
                std::vector<std::string> r = split(line, ',');
                if (r.size() > 1)
                {   
                    trim(r[1]);
                    int i = stoi(r[1]);
                    trim(r[2]);
                    int j = stoi(r[2]);
                    trim(r[3]);
                    double d = stod(r[3]);
                    trim(r[4]);
                    double t = stod(r[4]);
                    this->dist[i][j] = d;
                    // all_dist += d;
                    this->time[i][j] = t;
                    // all_time += t;
                    // if (d < this->min_dist) this->min_dist = d;
                    // if (d > this->max_dist) this->max_dist = d;
                    /*
                    
                    Note that we do not compute all_dist, all_time, this->min_dist, this->max_dist here.
                    See in this->floydWarshall()
                    
                    */
                }
                else
                {
                    flag = false;
                    break;
                }
            }
        }
        else if (results[0] == "DEPOT_SECTION")
        {
            getline(fp, line);
            trim(line);
            this->DC = stoi(line);
        }
    }
    fp.close();

    /*

    apply the Floyd-Warshall algorithm to compute the shortest path in terms of travel time between every pair of nodes.
    make sure that the travel time associated with these new edges naturally satisfies the triangle inequality, 
    allowing Proposition 1 to hold for any problem instance.
    
    */

    this->floydWarshall();  

    this->start_time = this->node[this->DC].start;
    this->end_time = this->node[this->DC].end;
    this->all_delivery = all_delivery;
    this->all_pickup = all_pickup;


    /*
   
    we preprocess the charging stations to rank them for insertion between each pair of nodes. 
    The ranking metric is the extra cost induced by insertion.
    To balance efficiency and solution quality, only the top sr·|F| ranked charging stations are considered for insertion in both PSI and SSI
    
    */

    std::vector<std::vector<int>> tmp_v_4(this->customer_num+this->station_num+1, std::vector<int>(this->station_num, 0));
    for (int i = 0; i <= this->customer_num + this->station_num; i++)
    {       
        this->optimal_staion.push_back(tmp_v_4);
    }
    std::vector<double> c_f_c_dist(this->station_num);
    std::vector<int> c_f_c_dist_argrank(this->station_num); 
    for (int i = 0; i <= this->customer_num; i++)    // c-f-c
    {       
        for (int j=0; j<= this->customer_num;j++){
              if (i!=j) {
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                if (i != this->DC && j!= this->DC){
                    for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                    }                    
                }
                else if (i == this->DC) {
                    if (this->dist[i][c_f_c_dist_argrank[0]+this->customer_num+1]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                }  
                else if (j == this->DC) {
                    if (this->dist[c_f_c_dist_argrank[0]+this->customer_num+1][j]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                } 
              }  
        }
    }    

    for (int i = 0; i <= this->customer_num; i++)    // c-f-f
    {       
        for (int j=this->customer_num+1; j<= this->customer_num+this->station_num;j++){
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        if (k!=j) {
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];
                        }
                        else{
                        c_f_c_dist[k-this->customer_num-1]=double(INFINITY);    
                        }
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                if (i != this->DC){
                    for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                    }                    
                }
                else {
                    if (this->dist[i][c_f_c_dist_argrank[0]+this->customer_num+1]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                }  
        }
    }    

    for (int i = this->customer_num+1; i <= this->customer_num+this->station_num; i++)    // f-f-c
    {       
        for (int j=0; j<= this->customer_num; j++){
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        if (k!=i) {
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];
                        }
                        else{
                        c_f_c_dist[k-this->customer_num-1]=double(INFINITY);    
                        }
                }

                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                if (j != this->DC){
                    for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                    }                    
                }
                else {
                    if (this->dist[c_f_c_dist_argrank[0]+this->customer_num+1][j]!=0){
                        for (int k=0;k<this->station_num;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                        }                         
                    }
                    else {
                    for (int k=0;k<this->station_num-1;k++){
                        optimal_staion[i][j][k]=c_f_c_dist_argrank[k+1]+this->customer_num+1;
                    } 
                    optimal_staion[i][j][this->station_num-1]=c_f_c_dist_argrank[0]+this->customer_num+1;
                    }
                }                      
        }
    } 

    for (int i = this->customer_num+1; i <= this->customer_num+this->station_num; i++)    // f-f-f
    {       
        for (int j = this->customer_num+1; j <= this->customer_num+this->station_num; j++){
              if (i!=j) {
                for (int k=this->customer_num+1;k<=this->customer_num+this->station_num;k++){
                        if (i!=k && j!=k){
                        c_f_c_dist[k-this->customer_num-1]=this->dist[i][k]+this->dist[k][j];    
                        }
                        else{
                        c_f_c_dist[k-this->customer_num-1]=double(INFINITY); 
                        }
                        
                }
                argsort(c_f_c_dist, c_f_c_dist_argrank, this->station_num);
                for (int k=0;k<this->station_num;k++){
                    optimal_staion[i][j][k]=c_f_c_dist_argrank[k]+this->customer_num+1;
                }                    
      
              }  
        }
    }       
    
    // print summary information
    printf("Node number: %d\n", this->node_num);
    printf("Customer number: %d\n", this->customer_num);
    printf("Station number: %d\n",this->station_num);
    printf("Avg pick-up/dilvery demand: %.4f,%.4f\n", this->all_pickup/this->customer_num, this->all_delivery/this->customer_num);
    printf("Starting/end time of DC: %.4f,%.4f\n", this->start_time, this->end_time);
   
    

    std::cout << '\n';
    if (parser.exists("random_seed"))
        this->seed = std::stoi(parser.retrieve<std::string>("random_seed"));
    this->rng.seed(this->seed);
    printf("Initial random seed: %d\n", this->seed);

    // set parameters
    if (parser.exists("pruning"))
    {
        printf("Pruning: on\n");
        this->pruning = true;
    }
    else printf("Pruning: off\n");

    if (parser.exists("output"))
    {
        this->if_output = true;
        this->output = parser.retrieve<std::string>("output");
        std::cout << "Write best solution to " << this->output << '\n';
    }

    if (parser.exists("time"))
        this->tmax = std::stoi(parser.retrieve<std::string>("time"));
    printf("Time limit: %d seconds\n", this->tmax);

    if (parser.exists("runs"))
        this->runs = std::stoi(parser.retrieve<std::string>("runs"));
    printf("Runs: %d\n", this->runs);

    if (parser.exists("g_1"))
        this->g_1 = std::stoi(parser.retrieve<std::string>("g_1"));
    printf("g_1: %d\n", this->g_1);

    if (parser.exists("pop_size"))
        this->p_size = std::stoi(parser.retrieve<std::string>("pop_size"));
    printf("Population size: %d\n", this->p_size);
    if (!chk_p_square(this->p_size))
    {
        printf("Expect popsize to be perfect squrare number\n");
        exit(-1);
    }
    int sr = int(sqrt(double(this->p_size)));
    if (sr == 1)
        this->latin.push_back(std::make_tuple(0.5, 0.5));
    else
    {
        double step = 1.0 / (sr - 1);
        for (int i = 0; i < sr; i++)
        {
            for (int j = 0; j < sr; j++)
            {
                double lambda = std::min(1.0, step * i);
                double gamma = std::min(1.0, step * j);
                this->latin.push_back(std::make_tuple(lambda, gamma));
            }
        }
        std::shuffle(this->latin.begin(), this->latin.end(), this->rng);
    }

    if (parser.exists("init"))
        this->init = parser.retrieve<std::string>("init");
    printf("Insertion for initialization: %s\n", this->init.c_str());
    if (parser.exists("k_init"))
        this->k_init = std::stoi(parser.retrieve<std::string>("k_init"));
    if (this->k_init == K)
        this->k_init = this->customer_num;
    printf("k_init: %d\n", this->k_init);

    if (parser.exists("cross_repair"))
        this->cross_repair = parser.retrieve<std::string>("cross_repair");
    printf("Insertion for crossover: %s\n", this->cross_repair.c_str());
    
    if (parser.exists("k_crossover"))
        this->k_crossover = std::stoi(parser.retrieve<std::string>("k_crossover"));
    if (this->k_crossover == K)
        this->k_crossover = this->customer_num;
    printf("k_crossover: %d\n", this->k_crossover);

    if (parser.exists("parent_selection"))
        this->selection = parser.retrieve<std::string>("parent_selection");
    printf("Parent selection: %s\n", this->selection.c_str());

    if (parser.exists("replacement"))
        this->replacement = parser.retrieve<std::string>("replacement");
    printf("Replacement strategy: %s\n", this->replacement.c_str());

    if (parser.exists("ls_prob"))
        this->ls_prob = std::stod(parser.retrieve<std::string>("ls_prob"));
    printf("Local search probability: %.2f\n", this->ls_prob);

    if (parser.exists("skip_finding_lo"))
    {
        printf("Skip finding_local_optima\n");
        this->skip_finding_lo = true;
    }

    if (parser.exists("O_1_eval"))
    {
        printf("O(1) evaluation: on\n");
        this->O_1_evl = true;
    }
    else
        printf("O(1) evaluation: off");

    if (parser.exists("no_crossover"))
    {
        printf("No crossover used\n");
        this->no_crossover = true;
    }

    if (parser.exists("two_opt"))
    {
        printf("2-opt: on\n");
        this->two_opt = true;
        small_opts.push_back("2opt");
        std::vector<Move> tmp_mem(this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("2opt", tmp_mem));
    }
    else
        printf("2-opt: off\n");

    if (parser.exists("two_opt_star"))
    {
        printf("2-opt*: on\n");
        this->two_opt_star = true;
        small_opts.push_back("2opt*");
        std::vector<Move> tmp_mem(this->vehicle.max_num * this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("2opt*", tmp_mem));
    }
    else
        printf("2-opt*: off\n");

    if (parser.exists("or_opt"))
    {
        printf("or-opt: on\n");
        this->or_opt = true;
        this->or_opt_len = std::stoi(parser.retrieve<std::string>("or_opt"));
        small_opts.push_back("oropt_single");
        small_opts.push_back("oropt_double");
        std::vector<Move> tmp_mem_0(this->vehicle.max_num);
        std::vector<Move> tmp_mem_1(this->vehicle.max_num * this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("oropt_single", tmp_mem_0));
        mem.insert(std::pair<std::string, std::vector<Move>>("oropt_double", tmp_mem_1));
    }
    else
        printf("or-opt: off\n");

    if (parser.exists("two_exchange"))
    {
        printf("2-exchange: on\n");
        this->two_exchange = true;
        this->exchange_len = std::stoi(parser.retrieve<std::string>("two_exchange"));
        small_opts.push_back("2exchange");
        std::vector<Move> tmp_mem(this->vehicle.max_num * this->vehicle.max_num);
        mem.insert(std::pair<std::string, std::vector<Move>>("2exchange", tmp_mem));
    }
    else
        printf("2-exchange: off\n");

    if (parser.exists("elo"))
        this->escape_local_optima = std::stoi(parser.retrieve<std::string>("elo"));
    printf("escape local optima number: %d\n", this->escape_local_optima);

    if (parser.exists("random_removal"))
    {
        printf("random_removal: on\n");
        this->random_removal = true;
        destroy_opts.push_back("random_removal");
    }
    else
        printf("random_removal: off\n");

    if (parser.exists("related_removal"))
    {
        printf("related_removal: on\n");
        this->related_removal = true;
        if (parser.exists("alpha"))
            this->alpha = std::stod(parser.retrieve<std::string>("alpha"));
        this->r = this->alpha * (this->all_dist / this->all_time);
        destroy_opts.push_back("related_removal");
        printf("alpha: %f, relateness norm factor: %f\n", this->alpha, this->r);
    }
    else
        printf("related_removal: off\n");

    if (parser.exists("removal_lower"))
    {
        this->destroy_ratio_l = std::stod(parser.retrieve<std::string>("removal_lower"));
    }
    printf("Destroy lower ration: %f\n", this->destroy_ratio_l);
    if (parser.exists("removal_upper"))
    {
        this->destroy_ratio_u = std::stod(parser.retrieve<std::string>("removal_upper"));
    }
    printf("Destroy upper ration: %f\n", this->destroy_ratio_u);

    if (parser.exists("regret_insertion"))
    {
        printf("regret_insertion: on\n");
        this->regret_insertion = true;
        repair_opts.push_back("regret_insertion");
    }
    else
        printf("regret_insertion: off\n");

    if (parser.exists("greedy_insertion"))
    {
        printf("greedy_insertion: on\n");
        this->greedy_insertion = true;
        repair_opts.push_back("greedy_insertion");
    }
    else
        printf("greedy_insertion: off\n");

    if (parser.exists("rd_removal_insertion"))
    {
        printf("Random removal and insertion: on\n");
        this->rd_removal_insertion = true;
    }
    else
        printf("Random removal and insertion: off\n");

    if (parser.exists("bks"))
        this->bks = std::stod(parser.retrieve<std::string>("bks"));
    
    if (parser.exists("individual_search"))
        this->individual_search = true;

    if (parser.exists("population_search"))
        this->population_search = true; 

    if (parser.exists("parallel_insertion"))
        this->parallel_insertion = true;    

    if (parser.exists("conservative_local_search"))
        this->conservative_local_search = true;
    
    if (parser.exists("aggressive_local_search"))
        this->aggressive_local_search = true;

    if (parser.exists("station_range")){
        this->station_range =std::ceil(this->station_num * std::stod(parser.retrieve<std::string>("station_range")));
    }  
    else  
        this->station_range = this->station_num;
    printf("Station range: %d\n", this->station_range);  

    if (parser.exists("subproblem_range")){
        this->subproblem_range = std::stoi(parser.retrieve<std::string>("subproblem_range"));
    }
    printf("Subproblem num: %d\n", this->subproblem_range); 

    int n_num = this->node_num;
    for (int i = 0; i < n_num; i++)
    {
        for (int j = 0; j < n_num; j++)
        {this->pm[i][j] = true;}
    }
    this->pre_processing();
}

void Data::floydWarshall() {
    this->all_dist = 0.0;
    this->all_time = 0.0;
    int n = this->node_num;
    std::vector<std::vector<double>> dist_shortest_t = this->dist;
    this->hyperarc = std::vector<std::vector<std::vector<int>>>(n, std::vector<std::vector<int>>(n, std::vector<int>(1, -1)));
    std::vector<std::vector<int>> next(n, std::vector<int>(n, -1));
    // complete graph
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
                next[i][j] = j;
        }
    }
    // only permit station k as intra-node in path from i to j, i.e. station k is not used to recharge
    for (int k = this->customer_num+1; k < n; ++k) {   
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (this->time[i][k] + this->time[k][j] < this->time[i][j]) {
                    this->time[i][j] = this->time[i][k] + this->time[k][j];   // shortest time from i to j
                    next[i][j] = next[i][k];
                }
            }
        }
    }
    // record all stations visited but do not recharge in the shortest path from i to j 
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::vector<int> path;
            int u, v;
            double total_dist = 0;
            u = i;
            v = j;
            while (u != v) {
                total_dist += this->dist[u][next[u][v]];
                u = next[u][v];
                if (u != v) path.push_back(u);
            }
            dist_shortest_t[i][j] = total_dist;   // in terms of shortest time, the path distance from i to j  
            this->hyperarc[i][j] = path;          // path information, i.e. path = (f_1, f_2, ... f_m),  we have i -> f_1 -> f_2 -> ... -> f_m -> j to replace i -> j, which is the shortest path in terms of time from i to j
            this->all_dist += dist_shortest_t[i][j];  
            this->all_time += time[i][j];
            if (dist_shortest_t[i][j] < this->min_dist) this->min_dist = dist_shortest_t[i][j];
            if (dist_shortest_t[i][j] > this->max_dist) this->max_dist = dist_shortest_t[i][j];
        }
    }
    this->dist = dist_shortest_t;
}

void Data::pre_processing()
{
    //printf("--------------------------------------------\n");
    if (this->related_removal)
    {
        int c_num = this->customer_num;
        int DC = this->DC;
        for (int i = 0; i <= c_num; i++)
        {
            if (i == DC) continue;
            for (int j = 0; j <= c_num; j++)
            {
                if(j == DC || j == i)
                    this->rm[i][j] = double(INFINITY);
                else
                {
                    auto &node_i = this->node[i];
                    auto &node_j = this->node[j];
                    double tmp_1 = this->r * std::max(node_j.start - node_i.s_time - this->time[i][j] - node_i.end, 0.0);
                    double tmp_2 = this->r * PENALTY_FACTOR * std::max(node_i.start + node_i.s_time + this->time[i][j] - node_j.end, 0.0);
                    double tmp_3 = this->dist[i][j];
                    this->rm[i][j] = tmp_3 + tmp_1 + tmp_2;
                }
            }
            argsort(this->rm[i], this->rm_argrank[i], c_num+1);
        }
    }
    if (this->pruning)
    {
        //printf("Do Pruning\n");
        int c_num = this->customer_num;
        int DC = this->DC;
        int count_tw = 0;
        int count_c = 0;
        for (int i = 0; i <= c_num; i++)
        {
            if (i == DC) continue;
            for (int j = 0; j <= c_num; j++)
            {
                if (j == DC || j == i) continue;
                double a_i = this->node[i].start;
                double s_i = this->node[i].s_time;
                double d_i = this->node[i].delivery;
                double p_i = this->node[i].pickup;

                double b_j = this->node[j].end;
                double d_j = this->node[j].delivery;
                double p_j = this->node[j].pickup;
                double time_ij = this->time[i][j];

                if (a_i + s_i + time_ij > b_j)
                {
                    this->pm[i][j] = false;
                    count_tw++;
                }
                if (d_i + d_j > this->vehicle.capacity || p_i + p_j > this->vehicle.capacity)
                {
                    this->pm[i][j] = false;
                    count_c++;
                }
            }
        }
        int sum = c_num*(c_num-1);
        printf("Total edges %d, prune by time window %d(%.4f%%), prune by capacity %d(%.4f%%)\n",\
                sum, count_tw, 100.0*double(count_tw)/sum, count_c, 100.0*double(count_c)/sum);
    }
}

// read over

void Data::clear_mem()
{
    for (auto &x : this->mem)
    {
        for (auto &move : x.second)
        {
            move.len_1 = 0;
        }
    }
}

Move &Data::get_mem(std::string &opt, const int &r1, const int &r2)
{
    if (opt == "2opt")
        return this->mem[opt][r1];
    else if (opt == "2opt*")
        return this->mem[opt][r1 * this->vehicle.max_num + r2];
    else if (opt == "oropt_single")
        return this->mem[opt][r1];
    else if (opt == "oropt_double")
        return this->mem[opt][r1 * this->vehicle.max_num + r2];
    else if (opt == "2exchange")
        return this->mem[opt][r1 * this->vehicle.max_num + r2];
    else
    {
        std::cout << "Unknown opt name: " << opt << std::endl;
        exit(-1); 
    }
}