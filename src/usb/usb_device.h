/*! \file
 *  \brief File containing the declaration of the \ref usb::usb_device abstract
 *         class.
 *  
 *  File containing the header information and declaration of the
 *  \ref usb::usb_device abstract class. This file includes the minimal number
 *  of files necessary to use any instance of the \ref usb::usb_device class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-03
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#include "usbfwd.h"
#include <string>

#define TIMEOUT_UNSET_VALUE       ((timeout_t) 0xFFFFFFFF)
#define CONFIGURATION_UNSET_VALUE ((configuration_t) 0xFFFFFFFF)
#define INTERFACE_UNSET_VALUE     ((interface_t) 0xFFFFFFFF)
#define ENDPOINT_UNSET_VALUE      ((endpoint_t) 0xFFFFFFFF)
#define DESCRIPTION_UNSET_VALUE   nullptr

class task_controller;

namespace usb
{

/*! \class usb_device
 *  \brief Provides an object-oriented approach to interacting and communicating
 *         with periphrials over USB.
 *  
 *  Abstract class representing a USB device that provides an object-oriented
 *  approach to interacting and communicating with peripherials over USB.
 *  Contains convenient methods for message passing and interacting with a
 *  generic connected USB device.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class usb_device
{
public:
  
  /*! \brief Type used for defining operation timeouts in milliseconds. */
  typedef unsigned int  timeout_t;
  
  /*! \brief Type used for indexing and identifying device configurations. */
  typedef unsigned int  configuration_t;
  
  /*! \brief Type used for indexing and identifying device interfaces. */
  typedef unsigned int  interface_t;
  
  /*! \brief Type used for indexing and identifying interface alt settings. */
  typedef unsigned int  alt_setting_t;
  
  /*! \brief Type used for indexing and identifying interface endpoints. */
  typedef unsigned int  endpoint_t;
  
  /*! \brief Type used for data transfer. */
  typedef unsigned char data_t;
  
  
  
  struct device_description;
  struct device_configuration;
  struct device_interface;
  struct device_alt_setting;
  struct device_endpoint;
  
  
  
  /*! \enum endpoint_transfer_type
   *  \brief Enumeration representing the transfer type of a device's endpoint.
   *  
   *  Enumeration representing the transfer type of a device's endpoint.
   */
  enum endpoint_transfer_type
  {
    /*! \brief Control endpoint. */
    ENDPOINT_TYPE_CONTROL,
    
    /*! \brief Isochronous endpoint. */
    ENDPOINT_TYPE_ISOCHRONOUS,
    
    /*! \brief Bulk transfer endpoint. */
    ENDPOINT_TYPE_BULK,
    
    /*! \brief Interrupt signal endpoint. */
    ENDPOINT_TYPE_INTERRUPT
  };
  
  /*! \enum endpoint_direction
   *  \brief Enumeration representing the direction of data transfer for a
   *         device endpoint.
   *  
   *  Enumeration indicating the direction of data transfer for a device
   *  endpoint.
   */
  enum endpoint_direction
  {
    /*! \brief Data flows through endpoint from system to USB device. */
    ENDPOINT_DIRECTION_IN,
    
    /*! \brief Data flows through endpoint from USB device to system. */
    ENDPOINT_DIRECTION_OUT
  };
  
  
  
  /*! \brief Class destructor.
   *  
   *  Class destructor. Exists only to satisfy compiler.
   */
  virtual ~usb_device();
  
  /*!
   *  \brief Initializes object, preparing it for use.
   *  
   *  Initializes object, preparing it for first use. Performs critical
   *  initialization that should not be performed in the constructor, such as
   *  fetch USB device information and construct device descriptors.
   *  
   *  This is a blocking operation that can take several seconds to complete.
   */
  virtual void init() = 0;
  
  
  
  /*!
   *  \brief Gets the current timeout setting in milliseconds.
   *  
   *  Gets the number of miliseconds of no response to wait before giving up.
   * 
   *  \return The current value of the timeout setting measured in milliseconds.
   */
  virtual timeout_t timeout() const = 0;
  
  /*!
   *  \brief Gets the ID of the device's current active configuration.
   *  
   *  Gets the ID of the device's current active configuration. This can be
   *  changed using \ref set_configuration(configuration_t configuration).
   * 
   *  \return The ID of the configuration this device is using.
   */
  virtual configuration_t configuration() const = 0;
  
  /*!
   *  \brief Gets the interface currently used for communication on the device.
   *  
   *  Gets the interface currently used for communiaction on the device. This
   *  can be changed using \ref set_interface(interface_t interface).
   *  
   *  \return The ID of the currently active interface.
   *  
   *  \see set_interface(interface_t interface)
   */
  virtual interface_t interface() const = 0;
  
  /*!
   *  \brief Gets the ID of the endpoint currently used by this object for
   *         input.
   *  
   *  Gets the ID of the endpoint currently used by this object for transferring
   *  data from the USB device to the system. This can be changed using
   *  \ref set_input_endpoint(endpoint_t input_endpoint).
   *  
   *  \return The ID of the endpoint used for input.
   *  
   *  \see set_input_endpoint(endpoint_t input_endpoint)
   */
  virtual endpoint_t input_endpoint() const = 0;
  
  /*!
   *  \brief Gets the ID of the endpoint currently used byt this object for
   *         output.
   *  
   *  Gets the ID of the endpoint currently used by this object for transferring
   *  data from the system to the USB device. This can be chagned using
   *  \ref set_output_endpoint(endpoint_t output_endpoint).
   *  
   *  \return The ID of the endpoint used for output.
   *  
   *  \see set_output_endpoint(endpoint_t output_endpoint)
   */
  virtual endpoint_t output_endpoint() const = 0;
  
  /*!
   *  \brief Gets the device's descriptor, which can be used to gather
   *         information on the nature of the device and its available
   *         interfaces and endpoints.
   *  
   *  Gets the device's descriptor, which can be used to gather information on
   *  the nature of the device and its available interfaces and endpoints. This
   *  descriptor cannot be modified, as the data it contains are defined by the
   *  device itself.
   *  
   *  The descriptor returned by this method is constructed during
   *  initialization and is permanently cached until this oject's destructor is
   *  called.
   *  
   *  \return A pointer to the device's descriptor struct. This object cannot be
   *          modified in any way.
   */
  virtual const device_description* get_device_description() const = 0;
  
  /*!
   *  \brief Gets the device's manufacturer string.
   *  
   *  Gets the device's manufacturer string.
   *  
   *  This is a blocking operation that can take several seconds to complete.
   *  
   *  \ref String representation of the device's manufacturer's name.
   */
  virtual std::string get_manufacturer_string() = 0;
  
  /*!
   *  \brief Gets the device's product string.
   *  
   *  Gets the device's product string.
   *  
   *  This is a blocking operation that can take several seconds to complete.
   *  
   *  \ref String representation of the device's product name.
   */
  virtual std::string get_product_string() = 0;
  
  /*!
   *  \brief Gets the device's serial number in string form.
   *  
   *  Gets the device's serial number in string form.
   *  
   *  This is a blocking operation that can take several seconds to complete.
   *  
   *  \ref String representation of the device's serial number.
   */
  virtual std::string get_serial_number() = 0;
  
  
  
  /*!
   *  \brief Sets the maximum number of miliseconds to wait before giving up.
   *  
   *  Sets the maximum number of miliseconds to wait before giving up.
   *  
   *  \param [in] timeout Milliseconds until timeout.
   */
  virtual void set_timeout(timeout_t timeout) = 0;
  
  /*!
   *  \brief Sets the configuration for this device to use.
   *  
   *  Sets the configuration for this device to use.
   *  
   *  \param [in] configuration The ID of the configuration to use.
   */
  virtual void set_configuration(configuration_t configuration) = 0;
  
  /**
   *  \brief Sets the target interface to use for communication.
   *  
   *  Sets the target interface to use for communication.
   *  
   *  \param [in] interface The ID of the interface to use for communication.
   */
  virtual void set_interface(interface_t interface) = 0;
  
  /*!
   *  \brief Sets the endpoint to use for input.
   *  
   *  Sets the endpoint to use for input on the device. In this context, the
   *  input endpoint is used to transfer data from the system to the USB device.
   *  
   *  \param [in] input_endpoint The ID of the endpoint to use for input.
   */
  virtual void set_input_endpoint(endpoint_t input_endpoint) = 0;
  
  /*!
   *  \brief Sets the endpoint to use for output.
   *  
   *  Sets the endpoint to use for output on the device. In this context, the
   *  output endpoint is used to transfer data from the USB device to the
   *  system.
   *  
   *  \param [in] output_endpoint The ID of the endpoint to use for output.
   */
  virtual void set_output_endpoint(endpoint_t output_endpoint) = 0;
  
  
  
  /*!
   *  \brief Enables the class for communication with the USB device.
   *  
   *  Enables the class for communication with the USB device. Must be called
   *  before attempting to communicate with the USB device. After use, the
   *  connection must be closed with a call to \ref close().
   *  
   *  \see close()
   */
  virtual void open() = 0;
  
  /*!
   *  \brief Closes the open connection between this object and the USB device.
   *  
   *  Closes the open connection between this object and the USB device. Must
   *  be called after finishing communications with the USB device in order to
   *  perform necessary clean up and deallocate resources.
   *  
   *  \see open()
   */
  virtual void close() = 0;
  
  /*!
   *  \brief Reads a sequence of bytes from the device.
   *  
   *  Reads a sequence of bytes from the device using the device's designated
   *  output endpoint. Uses the currently set timeout value to limit how long
   *  the operation can execute with no results.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \param [out] data The array to which to dump the results of the read.
   *  \param [in] num_bytes The maximum number of bytes to read from the device.
   *  
   *  \return The number of bytes read from the device.
   */
  virtual unsigned int read(data_t* data, unsigned int num_bytes) = 0;
  
  /*!
   *  \brief Reads a sequence of bytes from the device.
   *  
   *  Reads a sequence of bytes from the device using the device's designated
   *  output endpoint. Uses the provided timeout value to limit how long the
   *  operation can execute with no results.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \param [out] data The array to which to dump the results of the read.
   *  \param [in] num_bytes The maximum number of bytes to read from the device.
   *  \param [in] timeout The number of milliseconds to wait for a response
   *         before failing.
   *  
   *  \return The number of bytes read from the device.
   */
  virtual unsigned int read(data_t* data, unsigned int num_bytes, timeout_t timeout) = 0;
  
  /*!
   *  \brief Writes a sequence of bytes to the device.
   *  
   *  Writes a sequence of bytes to the device using the device's designated
   *  input endpoint. Uses the currently set timeout value to limit how long
   *  the operation can execute with no results.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \param [in] data The array containing the data to be sent to the device.
   *  \param [in] num_bytes The number of bytes to write to the device.
   *  
   *  \return The number of bytes written to the device.
   */
  virtual unsigned int write(const data_t* buffer, unsigned int num_bytes) = 0;
  
  /*!
   *  \brief Writes a sequence of bytes to the device.
   *  
   *  Writes a sequence of bytes to the device using the device's designated
   *  input endpoint. Uses the provided timeout value to limit how long the
   *  operation can execute with no results.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \param [in] data The array containing the data to be sent to the device.
   *  \param [in] num_bytes The number of bytes to write to the device.
   *  \param [in] timeout The number of milliseconds to wait for confirmation
   *         before failing.
   *  
   *  \return The number of bytes written to the device.
   */
  virtual unsigned int write(const data_t* buffer, unsigned int num_bytes, timeout_t timeout) = 0;
};



