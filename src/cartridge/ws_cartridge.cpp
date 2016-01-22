/*! \file
 *  \brief File containing the implementation of \ref ws_cartridge.
 *  
 *  File containing the implementation of \ref ws_cartridge.
 *  
 *  See corrensponding header file to view documentation for class, its
 *  methods, and its member variables.
 *  
 *  \see ws_cartridge
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-17
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "ws_cartridge.h"
#include "linkmasta/linkmasta_device.h"
#include "ws_rom_chip.h"
#include "ws_sram_chip.h"
#include "task/task_controller.h"
#include "task/forwarding_task_controller.h"
#include <fstream>
#include <sstream>
#include <iomanip>

//#ifdef VERBOSE
#include <iostream>
//#endif

using namespace std;

#define DEFAULT_BLOCK_SIZE 0x20000
#define DEFAULT_SRAM_SIZE  0x400000



ws_cartridge::ws_cartridge(linkmasta_device* linkmasta)
  : m_was_init(false), m_linkmasta(linkmasta), m_descriptor(nullptr),
    m_rom_chip(new ws_rom_chip(m_linkmasta)), m_sram_chip(new ws_sram_chip(m_linkmasta))
{
  // Nothing else to do
}

ws_cartridge::~ws_cartridge()
{
  if (m_descriptor != nullptr)
  {
    delete m_descriptor;
  }
  
  if (m_rom_chip != nullptr)
  {
    delete m_rom_chip;
  }
  
  if (m_sram_chip != nullptr)
  {
    delete m_sram_chip;
  }
}



system_type ws_cartridge::system() const
{
  return system_type::SYSTEM_WONDERSWAN;
}

cartridge_type ws_cartridge::type() const
{
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  return cartridge_type::CARTRIDGE_FLASHMASTA;
}

const cartridge_descriptor* ws_cartridge::descriptor() const
{
  return m_descriptor;
}



void ws_cartridge::init()
{
  if (m_was_init)
  {
    return;
  }
  m_was_init = true;
  
  m_linkmasta->init();
  m_linkmasta->open();
  m_rom_chip->reset();
  m_rom_chip->select_slot(0);
  build_cartridge_destriptor();
  build_slots_layout();
  build_game_metadata();
  m_linkmasta->close();
}

void ws_cartridge::backup_cartridge_game_data(std::ostream& fout, int slot, task_controller* controller)
{
  // Wonderswan games are stored in the upper addresses of a chip. That means
  // game metadata is stored at the very top (highest addresses) and the rest
  // of the game is just below that. Games may not necessarily occupy the whole
  // chip, just the upper addresses.
  // 
  // This procedure reads the size of the ROM from the top of the slot and only
  // backs up only the relevant data.
  
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  m_linkmasta->open();
  
  // Validate arguments
  if (slot != SLOT_ALL && (slot < 0 || slot >= (int) m_slots.size()))
  {
    throw std::invalid_argument("invalid slot number: " + std::to_string(slot));
  }
  else if (!m_rom_chip->select_slot(slot == SLOT_ALL ? 0 : (unsigned int) slot))
  {
    throw std::runtime_error("Error occured while attempting to switch slot");
  }
  
  m_linkmasta->close();
  
  unsigned int curr_slot = (slot == SLOT_ALL ? 0 : (unsigned int) slot);
  unsigned int slot_size = this->slot_size(curr_slot);
  
  // Determine the total number of bytes to write
  unsigned int bytes_written = 0;
  unsigned int bytes_total = 0;
  
  // Special case full-cartridge backup
  if (slot == SLOT_ALL)
  {
    bytes_total = descriptor()->num_bytes;
  }
  else
  {
    const game_metadata* metadata = get_game_metadata(slot);
    switch (metadata->rom_size)
    {
      case 0x0002:
        bytes_total = 0x80000; // 4Mbit (512KB)
        break;
        
      case 0x0003:
        bytes_total = 0x100000; // 8Mbit (1MB)
        break;
        
      case 0x0004:
        bytes_total = 0x200000; // 16Mbit (2MB)
        break;
        
      case 0x0006:
        bytes_total = 0x400000; // 32Mbit (4MB)
        break;
        
      case 0x0008:
        bytes_total = 0x800000; // 64Mbit (8MB)
        break;
        
      case 0x0009:
        bytes_total = 0x1000000; // 128Mbit (16MB)
        break;
        
      default:
        bytes_total = slot_size;
        break;
    }
  }
  
  // Initialize markers
  unsigned int curr_chip = 0;
  unsigned int curr_offset = (slot == SLOT_ALL ? 0 : slot_size - bytes_total);
  unsigned int curr_block = 0;
  unsigned int curr_slot_offset = 0;
  
  if (slot != SLOT_ALL)
  {
    // Calculate current slot's offset
    for (unsigned int i = 0; i < curr_slot; i++)
    {
      curr_slot_offset += this->slot_size(i);
    }
    
    // Figure out the current block
    for (unsigned int i = 0; i < descriptor()->chips[0]->num_blocks; ++i)
    {
      if (descriptor()->chips[0]->blocks[i]->base_address <= (curr_slot_offset + curr_offset)
          && descriptor()->chips[0]->blocks[i]->base_address
          + descriptor()->chips[0]->blocks[i]->num_bytes > (curr_slot_offset + curr_offset))
      {
        curr_block = i;
        break;
      }
    }
  }
  
  // Allocate a buffer with max size of a block
  const unsigned int BUFFER_MAX_SIZE = DEFAULT_BLOCK_SIZE;
  unsigned int       buffer_size = 0;
  unsigned char*     buffer = new unsigned char[BUFFER_MAX_SIZE];
  
  // Inform controller that task is starting
  if (controller != nullptr)
  {
    controller->on_task_start(bytes_total);
  }
  
  // Begin writing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    while (bytes_written < bytes_total && curr_chip < descriptor()->num_chips && (controller == nullptr || !controller->is_task_cancelled()))
    {
#ifdef VERBOSE
      std::cout << "(chip " << curr_chip << ", block " << curr_block << ") " << bytes_written << " B / " << bytes_total << " B (" << (bytes_written * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Convenience variables
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Calculate number of expected bytes
      unsigned int bytes_expected = (block->base_address + block->num_bytes) - (curr_slot_offset + curr_offset);
      if (bytes_expected > bytes_total - bytes_written)
      {
        // Make sure we don't write more bytes than we initially expected
        bytes_expected = bytes_total - bytes_written;
      }
      if (bytes_expected + curr_offset > slot_size)
      {
        // Make sure we don't overstep the slot bounds
        bytes_expected = slot_size - curr_offset;
      }
      if (bytes_expected > BUFFER_MAX_SIZE)
      {
        // Make sure we don't write too many bytes
        bytes_expected = BUFFER_MAX_SIZE;
      }
      
      // Attempt to read bytes from cartridge
      if (controller == nullptr)
      {
        buffer_size = m_rom_chip->read_bytes(curr_offset, buffer, bytes_expected);
      }
      else
      {
        // Create a forwarding controller to pass progress updates to
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          buffer_size = m_rom_chip->read_bytes(curr_offset, buffer, bytes_expected, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Check for errors
      if (buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fout.good())
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Write buffer to file
      fout.write((char*) buffer, buffer_size);
      
      // Update markers
      bytes_written += buffer_size;
      curr_offset += buffer_size;
      if (curr_slot_offset + curr_offset >= block->base_address + block->num_bytes)
      {
        curr_block++;
      }
      if (curr_block >= chip->num_blocks)
      {
        curr_block = 0;
        curr_chip++;
      }
      if (curr_offset >= slot_size)
      {
        curr_slot_offset += slot_size;
        curr_offset -= slot_size;
        if (curr_slot < num_slots() - 1)
        {
          curr_slot++;
          m_linkmasta->switch_slot(curr_slot);
        }
        slot_size = this->slot_size(curr_slot);
      }
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    // Error occured! Clean up and pass error on to caller
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_rom_chip->is_erasing());
    } catch (std::exception& ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_rom_chip->reset();
    } catch (std::exception& ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    try {
      m_linkmasta->close();
    } catch (std::exception& ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    // Inform controller of task end
    if (controller != nullptr)
    {
      controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
    }
    delete [] buffer;
    throw;
  }
  
  // Inform controller of task end
  if (controller != nullptr)
  {
    controller->on_task_end(controller->is_task_cancelled() && bytes_written < bytes_total ? task_status::CANCELLED : task_status::COMPLETED, bytes_written);
  }
  delete [] buffer;
}

void ws_cartridge::restore_cartridge_game_data(std::istream& fin, int slot, task_controller* controller)
{
  // Due to how WonderSwan games are read and stored on a cart, the game's meta
  // data is stored in the upper addresses. Because of how cartridges are made,
  // a WonderSwan can read from 0xFFFFFFFF and never be wrong because if the
  // chip is too small, the upper lines will simply not be connected, meaning
  // the device can consistently access the upper addresses of the cartridge.
  //
  // This procedure will write to the upper addresses only and ignore any
  // lower addresses. This will work as long as the game uses only relative
  // addressing and never absolute addressing. Or, if it does use absolute
  // addressing, will always have the higher bits set to 1.
  
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  m_linkmasta->open();
  
  // Validate arguments
  if ((slot < 0 || slot >= (int) m_slots.size()) && slot != SLOT_ALL)
  {
    throw std::invalid_argument("invalid slot number: " + std::to_string(slot));
  }
  
  // Determine the total number of bytes to write
  fin.seekg(0, fin.end);
  unsigned int bytes_written = 0;
  unsigned int bytes_total = (unsigned int) fin.tellg();
  fin.seekg(0, fin.beg);
  
  // Ensure file will fit
  unsigned int curr_slot = (slot == SLOT_ALL ? 0 : (unsigned int) slot);
  unsigned int slot_size = this->slot_size(curr_slot);
  
  if (m_rom_chip->selected_slot() != curr_slot && !m_rom_chip->select_slot(curr_slot))
  {
    throw std::runtime_error("Error occured while attempting to switch slot");
  }
  if ((slot == SLOT_ALL && bytes_total != descriptor()->num_bytes)
      || (slot != SLOT_ALL && bytes_total > slot_size))
  {
    throw std::runtime_error("File too large for destination");
  }
  
  m_linkmasta->close();
  
  // Initialize markers
  unsigned int curr_chip = 0;
  unsigned int curr_offset = (slot == SLOT_ALL ? 0 : slot_size - bytes_total);
  unsigned int curr_block = 0;
  
  // Search for starting block, since it's not clear exactly where it will be
  unsigned int slot_offset = 0;
  for (unsigned int i = 0; i < (unsigned int) curr_slot; ++i)
  {
    slot_offset += this->slot_size(i);
  }
  for (unsigned int i = 0; i < descriptor()->chips[0]->num_blocks; ++i)
  {
    if (descriptor()->chips[0]->blocks[i]->base_address <= (slot_offset + curr_offset)
        && descriptor()->chips[0]->blocks[i]->base_address
        + descriptor()->chips[0]->blocks[i]->num_bytes > (slot_offset + curr_offset))
    {
      curr_block = i;
      break;
    }
  }
  
  // Allocate a buffer with max size of a block
  const unsigned int BUFFER_MAX_SIZE = DEFAULT_BLOCK_SIZE;
  unsigned int       buffer_size = 0;
  unsigned char*     buffer = new unsigned char[BUFFER_MAX_SIZE];
  
  // Inform controller that task is starting
  if (controller != nullptr)
  {
    controller->on_task_start(bytes_total);
  }
  
  // Begin writing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    while (bytes_written < bytes_total && (controller == nullptr || !controller->is_task_cancelled()))
    {
#ifdef VERBOSE
      std::cout << "(chip " << curr_chip << ", block " << curr_block << ") " << bytes_written << " B / " << bytes_total << " B (" << (bytes_written * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Convenience variables
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Calculate number of expected bytes
      unsigned bytes_expected = block->num_bytes - (curr_offset + slot_offset - block->base_address);
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      if (bytes_expected > BUFFER_MAX_SIZE)
      {
        bytes_expected = BUFFER_MAX_SIZE;
      }
      if (bytes_expected > slot_size - curr_offset)
      {
        bytes_expected = slot_size - curr_offset;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) buffer, bytes_expected);
      buffer_size = ((unsigned int) fin.tellg()) - bytes_written;
      
      // Check for errors
      if (buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fin.good() && (buffer_size + bytes_written) < bytes_total)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Erase block from cartridge
      m_rom_chip->erase_block(block->base_address);
      
      // Wait for erasure to complete
      while (m_rom_chip->test_erasing())
      {
        // Give UI an opportunity to update
        if (controller != nullptr)
        {
          controller->on_task_update(task_status::RUNNING, 0);
        }
      }
      
      // Write buffer to cartridge
      if (controller == nullptr)
      {
        m_rom_chip->program_bytes(curr_offset, buffer, buffer_size);
      }
      else
      {
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(buffer_size);
        try
        {
          m_rom_chip->program_bytes(curr_offset, buffer, buffer_size, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Update markers
      bytes_written += buffer_size;
      curr_offset += buffer_size;
      if (curr_offset + slot_offset >= block->base_address + block->num_bytes)
      {
        curr_block++;
      }
      if (curr_block >= chip->num_blocks)
      {
        curr_block = 0;
        curr_chip++;
      }
      if (curr_offset >= slot_size)
      {
        slot_offset += slot_size;
        curr_offset -= slot_size;
        if (curr_slot < num_slots() - 1)
        {
          curr_slot++;
          m_linkmasta->switch_slot(curr_slot);
        }
        slot_size = this->slot_size(curr_slot);
      }
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    // Error occured! Clean up and pass error on to caller
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_rom_chip->is_erasing());
    } catch (exception ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_rom_chip->reset();
    } catch (exception ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    try {
      m_linkmasta->close();
    } catch (exception ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    if (controller != nullptr)
    {
      std::cout << controller->get_task_work_progress() << std::endl;
      controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
    }
    delete [] buffer;
    throw;
  }
  
  // Inform controller that task has ended
  if (controller != nullptr)
  {
    controller->on_task_end(controller->is_task_cancelled() && bytes_written < bytes_total ? task_status::CANCELLED : task_status::COMPLETED, bytes_written);
  }
  delete [] buffer;
}

bool ws_cartridge::compare_cartridge_game_data(std::istream& fin, int slot, task_controller* controller)
{
  // WonderSwan games store their metadata at the top of the chip on which they
  // reside. Thus, we only compare the contents of the file with the upper-most
  // addresses in the slot
  //
  // NOTE: When verifying individual slots, starts at the TOP addresses and scans
  // down to the lower addresses. When verifying all slots, starts at the BOTTOM
  // addresses and scans up.
  
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  m_linkmasta->open();
  
  // Validate arguments
  unsigned int curr_slot = (slot == SLOT_ALL ? 0 : (unsigned int) slot);
  unsigned int slot_size = this->slot_size(curr_slot);
  
  if ((slot < 0 || slot >= (int) this->num_slots()) && slot != SLOT_ALL)
  {
    throw std::invalid_argument("invalid slot number: " + std::to_string(slot));
  }
  else if (m_rom_chip->selected_slot() != curr_slot)
  {
    m_rom_chip->select_slot(curr_slot);
  }
  
  // determine the total number of bytes to compare
  fin.seekg(0, fin.end);
  unsigned int bytes_compared = 0;
  unsigned int bytes_total = (unsigned int) fin.tellg();
  fin.seekg(0, fin.beg);
  
  // Ensure file will fit
  if (bytes_total > (slot == SLOT_ALL ? descriptor()->num_bytes : slot_size))
  {
    return false;
  }
  
  // Initialize markers
  unsigned int curr_chip = 0;
  unsigned int curr_offset = (slot == SLOT_ALL ? 0 : slot_size);
  unsigned int curr_block = 0;
  bool         matched = true;
  
  // Search for starting block, since it's not clear exactly where it will be
  unsigned int slot_offset = 0;
  for (unsigned int i = 0; i < (unsigned int) curr_slot; ++i)
  {
    slot_offset += this->slot_size(i);
  }
  for (unsigned int i = 0; i < descriptor()->chips[0]->num_blocks; ++i)
  {
    if (descriptor()->chips[0]->blocks[i]->base_address <= (slot_offset + curr_offset)
        && descriptor()->chips[0]->blocks[i]->base_address
        + descriptor()->chips[0]->blocks[i]->num_bytes > (slot_offset + curr_offset))
    {
      curr_block = i;
      break;
    }
  }
  if (slot != SLOT_ALL)
  {
    curr_block--;
    curr_offset = descriptor()->chips[0]->blocks[curr_block]->base_address - slot_offset;
  }
  
  m_linkmasta->close();
  
  // Allocate a buffer with max size of a block
  const unsigned int BUFFER_MAX_SIZE = DEFAULT_BLOCK_SIZE;
  unsigned int       f_buffer_size = 0;
  unsigned char*     f_buffer = new unsigned char[BUFFER_MAX_SIZE];
  unsigned int       c_buffer_size = 0;
  unsigned char*     c_buffer = new unsigned char[BUFFER_MAX_SIZE];
  
  // Inform controller that task is starting
  if (controller != nullptr)
  {
    controller->on_task_start(bytes_total);
  }
  
  // Begin comparing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    while (bytes_compared < bytes_total && matched && (controller == nullptr || !controller->is_task_cancelled()))
    {
#ifdef VERBOSE
      std::cout << "(chip " << curr_chip << ", block " << curr_block << ") " << bytes_compared << " B / " << bytes_total << " B (" << (bytes_compared * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Convenience variables
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Calculate number of expected bytes
      unsigned bytes_expected = block->num_bytes - (curr_offset + slot_offset - block->base_address);
      if (bytes_expected > bytes_total - bytes_compared)
      {
        bytes_expected = bytes_total - bytes_compared;
      }
      if (bytes_expected > slot_size - curr_offset)
      {
        bytes_expected = slot_size - curr_offset;
      }
      
      // Attempt to read bytes from file
      if (slot != SLOT_ALL)
      {
        fin.seekg(-((int) (slot_size - curr_offset)), fin.end);
      }
      streamoff f_start = fin.tellg();
      fin.read((char*) f_buffer, bytes_expected);
      streamoff f_end = fin.tellg();
      f_buffer_size = (unsigned int) (f_end - f_start);
      
      // Check for errors
      if (f_buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fin.good() && (f_buffer_size + bytes_compared) < bytes_total)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Attempt to read bytes from cartridge
      if (controller == nullptr)
      {
        c_buffer_size = m_rom_chip->read_bytes(curr_offset, c_buffer, bytes_expected);
      }
      else
      {
        // Use a forwarding controller to pass progress updates to our controller
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          c_buffer_size = m_rom_chip->read_bytes(curr_offset, c_buffer, bytes_expected, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
          // Inform controller of error
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Check for errors
      if (c_buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Compare contents of buffers
      for (unsigned int i = 0; i < f_buffer_size && i < c_buffer_size; ++i)
      {
        if (f_buffer[i] != c_buffer[i])
        {
          matched = false;
          break;
        }
      }
      
      // Update markers
      bytes_compared += f_buffer_size;
      curr_offset += f_buffer_size;
      if (slot_offset + curr_offset >= block->base_address + block->num_bytes)
      {
        if (slot == SLOT_ALL)
        {
          curr_block++;
        }
        else
        {
          // Step backwards if verifying individual slot
          curr_block--;
          curr_offset = chip->blocks[curr_block]->base_address - slot_offset;
        }
      }
      if (curr_block >= chip->num_blocks)
      {
        curr_block = 0;
        curr_chip++;
      }
      if (curr_offset >= slot_size)
      {
        slot_offset += slot_size;
        curr_offset -= slot_size;
        if (curr_slot < num_slots() - 1)
        {
          curr_slot++;
          m_linkmasta->switch_slot(curr_slot);
        }
        slot_size = this->slot_size(curr_slot);
      }
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    // Error occured! Clean up and pass error on to caller
    // Note: I appologize for the change in style: it's to save lines
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_rom_chip->is_erasing());
    } catch (std::exception &ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_rom_chip->reset();
    } catch (std::exception &ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    try {
      m_linkmasta->close();
    } catch (std::exception &ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    // Inform controller that error
    if (controller != nullptr)
    {
      controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
    }
    delete [] f_buffer;
    delete [] c_buffer;
    throw;
  }
  
  // Inform controller that task has ended
  if (controller != nullptr)
  {
    controller->on_task_end(controller->is_task_cancelled() && bytes_compared < bytes_total ? task_status::CANCELLED : task_status::COMPLETED, bytes_compared);
  }
  delete [] f_buffer;
  delete [] c_buffer;
  return matched;
}

void ws_cartridge::backup_cartridge_save_data(std::ostream& fout, int slot, task_controller* controller)
{
  (void) slot;
  
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  // Determine the total number of bytes to write
  unsigned int bytes_written = 0;
  unsigned int bytes_total = DEFAULT_SRAM_SIZE; // Always assume 4 Mib chip
  
  // Allocate a buffer with max size of a block
  const unsigned int BUFFER_MAX_SIZE = DEFAULT_BLOCK_SIZE;
  unsigned int       buffer_size = 0;
  unsigned char*     buffer = new unsigned char[BUFFER_MAX_SIZE];
  
  // Inform controller that task is starting
  if (controller != nullptr)
  {
    controller->on_task_start(bytes_total);
  }
  
  // Begin writing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    while (bytes_written < bytes_total && (controller == nullptr || !controller->is_task_cancelled()))
    {
#ifdef VERBOSE
      std::cout << bytes_written << " B / " << bytes_total << " B (" << (bytes_written * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Calcualte number of expected bytes
      unsigned int bytes_expected = BUFFER_MAX_SIZE;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from cartridge
      if (controller == nullptr)
      {
        buffer_size = m_sram_chip->read_bytes(bytes_written, buffer, bytes_expected);
      }
      else
      {
        // Create a forwarding controller to pass progress updates to
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          buffer_size = m_sram_chip->read_bytes(bytes_written, buffer, bytes_expected, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Check for errors
      if (buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fout.good())
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Write buffer to file
      fout.write((char*) buffer, buffer_size);
      
      // Update markers
      bytes_written += buffer_size;
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    // Error occured! Clean up and pass error on to caller
    try {
      m_linkmasta->close();
    } catch (std::exception& ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    // Inform controller of task end
    if (controller != nullptr)
    {
      controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
    }
    delete [] buffer;
    throw;
  }
  
  // Inform controller of task end
  if (controller != nullptr)
  {
    controller->on_task_end(controller->is_task_cancelled() && bytes_written < bytes_total ? task_status::CANCELLED : task_status::COMPLETED, bytes_written);
  }
  delete [] buffer;
}

void ws_cartridge::restore_cartridge_save_data(std::istream& fin, int slot, task_controller* controller)
{
  (void) slot;
  
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  // Determine the total number of bytes to write
  fin.seekg(0, fin.end);
  unsigned int bytes_written = 0;
  unsigned int bytes_total = (unsigned int) fin.tellg();
  fin.seekg(0, fin.beg);
  
  // Allocate a buffer with max size of a block
  const unsigned int BUFFER_MAX_SIZE = DEFAULT_BLOCK_SIZE;
  unsigned int       buffer_size = 0;
  unsigned char*     buffer = new unsigned char[BUFFER_MAX_SIZE];
  
  // Inform controller that task is starting
  if (controller != nullptr)
  {
    controller->on_task_start(bytes_total);
  }
  
  // Begin writing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    while (bytes_written < bytes_total && (controller == nullptr || !controller->is_task_cancelled()))
    {
#ifdef VERBOSE
      std::cout << bytes_written << " B / " << bytes_total << " B (" << (bytes_written * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = BUFFER_MAX_SIZE;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) buffer, bytes_expected);
      buffer_size = ((unsigned int) fin.tellg()) - bytes_written;
      
      // Check for errors
      if (buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fin.good() && (buffer_size + bytes_written) < bytes_total)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Write buffer to cartridge
      if (controller == nullptr)
      {
        m_sram_chip->program_bytes(bytes_written, buffer, buffer_size);
      }
      else
      {
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(buffer_size);
        try
        {
          m_sram_chip->program_bytes(bytes_written, buffer, buffer_size, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Update markers
      bytes_written += buffer_size;
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    // Error occured! Clean up and pass error on to caller
    try {
      m_linkmasta->close();
    } catch (std::exception &ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    if (controller != nullptr)
    {
      controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
    }
    delete [] buffer;
    throw;
  }
  
  // Inform controller that task has ended
  if (controller != nullptr)
  {
    controller->on_task_end(controller->is_task_cancelled() && bytes_written < bytes_total ? task_status::CANCELLED : task_status::COMPLETED, bytes_written);
  }
  delete [] buffer;
}

bool ws_cartridge::compare_cartridge_save_data(std::istream& fin, int slot, task_controller* controller)
{
  (void) slot;
  
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  // determine the total number of bytes to compare
  fin.seekg(0, fin.end);
  unsigned int bytes_compared = 0;
  unsigned int bytes_total = (unsigned int) fin.tellg();
  fin.seekg(0, fin.beg);
  
  // Initialize markers
  bool         matched = true;
  
  // Allocate a buffer with max size of a block
  const unsigned int BUFFER_MAX_SIZE = DEFAULT_BLOCK_SIZE;
  unsigned int       f_buffer_size = 0;
  unsigned char*     f_buffer = new unsigned char[BUFFER_MAX_SIZE];
  unsigned int       c_buffer_size = 0;
  unsigned char*     c_buffer = new unsigned char[BUFFER_MAX_SIZE];
  
  // Inform controller that task is starting
  if (controller != nullptr)
  {
    controller->on_task_start(bytes_total);
  }
  
  // Begin comparing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    while (bytes_compared < bytes_total && matched && (controller == nullptr || !controller->is_task_cancelled()))
    {
#ifdef VERBOSE
      std::cout << bytes_compared << " B / " << bytes_total << " B (" << (bytes_compared * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = BUFFER_MAX_SIZE;
      if (bytes_expected > bytes_total - bytes_compared)
      {
        bytes_expected = bytes_total - bytes_compared;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) f_buffer, bytes_expected);
      f_buffer_size = ((unsigned int) fin.tellg()) - bytes_compared;
      
      // Check for errors
      if (f_buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fin.good() && (f_buffer_size + bytes_compared) < bytes_total)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Attempt to read bytes from cartridge
      if (controller == nullptr)
      {
        c_buffer_size = m_sram_chip->read_bytes(bytes_compared, c_buffer, bytes_expected);
      }
      else
      {
        // Use a forwarding controller to pass progress updates to our controller
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          c_buffer_size = m_sram_chip->read_bytes(bytes_compared, c_buffer, bytes_expected, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          (void) ex;
          // Inform controller of error
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Check for errors
      if (c_buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      
      // Compare contents of buffers
      for (unsigned int i = 0; i < f_buffer_size && i < c_buffer_size; ++i)
      {
        if (f_buffer[i] != c_buffer[i])
        {
          matched = false;
          break;
        }
      }
      
      // Update markers
      bytes_compared += f_buffer_size;
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    (void) ex;
    // Error occured! Clean up and pass error on to caller
    try {
      m_linkmasta->close();
    } catch (std::exception &ex2) {
      (void) ex2;
      // Well... this is awkward
    }
    
    // Inform controller that error
    if (controller != nullptr)
    {
      controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
    }
    delete [] f_buffer;
    delete [] c_buffer;
    throw;
  }
  
  // Inform controller that task has ended
  if (controller != nullptr)
  {
    controller->on_task_end(controller->is_task_cancelled() && bytes_compared < bytes_total ? task_status::CANCELLED : task_status::COMPLETED, bytes_compared);
  }
  delete [] f_buffer;
  delete [] c_buffer;
  return matched;
}

unsigned int ws_cartridge::num_slots() const
{
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  return (unsigned int) m_slots.size();
}

unsigned int ws_cartridge::slot_size(int slot) const
{
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  if (slot >= 0 && slot < (int) num_slots())
  {
    return m_slots[slot];
  }
  else
  {
    throw std::runtime_error("INVALID SLOT");
  }
}

std::string ws_cartridge::fetch_game_name(int slot)
{
  // Get developer ID from chip
  // Get game ID from chip
  // Check developer ID and game ID against database and get game name
  
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("Cartridge not initialized");
  }
  
  static const unsigned int high_addr = 0x00FFFFFF;
  static const unsigned int foot_addr = high_addr - 0x0000000A;
  
  m_linkmasta->open();
  
  // Jump to selected slot
  if (slot < 0 || slot >= (int) m_linkmasta->read_num_slots())
  {
    throw std::invalid_argument("invalid slot number: " + std::to_string(slot));
  }
  if (!m_linkmasta->switch_slot(slot))
  {
    throw std::runtime_error("Error occured while attempting to switch slot");
  }
  
  // Get developer ID
  ws_rom_chip::word_t w = m_rom_chip->read(foot_addr);
  unsigned char developer_id = (unsigned char) w;
  
  // Get game ID
  w = m_rom_chip->read(foot_addr + 0x00000002);
  unsigned char game_id = (unsigned char) w;
  
  m_linkmasta->close();
  
  std::stringstream r;
  
  r << std::setfill('0') << std::hex;
  
  r << "Developer: " << std::setw(2) << (int) developer_id;
  r << "  Game: " << std::setw(2) << (int) game_id;
  
  return std::string(r.str());
}

const ws_cartridge::game_metadata* ws_cartridge::get_game_metadata(int slot) const
{
  if (slot < 0 || slot >= (int) m_metadata.size()) return nullptr;
  
  return &m_metadata[slot];
}

unsigned int ws_cartridge::get_game_size(int slot) const
{
  if (slot < 0 || slot >= (int) m_metadata.size()) return 0;
  
  return calculate_game_size(m_metadata[slot].rom_size);
}

unsigned int ws_cartridge::calculate_game_size(int size_code)
{
  switch (size_code)
  {
  case 0x02:
    return 1 << 19;
    
  case 0x03:
    return 1 << 20;
    
  case 0x04:
    return 1 << 21;
    
  case 0x06:
    return 1 << 22;
    
  case 0x08:
    return 1 << 23;
    
  case 0x09:
    return 1 << 24;
    
  default:
    return 0;
  }
}



void ws_cartridge::build_cartridge_destriptor()
{
  if (m_descriptor != nullptr)
  {
    delete m_descriptor;
    m_descriptor = nullptr;
  }
  
  // Check if chip exists or not
  if (m_rom_chip->get_manufacturer_id() == 0x90 && m_rom_chip->get_device_id() == 0x90)
  {
    return;
  }
  
  // Initialize cartridge descriptor
  m_descriptor = new cartridge_descriptor(1);
  m_descriptor->system = system_type::SYSTEM_WONDERSWAN;
  m_descriptor->type = cartridge_type::CARTRIDGE_FLASHMASTA;
  m_descriptor->num_bytes = 0;
  
  // Build chip
  build_chip_descriptor(0);
  m_descriptor->num_bytes += m_descriptor->chips[0]->num_bytes;
}

void ws_cartridge::build_chip_descriptor(unsigned int chip_i)
{
  ws_rom_chip* chip = m_rom_chip;
  cartridge_descriptor::chip_descriptor* chip_desc;
  unsigned int num_bytes;
  unsigned int num_blocks;
  
  ws_rom_chip::manufact_id_t manufacturer = chip->get_manufacturer_id();
  ws_rom_chip::device_id_t   device_id    = chip->get_device_id();
  
  // Confirm that chip exists
  if (manufacturer == 0x90 && device_id == 0x90)
  {
    // Stop everything and exit function
    return;
  }
  
  // At the time of this code writing, cartridges contain a single chip
  // with a constant size and block size.
  num_bytes = 0x8000000;
  
  // Calculate number of blocks. (1 block per 64 Kib (8 KiB))
  num_blocks = num_bytes / DEFAULT_BLOCK_SIZE;
  
  // Initialize chip descriptor
  chip_desc = new cartridge_descriptor::chip_descriptor(num_blocks);
  chip_desc->num_bytes = num_bytes;
  chip_desc->chip_num = chip_i;
  chip_desc->manufacturer_id = manufacturer;
  chip_desc->device_id = device_id;
  
  // Add (unfinished) chip descriptor to device descriptor
  m_descriptor->chips[chip_i] = chip_desc;
  
  for (unsigned int i = 0; i < chip_desc->num_blocks; ++i)
  {
    build_block_descriptor(chip_i, i);
  }
}

void ws_cartridge::build_block_descriptor(unsigned int chip_i, unsigned int block_i)
{
  ws_rom_chip* chip = m_rom_chip;
  
  // Initialize block descriptor
  cartridge_descriptor::chip_descriptor::block_descriptor* block;
  block = new cartridge_descriptor::chip_descriptor::block_descriptor();
  block->block_num = block_i;
  
  // Add (unfinished) block descriptor to chip descriptor
  m_descriptor->chips[chip_i]->blocks[block_i] = block;
  
  // Determine size of block. All blocks are of uniform size
  block->num_bytes = DEFAULT_BLOCK_SIZE;
    
  // Determine base address of block based on index of block
  block->base_address = block_i * DEFAULT_BLOCK_SIZE;
  
  // Query chip for protection status of block
  block->is_protected = (chip->get_block_protection(block->base_address) == 0 ? false : true);
}

void ws_cartridge::build_slots_layout()
{
  if (m_rom_chip->current_mode() != ws_rom_chip::chip_mode::READ)
  {
    m_rom_chip->reset();
  }
  
  m_slots.resize(m_linkmasta->read_num_slots());
  m_metadata.clear();
  m_metadata.resize(m_slots.size());
  
  for (unsigned int i = 0; i < m_slots.size(); ++i)
  {
    m_slots[i] = m_linkmasta->read_slot_size(i);
  }
}

void ws_cartridge::build_game_metadata(int slot)
{
  if (m_metadata.empty()) return;
  
  if (slot == -1)
  {
    for (unsigned int i = 0; i < m_metadata.size(); i++)
    {
      build_game_metadata(i);
    }
  }
  else if (slot >= 0 && slot < (int) m_metadata.size())
  {
    unsigned char* buffer = new unsigned char[10];
    
    m_rom_chip->select_slot(slot);
    m_rom_chip->read_bytes(slot_size(slot) - 10, buffer, 10);
    
    m_metadata[slot].read_from_data_array(buffer);
    delete [] buffer;
  }
}



void ws_cartridge::game_metadata::read_from_data_array(const unsigned char* data)
{
  developer_id = data[0];
  minimum_system = data[1];
  game_id = data[2];
  mapper_version = data[3];
  rom_size = data[4];
  save_size = data[5];
  flags = data[6];
  RTC_present = data[7];
  checksum = (((unsigned short) data[8]) << 8) | data[9];
}

void ws_cartridge::game_metadata::write_to_data_array(unsigned char* data)
{
  data[0] = developer_id;
  data[1] = minimum_system;
  data[2] = game_id;
  data[3] = mapper_version;
  data[4] = rom_size;
  data[5] = save_size;
  data[6] = flags;
  data[7] = RTC_present;
  data[8] = checksum >> 8;
  data[9] = checksum & 0xFF;
}
