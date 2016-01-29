/*! \file
 *  \brief File containing the implementation of \ref usb::exception.
 *  
 *  File containing the implementation of \ref usb::exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "exception.h"

namespace usb
{

#define DEFAULT_WHAT "a USB error occured"

exception::exception(const std::string& what)
: runtime_error(DEFAULT_WHAT), m_what(std::string(DEFAULT_WHAT) + ": " + what)
{
  // Nothing else to do
}

exception::exception(const char* what)
  : exception(std::string(what))
{
  // Nothing else to do
}

exception::exception(const exception& other)
  : runtime_error(DEFAULT_WHAT), m_what(other.what())
{
  // Nothing else to do
}

exception::~exception()
{
  // Nothing else to do
}



const char* exception::what() const throw()
{
  return m_what.c_str();
}

}
