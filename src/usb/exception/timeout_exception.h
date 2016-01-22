/*! \file
 *  \brief File containing the declaration of the \ref usb::timeout_exception
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::timeout_exception class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref usb::timeout_exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_TIMEOUT_EXCEPTION_H__
#define __USB_TIMEOUT_EXCEPTION_H__

#include "exception.h"
#include <sstream>

namespace usb
{

/*!
 *  \brief Subclass of \ref usb::exception that indicates that an operation on a
 *         \ref usb::usb_device has taken longer than the alotted time to
 *         respond.
 *  
 *  Subclass of \ref usb::exception that indicates that an operation on a
 *  \ref usb::usb_device has taken longer than the alotted time to respond.
 *  Provides member functions for getting the timeout constraint that was
 *  violated.
 *  
 *  \see usb::exception
 */
class timeout_exception: public exception
{
public:
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message, extended with the provided timeout constraint tha was
   *         violated.
   *  
   *  Class constructor. Initializes a new instance with a default message,
   *  extended with the provided timeout constraint that was violated. This
   *  value can be retrieved via a call to \ref timeout()
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] timeout The timeout constraint (in milliseconds) that was
   *         violated by the operation.
   */
  explicit timeout_exception(unsigned int timeout);
  
  /*!
   *  \brief Class copy constructor.
   *  
   *  Class copy constructor. Performs a deep copy of an existing instance of
   *  this class, allowing the new instance to be completely independent of the
   *  original instance.
   *  
   *  \param [in] other The instance to copy.
   */
  timeout_exception(const timeout_exception& other);
  
  
  
  /*!
   *  \brief Gets the exception description string.
   *  
   *  Gets the exception description string, including the value of the timeout
   *  constraint provided during construction, represented in milliseconds.
   *  
   *  This member functions guarantees that no exception will ever be thrown.
   */
  virtual const char* what() const throw();
  
  /*!
   *  \brief Gets the timeout constraint provided during construction.
   *  
   *  Gets the value of the timeout constraint provided during construction.
   *  This value should be a positive integer representing the number of
   *  milliseconds that an operation was limited to.
   */
  unsigned int timeout() const throw();
  
  
  
private:
  
  /*! \brief The value of the timeout constraint. */
  unsigned int m_timeout;
  
  /*!
   *  \brief A private output string stream used for building the exception
   *         message provided by \ref what().
   */
  static std::ostringstream m_stream;
};

}

#endif /* defined(__USB_TIMEOUT_EXCEPTION_H__) */
