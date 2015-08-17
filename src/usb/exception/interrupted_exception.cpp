//
//  interrupted_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "interrupted_exception.h"

namespace usb
{

interrupted_exception::interrupted_exception()
  : exception("interrupted by system")
{
  // Nothing else to do
}

};
