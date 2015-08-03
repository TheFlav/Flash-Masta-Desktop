//
//  usb_exception.h
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __USB_EXCEPTION_H__
#define __USB_EXCEPTION_H__

#include <exception>
#include <stdexcept>

class usb_exception: public std::runtime_error
{
public:
  usb_exception(const char* what);
};

#endif /* defined(__USB_EXCEPTION_H__) */
