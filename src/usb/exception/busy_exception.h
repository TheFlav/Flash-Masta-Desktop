//
//  busy_exception.h
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __BUSY_EXCEPTION_H__
#define __BUSY_EXCEPTION_H__

#include "exception.h"

namespace usb
{

class busy_exception: public exception
{
public:
  busy_exception();
};

};

#endif /* defined(__BUSY_EXCEPTION_H__) */
