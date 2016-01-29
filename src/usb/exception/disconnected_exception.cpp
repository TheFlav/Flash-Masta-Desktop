/*! \file
 *  \brief File containing the implementation of
 *         \ref usb::disconnected_exception.
 *  
 *  File containing the implementation of \ref usb::disconnected_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::disconnected_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "disconnected_exception.h"

namespace usb
{

disconnected_exception::disconnected_exception()
  : exception("device disconneced mid-operation")
{
  // Nothing else to do
}

}
