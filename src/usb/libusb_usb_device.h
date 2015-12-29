/*! \file
 *  \brief File containing the declaration of the \ref usb::libusb_usb_device
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::libusb_usb_device class. This file includes the minimal number of
 *  files necessary to use any instance of the \ref usb::libusb_usb_device
 *  class.
 *
 *  \author Daniel Andrus
 *  \date 2015-08-05
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __LIBUSB_USB_DEVICE_H__
#define __LIBUSB_USB_DEVICE_H__

#include "usbfwd.h"
#include "usb_device.h"

struct libusb_device;
struct libusb_device_handle;
struct libusb_config_descriptor;
struct libusb_interface;
struct libusb_interface_descriptor;

namespace usb
{
 
/*! \class libusb_usb_device
 *  \brief Libusb-based implementation of the \ref usb_device class.
 *  
 *  Libusb-based implementation of te \ref usb_device class. Provies an
 *  object-oriented approach to interacting with USB devices using the C-based
 *  library Libusb.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class libusb_usb_device : public usb_device
{
public:
  
  /*!
   *  \brief Main constructor for the class.
   *  
   *  The main constructor for the class. Requires a \ref libusb_device
   *  struct (provided by the Libusb library) to be used as handle to the
   *  hardware device.
   *  
   *  \param [in] device The handle to the USb device to use for communicating
   *         with Libusb.
   */
  libusb_usb_device(libusb_device* device);
  
  /*!
   *  \brief The destructor for the class.
   *  
   *  The destructor for the class. Closes open connections and releases any
   *  dynamically allocated resources.
   */
  ~libusb_usb_device();
  
  /*!
   *  \see usb_device::init() 
   */
  void init();
  
  
  
  /*!
   *  \see usb_device::timeout()
   */
  timeout_t timeout() const;
  
  /*!
   *  \see usb_device::configuration()
   */
  configuration_t configuration() const;
  
  /*!
   *  \see usb_device::interface()
   */
  interface_t interface() const;
  
  /*!
   *  \see usb_device::input_endpoint()
   */
  endpoint_t input_endpoint() const;
  
  /*!
   *  \see usb_device::output_endpoint()
   */
  endpoint_t output_endpoint() const;
  
  /*!
   *  \see usb_device::get_device_description()
   */
  const device_description* get_device_description() const;
  
  /*!
   *  \see usb_device::get_manufacturer_string()
   */
  std::string get_manufacturer_string();
  
  /*!
   *  \see usb_device::get_product_string()
   */
  std::string get_product_string();
  
  /*!
   *  \see usb_device::get_serial_number()
   */
  std::string get_serial_number();
  
  
  
  /*!
   *  \see usb_device::set_timeout(timeout_t timeout)
   */
  void set_timeout(timeout_t timeout);
  
  /*!
   *  \see usb_device::set_configuration(configuration_t configuration)
   */
  void set_configuration(configuration_t configuration);
  
  /*!
   *  \see usb_device::set_interface(interface_t interface)
   */
  void set_interface(interface_t interface);
  
  /*!
   *  \see usb_device::set_input_endpoint(endpoint_t input_endpoint)
   */
  void set_input_endpoint(endpoint_t input_endpoint);
  
  /*!
   *  \see usb_device::set_output_endpoint(endpoint_t output_endpoint)
   */
  void set_output_endpoint(endpoint_t output_endpoint);
  
  
  
  /*!
   *  \see usb_device::open()
   */
  void open();
  
  /*!
   *  \see usb_device::close()
   */
  void close();
  
  /*!
   *  \see usb_device::read(data_t* data, unsigned int num_bytes)
   */
  unsigned int read(data_t* data, unsigned int num_bytes);
  
  /*!
   *  \see usb_device::read(data_t* data, unsigned int num_bytes, timeout_t timeout)
   */
  unsigned int read(data_t* data, unsigned int num_bytes, timeout_t timeout);
  
  /*!
   *  \see usb_device::write(const data_t* buffer, unsigned int num_bytes)
   */
  unsigned int write(const data_t* buffer, unsigned int num_bytes);
  
  /*!
   *  \see usb_device::write(const data_t* buffer, unsigned int num_bytes, timeout_t timeout)
   */
  unsigned int write(const data_t* buffer, unsigned int num_bytes, timeout_t timeout);
  
  
  
