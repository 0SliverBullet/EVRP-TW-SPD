# EVRP-TW-SPD Benchmark

The benchmark contains both the *akb* set and the *jd* set. 

- **the *akb* set**: includes only synthetic instances of small and medium scales. Specifically, This *akb* set

  has 92 instances, including 

  - 36 small-scale instances (twelve 5-customer instances, twelve 10-customer instances, and twelve 15-customer instances, each with 2 to 8 stations), e.g., `c101C5.txt` is the 5-customer instance,    `c101C10.txt` is the 10-customer instance, and `c103C15.txt` is the 15-customer instance.
  - 56 medium-scale instances (100-customer instances with 21 stations ), e.g., `c101_21.txt` is the 100-customer instance.

- **the *jd* set**: a new benchmark set derived from JD Logistics’ distribution system. We generated instances with 200, 400, 600, 800, and 1000 customers, each with 100 stations. For each problem scale, we generated 4 instances, resulting in a set of 20 large scale instances, e.g., `jd200_1.txt` is the 200-customer instance.

## Instance File Structure 

Note that the corresponding variables in our paper follow "i.e.,".

1. Basic Information: This section contains detailed information about a electric vehicle (EV).

| Item                 | Comment                                                      |
| -------------------- | ------------------------------------------------------------ |
| **NAME**             | The name of the instance, e.g.,  `c101_21`, `200_1`.         |
| **TYPE**             | Specifies the problem type, which is `EVRP-TW-SPD`.          |
| **DIMENSION**        | The total number of nodes in the instance, i.e., $1+M+P$.    |
| **VEHICLES**         | Ignore.                                                      |
| **DISPATCHINGCOST**  | The cost incurred when dispatching a vehicle, i.e., $\mu_1$. |
| **UNITCOST**         | The cost per unit distance, i.e., $\mu_2$.                   |
| **CAPACITY**         | The maximum loading capacity of each vehicle, i.e, $C$.      |
| **ELECTRIC_POWER**   | The maximum battery capacity of each vehicle, i.e., $Q$.     |
| **CONSUMPTION_RATE** | The the EV’s battery energy consumption rate per unit of travel distance, i.e., $h$. |
| **RECHARGING_RATE**  | The recharging rate, i.e., $g$. Note that it takes time $g$ to charge one unit of battery energy. |
| **EDGE_WEIGHT_TYPE** | Ignore.                                                      |

2. Node Section (NODE_SECTION): This section contains detailed information about each node in the instance.

| Item                              | Comment                                                      |
| --------------------------------- | ------------------------------------------------------------ |
| **ID**                            | Unique identifier for the node, i.e., $i \in \boldsymbol{V}'$. |
| **type**                          | The type of node (`d` for depot, `c` for customer, `f` for charging station). |
| **x / lng**                       | The x-coordinate (in the *akb* set) / longitude (in the *jd* set) of the node, i.e., $x_i / lng_i$. |
| **y / lat**                       | The y-coordinate (in the *akb* set) / latitude (in the *jd* set)  of the node, i.e., $y_i / lat_i$. |
| **delivery**                      | The amount of goods delivered to the node, i.e., $u_i$.      |
| **pickup**                        | The amount of goods to be picked up at the node, i.e., $v_i$. |
| **ready_time / first_receive_tm** | The earliest time at which service at the node can begin. i.e., $e_i$. |
| **due_date /  last_receive_tm**   | The latest time at which service at the node can begin. i.e., $l_i$. |
| **service_time**                  | The time required to service the node, i.e., $s_i$.          |

3. Distance and Time Section (DISTANCETIME_SECTION): This section provides the distances and time taken to travel between nodes. 

| Item          | Comment                                                      |
| ------------- | ------------------------------------------------------------ |
| **ID**        | Unique identifier for the directly connected edge, i.e., $(i, j) \in \boldsymbol{E}$. |
| **from_node** | The starting node ID, i.e., $i$.                             |
| **to_node**   | The ending node ID, i.e, $j$.                                |
| **distance**  | The travel distance between the two nodes, i.e., $d_{ij}$.   |
| **spend_tm**  | The travel time between the two nodes, i.e., $t_{ij}$.       |

4. Depot Section (DEPOT_SECTION): This section indicates the depot's ID, i.e., $0$.