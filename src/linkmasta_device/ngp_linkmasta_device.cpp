//
//  ngp_linkmasta_flash_device.cpp
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ngp_linkmasta_device.h"
#include "usb/usb_device.h"
#include "cartridge/ngp_cartridge.h"
#include "ngp_linkmasta_messages.h"
#include "tasks/task_controller.h"
#include <limits>

using namespace usb;

typedef ngp_linkmasta_device::data_t     data_t;
typedef ngp_linkmasta_device::word_t     word_t;
typedef ngp_linkmasta_device::timeout_t  timeout_t;
typedef ngp_linkmasta_device::version_t  version_t;
typedef ngp_linkmasta_device::chip_index chip_index;



#define NGP_LINKMASTA_VENDOR_ID_1       0x20A0
#define NGP_LINKMASTA_PRODUCT_ID_1      0x4178
#define NGP_LINKMASTA_VENDOR_ID_2       0x20A0
#define NGP_LINKMASTA_PRODUCT_ID_2      0x4256
#define NGP_LINKMASTA_USB_CONFIGURATION 0x01
#define NGP_LINKMASTA_USB_INTERFACE     0X00
#define NGP_LINKMASTA_USB_ALT_SETTING   0X00
#define NGP_LINKMASTA_USB_ENDPOINT_IN   0x81
#define NGP_LINKMASTA_USB_ENDPOINT_OUT  0x02
#define NGP_LINKMASTA_USB_RXTX_SIZE     64
#define NGP_LINKMASTA_USB_TIMEOUT       2000

using namespace ngpmsg;



// CONSTRUCTORS, INITIALIZERS, AND DESTRUCTORS

ngp_linkmasta_device::ngp_linkmasta_device(usb_device* usb_device)
  : m_usb_device(usb_device),
    m_was_init(false), m_is_open(false), m_firmware_version_set(false),
    m_firmware_major_version(0), m_firmware_minor_version(0)
{
  // Nothing else to do
}

ngp_linkmasta_device::~ngp_linkmasta_device()
{
  if (m_is_open)
  {
    try
    {
      close();
    }
    catch (std::exception& ex)
    {
      // Do nothing, fail silently
    }
  }
  
  if (m_was_init)
  {
    delete m_usb_device;
  }
}

void ngp_linkmasta_device::init()
{
  if (m_was_init)
  {
    return;
  }
  
  // Initiailze USB device
  m_usb_device->init();
  
  // Validate device
  const usb_device::device_description* desc;
  desc = m_usb_device->get_device_description();
  
  if ((desc->vendor_id != NGP_LINKMASTA_VENDOR_ID_1 || desc->product_id != NGP_LINKMASTA_PRODUCT_ID_1)
      && (desc->vendor_id != NGP_LINKMASTA_VENDOR_ID_2 || desc->product_id != NGP_LINKMASTA_PRODUCT_ID_2))
  {
    throw std::runtime_error("USB Device not identified as Neo Linkmasta");
  }
  
  // Set device configuration
  m_usb_device->set_timeout(NGP_LINKMASTA_USB_TIMEOUT);
  m_usb_device->set_configuration(NGP_LINKMASTA_USB_CONFIGURATION);
  m_usb_device->set_interface(NGP_LINKMASTA_USB_INTERFACE);
  m_usb_device->set_input_endpoint(NGP_LINKMASTA_USB_ENDPOINT_IN);
  m_usb_device->set_output_endpoint(NGP_LINKMASTA_USB_ENDPOINT_OUT);
  
  m_was_init = true;
}



bool ngp_linkmasta_device::is_open() const
{
  return m_is_open;
}

timeout_t ngp_linkmasta_device::timeout() const
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  return m_usb_device->timeout();
}

version_t ngp_linkmasta_device::firmware_version()
{
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR");
  }
  
  if (!m_firmware_version_set)
  {
    fetch_firmware_version();
  }
  
  return std::to_string(m_firmware_major_version) + "."
         + std::to_string(m_firmware_minor_version);
}

bool ngp_linkmasta_device::is_integrated_with_cartridge() const
{
  return m_usb_device->get_device_description()->product_id == NGP_LINKMASTA_PRODUCT_ID_2;
}



