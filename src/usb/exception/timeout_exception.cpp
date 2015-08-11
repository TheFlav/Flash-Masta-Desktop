//
//  timeout_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "timeout_exception.h"

using namespace usb;

std::ostringstream timeout_exception::m_stream;

timeout_exception::timeout_exception(unsigned int timeout)
  : exception("operation exceeded timeout"), m_timeout(timeout)
{
  // Nothing else to do
}

const char* timeout_exception::what() const throw()
{
  m_stream.str("");
  m_stream << exception::what() << ": " << timeout() << " milliseconds";
  return m_stream.str().c_str();
}

unsigned int timeout_exception::timeout() const
{
  return m_timeout;
}
