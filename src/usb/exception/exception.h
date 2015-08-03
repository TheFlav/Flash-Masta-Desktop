//
//  exception.h
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __USB_EXCEPTION_H__
#define __USB_EXCEPTION_H__

#include "../usbfwd.h"
#include <exception>
#include <stdexcept>

namespace usb
{

class exception: public std::runtime_error
{
public:
  exception(const char* what);
  exception();
  virtual ~exception();
  virtual const char* what() const throw();
  
private:
  char* const m_what;
};

};

#endif /* defined(__USB_EXCEPTION_H__) */
