/*! \file
 *  \brief File containing the declaration of the
 *         \ref usb::unconfigured_exception class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::unconfigured_exception class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref usb::unconfigured_exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-04
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_UNCONFIGURED_EXCEPTION_H__
#define __USB_UNCONFIGURED_EXCEPTION_H__

#include "exception.h"
#include <string>

namespace usb
{

/*!
 *  \brief Subclass of \ref usb::exception that indicates that a
 *         \ref usb::usb_device was not configured before an operation was
 *         attempted on it.
 *  
 *  Subclass of \ref usb::exception that indicates that a \ref usb::usb_device
 *  was not configured before an operation was attempted on it. Provides member
 *  functions to get a description of the specific configuration property that
 *  was neglected to cause this exception.
 *  
 *  \see usb::exception
 */
class unconfigured_exception: public exception
{
public:
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message that is extended to include the provided configuration
   *         description.
   *  
   *  Class constructor. Initializes a new instance with a default message that
   *  is extended to include the provided configuration description.
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] missing_config The name or a short description of the
   *         configuration that was neglected to produce this exception. This
   *         value can be retrieved again via a call to \ref missing_config().
   */
  explicit unconfigured_exception(const std::string& missing_config);
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message that is extended to include the provided configuration
   *         description.
   *  
   *  Class constructor. Initializes a new instance with a default message that
   *  is extended to include the provided configuration description.
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] missing_config The name or a short description of the
   *         configuration that was neglected to produce this exception. This
   *         value can be retrieved again via a call to \ref missing_config().
   */
  explicit unconfigured_exception(const char* missing_config);
  
  /*!
   *  \brief Class copy constructor.
   *  
   *  Class copy constructor. Performs a deep copy of an existing instance of
   *  this class, allowing the new instance to be completely independent of the
   *  original instance.
   *  
   *  \param [in] other The instance to copy.
   */
  unconfigured_exception(const unconfigured_exception& other);
  
  
  
  /*!
   *  \brief Gets the name or description provided during construction of the
   *         missing configuration that caused this excpetion to occur.
   *  
   *  Gets the name or description provided during construction of the missing
   *  configuration that caused this excpetion to occur.
   *  
   *  \return The name or description of the missing configuration.
   */
  const char* missing_config() const;
  
  
  
private:
  
  /*!
   *  \brief Name or short description of the neglected configuration provided
   *         during construction.
   */
  std::string m_missing_config;
};

}

#endif /* defined(__USB_UNCONFIGURED_EXCEPTION_H__) */
