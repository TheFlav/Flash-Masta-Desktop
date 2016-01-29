/*! \file
 *  \brief File containing the declaration of the \ref usb::busy_exception
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::busy_exception class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref usb::busy_exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_BUSY_EXCEPTION_H__
#define __USB_BUSY_EXCEPTION_H__

#include "exception.h"

namespace usb
{

/*!
 *  \brief Subclass of \ref usb::exception that indicates that a
 *         \ref usb::usb_device is busy.
 *
 *  Subclass of \ref usb::exception that indicates that a \ref usb::usb_device
 *  is too busy to perform the requested operation.
 *  
 *  \see usb::exception
 */
class busy_exception: public exception
{
public:
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message.
   */
  busy_exception();
};

}

#endif /* defined(__USB_BUSY_EXCEPTION_H__) */