/*! \struct usb_device::device_description
 *  \brief Descriptor struct for conveying metadata about a USB device.
 *  
 *  Descriptor struct containing metadata about a \ref usb_device, including
 *  identification codes, onboard configurations, interfaces, alternate
 *  settings, and endpoints.
 */
struct usb_device::device_description
{
  /*!
   *  \brief The main constructor for the class.
   *  
   *  Main constructor for the struct. Initializes dynamically allocated arrays.
   *  Once these arrays have been allocated at construction time, they cannot
   *  be resized; only their contents can be modified.
   *  
   *  \param [in] num_configurations The number of configurations available on
   *         the device. This value directly determines the number of
   *         elements allocated in the \ref configurations array.
   */
  explicit device_description(unsigned int num_configurations);
  
  /*!
   *  /brief The copy constructor for the struct.
   *  
   *  Copy constructor for the struct. Performs a deep copy of every element
   *  in the current struct, as well as of every element it contains pointers
   *  to.
   *  
   *  \param [in] other The original object to copy.
   */
  device_description(const device_description& other);
  
  /*!
   *  \brief Destructor.
   *  
   *  Destructor for the struct. Releases dynamically allocated resources and
   *  deletes any \ref device_configuration objects referenced in
   *  \ref configurations.
   */
  ~device_description();
  
  
  
