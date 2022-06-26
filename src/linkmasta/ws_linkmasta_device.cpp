/*! \file
 *  \brief File containing the implementation of \ref ws_linkmasta_device.
 *  
 *  File containing the implementation of \ref ws_linkmasta_device. See
 *  corresponding header file to view documentation for the class, its methods,
 *  and its member variables.
 *  
 *  \see ws_linkmasta_device
 *  \see linkmasta_device
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-14
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "ws_linkmasta_device.h"
#include "usb/usb_device.h"
#include "ws_linkmasta_messages.h"
#include "task/task_controller.h"
#include "cartridge/ws_cartridge.h"
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
    m_slot_info_set(false), m_firmware_major_version(0),
    m_firmware_minor_version(0), m_static_num_slots(false),
    m_static_slot_sizes(false)
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
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  if (!m_firmware_version_set)
  {
    fetch_firmware_version();
  }
  
  return std::to_string(m_firmware_major_version) + "."
  + std::to_string(m_firmware_minor_version);
}

bool ws_linkmasta_device::is_integrated_with_cartridge() const
{
  return true;
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
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  uint8_t data;
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_read8_command(buffer, address, chip);
  m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  if (get_read8_reply(buffer, &address, &data))
  {
    return data;
  }
  else
  {
    throw std::runtime_error("Error occured while attempting to read word");
  }
}

void ws_linkmasta_device::write_word(chip_index chip, address_t address, word_t data)
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
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_write8_command(buffer, address, data, chip);
  m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  get_result_reply(buffer, &result);
  if (result != MSG_RESULT_SUCCESS)
  {
    throw std::runtime_error("Error occured while attempting to write word");
  }
}

bool ws_linkmasta_device::test_for_cartridge()
{
  return true;
}

cartridge* ws_linkmasta_device::build_cartridge()
{
  ws_cartridge* cart = new ws_cartridge(this);
  cart->init();
  return cart;
}



linkmasta_system ws_linkmasta_device::system() const
{
  return linkmasta_system::LINKMASTA_WONDERSWAN;
}

bool ws_linkmasta_device::supports_read_bytes() const
{
  return true;
}

bool ws_linkmasta_device::supports_program_bytes() const
{
  return true;
}

bool ws_linkmasta_device::supports_read_num_slots() const
{
  return true;
}

bool ws_linkmasta_device::supports_read_slot_size() const
{
  return true;
}

bool ws_linkmasta_device::supports_switch_slot() const
{
  return true;
}



unsigned int ws_linkmasta_device::read_bytes(chip_index chip, address_t start_address, data_t *buffer, unsigned int num_bytes, task_controller* controller)
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
  data_t   _buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  
  // Inform the controller that the task has begun
  if (controller != nullptr)
  {
    controller->on_task_start(num_bytes);
  }
  
  // Get as many bytes of data as possible in packets of 64
  while ((num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE >= 1
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    // Calculate number of packets. Don't go over packet limit
    unsigned int num_packets = (num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }
    
    build_read64xN_command(_buffer, start_address + offset, num_packets, chip);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    for (unsigned int packets_i = 0; packets_i < num_packets; ++packets_i)
    {
      // Get response from device and write directly to buffer
      if (m_usb_device->read(&buffer[offset], WS_LINKMASTA_USB_RXTX_SIZE) != WS_LINKMASTA_USB_RXTX_SIZE)
      {
        throw std::runtime_error("Unexpected number of bytes received");
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
    build_read8_command(_buffer, start_address + offset, chip);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    m_usb_device->read(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    uint32_t address;
    uint8_t data;
    if (get_read8_reply(_buffer, &address, &data))
    {
      buffer[offset] = (data_t) (data);
    }
    else
    {
      throw std::runtime_error("ERROR");
    }
    
    // Update offset and inform controller of progress
    offset += 1;
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

static bool program_operation_is_noop(chip_index chip, const data_t *buffer, unsigned int num_bytes)
{
  if (chip == target_enum::TARGET_ROM)
  {
    for (unsigned int i = 0; i < num_bytes; i++)
    {
      if (((const uint8_t *) buffer)[i] != 0xFF)
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

unsigned int ws_linkmasta_device::program_bytes(chip_index chip, address_t start_address, const data_t *buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller)
{
  (void) bypass_mode;
  
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  // Validate chip index
  if (chip != target_enum::TARGET_ROM
      && chip != target_enum::TARGET_SRAM)
  {
    throw std::invalid_argument("Unexpected chip value " + std::to_string(chip));
  }
  
  // Some working variables
  data_t   _buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  unsigned int offset = 0;
  uint8_t  result;
  
  // Inform controller that task has started
  if (controller != nullptr)
  {
    controller->on_task_start(num_bytes);
  }
  
  // Read in packets of 64 bytes
  while ((num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE >= 1
         && (controller == nullptr || !controller->is_task_cancelled()))
  {
    // Makes sure we don't go over the packet limit
    unsigned int num_packets = (num_bytes - offset) / WS_LINKMASTA_USB_RXTX_SIZE;
    if (num_packets > std::numeric_limits<uint8_t>::max())
    {
      num_packets = std::numeric_limits<uint8_t>::max();
    }

    if (chip == target_enum::TARGET_ROM)
    {
      // Optimization: If the next packet contains only 0xFF bytes, a write to a
      // NOR-based flash chip will be equivalent to a no-operation. Skip.
      if (program_operation_is_noop(chip, &buffer[offset], WS_LINKMASTA_USB_RXTX_SIZE))
      {
        // Update offset and inform controller of progress
        offset += WS_LINKMASTA_USB_RXTX_SIZE;
        if (controller != nullptr)
        {
          controller->on_task_update(task_status::RUNNING, WS_LINKMASTA_USB_RXTX_SIZE);
        }
        continue;
      }

      // Optimization: Do the same to the right-hand side, from a certain minimum
      // burst size.
      uint8_t first_skippable_num_packets = 0;
      uint8_t skippable_num_packets_count = 0;
      for (uint8_t i = 4; i < num_packets; i++)
      {
        if (program_operation_is_noop(chip, &buffer[offset + (WS_LINKMASTA_USB_RXTX_SIZE * i)],
          WS_LINKMASTA_USB_RXTX_SIZE))
        {
          if (first_skippable_num_packets == 0)
          {
            first_skippable_num_packets = i;
            skippable_num_packets_count = 0;
          }
          skippable_num_packets_count++;
          if (skippable_num_packets_count >= 4)
          {
            num_packets = first_skippable_num_packets;
            break;
          }
        }
        else
        {
          first_skippable_num_packets = 0;
        }
      }
    }

    // Treat writes to flash and sram differently
    switch (chip)
    {
    case target_enum::TARGET_ROM:
      build_flash_write64xN_command(_buffer, start_address + offset, num_packets);
      break;
      
    case target_enum::TARGET_SRAM:
      build_sram_write64xN_command(_buffer, start_address + offset, num_packets);
      break;
    }
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    // Send chunks of 64 bytes to device
    for (unsigned int packet_i = 0; packet_i < num_packets; ++packet_i)
    {
      // Tread writes to flash and sram differently
      switch (chip)
      {
      case target_enum::TARGET_ROM:
        build_flash_write64xN_data_packet(_buffer, &buffer[offset]);
        break;
        
      case target_enum::TARGET_SRAM:
        build_sram_write64xN_data_packet(_buffer, &buffer[offset]);
        break;
      }

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
    get_write64xN_reply(_buffer, &result, &packets_processed);
    
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
  // Only do this if we're programming the flash chip
  while (num_bytes - offset >= WS_LINKMASTA_USB_RXTX_SIZE / 2
         && (controller == nullptr || !controller->is_task_cancelled())
         && chip == target_enum::TARGET_ROM)
  {
    build_flash_write_32_command(_buffer, start_address + offset, &buffer[offset]);
    m_usb_device->write(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    
    // Verify that operation worked
    m_usb_device->read(_buffer, WS_LINKMASTA_USB_RXTX_SIZE);
    get_result_reply(_buffer, &result);
    if (result != MSG_RESULT_SUCCESS)
    {
      throw std::runtime_error("Error occured while attempting to program bytes");
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
    switch (chip)
    {
    case target_enum::TARGET_ROM:
      {
        unsigned int num_bytes_ = num_bytes;
        if (num_bytes_ > WS_LINKMASTA_USB_RXTX_SIZE / 2)
        {
          num_bytes_ = (WS_LINKMASTA_USB_RXTX_SIZE / 2);
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
          throw std::runtime_error("Error occured while attempting to program bytes");
        }
        
        // Update offset and inform controller of progress
        offset += num_bytes_ - offset;
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::RUNNING, num_bytes - offset);
        }
      }
      break;
      
    case target_enum::TARGET_SRAM:
      {
        build_write8_command(_buffer, start_address + offset, buffer[offset], chip);
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
          throw std::runtime_error("Error occured while attempting to program bytes");
        }
        
        // Update offset and inform controller of progress
        offset += 1;
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::RUNNING, 1);
        }
      }
      break;
    }
  }
  
  // Inform controller that task is complete
  if (controller != nullptr)
  {
    controller->on_task_end(offset < num_bytes && controller->is_task_cancelled() ? task_status::CANCELLED : task_status::COMPLETED, offset);
  }
  return offset;
}

unsigned int ws_linkmasta_device::read_num_slots()
{
  // Make sure object has been initialized at least
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  if (!m_slot_info_set)
  {
    fetch_slot_info();
  }
  
  if (m_static_num_slots)
  {
    return m_num_slots;
  }
  else
  {
    return 0; // TODO: Not implemented
  }
}

unsigned int ws_linkmasta_device::read_slot_size(unsigned int slot_num)
{
  (void) slot_num;
  
  // Make sure object has been initialized at least
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  if (!m_slot_info_set)
  {
    fetch_slot_info();
  }
  
  if (m_static_slot_sizes)
  {
    return m_slot_size;
  }
  else
  {
    return 0;
  }
}

bool ws_linkmasta_device::switch_slot(unsigned int slot_num)
{
  // Make sure object has been initialized at least
  if (!m_was_init)
  {
    throw std::runtime_error("Device not initialized");
  }
  if (!m_is_open)
  {
    throw std::runtime_error("Device not opened");
  }
  
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  
  build_set_cartslot_command(buffer, (unsigned char) slot_num);
  m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  uint8_t result;
  get_result_reply(buffer, &result);
  
  return (result == MSG_RESULT_SUCCESS);
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
    throw std::runtime_error("Unexpected number of bytes received");
  }
  
  uint8_t majVer, minVer;
  get_getversion_reply(buffer, &majVer, &minVer);
  
  m_firmware_major_version = (unsigned int) majVer;
  m_firmware_minor_version = (unsigned int) minVer;
  m_firmware_version_set = true;
}

void ws_linkmasta_device::fetch_slot_info()
{
  data_t buffer[WS_LINKMASTA_USB_RXTX_SIZE] = {0};
  build_getcartinfo_command(buffer);
  
  unsigned int num_bytes;
  
  // Send command
  num_bytes = m_usb_device->write(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  
  // Fetch reply
  num_bytes = m_usb_device->read(buffer, WS_LINKMASTA_USB_RXTX_SIZE);
  if (num_bytes != WS_LINKMASTA_USB_RXTX_SIZE)
  {
    throw std::runtime_error("Unexpected number of bytes received");
  }
  
  uint8_t isSlotNumFixed, isSlotSizeFixed, numSlotsPerCart, numAddrLinesPerSlot;
  get_getcartinfo_reply(buffer, &isSlotNumFixed, &isSlotSizeFixed, &numSlotsPerCart, &numAddrLinesPerSlot);
  
  m_static_num_slots = (isSlotNumFixed == 1);
  m_static_slot_sizes = (isSlotSizeFixed == 1);
  m_num_slots = (unsigned int) numSlotsPerCart;
  m_slot_size = 1 << numAddrLinesPerSlot;
}
