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
#include <readfile.h>

struct TRCplexSol{
	IloEnv env;
	IloModel model;
	IloCplex cplex;
    std::map <std::string, IloNumVar> vars;
    std::map <std::string, IloConstraint> constrs;
    TRCplexSol(IloEnv _env, IloModel _model){
    	model = _model;
    	env = _env;
    }
};


IloNumVar buildVar(IloEnv &env,IloNumVar::Type numtype, int lb, int ub, std::string varname);
IloConstraint buildConstr(IloModel* model, IloExpr exprs, int rhs, bool equal, std::string cname);
int buildmodel(TRCplexSol* sol, trdata* dat, trparams par);
int buildmodelLoad(TRCplexSol* sol, trdata* dat, trparams par);
IloCplex solvemodel(TRCplexSol* sol, trdata* dat, trparams par);
t_orders getDemandSourceAt(int i, trdata* dat);
t_orders getDemandTargetAt(int i, trdata* dat);
std::string getname (std::string baseName, int ind[], int indsize);
std::string getname (std::string baseName, int i);
std::string getname (std::string baseName, int i, int j);
std::string getname (std::string baseName, int i, int j, int k);
std::string getname (std::string baseName, int i, int j, int k, int p);



bool debug = false;



int load_csv(trdata * dat, std::string filenamebase)
//int load_data_tw(vector<int>* profits, vector<coord>* coords, int* n, int* m, float* tau, string filename)
{
	using std::string;
    /*
     * READING GENERAL
     */
	string fsheet = "general";
    string filename = filenamebase + "_" + fsheet + ".csv";
    std::cout << "reading " << filename << std::endl;
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

        for (int i = 0; i < 7; i++){
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
			else if (token1 == "H")
				dat->H = stof(token2);
			else if (token1 == "source")
				dat->source = stoi(token2);
			else if (token1 == "target")
				dat->target = stoi(token2);
			else if (token1 == "FS")
				dat->FS = stoi(token2);

			//cout << "READ " <<  token1 << "\n";
        }
        myfile.close();
    } else{
    	std::cout << "error opening file\n";
    }


        /*
         * READING NETWORK
         */
	fsheet = "network";
	filename = filenamebase + "_" + fsheet + ".csv";
	std::cout << "reading " << filename << std::endl;
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
			startsub = pos +1;
			pos = line.find(delimiter, startsub);
			int c = stoi(line.substr(startsub, pos - startsub));
			startsub = pos +1;
			pos = line.find(delimiter, startsub);
			float t = stof(line.substr(startsub, pos - startsub));
			//cout << from << "\t" << to << "\t"<< c << "\t" << t << "\n";
			dat->insertstar(from, to);
			t_arc arc = std::make_tuple(from, to);
			s_cost arc_cost = {c, t};
			dat->arcs[arc] = arc_cost;
		}
		myfile2.close();
	} else
		std::cout << "error opening file\n";
	/*
	 * READING ORDERS
	 */
	fsheet = "orders";
	filename = filenamebase + "_" + fsheet + ".csv";
	std::cout << "reading " << filename << std::endl;
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
/*
 * add a new source (node n+1) and a new target (node n+2)
 * arcs from the new source will have 0 travellilng time and FS_i cost
 * while arcs towards the new target will have 0 travelling time and 0 cost
 */
