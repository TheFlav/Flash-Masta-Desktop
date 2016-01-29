/*! \file
 *  \brief File containing the implementation of
 *         \ref usb::uninitialized_exception.
 *  
 *  File containing the implementation of \ref usb::uninitialized_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::uninitialized_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-04
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

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

}
