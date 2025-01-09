#include "data.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
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

    if (parser.exists("dummy_stations"))
        this->dummy_stations = std::stoi(parser.retrieve<std::string>("dummy_stations"));
    printf("Dummy stations: %d |F|\n", this->dummy_stations);

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
            for (int i = 0; i < this->node_num; i++)
            {
                this->node.push_back({0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true});
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

            /* append dummy stations */

            // update total number of nodes, station num
            this->station_cardinality = this->station_num; 
            this->station_num = this->station_num * this->dummy_stations;
            this->node_cardinality = this->node_num;
            this->node_num = this->node_num + (this->dummy_stations - 1) * this->station_cardinality;

            // copy stations
            for (int i = this->customer_num + 1; i <= this->customer_num + this->station_cardinality; i++){
                for (int j = 1; j <= this->dummy_stations - 1; j++){
                    this->node.push_back({0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true});
                    int dummy_idx = i + j * this->station_cardinality;
                    this->node[dummy_idx].type = this->node[i].type;
                    this->node[dummy_idx].x = this->node[i].x;
                    this->node[dummy_idx].y = this->node[i].y;
                    this->node[dummy_idx].delivery = this->node[i].delivery;
                    this->node[dummy_idx].pickup = this->node[i].pickup;
                    this->node[dummy_idx].start = this->node[i].start;
                    this->node[dummy_idx].end = this->node[i].end;
                    this->node[dummy_idx].s_time = this->node[i].s_time;
                    this->node[dummy_idx].idle = this->node[i].idle;
                }

            }

            // init dist, time, rm, rm_argrank, pm
            std::vector<double> tmp_v_1(this->node_num, 0.0);
            std::vector<bool> tmp_v_2(this->node_num, 0.0);
            std::vector<int> tmp_v_3(this->node_num, 0);
            for (int i = 0; i < this->node_num; i++)
            {
                this->dist.push_back(tmp_v_1);
                this->time.push_back(tmp_v_1);
                this->rm.push_back(tmp_v_1);
                this->rm_argrank.push_back(tmp_v_3);
                this->pm.push_back(tmp_v_2);
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
                    all_dist += d;
                    this->time[i][j] = t;
                    all_time += t;
                    if (d < this->min_dist) this->min_dist = d;
                    if (d > this->max_dist) this->max_dist = d;
                    /*
                    
                    Note that we need compute all_dist, all_time, this->min_dist, this->max_dist here.

                    */
                }
                else
                {
                    flag = false;
                    break;
                }
            }

            /* append dummy stations */
            
            for (int i = this->customer_num + 1; i <= this->customer_num + this->station_cardinality; i++){
                for (int j = 1; j <= this->dummy_stations - 1; j++){
                    int dummy_idx = i + j * this->station_cardinality;
                    double d = 0;
                    double t = 0;
                    for (int k = 0; k < this->node_cardinality; k++){
                            // rows
                            d = this->dist[i][k];
                            t = this->time[i][k];
                            this->dist[dummy_idx][k] = d;
                            all_dist += d;
                            this->time[dummy_idx][k] = t;
                            all_time += t;
                            if (d < this->min_dist) this->min_dist = d;
                            if (d > this->max_dist) this->max_dist = d;       
                            // columns
                            d = this->dist[k][i];
                            t = this->time[k][i];
                            this->dist[k][dummy_idx] = d;
                            all_dist += d;
                            this->time[k][dummy_idx] = t;
                            all_time += t;
                            if (d < this->min_dist) this->min_dist = d;
                            if (d > this->max_dist) this->max_dist = d;
                    }

                    for (int k = this->customer_num + 1; k <= this->customer_num + this->station_cardinality; k++){
                        for (int l = 1; l <= this->dummy_stations - 1; l++){
                            int dummy_idy = k + l * this->station_cardinality;
                            // diagonals
                            d = this->dist[i][k];
                            t = this->time[i][k];
                            this->dist[dummy_idx][dummy_idy] = d;
                            all_dist += d;
                            this->time[dummy_idx][dummy_idy] = t;
                            all_time += t;
                            if (d < this->min_dist) this->min_dist = d;
                            if (d > this->max_dist) this->max_dist = d;  
                        }
                    }    

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
    /*
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

    if (parser.exists("runs"))
        this->runs = std::stoi(parser.retrieve<std::string>("runs"));
    printf("Runs: %d\n", this->runs);

    if (parser.exists("g_1"))
        this->g_1 = std::stoi(parser.retrieve<std::string>("g_1"));
    printf("g_1: %d\n", this->g_1);

    printf("---------- Adapt-CMSA-STD Parameters Begin ---------- \n");
    if (parser.exists("t_prop"))
        this->t_prop = std::stod(parser.retrieve<std::string>("t_prop"));
    printf("    t_prop        : %.2lf\n", this->t_prop);

    if (parser.exists("t_ILP"))
        this->t_ILP = std::stod(parser.retrieve<std::string>("t_ILP"));
    printf("    t_ILP         : %.2lf\n", this->t_ILP);

    if (parser.exists("alpha_LB"))
        this->alpha_LB = std::stod(parser.retrieve<std::string>("alpha_LB"));
    printf("    alpha_LB      : %.2lf\n", this->alpha_LB);

    if (parser.exists("alpha_UB"))
        this->alpha_UB = std::stod(parser.retrieve<std::string>("alpha_UB"));
    printf("    alpha_UB      : %.2lf\n", this->alpha_UB);

    if (parser.exists("alpha_red"))
        this->alpha_red = std::stod(parser.retrieve<std::string>("alpha_red"));
    printf("    alpha_red     : %.2lf\n", this->alpha_red);

    if (parser.exists("d_rate"))
        this->d_rate = std::stod(parser.retrieve<std::string>("d_rate"));
    printf("    d_rate        : %.2lf\n", this->d_rate);

    if (parser.exists("h_rate"))
        this->h_rate = std::stod(parser.retrieve<std::string>("h_rate"));
    printf("    h_rate        : %.2lf\n", this->h_rate);

    if (parser.exists("n_a"))
        this->n_a = std::stoi(parser.retrieve<std::string>("n_a"));
    printf("    n_a           : %d\n", this->n_a);

    if (parser.exists("l_size"))
        this->l_size = std::stoi(parser.retrieve<std::string>("l_size"));
    printf("    l_size        : %d\n", this->l_size);

    if (parser.exists("delta_n"))
        this->delta_n = std::stoi(parser.retrieve<std::string>("delta_n"));
    printf("    delta_n       : %d\n", this->delta_n);

    if (parser.exists("delta_l_size"))
        this->delta_l_size = std::stoi(parser.retrieve<std::string>("delta_l_size"));
    printf("    delta_l_size  : %d\n", this->delta_l_size);

    if (parser.exists("dummy_stations"))
        this->dummy_stations = std::stoi(parser.retrieve<std::string>("dummy_stations"));
    printf("    dummy_stations: %d\n", this->dummy_stations);
    printf("---------- Adapt-CMSA-STD Parameters End   ---------- \n");

    if (parser.exists("O_1_eval"))
    {
        printf("O(1) evaluation: on\n");
        this->O_1_evl = true;
    }
    else
        printf("O(1) evaluation: off");

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

    if (parser.exists("bks"))
        this->bks = std::stod(parser.retrieve<std::string>("bks"));

    if (parser.exists("station_range")){
        this->station_range =std::ceil(this->station_num * std::stod(parser.retrieve<std::string>("station_range")));
    }  
    else  
        this->station_range = this->station_num;
    printf("Station range: %d\n", this->station_range);  

    int n_num = this->node_num;
    for (int i = 0; i < n_num; i++)
    {
        for (int j = 0; j < n_num; j++)
        {this->pm[i][j] = true;}
    }
    this->pre_processing();

    // print dummy station 
    // for (int i = 0; i < this->node_num; i++){
    //     for (int j = 0; j < this->node_num; j++){
    //         printf("%.2lf ", this->dist[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
    // exit(0);
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