//
//  interrupted_exception.h
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __INTERRUPTED_EXCEPTION_H__
#define __INTERRUPTED_EXCEPTION_H__

#include "exception.h"

namespace usb
{

class interrupted_exception: public exception
{
public:
  interrupted_exception();
};

};

#endif /* defined(__INTERRUPTED_EXCEPTION_H__) */
