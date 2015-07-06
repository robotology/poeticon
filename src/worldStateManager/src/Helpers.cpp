/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "Helpers.h"

/**********************************************************/
bool euclideanDistance(yarp::sig::Vector &v1, yarp::sig::Vector &v2, float &dist)
{
    if (v1.size() != v2.size())
    {
        yWarning() << __func__ << "inputs must have the same size";
        return false;
    }

    double sum = 0.0;
    for (int dim=0; dim<v1.size(); ++dim)
        sum += pow(v1[dim]-v2[dim],2.0);

    dist = sqrt(sum);

    return true;
}

/**********************************************************/
bool is_integer(const float k)
{
    return std::floor(k)==k;
}