  /*! \brief Integer representing the class of device connected. */
  int                     device_class;
  
  /*! \brief Assigned vendor identifier for the device. */
  int                     vendor_id;
  
  /*! \brief Unique product identifier assigned by vendor. */
  int                     product_id;
  
  /*! \brief The total number of configurations onboard the device. */
  const unsigned int      num_configurations;
  
  /*! \brief Dynamically allocated array of configuration descriptors. */
  device_configuration**  const configurations;
};



/*! \struct usb_device::device_configuration
 *  \brief Descriptor struct for a single configuration on the deivce.
 *  
 *  Descriptor for a configuration on a device. Contains metadata on the
 *  configuration, including the unique identifier of a configuration and
 *  descriptors of the interfaces found in this configuration.
 */
struct usb_device::device_configuration  
{
  /*!
   *  \brief Main constructor for the struct.
   *  
   *  Main constructor for the class. Dynamically allocates necessary resources
   *  based on parameters passed to it.
   *  
   *  \param [in] num_interfaces The number of interfaces contained in this
   *         configuration. This value is directly used for one-time dynamic
   *         resource allocation, which means data dependent on this value
   *         cannot be changed after this constructor is called.
   */
  explicit device_configuration(unsigned int num_interfaces);
  
  /*!
   *  \brief Copy constructor for the struct.
   *  
   *  Copy constructor for the struct. Performs a deep copy so as to make the
   *  new instance of the struct completely and entirely independent from the
   *  original.
   *  
   *  \param [in] other The original object to copy.
   */
  device_configuration(const device_configuration& other);
  
