/*! \file
 *  \brief File containing the implementation of \ref usb::unopen_exception.
 *  
 *  File containing the implementation of \ref usb:unopen_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::unopen_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-04
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "unopen_exception.h"

namespace usb
{

unopen_exception::unopen_exception(const std::string& class_name)
  : exception("an object of type " + class_name + " was used without being opened"),
    m_class_name(class_name)
{
  // Nothing else to do
}

unopen_exception::unopen_exception(const char* class_name)
  : unopen_exception(std::string(class_name))
{
  // Nothing else to do
}

unopen_exception::unopen_exception(const unopen_exception& other)
  : unopen_exception(other.m_class_name)
{
  // Nothing else to do
}



const char* unopen_exception::class_name() const
{
  return m_class_name.c_str();
}

}
