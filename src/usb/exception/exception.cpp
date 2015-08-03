//
//  exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "exception.h"
#include <cstring>

using namespace usb;

#define DEFAULT_WHAT "a USB error occured"

exception::exception(const char* what)
  : runtime_error(DEFAULT_WHAT),
    m_what(new char [strlen(DEFAULT_WHAT) + strlen(": ") + strlen(what)])
{
  // Pre-build "what"
  strcpy(m_what, DEFAULT_WHAT);
  strcat(m_what, ": ");
  strcat(m_what, what);
}

exception::exception()
  : runtime_error(DEFAULT_WHAT),
    m_what(new char [strlen(DEFAULT_WHAT)])
{
  strcmp(m_what, DEFAULT_WHAT);
}

exception::~exception()
{
  delete m_what;
}

const char* exception::what() const throw()
{
  return m_what;
}
