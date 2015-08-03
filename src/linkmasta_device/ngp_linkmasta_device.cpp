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
#include <limits>

using namespace usb;

typedef ngp_linkmasta_device::data_t     data_t;
typedef ngp_linkmasta_device::timeout_t  timeout_t;
typedef ngp_linkmasta_device::version_t  version_t;
typedef ngp_linkmasta_device::chip_index chip_index;



#define NGP_LINKMASTA_VENDOR_ID         0x20A0
#define NGP_LINKMASTA_PRODUCT_ID        0x4178
#define NGP_LINKMASTA_USB_CONFIGURATION 0x01
#define NGP_LINKMASTA_USB_INTERFACE     0X00
#define NGP_LINKMASTA_USB_ALT_SETTING   0X00
#define NGP_LINKMASTA_USB_ENDPOINT_IN   0x81
#define NGP_LINKMASTA_USB_ENDPOINT_OUT  0x02
#define NGP_LINKMASTA_USB_RXTX_SIZE     64
#define NGP_LINKMASTA_USB_TIMEOUT       2000




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
    close();
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
  
  if (desc->vendor_id != NGP_LINKMASTA_VENDOR_ID
      || desc->product_id != NGP_LINKMASTA_PRODUCT_ID)
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



inline void ngp_linkmasta_device::set_timeout(timeout_t timeout)
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

data_t ngp_linkmasta_device::read_byte(chip_index chip, address_t address)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  data_t data;
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
    throw std::runtime_error("ERROR"); // TODO
  }
}

void ngp_linkmasta_device::write_byte(chip_index chip, address_t address, data_t data)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  uint8_t result;
  data_t buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_write_command(buffer, address, data, chip);
  m_usb_device->write(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
  get_result_reply(buffer, &result);
  if (result == MSG_RESULT_SUCCESS)
  {
    // Yay!
  }
  else
  {
    throw std::runtime_error("ERROR"); // TODO
  }
}



bool ngp_linkmasta_device::supports_read_bytes() const
{
  return true;
}

bool ngp_linkmasta_device::supports_program_bytes() const
{
  return true;
}

bool ngp_linkmasta_device::supports_erase_chip() const
{
  return false;
}

bool ngp_linkmasta_device::supports_erase_chip_block() const
{
  return false;
}

bool ngp_linkmasta_device::supports_read_manufacturer_id() const
{
  return false;
}

bool ngp_linkmasta_device::supports_read_device_id() const
{
  return false;
}

bool ngp_linkmasta_device::supports_read_block_protection() const
{
  return false;
}



unsigned int ngp_linkmasta_device::read_bytes(chip_index chip, address_t start_address, data_t *buffer, unsigned int num_bytes)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Some working variables
  data_t   _buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  
  
  // Get as many bytes of data as possible in packets of 64
  while ((offset - num_bytes) / NGP_LINKMASTA_USB_RXTX_SIZE >= 1)
  {
    unsigned int num_packets = (num_bytes - offset) / NGP_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }
    
    build_read64xN_command(_buffer, start_address + offset, chip, num_packets);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    for (unsigned int packets_i = 0;
         packets_i < num_packets;
         ++packets_i, offset += NGP_LINKMASTA_USB_RXTX_SIZE)
    {
      // Get response from device and write directly to buffer
      if (m_usb_device->read(&buffer[offset], NGP_LINKMASTA_USB_RXTX_SIZE) != NGP_LINKMASTA_USB_RXTX_SIZE)
      {
        throw std::runtime_error("ERROR"); // TODO
      }
    }
  }
  
  // Get any remaining bytes of data individually
  while (num_bytes - offset > 0)
  {
    build_read_command(_buffer, start_address + offset, chip);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    uint32_t address;
    uint8_t  data;
    if (get_read_reply(_buffer, &address, &data) == MSG_RESULT_SUCCESS)
    {
      buffer[offset] = data;
    }
    else
    {
      throw std::runtime_error("ERROR");
    }
    ++offset;
  }
  
  // Yeah, yeah, yeah, I know, but exceptions get thrown if something goes wrong
  // so don't judge me.
  return num_bytes;
}

unsigned int ngp_linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t *buffer, unsigned int num_bytes, bool bypass_mode)
{
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR");
  }
  
  // Some working variables
  data_t   _buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset;
  uint8_t  result;
  
  offset = 0;
  
  // Inform device of incoming data
  while ((num_bytes - offset) / NGP_LINKMASTA_USB_RXTX_SIZE >= 1)
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
    for (unsigned int packet_i = 0;
         packet_i < num_packets;
         ++packet_i, offset += NGP_LINKMASTA_USB_RXTX_SIZE)
    {
      build_flash_write64xN_data_packet(_buffer, &buffer[offset]);
      m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    }
    
    // Verify that operaton worked
    uint8_t packets_processed;
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    get_flash_write64xN_reply(_buffer, &result, &packets_processed);
    
    if(result != MSG_WRITE64xN_REPLY)
    {
      throw std::runtime_error("ERROR"); // TODO
    }
    if(packets_processed != num_packets)
    {
      throw std::runtime_error("ERROR"); // TODO
    }
  }
  
  // If at least 32 bytes remain, write them
  while (num_bytes - offset >= NGP_LINKMASTA_USB_RXTX_SIZE / 2)
  {
    build_flash_write_32_command(_buffer, start_address + offset, &buffer[offset], chip, bypass_mode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      throw std::runtime_error("ERROR"); // TODO
    }
    
    offset += NGP_LINKMASTA_USB_RXTX_SIZE / 2;
  }
  
  // If any bytes remain, write them
  while (num_bytes - offset > 0)
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
      throw std::runtime_error("ERROR"); // TODO
    }
    
    offset += num_bytes_ - offset;
  }
  
  return num_bytes;
}

void ngp_linkmasta_device::erase_chip(chip_index chip)
{
  throw std::runtime_error("ERROR: NOT SUPPORTED"); // TODO
}

void ngp_linkmasta_device::erase_chip_block(chip_index chip, address_t block_address)
{
  throw std::runtime_error("ERROR: NOT SUPPORTED"); // TODO
}

unsigned int ngp_linkmasta_device::read_manufacturer_id(chip_index chip)
{
  throw std::runtime_error("ERROR: NOT SUPPORTED"); // TODO
}

unsigned int ngp_linkmasta_device::read_device_id(chip_index chip)
{
  throw std::runtime_error("ERROR: NOT SUPPORTED"); // TODO
}

bool ngp_linkmasta_device::read_block_protection(chip_index chip, address_t block_address)
{
  throw std::runtime_error("ERROR: NOT SUPPORTED"); // TODO
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
    throw std::runtime_error("ERROR"); // TODO
  }
  
  uint8_t majVer, minVer;
  get_getversion_reply(buffer, &majVer, &minVer);
  
  m_firmware_major_version = (unsigned int) majVer;
  m_firmware_major_version = (unsigned int) minVer;
  m_firmware_version_set = true;
}


