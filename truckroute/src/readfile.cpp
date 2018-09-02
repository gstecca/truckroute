/*
 * readfile.cpp
 *
 *  Created on: Aug 31, 2018
 *      Author: gstecca
 */

#include "readfile.h"
#include <string>
#include <map>
#include <iostream>
#include <fstream>

std::map<std::string,std::string> readparams(std::string filename){
	bool debug = false;
	using std::string;
	std::map<std::string,std::string> ret;
    /*
     * READING GENERAL
     */

    std::cout << "reading " << filename << std::endl;
	std::ifstream myfile(filename);
    string line;
	string delimiter = "=";
	size_t pos;
	int startsub;
    if(myfile) {
        while (getline(myfile, line)){
			//cout << line << "\n";
			startsub = 0;
			pos = line.find(delimiter);
			string token1 = line.substr(startsub, pos);
			startsub = pos + 1;
			pos = line.find(delimiter, startsub);
			string token2 = line.substr(startsub, pos - startsub);
			ret[token1] = token2;
        }
        myfile.close();
        if (debug){
        	for(auto const& par : ret){
        		std::cout << "key: " << par.first << "\tvalue: " << par.second << std::endl;
        	}
        }
    } else
    	std::cout << "error opening file\n";

	return ret;
}



