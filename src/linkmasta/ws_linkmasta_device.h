/*! \file
 *  \brief File containing the declaration of the \ref ws_linkmasta_device
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref ws_linkmasta_device class. This file includes the minimal number of
 *  files necessary to use any instance of the \ref ngp_linkmasta_device class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-14
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __WS_LINKMASTA_DEVICE_H__
#define __WS_LINKMASTA_DEVICE_H__

#include "linkmasta_device.h"
#include "usb/usbfwd.h"



/*! \class ws_linkmasta_device
 *  \brief Provides functionality for communicating with a Wonderswan LinkMasta
 *         device.
 *  
 *  Class for communicating with a Wonderswan LinkMasta. Allows for easy
 *  interaction and communication with the device and abstraction of
 *  implementation details. Provides convenient methods for message passing and
 *  for interacting with any connected cartridge.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 *  
 *  \see linkamsta_device
 */
class ws_linkmasta_device: public linkmasta_device
{
public:
  
  /*!
   *  \brief Default class constructor, assumes control of provided
   *         \ref usb::usb_device.
   *  
   *  Default class constructor. Given a valid instance of \ref usb::usb_device,
   *  assumes complete control over the usb device and abstracts intraction with
   *  said device. Upon destruction, also deletes the usb device.
   *  
   *  \param [in,out] usb_device \ref usb::usb_device to communicate with.
   *         Cannot be nullptr.
   */
                   ws_linkmasta_device(usb::usb_device* usb_device);
  
  /*!
   *  \brief Class destructor.
   *  
   *  Class destructor. Also destorys the \ref usb::usb_device provided during
   *  construction.
   */
                   ~ws_linkmasta_device();
  
  /*!
   *  \see linkmasta_device::init()
   */
  void             init();
  
  
  
  /*!
   *  \see linkmasta_device::is_open()
   */
  bool             is_open() const;
  
  /*!
   *  \see linkmasta_device::timeout()
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
   *  \see linkamstas_device::close()
   */
  void             close();
  
  /*!
   *  \see linkmasta_device::read_word(chip_index chip, address_t address)
   */
  word_t           read_word(chip_index chip, address_t address);
  
  /*!
   *  \see linkamsta_device::write_word(chip_index chip, address_t address, word_t data)
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
   *  \return true
   *  
   *  \see linkmasta_device::supports_read_num_slots()
   */
  bool             supports_read_num_slots() const;
  
  /*!
   *  \return true
   *  
   *  \see supports_read_slot_size()
   */
  bool             supports_read_slot_size() const;
  
  /*!
   *  \return true
   *  
   *  \see supports_switch_slot()
   */
  bool             supports_switch_slot() const;
  
  
  
  /*!
   *  \see linkmasa_device::read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr)
   */
  unsigned int     read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr);
  
  /*!
   *  \see linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr)
   */
  unsigned int     program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr);
  
  /*!
   *  \see linkmasta_device::read_num_slots()
   */
  unsigned int     read_num_slots();
  
  /*!
   *  \see linkmasta_device::read_slot_size(unsigned int slot_num)
   */
  unsigned int     read_slot_size(unsigned int slot_num);
  
  /*!
   *  \see linkmasta_device::switch_slot(unsigned int slot_num)
   */
  bool             switch_slot(unsigned int slot_num);
  
  
  
private:
  
  /*!
   *  \brief Fetches the firmware version directly from the associated LinkMasta
   *         device through USB.
   *  
   *  Fetches the firmware version directly fromt he associated LinkMatsa device
   *  through USB and caches the received results for quick access using
   *  \ref firmware_version(). This function should only ever need to be called
   *  once during initialization.
   *  
   *  This is a blocking function that can take several seconds to complete.
   */
  void             fetch_firmware_version();
  
  /*!
   *  \brief Fetches slot layout information from the cartridge connected to the
   *         associated LinkMasta.
   *  
   *  Fetches slot layout information from the cartridge connected to the
   *  associated LinkMasta. If no cartridge is connected to the LinkMasta when
   *  this method is called, the result is undefined. The results of this
   *  operation is cached and can be accessed using \ref read_num_slots()
   *  and \ref read_slot_size(unsigned int slot_num).
   *  
   *  This is a blocking function that can take several seconds to complete.
   */
  void             fetch_slot_info();
  
  
  
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
  
  /*!
   *  \brief Flag indicating that the slot information has been previosuly
   *         cached via a call to \ref fetch_slot_info().
   */
  bool             m_slot_info_set;
  
  
  /*! \brief Cached firmware major version. */
  unsigned int     m_firmware_major_version;
  
  /*! \brief Cached firmware minor version. */
  unsigned int     m_firmware_minor_version;
  
  /*! \brief Cached flag indicating if number of slots will never change. */
  bool             m_static_num_slots;
  
  /*! \brief Cached flag indicating if sizes of slots will never change. */
  bool             m_static_slot_sizes;
  
  /*! \brief Cached number of slots on cartridge (if static). */
  unsigned int     m_num_slots;
  
  /*! \brief Cached size of slots on cartridge (if uniform and static). */
  unsigned int     m_slot_size;
};

#endif /* defined(__WS_LINKMASTA_DEVICE_H__) */
