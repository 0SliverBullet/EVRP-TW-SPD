# EVRP-TW-SPD


## Overview



The dataset and unofficial CPLEX code for the Electric Vehicle Routing Problem with Time Windows, Simultaneous Pickup and Deliveries (EVRP-TW-SPD), based on the paper *["Application of CMSA to the Electric Vehicle Routing Problem with Time Windows, Simultaneous Pickup and Deliveries, and Partial Vehicle Charging"](https://doi.org/10.1007/978-3-031-26504-4_1)* (Akbay et al., 2022), have been reimplemented here.



The official EVRP-TW-SPD instances used in Akbay et al. (2022) can be found at [EVRP-TW-SPD-Instances](https://github.com/manilakbay/EVRP-TW-SPD-Instances).



## File Structure

```
EVRP-TW-SPD/
│
├── README.md                   # overview
│
├── src/                        # CPLEX code (unofficial)
│   ├── EVRP-TW-SPD.mod         # EVRP-TW-SPD MILP .mod file 
│   ├── EVRP-TW-SPD.ops         # specific CPLEX optimization settings .ops file (not optimal)
│   ├── cplex_evrptwspd         # CPLEX input data .dat files
│   └── ...  
│
├── data/                       # datasets (official)
│   ├── evrptwspd_instances     # the EVRP-TW-SPD dataset used in Akbay et al. (2022)
│   ├── evrptw_instances        # the E-VRPTW dataset used in Schneider et al. (2014)
│   └── ...
│
├── CITATION.cff
└── LICENSE        
```



We generated the EVRP-TW-SPD dataset and performed CPLEX 20.1 verification.



## Dataset Generation



As stated in the paper by Akbay et al. (2022), the EVRP-TW-SPD dataset they used was generated based on the E-VRPTW problem instances introduced in the paper *["The Electric Vehicle Routing Problem with Time Windows and Recharging Stations"](https://doi.org/10.1287/trsc.2013.0490)* (Schneider et al., 2014), where the delivery demand of each customer was divided into separate delivery and pickup demands using the approach from the paper *["A Cluster Insertion Heuristic for Single and Multiple Depot Vehicle Routing Problems with Backhauling"](https://link.springer.com/article/10.1057/palgrave.jors.2600808)* (Salhi et al., 1999).

**Note that when calculating the separated delivery value, Akbay et al. (2022) automatically reserved the integer part by truncating the decimal part.** We followed the details of their dataset generation process; thus, the EVRP-TW-SPD dataset generated in our repository should match the official EVRP-TW-SPD instances presented in Akbay et al. (2022).

The EVRP-TW-SPD dataset instance generator `evrptwspd_instances_generator.py` was implemented in Python.



## Verification



To verify the correctness of the EVRP-TW-SPD dataset generation, we provide **unofficial** CPLEX code to compare results with the corresponding numerical results in the paper by Akbay et al. (2022), as their CPLEX code is not open-sourced.

For simplicity, this CPLEX code is based on [E-VRPTW](https://github.com/0SliverBullet/E-VRPTW), where we **set three dummy vertices** for each charging station in the `.dat` files. **However, the `.ops` file is not exactly the same as in the paper by Akbay et al. (2022).**

Experiments were performed on a personal computer with an 11th Gen Intel® Core™ i5-1135G7 CPU, which has 8 cores operating at 2.40 GHz, and a minimum of 16 GB of RAM.

#### Computational results of CPLEX 20.1 on small-scale instances with five customers

<table>
    <tr>
        <th rowspan="2">Instances name</th>
        <th colspan="3">CPLEX 20.1 numerical results in paper (Akbay et al., 2022)</th>
        <th colspan="3">CPLEX 20.1 numerical results in verification </th>
        <th rowspan="2">gap(%)</th>
    </tr>
    <tr>
        <th>m</th>
        <th>best</th>
        <th>time</th>
        <th>m</th>
        <th>best</th>
        <th>time</th>
    </tr>
    <tr>
        <td>c101C5</td>
        <td>2</td>
        <td>2257.75</td>
        <td>0.61</td>
        <td>2</td>
        <td>2257.75</td>
        <td>4.62</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>c103C5</td>
        <td>1</td>
        <td>1175.37</td>
        <td>0.58</td>
        <td>1</td>
        <td>1175.37</td>
        <td>1.24</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>c206C5</td>
        <td>1</td>
        <td>1242.56</td>
        <td>0.82</td>
        <td>1</td>
        <td>1242.56</td>
        <td>114.26</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>c208C5</td>
        <td>1</td>
        <td>1158.48</td>
        <td>0.12</td>
        <td>1</td>
        <td>1158.48</td>
        <td>1.03</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>r104C5</td>
        <td>2</td>
        <td>2136.69</td>
        <td>0.03</td>
        <td>2</td>
        <td>2136.69</td>
        <td>1.67</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>r105C5</td>
        <td>2</td>
        <td>2156.08</td>
        <td>0.04</td>
        <td>2</td>
        <td>2156.08</td>
        <td>0.90</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>r202C5</td>
        <td>1</td>
        <td>1128.78</td>
        <td>0.08</td>
        <td>1</td>
        <td>1128.78</td>
        <td>1.16</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>r203C5</td>
        <td>1</td>
        <td>1179.06</td>
        <td>0.04</td>
        <td>1</td>
        <td>1179.06</td>
        <td>1.32</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>rc105C5</td>
        <td>2</td>
        <td>2233.77</td>
        <td>3.10</td>
        <td>2</td>
        <td>2233.77</td>
        <td>6.51</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>rc108C5</td>
        <td>2</td>
        <td>2253.93</td>
        <td>0.27</td>
        <td>2</td>
        <td>2253.93</td>
        <td>6.04</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>rc204C5</td>
        <td>1</td>
        <td>1176.39</td>
        <td>0.36</td>
        <td>1</td>
        <td>1176.39</td>
        <td>12.41</td>
        <td>0.00</td>
    </tr>
    <tr>
        <td>rc208C5</td>
        <td>1</td>
        <td>1167.98</td>
        <td>0.17</td>
        <td>1</td>
        <td>1167.98</td>
        <td>1.13</td>
        <td>0.00</td>
    </tr>
</table>

where *m* denotes the vehicle number, *best* represents the optimal objective values, *time* indicates the total runtime in seconds, and *gap* is calculated as $\frac{best_{\text{in verification}} - best_{\text{in paper}}}{best_{\text{in paper}}} \times 100\%$.


#### CPLEX 20.1 verification conclusion

The computational results for the EVRP-TW-SPD dataset in our repository **align with** those from the dataset officially used in the paper by Akbay et al. (2022), except for the running times in CPLEX (due to different `.ops` files and running machines), indicating the correctness of the EVRP-TW-SPD dataset generation.



## References

- Akbay, M. A., Kalayci, C. B., & Blum, C. (2022). Application of cmsa to the electric vehicle routing problem with time windows, simultaneous pickup and deliveries, and partial vehicle charging. In *Metaheuristics International Conference* (pp. 1-16). Cham: Springer International Publishing.
- Schneider, M., Stenger, A., & Goeke, D. (2014). The electric vehicle-routing problem with time windows and recharging stations. *Transportation science*, *48*(4), 500-520.
- Salhi, S., & Nagy, G. (1999). A cluster insertion heuristic for single and multiple depot vehicle routing problems with backhauling. *Journal of the operational Research Society*, *50*, 1034-1042.
