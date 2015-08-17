//
//  uninitialized_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "uninitialized_exception.h"

namespace usb
{

uninitialized_exception::uninitialized_exception(const std::string& class_name)
  : exception("object of type " + class_name + " used without being initialized"), m_class_name(class_name)
{
  // Nothing else to do
}

uninitialized_exception::uninitialized_exception(const char* class_name)
  : uninitialized_exception(std::string(class_name))
{
  // Nohing else to do
}
                            
uninitialized_exception::uninitialized_exception(const uninitialized_exception& other)
  : uninitialized_exception(std::string(other.m_class_name))
{
  // Nothing else to do
}

const char* uninitialized_exception::class_name() const
{
  return m_class_name.c_str();
}

};
