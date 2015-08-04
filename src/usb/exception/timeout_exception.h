//
//  timeout_exception.h
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __USB_TIMEOUT_EXCEPTION_H__
#define __USB_TIMEOUT_EXCEPTION_H__

#include "exception.h"
#include <sstream>

namespace usb
{

class timeout_exception: public exception
{
public:
  timeout_exception(unsigned int timeout);
  virtual const char* what() const throw();
  unsigned int timeout() const;
  
private:
  unsigned int m_timeout;
  static std::ostringstream m_stream;
};

};

#endif /* defined(__USB_TIMEOUT_EXCEPTION_H__) */
