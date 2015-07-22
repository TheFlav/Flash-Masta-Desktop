#include "usb_device.h"
#include <stddef.h>

typedef usb_device::device_description device_description;
typedef usb_device::device_configuration device_configuration;
typedef usb_device::device_interface device_interface;
typedef usb_device::device_alt_setting device_alt_setting;
typedef usb_device::device_endpoint device_endpoint;



usb_device::~usb_device()
{
  // Nothing else to do
}



device_description::device_description(unsigned int num_configurations)
  : num_configurations(num_configurations),
    configurations(new device_configuration*[num_configurations])
{
  unsigned int i;
  for (i = 0; i < this->num_configurations; ++i)
  {
    configurations[i] = NULL;
  }
}

device_description::device_description(const device_description& other)
  : device_class(other.device_class), vendor_id(other.vendor_id),
    product_id(other.product_id), num_configurations(other.num_configurations),
    configurations(new device_configuration*[other.num_configurations])
{
  unsigned int i;
  for (i = 0; i < num_configurations; ++i)
  {
    configurations[i] = new device_configuration(*other.configurations[i]);
  }
}

device_description::~device_description()
{
  unsigned int i;
  for (i = 0; i < num_configurations; ++i)
  {
    if (configurations[i] != NULL)
    {
      delete configurations[i];
    }
  }
  delete [] configurations;
}



device_configuration::device_configuration(unsigned int num_interfaces)
  : num_interfaces(num_interfaces),
    interfaces(new device_interface*[num_interfaces])
{
  unsigned int i;
  for (i = 0; i < this->num_interfaces; ++i)
  {
    interfaces[i] = NULL;
  }
}

device_configuration::device_configuration(const device_configuration& other)
  : num_interfaces(other.num_interfaces),
    interfaces(new device_interface*[other.num_interfaces])
{
  unsigned int i;
  for (i = 0; i < num_interfaces; ++i)
  {
    interfaces[i] = new device_interface(*other.interfaces[i]);
  }
}

device_configuration::~device_configuration()
{
  unsigned int i;
  for (i = 0; i < num_interfaces; ++i)
  {
    if (interfaces[i] != NULL)
    {
      delete interfaces[i];
    }
  }
  delete [] interfaces;
}



device_interface::device_interface(unsigned int num_alt_settings)
  : num_alt_settings(num_alt_settings),
    alt_settings(new device_alt_setting*[num_alt_settings])
{
  unsigned int i;
  for (i = 0; i < this->num_alt_settings; ++i)
  {
    alt_settings[i] = NULL;
  }
}

device_interface::device_interface(const device_interface& other)
  : num_alt_settings(other.num_alt_settings),
    alt_settings(new device_alt_setting*[other.num_alt_settings])
{
  unsigned int i;
  for (i = 0; i < num_alt_settings; ++i)
  {
    alt_settings[i] = new device_alt_setting(*other.alt_settings[i]);
  }
}

device_interface::~device_interface()
{
  unsigned int i;
  for (i = 0; i < num_alt_settings; ++i)
  {
    if (alt_settings[i] != NULL)
    {
      delete alt_settings[i];
    }
  }
  delete [] alt_settings;
}



device_alt_setting::device_alt_setting(unsigned int num_endpoints)
  : num_endpoints(num_endpoints),
    endpoints(new device_endpoint*[num_endpoints])
{
  unsigned int i;
  for (i = 0; i < this->num_endpoints; ++i)
  {
    endpoints[i] = NULL;
  }
}

device_alt_setting::device_alt_setting(const device_alt_setting& other)
  : interface_num(other.interface_num),
    num_endpoints(other.num_endpoints),
    endpoints(new device_endpoint*[other.num_endpoints])
{
  unsigned int i;
  for (i = 0; i < num_endpoints; ++i)
  {
    endpoints[i] = new device_endpoint(*other.endpoints[i]);
  }
}

device_alt_setting::~device_alt_setting()
{
  unsigned int i;
  for (i = 0; i < num_endpoints; ++i)
  {
    if (endpoints[i] != NULL)
    {
      delete endpoints[i];
    }
  }
  delete [] endpoints;
}



device_endpoint::device_endpoint()
{
  // Nothing else to do here
}

device_endpoint::device_endpoint(const device_endpoint& other)
  : desc_type(other.desc_type), address(other.address)
{
  // Nothing else to do here
}
