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
using namespace std;

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	IloEnv env;
	IloModel model(env);
	trdata dat;
	string filenamebase = "input/instanzaNord1";
	load_csv(&dat, filenamebase);
	return 0;
}

int load_csv(trdata * dat, string filenamebase)
//int load_data_tw(vector<int>* profits, vector<coord>* coords, int* n, int* m, float* tau, string filename)
{
    /*
     * READING GENERAL
     */
	string fsheet = "general";
    string filename = filenamebase + "_" + fsheet + ".csv";
	cout << "reading " << filename << std::endl;
	ifstream myfile(filename);
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
			cout << line << "\n";
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

			cout << "READ " <<  token1 << "\n";
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
        ifstream myfile2(filename);
        if(myfile){
			getline(myfile2, line); // skip first line
			while(getline(myfile2, line)) {
				pos = line.find(delimiter);
				token1 = line.substr(startsub, pos);
				startsub = pos + 1;
				pos = line.find(delimiter, startsub);
				token2 = line.substr(startsub, pos - startsub);
				startsub = pos +1;
				pos = line.find(delimiter, startsub);
				string token3 = line.substr(startsub, pos - startsub);
				cout << token1 << "," << token2 << ","<< token3 << "\n";
			}
        } else
            cout << "error opening file\n";

        /*


        //

         //

        //cout << dat->m << "\n";

        startsub = pos + 1; //
        pos = line.find(delimiter, startsub);
        token = line.substr(startsub, pos - startsub);
            // coords[i][1] = stof(token);
        dat->n  = stoi(token);

        getline(myfile, line); //skip a line

       // c->y = stof(token);
            // cout << "y coord: " << token << endl;
            // profit

        int i = 0;



        while(getline(myfile, line)) {
            */
           /*
            * 	i x y d S f a list O C
            *
            *   Where
            *   i = vertex number
            *   x = x coordinate
            *   y = y coordinate
            *   d = service duration or visiting time
            *   S = profit of the location
            *   f = not relevant
            *   a = not relevant
            *   list = not relevant (length of the list depends on a)
            *   O = opening of time window (earliest time for start of service)
            *   C = closing of time window (latest time for start of service)
            *
            *    * REMARKS *
            *    - The first point (index 0) is the starting AND ending point.
            *    - The number of paths (P) is not included in the data file. This
            *
            *
            *
            */
            //cout << "line: " << line << "\n";
        	/*
        	cout << line << "\n";



            string buf; // Have a buffer string
            stringstream ss(line); // Insert the string into a stream

            vector<string> tokens; // Create vector to hold our words


            while (ss >> buf)
                tokens.push_back(buf);


            */

            /*cout << "LINE TOKENIZED  \n";
            for (string aaa : tokens)
                cout << aaa << "*";
            cout << "\n";
            */

        	/*
        	 *
        	coord* c = new coord();
            tw * ttw = new tw();

            dat->id.push_back(stoi(tokens[0]));
            float x = stof(tokens[1]);
            float y = stof(tokens[2]);
            c->x = x;
            c->y = y;
            dat->coords.push_back(*c);
            //visit time
            dat->s.push_back(stoi(tokens[3]));
            // profit
            dat->p.push_back(stoi(tokens[4]));
            // time windows
            ttw->l = stoi(tokens[tokens.size()-2]);
            ttw->r = stoi(tokens[tokens.size()-1]);
            dat->atw.push_back(*ttw);
            if(i == 0){
                dat->tau = ttw->r;
                i = 1;
            }


        }
        */

    return 0;
}
