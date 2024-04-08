/*********************************************
 * OPL 20.1.0.0 Model
 * Author: lenovo
 * Creation Date: 2024年4月3日 at 上午11:48:49
 *********************************************/
/*****************************************************************************
 *
 * SETS
 *
 *****************************************************************************/
{int} Depot_0 = ...;
{int} Depot_N1 = ...;

{int} Stations = ...;
{int} Stations_0 = Depot_0 union Stations;

{int} Customers = ...;
{int} Customers_0 = Depot_0 union Customers;

{int} StationsCustomers =  Stations union Customers;
{int} StationsCustomers_0 = Depot_0 union Stations union Customers; 
{int} StationsCustomers_N1 = Stations union Customers union Depot_N1;

{int} Total = Depot_0 union Stations union Customers union Depot_N1;

/*****************************************************************************
 *
 * PARAMETERS
 *
 *****************************************************************************/

//Travel time
float v = ...; // Average vehicle's velocity [km/h]
float Time[Total][Total]; // Cost or distance between i and j

// Distance
float   XCoord[Total] = ...;
float   YCoord[Total] = ...;
float d[Total][Total]; // Cost or distance between i and j

// Capacity
float C = ...;

// Battery
float Q = ...;

// Charging and discharging rate
float g = ...; // recharging rate
float h = ...; // charge consumption rate

// Demand
float q[Total] = ...; //delivery
float p[Total] = ...; //pickup

// Time windows
float e[Total] = ...; // Lower Bound of the Time Window
float l[Total] = ...; // Upper Bound of the Time Window
float s[Total] = ...;
int M=1000;

execute INITIALIZE {
	for(var i in Total) {
		for (var j in Total){
			if (i == j) {
				d[i][j] = 0;
				Time[i][j] = 0;
			} else {
			    d[i][j] = Math.sqrt(Math.pow(XCoord[i]-XCoord[j], 2) + Math.pow(YCoord[i]-YCoord[j], 2));
			    //d[i][j] = Math.floor(Math.sqrt(Math.pow(XCoord[i]-XCoord[j], 2) + Math.pow(YCoord[i]-YCoord[j], 2))*10)/10;
		        Time[i][j] = d[i][j]/v;
	       	}
	     }
     }
}
/*****************************************************************************
 *
 * Decision variables and objective function
 *
 *****************************************************************************/

dvar float+ t[Total]; // time arrival at vertex i
dvar float+ U[StationsCustomers_0][StationsCustomers_N1]; //  the remaining cargo to be delivered to customers of the route
dvar float+ V[StationsCustomers_0][StationsCustomers_N1]; //  the amount of cargo already collected (picked up) at already visited customers
dvar float+ y[Total]; // remaining battery at vertex i on arrival
dvar float+ Y[Total]; // remaining battery at vertex i on departure
dvar boolean x[StationsCustomers_0][StationsCustomers_N1]; // 1 if a vehicle drives directly from vertex i to vertex j

    
// Objective function [1]

minimize (sum(i in StationsCustomers_0, j in StationsCustomers_N1 : i != j) (d[i][j]*x[i][j])) + (sum(j in StationsCustomers_N1) x[0][j]*M);


/*****************************************************************************
 *
 * Constraints
 *
 *****************************************************************************/

subject to {

   	// Each customer is visited exactly once [2]
	forall (i in Customers)
		sum(j in StationsCustomers_N1 : i != j) x[i][j] == 1;
		
   	// Each dummy stations is visited not more than once [3]
	forall (i in Stations)
		sum(j in StationsCustomers_N1 : i != j) x[i][j] <= 1;

   	// After a vehicle arrives at a customer it has to leave for another destination [4]
   	forall(j in StationsCustomers)
     	sum(i in StationsCustomers_0 : i != j) x[i][j] - sum(i in StationsCustomers_N1 : i != j) x[j][i] == 0;
    
    // time feasibility for arcs leaving customers and depots [5]
    forall(i in Customers_0, j in StationsCustomers_N1 : i != j)
      t[i] + (Time[i][j] + s[i])*x[i][j] - l[0]*(1-x[i][j]) <= t[j];

    // time feasibility for arcs leaving recharging and depots [6]
    forall(i in Stations, j in StationsCustomers_N1 : i != j)
      t[i] + Time[i][j]*x[i][j] + g*(Y[i]-y[i]) - (l[0]+(Q*g))*(1-x[i][j]) <= t[j];
      
    // Every vertex is visited within the time window [7]
    forall(j in Total)
      e[j] <= t[j] <= l[j];
 
    // guarantee that the delivery and pickup demands of customers are satisfied simultaneously [8], [9], [10], [11], [12]
    forall(j in StationsCustomers_N1){    
      U[0][j]<=C;
      V[0][j] == 0;
    }    
     
    forall(j in StationsCustomers)
       sum(i in StationsCustomers_0 : i != j) U[i][j] - sum(i in StationsCustomers_N1 : i != j) U[j][i] == q[j];

    forall(j in StationsCustomers)
       sum(i in StationsCustomers_N1 : i != j) V[j][i] - sum(i in StationsCustomers_0 : i != j) V[i][j] == p[j]; 
       
    forall(i in StationsCustomers_0, j in StationsCustomers_N1: i != j)
       U[i][j] + V[i][j] <= C*x[i][j];     
     
	// battery constraints [13], [14], [15]
    forall(i in Customers, j in StationsCustomers_N1 : i != j)
      y[j] <= y[i] - (h*d[i][j])*x[i][j] + Q*(1-x[i][j]);
      
    forall(i in Stations_0, j in StationsCustomers_N1 : i != j)
      y[j] <= Y[i] - (h*d[i][j])*x[i][j] + Q*(1-x[i][j]);
	
    forall(i in Stations_0) {
   	  y[i] <= Y[i];
      Y[i] <= Q;
     }

};

execute DISPLAY {
    writeln("Solutions: ");
		for(var i in StationsCustomers_0)
			for (var j in StationsCustomers_N1)	
				if(x[i][j] == 1)
					writeln("Travel from ", i, " to ", j);					
}