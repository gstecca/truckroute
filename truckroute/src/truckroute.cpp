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
#include <string>
#include <map>
using std::cout;
using std::cin;


IloNumVar buildVar(IloEnv &env,IloNumVar::Type numtype, int lb, int ub, std::string varname);
IloConstraint buildConstr(IloModel* model, IloExpr exprs, int rhs, std::string cname);
std::string getname (std::string baseName, int ind[], int indsize);

int main() {
	cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	trdata dat;
	std::string filenamebase = "input/instanzaNord1";
	load_csv(&dat, filenamebase);
	trparams par = fillparams(&dat);
	buildmodel(&model, &dat, par);
	cout << dat.to_string();
	return 0;
}

int load_csv(trdata * dat, std::string filenamebase)
//int load_data_tw(vector<int>* profits, vector<coord>* coords, int* n, int* m, float* tau, string filename)
{
	using std::string;
    /*
     * READING GENERAL
     */
	string fsheet = "general";
    string filename = filenamebase + "_" + fsheet + ".csv";
	cout << "reading " << filename << std::endl;
	std::ifstream myfile(filename);
    string line;
	string delimiter = ",";
	string token1;
	string token2;
	size_t pos;
	int startsub;
    if(myfile) {
        //* dat = new toptwdata()
        getline(myfile, line);

        for (int i = 0; i < 5; i++){
			getline(myfile, line);
			//cout << line << "\n";
			startsub = 0;
			pos = line.find(delimiter);
			token1 = line.substr(startsub, pos);
			startsub = pos + 1;
			pos = line.find(delimiter, startsub);
			token2 = line.substr(startsub, pos - startsub);
			if (token1 == "k")
				dat->k = stoi(token2);
			else if (token1 == "C")
				dat->C = stoi(token2);
			else if (token1 == "F")
				dat->F = stoi(token2);
			else if (token1 == "source")
				dat->source = stoi(token2);
			else if (token1 == "target")
				dat->target = stoi(token2);

			//cout << "READ " <<  token1 << "\n";
        }
        myfile.close();
    } else
        cout << "error opening file\n";


        /*
         * READING NETWORK
         */
        fsheet = "network";
        filename = filenamebase + "_" + fsheet + ".csv";
        cout << "reading " << filename << std::endl;
        std::ifstream myfile2(filename);
        if(myfile2){
			getline(myfile2, line); // skip first line
			while(getline(myfile2, line)) {
				startsub = 0;
				pos = line.find(delimiter);
				int from = stoi(line.substr(startsub, pos));
				startsub = pos + 1;
				pos = line.find(delimiter, startsub);
				int to = stoi(line.substr(startsub, pos - startsub));
				dat->insertstar(from, to);
				startsub = pos +1;
				pos = line.find(delimiter, startsub);
				int c = stoi(line.substr(startsub, pos - startsub));
				startsub = pos +1;
				pos = line.find(delimiter, startsub);
				float t = stof(line.substr(startsub, pos - startsub));
				//cout << from << "\t" << to << "\t"<< c << "\t" << t << "\n";
				t_arc arc = std::make_tuple(from, to);
				s_cost arc_cost = {c, t};
				dat->arcs[arc] = arc_cost;
			}
			myfile2.close();
        } else
            cout << "error opening file\n";
        /*
         * READING ORDERS
         */
        fsheet = "orders";
        filename = filenamebase + "_" + fsheet + ".csv";
        cout << "reading " << filename << std::endl;
        std::ifstream myfile3(filename);
        if (myfile3){
        	getline(myfile3, line); // skipping first line
        	while(getline(myfile3, line)){
        		startsub = 0;
        		pos = line.find(delimiter);
        		int from = stoi(line.substr(startsub, pos));
        		startsub = pos + 1;
        		pos = line.find(delimiter, startsub);
        		int to = stoi(line.substr(startsub, pos - startsub));
        		startsub = pos + 1;
        		pos = line.find(delimiter, startsub);
        		int d = stoi(line.substr(startsub, pos - startsub));
        		t_odserv od = std::make_tuple(from, to);
        		if (dat->orders[od])
        			dat->orders[od] = dat->orders[od] + d;
        		else
        			dat->orders[od] = d;
        		//cout << from << "\t" << to << "\t"<< d << "\n";
        	}
        }


    return 0;
}

