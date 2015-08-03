//
//  disconnected_exception.h
//  FlashMasta
//
//  Created by Dan on 8/3/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __DISCONNECTED_EXCEPTION_H__
#define __DISCONNECTED_EXCEPTION_H__

#include "exception.h"

namespace usb
{

class disconnected_exception: public exception
{
public:
  disconnected_exception();
};

};

#endif /* defined(__DISCONNECTED_EXCEPTION_H__) */
