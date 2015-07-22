#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#define TIMEOUT_UNSET_VALUE       -1
#define CONFIGURATION_UNSET_VALUE -1
#define INTERFACE_UNSET_VALUE     -1
#define ENDPOINT_UNSET_VALUE      -1
#define DESCRIPTION_UNSET_VALUE   NULL

class usb_device
{
public:
  typedef int timeout_t;
  typedef int configuration_t;
  typedef int interface_t;
  typedef int endpoint_t;
  typedef unsigned char data_t;
  
  struct device_description;
  struct device_configuration;
  struct device_interface;
  struct device_alt_setting;
  struct device_endpoint;
  
  virtual ~usb_device();
  
  /**
   * Performs critical initialization that should not be performed
   * in the constructor, such as fetch USB device information and
   * construct device descriptors.
   */
  virtual void init() = 0;
  
  /**
   * Gets the number of miliseconds of no response to wait before
   * giving up.
   * 
   * @return The currenlty set timeout.
   */
  virtual timeout_t timeout() const = 0;
  
  /**
   * Gets the configuration for this device to use.
   * 
   * @return The configuration this device is using.
   */
  virtual configuration_t configuration() const = 0;
  
  /**
   * Gets the target interface to use for communication.
   * 
   * @return The number of the interface use for communication.
   */
  virtual interface_t interface() const = 0;
  
  virtual endpoint_t input_endpoint() const = 0;
  virtual endpoint_t output_endpoint() const = 0;
  virtual const device_description* get_device_description() const = 0;
  
  /**
   * Sets the maximum number of miliseconds to wait before giving up.
   * 
   * @param timeout - Milliseconds until timeout.
   */
  virtual void set_timeout(timeout_t timeout) = 0;
  
  /**
   * Sets the configuration for this device to use.
   * 
   * @param configuration - The configuration to use.
   */
  virtual void set_configuration(configuration_t configuration) = 0;
  
  /**
   * Sets the target interface to use for communication.
   * 
   * @param interface - The interface to use for communication.
   */
  virtual void set_interface(interface_t interface) = 0;
  
  virtual void set_input_endpoint(endpoint_t input_endpoint) = 0;
  virtual void set_output_endpoint(endpoint_t output_endpoint) = 0;
  
  virtual void open() = 0;
  virtual void close() = 0;
  virtual unsigned int read(const data_t* data, unsigned int num_bytes) = 0;
  virtual unsigned int read(const data_t* data, unsigned int num_bytes, timeout_t timeout) = 0;
  virtual unsigned int write(data_t* buffer, unsigned int num_bytes) = 0;
  virtual unsigned int write(data_t* buffer, unsigned int num_bytes, timeout_t timeout) = 0;
};



struct usb_device::device_description
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



struct usb_device::device_configuration  
{
  explicit device_configuration(unsigned int num_interfaces);
  device_configuration(const device_configuration& other);
  ~device_configuration();
  
  const unsigned int num_interfaces;
  device_interface** const interfaces;
};



struct usb_device::device_interface
{
  explicit device_interface(unsigned int num_alt_settings);
  device_interface(const device_interface& other);
  ~device_interface();
  
  const unsigned int num_alt_settings;
  device_alt_setting** const alt_settings;
};



struct usb_device::device_alt_setting
{
  explicit device_alt_setting(unsigned int num_endpoints);
  device_alt_setting(const device_alt_setting& other);
  ~device_alt_setting();
  
  usb_device::interface_t interface_num;
  const unsigned int num_endpoints;
  device_endpoint** const endpoints;
};



struct usb_device::device_endpoint
{
  device_endpoint();
  device_endpoint(const device_endpoint& other);
  
  int desc_type;
  usb_device::endpoint_t address;
};

#endif // __USB_COMMUNICATOR_H__
