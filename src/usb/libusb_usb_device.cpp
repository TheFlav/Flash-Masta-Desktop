/*! \file
 *  \brief File containing the implementation of the \ref usb::libusb_usb_device
 *         class.
 *  
 *  File containing the implementation of the \ref usb::libusb_usb_device class.
 *  See corresponding header file to view documentation for the class, its
 *  methods, and its member variables.
 *  
 *  \see usb::libusb_usb_device
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-05
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifdef _MSC_VER
#pragma warning( disable : 4200 )
#endif

#include "libusb_usb_device.h"
#include "usb.h"
#include "tasks/task_controller.h"
#include "tasks/forwarding_task_controller.h"
#include "libusb-1.0/libusb.h"
#include <stdexcept>
#include <string>

#define CLASS_NAME "libusb_usb_device"
#define CONFIG_NAME "USB configuration"
#define INTERFACE_NAME "USB interface"
#define INPUT_ENDPOINT_NAME "USB input endpoint"
#define OUTPUT_ENDPOINT_NAME "USB output endpoint"
#define TIMEOUT_NAME "USB timeout"

namespace usb
{

typedef libusb_usb_device::timeout_t            timeout_t;
typedef libusb_usb_device::configuration_t      configuration_t;
typedef libusb_usb_device::interface_t          interface_t;
typedef libusb_usb_device::endpoint_t           endpoint_t;
typedef libusb_usb_device::data_t               data_t;

typedef libusb_usb_device::device_description   device_description;
typedef libusb_usb_device::device_configuration device_configuration;
typedef libusb_usb_device::device_interface     device_interface;
typedef libusb_usb_device::device_alt_setting   device_alt_setting;
typedef libusb_usb_device::device_endpoint      device_endpoint;


libusb_usb_device::libusb_usb_device(libusb_device* device)
  : m_was_initialized    (false),
    m_is_open            (false),
    m_kernel_was_attached(false),
    m_timeout_set        (true),
    m_configuration_set  (false),
    m_interface_set      (false),
    m_input_endpoint_set (false),
    m_output_endpoint_set(false),
    m_timeout            (0),
    m_configuration      (0),
    m_old_configuration  (0),
    m_interface          (0),
    m_input_endpoint     (0),
    m_output_endpoint    (0),
    m_alt_setting        (0),
    m_device             (device),
    m_device_handle      (nullptr),
    m_device_description (nullptr),
    m_manufacturer_string(),
    m_manufacturer_string_set(false),
    m_product_string     (),
    m_product_string_set (false),
    m_serial_number      (),
    m_serial_number_set  (false)
{
  // Increment the reference counter for the device
  libusb_ref_device(m_device);
}

libusb_usb_device::~libusb_usb_device()
{
  // Clean up results of being initialized
  if (m_was_initialized)
  {
    if (m_device_description != nullptr)
    {
      delete m_device_description;
    }
  }
  
  // Close connection if opened
  if (m_is_open)
  {
    try
    {
      close();
    }
    catch (std::exception& ex)
    {
      (void) ex;
      // Do nothing, fail silently
    }
  }
  
  // Decrement the reference counter for the device
  libusb_unref_device(m_device);
}

void libusb_usb_device::init()
{
  // Check if we were already initialized
  if (m_was_initialized)
  {
    return;
  }
  
  // Build device description
  m_device_description = build_device_description();
  
  // TODO: Error check
  
  m_was_initialized = true;
}



timeout_t libusb_usb_device::timeout() const
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_timeout_set) throw unconfigured_exception(TIMEOUT_NAME);

  return m_timeout;
}

configuration_t libusb_usb_device::configuration() const
{
  // Check for errors
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);

  return m_device_description
    ->configurations[m_configuration]
    ->config_id;
}

interface_t libusb_usb_device::interface() const
{
  // Check for errors
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);

  return get_device_description()
    ->configurations[m_configuration]
    ->interfaces[m_interface]
    ->interface_id;
}

endpoint_t libusb_usb_device::input_endpoint() const
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);
  if (!m_input_endpoint_set) throw unconfigured_exception(INPUT_ENDPOINT_NAME);
  
  return get_device_description()
    ->configurations[m_configuration]
    ->interfaces[m_interface]
    ->alt_settings[m_alt_setting]
    ->endpoints[m_input_endpoint]
    ->address;
}

endpoint_t libusb_usb_device::output_endpoint() const
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);
  if (!m_output_endpoint_set) throw unconfigured_exception(OUTPUT_ENDPOINT_NAME);
  
  return get_device_description()
    ->configurations[m_configuration]
    ->interfaces[m_interface]
    ->alt_settings[m_alt_setting]
    ->endpoints[m_output_endpoint]
    ->address;
}

const device_description* libusb_usb_device::get_device_description() const
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  
  return m_device_description;
}

std::string libusb_usb_device::get_manufacturer_string()
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  
  if (!m_manufacturer_string_set)
  {
    bool was_open = m_is_open;
    char buffer[512] = "";
    
    if (!was_open)
    {
      open();
    }
    
    // Get descriptor so we can find string indexes
    libusb_device_descriptor desc;
    int error = libusb_get_device_descriptor(m_device, &desc);
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
      return nullptr;
    }
    
    // Fetch the English version of the string
    libusb_get_string_descriptor_ascii(m_device_handle, desc.iManufacturer, /* 0x0409, */ (unsigned char*) buffer, 512);
    
    if (!was_open)
    {
      close();
    }
    
    // Copy string over
    m_manufacturer_string = std::string(buffer);
    m_manufacturer_string_set = true;
  }
  
  return m_manufacturer_string;
}

