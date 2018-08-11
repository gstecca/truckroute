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
using std::cout;
using std::cin;


int main() {
	cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	trdata dat;
	std::string filenamebase = "input/instanzaNord1";
	load_csv(&dat, filenamebase);
	buildmodel(&model, &dat);
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
				dat->n.insert(from);
				startsub = pos + 1;
				pos = line.find(delimiter, startsub);
				int to = stoi(line.substr(startsub, pos - startsub));
				dat->n.insert(to);
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

int buildmodel(IloModel* model, trdata* dat) {
	using std::string;
	int n = dat->n.size();
	int m = dat->k;
	IloEnv env = model->getEnv();
    IloBoolVar x[n+2][n+2][m];
    for(int r = 0; r < n+2; r++) {
        for(int c = 0; c < n+2; c++) {
            for(int k=0; k<m; k++) {
                string varName = "x#" + std::to_string(r) + "#" + std::to_string(c) + "#" + std::to_string(k);
                x[r][c][k] = IloBoolVar(env, 0, 1, varName.c_str());
                //vars[varName] = x[r][c][k];
            }

        }
    }
	return 1;
}
