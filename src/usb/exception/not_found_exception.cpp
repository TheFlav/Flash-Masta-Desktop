/*! \file
 *  \brief File containing the implementation of \ref usb::not_found_exception.
 *  
 *  File containing the implementation of \ref usb::not_found_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::not_found_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-04
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "not_found_exception.h"

namespace usb
{

not_found_exception::not_found_exception(const std::string& item_not_found)
  : exception(item_not_found + " not found"), m_item_not_found(item_not_found)
{
  // Nothing else to do
}

not_found_exception::not_found_exception(const char* item_not_found)
  : exception(std::string(item_not_found))
{
  // Nothing else to do
}

not_found_exception::not_found_exception(const not_found_exception& other)
  : exception(other.m_item_not_found)
{
  // Nothing else to do
}



const char* not_found_exception::item_not_found() const
{
  return m_item_not_found.c_str();
}

}
