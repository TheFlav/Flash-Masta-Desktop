/*! \file
 *  \brief File containing the bare minimum forward declarations necessary to
 *         work with objects in the USB module.
 *  
 *  File containing the bare minimum forward declarations necessary to work with
 *  objects in the USB module. Forward declares classes that are contained in
 *  this class.
 *  
 *  This file containins only forward declarations, but no detailes about the
 *  classes themselves or the methods and properties they contain. This file is
 *  to be used to forward declare classes so that other header files can declare
 *  pointers to classes in this module without knowing the class details.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USBFWD_H__
#define __USBFWD_H__

namespace usb
{

class usb_device;
class libusb_usb_device;

class exception;
class busy_exception;
class disconnected_exception;
class interrupted_exception;
class not_found_exception;
class timeout_exception;
class unconfigured_exception;
class uninitialized_exception;
class unopen_exception;

}

#endif /* defined(__USBFWD_H__) */