private:
  
  /*!
   *  \brief Builds the device's \ref device_description descriptor.
   *  
   *  Builds the device's \ref device_description descriptor.
   *  
   *  \return Pointer to the newly created \ref device_description object. If
   *          an error occured, will return nullptr instead.
   */
  device_description*   build_device_description();
  
  /*!
   *  \brief Builds one of the device's \ref device_configuration descriptors.
   *  
   *  Builds one of the device's \ref device_configuration descriptors.
   *  
   *  \param [in] index The index of the configuration to build a descriptor
   *         for.
   *  
   *  \return A pointer to the new descriptor. If an error occured or the
   *          provided parameters are invalid, will return nullptr instead.
   */
  device_configuration* build_device_config(unsigned int index);
  
  /*!
   *  \brief Builds one of the device's \ref device_interface descriptors.
   *  
   *  Builds one of the device's \ref device_interface descriptors.
   *  
   *  \param [in] config The libusb configuration descriptor to which the
   *         requested interface belongs.
   *  \param [in] index The index of the interface under the given
   *         configuration.
   *  
   *  \return Pointer to the new interface descriptor. If an error occured or
   *          a parameter was invalid, will return nullptr instead.
   */
  device_interface*     build_device_interface(const libusb_config_descriptor* config, unsigned int index);
  
  /*!
   *  \brief Builds one of the device's \ref device_alt_setting descriptors.
   *  
   *  Builds one of the device's \ref device_alt_setting descriptors.
   *  
   *  \param [in] interface The libusb interface descriptor to which the new
   *         alternate setting belongs.
   *  \param [in] index The index of the alternate setting under the given
   *         interface.
   *  
   *  \return Pointer to the new alternate setting descriptor. If an error
   *          occured or a parameter was invalid, will return nullptr instead.
   */
  device_alt_setting*   build_device_alt_setting(const libusb_interface* interface, unsigned int index);
  
  /*!
   *  \brief Builds one of the device's \ref device_endpoint descriptors.
   *  
   *  Builds one of the device's \ref device_endpoint descriptors.
   *  
   *  \param [in] interface The libusb interface descriptor to which the new
   *         endpoint belongs.
   *  \param [in] index The index of the endpoint under the given interface.
   *  
   *  \return Pointer to the new endpoint descriptor. If an error occured or a
   *          parameter was invalid, will return nullptr instead.
   */
  device_endpoint*      build_device_endpoint(const libusb_interface_descriptor* interface, unsigned int index);
  
  /*!
   *  \brief Detects if the given integer indicates that an error has occured.
   *  
   *  Detects if the given integer indicates that an error has occured during
   *  a previous Libusb operation. Compares the given error code against a table
   *  of known error codes to determine if the code represents than an error has
   *  occured.
   *  
   *  \param [in] libusb_error Libusb error code to compare against known error
   *         codes to determine if the code means an error has occured.
   *  
   *  \return True if \ref libusb_error is indicative of an error, or false if
   *          all is well.
   */
  static bool           libusb_error_occured(int libusb_error);
  
  /*!
   *  \brief Given a Libusb error code, throws the corresponding
   *         \ref usb::exception exception.
   *  
   *  Given a Libusb error code, throws the corresponding \ref usb::exception
   *  exception. If \ref libusb_error indicates that no error has occured, then
   *  no exception will be thrown.
   *  
   *  \param [in] libusb_error Libusb error code representing the type of error
   *         that has occured.
   *  \param [in] timeout If relevant, the timeout value of the operation that
   *         encountered the error. This value is only used if the error is a
   *         type is related to the timeout value.
   */
  static void           throw_libusb_exception(int libusb_error, timeout_t timeout);
  
  
  
  /*! \brief Flag indicating that the object has been initalized. */
  bool                  m_was_initialized;
  
  /*! \brief Flag indicating that the connection to the USB device is open. */
  bool                  m_is_open;
  
  /*!
   *  \brief Flag indicating that the system's kernel was previously attached to
   *         the device and should be reattached when the connection is closed.
   */
  bool                  m_kernel_was_attached;
  
  /*! \brief Flag indicating that the timeout value has been previously set. */
  bool                  m_timeout_set;
  
  /*! \brief Flag indicating that the configuration has been previously set. */
  bool                  m_configuration_set;
  
  /*! \brief Flag indicating that the interface has been previously set. */
  bool                  m_interface_set;
  
  /*! \brief Flag indicating that the input endpoint has been manually set. */
  bool                  m_input_endpoint_set;
  
  /*! \brief Flag indicating that the output endpoint has been manually set. */
  bool                  m_output_endpoint_set;
  
  
  
  /*! \brief Value of the timeout for blocking operations in milliseconds. */
  unsigned int          m_timeout;
  
  /*! \brief The ID of the configuration set for the device. */
  int                   m_configuration;
  
  /*! \brief The ID of the device's previous configuration before opening. */ 
  int                   m_old_configuration;
  
  /*! \brief The ID of the interface set for the device. */
  int                   m_interface;
  
  /*! \brief The address of the endpoint used for write operations. */
  unsigned char         m_input_endpoint;
  
  /*! \brief The address of the endpoint used for read operations. */
  unsigned char         m_output_endpoint;
  
  /*! \brief the ID of the laternate setting set for the device. */
  unsigned int          m_alt_setting;
  
  
  
  /*! \brief Device struct used for interacting with Libusb. */
  libusb_device* const  m_device;
  
  /*! \brief Handle used for communications with the device through Libusb. */
  libusb_device_handle* m_device_handle;
  
  /*! \brief Cached device descritor built during initialization. */
  device_description*   m_device_description;
  
  /*! \brief Cached value of the device's manufacturer string. */
  std::string           m_manufacturer_string;
  
  /*! \brief Flag indicating that \ref m_manufacturer_string has been set. */
  bool                  m_manufacturer_string_set;
  
  /*! \brief Cached value of the device's product string. */
  std::string           m_product_string;
  
  /*! \brief Flag indicating that \ref m_product_string has been set. */
  bool                  m_product_string_set;
  
  /*! \brief Cached value of the device's serial number string. */
  std::string           m_serial_number;
  
  /*! \brief Flag indicating that \ref m_serial_number has been set. */
  bool                  m_serial_number_set;
};

}

#endif /* defined(__LIBUSB_USB_DEVICE_H__) */
