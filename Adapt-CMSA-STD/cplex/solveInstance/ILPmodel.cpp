// -------------------------------------------------------------- -*- C++ -*-
// File: ILPModel.cpp
// --------------------------------------------------------------------------
// Licensed Materials
// Author: Zubin Zheng
// Creation Date: 2025/01/11
// --------------------------------------------------------------------------
//
// ILPModel.cpp -- EVRP-TW-SPD ILP Model
// Example Instance: c103C5
// --------------------------------------------------------------------------
#include <cstdlib>
#include <ilcplex/ilocplex.h>
#include <cmath>
#include <vector>
#include <iostream>

ILOSTLBEGIN

/*****************************************************************************
 *
 * SETS
 *
 *****************************************************************************/
IloInt numDepots0, numDepotsN1, numStations, numCustomers;
IloInt numStations0, numCustomer0, numStationsCustomers, numStationsCustomers0, numStationsCustomersN1;
IloInt numTotal;
std::vector<IloInt> Depot_0, Depot_N1, Stations, Customers;
std::vector<IloInt> Stations_0, Customers_0, StationsCustomers, StationsCustomers_0, StationsCustomers_N1;
std::vector<IloInt> Total;
/*****************************************************************************
 *
 * PARAMETERS
 *
 *****************************************************************************/
IloNum v, C, Q, g, h;
IloNum M;

IloNumArray xCoord, yCoord, q, p;
IloNumArray e, l, s;
// vector<double> s;
IloArray<IloNumArray> d, Time;

