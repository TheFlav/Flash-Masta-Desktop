/*! \file
 *  \brief File containing includes necessary for accessing all classes and
 *         all functionality in the USB module.
 *  
 *  File containing includes necessary for accessing all classes and all
 *  functionality in the USB module. If this file is included in another file,
 *  then no other files in this module will need to be included in order to have
 *  access to the full functionality of this module.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_H__
#define __USB_H__


#include "usbfwd.h"
#include "usb_device.h"
#include "libusb_usb_device.h"

#include "exception/exception.h"
#include "exception/busy_exception.h"
#include "exception/disconnected_exception.h"
#include "exception/interrupted_exception.h"
#include "exception/not_found_exception.h"
#include "exception/timeout_exception.h"
#include "exception/uninitialized_exception.h"
#include "exception/unconfigured_exception.h"
#include "exception/unopen_exception.h"


#endif /* defined(__USB_H__) */
