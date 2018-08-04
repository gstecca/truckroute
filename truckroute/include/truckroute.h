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


struct mdata {
	std::vector<int> o; // order
	std::vector<int> o_s; // order source
	std::vector<int> o_t; // order target
	std::map<int,int> order_qty;
	std::map<int,int> c; // arc cost
	std::map<int,int> t; //arc traveling time
	std::vector<int> n;
	int C; //max vehicle capacity
	int k; //number of vehicles
	int source;
	int target;

};





#endif /* INCLUDE_TRUCKROUTE_H_ */
