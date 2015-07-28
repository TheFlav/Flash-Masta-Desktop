//
//  ngp_linkmasta_flash_device.cpp
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ngp_linkmasta_flash_device.h"
#include "usb_device/usb_device.h"
#include "cartridge/ngp_cartridge.h"
#include "ngp_linkmasta_messages.h"



typedef ngp_linkmasta_flash_device::timeout_t  timeout_t;
typedef ngp_linkmasta_flash_device::version_t  version_t;
typedef ngp_linkmasta_flash_device::chip_index chip_index;



#define NGP_LINKMASTA_VENDOR_ID         0x20A0
#define NGP_LINKMASTA_PRODUCT_ID        0x4178
#define NGP_LINKMASTA_USB_CONFIGURATION 0x00
#define NGP_LINKMASTA_USB_INTERFACE     0X00
#define NGP_LINKMASTA_USB_ALT_SETTING   0X00
#define NGP_LINKMASTA_USB_ENDPOINT_IN   0x81
#define NGP_LINKMASTA_USB_ENDPOINT_OUT  0x02
#define NGP_LINKMASTA_USB_RXTX_SIZE     64
#define NGP_LINKMASTA_USB_TIMEOUT       2000



// CONSTRUCTORS, INITIALIZERS, AND DESTRUCTORS

ngp_linkmasta_flash_device::ngp_linkmasta_flash_device(usb_device* usb_device)
  : m_usb_device(usb_device),
    m_was_init(false), m_is_open(false), m_firmware_version_set(false),
    m_firmware_major_version(0), m_firmware_minor_version(0)
{
  // Nothing else to do
}

ngp_linkmasta_flash_device::~ngp_linkmasta_flash_device()
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

void ngp_linkmasta_flash_device::init()
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



inline bool ngp_linkmasta_flash_device::is_open() const
{
  return m_is_open;
}

inline timeout_t ngp_linkmasta_flash_device::timeout() const
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  return m_usb_device->timeout();
}

version_t ngp_linkmasta_flash_device::firmware_version()
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



inline void ngp_linkmasta_flash_device::set_timeout(timeout_t timeout)
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  return m_usb_device->set_timeout(timeout);
}



void ngp_linkmasta_flash_device::open()
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

void ngp_linkmasta_flash_device::close()
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

unsigned int ngp_linkmasta_flash_device::read(chip_index chip, address_t start_address, data_t *buffer, unsigned int num_bytes)
{
  return read(chip, start_address, buffer, num_bytes, timeout());
}

unsigned int ngp_linkmasta_flash_device::read(chip_index chip, address_t start_address, data_t *buffer, unsigned int num_bytes, timeout_t timeout)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Some working variables
  data_t   _buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  int      mode;
  
  // Determine mode based on number of bytes to fetch
  if (num_bytes == 1)
    mode = 1;
  else if (num_bytes <= 64)
    mode = 2;
  else
    mode = 3;
  
  // Compose read byte command
  switch (mode)
  {
    case 1:
      build_read_command(_buffer, start_address, chip);
      break;
      
    case 2:
      build_read64_command(_buffer, start_address, chip);
      break;
      
    case 3:
      build_read64xN_command(_buffer, start_address, chip, num_bytes / 64 + (num_bytes % 64 == 0 ? 0 : 1));
      break;
  }
  
  // Send command to device
  m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
  
  // Process response
  switch (mode)
  {
    case 1:
      // Read response
      if (m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout) != NGP_LINKMASTA_USB_RXTX_SIZE)
      {
        throw std::runtime_error("ERROR"); // TODO
      }
      get_read_reply(_buffer, &start_address, buffer);
      break;
      
    case 2:
    case 3:
      // Fetch 64-byte blocks
      for (unsigned int i = 0;
           i < num_bytes && num_bytes - i >= NGP_LINKMASTA_USB_RXTX_SIZE;
           i += NGP_LINKMASTA_USB_RXTX_SIZE)
      {
        if (m_usb_device->read(buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout) != NGP_LINKMASTA_USB_RXTX_SIZE)
        {
          throw std::runtime_error("ERROR"); // TODO
        }
      }
      
      if (num_bytes % NGP_LINKMASTA_USB_RXTX_SIZE == 0)
      {
        break;
      }
      
      // Fetch remainder of bytes
      if (m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout) != NGP_LINKMASTA_USB_RXTX_SIZE)
      {
        throw std::runtime_error("ERROR"); // TODO
      }
      for (unsigned int i = 0; i < num_bytes % NGP_LINKMASTA_USB_RXTX_SIZE; ++i)
      {
        buffer[num_bytes / NGP_LINKMASTA_USB_RXTX_SIZE + i] = _buffer[i];
      }
      break;
  }
  
  return num_bytes;
}

unsigned int ngp_linkmasta_flash_device::write(chip_index chip, address_t start_address, const data_t *buffer, unsigned int num_bytes)
{
  return write(chip, start_address, buffer, num_bytes, timeout());
}

unsigned int ngp_linkmasta_flash_device::write(chip_index chip, address_t start_address, const data_t *buffer, unsigned int num_bytes, timeout_t timeout)
{
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR");
  }
  
  // Some working variables
  data_t   _buffer[NGP_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset;
  bool     bypassMode;
  uint8_t  result;
  
  offset = 0;
  bypassMode = false;
  
  // Inform device of incoming data
  if (num_bytes / NGP_LINKMASTA_USB_RXTX_SIZE >= 1)
  {
    unsigned int num_packets = num_bytes / NGP_LINKMASTA_USB_RXTX_SIZE;
    build_flash_write64xN_command(_buffer, start_address + offset, chip, num_packets, bypassMode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
    
    // Send chunks of 64 bytes to device
    for (offset = 0;
         offset < num_packets * NGP_LINKMASTA_USB_RXTX_SIZE;
         offset += NGP_LINKMASTA_USB_RXTX_SIZE)
    {
      build_flash_write64xN_data_packet(_buffer, &buffer[offset]);
      m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
    }
    
    // Verify that operaton worked
    uint8_t packets_processed;
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
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
  if (num_bytes - offset >= NGP_LINKMASTA_USB_RXTX_SIZE / 2)
  {
    build_flash_write_32_command(_buffer, start_address + offset, &buffer[offset], chip, bypassMode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      throw std::runtime_error("ERROR"); // TODO
    }
    
    offset += NGP_LINKMASTA_USB_RXTX_SIZE / 2;
  }
  
  // If any bytes remain, write them
  if (num_bytes - offset > 0)
  {
    build_flash_write_N_command(_buffer, start_address + offset, &buffer[offset], chip, num_bytes - offset, bypassMode);
    m_usb_device->write(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, NGP_LINKMASTA_USB_RXTX_SIZE, timeout);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      throw std::runtime_error("ERROR"); // TODO
    }
    
    offset += num_bytes - offset;
  }
  
  return num_bytes;
}



void ngp_linkmasta_flash_device::fetch_firmware_version()
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
    throw std::runtime_error("ERROR");
  }
  
  uint8_t majVer, minVer;
  get_getversion_reply(buffer, &majVer, &minVer);
  
  m_firmware_major_version = (unsigned int) majVer;
  m_firmware_major_version = (unsigned int) minVer;
  m_firmware_version_set = true;
}


