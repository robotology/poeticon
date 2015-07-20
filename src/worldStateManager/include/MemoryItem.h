/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __SHORT_TERM_MEMORY_ITEM_H__
#define __SHORT_TERM_MEMORY_ITEM_H__

#include <iostream>
#include <string>

using namespace std;

struct MemoryItem
{
    const int id;
    const string name;
    const bool isHand;

    // constructor
    MemoryItem(const int _id, const string _name, const bool _isHand)
    : id(_id), name(_name), isHand(_isHand)
    {}

    // copy constructor
    MemoryItem(const MemoryItem& other)
    : id(other.id), name(other.name), isHand(other.isHand)
    {}

    // copy assignment operator
    MemoryItem& operator=(const MemoryItem& mi)
    {
        // TODO: fix
        /*
        if(this != &mi)
        {
            name = mi.name;
            isHand = mi.isHand;
        }
        */
        return *this;
    }

    // destructor
    virtual ~MemoryItem()
    {}

    // http://stackoverflow.com/questions/1549930/c-equivalent-of-java-tostring
    virtual std::ostream& toString(std::ostream& o) const
    {
        return o << "id=" << id << " name=" << name;
    }
};

// inline because http://stackoverflow.com/questions/12802536/c-multiple-definitions-of-operator
inline ostream& operator<<(ostream& str, const MemoryItem& mi)
{
    return mi.toString(str);
}

#endif
