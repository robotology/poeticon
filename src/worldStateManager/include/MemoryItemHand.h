/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#ifndef __SHORT_TERM_MEMORY_ITEM_HAND_H__
#define __SHORT_TERM_MEMORY_ITEM_HAND_H__

#include <yarp/os/LogStream.h>

#include "MemoryItem.h"

struct MemoryItemHand : public MemoryItem
{
    bool isFree;

    // constructor
    MemoryItemHand(const int _id, const string _name, const bool _isHand,
                   bool _isFree)
    : MemoryItem(_id,_name,_isHand),
      isFree(_isFree)
    {}

    // copy constructor
    MemoryItemHand(const MemoryItemHand& other)
    : MemoryItem(other),
      isFree(other.isFree)
    {}

    // copy assignment operator
    MemoryItemHand& operator=(const MemoryItemHand& other)
    {
        // TODO: other fields
        return *this;
    }

    // destructor
    ~MemoryItemHand()
    {}

    virtual std::ostream& toString(std::ostream& o) const
    {
        // concatenate base class fields with derived fields
        return MemoryItem::toString(o) << " "
            << "isFree=" << (isFree?"true":"false");
    }
};

#endif
