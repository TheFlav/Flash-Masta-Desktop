//
//  disconnected_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "disconnected_exception.h"

namespace usb
{

disconnected_exception::disconnected_exception()
  : exception("device disconneced mid-operation")
{
  // Nothing else to do
}

};
