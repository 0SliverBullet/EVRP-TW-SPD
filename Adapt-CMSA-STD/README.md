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
./bin/evrp-tw-spd --problem ./data/akb_instances/c101c5.txt --pruning --time 105 --runs 10 --g_1 20 --O_1_eval --two_opt --two_opt_star --or_opt 2 --two_exchange 2 --dummy_stations 3
```



```shell
cmake -G "Visual Studio 16 2019" -B build
```



CPLEX installation path, etc.

```shell
D:\IBM\ILOG\CPLEX_Studio201

D:\IBM\ILOG\CPLEX_Studio201\cplex\include\
D:\IBM\ILOG\CPLEX_Studio201\concert\include\

D:\IBM\ILOG\CPLEX_Studio201\cplex\lib\x64_windows_msvc14\stat_mda\
D:\IBM\ILOG\CPLEX_Studio201\concert\lib\x64_windows_msvc14\stat_mda\


```

