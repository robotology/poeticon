#ifndef __PLANNER_HELPERS_H__
#define __PLANNER_HELPERS_H__


#include <sstream>
#include <vector>
#include <string>

using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

int find_element(vector<string> vect, string elem);

std::vector<std::string> split(const std::string &s, char delim);

int vect_compare (vector<string> vect1, vector<string> vect2);

string NumbertoString(int number);

#endif
