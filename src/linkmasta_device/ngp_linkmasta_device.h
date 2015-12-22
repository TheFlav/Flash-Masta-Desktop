/*! \file
 *  \brief File containing the declaration of the \ref ngp_linkmasta_device
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref ngp_linkmasta_device abstract class. This file includes the minimal
 *  number of files necessary to use any instance of the
 *  \ref ngp_linkmasta_device class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-24
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __NGP_LINKMASTA_DEVICE_H__
#define __NGP_LINKMASTA_DEVICE_H__

#include "usb/usbfwd.h"
#include "linkmasta_device.h"
#include "cartridge/ngp_cartridge.h"



/*! \class ngp_linkmasta_device
 *  \brief Provides functionality for communicating with a Neo Geo Pocket
 *         LinkMasta device.
 *  
 *  Class for communicating with a Neo Geo Pocket LinkMasta. Allows for easy
 *  interaction and communication with the device and abstraction of
 *  implementation details. Provides convenient methods for message passing and
 *  for interacting with any connected cartridge.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 *  
 *  \see linkmasta_device
 */
class ngp_linkmasta_device: public linkmasta_device
{
public:
  
  /*!
   *  \brief Default class constructor, assumes control of provided
   *         \ref usb::usb_device.
   *  
   *  Default class constructor. Given a valid instance of \ref usb::usb_device,
   *  assumes complete control over the usb device and abstracts interaction
   *  with said device. Upon destruction, also deletes the usb device.
   *  
   *  \param [in,out] usb_device \ref usb::usb_device to communicate with.
   *         Cannot be nullptr.
   */
                   ngp_linkmasta_device(usb::usb_device* usb_device);
  
  /*!
   *  \brief Class destructor.
   *  
   *  Class destructor. Also destroys the \ref usb::usb_device provided during
   *  construction.
   */
                   ~ngp_linkmasta_device();
  
  /*!
   *  \see linkmasta_device::init()
   */
  void             init();
  
  
  
  /*!
   *  \see linkmasta_device::is_open()
   */
  bool             is_open() const;
  
  /*!
   *  \see linkamsta_device::timeout()
   */
  timeout_t        timeout() const;
  
  /*!
   *  \see linkmasta_device::firmware_version()
   */
  version_t        firmware_version();
  
  /*!
   *  \see linkmasta_device::is_integrated_with_cartridge()
   */
  bool             is_integrated_with_cartridge() const;
  
  
  
  /*!
   *  \see linkmasta_device::set_timeout(timeout_t timeout)
   */
  void             set_timeout(timeout_t timeout);
  
  /*!
   *  \see linkmasta_device::open()
   */
  void             open();
  
  /*!
   *  \see linkmasta_device::close()
   */
  void             close();
  
  /*!
   *  \see linkmasta_device::read_word(chip_index chip, address_t address)
   */
  word_t           read_word(chip_index chip, address_t address);
  
  /*!
   *  \see linkmasta_device::write_word(chip_index chip, address_t address, word_t data)
   */
  void             write_word(chip_index chip, address_t address, word_t data);
  
  /*!
   *  \see linkmasta_device::test_for_cartridge()
   */
  bool             test_for_cartridge();
  
  /*!
   *  \see linkmasta_device::build_cartridge()
   */
  cartridge*       build_cartridge();
  
  
  
  /*!
   *  \see linkmasta_device::system()
   */
  linkmasta_system system() const;
  
  /*!
   *  \return true
   *  
   *  \see linkmasta_device::supports_read_bytes()
   */
  bool             supports_read_bytes() const;
  
  /*!
   *  \return true
   *  
   *  \see linkmasta_device::supports_program_bytes()
   */
  bool             supports_program_bytes() const;
  
  
  
  /*!
   *  \see linkmasta_device::read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr)
   */
  unsigned int     read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr);
  
  /*!
   *  \see linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr)
   */
  unsigned int     program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr);
  
  
  
private:
  
  /*!
   *  \brief Fetches the firmware version directly from the associated LinkMasta
   *         device through USB.
   *  
   *  Fetches the firmware version directly from the associated LinkMasta device
   *  through USB and caches the received results for quick access using
   *  \ref firmware_version(). This function should only ever need to be called
   °  once during initalization.
   *  
   *  This is a blocking function that can take several seconds to complete.
   */
  void             fetch_firmware_version();
  
  
  
  /*!
   *  \brief The designated \ref usb::usb_device with which this object is to
   *         interact.
   */
  usb::usb_device* const m_usb_device;
  
  /*! \brief Flag indicating that the device has been previously initialized. */
  bool             m_was_init;
  
  /*! \brief Flag indicating that the device is currently open. */
  bool             m_is_open;
  
  /*!
   *  \brief Flag indicating that the firmware version has been previously
   *         cached via a call to \ref fetch_firmware_version().
   */
  bool             m_firmware_version_set;
  
  /*! \brief Cached firmware major version. */
  unsigned int     m_firmware_major_version;
  
  /*! \brief Cached firmware minor version. */
  unsigned int     m_firmware_minor_version;
};

#endif /* defined(__NGP_LINKMASTA_DEVICE_H__) */
