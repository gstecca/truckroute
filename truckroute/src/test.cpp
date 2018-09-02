/*
 * test.cpp
 *
 *  Created on: Aug 31, 2018
 *      Author: gstecca
 */

#include "readfile.h"
#include <string>
#include <map>
#include <iostream>
int main2( int argc, char *argv[] ) {
	std::string filename = "config.txt";
	std::map<std::string,std::string> parameters = readparams(filename);
	int n = stoi(parameters.at("n"));
	std::cout << "n integer: " << n << std::endl;
	std::cout << "total keys found: " << parameters.size() << std::endl;
	return 0;

}

