#ifndef __USB_COMMUNICATOR_H__
#define __USB_COMMUNICATOR_H__

class usb_communicator
{
public:
  struct device_description;
  struct device_configuration;
  struct device_interface;
  struct device_alt_setting;
  struct device_endpoint;
  
  virtual ~usb_communicator();
  
  virtual void open() = 0;
  virtual void close() = 0;
  
  /**
   * Gets the number of miliseconds of no response to wait before
   * giving up.
   * 
   * @return The currenlty set timeout.
   */
  virtual int timeout() const = 0;
  
  virtual int input_endpoint() const = 0;
  virtual int output_endpoint() const = 0;
  
  /**
   * Sets the maximum number of miliseconds to wait before giving up.
   * 
   * @param timeout - Milliseconds until timeout
   */
  virtual void set_timeout(int timeout) = 0;
  
  virtual void set_input_endpoint(int input_endpoint) = 0;
  virtual void set_output_endpoint(int output_endpoint) = 0;
  virtual const device_description get_device_description() = 0;
  
  /**
   * @brief bulk_transfer
   * @param data - array of data to send
   * @param num_bytes - number of bytes to transfer
   * @return number of bytes transferred
   */
  virtual unsigned int bulk_transfer(unsigned char* data, unsigned int num_bytes) = 0;
};



struct usb_communicator::device_description
{
  explicit device_description(unsigned int num_configurations);
  device_description(const device_description& other);
  ~device_description();
  
  int device_class;
  int vendor_id;
  int product_id;
  const unsigned int num_configurations;
  device_configuration** const configurations;  
};



struct usb_communicator::device_configuration  
{
  explicit device_configuration(unsigned int num_interfaces);
  device_configuration(const device_configuration& other);
  ~device_configuration();
  
  const unsigned int num_interfaces;
  device_interface** const interfaces;
};



struct usb_communicator::device_interface
{
  explicit device_interface(unsigned int num_alt_settings);
  device_interface(const device_interface& other);
  ~device_interface();
  
  const unsigned int num_alt_settings;
  device_alt_setting** const alt_settings;
};



struct usb_communicator::device_alt_setting
{
  explicit device_alt_setting(unsigned int num_endpoints);
  device_alt_setting(const device_alt_setting& other);
  ~device_alt_setting();
  
  int interface_num;
  const unsigned int num_endpoints;
  device_endpoint** const endpoints;
};



struct usb_communicator::device_endpoint
{
  device_endpoint();
  device_endpoint(const device_endpoint& other);
  
  int desc_type;
  int address;
};

#endif // __USB_COMMUNICATOR_H__