int addServices(trdata *dat, trparams& par){
	int mymax = *dat->n.rbegin();
	int newSource = mymax + 1;
	int newTarget = mymax +2;
	int oldSource = dat->source;
	dat->source = newSource;
	dat->target = newTarget;
	for(auto const& i : dat->n){
		int from = dat->source;
		float t = 0;
		int c = dat->FS;
		if (i == oldSource)
			c = 0;
		dat->insertstar(from, i);
		t_arc arc = std::make_tuple(from, i);
		s_cost arc_cost = {c, t};
		dat->arcs[arc] = arc_cost;

		c = 0;
		int to = dat->target;
		dat->insertstar(i, to);
		t_arc arc2 = std::make_tuple(i, to);
		s_cost arc2_cost = {c,t};
		dat->arcs[arc2] = arc2_cost;
	}


	return 0;
}
int buildmodel(TRCplexSol* sol, trdata* dat, trparams par) {
	using std::string;
	using std::to_string;
	using std::get;
    std::map <std::string, IloNumVar> vars;
    std::map <std::string, IloConstraint> constrs;
	IloModel* model = &(sol->model);
    IloEnv env = sol->env;



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
        for (auto const &to : dat->fstar.at(os)){
        	int ind[] = {os,to,os,ot};
        	std::string uname = getname("u", ind,4);
        	exprs += vars.at(uname);
        }
        int ind[] = {os,ot};
        std::string cname = getname("c1_orders", ind,2);
        constrs[cname] = buildConstr(model, exprs, 1, true, cname);
        exprs.end();
        for (auto const &from : dat->bstar.at(ot)){
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
        	for(auto const &j : dat->bstar.at(i)){
        		int ind[] = {j,i,os,ot};
        		expr += vars.at(getname("u", ind, 4));
        	}
        	for(auto const &j : dat->fstar.at(i)){
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
        		IloExpr expr4(env);
        		std::string x = getname("x", i, j, k);
        		std::string u = getname("u", i,j,os,ot);
        		std::string y = getname("y", os,ot,k);

				//expr4 = vars.at(u) + par.M*vars.at(y) - vars.at(x);
        		expr4 = vars.at(u) + vars.at(y) - vars.at(x);
        		int ind[] = {os,ot,i,j,k};
        		int indvar[] = {i,j,os,ot,k};
        		cname = getname("c4_x_u", ind, 5);
        		//constrs[cname] = buildConstr(model, expr4, par.M, false, cname);
        		constrs[cname] = buildConstr(model, expr4, 1, false, cname);
        		expr4.end();

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
        IloExpr expr8(env);
        for (int k = 0; k < dat->k; k++){
        	auto vy = vars.at(getname("y", os, ot, k));
        	expr8 += vy;
        }
        cname = getname("c8_y", os,ot);
        constrs[cname] = buildConstr(model, expr8, 1, true, cname);
        expr8.end();
    }
    int nvel = dat->k;

    for (int k = 0; k < nvel; k++){
        IloExpr exprxs(env);
    	for(auto const &j : dat->fstar.at(dat->source)){
    		std::string xs = getname("x", dat->source, j, k);
    		auto vxs = vars.at(xs);
    		exprxs += vxs;
    	}
    	int ind[] = {k};
    	std::string cname = getname("c9_ksource", ind, 1);
    	constrs[cname] = buildConstr(model, exprxs, 1, true, cname);
    	exprxs.end();
    }


    for(int k=0; k < dat->k; k++){
    	// debug
    	for(auto const &ii : dat->bstar.at(dat->target)){
    		std::string xt = getname("x", ii, dat->target, k);
    	}
    	IloExpr exprxt(env);
    	int target = dat->target;
    	for(auto const& i : dat->bstar.at(target)){
    		std::string xt = getname("x", i, dat->target, k);
    		auto vxt = vars.at(xt);
    		exprxt += vxt;
    	}
    	int ind[] = {k};
    	std::string cname = getname("c10_ktarget", ind, 1);
    	constrs[cname] = buildConstr(model, exprxt, 1, true, cname);
    	exprxt.end();
    }

    for(int k = 0; k < nvel; k++){

    	for (auto const &i : dat->n){
            IloExpr exprxf(env);
    		if(i == dat->source || i == dat->target)
    			continue;
    		for (auto const &j : dat->bstar.at(i)){
    			//std::cout << "arc j->i:"  << j << "->" << i << std::endl; //problem for arc 0->2
    			exprxf += vars.at(getname("x",j,i,k));
    		}
    		for (auto const &j : dat->fstar.at(i))
    			exprxf -= vars.at(getname("x",i,j,k));
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
    			exprd += d*vars.at(w);

    		}
			std::string x = getname("x", i,j,k);
			std::string aik = getname("a",i,k);
			std::string ajk = getname("a",j,k);
			exprseq += vars.at(aik) - vars.at(ajk) + par.MSEQ*vars.at(x);

			std::string cname = getname("c12_demand", i,j,k);
			constrs[cname] = buildConstr(model, exprd, dat->C, false, cname);
			cname = getname("c13_sequence", i,j,k);
			int rhs = par.MSEQ - arc.second.t;
			if (par.subTourElimination)
				constrs[cname] = buildConstr(model, exprseq, rhs, false, cname);
			exprd.end();
			exprseq.end();
    	}

    	IloExpr exprz(env);
    	exprz += vars.at(getname("a", dat->target,k))/dat->H;
    	exprz -= vars.at(getname("z",k));
    	constrs[getname("c14z_",k)] = buildConstr(model, exprz, 1, false, getname("c14z_",k));
    }

    /**
     * c19_constraint orders precedence
     */

    for (auto const& order : dat->orders){
    	for(int k = 0; k < dat->k; k++){
    		IloExpr exprop(env);
			int i = get<0>(order.first);
			int j = get<1>(order.first);
			exprop += vars.at(getname("a", i, k)) - vars.at(getname("a", j, k))
					+ par.MSEQ * vars.at(getname("y",i,j,k));
			constrs[getname("c19_oprec_",i, j, k)] =
					buildConstr(model, exprop, par.MSEQ, false, getname("c19_oprec_",i, j, k));
			exprop.end();
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
			obj += arc.second.c * vars.at(getname("x", i, j, k));
		}
		int zind[] = {k};
		std::string zname = getname("z", zind,1);
		obj += dat->F * vars.at(zname);
	}
	model->add(IloMinimize(env, obj));
	obj.end();

	std::cout << "n. of variables created: " <<  vars.size() << std::endl;
	std::cout << "n. of constraints created: " << constrs.size() << std::endl;
	sol->constrs = constrs;
	sol->vars = vars;

	return 0;
}

