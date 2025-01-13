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
./bin/evrp-tw-spd --problem ./data/akb_instances/c101C5.txt --pruning --time 105 --runs 10 --g_1 20 --O_1_eval --two_opt --two_opt_star --or_opt 2 --two_exchange 2 --dummy_stations 3
```



```shell
cmake -G "Visual Studio 16 2019" -B build
```



**CPLEX path:**

```shell

- Windows:

D:\IBM\ILOG\CPLEX_Studio201

D:\IBM\ILOG\CPLEX_Studio201\cplex\include
D:\IBM\ILOG\CPLEX_Studio201\concert\include

D:\IBM\ILOG\CPLEX_Studio201\cplex\lib\x64_windows_msvc14\stat_mda
D:\IBM\ILOG\CPLEX_Studio201\concert\lib\x64_windows_msvc14\stat_mda

- Linux:

/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/cplex/include
/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/concert/include

/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic
/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/concert/lib/x86-64_linux/static_pic

```

**Note:**

1. Please note that Windows does not distinguish between upper and lower case of file names read in, while Linux does! i.e., `c101c5.txt` is not excatly the same one as `c101C5.txt`. 


2. Run the following command to check the architecture, when you consider which CPLEX version to download:

```shell
uname -m

```

## Useful Links

CPLEX Configurations on Linux Server:

1. [安装IBM CPLEX学术版 academic edition | conda 安装 CPLEX][https://blog.csdn.net/yihang___/article/details/125385030]
2. [IBM ILOG CPLEX Optimization Studio 20.10 for Linux x86-64][https://www-50.ibm.com/isc/esd/dswdown/home?ticket=Xa.2%2FXb.Z7LJBh8BR1xZKHRxeXt8fCYJ%2FXc.%2FXd.%2FXf.%2FXg.13164104%2FXi.%2FXY.scholars%2FXZ.1oQFZSHe_FbCcgVKn350AG3p02seIZhB&partNumber=G0798ML]
3. [Cplex20.1版本bin包Linux安装过程][https://blog.csdn.net/wuxiaolongah/article/details/122649495]

Result:

```shell

===============================================================================
Installation Complete
---------------------

IBM ILOG CPLEX Optimization Studio 20.1.0 has been successfully installed to:

   /home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201

PRESS <ENTER> TO EXIT THE INSTALLER:

```

**C++ invokes CPLEX in Visual Studio Code on Linux Server (the most important):**

Please see folder `cplex/demo` which is a tutorial example; also refer to the following link

1. [visual studio code 远程连接到服务器 如何配置使得 C++ 可以调用 CPLEX 头文件][https://blog.csdn.net/qq_51744740/article/details/142151784]

```shell
export CPLEX_STUDIO_DIR201=/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201
export CPLUS_INCLUDE_PATH=$CPLEX_STUDIO_DIR201/cplex/include:$CPLEX_STUDIO_DIR201/concert/include:$CPLUS_INCLUDE_PATH
export LIBRARY_PATH=$CPLEX_STUDIO_DIR201/cplex/lib/x86-64_linux/static_pic:$CPLEX_STUDIO_DIR201/concert/lib/x86-64_linux/static_pic:$LIBRARY_PATH
export LD_LIBRARY_PATH=$CPLEX_STUDIO_DIR201/cplex/lib/x86-64_linux/static_pic:$CPLEX_STUDIO_DIR201/concert/lib/x86-64_linux/static_pic:$LD_LIBRARY_PATH
export PATH=$CPLEX_STUDIO_DIR201/cplex/bin/x86-64_linux:$PATH

```

in folder `build`:

```shell
./build/evrp-tw-spd --problem ./data/akb_instances/c101C5.txt --pruning --time 105 --runs 10 --g_1 20 --O_1_eval --two_opt --two_opt_star --or_opt 2 --two_exchange 2 --dummy_stations 3

```

