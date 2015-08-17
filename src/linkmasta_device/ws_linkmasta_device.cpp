//
//  ws_linkmasta_device.cpp
//  FlashMasta
//
//  Created by Dan on 8/14/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ws_linkmasta_device.h"
#include "usb/usb_device.h"
#include "ws_linkmasta_messages.h"
#include "tasks/task_controller.h"
#include <limits>

using namespace usb;

typedef ws_linkmasta_device::data_t     data_t;
typedef ws_linkmasta_device::word_t     word_t;
typedef ws_linkmasta_device::timeout_t  timeout_t;
typedef ws_linkmasta_device::version_t  version_t;
typedef ws_linkmasta_device::chip_index chip_index;

#define WS_LINKMASTA_VENDOR_ID          0x20A0
#define WS_LINKMASTA_PRODUCT_ID         0x4252
#define WS_LINKMASTA_USB_CONFIGURATION  0x01
#define WS_LINKMASTA_USB_INTERFACE      0X00
#define WS_LINKMASTA_USB_ALT_SETTING    0X00
#define WS_LINKMASTA_USB_ENDPOINT_IN    0x81
#define WS_LINKMASTA_USB_ENDPOINT_OUT   0x02
#define WS_LINKMASTA_USB_RXTX_SIZE      64
#define WS_LINKMASTA_USB_TIMEOUT        2000

using namespace wsmsg;



ws_linkmasta_device::ws_linkmasta_device(usb::usb_device* usb_device)
  : m_usb_device(usb_device),
    m_was_init(false), m_is_open(false), m_firmware_version_set(false),
    m_firmware_major_version(0), m_firmware_minor_version(0)
{
  // Nothing else to do
}

ws_linkmasta_device::~ws_linkmasta_device()
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

void ws_linkmasta_device::init()
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
  
  if (desc->vendor_id != WS_LINKMASTA_VENDOR_ID
      || desc->product_id != WS_LINKMASTA_PRODUCT_ID)
  {
    throw std::runtime_error("USB Device not identified as Linkmasta");
  }
  
  // Set device configuration
  m_usb_device->set_timeout(WS_LINKMASTA_USB_TIMEOUT);
  m_usb_device->set_configuration(WS_LINKMASTA_USB_CONFIGURATION);
  m_usb_device->set_interface(WS_LINKMASTA_USB_INTERFACE);
  m_usb_device->set_input_endpoint(WS_LINKMASTA_USB_ENDPOINT_IN);
  m_usb_device->set_output_endpoint(WS_LINKMASTA_USB_ENDPOINT_OUT);
  
  m_was_init = true;
}



bool ws_linkmasta_device::is_open() const
{
  return m_is_open;
}

timeout_t ws_linkmasta_device::timeout() const
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  return m_usb_device->timeout();
}

version_t ws_linkmasta_device::firmware_version()
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



void ws_linkmasta_device::set_timeout(timeout_t timeout)
{
  // Make sure object was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR: Object not initialized");
  }
  
  return m_usb_device->set_timeout(timeout);
}



void ws_linkmasta_device::open()
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

void ws_linkmasta_device::close()
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

word_t ws_linkmasta_device::read_word(chip_index chip, address_t address)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  uint16_t data;
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_read16_command(buffer, address);
  m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  if (get_read16_reply(buffer, &address, &data))
  {
    return data;
  }
  else
  {
    throw std::runtime_error("ERROR"); // TODO
  }
}

void ws_linkmasta_device::write_word(chip_index chip, address_t address, word_t data)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  uint8_t result;
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_write16_command(buffer, address, data);
  m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
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



bool ws_linkmasta_device::supports_read_bytes() const
{
  return true;
}

bool ws_linkmasta_device::supports_program_bytes() const
{
  return true;
}



