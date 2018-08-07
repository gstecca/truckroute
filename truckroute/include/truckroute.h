/*
 * mydata.h
 *
 *  Created on: Aug 3, 2018
 *      Author: gstecca
 */

#ifndef INCLUDE_TRUCKROUTE_H_
#define INCLUDE_TRUCKROUTE_H_

#include <vector>
#include <map>
#include <string>



struct order {
	int from;
	int to;
	int qty;
};


struct trdata {
	std::vector<int> o; // order
	std::vector<int> o_s; // order source
	std::vector<int> o_t; // order target
	std::map<int,int> order_qty;
	std::map<int,int> c; // arc cost
	std::map<int,int> t; //arc traveling time
	std::vector<int> n;
	int C; //max vehicle capacity
	int k; //number of vehicles
	int F; // fixed cost for vehicle stop
	int source;
	int target;

};

int load_csv(trdata * dat, std::string filenamebase);





#endif /* INCLUDE_TRUCKROUTE_H_ */
