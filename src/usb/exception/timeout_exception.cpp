/*! \file
 *  \brief File containing the implementation of \ref usb::timeout_exception.
 *  
 *  File containing the implementation of \ref usb::timeout_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::timeout_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "timeout_exception.h"

namespace usb
{

std::ostringstream timeout_exception::m_stream;

timeout_exception::timeout_exception(unsigned int timeout)
  : exception("operation exceeded timeout"), m_timeout(timeout)
{
  // Nothing else to do
}

timeout_exception::timeout_exception(const timeout_exception& other)
  : timeout_exception(other.m_timeout)
{
  // Nothing else to do
}



const char* timeout_exception::what() const throw()
{
  m_stream.str("");
  m_stream << exception::what() << ": " << timeout() << " milliseconds";
  return m_stream.str().c_str();
}

unsigned int timeout_exception::timeout() const throw()
{
  return m_timeout;
}

}