unsigned int ws_linkmasta_device::read_bytes(chip_index chip, address_t start_address, data_t *buffer, unsigned int num_bytes, task_controller* controller)
{
  // Make sure we are in a ready state
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Some working variables
  data_t   _buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  
  // Because of WS's curious 2-byte words, always remove lower order bit
  start_address = start_address & (address_t) ~1;
  num_bytes = num_bytes & (unsigned int) ~1;
  
  // Inform the controller that the task has begun
  if (controller != nullptr)
  {
    controller->on_task_start(num_bytes);
  }
  
  // Get as many bytes of data as possible in packets of 64
  while ((offset - num_bytes) / WS_LINKMASTA_USB_RXTX_SIZE >= 1
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    // Calculate number of packets. Don't go over packet limit
    unsigned int num_packets = (num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }
    
    build_read64xN_command(_buffer, start_address + offset, num_packets);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    for (unsigned int packets_i = 0; packets_i < num_packets; ++packets_i)
    {
      // Get response from device and write directly to buffer
      if (m_usb_device->read(&buffer[offset], WS_LINKMASTA_USB_RXTX_SIZE) != WS_LINKMASTA_USB_RXTX_SIZE)
      {
        throw std::runtime_error("ERROR"); // TODO
      }
      
      // Update offset and inform controller of progress
      offset += WS_LINKMASTA_USB_RXTX_SIZE;
      if (controller != nullptr)
      {
        controller->on_task_update(task_status::RUNNING, WS_LINKMASTA_USB_RXTX_SIZE);
      }
    }
  }
  
  // Get any remaining bytes of data individually
  while (num_bytes - offset > 0
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    build_read16_command(_buffer, start_address + offset);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    m_usb_device->read(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    uint32_t address;
    uint16_t data;
    if (get_read16_reply(_buffer, &address, &data) == MSG_RESULT_SUCCESS)
    {
      buffer[offset] = (data_t) (data >> 8);
      buffer[offset+1] = (data_t) (data);
    }
    else
    {
      throw std::runtime_error("ERROR");
    }
    
    // Update offset and inform controller of progress
    offset += 2;
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

unsigned int ws_linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t *buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller)
{
  if (!m_was_init || !m_is_open)
  {
    throw std::runtime_error("ERROR");
  }
  
  // Some working variables
  data_t   _buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  uint8_t  result;
  
  // Because of WS's curious 2-byte words, always remove lower order bit
  start_address = start_address & (address_t) ~1;
  num_bytes = num_bytes & (unsigned int) ~1;
  
  // Inform controller that task has started
  if (controller != nullptr)
  {
    controller->on_task_start(num_bytes);
  }
  
  // Program in packets of 64 bytes
  while ((num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE >= 1
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    // Makes sure we don't go over the packet limit
    unsigned int num_packets = (num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }
    
    build_flash_write64xN_command(_buffer, start_address + offset, num_packets);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    // Send chunks of 64 bytes to device
    for (unsigned int packet_i = 0; packet_i < num_packets; ++packet_i)
    {
      build_flash_write64xN_data_packet(_buffer, &buffer[offset]);
      m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
      
      // Update offset and inform controller of progress
      offset += WS_LINKMASTA_USB_RXTX_SIZE;
      if (controller != nullptr)
      {
        controller->on_task_update(task_status::RUNNING, WS_LINKMASTA_USB_RXTX_SIZE);
      }
    }
    
    // Verify that operaton worked
    uint8_t packets_processed;
    m_usb_device->read(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
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
  while (num_bytes - offset >= WS_LINKMASTA_USB_RXTX_SIZE / 2
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    build_flash_write_32_command(_buffer, start_address + offset, &buffer[offset]);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      throw std::runtime_error("ERROR"); // TODO
    }
    
    // Update offset and inform controller of progress
    offset += WS_LINKMASTA_USB_RXTX_SIZE / 2;
    if (controller != nullptr)
    {
      controller->on_task_update(task_status::RUNNING, WS_LINKMASTA_USB_RXTX_SIZE / 2);
    }
  }
  
  // If any bytes remain, write them
  while (num_bytes - offset > 0
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    unsigned int num_bytes_ = num_bytes;
    if (num_bytes_ >= WS_LINKMASTA_USB_RXTX_SIZE / 2)
    {
      num_bytes_ = (WS_LINKMASTA_USB_RXTX_SIZE / 2) - 1;
    }
    
    build_flash_write_N_command(_buffer, start_address + offset, &buffer[offset], num_bytes_ - offset);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      if (controller != nullptr)
      {
        controller->on_task_end(task_status::ERROR, offset);
      }
      throw std::runtime_error("ERROR"); // TODO
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



void ws_linkmasta_device::fetch_firmware_version()
{
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  build_getversion_command(buffer);
  
  unsigned int num_bytes;
  
  // Send command
  num_bytes = m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  // Fetch reply
  num_bytes = m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  if (num_bytes != WS_LINKMASTA_USB_RXTX_SIZE)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  uint8_t majVer, minVer;
  get_getversion_reply(buffer, &majVer, &minVer);
  
  m_firmware_major_version = (unsigned int) majVer;
  m_firmware_minor_version = (unsigned int) minVer;
  m_firmware_version_set = true;
}