int buildmodelLoad(TRCplexSol* sol, trdata* dat, trparams par) {
	using std::string;
	using std::to_string;
	using std::get;
    std::map <std::string, IloNumVar> vars;
    std::map <std::string, IloConstraint> constrs;
	IloModel* model = &(sol->model);
    IloEnv env = sol->env;



    /*
     * CREATION OF VARIABLES: x, y, a, L,
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
    	int i = get<0>(arc.first);
    	int j = get<1>(arc.first);
        for(int k = 0; k < dat->k; k++) {
        	string name = getname("L", i,j,k);
        	if (debug)
        		std::cout << "L_" << i << "_" << j << "_" << k << std::endl;
        	vars[name] = buildVar(env, IloNumVar::Int, 0, dat->C, name);
        }
    }
	for(int k = 0; k < dat->k; k++){
		int ind[] = {k};
		string name = getname("z", ind, 1);
		vars[name] = buildVar(env, IloNumVar::Int, 0, par.maxz, name);
	}

	//vars.at(getname("y",0,3,1)).setLB(1);

	/*
	 * CREATION OF CONSTRAINTS
	 */

	for(auto const &order : dat->orders){
    	int os = get<0>(order.first);
    	int ot = get<1>(order.first);
        IloExpr expr8(env);
        for (int k = 0; k < dat->k; k++){
        	auto vy = vars.at(getname("y", os, ot, k));
        	expr8 -= vy;
        }
        string cname = getname("c8_y", os,ot);
        constrs[cname] = buildConstr(model, expr8, -1, true, cname);
        expr8.end();
    }


    int nvel = dat->k;
    for (int k = 0; k < nvel; k++){
        IloExpr exprxs(env);
    	for(auto const &j : dat->fstar.at(dat->source)){
    		std::string xs = getname("x", dat->source, j, k);
    		auto vxs = vars.at(xs);
    		exprxs += vxs;
    	}
    	int ind[] = {k};
    	std::string cname = getname("c9_ksource", ind, 1);
    	constrs[cname] = buildConstr(model, exprxs, 1, true, cname);
    	exprxs.end();
    }


    for(int k=0; k < dat->k; k++){
    	// debug
    	for(auto const &ii : dat->bstar.at(dat->target)){
    		std::string xt = getname("x", ii, dat->target, k);
    	}
    	IloExpr exprxt(env);
    	int target = dat->target;
    	for(auto const& i : dat->bstar.at(target)){
    		std::string xt = getname("x", i, dat->target, k);
    		auto vxt = vars.at(xt);
    		exprxt += vxt;
    	}
    	int ind[] = {k};
    	std::string cname = getname("c10_ktarget", ind, 1);
    	constrs[cname] = buildConstr(model, exprxt, 1, true, cname);
    	exprxt.end();
    }

    for(int k = 0; k < nvel; k++){

    	for (auto const &i : dat->n){
            IloExpr exprxf(env);
    		if(i == dat->source || i == dat->target)
    			continue;
    		for (auto const &j : dat->bstar.at(i)){
    			exprxf += vars.at(getname("x",j,i,k));
    		}
    		for (auto const &j : dat->fstar.at(i))
    			exprxf -= vars.at(getname("x",i,j,k));
        	std::string cname = getname("c11_xkflow", k,i);
        	constrs[cname] = buildConstr(model, exprxf, 0, true, cname);
        	exprxf.end();
    	}

        if (par.subTourElimination){
			for (auto const &arc : dat->arcs){
				IloExpr exprseq(env);
				int i = get<0>(arc.first);
				int j = get<1>(arc.first);
				std::string x = getname("x", i,j,k);
				std::string aik = getname("a",i,k);
				std::string ajk = getname("a",j,k);
				exprseq += vars.at(aik) - vars.at(ajk) + par.MSEQ*vars.at(x);
				std::string cname = getname("c13_sequence", i,j,k);
				int rhs = par.MSEQ - arc.second.t;
				constrs[cname] = buildConstr(model, exprseq, rhs, false, cname);
			}
        }


    	/*
//    	 * L_{j,k} <= \sum_{i in \delta^-(j)}L_{i,k} + \sum_{p}dplus_{p,j}*y_{p,k} - \sum{p}dminus_{p,j}*y_{p,k}
    	 */

    	for(auto const &i : dat->n){
    		IloExpr exprLoadjk(env);
    		for(auto const &j : dat->fstar.at(i))
    			exprLoadjk -= vars.at(getname("L",i,j,k));
    		t_orders dplus = getDemandSourceAt(i, dat);
    		t_orders dminus = getDemandTargetAt(i, dat);
    		for(auto const &bi : dat->bstar.at(i)){
    			//if (bi == dat->target || i == dat->source)
    				//continue;
    			exprLoadjk += vars.at(getname("L",bi,i,k));
    		}
    		for (auto const &order : dplus){
    			exprLoadjk += order.second * vars.at(getname("y", get<0>(order.first), get<1>(order.first), k));
    		}
    		for (auto const &order : dminus){

    			exprLoadjk -= order.second * vars.at(getname("y", get<0>(order.first), get<1>(order.first), k));
    		}
    		std::string cname = getname("c15_Loadik" ,i, k);
    		constrs[cname] = buildConstr(model, exprLoadjk, 0, true, cname);
    		exprLoadjk.end();
    	}

    	/*
    	 * \sum_j x_{l(p),jk} >= y_{p,k}
    	 * \sum_i x_{i, m(p),k} >= y_{p,k}
    	 */

    	for(auto const &order : dat->orders){
    		IloExpr exprXYout(env);
    		IloExpr exprXYin(env);
    		int ofrom = get<0>(order.first);
    		int oto= get<1>(order.first);
    		exprXYin += vars.at(getname("y",ofrom, oto,k));
    		for(auto const& j : dat->fstar.at(ofrom))
    			exprXYin -= vars.at(getname("x", ofrom, j, k));
    		std::string cname = getname("c17_XYin", ofrom, oto,k);
    		constrs[cname] = buildConstr(model, exprXYin, 0, false, cname);
    		exprXYin.end();

    		exprXYout += vars.at(getname("y",ofrom, oto,k));
    		for(auto const& i : dat->bstar.at(oto))
    			exprXYout -= vars.at(getname("x", i, oto, k));
    		cname = getname("c17_XYout", ofrom, oto,k);
    		constrs[cname] = buildConstr(model, exprXYout, 0, false, cname);
    		exprXYout.end();
    	}


    	/*
    	 * L_{i,j,k}/C <= x_{i,j,k}
    	 */
    	for(auto const &arc : dat->arcs){
    		for(int k=0; k < dat->k; k++){
				IloExpr exprL(env);
				int i = get<0>(arc.first);
				int j = get<1>(arc.first);
				exprL += vars.at(getname("L",i,j,k))/dat->C - vars.at(getname("x",i,j,k));
				std::string cname = getname("c17_LX", i, j,k);
				constrs[cname] = buildConstr(model, exprL, 0, false, cname);
				exprL.end();
    		}
    	}

    	/*
    	 * strenghten constraint 1
    	 * L_start
    	 *
    	 */
/*

*/
    	if(par.validinequality01){

        	for(auto const & j : dat->n){
    			IloExpr exprSc1(env);
    			for(auto const& order : dat->orders){
    				if(get<1>(order.first) == j)
    					exprSc1 += order.second * vars.at(getname("y", get<0>(order.first),j,k));
    			}
    			for(auto const& i : dat->bstar.at(j))
    				exprSc1 -= vars.at(getname("L", i,j,k));
    			std::string cname = getname("CS18Lstart_",j,k);
    			constrs[cname] = buildConstr(model, exprSc1, 0, false, cname);
    			exprSc1.end();
        	}

/*NOT IMPROVEMENT
        	for(auto const & i : dat->n){
    			IloExpr exprSc2(env);
    			for(auto const& order : dat->orders){
    				if(get<0>(order.first) == i)
    					exprSc2 += order.second * vars.at(getname("y", i, get<1>(order.first),k));
    			}
    			for(auto const& ip : dat->fstar.at(i))
    				exprSc2 -= vars.at(getname("L",i,ip,k));
    			std::string cname = getname("CS19Lend_",i,k);
    			constrs[cname] = buildConstr(model, exprSc2, 0, false, cname);
    			exprSc2.end();
        	}
*/
    		/*  THESE 2 CONSTRAINTS WILL NOT WORK UNTIL WILL NOT HAVE A VARIABLE TELLING IF AN ARC (i,j)
    		 * IS TRAVERSED BY A VEHICLE k WITH A ORDER p
    		 */

        	/*
			for(auto const & arc : dat->arcs){
				int i = get<0>(arc.first);
				int j = get<1>(arc.first);
				IloExpr exprSc1(env);
				exprSc1 -= vars.at(getname("L",i,j,k));
				for(auto const& order : dat->orders){
					if(get<0>(order.first) == i)
						exprSc1 += order.second * vars.at(getname("y", i,get<1>(order.first),k));
				}
				std::string cname = getname("CS18Lstart_",i,j,k);
				constrs[cname] = buildConstr(model, exprSc1, 0, false, cname);
				exprSc1.end();



				IloExpr exprSc2(env);
				exprSc2 -= vars.at(getname("L",i,j,k));
				for(auto const& order : dat->orders){
					if(get<1>(order.first) == j)
						exprSc2 += order.second * vars.at(getname("y", get<0>(order.first),j,k));
				}
				cname = getname("CS19Lend_",i,j,k);
				constrs[cname] = buildConstr(model, exprSc2, 0, false, cname);
				exprSc2.end();
			}
			*/
    	}

    	if (par.zconstraint){
    		IloExpr exprz(env);
    		exprz += vars.at(getname("a", dat->target,k))/dat->H;
    		exprz -= vars.at(getname("z",k));
    		constrs[getname("c14z_",k)] = buildConstr(model, exprz, 1, false, getname("c14z_",k));
    		exprz.end();
    	}


    }
    /**
     * c19_constraint orders precedence
     */
    for (auto const& order : dat->orders){
    	for(int k = 0; k < dat->k; k++){
    		IloExpr exprop(env);
			int i = get<0>(order.first);
			int j = get<1>(order.first);
			exprop += vars.at(getname("a", i, k)) - vars.at(getname("a", j, k))
					+ par.MSEQ * vars.at(getname("y",i,j,k));
			constrs[getname("c19_oprec_",i, j, k)] =
					buildConstr(model, exprop, par.MSEQ, false, getname("c19_oprec_",i, j, k));
			exprop.end();
		}
    }

	/*
	 * restricting constraint 1 and 2
	 *
	 */
/*
    for(auto const &i : dat->n){
    	if(i == dat->source || i == dat->target)
    		continue;
    	IloExpr exprRs1(env);
    	IloExpr exprRs2(env);
    	for(int k = 0; k < dat->k; k++){
    		for(auto const & j : dat->fstar.at(i))
    			exprRs1 += vars.at(getname("x",i,j,k));
    		for(auto const & j : dat->fstar.at(i))
    			exprRs2 += vars.at(getname("x",j,i,k));
    	}
		std::string cname = "CRS1_" + std::to_string(i);
		constrs[cname] = buildConstr(model,exprRs1,1,true, cname);
		exprRs1.end();
		cname = "CRS2_" + std::to_string(i);
		constrs[cname] = buildConstr(model,exprRs2,1,true, cname);
		exprRs2.end();
    }
*/

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
			obj += arc.second.c * vars.at(getname("x", i, j, k));
		}
		int zind[] = {k};
		std::string zname = getname("z", zind,1);
		obj += dat->F * vars.at(zname);
	}
	model->add(IloMinimize(env, obj));
	obj.end();

	std::cout << "n. of variables created: " <<  vars.size() << std::endl;
	std::cout << "n. of constraints created: " << constrs.size() << std::endl;
	sol->constrs = constrs;
	sol->vars = vars;

	return 0;
}

