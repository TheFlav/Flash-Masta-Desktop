/*! \file
 *  \brief File containing the declaration of the \ref usb::exception class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::exception class. This file includes the minimal number
 *  of files necessary to use any instance of the \ref usb::exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_EXCEPTION_H__
#define __USB_EXCEPTION_H__

#include "../usbfwd.h"
#include <exception>
#include <stdexcept>
#include <string>

namespace usb
{

/*!
 *  \brief Custom exception class that extends the \ref std::runtime_error
 *         class.
 *  
 *  Custom usb-specific exception class that can be thrown by classes that
 *  extend the \ref usb::usb_device class, allowing for fine-grained exception
 *  handling.
 *  
 *  \see std::runtime_error
 */
class exception: public std::runtime_error
{
public:
  
  /*!
   *  \brief Class constructor, initializes the new instance with a message.
   *  
   *  Class constructor, initializes the new instance with an error message
   *  describing the nature of the error.
   *  
   *  \param [in] what Exception message describing the type of error that
   *         occured.
   */
  explicit exception(const std::string& what);
  
  /*!
   *  \brief Forwarding class constructor, invokes
   *         \ref exception(const std::string& what).
   *  
   *  Forwarding class constructor, invokes
   *  \ref exception(const std::string& what).
   *  
   *  \param [in] what Exception message describing the type of error that
   *         occured.
   *  
   *  \see exception(const std::string& what)
   */
  explicit exception(const char* what);
  
  /*
   *  \brief Class copy constructor.
   *  
   *  Class copy constructor. Performs a deep copy of an existing instance of
   *  this class, allowing the new instance to be completely independent of the
   *  original instance.
   *  
   *  \param [in] other The instance to copy.
   */
  exception(const exception& other);
  
  /*!
   *  \brief Class destructor.
   *  
   *  Class destructor. Performs any cleanup necessary for safe deletion.
   */
  virtual ~exception();
  
  
  
  /*!
   *  \brief Gets the exception description string.
   *  
   *  Gets the exception description string.
   *  
   *  This member functions guarantees that no exception will ever be thrown.
   */
  virtual const char* what() const throw();
  
  
  
private:
  
  /*! \brief The exception description. */
  std::string m_what;
};

}

#endif /* defined(__USB_EXCEPTION_H__) */
