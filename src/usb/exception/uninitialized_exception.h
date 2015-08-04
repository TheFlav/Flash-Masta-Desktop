//
//  uninitialized_exception.h
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __UNINITIALIZED_EXCEPTION_H__
#define __UNINITIALIZED_EXCEPTION_H__

#include "exception.h"
#include <string>

namespace usb
{

class uninitialized_exception: public exception
{
public:
  explicit uninitialized_exception(const std::string& class_name);
  explicit uninitialized_exception(const char* class_name);
  uninitialized_exception(const uninitialized_exception& other);
  
  const char* class_name() const;
  
private:
  std::string m_class_name;
};

};

#endif /* defined(__UNINITIALIZED_EXCEPTION_H__) */