IloNumVarArray getNumVarArray (std::map<std::string, IloNumVar>* mapVars) {
	IloNumVarArray vars;
	for (auto &aVar : *mapVars){
		IloNumVar aVar2  = aVar.second;
		vars.add(aVar2);
	}
	return vars;
}

IloModel relaxModel(IloEnv& env, IloModel& model, std::map<std::string, IloNumVar>& vars){

	IloModel relax(env);
	relax.add(model);
	for(auto& aVar : vars)
		relax.add(IloConversion(env, aVar.second, ILOFLOAT));
	return relax;
}

IloCplex solvemodel(TRCplexSol* sol, trdata* dat, trparams par){
	IloCplex cplex(sol->model);
	sol->cplex = cplex;
	cplex.setParam(IloCplex::TiLim, par.timeLimit);
	cplex.exportModel(("logs/model_s" + std::to_string(par.service) + "_" + par.modelType + "_" + par.instance + ".lp").c_str());

	try{
		cplex.solve();
	cplex.writeSolution(("logs/solution_s" + std::to_string(par.service) + "_" + par.instance + ".sol").c_str() );
	}
	   catch (IloException& ex) {
	      std::cerr << "Error: " << ex << std::endl;
	   }
	   catch (...) {
	      std::cerr << "Error" << std::endl;
	   }
	cplex.out() << "Solution status: " << cplex.getStatus() << std::endl;
	try{
	cplex.out() << " Solution value: " << cplex.getObjValue () << std::endl;
	} catch (IloException & ex){
		std::cerr << "no value solution value available" << std::endl;
	}
	return cplex;
}