void ngp_linkmasta_device::set_timeout(timeout_t timeout)
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  return m_usb_device->set_timeout(timeout);
}



void ngp_linkmasta_device::open()
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  // Make sure object isn't already open
  if (m_is_open)
  {
    return;
  }
  
  m_usb_device->open();
  
  m_is_open = true;
}

void ngp_linkmasta_device::close()
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  // Make sure object is open
  if (!m_is_open)
  {
    return;
  }
  
  m_usb_device->close();
  
  m_is_open = false;
}

word_t ngp_linkmasta_device::read_word(chip_index chip, address_t address)
{
  // Make sure we are in a ready state
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  uint8_t data;
  data_t buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_read_command(buffer, address, chip);
  m_usb_device->write(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  if (get_read_reply(buffer, &address, &data))
  {
    return data;
  }
  else
  {
    throw std::runtime_error("Error occured when reading word from device");
  }
}

void ngp_linkmasta_device::write_word(chip_index chip, address_t address, word_t data)
{
  // Make sure we are in a ready state
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  uint8_t result;
  data_t buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_write_command(buffer, address, (uint8_t) data, chip);
  m_usb_device->write(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  get_result_reply(buffer, &result);
  if (result == MSG_RESULT_SUCCESS)
  {
    // Yay!
  }
  else
  {
    throw std::runtime_error("Error occured while attempting to write word to device");
  }
}

bool ngp_linkmasta_device::test_for_cartridge()
{
  if (is_integrated_with_cartridge())
  {
    return true;
  }
  else
  {
    return ngp_cartridge::test_for_cartridge(this);
  }
}

cartridge* ngp_linkmasta_device::build_cartridge()
{
  ngp_cartridge* cart = new ngp_cartridge(this);
  cart->init();
  return cart;
}



linkmasta_system ngp_linkmasta_device::system() const
{
  return linkmasta_system::LINKMASTA_NEO_GEO_POCKET;
}

bool ngp_linkmasta_device::supports_read_bytes() const
{
  return true;
}

bool ngp_linkmasta_device::supports_program_bytes() const
{
  return true;
}



unsigned int ngp_linkmasta_device::read_bytes(chip_index chip, address_t start_address, data_t *buffer, unsigned int num_bytes, task_controller* controller)
{
  // Make sure we are in a ready state
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  // Some working variables
  data_t   _buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  
  // Inform the controller that the task has begun
  if (controller != nullptr)
  {
    controller->on_task_start(num_bytes);
  }
  
  // Get as many bytes of data as possible in packets of 64
  while ((num_bytes - offset) / NGP_LINKMASTA_USB_RXTX_SIZE >= 1
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    // Calculate number of packets. Don't go over packet limit
    unsigned int num_packets = (num_bytes - offset) / NGP_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }
    
    build_read64xN_command(_buffer, start_address + offset, chip, num_packets);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    for (unsigned int packets_i = 0; packets_i < num_packets; ++packets_i)
    {
      // Get response from device and write directly to buffer
      if (m_usb_device->read(&buffer[offset], NGP_LINKMASTA_USB_RXTX_SIZE) != NGP_LINKMASTA_USB_RXTX_SIZE)
      {
        throw std::runtime_error("Unexpected number of bytes received from USB device");
      }
      
      // Update offset and inform controller of progress
      offset += NGP_LINKMASTA_USB_RXTX_SIZE;
      if (controller != nullptr)
      {
        controller->on_task_update(task_status::RUNNING, NGP_LINKMASTA_USB_RXTX_SIZE);
      }
    }
  }
  
  // Get any remaining bytes of data individually
  while (num_bytes - offset > 0
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    build_read_command(_buffer, start_address + offset, chip);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    uint32_t address;
    uint8_t  data;
    if (get_read_reply(_buffer, &address, &data))
    {
      buffer[offset] = data;
    }
    else
    {
      throw std::runtime_error("ERROR");
    }
    
    // Update offset and inform controller of progress
    ++offset;
    if (controller != nullptr)
    {
      controller->on_task_update(task_status::RUNNING, 1);
    }
  }
  
  // Inform controller that task is complete
  if (controller != nullptr)
  {
    controller->on_task_end(offset < num_bytes && controller->is_task_cancelled() ?  task_status::CANCELLED : task_status::COMPLETED, offset);
  }
  return offset;
}

unsigned int ngp_linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t *buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller)
{
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR");
  }
  
  // Some working variables
  data_t   _buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  uint8_t  result;
  
  // Inform controller that task has started
  if (controller != nullptr)
  {
    controller->on_task_start(num_bytes);
  }
  
  // Inform device of incoming data
  while ((num_bytes - offset) / NGP_LINKMASTA_USB_RXTX_SIZE >= 1
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    // Makes sure we don't go over the packet limit
    unsigned int num_packets = (num_bytes - offset) / NGP_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }
    
    build_flash_write64xN_command(_buffer, start_address + offset, chip, num_packets, bypass_mode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    // Send chunks of 64 bytes to device
    for (unsigned int packet_i = 0; packet_i < num_packets; ++packet_i)
    {
      build_flash_write64xN_data_packet(_buffer, &buffer[offset]);
      m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
      
      // Update offset and inform controller of progress
      offset += NGP_LINKMASTA_USB_RXTX_SIZE;
      if (controller != nullptr)
      {
        controller->on_task_update(task_status::RUNNING, NGP_LINKMASTA_USB_RXTX_SIZE);
      }
    }
    
    // Verify that operaton worked
    uint8_t packets_processed;
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    get_flash_write64xN_reply(_buffer, &result, &packets_processed);
    
    if(result != MSG_WRITE64xN_REPLY)
    {
      throw std::runtime_error("Unexpected reply from device");
    }
    if(packets_processed != num_packets)
    {
      throw std::runtime_error("Unexpected number of packets processed");
    }
  }
  
  // If at least 32 bytes remain, write them
  while (num_bytes - offset >= NGP_LINKMASTA_USB_RXTX_SIZE / 2
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    build_flash_write_32_command(_buffer, start_address + offset, &buffer[offset], chip, bypass_mode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      throw std::runtime_error("Error occured while attempting to program bytes");
    }
    
    // Update offset and inform controller of progress
    offset += NGP_LINKMASTA_USB_RXTX_SIZE / 2;
    if (controller != nullptr)
    {
      controller->on_task_update(task_status::RUNNING, NGP_LINKMASTA_USB_RXTX_SIZE / 2);
    }
  }
  
  // If any bytes remain, write them
  while (num_bytes - offset > 0
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    unsigned int num_bytes_ = num_bytes;
    if (num_bytes_ >= NGP_LINKMASTA_USB_RXTX_SIZE / 2)
    {
      num_bytes_ = (NGP_LINKMASTA_USB_RXTX_SIZE / 2) - 1;
    }
    
    build_flash_write_N_command(_buffer, start_address + offset, &buffer[offset], chip, num_bytes_ - offset, bypass_mode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      if (controller != nullptr)
      {
        controller->on_task_end(task_status::ERROR, offset);
      }
      throw std::runtime_error("Error occured while attempting to program bytes");
    }
    
    // Update offset and inform controller of progress
    offset += num_bytes_ - offset;
    if (controller != nullptr)
    {
      controller->on_task_end(task_status::RUNNING, num_bytes - offset);
    }
  }
  
  // Inform controller that task is complete
  if (controller != nullptr)
  {
    controller->on_task_end(offset < num_bytes && controller->is_task_cancelled() ? task_status::CANCELLED : task_status::COMPLETED, offset);
  }
  return offset;
}



void ngp_linkmasta_device::fetch_firmware_version()
{
  data_t buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  build_getversion_command(buffer);
  
  unsigned int num_bytes;
  
  // Send command
  num_bytes = m_usb_device->write(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  
  // Fetch reply
  num_bytes = m_usb_device->read(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  if (num_bytes != NGP_LINKMASTA_USB_RXTX_SIZE)
  {
    throw std::runtime_error("Unexpected number of bytes received");
  }
  
  uint8_t majVer, minVer;
  get_getversion_reply(buffer, &majVer, &minVer);
  
  m_firmware_major_version = (unsigned int) majVer;
  m_firmware_minor_version = (unsigned int) minVer;
  m_firmware_version_set = true;
}