int buildmodel(IloModel* model, trdata* dat, trparams par) {
	using std::string;
	using std::to_string;
	using std::get;
    std::map <std::string, IloNumVar> vars;
    std::map <std::string, IloConstraint> constrs;
	IloEnv env = model->getEnv();


    /*
     * CREATION OF VARIABLES: x, y, a, u, w, z
     */
    for(auto arc : dat->arcs){
    	for(int k = 0; k < dat->k; k++){
    		int ind[] = {get<0>(arc.first), get<1>(arc.first),k};
    		string name = getname ("x", ind, 3);
    		vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
    	}
    }

    for(auto order : dat->orders){
    	for(int k = 0; k < dat->k; k++){
    		int ind[] = {get<0>(order.first), get<1>(order.first), k};
    		string name = getname ("y", ind, 3);
    		vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
    	}

    }

    for(int i : dat->n){
    	for (int k=0; k < dat->k; k++){
    		int ind[] = {i,k};
    		string name = getname("a", ind, 2);
    		vars[name] = buildVar(env, IloNumVar::Float, 0, par.maxtimetour, name);
    	}
    }

    for(auto arc : dat->arcs) {
        for(auto order : dat->orders) {
        	int ind[] = {get<0>(arc.first), get<1>(arc.first), get<0>(order.first), get<1>(order.first)};
        	string name = getname("u", ind, 4);
            vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
        }
    }
    for(auto arc : dat->arcs) {
        for(auto order : dat->orders) {
        	for(int k = 0; k < dat->k; k++){
        		int ind[] = {get<0>(arc.first), get<1>(arc.first), get<0>(order.first), get<1>(order.first),k};
        		string name = getname("w", ind, 5);
        		vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
        	}
        }
    }
	for(int k = 0; k < dat->k; k++){
		int ind[] = {k};
		string name = getname("z", ind, 1);
		vars[name] = buildVar(env, IloNumVar::Int, 0, par.maxz, name);
	}

	/*
	 * CREATION OF CONSTRAINTS
	 */
    for(auto order : dat->orders){
    	int os = get<0>(order.first);
    	int ot = get<1>(order.first);
    	IloExpr exprs(env);
    	IloExpr exprt(env);
        for (auto to : dat->fstar[os]){
        	int ind[] = {os,to,os,ot};
        	std::string uname = getname("u", ind,4);
        	exprs += vars[uname];
        }
        int ind[] = {os,ot};
        std::string cname = getname("c1_orders", ind,2);
        constrs[cname] = buildConstr(model, exprs, 1, cname);
        exprs.end();
        for (auto from : dat->bstar[ot]){
        	int ind[] = {from, ot, os, ot};
        	std::string uname = getname("u", ind, 4);
        	exprt += vars[uname];
        }
        cname = getname("c2_ordert", ind,2);
        constrs[cname] = buildConstr(model, exprt, 1, cname);
        exprt.end();
    }

	return 1;
}


inline IloNumVar buildVar(IloEnv &env,IloNumVar::Type numtype, int lb, int ub, std::string varname){
	IloNumVar v(env, lb, ub, numtype, varname.c_str());
	return v;
}
IloConstraint buildConstr(IloModel* model, IloExpr exprs, int rhs, std::string cname){
    IloConstraint ic(exprs==rhs);
    ic.setName(cname.c_str());
    model->add(ic);
	return ic;
}
inline std::string getname (std::string baseName, int ind[], int indsize){
	using std::string;
	using std::to_string;
	string varname = "" + baseName;//"x#" + to_string(a) + "#" + to_string(b) + "#" + to_string(c);
	for (int i = 0; i < indsize; i++){
		varname += "#" + to_string(ind[i]);
	}
	return varname;
}
trparams fillparams(trdata * dat){
	trparams t;
	t.maxtimetour = 10000;
	t.maxz = 10;
	return t;
}
