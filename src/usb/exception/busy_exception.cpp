/*! \file
 *  \brief File containing the implementation of \ref usb::busy_exception.
 *  
 *  File containing the implementation of \ref usb::busy_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::busy_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "busy_exception.h"

namespace usb
{

busy_exception::busy_exception()
  : exception("device is busy")
{
  // Nothing else to do
}

}
