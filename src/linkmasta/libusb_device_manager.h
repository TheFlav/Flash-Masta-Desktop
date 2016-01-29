/*! \file
 *  \brief File containing the declaration of the \ref libusb_device_manager
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref libusb_device_manager class. This file includes the minimal number of
 *  files necessary to use any instance of the \ref libusb_device_manager class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-09-08
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __LIBUSB_DEVICE_MANAGER_H__
#define __LIBUSB_DEVICE_MANAGER_H__

#include "device_manager.h"

#include <map>
#include <string>

struct libusb_context;
struct libusb_device;



/*!
 *  \brief Complete implementation of \ref device_manager using the libusb
 *         usb library.
 *  
 *  Complete implementation of the \ref device_manager using the libusb library
 *  for device monitoring.
 */
class libusb_device_manager : public device_manager
{
public:
  
  /*!
   *  \brief Class constructor. Initializes libraries and member variables.
   *  
   *  Class constructor. Initializes libraries and member variables.
   */
                            libusb_device_manager();
  
  /*!
   *  \brief Class destructor. Frees dynamic memory and deinitializes libraries.
   *  
   *  Class destructor. Frees dynamic memory and deinitalizes libraries.
   */
                            ~libusb_device_manager();
  
  
  
  /*!
   *  \see device_manager::get_connected_devices()
   */
  std::vector<unsigned int> get_connected_devices();
  
  /*!
   *  \see device_manager::try_get_connected_devices(std::vector<unsigned int>&)
   */
  bool                      try_get_connected_devices(std::vector<unsigned int>& devices);
  
  /*!
   *  \see device_manager::is_connected(unsigned int)
   */
  bool                      is_connected(unsigned int id);
  
  /*!
   *  \see device_manager::get_vendor_id(unsigned int)
   */
  unsigned int              get_vendor_id(unsigned int id);
  
  /*!
   *  \see device_manager::get_product_id(unsigned int)
   */
  unsigned int              get_product_id(unsigned int id);
  
  /*!
   *  \see device_manager::get_manufacturer_string(unsigned int)
   */
  std::string               get_manufacturer_string(unsigned int id);
  
  /*!
   *  \see device_manager::get_product_string(unsigned int)
   */
  std::string               get_product_string(unsigned int id);
  
  /*!
   *  \see device_manager::get_serial_number(unsigned int)
   */
  std::string               get_serial_number(unsigned int id);
  
  /*!
   *  \see device_manager::get_linkmasta_device(unsigned int)
   */
  linkmasta_device*         get_linkmasta_device(unsigned int id);
  
  /*!
   *  \see device_manager::is_device_claimed(unsigned int)
   */
  bool                      is_device_claimed(unsigned int id);
  
  /*!
   *  \see device_manager::try_claim_device(unsigned int)
   */
  bool                      try_claim_device(unsigned int id);
  
  /*!
   *  \see device_manager::release_device(unsigned int)
   */
  void                      release_device(unsigned int id);
  
  
  
protected:
  
  /*!
   *  \see device_manager::refresh_device_list()
   */
  void                      refresh_device_list();
  
  
  
private:
  
  /*!
   *  \brief Checks to see if a device with the supplied vendor id and product
   *         id is officially supported by this class.
   *  
   *  Checks to see if a device with the supplied vendor id and product id is
   *  officially supported by this class. This function should identify
   *  whether or not such a device is recognized as a LinkMasta device that this
   *  manager supports.
   *  
   *  \param [in] vendor_id The vendor id of the usb device.
   *  \param [in] product_id The product id of the usb device.
   *  
   *  \return true if this manager can support this device, false if not.
   */
  static bool               is_supported(unsigned int vendor_id, unsigned int product_id);
  
  
  
private:
  
  /*! \brief Internal libusb handle. */
  libusb_context*           m_libusb;
  
  /*! \brief Asynchronous lock to prevent concurrent accesses to libusb. */
  std::mutex                m_libusb_mutex;
  
  /*! \brief Flag indicating that the libusb library has been initalized. */
  bool                      m_libusb_init;
  
  /*!
   *  \brief Struct containing data about a connected device.
   *  
   *  Struct containing data about a connected device so that metadata about the
   *  device can be cached to prevent unnecessary operations.
   */
  struct                    connected_device
  {
    /*! \brief Internal id of the device as provided by \ref generate_id(). */
    unsigned int              id;
    
    /*! \brief USB vendor ID of the device. */
    unsigned int              vendor_id;
    
    /*! \brief USB product ID of the device. */
    unsigned int              product_id;
    
    /*! \brief USB device manufacutrer string. */
    std::string               manufacturer_string;
    
    /*! \brief USB device product string. */
    std::string               product_string;
    
    /*! \brief USB device serial number string. */
    std::string               serial_number;
    
    /*! \brief Pointer to libusb handle. */
    libusb_device*            device;
    
    /*! \brief Pointer to generated \ref linkmasta_device object. */
    linkmasta_device*         linkmasta;
    
    /*! \brief Flag indicating device is currently claimed. */
    bool                      claimed;
  };
  
  /*! \brief Map for keeping track of and accessing connected devices. */
  std::map<unsigned int, connected_device> m_connected_devices;
  
  /*! \brief Mutex for locking the \ref m_connected_devices map. */
  std::mutex                m_connected_devices_mutex;
  
  /*!
   *  \brief List of devices that have been disconnected and need to be removed
   *  from the \ref m_connected_devices map.
   */
  std::vector<connected_device> m_disconnected_devices;
};

#endif /* defined(__LIBUSB_DEVICE_MANAGER_H__) */
