/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __WSM_HELPERS_H__
#define __WSM_HELPERS_H__

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <vector>

#include <yarp/os/LogStream.h>
#include <yarp/sig/Vector.h>

/**********************************************************/
template<class key,class val>
void dumpMap(const std::map<key,val> &m)
{
    std::ostringstream fullMapContent; // output stream we'll feed to yDebug macro
    size_t items_remaining = m.size(); // http://stackoverflow.com/a/151112
    bool last_iteration = false; 
    for(typename std::map<key,val>::const_iterator iter = m.begin();
        iter != m.end();
        ++iter)
    {
        fullMapContent << iter->first << " " << iter->second;
        last_iteration = (items_remaining-- == 1);
        if (!last_iteration)
            fullMapContent << "; ";
    }
    yInfo() << "<id label>: [" << fullMapContent.str().c_str() << "]";
}

/**********************************************************/
bool euclideanDistance(yarp::sig::Vector &v1, yarp::sig::Vector &v2, float &dist);

/**********************************************************/
template<class key,class val>
bool mergeMaps(const std::map<key,val> &map1, const std::map<key,val> &map2, std::map<key,val> &result)
{
    result = map1;
    result.insert(map2.begin(), map2.end());
    return true;
}

/**********************************************************/
bool vectorsDiffer(const std::vector<int> &v1, const std::vector<int> &v2);

#endif
