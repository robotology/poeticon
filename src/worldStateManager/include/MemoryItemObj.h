/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef SHORT_TERM_MEMORY_ITEM_OBJECT_H
#define SHORT_TERM_MEMORY_ITEM_OBJECT_H

#include <yarp/os/Bottle.h>

#include "MemoryItem.h"

using namespace yarp::os;

struct MemoryItemObj : public MemoryItem
{
    Bottle pos2d;
    Bottle desc2d;
    Bottle tooldesc2d;
    string inHand;
    Bottle onTopOf;
    Bottle reachW;
    Bottle pullW;
    Bottle pushW;

    // constructors
    MemoryItemObj(const int _id, const string _name, const bool _isHand)
    : MemoryItem(_id,_name,_isHand)
    {}

    MemoryItemObj(const int _id, const string _name, const bool _isHand,
                  Bottle _pos2d,
                  Bottle _desc2d, Bottle _tooldesc2d,
                  string _inHand,
                  Bottle _onTopOf,
                  Bottle _reachW, Bottle _pullW, Bottle _pushW)
    : MemoryItem(_id,_name,_isHand),
      pos2d(_pos2d),
      desc2d(_desc2d), tooldesc2d(_tooldesc2d),
      inHand(_inHand),
      onTopOf(_onTopOf),
      reachW(_reachW), pullW(_pullW), pushW(_pushW)
    {}

    // copy constructor
    MemoryItemObj(const MemoryItemObj& other)
    : MemoryItem(other),
      pos2d(other.pos2d),
      desc2d(other.desc2d),
      tooldesc2d(other.tooldesc2d),
      inHand(other.inHand),
      onTopOf(other.onTopOf),
      reachW(other.reachW),
      pullW(other.pullW),
      pushW(other.pushW)
    {}

    // copy assignment operator
    MemoryItemObj& operator=(const MemoryItemObj& other)
    {
        // TODO: other fields
        return *this;
    }

    // destructor
    ~MemoryItemObj()
    {}

    virtual std::ostream& toString(std::ostream& o) const
    {
        // concatenate base class fields with derived fields
        return MemoryItem::toString(o) << " "
            << "pos2d=" << pos2d.toString().c_str() << " "
            << "desc2d=" << desc2d.toString().c_str() << " "
            << "tooldesc2d=" << tooldesc2d.toString().c_str() << " "
            << "in_hand=" << inHand << " "
            << "on_top_of=" << onTopOf.toString().c_str() << " "
            << "reachable_with=" << reachW.toString().c_str() << " "
            << "pullable_with=" << pullW.toString().c_str() << " "
            << "pushable_with=" << pushW.toString().c_str();
    }
};

#endif
