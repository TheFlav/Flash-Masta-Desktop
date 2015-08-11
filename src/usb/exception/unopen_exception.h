//
//  unopen_exception.h
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __UNOPEN_EXCEPTION_H__
#define __UNOPEN_EXCEPTION_H__

#include "exception.h"

namespace usb
{

class unopen_exception: public exception
{
public:
  explicit unopen_exception(const std::string& class_name);
  explicit unopen_exception(const char* class_name);
  unopen_exception(const unopen_exception& other);
  
  const char* class_name() const;
  
private:
  std::string m_class_name;
};

};

#endif /* defined(__UNOPEN_EXCEPTION_H__) */
