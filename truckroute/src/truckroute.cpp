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
IloConstraint buildConstr(IloModel* model, IloExpr exprs, int rhs, bool equal, std::string cname);
std::string getname (std::string baseName, int ind[], int indsize);
std::string getname (std::string baseName, int i, int j);
std::string getname (std::string baseName, int i, int j, int k);
std::string getname (std::string baseName, int i, int j, int k, int p);

int main() {
	cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	trdata dat;
	std::string filenamebase = "input/instanzaNord1";
	load_csv(&dat, filenamebase);
	cout << dat.to_string() << std::endl;
	trparams par = fillparams(&dat);
	int ret = buildmodel(&model, &dat, par);
	cout << "build model return " << ret << std::endl;
	//solvemodel(&model, &dat, par);
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
    for(auto const &arc : dat->arcs){
    	for(int k = 0; k < dat->k; k++){
    		int ind[] = {get<0>(arc.first), get<1>(arc.first),k};
    		string name = getname ("x", ind, 3);
    		vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
    	}
    }

    for(auto const &order : dat->orders){
    	for(int k = 0; k < dat->k; k++){
    		int ind[] = {get<0>(order.first), get<1>(order.first), k};
    		string name = getname ("y", ind, 3);
    		vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
    	}

    }

    for(auto const &i : dat->n){
    	for (int k=0; k < dat->k; k++){
    		int ind[] = {i,k};
    		string name = getname("a", ind, 2);
    		vars[name] = buildVar(env, IloNumVar::Float, 0, par.maxtimetour, name);
    	}
    }

    for(auto const &arc : dat->arcs) {
        for(auto order : dat->orders) {
        	int ind[] = {get<0>(arc.first), get<1>(arc.first), get<0>(order.first), get<1>(order.first)};
        	string name = getname("u", ind, 4);
            vars[name] = buildVar(env, IloNumVar::Bool, 0, 1, name);
        }
    }
    for(auto const &arc : dat->arcs) {
        for(auto const &order : dat->orders) {
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
    for(auto const &order : dat->orders){
    	int os = get<0>(order.first);
    	int ot = get<1>(order.first);
    	IloExpr exprs(env);
    	IloExpr exprt(env);
        for (auto const &to : dat->fstar[os]){
        	int ind[] = {os,to,os,ot};
        	std::string uname = getname("u", ind,4);
        	exprs += vars.at(uname);
        }
        int ind[] = {os,ot};
        std::string cname = getname("c1_orders", ind,2);
        constrs[cname] = buildConstr(model, exprs, 1, true, cname);
        exprs.end();
        for (auto const &from : dat->bstar[ot]){
        	int ind[] = {from, ot, os, ot};
        	std::string uname = getname("u", ind, 4);
        	exprt += vars.at(uname);
        }
        cname = getname("c2_ordert", ind,2);
        constrs[cname] = buildConstr(model, exprt, 1, true, cname);
        exprt.end();
        for(auto const &i : dat->n){
        	if (i == os || i == ot)
        		continue;
        	IloExpr expr(env);
        	for(auto const &j : dat->bstar[i]){
        		int ind[] = {j,i,os,ot};
        		expr += vars.at(getname("u", ind, 4));
        	}
        	for(auto const &j : dat->fstar[i]){
        		int ind[] = {i,j,os,ot};
        		expr -= vars.at(getname("u", ind, 4));
        	}
            int ind[] = {os, ot, i};
            cname = getname("c3_ordflow", ind, 2);
            constrs[cname] = buildConstr(model, expr, 0, true, cname);
            expr.end();
        }

        for (auto const &arc : dat->arcs){
    		int i = get<0>(arc.first);
    		int j = get<1>(arc.first);
        	for(int k = 0; k < dat->k; k++){
        		IloExpr expr(env);
        		std::string x = getname("x", i, j, k);
        		std::string u = getname("u", i,j,os,ot);
        		std::string y = getname("y", os,ot,k);

        		//expr += vars[u] ;
        		//expr+= par.M*vars[y];
				//expr += (-1)*vars[x];
				expr = vars.at(u) + par.M*vars.at(y) - vars.at(x);
        		int ind[] = {os,ot,i,j,k};
        		int indvar[] = {i,j,os,ot,k};
        		cname = getname("c4_x_u", ind, 5);
        		constrs[cname] = buildConstr(model, expr, par.M, false, cname);
        		expr.end();

        		IloExpr expr5(env);
        		std::string w = getname("w", indvar, 5);
        		expr5 += vars.at(w) - vars.at(u);
        		cname = getname ("c5_w_u", ind, 5);
        		constrs[cname] = buildConstr(model, expr5, 0, false, cname);
        		expr5.end();

        		IloExpr expr6(env);
        		expr6 += vars.at(w) - vars.at(y);
        		cname = getname ("c6_w_y", ind, 5);
        		constrs[cname] = buildConstr(model, expr6, 0, false, cname);
        		expr6.end();

        		IloExpr expr7(env);
        		expr7 += vars.at(u);
        		expr7 += vars.at(y);
				expr7 += -1*vars.at(w);
        		cname = getname ("c7_w_u_y", ind, 5);
        		constrs[cname] = buildConstr(model, expr7, 1, false, cname);
        		expr7.end();

        	}
        }
        IloExpr expr(env);
        for (int k = 0; k < dat->k; k++){
        	auto vy = vars.at(getname("y", os, ot, k));
        	expr += vy;
        }
        cname = getname("c8_y", os,ot);
        constrs[cname] = buildConstr(model, expr, 1, true, cname);
        expr.end();



    }
    cout << "debug" << std::endl;

    for (int k = 0; k < dat->k; k++){
        IloExpr exprxs(env);
    	//debug
        for(auto const &j : dat->fstar[dat->source]){
    		cout << j << std::endl;
    		std::string xs = getname("x", dat->source, j, k);
    		cout << vars.count(xs) << std::endl;
    	}

    	for(auto const &j : dat->fstar[dat->source]){
    		cout << j << std::endl;
    		std::string xs = getname("x", dat->source, j, k);
    		auto vxs = vars.at(xs);
    		exprxs += vxs;
    	}
    	int ind[] = {k};
    	std::string cname = getname("c9_ksource", ind, 1);
    	constrs[cname] = buildConstr(model, exprxs, 1, true, cname);
    	exprxs.end();
    	cout << "source " << dat->source << std::endl;
    	cout << "target " << dat->target << std::endl;

    	// debug
    	for(auto const &i : dat->bstar.at(dat->target)){
    		cout << i << std::endl;
    		std::string xt = getname("x", i, dat->target, k);
    		cout << vars.count(xt) << std::endl;
    	}
    	IloExpr exprxt(env);
    	int target = dat->target;
    	cout << target << std::endl;
    	for(auto const& i : dat->bstar.at(target)){
    		std::string xt = getname("x", i, dat->target, k);
    		auto vxt = vars.at(xt);
    		exprxt += vxt;
    	}


    	cname = getname("c10_ktarget", ind, 1);
    	constrs[cname] = buildConstr(model, exprxt, 1, true, cname);

    	exprxt.end();

    	for (auto const &i : dat->n){
            IloExpr exprxf(env);
    		if(i == dat->source || i == dat->target)
    			continue;
    		for (auto const &j : dat->bstar[i])
    			exprxf += vars[getname("x",j,i)];
    		for (auto const &j : dat->fstar[i])
    			exprxf -= vars[getname("x",i,j)];
        	std::string cname = getname("c11_xkflow", k,i);
        	constrs[cname] = buildConstr(model, exprxf, 0, true, cname);
        	exprxf.end();
    	}

    	for (auto const &arc : dat->arcs){
    		IloExpr exprd(env);
    		IloExpr exprseq(env);
    		int i = get<0>(arc.first);
    		int j = get<1>(arc.first);
    		for (auto const &order : dat->orders){
    			int os = get<0>(order.first);
    			int ot = get<1>(order.first);
    			int d = order.second;
    			int ind[] = {i,j,os,ot,k};
    			std::string w = getname("w", ind, 5);
    			exprd += d*vars[w];

    		}
			std::string x = getname("x", i,j,k);
			std::string aik = getname("a",i,k);
			std::string ajk = getname("a",j,k);
			exprseq += vars[aik] - vars[ajk] + par.MSEQ*vars[x];

			std::string cname = getname("c12_demand", i,j,k);
			constrs[cname] = buildConstr(model, exprd, dat->C, false, cname);
			cname = getname("c13_sequence", i,j,k);
			int rhs = par.MSEQ - arc.second.t;
			constrs[cname] = buildConstr(model, exprd, rhs, false, cname);
    	}
    }
    	/*
    	 *
    	 * OBJECTIVE FUNCTION
    	 * model.add(IloMaximize(env, obj));
      obj.end();
    	 *
    	 */

    	IloExpr obj(env);
    	for(int k = 0; k < dat->k; k++){
    		for (auto const &arc : dat->arcs){
    			int i = get<0>(arc.first);
    			int j = get<1>(arc.first);
    			obj += arc.second.c * vars[getname("x", i, j, k)];
    		}
    		int zind[] = {k};
    		std::string zname = getname("z", zind,1);
    		obj += dat->F * vars[zname];
    	}
    	model->add(IloMinimize(env, obj));
    	obj.end();


	return 0;
}

int solvemodel(IloModel* model, trdata* dat, trparams par){
	IloCplex cplex(*model);
	cplex.exportModel("model.lp");
	cplex.solve();
	cplex.out() << "Solution status: " << cplex.getStatus() << std::endl;
	cplex.out() << " Solution value: " << cplex.getObjValue () << std::endl;
	cplex.writeSolution("solution.sol");
	return 0;
}

inline IloNumVar buildVar(IloEnv &env,IloNumVar::Type numtype, int lb, int ub, std::string varname){
	IloNumVar v(env, lb, ub, numtype, varname.c_str());
	return v;
}
IloConstraint buildConstr(IloModel* model, IloExpr exprs, int rhs, bool equal, std::string cname){
	if(equal){
		IloConstraint ic(exprs==rhs);
		ic.setName(cname.c_str());
		model->add(ic);
		return ic;
	}
	else{
		IloConstraint ic(exprs<=rhs);
	    ic.setName(cname.c_str());
	    model->add(ic);
		return ic;
	}

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
inline std::string getname (std::string baseName, int i, int j){
	int ind[] = {i,j};
	return getname(baseName, ind, 2);
}
inline std::string getname(std::string baseName, int i, int j, int k){
	int ind[] = {i, j, k};
	return getname(baseName, ind, 3);
}
inline std::string getname(std::string baseName, int i, int j, int k, int p){
	int ind[] = {i, j, k, p};
	return getname(baseName, ind, 4);
}
trparams fillparams(trdata * dat){
	trparams t;
	t.maxtimetour = 10000;
	t.maxz = 10;
	t.M = 1000000;
	t.MSEQ = 1000;
	return t;
}