// Define data structures
void defineData(IloEnv env) {
    // Initialize parameters here based on input
    // For simplicity, hardcoded initialization is provided
    numDepots0 = 1;
    numDepotsN1 = 1;
    numStations = 6;  // 3 dummy stations for each station
    numCustomers = 5;

    numStations0 = numDepots0 + numStations;
    numCustomer0 = numDepots0 + numCustomers;
    numStationsCustomers = numStations + numCustomers;
    numStationsCustomers0 = numDepots0 + numStations + numCustomers;
    numStationsCustomersN1 = numStations + numCustomers + numDepotsN1;
    numTotal = numDepots0 + numStations + numCustomers + numDepotsN1;

    Depot_0.push_back(0);
    Depot_N1.push_back(12);
    for (IloInt i = 1; i <= 6; i++){
        Stations.push_back(i);
    }
    for (IloInt i = 7; i <= 11; i++){
        Customers.push_back(i);
    }
    for (IloInt i = 0; i <= 6; i++){
        Stations_0.push_back(i);
    }
    Customers_0.push_back(0);
    for (IloInt i = 7; i <= 11; i++){
        Customers_0.push_back(i);
    }
    for (IloInt i = 1; i <= 11; i++){
        StationsCustomers.push_back(i);
    }
    for (IloInt i = 0; i <= 11; i++){
        StationsCustomers_0.push_back(i);
    }
    for (IloInt i = 1; i <= 12; i++){
        StationsCustomers_N1.push_back(i);
    }
    for (IloInt i = 0; i <= 12; i++){
        Total.push_back(i);
    }

    v = 1.0; // Example velocity
    C = 200.0;
    Q = 77.75;
    g = 3.47;
    h = 1.0;
    M = 1000;

    xCoord = IloNumArray(env, numTotal);
    yCoord = IloNumArray(env, numTotal);
    q = IloNumArray(env, numTotal);
    p = IloNumArray(env, numTotal);
    e = IloNumArray(env, numTotal);
    l = IloNumArray(env, numTotal);
     // s = vector<double>(numTotal);
    s = IloNumArray(env, numTotal);

    d = IloArray<IloNumArray>(env, numTotal);
    Time = IloArray<IloNumArray>(env, numTotal);

    xCoord[0] = 40.0; xCoord[1] = 40.0; xCoord[2] = 40.0; xCoord[3] = 40.0;
    xCoord[4] = 39.0; xCoord[5] = 39.0; xCoord[6] = 39.0; xCoord[7] = 30.0;
    xCoord[8] = 58.0; xCoord[9] = 48.0; xCoord[10] = 40.0; xCoord[11] = 25.0;
    xCoord[12] = 40.0;

    yCoord[0] = 50.0; yCoord[1] = 50.0; yCoord[2] = 50.0; yCoord[3] = 50.0;
    yCoord[4] = 26.0; yCoord[5] = 26.0; yCoord[6] = 26.0; yCoord[7] = 50.0;
    yCoord[8] = 75.0; yCoord[9] = 40.0; yCoord[10] = 15.0; yCoord[11] = 50.0;
    yCoord[12] = 50.0;

    q[0] = 0.0; q[1] = 0.0; q[2] = 0.0; q[3] = 0.0;
    q[4] = 0.0; q[5] = 0.0; q[6] = 0.0; q[7] = 6.0;
    q[8] = 15.0; q[9] = 8.0; q[10] = 15.0; q[11] = 5.0;
    q[12] = 0.0;

    p[0] = 0.0; p[1] = 0.0; p[2] = 0.0; p[3] = 0.0;
    p[4] = 0.0; p[5] = 0.0; p[6] = 0.0; p[7] = 4.0;
    p[8] = 5.0; p[9] = 2.0; p[10] = 25.0; p[11] = 5.0;
    p[12] = 0.0;

    e[0] = 0.0; e[1] = 0.0; e[2] = 0.0; e[3] = 0.0;
    e[4] = 0.0; e[5] = 0.0; e[6] = 0.0; e[7] = 0.0;
    e[8] = 0.0; e[9] = 67.0; e[10] = 989.0; e[11] = 0.0;
    e[12] = 0.0;

    l[0] = 1236.0; l[1] = 1236.0; l[2] = 1236.0; l[3] = 1236.0;
    l[4] = 1236.0; l[5] = 1236.0; l[6] = 1236.0; l[7] = 1136.0;
    l[8] = 1115.0; l[9] = 139.0; l[10] = 1063.0; l[11] = 1131.0;
    l[12] = 1236.0;

    s[0] = 0.0; s[1] = 0.0; s[2] = 0.0; s[3] = 0.0;
    s[4] = 0.0; s[5] = 0.0; s[6] = 0.0; s[7] = 90.0;
    s[8] = 90.0; s[9] = 90.0; s[10] = 90.0; s[11] = 90.0;
    s[12] = 0.0;

    for (IloInt i = 0; i < numTotal; ++i) {
        d[i] = IloNumArray(env, numTotal);
        Time[i] = IloNumArray(env, numTotal);
    }

    // Fill in distances and Times based on coordinates
    for (IloInt i: Total) {
        for (IloInt j: Total) {
            if (i == j) {
                d[i][j] = 0;
                Time[i][j] = 0;
            } else {
                d[i][j] = std::sqrt(std::pow(xCoord[i] - xCoord[j], 2) + std::pow(yCoord[i] - yCoord[j], 2));
                Time[i][j] = d[i][j] / v;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    IloEnv env;
    try {
        defineData(env);

        // Decision variables
        IloModel model(env);

        IloNumVarArray t(env, numTotal, 0.0, IloInfinity);  // IloInfinity to be optimized
        IloArray<IloNumVarArray> U(env, numTotal);
        IloArray<IloNumVarArray> V(env, numTotal);
        IloNumVarArray y(env, numTotal, 0.0, Q);
        IloNumVarArray Y(env, numTotal, 0.0, Q);
        IloArray<IloBoolVarArray> x(env, numTotal);


        for (IloInt i: Total) {
            U[i] = IloNumVarArray(env, numTotal, 0.0, C);
            V[i] = IloNumVarArray(env, numTotal, 0.0, C);
            x[i] = IloBoolVarArray(env, numTotal);
        }

        // Objective function [1]
        IloExpr totalCost(env);
        for (IloInt i: StationsCustomers_0){
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    totalCost += d[i][j] * x[i][j];
                }
            }
        }
        for (IloInt j: StationsCustomers_N1){
            totalCost += M * x[0][j];
        }
        model.add(IloMinimize(env, totalCost));
        totalCost.end();

        // Constraints
        for (IloInt i: Total) {
            model.add(x[i][Depot_0[0]] == 0); // No arcs to the depot_0
            model.add(x[Depot_N1[0]][i] == 0); // No arcs from the depot_N1
        }
        

        // Each customer is visited exactly once [2]
        for (IloInt i: Customers) {
            IloExpr customerVisitConstraint(env);
            for (IloInt j: StationsCustomers_N1) {
                if (i != j) {
                    customerVisitConstraint += x[i][j];
                }
            }
            model.add(customerVisitConstraint == 1);
            customerVisitConstraint.end();
        }

        // Each dummy stations is visited not more than once [3]

        for (IloInt i: Stations) {
            IloExpr stationVisitConstraint(env);
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    stationVisitConstraint += x[i][j];
                }
            }
            model.add(stationVisitConstraint <= 1);
            stationVisitConstraint.end();
        }

        // After a vehicle arrives at a customer it has to leave for another destination [4]

        for (IloInt j: StationsCustomers){
            IloExpr flowVisitConstraint(env);
            for (IloInt i: StationsCustomers_0) {
                if (i != j) {
                    flowVisitConstraint += x[i][j];
                }
            }
            for (IloInt i: StationsCustomers_N1) {
                if (i != j) {
                    flowVisitConstraint -= x[j][i];
                }
            }
            model.add(flowVisitConstraint == 0);
            flowVisitConstraint.end();
        }

        // Time feasibility for arcs leaving customers and depots [5]
        for (IloInt i: Customers_0){
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    model.add((t[i] + (Time[i][j] + s[i]) * x[i][j] - l[0] * (1 - x[i][j])) <= t[j]);
                }
            }
        }

        // Time feasibility for arcs leaving recharging and depots [6]
        for (IloInt i: Stations){
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    model.add((t[i] + Time[i][j] * x[i][j] + g * (Y[i] - y[i]) - (l[0] + (Q * g)) * (1 - x[i][j])) <= t[j]);
                }
            }

        }

        // Every vertex is visited within the Time window [7]
        for (IloInt j: Total){
            model.add(e[j] <= t[j]);
            model.add(t[j] <= l[j]);
        }


        // guarantee that the delivery and pickup demands of customers are satisfied simultaneously [8], [9], [10], [11], [12]
        for (IloInt j: StationsCustomers_N1){
            model.add(U[0][j] <= C);
            model.add(V[0][j] == 0);
        }

        for (IloInt j: StationsCustomers){
            IloExpr demandConstraint(env);
            for (IloInt i: StationsCustomers_0){
                if (i != j) {
                    demandConstraint += U[i][j];
                }
            }
            for (IloInt i: StationsCustomers_N1){
                if (i != j) {
                    demandConstraint -= U[j][i];
                }
            }
            model.add(demandConstraint == q[j]);
            demandConstraint.end();
        }

        for (IloInt j: StationsCustomers){
            IloExpr demandConstraint(env);
            for (IloInt i: StationsCustomers_N1){
                if (i != j) {
                    demandConstraint += V[j][i];
                }
            }
            for (IloInt i: StationsCustomers_0){
                if (i != j) {
                    demandConstraint -= V[i][j];
                }
            }
            model.add(demandConstraint == p[j]);
            demandConstraint.end();
        } 

        for (IloInt i: StationsCustomers_0){
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    model.add((U[i][j] + V[i][j]) <= (C * x[i][j]));
                }
            }
        }

        // battery constraints [13], [14], [15]
        for (IloInt i: Customers){
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    model.add(y[j] <= (y[i] - h * d[i][j] * x[i][j] + Q * (1 - x[i][j])));
                }
            }
        }

        for (IloInt i: Stations_0){
            for (IloInt j: StationsCustomers_N1){
                if (i != j) {
                    model.add(y[j] <= (Y[i] - h * d[i][j] * x[i][j] + Q * (1 - x[i][j])));
                }
            }
        }

        for (IloInt i: Stations_0){
            model.add(y[i] <= Y[i]);
            model.add(Y[i] <= Q);
        }

        // Solve the model
        IloCplex cplex(model);

        // cplex configuration:
        double time_limit = 5; 
        cplex.setOut(env.getNullStream()); // Suppress output
        // pass the time limit to CPLEX
        cplex.setParam(IloCplex::Param::TimeLimit, time_limit); // Time limit in seconds
        cplex.setParam(IloCplex::Param::Threads, 1); // Single thread

        double start_time = cplex.getTime();

        if (cplex.solve()) {
            // Display solution for x[i][j]
            env.out() << "Solution (matrix):" << std::endl;
            for (IloInt i : Total) {
                for (IloInt j : Total) {
                    if (i != j) {
                        env.out() << std::abs(cplex.getValue(x[i][j])) << " ";
                    } else {
                        env.out() << "0 "; // Diagonal or invalid entries
                    }
                }
                env.out() << std::endl; // Newline for the next row
            }
            // env.out() << "Solution (arc): " << std::endl;
            // for (IloInt i : StationsCustomers_0) {
            //     for (IloInt j : StationsCustomers_N1) {
            //         if (i != j && cplex.getValue(x[i][j]) > 0.5) { // Check if x[i][j] is selected
            //             env.out() << "Travel from " << i << " to " << j << std::endl;
            //         }
            //     }
            // }
            // Display results
            env.out() << "Solution status: " << cplex.getStatus() << std::endl;
            env.out() << "Total cost: " << cplex.getObjValue() << std::endl;
        } else {
            std::cout << "No solution found." << std::endl;
        }

        double end_time = cplex.getTime();
        double elapsed_time = end_time - start_time;

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Execution time: " << elapsed_time << " seconds" << std::endl;


    } catch (IloException& e) {
        env.out() << "Error: " << e.getMessage() << std::endl;
    } catch (...) {
        env.out() << "Unknown error" << std::endl;
    }

    env.end();
    return 0;
}