int writeSolution(TRCplexSol * sol, trdata* dat, trparams* par){
	using std::get;
	IloCplex& cplex = sol->cplex;
	std::cout << "Solution status: " << cplex.getCplexStatus() << std::endl;
	try{
		std::cout << "Solution value: " << cplex.getObjValue() << std::endl;
		IloNum tolerance = cplex.getParam(
		   IloCplex::Param::MIP::Tolerances::Integrality);
		//printing x[i][j][k]
		std::cout << "************x************" << std::endl;
		std::cout << "i\tj\tk\tx"<< std::endl;
		for(int k = 0; k < dat->k; k++){
			for(auto const& arc : dat->arcs){
				int i = get<0>(arc.first);
				int j = get<1>(arc.first);
				std::string x = getname("x",i,j,k);
				int xval = cplex.getValue(sol->vars.at(x));
				if (xval > tolerance)
					std::cout << i << "\t" << j << "\t" << k << "\t"<<xval << std::endl;
			}
		}
		std::cout << "************a************" << std::endl;
		std::cout << "i\tk\ttime"<< std::endl;
		for(int k = 0; k < dat->k; k++){
			for(auto const& i : dat->n){
				std::string as = getname("a",i,k);
				auto a = sol->vars.at(as);
				if(cplex.isExtracted(a)){
					float aval = cplex.getValue(sol->vars.at(as));
					std::cout << i << "\t" << k << "\t" << aval << std::endl;
				}
				//else
					//std::cout << "ERROR: A VARIABLE NOT EXTRACTED: a_" << i << "_" << k << std::endl;

			}
		}
		std::cout << "*****************y_p_k**********" << std::endl;
		std::cout << "order_from\torder_to\tk\tdemand\tvalue" << std::endl;
		for (auto const& order : dat->orders){
			for (int k = 0; k < dat->k; k++){
				int i = get<0>(order.first);
				int j = get<1>(order.first);
				std::string y = getname("y",i,j,k);
				int yval = cplex.getValue(sol->vars.at(y));
				if (yval > tolerance)
					std::cout << i << "\t" << j << "\t" << k << "\t"  << order.second  << "\t" << yval<< std::endl;
			}
		}

		if(par->modelType == "load"){
			std::cout << "*****************L_i_j_k**********" << std::endl;
			std::cout << "i\tj\tk\tLoad" << std::endl;
			for (auto const & arc : dat->arcs ){
				int i = get<0>(arc.first);
				int j = get<1>(arc.first);
				for (int k = 0; k < dat->k; k++){
					std::string L = getname("L",i,j,k);
					int Lval = cplex.getValue(sol->vars.at(L));
					if (Lval > tolerance)
						std::cout << i << "\t" << j <<  "\t" << k <<"\t"<< Lval << std::endl;
				}
			}

		}
		std::cout << "*****************z_k**********" << std::endl;
		std::cout << "k\tz value" << std::endl;
		for (int k = 0; k < dat->k; k++){
			std::string z = getname("z",k);
			int zval = cplex.getValue(sol->vars.at(z));
			std::cout << k << "\t" << zval<< std::endl;
		}

	}
    catch(IloException& e){
    	std::cerr << e << std::endl;
    	std::cout << "it was not possible to write solution" << std::endl;
    }
	return 0;

}
/*
int solve_exactly(IloEnv& env, IloModel model, toptwdata& dat, params& par,

		map<string, IloConstraint>& mapConstr, map<string, IloNumVar>& mapVar, float **t ){
	int n = dat.n;
	int m = par.m;

    IloCplex cplex(model);
    cplex.exportModel("model.lp");

    cplex.solve();
    cplex.out() << "Solution status: " << cplex.getStatus() << endl;
    cplex.out() << " Solution value: " << cplex.getObjValue () << endl;



    IloNum tolerance = cplex.getParam(
       IloCplex::Param::MIP::Tolerances::Integrality);
    cplex.out() << "Optimal value: " << cplex.getObjValue() << endl;
    cplex.out() << "node, vehicle  - y[i][v]" << endl;
    for(int i = 0; i < n+2; i++) {
        for(int v = 0; v <m; v++){
        	if ( mapVar.find( "y#" + to_string(i) + "#" + to_string(v) ) == mapVar.end())
        			continue;
          if (cplex.getValue(mapVar["y#" + to_string(i) + "#" + to_string(v)]) >= 1 - tolerance) {
              cplex.out() <<  i << "," << v << endl;
          }
      }
    }

    cout << "x[i][j][k]" << endl;
    for(int i = 0; i < n+2; i++) {
        for(int j = 0; j <n+2; j++){
            for(int k = 0; k < m; k++){
              if (cplex.isExtracted(mapVar["x#" + to_string(i) + "#" + to_string(j)+ "#" + to_string(k)])
              		&& cplex.getValue(mapVar["x#" + to_string(i) + "#" + to_string(j)+ "#" + to_string(k)]) >= 1 - tolerance) {
              cout <<  i << "," << j << "," << k << "\n";
            }

          }
      }
    }

   cout << "a[i][k]" << endl;
    for(int i = 0; i < n+1; i++) {
            for(int k = 0; k < m; k++){
              if (cplex.isExtracted(mapVar["a#" + to_string(i) + "#" + to_string(k)])
              		&& cplex.getValue(mapVar["a#" + to_string(i) + "#" + to_string(k)]) >= 1 - tolerance) {
              cout <<  i << "," << k << "\n";
            }

          }
      }

	return 0;
}

*/

