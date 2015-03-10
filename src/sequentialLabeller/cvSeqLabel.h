/*
 * Copyright (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#pragma once

// OpenCV 2
#include <opencv2/core/core.hpp>
// OpenCV 1
//#include <cxcore.h> //OpenCv header file

int cvSeqLabel( IplImage *in, IplImage *out, IplImage *tmp);
