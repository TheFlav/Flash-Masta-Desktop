//
//  busy_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "busy_exception.h"

namespace usb
{

busy_exception::busy_exception()
  : exception("device is busy")
{
  // Nothing else to do
}

};