t_orders getDemandSourceAt(int i, trdata* dat){
	using std::get;
	t_orders orders;
	for (auto const& order : dat->orders){
		if(get<0>(order.first) == i)
			orders[order.first] = order.second;
	}
	return orders;
}


t_orders getDemandTargetAt(int i, trdata* dat){
	using std::get;
	t_orders orders;
	for (auto const& order : dat->orders){
		if(get<1>(order.first) == i)
			orders[order.first] = order.second;
	}
	return orders;
}


inline IloNumVar buildVar(IloEnv &env,IloNumVar::Type numtype, int lb, int ub, std::string varname){
	IloNumVar v(env, lb, ub, numtype, varname.c_str());
	return v;
}
IloConstraint buildConstr(IloModel *model, IloExpr exprs, int rhs, bool equal, std::string cname){
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
std::string getname (std::string baseName, int ind[], int indsize){
	using std::string;
	using std::to_string;
	string varname = "" + baseName;//"x#" + to_string(a) + "#" + to_string(b) + "#" + to_string(c);
	for (int i = 0; i < indsize; i++){
		varname += "_" + to_string(ind[i]);
	}
	return varname;
}
std::string getname (std::string baseName, int i){
	int ind[] = {i};
	return getname(baseName, ind, 1);
}
std::string getname (std::string baseName, int i, int j){
	int ind[] = {i,j};
	return getname(baseName, ind, 2);
}
std::string getname(std::string baseName, int i, int j, int k){
	int ind[] = {i, j, k};
	return getname(baseName, ind, 3);
}
std::string getname(std::string baseName, int i, int j, int k, int p){
	int ind[] = {i, j, k, p};
	return getname(baseName, ind, 4);
}
trparams fillparams(std::string filename){
	trparams t;
	t.p = readparams(filename);
	t.M = stoi(t.p.at("M"));
	t.MSEQ = stoi(t.p.at("MSEQ"));
	t.instance = t.p.at("instance");
	t.linearize = t.p.at("linearize");
	t.maxtimetour = stof(t.p.at("maxtimetour"));
	t.maxz = stoi(t.p.at("maxz"));
	t.modelType = t.p.at("modelType");
	t.subTourElimination =  t.p.at("subTourElimination")!="0";
	t.zconstraint = t.p.at("zconstraint") != "0";
	t.service = t.p.at("service") != "0";
	t.timeLimit = stoi(t.p.at("timeLimit"));
	t.validinequality01 = t.p.at("validinequality01") != "0";
	return t;
}

int main( int argc, char *argv[] ) {
	std::cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	TRCplexSol sol(env, model);
	trdata dat;
	trparams par = fillparams("config.txt");
	if (argc > 1){
		par.instance = argv[1];
	}
	if (argc > 2){
		par.modelType = argv[2];
	}
	std::cout << "+++++++++++model type " << par.modelType;
 	std::string instanceName = par.instance;
	std::string linearize = par.linearize;
	std::cout << "+++++++solving instance " << instanceName << std::endl;
	std::string filenamebase = "input/" + instanceName;
	load_csv(&dat, filenamebase);
	std::cout << dat.to_string() << std::endl;
	if(par.service){
		std::cout <<"************ADDING SERVICES (new sources and new target)************" << std::endl;
		addServices(&dat, par);
		std::cout << "updated data-------------"  << std::endl;
		std::cout << dat.to_string() << std::endl;
	}

	std::cout << "*************BUILDING MODEL**************" << std::endl;
	int ret = -1;
	if (par.modelType == "original")
		ret = buildmodel(&sol, &dat, par);
	else
		ret = buildmodelLoad(&sol, &dat, par);

	std::cout << "build model return " << ret << std::endl;
	if (linearize == "lp"){
		std::cout << "*************SOLVING RELAXED MODEL***************" << std::endl;
		IloCplex cplex (model);
		sol.cplex = cplex;
		IloModel relaxedModel = relaxModel(env, model, sol.vars);
		sol.model = relaxedModel;
	}
	else
		std::cout << "*************SOLVING MIP MODEL***************" << std::endl;

	solvemodel(&sol, &dat, par);
	sol.model = model;
	std::cout << "*************SOLUTION****************" << std::endl;
	writeSolution(&sol, &dat, &par);
	env.end();


	return 0;
}