std::string libusb_usb_device::get_product_string()
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  
  if (!m_product_string_set)
  {
    bool was_open = m_is_open;
    char buffer[512] = "";
    
    if (!was_open)
    {
      open();
    }
    
    // Get descriptor so we can find string indexes
    libusb_device_descriptor desc;
    int error = libusb_get_device_descriptor(m_device, &desc);
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
      return nullptr;
    }
    
    // Fetch the English version of the string
    libusb_get_string_descriptor_ascii(m_device_handle, desc.iProduct, /* 0x0409, */ (unsigned char*) buffer, 512);
    
    if (!was_open)
    {
      close();
    }
    
    // Copy string over
    m_product_string = std::string(buffer);
    m_product_string_set = true;
  }
  
  return m_product_string;
}

std::string libusb_usb_device::get_serial_number()
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  
  if (!m_serial_number_set)
  {
    bool was_open = m_is_open;
    char buffer[512] = "";
    
    if (!was_open)
    {
      open();
    }
    
    // Get descriptor so we can find string indexes
    libusb_device_descriptor desc;
    int error = libusb_get_device_descriptor(m_device, &desc);
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
      return nullptr;
    }
    
    // Fetch the English version of the string
    libusb_get_string_descriptor_ascii(m_device_handle, desc.iSerialNumber, /* 0x0409, */ (unsigned char*) buffer, 512);
    
    if (!was_open)
    {
      close();
    }
    
    // Copy string over
    m_serial_number = std::string(buffer);
    m_serial_number_set = true;
  }
  
  return m_serial_number;
}



void libusb_usb_device::set_timeout(timeout_t timeout)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  
  m_timeout = (unsigned int) timeout;
  m_timeout_set = true;
}

void libusb_usb_device::set_configuration(configuration_t configuration)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  
  int error;
  const device_configuration* config = nullptr;
  
  // Search for config in descriptor
  for (unsigned int i = 0; i < m_device_description->num_configurations; ++i)
  {
    if (m_device_description->configurations[i]->config_id == configuration)
    {
      config = m_device_description->configurations[i];
      configuration = i;
    }
  }
  
  // Validate arguments
  if (config == nullptr)
  {
    throw not_found_exception("configuration{id: " + std::to_string(configuration) + "}");
    return;
  }
  
  // Only change configuration if new value differs from old value
  if (!m_configuration_set || m_configuration != (int) configuration)
  {
    m_configuration = (unsigned int) configuration;
    m_configuration_set = true;
    m_interface_set = false;
    
    // Switch device's current configuration
    if (m_is_open)
    {
      error = libusb_set_configuration(m_device_handle, m_device_description->configurations[m_configuration]->config_id);
      if (libusb_error_occured(error))
      {
        throw_libusb_exception(error, timeout());
        return;
      }
    }
  }
}

