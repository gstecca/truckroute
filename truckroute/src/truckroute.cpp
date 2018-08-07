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
	string filenamebase = "instanzaNord1";
	load_csv(&dat, filenamebase);
	dat.C = 33;
	dat.k = 7;

	cout << "problem has to be solved for k=" << dat.k << " vehicles\n";
	return 0;
}

int load_csv(trdata * dat, string filenamebase)
//int load_data_tw(vector<int>* profits, vector<coord>* coords, int* n, int* m, float* tau, string filename)
{
    string fsheet = "general";
    string filename = filenamebase + "_" + fsheet + ".csv";
	cout << "reading " << filename << std::endl;
	ifstream myfile(filename);
    cout << "filename: " << filename << "\n";
    string line;
    if(myfile) {
        //* dat = new toptwdata()
        getline(myfile, line);
        string delimiter = " ";
        string token;
        int startsub = 0;
        size_t pos = line.find(delimiter);
        token = line.substr(startsub, pos);
        //

        startsub = pos + 1; //
        pos = line.find(delimiter, startsub);
        token = line.substr(startsub, pos - startsub);
            // coords[i][1] = stof(token);
        dat->m = stoi(token);
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

            string buf; // Have a buffer string
            stringstream ss(line); // Insert the string into a stream

            vector<string> tokens; // Create vector to hold our words


            while (ss >> buf)
                tokens.push_back(buf);
            /*cout << "LINE TOKENIZED  \n";
            for (string aaa : tokens)
                cout << aaa << "*";
            cout << "\n";
            */
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

    } else
        cout << "error opening file\n";



    return 0;
}
