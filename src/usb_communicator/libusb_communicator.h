#ifndef __LIBUSB_COMMUNICATOR_H__
#define __LIBUSB_COMMUNICATOR_H__

#include "usb_communicator.h"

struct libusb_device;
struct libusb_device_handle;
struct libusb_config_descriptor;
struct libusb_interface;
struct libusb_interface_descriptor;

class libusb_communicator : public usb_communicator
{
public:
  libusb_communicator(libusb_device* device, libusb_device_handle* handle);
  
  void open();
  void close();
  const device_description get_device_description();
  int bulk_write(unsigned char* data, unsigned int bytes);
  int bulk_read(unsigned char* buffer, unsigned int max_bytes);
  
private:
  const device_configuration get_device_config(unsigned int index);
  const device_interface     get_device_interface(const libusb_config_descriptor* config, unsigned int index);
  const device_alt_setting   get_device_alt_setting(const libusb_interface* interface, unsigned int index);
  const device_endpoint      get_device_endpoint(const libusb_interface_descriptor* interface, unsigned int index);
  
  bool m_is_open;
  libusb_device* m_device;
  libusb_device_handle* m_handle;
};

#endif // __LIBUSB_COMMUNICATOR_H__