void libusb_usb_device::set_interface(interface_t interface)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  
  int error;
  const device_interface* inter = nullptr;
  const device_configuration* config = m_device_description->configurations[m_configuration];
  
  // Search for interface in descriptor
  for (unsigned int i = 0; i < config->num_interfaces; ++i)
  {
    if (config->interfaces[i]->interface_id == interface)
    {
      inter = config->interfaces[i];
      interface = i;
    }
  }
  
  // Validate arguments
  if (inter == nullptr)
  {
    throw not_found_exception("interface{id: " + std::to_string(interface) + "}");
  }
  
  // Claim interface if applicable
  if (m_is_open)
  {
    // Release old interface if one was previously claimed
    if (m_interface_set)
    {
      error = libusb_release_interface(m_device_handle, m_interface);
      if (libusb_error_occured(error))
      {
        throw_libusb_exception(error, timeout());
      }
    }
    else
    {
      m_interface_set = true;
    }
    
    m_interface = (unsigned int) interface;
    
    error = libusb_claim_interface(m_device_handle, m_interface);
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
    }
  }
  else
  {
    m_interface_set = true;
    m_interface = (unsigned int) interface;
  }
  
  
  // Automatically set endpoints if unset
  if (!m_input_endpoint_set|| !m_output_endpoint_set)
  {
    const device_alt_setting* alt_setting = m_device_description->
      configurations[m_configuration]->interfaces[m_interface]->
      alt_settings[m_alt_setting];
    
    unsigned int num_endpoints = alt_setting->num_endpoints;
    
    for (unsigned int i = 0; i < num_endpoints && (!m_input_endpoint_set || !m_output_endpoint_set); ++i)
    {
      if (alt_setting->endpoints[i]->transfer_type != ENDPOINT_TYPE_BULK)
      {
        continue; 
      }
      if (alt_setting->endpoints[i]->direction == ENDPOINT_DIRECTION_OUT && !m_output_endpoint_set)
      {
        m_output_endpoint_set = true;
        m_output_endpoint = i;
      }
      else if (alt_setting->endpoints[i]->direction == ENDPOINT_DIRECTION_IN && !m_input_endpoint_set)
      {
        m_input_endpoint_set = true;
        m_input_endpoint = i;
      }
    }
  }
}

void libusb_usb_device::set_input_endpoint(endpoint_t input_endpoint)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);
  
  // Validate input
  if (input_endpoint > 255)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(input_endpoint)
                                + " for argument 1: expected value between 0 and 255.");
  }
  else if ((input_endpoint & 0x70) != 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(input_endpoint)
                                + " for argument 1: invaild endpoint address.");
  }
  else if ((input_endpoint & LIBUSB_ENDPOINT_IN) == 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(input_endpoint)
                                + " for argument 1: endpoint address does not indicate input.");
  }
  
  // Find desired endpoint
  device_endpoint* ep = nullptr;
  device_alt_setting* as = get_device_description()->configurations[m_configuration]->interfaces[m_interface]->alt_settings[m_alt_setting];
  unsigned int i;
  for (i = 0; i < as->num_endpoints; ++i)
  {
    if (as->endpoints[i]->address == input_endpoint)
    {
      ep = as->endpoints[i];
      break;
    }
  }
  
  if (ep == nullptr)
  {
    throw not_found_exception("endpoint{address: " + std::to_string(input_endpoint) + "}");
  }
  
  m_input_endpoint = i;
  m_input_endpoint_set = true;
}

void libusb_usb_device::set_output_endpoint(endpoint_t output_endpoint)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);
  
  // Validate input
  if (output_endpoint > 255)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(output_endpoint)
                                + " for argument 1: expected value between 0 and 255.");
  }
  else if ((output_endpoint & 0x70) != 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(output_endpoint)
                                + " for argument 1: invaild endpoint address.");
  }
  else if ((output_endpoint & LIBUSB_ENDPOINT_IN) != 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(output_endpoint)
                                + " for argument 1: endpoint address does not indicate output.");
  }
  
  // Find desired endpoint
  device_endpoint* ep = nullptr;
  device_alt_setting* as = get_device_description()->configurations[m_configuration]->interfaces[m_interface]->alt_settings[m_alt_setting];
  unsigned int i;
  for (i = 0; i < as->num_endpoints; ++i)
  {
    if (as->endpoints[i]->address == output_endpoint)
    {
      ep = as->endpoints[i];
      break;
    }
  }
  
  if (ep == nullptr)
  {
    throw not_found_exception("endpoint{address: " + std::to_string(output_endpoint) + "}");
  }
  
  m_output_endpoint = i;
  m_output_endpoint_set = true;
}



void libusb_usb_device::open()
{
  if (m_is_open)
  {
    // Already open: do nothing
    return;
  }
  
  int error;
  
  // Attempt to open device
  error = libusb_open(m_device, &m_device_handle);
  if (libusb_error_occured(error))
  {
    throw_libusb_exception(error, timeout());
    return;
  }
  
  // Attempt to detach the OS kernel driver if it is attached
  if (libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER))
  {
    error = libusb_kernel_driver_active(m_device_handle, m_interface);
    if (error == 1)
    {
        error = libusb_detach_kernel_driver(m_device_handle, m_interface);
        if (libusb_error_occured(error))
        {
          throw_libusb_exception(error, timeout());
          return;
        }
        m_kernel_was_attached = true;
    }
    else if (libusb_error_occured(error))
    {
        throw_libusb_exception(error, timeout());
        return;
    }
  }
  else
  {
    m_kernel_was_attached = false;
  }
  
  // Attempt to retrieve the current configuration so we can restore it later
  error = libusb_get_configuration(m_device_handle, &m_old_configuration);
  if (libusb_error_occured(error))
  {
    throw_libusb_exception(error, timeout());
    return;
  }
  
  // Update m_old_configuration to be index of configuration
  for (unsigned int i = 0; i < m_device_description->num_configurations; ++i)
  {
    if ((int) m_device_description->configurations[i]->config_id == m_old_configuration)
    {
      m_old_configuration = i;
      break;
    }
  }
  
  // Attempt to set the configuration
  if (m_configuration_set)
  {
    if (m_configuration != m_old_configuration)
    {
      error = libusb_set_configuration(m_device_handle, m_device_description->configurations[m_configuration]->config_id);
      if (libusb_error_occured(error))
      {
        throw_libusb_exception(error, timeout());
        return;
      }
    }
  }
  else
  {
    m_configuration_set = true;
    m_configuration = m_old_configuration;
  }
  
  // Attempt to claim the desired interface
  if (m_interface_set)
  {
    error = libusb_claim_interface(m_device_handle, interface());
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
      return;
    }
  }
  
  m_is_open = true;
}

void libusb_usb_device::close()
{
  if (!m_is_open)
  {
    // Already closed: do nothing
    return;
  }
  
  int error;
  m_is_open = false;
  
  // Attempt to release the claim on the interface
  if (m_interface_set)
  {
    error = libusb_release_interface(m_device_handle, m_interface);
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
      return;
    }
  }
  
  // Attempt to reset the configuration to what it was previously
  error = libusb_set_configuration(m_device_handle, m_device_description->configurations[m_old_configuration]->config_id);
  if (libusb_error_occured(error))
  {
    throw_libusb_exception(error, timeout());
    return;
  }
  else
  {
    m_configuration_set = false;
  }
  
  // Attempt to reattach the kernel driver
  if (m_kernel_was_attached)
  {
    error = libusb_attach_kernel_driver(m_device_handle, m_interface);
    if (libusb_error_occured(error))
    {
      throw_libusb_exception(error, timeout());
      return;
    }
  }
  
  // Close the device
  libusb_close(m_device_handle);
  
  m_device_handle = nullptr;
}

unsigned int libusb_usb_device::read(data_t *buffer, unsigned int num_bytes)
{
  return read(buffer, num_bytes, timeout());
}

unsigned int libusb_usb_device::read(data_t* buffer, unsigned int num_bytes, timeout_t timeout)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_is_open) throw unopen_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);
  if (!m_input_endpoint_set) throw unconfigured_exception(INPUT_ENDPOINT_NAME);
  
  int bytes_written = 0;
  unsigned char endpoint = get_device_description()
    ->configurations[m_configuration]
    ->interfaces[m_interface]
    ->alt_settings[m_alt_setting]
    ->endpoints[m_input_endpoint]
    ->address;
  
  // Transfer data, catching errors and throwing exceptions if necessary
  int error = libusb_bulk_transfer(m_device_handle, endpoint, buffer, num_bytes, &bytes_written, (unsigned int) timeout);
  if (libusb_error_occured(error))
  {
    throw_libusb_exception(error, timeout);
    return bytes_written;
  }
  
  // Adjust number of bytes read to conform to the return type
  if (bytes_written < 0)
  {
    bytes_written = 0;
  }
  
  return (unsigned int) bytes_written;
}

unsigned int libusb_usb_device::write(const data_t* data, unsigned int num_bytes)
{
  return write(data, num_bytes, timeout());
}

unsigned int libusb_usb_device::write(const data_t* data, unsigned int num_bytes, timeout_t timeout)
{
  if (!m_was_initialized) throw uninitialized_exception(CLASS_NAME);
  if (!m_is_open) throw unopen_exception(CLASS_NAME);
  if (!m_configuration_set) throw unconfigured_exception(CONFIG_NAME);
  if (!m_interface_set) throw unconfigured_exception(INTERFACE_NAME);
  if (!m_output_endpoint_set) throw unconfigured_exception(OUTPUT_ENDPOINT_NAME);
  
  // Copy data array to new array that is writiable
  data_t* data_writable = new data_t[num_bytes];
  for (unsigned int i = 0; i < num_bytes; ++i)
  {
    data_writable[i] = data[i];
  }
  
  int bytes_read = 0;
  unsigned char endpoint = get_device_description()
    ->configurations[m_configuration]
    ->interfaces[m_interface]
    ->alt_settings[m_alt_setting]
    ->endpoints[m_output_endpoint]
    ->address;
  
  // Transfer data, catching errors and throwing exceptions if necessary
  int error = libusb_bulk_transfer(m_device_handle, endpoint, data_writable, num_bytes, &bytes_read, (unsigned int) timeout);
  if (libusb_error_occured(error))
  {
    delete [] data_writable;
    throw_libusb_exception(error, timeout);
    return bytes_read;
  }
  
  delete [] data_writable;
  
  // Adjust number of bytes read to conform to the return type
  if (bytes_read < 0)
  {
    bytes_read = 0;
  }
  
  return (unsigned int) bytes_read;
}



device_description* libusb_usb_device::build_device_description()
{
  libusb_device_descriptor device_descriptor;
  unsigned int device_num_configurations;
  
  // Fetch device descriptor, catching errors and throwing exceptions
  int error = libusb_get_device_descriptor(m_device, &device_descriptor);
  if (libusb_error_occured(error))
  {
    throw_libusb_exception(error, timeout());
    return nullptr;
  }
  
  device_num_configurations = device_descriptor.bNumConfigurations;
  
  // Create description and fill with data
  device_description* description = new device_description(device_num_configurations);
  description->device_class = device_descriptor.bDeviceClass;
  description->vendor_id = device_descriptor.idVendor;
  description->product_id = device_descriptor.idProduct;
  
  // Fetch remainder of device info
  unsigned int i;
  for (i = 0; i < description->num_configurations; ++i)
  {
    description->configurations[i] = build_device_config(i);
    
    // Validate configuration
    if (description->configurations[i] == nullptr)
    {
      delete description; // necessary to prevent memory leaks in the event of an error
      return nullptr;
    }
  }
  
  return description;
}

device_configuration* libusb_usb_device::build_device_config(unsigned int index)
{
  libusb_config_descriptor* libusb_config;
  unsigned int config_num_interfaces;
  
  // Fetch configuration, catching errors and throwing exceptions
  int error = libusb_get_config_descriptor(m_device, index, &libusb_config);
  if (libusb_error_occured(error))
  {
    throw_libusb_exception(error, timeout());
    return nullptr;
  }
  
  config_num_interfaces = libusb_config->bNumInterfaces;
  
  // Create config and fill with information
  device_configuration* configuration = new device_configuration(config_num_interfaces);
  configuration->config_id = libusb_config->bConfigurationValue;
  
  // Fetch remainder of config info
  unsigned int i;
  for (i = 0; i < configuration->num_interfaces; ++i)
  {
    configuration->interfaces[i] = build_device_interface(libusb_config, i);
  }
  
  libusb_free_config_descriptor(libusb_config);
  
  return configuration;
}

device_interface* libusb_usb_device::build_device_interface(const libusb_config_descriptor* config, unsigned int index)
{
  const libusb_interface* libusb_interface_;
  unsigned int interface_num_alt_settings;
  
  // Fetch interface
  libusb_interface_ = &(config->interface[index]);
  
  interface_num_alt_settings = libusb_interface_->num_altsetting;
  
  // Create interface and fill with information
  device_interface* interface = new device_interface(interface_num_alt_settings);
  
  // Fetch remainder of config info
  unsigned int i;
  for (i = 0; i < interface->num_alt_settings; ++i)
  {
    interface->alt_settings[i] = build_device_alt_setting(libusb_interface_, i);
  }
  
  if (interface->num_alt_settings > 0)
  {
    interface->interface_id = interface->alt_settings[0]->interface_id;
  }
  
  return interface;
}

device_alt_setting* libusb_usb_device::build_device_alt_setting(const libusb_interface* interface, unsigned int index)
{
  const libusb_interface_descriptor* libusb_altsetting;
  unsigned int altsetting_num_endpoints;
  
  // Fetch alternate setting
  libusb_altsetting = &(interface->altsetting[index]);
  
  altsetting_num_endpoints = libusb_altsetting->bNumEndpoints;
  
  // Create alt setting and fill with information
  device_alt_setting* alt_setting = new device_alt_setting(altsetting_num_endpoints);
  alt_setting->interface_id = libusb_altsetting->bInterfaceNumber;
  alt_setting->alt_setting_id = libusb_altsetting->bAlternateSetting;
  
  // Fetch remainder of alt setting info
  unsigned int i;
  for (i = 0; i < alt_setting->num_endpoints; ++i)
  {
    alt_setting->endpoints[i] = build_device_endpoint(libusb_altsetting, i);
  }
  
  return alt_setting;
}

device_endpoint* libusb_usb_device::build_device_endpoint(const libusb_interface_descriptor* interface, unsigned int index)
{
  const libusb_endpoint_descriptor* libusb_endpoint;
  
  // Fetch endpoint
  libusb_endpoint = &(interface->endpoint[index]);
  
  // Create endpoint and will with information
  device_endpoint* endpoint = new device_endpoint();
  endpoint->address = libusb_endpoint->bEndpointAddress;
  
  switch (endpoint->address & LIBUSB_ENDPOINT_IN)
  {
  case LIBUSB_ENDPOINT_IN:
    endpoint->direction = ENDPOINT_DIRECTION_IN;
    break;
  case LIBUSB_ENDPOINT_OUT:
    endpoint->direction = ENDPOINT_DIRECTION_OUT;
    break;
  default:
    break;
  }
  
  switch (libusb_endpoint->bmAttributes & 0x03)
  {
  case 0:
    endpoint->transfer_type = ENDPOINT_TYPE_CONTROL;
    break;
  case 1:
    endpoint->transfer_type = ENDPOINT_TYPE_ISOCHRONOUS;
    break;
  case 2:
    endpoint->transfer_type = ENDPOINT_TYPE_BULK;
    break;
  case 3:
    endpoint->transfer_type = ENDPOINT_TYPE_INTERRUPT;
    break;
  default:
    break;
  }
  
  return endpoint;
}

bool libusb_usb_device::libusb_error_occured(int libusb_error)
{
  return (libusb_error != LIBUSB_SUCCESS);
}

void libusb_usb_device::throw_libusb_exception(int libusb_error, timeout_t timeout)
{
  switch (libusb_error)
  {
    case LIBUSB_ERROR_NO_DEVICE:
      throw disconnected_exception();
    case LIBUSB_ERROR_BUSY:
      throw busy_exception();
    case LIBUSB_ERROR_TIMEOUT:
      throw timeout_exception(timeout);
    case LIBUSB_ERROR_INTERRUPTED:
      throw interrupted_exception();
    case LIBUSB_SUCCESS:        // No error occured, but function *must* throw exception
    case LIBUSB_ERROR_NOT_FOUND:
    case LIBUSB_ERROR_IO:
    case LIBUSB_ERROR_INVALID_PARAM:
    case LIBUSB_ERROR_ACCESS:
    case LIBUSB_ERROR_OVERFLOW:
    case LIBUSB_ERROR_PIPE:
    case LIBUSB_ERROR_NO_MEM:
    case LIBUSB_ERROR_NOT_SUPPORTED:
    case LIBUSB_ERROR_OTHER:
    default:
      throw usb::exception("libusb error code " + std::to_string(libusb_error) + " (" + libusb_error_name(libusb_error) + ")");
  }
}

}
