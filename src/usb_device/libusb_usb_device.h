#ifndef __LIBUSB_USB_DEVICE_H__
#define __LIBUSB_USB_DEVICE_H__

#include "usb_device.h"

struct libusb_device;
struct libusb_device_handle;
struct libusb_config_descriptor;
struct libusb_interface;
struct libusb_interface_descriptor;

class libusb_usb_device : public usb_device
{
public:
  libusb_usb_device(libusb_device* device);
  ~libusb_usb_device();
  void                  init();
  
  timeout_t             timeout() const;
  configuration_t       configuration() const;
  interface_t           interface() const;
  endpoint_t            input_endpoint() const;
  endpoint_t            output_endpoint() const;
  const device_description* get_device_description() const;

  void                  set_timeout(timeout_t timeout);
  void                  set_configuration(configuration_t configuration);
  void                  set_interface(interface_t interface);
  void                  set_input_endpoint(endpoint_t input_endpoint);
  void                  set_output_endpoint(endpoint_t output_endpoint);

  void                  open();
  void                  close();
  unsigned int          read(const data_t* data, unsigned int num_bytes) = 0;
  unsigned int          read(const data_t* data, unsigned int num_bytes, timeout_t timeout);
  unsigned int          write(data_t* buffer, unsigned int num_bytes) = 0;
  unsigned int          write(data_t* buffer, unsigned int num_bytes, timeout_t timeout);
  
private:
  device_description*   build_device_description();
  device_configuration* build_device_config(unsigned int index);
  device_interface*     build_device_interface(const libusb_config_descriptor* config, unsigned int index);
  device_alt_setting*   build_device_alt_setting(const libusb_interface* interface, unsigned int index);
  device_endpoint*      build_device_endpoint(const libusb_interface_descriptor* interface, unsigned int index);
  
  bool                  m_was_initialized;
  bool                  m_is_open;
  bool                  m_kernel_was_attached;
  bool                  m_timeout_set;
  bool                  m_configuration_set;
  bool                  m_interface_set;
  bool                  m_input_endpoint_set;
  bool                  m_output_endpoint_set;

  unsigned int          m_timeout;
  int                   m_configuration;
  int                   m_old_configuration;
  int                   m_interface;
  unsigned char         m_input_endpoint;
  unsigned char         m_output_endpoint;

  libusb_device*        m_device;
  libusb_device_handle* m_device_handle;
  device_description*   m_device_description;
};

#endif // __LIBUSB_COMMUNICATOR_H__
