/*! \file
 *  \brief File containing the declaration of the
 *         \ref usb::disconnected_exception class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::disconnected_exception class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref usb::disconnected_exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_DISCONNECTED_EXCEPTION_H__
#define __USB_DISCONNECTED_EXCEPTION_H__

#include "exception.h"

namespace usb
{

/*!
 *  \brief Subclass of \ref usb::exception that indicates that a
 *         \ref usb::usb_device has been disconnected from the system.
 *  
 *  Subclass of \ref usb::exception that indicates that a \ref usb::usb_device
 *  cannot be accessed because it has been disconnected from the system. This
 *  exception can indicate that the device was never connected to the system,
 *  that it was disconnected mid-operation, or that it was disconnected between
 *  operations.
 *  
 *  \see usb::exception
 */
class disconnected_exception: public exception
{
public:
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message.
   */
  disconnected_exception();
};

}

#endif /* defined(__USB_DISCONNECTED_EXCEPTION_H__) */
