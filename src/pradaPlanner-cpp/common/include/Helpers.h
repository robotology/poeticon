/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Alexandre Antunes <aleksander88@gmail.com>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

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
