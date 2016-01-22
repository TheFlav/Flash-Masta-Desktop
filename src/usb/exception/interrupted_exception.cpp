/*! \file
 *  \brief File containing the implementation of
 *         \ref usb::interrupted_exception.
 *  
 *  File containing the implementation of \ref usb::interrupted_exception.
 *  
 *  See corrensponding header file to view documentation for class, its methods,
 *  and its member variables.
 *  
 *  \see usb::interrupted_exception
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "interrupted_exception.h"

namespace usb
{

interrupted_exception::interrupted_exception()
  : exception("interrupted by system")
{
  // Nothing else to do
}

}