  /*!
   *  \brief Destructor.
   *  
   *  Destructor for the struct. Releases dynamically allocated resources and
   *  deletes any \ref device_interface objects referenced in \ref interfaces.
   */
  ~device_configuration();
  
  
  
  /*!
   *  \brief The unique identifier for this configuration. This is not the same
   *         as the index number used to access this struct from its parent
   *         \ref device_description object.
   */
  unsigned int              config_id;
  
  /*! \brief The number of interfaces contained in this configuration. */
  const unsigned int        num_interfaces;
  
  /*! \brief Dynamic array of interface descriptors. */
  device_interface** const  interfaces;
};



/*! \struct usb_device::device_interface
 *  \brief Descriptor struct for a single interface on a single configuration
 *         on a \ref usb_device.
 *  
 *  Descriptor for an interface on the device.. Contains metadata on the
 *  interface, including unique identifier and the alternate settings supported
 *  by this interface.
 */
struct usb_device::device_interface
{
  /*!
   *  \brief Main constructor for the struct.
   *  
   *  Main constructor for the struct. Dynamically allocates resources based
   *  on input parameters.
   *  
   *  \param [in] num_alt_settings The number of alternate settings to prepare
   *         this object for.
   */
  explicit device_interface(unsigned int num_alt_settings);
  
  /*!
   *  \brief Copy constructor for the struct.
   *  
   *  Copy constructor for the struct. Performs a deep copy so that all the
   *  new object is completely independent from the original instance.
   *  
   *  \param [in] other The original object to copy.
   */
  device_interface(const device_interface& other);
  
