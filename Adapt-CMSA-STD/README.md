# Adapt-CMSA

Reference unofficial `C++` reimplementation of multiple variants of the Adapt-CMSA:

- Adapt-CMSA-STD
- Adapt-CMSA-SETCOV

**compile:**

```shell
g++ -std=c++11 -o ../bin/evrp-tw-spd -O3 evrp_tw_spd_solver.cpp operator.cpp search_framework.cpp solution.cpp util.cpp data.cpp
```

```c++
# parameters:
    double t_prop;
    double t_ILP;
    double alpha_LB; 
    double alpha_UB; 
    double alpha_red; 
    double d_rate; 
    double h_rate; 
    int n_a; 
    int l_size; 
    double gamma; 
    int delta_n; 
    int delta_l_size; 
    double delta_gamma; 
    double infeasible_rate; 
    int dummy_stations;
```

**execute:**

```shell
./bin/evrp-tw-spd --problem ./data/akb_instances/c103c5.txt --pruning --time 105 --runs 10 --g_1 20 --O_1_eval --two_opt --two_opt_star --or_opt 2 --two_exchange 2 --dummy_stations 3
```

