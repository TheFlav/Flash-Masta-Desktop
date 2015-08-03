//
//  usb_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "usb_exception.h"

usb_exception::usb_exception(const char* what)
  : runtime_error(what)
{
  // Nothing else to do
}