  /*!
   *  \brief Destructor.
   *  
   *  Destructor for the struct. Releases dynamically allocated resources and
   *  deletes any \ref device_alt_setting objects referenced in
   *  \ref alt_settings.
   */
  ~device_interface();
  
  
  
  /*!
   *  \brief Unique identifier for the interface. This is not the same as the
   *         index number used to access this struct from its parent
   *         \ref device_configuration object.
   */
  usb_device::interface_t interface_id;
  
  /*! \brief Number of alternate settings supported by this interface. */
  const unsigned int      num_alt_settings;
  
  /*! \brief Array of alternate settings supported by this interface. */
  device_alt_setting**    const alt_settings;
};



/*! \struct usb_device::device_alt_setting
 *  \brief Descriptor struct for a single alternate setting on a single
 *         interface on a single configuration on a \ref usb_device.
 *  
 *  Descriptor struct for an alternate setting on the device. Contains metadata
 *  about the alternate setting, including the unique identifier for the
 *  interface to which it belongs, the alternate setting's unique identifier,
 *  the number of endpoints located in this alternate setting, and descriptors
 *  for the endpoints themselves.
 */
struct usb_device::device_alt_setting
{
  /*!
   *  \brief Main constructor for the struct.
   *  
   *  Main constructor for the struct. Dynamically allocates resources based on
   *  input parameters.
   *  
   *  \param [in] num_endpoints The number of endpoints contained by this
   *         alternate setting.
   */
  explicit device_alt_setting(unsigned int num_endpoints);
  
  /*!
   *  \brief Copy constructor for the struct.
   *  
   *  Copy constructor for the struct. Performs a deep copy of all elements
   *  owned by this object so that the new instance is completely independent
   *  from the original.
   *  
   *  \param [in] other The original object to copy.
   */
  device_alt_setting(const device_alt_setting& other);
  
  /*!
   *  \brief Destructor.
   *  
   *  Destructor for the struct. Releases dynamically allocated resources and
   *  deletes any \ref device_endpoint objects referenced from \ref endpoints.
   */
  ~device_alt_setting();
  
  
  
  /*!
   *  \brief The unique identifier of the parent \ref device_interface object.
   */
  usb_device::interface_t   interface_id;
  
  /*!
   *  \brief Unique identifier for the alternate setting. This is not the same
   *         as the index number used to access this struct from the parent
   *         \ref device_interface object.
   */
  usb_device::alt_setting_t alt_setting_id;
  
  /*! \brief The number of endpoints contained in this alternate setting. */
  const unsigned int        num_endpoints;
  
  /*! \brief Array of endpoint descriptors. */
  device_endpoint** const   endpoints;
};



/*! \struct usb_device::device_endpoint
 *  \brief Descriptor struct for a single endpoint of an alternate setting of
 *         an interface of a configuration on a \ref usb_device. And a partridge
 *         in a pear tree.
 *  
 *  Descriptor struct for an endpoint on the device. Contains metadata about the
 *  endpoint, including the endpoint's address, its transfer type, the 
 */
struct usb_device::device_endpoint
{
  /*!
   *  \brief Default constructor for the struct.
   *  
   *  Default constructor for this struct.
   */
  device_endpoint();
  
  /*!
   *  \brief Copy constructor for the struct.
   *  
   *  Copy constructor for the struct. Performs a deep copy of each property so
   *  that the new instance is completely independent from the original.
   *  
   *  \param [in] other The original object to copy.
   */
  device_endpoint(const device_endpoint& other);
  
  
  
  /*! \brief Address of the endpoint. */
  usb_device::endpoint_t              address;
  
  /*! \brief Type of data supported by this endpoint. */
  usb_device::endpoint_transfer_type  transfer_type;
  
  /*! \brief The direction of data flow of this endpoint. */
  usb_device::endpoint_direction      direction;
};

}

#endif /* defined(__USB_DEVICE_H__) */
