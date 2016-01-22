/*! \file
 *  \brief File containing the declaration of the \ref usb::unopen_exception
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::unopen_exception class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref usb::unopen_exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-04
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_UNOPEN_EXCEPTION_H__
#define __USB_UNOPEN_EXCEPTION_H__

#include "exception.h"

namespace usb
{

/*!
 *  \brief Subclass of \ref usb::exception that indicates that a
 *         \ref usb::usb_device was not opened before an operation was attempted
 *         on it.
 *  
 *  Subclass of \ref usb::exception that indicates that a \ref usb::usb_device
 *  was not opened before an operation was attempted on it. Provides member
 *  functions to get the class name of the instance that threw this exception,
 *  which can be useful for debugging purposes.
 *  
 *  \see usb::exception
 */
class unopen_exception: public exception
{
public:
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message that is extended to include the provided class name.
   *  
   *  Class constructor. Initializes a new instance with a default message that
   *  is extended to include the provided class name.
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] class_name The name of the class that threw this exception.
   */
  explicit unopen_exception(const std::string& class_name);
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message that is extended to include the provided class name.
   *  
   *  Class constructor. Initializes a new instance with a default message that
   *  is extended to include the provided class name.
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] class_name The name of the class that threw this exception.
   */
  explicit unopen_exception(const char* class_name);
  
  /*!
   *  \brief Class copy constructor.
   *  
   *  Class copy constructor. Performs a deep copy of an existing instance of
   *  this class, allowing the new instance to be completely independent of the
   *  original instance.
   *  
   *  \param [in] other The instance to copy.
   */
  unopen_exception(const unopen_exception& other);
  
  
  
  /*!
   *  \brief Gets the name provided during construction of the class that was
   *         not opened before an operation was performed on the device.
   *  
   *  Gets the name provided during construction of the class that was not
   *  opened before an operation was performed on the device.
   *  
   *  \return The name of the class that threw the exception.
   */
  const char* class_name() const;
  
  
  
private:
  
  /*! \brief The name of the class that was provided during construction. */
  std::string m_class_name;
};

}

#endif /* defined(__USB_UNOPEN_EXCEPTION_H__) */
