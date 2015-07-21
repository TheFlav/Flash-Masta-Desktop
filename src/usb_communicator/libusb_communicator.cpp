#include "libusb_communicator.h"
#include "libusb-1.0/libusb.h"

typedef usb_communicator::device_description device_description;
typedef usb_communicator::device_configuration device_configuration;
typedef usb_communicator::device_interface device_interface;
typedef usb_communicator::device_alt_setting device_alt_setting;
typedef usb_communicator::device_endpoint device_endpoint;


libusb_communicator::libusb_communicator(libusb_device* device, libusb_device_handle* handle)
  : m_is_open(false), m_device(device), m_handle(handle)
{
  // Nothing else to do
}

void libusb_communicator::open()
{
  if (m_is_open)
  {
    // TODO: Already open
  }
  
  libusb_open(m_device, &m_handle);
  
  m_is_open = true;
}

void libusb_communicator::close()
{
  if (!m_is_open)
  {
    // TODO: Already open
  }
  
  libusb_close(m_handle);
  
  m_is_open = false;
}

const device_description libusb_communicator::get_device_description()
{
  libusb_device_descriptor device_descriptor;
  
  unsigned int device_num_configurations;
  
  int result;
  
  // Fetch device descriptor
  result = libusb_get_device_descriptor(m_device, &device_descriptor);
  if (result < 0)
  {
    // TODO: error occured
  }
  
  device_num_configurations = device_descriptor.bNumConfigurations;
  
  // Create description and fill with data
  device_description description(device_num_configurations);
  description.device_class = device_descriptor.bDeviceClass;
  description.vendor_id = device_descriptor.idVendor;
  description.product_id = device_descriptor.idProduct;
  
  // Fetch remainder of device info
  unsigned int i;
  for (i = 0; i < description.num_configurations; ++i)
  {
    description.configurations[i] = new device_configuration(get_device_config(i));
  }
  
  return description;
}

int libusb_communicator::bulk_write(unsigned char* data, unsigned int bytes)
{
  // TODO: This is just placeholder stuff
  data = data + 0;
  bytes = bytes + 0;
  return 0;
}

int libusb_communicator::bulk_read(unsigned char* buffer, unsigned int max_bytes)
{
  // TODO: This is just placeholder stuff
  buffer = buffer + 0;
  max_bytes = max_bytes + 0;
  return 0;
}



const device_configuration libusb_communicator::get_device_config(unsigned int index)
{
  libusb_config_descriptor* libusb_config;
  unsigned int config_num_interfaces;
  
  // Fetch configuration
  if (libusb_get_config_descriptor(m_device, index, &libusb_config) < 0)
  {
    // TODO: Error occured
  }
  
  config_num_interfaces = libusb_config->bNumInterfaces;
  
  // Create config and fill with information
  device_configuration configuration(config_num_interfaces);
  
  // Fetch remainder of config info
  unsigned int i;
  for (i = 0; i < configuration.num_interfaces; ++i)
  {
    configuration.interfaces[i] = new device_interface(get_device_interface(libusb_config, i));
  }
  
  libusb_free_config_descriptor(libusb_config);
  
  return configuration;
}

const device_interface libusb_communicator::get_device_interface(const libusb_config_descriptor* config, unsigned int index)
{
  const libusb_interface* libusb_interface_;
  unsigned int interface_num_alt_settings;
  
  // Fetch interface
  libusb_interface_ = &(config->interface[index]);
  
  interface_num_alt_settings = libusb_interface_->num_altsetting;
  
  // Create interface and fill with information
  device_interface interface(interface_num_alt_settings);
  
  // Fetch remainder of config info
  unsigned int i;
  for (i = 0; i < interface.num_alt_settings; ++i)
  {
    interface.alt_settings[i] = new device_alt_setting(get_device_alt_setting(libusb_interface_, i));
  }
  
  return interface;
}

const device_alt_setting libusb_communicator::get_device_alt_setting(const libusb_interface* interface, unsigned int index)
{
  const libusb_interface_descriptor* libusb_altsetting;
  unsigned int altsetting_num_endpoints;
  
  // Fetch alternate setting
  libusb_altsetting = &(interface->altsetting[index]);
  
  altsetting_num_endpoints = libusb_altsetting->bNumEndpoints;
  
  // Create alt setting and fill with information
  device_alt_setting alt_setting(altsetting_num_endpoints);
  alt_setting.interface_num = libusb_altsetting->bInterfaceNumber;
  
  // Fetch remainder of alt setting info
  unsigned int i;
  for (i = 0; i < alt_setting.num_endpoints; ++i)
  {
    alt_setting.endpoints[i] = new device_endpoint(get_device_endpoint(libusb_altsetting, i));
  }
  
  return alt_setting;
}

const device_endpoint libusb_communicator::get_device_endpoint(const libusb_interface_descriptor* interface, unsigned int index)
{
  const libusb_endpoint_descriptor* libusb_endpoint;
  
  // Fetch endpoint
  libusb_endpoint = &(interface->endpoint[index]);
  
  // Create endpoint and will with information
  device_endpoint endpoint;
  endpoint.address = libusb_endpoint->bEndpointAddress;
  endpoint.desc_type = libusb_endpoint->bDescriptorType;
  
  return endpoint;
}

