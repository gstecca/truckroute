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
typedef std::tuple<int, int> t_odserv; // origin - destination service

struct s_cost {
	int c; //cost
	float t; //time
};


struct trdata {
	std::map<t_odserv, int> orders; // order
	std::map<t_arc,s_cost> arcs; // arc cost
	std::set<int> n; //nodes
	std::map<int, std::set<int>> fstar; //forward star
	std::map<int, std::set<int>> bstar; //backward star
	int C; //max vehicle capacity
	int k; //number of vehicles
	int F; // fixed cost for vehicle stop
	int source;
	int target;
	void insertstar(int i, int j){
		n.insert(i);
		n.insert(j);
		if (fstar.count(i) == 0)
			fstar[i] = std::set<int>();
		if (bstar.count(j) == 0)
			bstar[j] = std::set<int>();
		fstar[i].insert(j);
		bstar[j].insert(i);

	}
	std::string to_string(){
		std::string ss = "";
		ss += "C:" + std::to_string(C)  + "\t k:" + std::to_string(k) + "\t F:" + std::to_string(F) + "\t source: " + std::to_string(source);
		ss += "\n nodes: " + std::to_string(n.size());
		ss += "\n arcs: " + std::to_string(arcs.size());
		ss += "\n orders: " + std::to_string(orders.size()) + "\n";
		ss += "arcs:\n";
		ss += "from\tto\tcost\ttime\n";
		for (auto a : arcs){
			ss += std::to_string(std::get<0>(a.first)) + "\t" + std::to_string(std::get<1>(a.first))
					+ "\t" + std::to_string(a.second.c) + "\t" + std::to_string( a.second.t) + "\n" ;
		}
		ss += "orders:\n";
		ss += "from\tto\tcost\n";
		for (auto o : orders){
			ss += std::to_string(std::get<0>(o.first)) + "\t" + std::to_string(std::get<1>(o.first))
					+ "\t" + std::to_string(o.second) + "\n" ;
		}
		return ss;
	}

};

struct trparams{
	float maxtimetour;
	int maxz;
	int M; //bigM
	int MSEQ; // bigM for sequence constraint

};

int load_csv(trdata * dat, std::string filenamebase);
int buildmodel(IloModel* model, trdata* dat, trparams par);
int solvemodel(IloModel* model, trdata* dat, trparams par);
trparams fillparams(trdata *dat);





#endif /* INCLUDE_TRUCKROUTE_H_ */
