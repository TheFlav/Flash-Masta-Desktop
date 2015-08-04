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
#include <string>

namespace usb
{

class exception: public std::runtime_error
{
public:
  explicit exception(const std::string& what);
  explicit exception(const char* what);
  exception(const exception& other);
  virtual ~exception() {};
  virtual const char* what() const throw();
  
private:
  std::string m_what;
};

};

#endif /* defined(__USB_EXCEPTION_H__) */
