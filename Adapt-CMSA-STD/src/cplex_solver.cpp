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
#include "cplex_solver.h"
void ILPmodel(Solution& s, double& t_solve, Data& data, double time_limit, const std::vector<std::vector<int>>& adjMatrix, bool& optimal){

    s.cost = double(INFINITY);

    IloEnv env;
    IloInt numTotal = data.cplex_data.numTotal;
    try {
        // Decision variables
        IloModel model(env);

        IloNumVarArray t(env, numTotal, data.start_time, data.end_time); 
        IloArray<IloNumVarArray> U(env, numTotal);
        IloArray<IloNumVarArray> V(env, numTotal);
        IloNumVarArray y(env, numTotal, 0.0, data.cplex_data.Q);
        IloNumVarArray Y(env, numTotal, 0.0, data.cplex_data.Q);
        IloArray<IloBoolVarArray> x(env, numTotal);


        for (IloInt i: data.cplex_data.Total) {
            U[i] = IloNumVarArray(env, numTotal, 0.0, data.cplex_data.C);
            V[i] = IloNumVarArray(env, numTotal, 0.0, data.cplex_data.C);
            x[i] = IloBoolVarArray(env, numTotal);
        }

        // Objective function [1]
        IloExpr totalCost(env);
        for (IloInt i: data.cplex_data.StationsCustomers_0){
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    totalCost += data.cplex_data.u_1 * data.cplex_data.d[i][j] * x[i][j];
                }
            }
        }
        for (IloInt j: data.cplex_data.StationsCustomers_N1){
            totalCost += data.cplex_data.u_2 * x[0][j];
        }
        model.add(IloMinimize(env, totalCost));
        totalCost.end();

        // Constraints
        for (IloInt i: data.cplex_data.Total) {
            model.add(x[i][data.cplex_data.Depot_0[0]] == 0); // No arcs to the depot_0
            model.add(x[data.cplex_data.Depot_N1[0]][i] == 0); // No arcs from the depot_N1
        }
        
        if (!adjMatrix.empty()) {
            IloInt rows = adjMatrix.size();
            IloInt cols = adjMatrix[0].size(); 
            for (IloInt i = 0; i < rows; i++) {
                for (IloInt j = 1; j < cols; j++) {
                    if (i != j && adjMatrix[i][j] == 0) {
                        model.add(x[i][j] == 0);
                    }
                }
            }
            for (IloInt i = 0; i < rows; i++) {
                    if (adjMatrix[i][0] == 0) {
                        model.add(x[i][cols] == 0);
                    }
                
            }
        } else {
            std::cout << "adjMatrix is empty!" << std::endl;
        }
        

        // Each customer is visited exactly once [2]
        for (IloInt i: data.cplex_data.Customers) {
            IloExpr customerVisitConstraint(env);
            for (IloInt j: data.cplex_data.StationsCustomers_N1) {
                if (i != j) {
                    customerVisitConstraint += x[i][j];
                }
            }
            model.add(customerVisitConstraint == 1);
            customerVisitConstraint.end();
        }

        // Each dummy stations is visited not more than once [3]

        for (IloInt i: data.cplex_data.Stations) {
            IloExpr stationVisitConstraint(env);
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    stationVisitConstraint += x[i][j];
                }
            }
            model.add(stationVisitConstraint <= 1);
            stationVisitConstraint.end();
        }

        // After a vehicle arrives at a customer it has to leave for another destination [4]

        for (IloInt j: data.cplex_data.StationsCustomers){
            IloExpr flowVisitConstraint(env);
            for (IloInt i: data.cplex_data.StationsCustomers_0) {
                if (i != j) {
                    flowVisitConstraint += x[i][j];
                }
            }
            for (IloInt i: data.cplex_data.StationsCustomers_N1) {
                if (i != j) {
                    flowVisitConstraint -= x[j][i];
                }
            }
            model.add(flowVisitConstraint == 0);
            flowVisitConstraint.end();
        }

        // Time feasibility for arcs leaving customers and depots [5]
        for (IloInt i: data.cplex_data.Customers_0){
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    model.add((t[i] + (data.cplex_data.Time[i][j] + data.cplex_data.s[i]) * x[i][j] - data.cplex_data.l[0] * (1 - x[i][j])) <= t[j]);
                }
            }
        }

        // Time feasibility for arcs leaving recharging and depots [6]
        for (IloInt i: data.cplex_data.Stations){
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    model.add((t[i] + data.cplex_data.Time[i][j] * x[i][j] + data.cplex_data.g * (Y[i] - y[i]) - (data.cplex_data.l[0] + (data.cplex_data.Q * data.cplex_data.g)) * (1 - x[i][j])) <= t[j]);
                }
            }

        }

        // Every vertex is visited within the Time window [7]
        for (IloInt j: data.cplex_data.Total){
            model.add(data.cplex_data.e[j] <= t[j]);
            model.add(t[j] <= data.cplex_data.l[j]);
        }


        // guarantee that the delivery and pickup demands of customers are satisfied simultaneously [8], [9], [10], [11], [12]
        for (IloInt j: data.cplex_data.StationsCustomers_N1){
            model.add(U[0][j] <= data.cplex_data.C);
            model.add(V[0][j] == 0);
        }

        for (IloInt j: data.cplex_data.StationsCustomers){
            IloExpr demandConstraint(env);
            for (IloInt i: data.cplex_data.StationsCustomers_0){
                if (i != j) {
                    demandConstraint += U[i][j];
                }
            }
            for (IloInt i: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    demandConstraint -= U[j][i];
                }
            }
            model.add(demandConstraint == data.cplex_data.q[j]);
            demandConstraint.end();
        }

        for (IloInt j: data.cplex_data.StationsCustomers){
            IloExpr demandConstraint(env);
            for (IloInt i: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    demandConstraint += V[j][i];
                }
            }
            for (IloInt i: data.cplex_data.StationsCustomers_0){
                if (i != j) {
                    demandConstraint -= V[i][j];
                }
            }
            model.add(demandConstraint == data.cplex_data.p[j]);
            demandConstraint.end();
        } 

        for (IloInt i: data.cplex_data.StationsCustomers_0){
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    model.add((U[i][j] + V[i][j]) <= (data.cplex_data.C * x[i][j]));
                }
            }
        }

        // battery constraints [13], [14], [15]
        for (IloInt i: data.cplex_data.Customers){
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    model.add(y[j] <= (y[i] - data.cplex_data.h * data.cplex_data.d[i][j] * x[i][j] + data.cplex_data.Q * (1 - x[i][j])));
                }
            }
        }

        for (IloInt i: data.cplex_data.Stations_0){
            for (IloInt j: data.cplex_data.StationsCustomers_N1){
                if (i != j) {
                    model.add(y[j] <= (Y[i] - data.cplex_data.h * data.cplex_data.d[i][j] * x[i][j] + data.cplex_data.Q * (1 - x[i][j])));
                }
            }
        }

        for (IloInt i: data.cplex_data.Stations_0){
            model.add(y[i] <= Y[i]);
            model.add(Y[i] <= data.cplex_data.Q);
        }

        // Solve the model
        IloCplex cplex(model);

        // cplex configuration:
        cplex.setOut(env.getNullStream()); // Suppress output
        // pass the time limit to CPLEX
        cplex.setParam(IloCplex::Param::TimeLimit, time_limit); // Time limit in seconds
        cplex.setParam(IloCplex::Param::Threads, 1); // Single thread
        cplex.setParam(IloCplex::Param::MIP::Tolerances::AbsMIPGap, 0.001);

        double start_time = cplex.getTime();

        if (cplex.solve()) {
            // Display solution for x[i][j]
            std::vector<std::vector<int>> solMatrix(numTotal, std::vector<int>(numTotal, 0));
            std::vector<int> next(numTotal, numTotal-1);
            // env.out() << "Solution (matrix):" << std::endl;
            for (IloInt i : data.cplex_data.Total) {
                for (IloInt j : data.cplex_data.Total) {
                    if (i != j) {
                        // env.out() << (cplex.getValue(x[i][j]) > 0.5) << " ";
                        if (cplex.getValue(x[i][j]) > 0.5) {
                            solMatrix[i][j] = 1;
                            next[i] = j;
                        }
                    } 
                    // else {
                    //     env.out() << "0 "; // Diagonal or invalid entries
                    // }
                }
                // env.out() << std::endl; // Newline for the next row
            }
            // env.out() << "Solution (arc): " << std::endl;
            // for (IloInt i : data.cplex_data.Total) {
            //     for (IloInt j : data.cplex_data.Total) {
            //         if (i != j && cplex.getValue(x[i][j]) > 0.5) { // Check if x[i][j] is selected
            //             env.out() << "Travel from " << i << " to " << j << std::endl;
            //         }
            //     }
            // }

            // Display results
            env.out() << "Solution status: " << cplex.getStatus() << std::endl;
            env.out() << "Total cost: " << cplex.getObjValue() << std::endl;

            optimal = (cplex.getCplexStatus() == IloCplex::Optimal);

            s.routeListRepresentation(solMatrix, next, data);

        } else {
            std::cout << "No solution found." << std::endl;
        }

        double end_time = cplex.getTime();
        double elapsed_time = end_time - start_time;

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Execution time: " << elapsed_time << " seconds" << std::endl;

        t_solve = elapsed_time;


    } catch (IloException& e) {
        env.out() << "Error: " << e.getMessage() << std::endl;
    } catch (...) {
        env.out() << "Unknown error" << std::endl;
    }

    env.end();
}