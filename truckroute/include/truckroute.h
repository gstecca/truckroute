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
#include <tuple>
#include <set>


typedef std::tuple<int, int> t_arc;

struct s_cost {
	int c; //cost
	float t; //time
};


struct trdata {
	std::vector<int> o; // order
	std::vector<int> o_s; // order source
	std::vector<int> o_t; // order target
	std::map<int,int> order_qty;
	std::map<t_arc,s_cost> arcs; // arc cost
	std::map<int,int> t; //arc traveling time
	std::set<int> n; //nodes
	int C; //max vehicle capacity
	int k; //number of vehicles
	int F; // fixed cost for vehicle stop
	int source;
	int target;

};

int load_csv(trdata * dat, std::string filenamebase);





#endif /* INCLUDE_TRUCKROUTE_H_ */
