/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

// This is an automatically generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_OPC2PRADA_IDL
#define YARP_THRIFT_GENERATOR_OPC2PRADA_IDL

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>
#include <yarp/os/Bottle.h>

class OPC2PRADA_IDL;


class OPC2PRADA_IDL : public yarp::os::Wire {
public:
  OPC2PRADA_IDL();
  /**
   * Update the state text file.
   * @return true/false on success/failure
   */
  virtual bool update();
  /**
   * Load correspondence between ID and Labels in the DataBase
   * @return Bottle with ID and Objects names
   */
  virtual yarp::os::Bottle loadObjects();
  /**
   * Bottle
   * @return Bottle with 2d features
   */
  virtual yarp::os::Bottle query2d(const std::int32_t id);
  /**
   * Bottle
   * @return Bottle with 2d features of the tool
   */
  virtual yarp::os::Bottle querytool2d(const std::int32_t id);
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  bool read(yarp::os::ConnectionReader& connection) override;
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif
