#include "libusb_usb_device.h"
#include "libusb-1.0/libusb.h"
#include <stdexcept>
#include <string>

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
    m_timeout_set        (false),
    m_configuration_set  (false),
    m_interface_set      (false),
    m_input_endpoint_set (0),
    m_output_endpoint_set(0),
    m_timeout            (0),
    m_configuration      (0),
    m_old_configuration  (0),
    m_interface          (0),
    m_input_endpoint     (0),
    m_output_endpoint    (0),
    m_device             (device),
    m_device_handle      (NULL),
    m_device_description (NULL)
{
  // Increment the reference counter for the device
  libusb_ref_device(m_device);
}

libusb_usb_device::~libusb_usb_device()
{
  if (m_device_description != NULL)
  {
    delete m_device_description;
  }
  
  // Decrement the reference counter for the device
  libusb_unref_device(m_device);
}

void libusb_usb_device::init()
{
  // Build device description
  m_device_description = build_device_description();
  
  // TODO: Error check
  
  m_was_initialized = true;
}



inline timeout_t libusb_usb_device::timeout() const
{
  return (timeout_t) (m_timeout_set ? m_timeout : TIMEOUT_UNSET_VALUE);
}

inline configuration_t libusb_usb_device::configuration() const
{
  return (configuration_t) (m_configuration_set ? m_configuration : CONFIGURATION_UNSET_VALUE);
}

inline interface_t libusb_usb_device::interface() const
{
  return (interface_t) (m_interface_set ? m_interface : INTERFACE_UNSET_VALUE);
}

inline endpoint_t libusb_usb_device::input_endpoint() const
{
  return (endpoint_t) (m_input_endpoint_set ? m_input_endpoint : ENDPOINT_UNSET_VALUE);
}

inline endpoint_t libusb_usb_device::output_endpoint() const
{
  return (endpoint_t) (m_output_endpoint_set ? m_output_endpoint : ENDPOINT_UNSET_VALUE);
}

inline const device_description* libusb_usb_device::get_device_description() const
{
  return m_device_description;
}



void libusb_usb_device::set_timeout(timeout_t timeout)
{
  // Validate input
  if (timeout < 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(timeout)
                                + " for argument 1: expected value greater than 0.");
  }
  
  m_timeout = (unsigned int) timeout;
  m_timeout_set = true;
}

void libusb_usb_device::set_configuration(configuration_t configuration)
{
  m_configuration = (unsigned int) configuration;
  m_configuration_set = true;
}

void libusb_usb_device::set_interface(interface_t interface)
{
  m_interface = (unsigned int) interface;
  m_interface_set = true;
}

void libusb_usb_device::set_input_endpoint(endpoint_t input_endpoint)
{
  // Validate input
  if (input_endpoint < 0 || input_endpoint > 255)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(input_endpoint)
                                + " for argument 1: expected value between 0 and 255.");
  }
  
  m_input_endpoint = (unsigned char) input_endpoint;
  m_input_endpoint_set = true;
}

void libusb_usb_device::set_output_endpoint(endpoint_t output_endpoint)
{
  // Validate input
  if (output_endpoint < 0 || output_endpoint > 255)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(output_endpoint)
                                + " for argument 1: expected value between 0 and 255");
  }
  
  m_output_endpoint = (unsigned char) output_endpoint;
  m_output_endpoint_set =  true;
}



void libusb_usb_device::open()
{
  if (m_is_open)
  {
    // TODO: Already open
    return;
  }
  
  // Attempt to open device
  if (libusb_open(m_device, &m_device_handle) != 0)
  {
    // TODO: Throw exception
  }
  
  // Attempt to detach the OS kernel driver if it is attached
  if (libusb_kernel_driver_active(m_device_handle, m_interface))
  {
    if (libusb_detach_kernel_driver(m_device_handle, m_interface) != 0)
    {
      // TODO: Throw exception
    }
    m_kernel_was_attached = true;
  }
  else
  {
    m_kernel_was_attached = false;
  }
  
  // Attempt to retrieve the current configuration so we can restore it later
  if (libusb_get_configuration(m_device_handle, &m_old_configuration) != 0)
  {
    // TODO: Throw exception
  }  
  
  // Attempt to set the configuration
  if (libusb_set_configuration(m_device_handle, m_configuration) != 0)
  {
    // TODO: Throw exception
  }
  
  // Attempt to claim the desired interface
  if (libusb_claim_interface(m_device_handle, m_interface) != 0)
  {
    // TODO: Throw exception
  }
  
  m_is_open = true;
}

void libusb_usb_device::close()
{
  if (!m_is_open)
  {
    // TODO: Already open
    return;
  }
  
  m_is_open = false;
  
  // Attempt to release the claim on the interface
  if (libusb_release_interface(m_device_handle, m_interface) != 0)
  {
    // TODO: Throw exception
  }
  
  // Attempt to reset the configuration to what it was previously
  if (libusb_set_configuration(m_device_handle, m_old_configuration) != 0)
  {
    // TODO: Throw exception
  }
  
  // Attempt to reattach the kernel driver
  if (m_kernel_was_attached)
  {
    if (libusb_attach_kernel_driver(m_device_handle, m_interface) != 0)
    {
      // TODO: Throw exception
    }
  }
  
  // Close the device
  libusb_close(m_device_handle);
  
  m_device_handle = NULL;
}

unsigned int libusb_usb_device::read(const data_t*data, unsigned int num_bytes)
{
  return read(data, num_bytes, m_timeout);
}

unsigned int libusb_usb_device::read(const data_t* data, unsigned int num_bytes, timeout_t timeout)
{
  if (!m_was_initialized || !m_is_open)
  {
    // TODO: Throw exception
  }
  
  if (timeout < 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(timeout)
                                + " for argument 3: expected value greater than 0.");
  }
  
  // Copy data array to new array that is writiable
  data_t* data_writable = new data_t[num_bytes];
  for (unsigned int i = 0; i < num_bytes; ++i)
  {
    data_writable[i] = data[i];
  }
  
  int bytes_read = 0;
  
  if (libusb_bulk_transfer(m_device_handle, m_output_endpoint, data_writable, num_bytes, &bytes_read, (unsigned int) timeout) != 0)
  {
    // TODO: Throw exception
    delete [] data_writable;
  }
  
  delete [] data_writable;
  
  // Adjust number of bytes read to conform to the return type
  if (bytes_read < 0)
  {
    bytes_read = 0;
  }
  
  return (unsigned int) bytes_read;
}

unsigned int libusb_usb_device::write(data_t *buffer, unsigned int num_bytes)
{
  return write(buffer, num_bytes, m_timeout);
}

unsigned int libusb_usb_device::write(data_t *buffer, unsigned int num_bytes, timeout_t timeout)
{
  if (!m_was_initialized || !m_is_open)
  {
    // TODO: Throw exception
  }
  
  if (timeout < 0)
  {
    throw std::invalid_argument("Unexpected value " + std::to_string(timeout)
                                + " for argument 3: expected value greater than 0.");
  }
  
  int bytes_written = 0;
  
  if (libusb_bulk_transfer(m_device_handle, m_input_endpoint, buffer, num_bytes, &bytes_written, (unsigned int) timeout) != 0)
  {
    // TODO: Throw exception
  }
  
  // Adjust number of bytes read to conform to the return type
  if (bytes_written < 0)
  {
    bytes_written = 0;
  }
  
  return (unsigned int) bytes_written;
}



device_description* libusb_usb_device::build_device_description()
{
  libusb_device_descriptor device_descriptor;
  unsigned int device_num_configurations;
  
  // Fetch device descriptor
  if (libusb_get_device_descriptor(m_device, &device_descriptor) < 0)
  {
    // TODO: error occured
    return NULL;
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
    if (description->configurations[i] == NULL)
    {
      delete description; // necessary to prevent memory leaks in the event of an error
      return NULL;
    }
  }
  
  return description;
}

device_configuration* libusb_usb_device::build_device_config(unsigned int index)
{
  libusb_config_descriptor* libusb_config;
  unsigned int config_num_interfaces;
  
  // Fetch configuration
  if (libusb_get_config_descriptor(m_device, index, &libusb_config) < 0)
  {
    // TODO: Error occured
    return NULL;
  }
  
  config_num_interfaces = libusb_config->bNumInterfaces;
  
  // Create config and fill with information
  device_configuration* configuration = new device_configuration(config_num_interfaces);
  
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
  alt_setting->interface_num = libusb_altsetting->bInterfaceNumber;
  
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
  endpoint->desc_type = libusb_endpoint->bDescriptorType;
  
  return endpoint;
}

