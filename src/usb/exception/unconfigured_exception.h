//
//  unconfigured_exception.h
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __UNCONFIGURED_EXCEPTION_H__
#define __UNCONFIGURED_EXCEPTION_H__

#include "exception.h"
#include <string>

namespace usb
{

class unconfigured_exception: public exception
{
public:
  explicit unconfigured_exception(const std::string& missing_config);
  explicit unconfigured_exception(const char* missing_config);
  unconfigured_exception(const unconfigured_exception& other);
  
  const char* missing_config() const;
  
private:
  std::string m_missing_config;
};

};

#endif /* defined(__UNCONFIGURED_EXCEPTION_H__) */
