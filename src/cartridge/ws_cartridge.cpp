//
//  ws_cartridge.cpp
//  FlashMasta
//
//  Created by Dan on 8/17/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ws_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"
#include "ws_game_chip.h"
#include "tasks/task_controller.h"
#include "tasks/forwarding_task_controller.h"
#include <fstream>

#ifdef VERBOSE
#include <iostream>
#endif

using namespace std;

#define DEFAULT_BLOCK_SIZE 0x10000



ws_cartridge::ws_cartridge(linkmasta_device* linkmasta)
  : m_was_init(false), m_linkmasta(linkmasta), m_descriptor(nullptr),
    m_game_chip(nullptr)
{
  // Nothing else to do
}

ws_cartridge::~ws_cartridge()
{
  // Nothing else to do
}



system_type ws_cartridge::system() const
{
  return system_type::WONDERSWAN;
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
  
  m_linkmasta->init();
  m_linkmasta->open();
  build_cartridge_destriptor();
  m_linkmasta->close();
  
  m_was_init = true;
}

bool ws_cartridge::compare_file_to_cartridge(std::ifstream& fin, task_controller* controller)
{
  // Ensure class was initialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // determine the total number of bytes to compare
  fin.seekg(0, fin.end);
  unsigned int bytes_compared = 0;
  unsigned int bytes_total = (unsigned int) fin.tellg();
  fin.seekg(0, fin.beg);
  
  // Ensure file will fit
  if (bytes_total > descriptor()->num_bytes)
  {
    return false;
  }
  
  // Initialize markers
  unsigned int curr_chip = 0;
  unsigned int curr_block = 0;
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
      std::cout << "(chip " << curr_chip << ", block " << curr_block << ") " << bytes_compared << " B / " << bytes_total << " B (" << (bytes_compared * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = descriptor()->chips[curr_chip]->blocks[curr_block]->num_bytes;
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
        c_buffer_size = m_game_chip->read_bytes(
          descriptor()->chips[curr_chip]->blocks[curr_block]->base_address,
          c_buffer, bytes_expected
        );
      }
      else
      {
        // Use a forwarding controller to pass progress updates to our controller
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          c_buffer_size = m_game_chip->read_bytes(
            descriptor()->chips[curr_chip]->blocks[curr_block]->base_address,
            c_buffer, bytes_expected, &fwd_controller
          );
        }
        catch (std::exception& ex)
        {
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
      curr_block++;
      if (curr_block >= descriptor()->chips[curr_chip]->num_blocks)
      {
        curr_block = 0;
        curr_chip++;
      }
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    // Error occured! Clean up and pass error on to caller
    // Note: I appologize for the change in style: it's to save lines
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_game_chip->is_erasing());
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_game_chip->reset();
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      m_linkmasta->close();
    } catch (exception ex2) {
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

void ws_cartridge::write_file_to_cartridge(std::ifstream& fin, task_controller* controller)
{
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Determine the total number of bytes to write
  fin.seekg(0, fin.end);
  unsigned int bytes_written = 0;
  unsigned int bytes_total = (unsigned int) fin.tellg();
  fin.seekg(0, fin.beg);
  
  // Ensure file will fit
  if (bytes_total > descriptor()->num_bytes)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Initialize markers
  unsigned int curr_chip = 0;
  unsigned int curr_block = 0;
  
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
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = descriptor()->chips[curr_chip]->blocks[curr_block]->num_bytes;
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
      
      // Erase block from cartridge
      m_game_chip->erase_block(
        descriptor()->chips[curr_chip]->blocks[curr_block]->base_address
      );
      
      // Wait for erasure to complete
      while (m_game_chip->test_erasing());
      
      // Write buffer to cartridge
      if (controller == nullptr)
      {
        m_game_chip->program_bytes(
          descriptor()->chips[curr_chip]->blocks[curr_block]->base_address,
          buffer, buffer_size
        );
      }
      else
      {
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(buffer_size);
        try
        {
          m_game_chip->program_bytes(
            descriptor()->chips[curr_chip]->blocks[curr_block]->base_address,
            buffer, buffer_size, &fwd_controller
          );
        }
        catch (std::exception& ex)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      // Update markers
      bytes_written += buffer_size;
      curr_block++;
      if (curr_block >= descriptor()->chips[curr_chip]->num_blocks)
      {
        curr_block = 0;
        curr_chip++;
      }
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    // Error occured! Clean up and pass error on to caller
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_game_chip->is_erasing());
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_game_chip->reset();
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      m_linkmasta->close();
    } catch (exception ex2) {
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

void ws_cartridge::write_cartridge_to_file(std::ofstream& fout, task_controller* controller)
{
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Determine the total number of bytes to write
  unsigned int bytes_written = 0;
  unsigned int bytes_total = descriptor()->num_bytes;
  
  // Initialize markers
  unsigned int curr_chip = 0;
  unsigned int curr_block = 0;
  
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
      
      // Calcualte number of expected bytes
      unsigned int bytes_expected = descriptor()->chips[curr_chip]->blocks[curr_block]->num_bytes;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from cartridge
      if (controller == nullptr)
      {
        buffer_size = m_game_chip->read_bytes(
          descriptor()->chips[curr_chip]->blocks[curr_block]->base_address,
          buffer, bytes_expected
        );
      }
      else
      {
        // Create a forwarding controller to pass progress updates to
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          buffer_size = m_game_chip->read_bytes(
            descriptor()->chips[curr_chip]->blocks[curr_block]->base_address,
            buffer, bytes_expected, &fwd_controller
          );
        }
        catch (std::exception& ex)
        {
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
      curr_block++;
      if (curr_block >= descriptor()->chips[curr_chip]->num_blocks)
      {
        curr_block = 0;
        curr_chip++;
      }
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    // Error occured! Clean up and pass error on to caller
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_game_chip->is_erasing());
    } catch (std::exception& ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_game_chip->reset();
    } catch (std::exception& ex2) {
      // Well... this is awkward
    }
    
    try {
      m_linkmasta->close();
    } catch (std::exception& ex2) {
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



void ws_cartridge::build_cartridge_destriptor()
{
  if (m_descriptor != nullptr)
  {
    delete m_descriptor;
    m_descriptor = nullptr;
  }
  
  ws_game_chip* chip;
  
  chip = new ws_game_chip(m_linkmasta);
  
  // Check if chip exists or not
  if (chip->get_manufacturer_id() == 0x90 && chip->get_device_id() == 0x90)
  {
    delete chip;
    return;
  }
  
  m_game_chip = chip;
  
  // Initialize chip
  m_game_chip->test_bypass_support();
  
  // Initialize cartridge descriptor
  m_descriptor = new cartridge_descriptor(1);
  m_descriptor->type = system_type::WONDERSWAN;
  m_descriptor->num_bytes = 0;
  
  // Build chip
  build_chip_descriptor(0);
  m_descriptor->num_bytes += m_descriptor->chips[0]->num_bytes;
}

void ws_cartridge::build_chip_descriptor(unsigned int chip_i)
{
  ws_game_chip* chip = m_game_chip;
  cartridge_descriptor::chip_descriptor* chip_desc;
  unsigned int num_bytes;
  unsigned int num_blocks;
  
  ws_game_chip::manufact_id_t manufacturer = chip->get_manufacturer_id();
  ws_game_chip::device_id_t   device_id    = chip->get_device_id();
  ws_game_chip::device_id_t   size_id      = chip->get_size_id();
  
  // Confirm that chip exists
  if (manufacturer == 0x90 && device_id == 0x90)
  {
    // Stop everything and exit function
    return;
  }
  
  // Determine size of chip based on some size number
  switch (size_id)
  {
    case 0x2228:  // 1 Gib (2^30 bits) = 128 MiB (2^27 bytes)
      num_bytes = 0x8000000;
      break;
      
    case 0x2223:  // 512 Mib (2^29 bits) = 64 MiB (2^26 bytes)
      num_bytes = 0x4000000;
      break;
      
    case 0x2222:  // 256 Mib (2^28 bits) = 32 MiB (2^25 bytes)
      num_bytes = 0x2000000;
      break;
      
    case 0x2221:  // 128 MiB (2^27 bits) = 16 MiB (2^24 bytes)
      num_bytes = 0x1000000;
      break;
      
    default:    // Unknown chip? Too bad. No bytes 4 u
      num_bytes = 0x00;
      break;
  }
  
  // Calculate number of blocks. (1 block per 64 Kib (8 KiB))
  num_blocks = num_bytes / DEFAULT_BLOCK_SIZE;
  if (num_blocks > 0)
  {
    // Account for last block being split into 4
    num_blocks += 3;
  }
  
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
  ws_game_chip* chip = m_game_chip;
  
  // Initialize block descriptor
  cartridge_descriptor::chip_descriptor::block_descriptor* block;
  block = new cartridge_descriptor::chip_descriptor::block_descriptor();
  block->block_num = block_i;
  
  // Add (unfinished) block descriptor to chip descriptor
  m_descriptor->chips[chip_i]->blocks[block_i] = block;
  
  // Determine size of block based on index of block relative to total number
  // of blocks on chip
  switch (m_descriptor->chips[chip_i]->num_blocks - block_i)
  {
    case 1:    // Last block on chip
      block->num_bytes = DEFAULT_BLOCK_SIZE / 4;
      break;
      
    case 2:    // Second-last block on chip
    case 3:    // Third-last block on chip
      block->num_bytes = DEFAULT_BLOCK_SIZE / 8;
      break;
      
    case 4:    // Fourth-last block on chip
      block->num_bytes = DEFAULT_BLOCK_SIZE / 2;
      break;
      
    default:   // Some other block
      block->num_bytes = DEFAULT_BLOCK_SIZE;
      break;
  }
  
  // Determine base address of block based on index of block relative to total
  // number of blocks on chip
  block->base_address = 0;
  unsigned int num_basic_blocks = m_descriptor->chips[chip_i]->num_blocks - 4;
  switch (m_descriptor->chips[chip_i]->num_blocks - block_i)
  {
      // Note: Fall-throughs are intended
    case 1:    // Last block on chip
      block->base_address += DEFAULT_BLOCK_SIZE / 8;
      
    case 2:    // Second-last block on chip
      block->base_address += DEFAULT_BLOCK_SIZE / 8;
      
    case 3:    // Third-last block on chip
      block->base_address += DEFAULT_BLOCK_SIZE / 2;
      
    case 4:    // Fourth-last block on chip
    default:   // Some other block
      block->base_address += (block_i > num_basic_blocks ? num_basic_blocks : block_i) * DEFAULT_BLOCK_SIZE;
      break;
  }
  
  // Query chip for protection status of block
  block->is_protected = (chip->get_block_protection(block->base_address) == 0 ? false : true);
}


