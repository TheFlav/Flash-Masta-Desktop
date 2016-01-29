/*! \file
 *  \brief File containing the declaration of the \ref usb::not_found_exception
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::not_found_exception class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref usb::not_found_exception class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-04
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_NOT_FOUND_EXCEPTION_H__
#define __USB_NOT_FOUND_EXCEPTION_H__

#include "exception.h"
#include <string>

namespace usb
{

/*!
 *  \brief Subclass of \ref usb::exception that indicates that a
 *         \ref usb::usb_device could not be found.
 *  
 *  Subclass of \ref usb::exception that indicates that a \ref usb::usb_device
 *  could not be found when attempting to perform an operation on it. Contains
 *  member functions for describing the item that was not found.
 *  
 *  \see usb::exception
 */
class not_found_exception: public exception
{
public:
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message and extends that message to contain the provided
   *         description of the item not found.
   *  
   *  Class constructor. Initializes a new instance with a default message and
   *  extends that message to contain the provided description of the item not
   *  found. This value can be gotten using the \ref item_not_found() member
   *  function. Must be called explicitly in order to disallow type casts.
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] item_not_found The name or a description of the item not
   *         found.
   */
  explicit not_found_exception(const std::string& item_not_found);
  
  /*!
   *  \brief Class constructor. Initializes a new instance with a default
   *         message and extends that message to contain the provided
   *         description of the item not found.
   *  
   *  Class constructor. Initializes a new instance with a default message and
   *  extends that message to contain the provided description of the item not
   *  found. This value can be gotten using the \ref item_not_found() member
   *  function.
   *  
   *  Must be called explicitly in order to disallow type casts.
   *  
   *  \param [in] item_not_found The name or a description of the item not
   *         found.
   */
  explicit not_found_exception(const char* item_not_found);
  
  /*
   *  \brief Class copy constructor.
   *  
   *  Class copy constructor. Performs a deep copy of an existing instance of
   *  this class, allowing the new instance to be completely independent of the
   *  original instance.
   *  
   *  \param [in] other The instance to copy.
   */
  not_found_exception(const not_found_exception& other);
  
  
  
  /*!
   *  \brief Gets the description of the item not found that was provided during
   *         construction.
   *  
   *  Gets the description or name of the item not found that was provided
   *  during construction.
   *  
   *  \return Immutable C-style string containing a description or the name of
   *          the item not found, provided as-is during construction.
   */
  const char* item_not_found() const;
  
  
  
private:
  
  /*! \brief String containing the name or description of the item not found. */
  std::string m_item_not_found;
};

}

#endif /* defined(__USB_NOT_FOUND_EXCEPTION_H__) */
