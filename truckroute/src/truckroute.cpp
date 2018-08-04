//============================================================================
// Name        : truckroute.cpp
// Author      : gstecca
// Version     :
// Copyright   : no copyright here
// Description : Hello World in C++
//============================================================================

#include <iostream>
#include <ilcplex/ilocplex.h>
#include <truckroute.h>
using namespace std;

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	mdata data;
	data.C = 33;
	data.k = 7;

	cout << "problem has to be solved for k=" << data.k << " vehicles\n";
	return 0;
}
