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
// http://stackoverflow.com/a/29798
inline const char * const BoolToString(bool &b)
{
    return b ? "true" : "false";
}

/**********************************************************/
struct compareSecond
{
    template <typename Pair>
    bool operator()(const Pair &a, const Pair &b)
    {
        return a.second < b.second;
    }
};

/**********************************************************/
template<class key,class val>
void dumpMap(const std::map<key,val> &m)
{
    // output stream we'll feed to yDebug macro
    std::ostringstream fullMapContent;

    // http://stackoverflow.com/a/151112
    size_t items_remaining = m.size();
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
bool is_integer(const float k);

/**********************************************************/
template<class key,class val>
bool mapContainsKey(const std::map<key,val> &m, const key &k)
{
    return m.count(k); // 1 if element present in the map, 0 otherwise
}

/**********************************************************/
template<class key,class val>
bool mapContainsValue(const std::map<key,val> &m, const val &v)
{
    bool found = false;

    for(typename std::map<key,val>::const_iterator iter = m.begin();
        iter != m.end();
        ++iter)
    {
        if (iter->second == v)
            found = true;
    }

    return found;
}

#endif
