//============================================================================
// Name        : truckroute.cpp
// Author      : gstecca
// Version     :
// Copyright   : Giuseppe Stecca
// Description : pick-up and deliveries with specific constraints
//============================================================================

#include <iostream>
#include <ilcplex/ilocplex.h>
//#include <ilconcert/ilomodel.h>
#include <truckroute.h>
#include <string>
#include <map>

/**
 * A simple struc used to wrap cplex object and store in maps variable and constraints which will be retrieved later
 */
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
IloCplex solvemodel(TRCplexSol* sol, trdata* dat, trparams par);
int writeSolution(TRCplexSol* sol, trdata* dat);
std::string getname (std::string baseName, int ind[], int indsize);
std::string getname (std::string baseName, int i, int j);
std::string getname (std::string baseName, int i, int j, int k);
std::string getname (std::string baseName, int i, int j, int k, int p);

/**
 * entry point for the application
 * run as ./truckroute instanceName
 * the instanceName will define the set of input files loaded from "input" subfoalder
 * the input data are stored in the following csv files:
 * modelInstance_general.csv, modelInstance_network.csv, modelInstance_orders.csv
 * @param instanceName the name of the instance
 */

int main( int argc, char *argv[] ) {
	std::cout << "!!Starting!!!" << std::endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	TRCplexSol sol(env, model);
	trdata dat;
	std::string instanceName = argv[1];
	std::cout << "+++++++solving instance " << instanceName << std::endl;
	std::string filenamebase = "input/" + instanceName;
	load_csv(&dat, filenamebase);
	std::cout << dat.to_string() << std::endl;
	trparams par = fillparams();
	std::cout << "*************BUILDING MODEL**************" << std::endl;
	int ret = buildmodel(&sol, &dat, par);
	std::cout << "build model return " << ret << std::endl;
	std::cout << "*************SOLVING MODEL***************" << std::endl;
	solvemodel(&sol, &dat, par);
	std::cout << "*************SOLUTION****************" << std::endl;
	writeSolution(&sol, &dat);


	return 0;
}

/**
 * Load data from the csv files and store in the trdata struct
 * @param trdata * dat Pointer to the data struct
 * @param filenamebase The name of the instance
 */
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


        while (getline(myfile, line)){
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
    	std::cout << "error opening file\n";


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

/**
 * Build the model in cplex
 * @param sol Reference to where the cplex enviroment is stored
 * @dat Reference to the data struct where data is stored
 * @par Instance and Model parameters. Load from config.txt file
 */
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

        		//expr += vars[u] ;
        		//expr+= par.M*vars[y];
				//expr += (-1)*vars[x];
				expr4 = vars.at(u) + par.M*vars.at(y) - vars.at(x);
        		int ind[] = {os,ot,i,j,k};
        		int indvar[] = {i,j,os,ot,k};
        		cname = getname("c4_x_u", ind, 5);
        		constrs[cname] = buildConstr(model, expr4, par.M, false, cname);
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
    std::cout << "debug" << std::endl;
    int nvel = dat->k;
    for (int k = 0; k < nvel; k++){
        //IloExpr exprxs(env);
    	//debug
    	std::cout << k << std::endl;
    	int source = dat->source;
        auto fstarset = dat->fstar.at(source);
        for(auto const &jj : fstarset){
    		std::cout << "k:" << k << "  j:" << jj << std::endl;
    		//std::string xs = getname("x", dat->source, j, k);
    		//cout << vars.count(xs) << std::endl;
    	}
    }

    for (int k = 0; k < nvel; k++){
        IloExpr exprxs(env);
    	for(auto const &j : dat->fstar.at(dat->source)){
    		std::cout << j << std::endl;
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
    			std::cout << "arc j->i:"  << j << "->" << i << std::endl; //problem for arc 0->2
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
			constrs[cname] = buildConstr(model, exprseq, rhs, false, cname);
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
/**
 * Solve the model. It export the model under model.lp file in lp format. Then solve it
 * Write at screen the solution status and the solution value. Write all the solution variable values to the
 * file solution.sol
 */
IloCplex solvemodel(TRCplexSol* sol, trdata* dat, trparams par){
	IloCplex cplex(sol->model);
	sol->cplex = cplex;
	cplex.exportModel("model.lp");
	try{
		cplex.solve();
	}catch (IloException& e) {
		  std::cout << e.getMessage() << std::endl;
		  e.end();
		}
	cplex.out() << "Solution status: " << cplex.getStatus() << std::endl;
	cplex.out() << " Solution value: " << cplex.getObjValue () << std::endl;
	cplex.writeSolution("solution.sol");
	return cplex;
}

/**
 * Write x and a variables at screen
 */
int writeSolution(TRCplexSol * sol, trdata* dat){
	using std::get;
	IloCplex& cplex = sol->cplex;
	std::cout << "Solution status: " << cplex.getCplexStatus() << std::endl;
	std::cout << "Solution value: " << cplex.getObjValue() << std::endl;
    IloNum tolerance = cplex.getParam(
       IloCplex::Param::MIP::Tolerances::Integrality);
    //printing x[i][j][k]
    std::cout << "************x************" << std::endl;
    std::cout << "i\tj\tk"<< std::endl;
    for(int k = 0; k < dat->k; k++){
    	for(auto const& arc : dat->arcs){
    		int i = get<0>(arc.first);
    		int j = get<1>(arc.first);
    		std::string x = getname("x",i,j,k);
    		int xval = cplex.getValue(sol->vars.at(x));
    		if (xval >= 1 - tolerance)
    			std::cout << i << "\t" << j << "\t" << k << std::endl;
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
    		else
    			std::cout << "ERROR: A VARIABLE NOT EXTRACTED: a_" << i << "_" << k << std::endl;

    	}
    }
	return 0;

}


/**
 * build a cplex IloNumVar
 * @param env Reference to the cplex environment
 * @param numtype Type of the variable (Continuos, Integer, Boolean)
 * @lb lower bound of variable domain
 * @ub upper bound of variable domain
 * @param varname unique variable map used also as key of the map storing the variables
 */
inline IloNumVar buildVar(IloEnv &env,IloNumVar::Type numtype, int lb, int ub, std::string varname){
	IloNumVar v(env, lb, ub, numtype, varname.c_str());
	return v;
}
/**
 * build a cplex constraint based on expression, right hand side, boolean value for equality sign, and name.
 * @param model the cplex model
 * @param exprs the cplex IloExpr object. It is the lefthand side of the constraint
 * @param rhs the righthand side of the model
 * @param equal Boolean value True if the constraint is equality constraint. Otherwise it will be a less or equal (<=) constraints
 * @return the cplex constraint
 */
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
/**
 * Utility function for creating a constraint of variable name based on an array of indeces
 * @param baseName the base of the name, e.g. x
 * @param ind[] the array of indices, e.g. {1,2,1}
 * @param indsize The size of the ind array e.g. 3
 * @return the variable or constraint name e.g. x#1#2#1
 */
std::string getname (std::string baseName, int ind[], int indsize){
	using std::string;
	using std::to_string;
	string varname = "" + baseName;//"x#" + to_string(a) + "#" + to_string(b) + "#" + to_string(c);
	for (int i = 0; i < indsize; i++){
		varname += "#" + to_string(ind[i]);
	}
	return varname;
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
/**
 * fill the tparams struct with standard values
 */
trparams fillparams(){
	trparams t;
	t.maxtimetour = 10000;
	t.maxz = 10;
	t.M = 1000000;
	t.MSEQ = 1000;
	return t;
}
