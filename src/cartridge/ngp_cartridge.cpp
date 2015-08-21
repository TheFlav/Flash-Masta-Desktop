#include "ngp_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"
#include "ngp_chip.h"
#include "tasks/task_controller.h"
#include "tasks/forwarding_task_controller.h"
#include <iostream>

using namespace std;

#define DEFAULT_BLOCK_SIZE 0x10000
#define NGF_HEADER_VERSION 0x0053



struct NGFheader
{
  uint16_t version;
  uint16_t num_blocks;
  uint32_t num_bytes;
};

struct NGFblock
{
  uint32_t address;
  uint32_t num_bytes;
};



ngp_cartridge::ngp_cartridge(linkmasta_device* linkmasta)
  : m_was_init(false),
    m_linkmasta(linkmasta), m_descriptor(nullptr), m_num_chips(0)
{
  for (unsigned int i = 0; i < MAX_NUM_CHIPS; ++i)
  {
    m_chips[i] = nullptr;
  }
}

ngp_cartridge::~ngp_cartridge()
{
  if (m_descriptor != nullptr)
  {
    delete m_descriptor;
  }
  
  for (unsigned int i = 0; i < m_num_chips; ++i)
  {
    delete m_chips[i];
  }
}

// Note: documentation contained in super class cartridge
system_type ngp_cartridge::system() const
{
  return system_type::NEO_GEO_POCKET;
}

const cartridge_descriptor* ngp_cartridge::descriptor() const
{
  return m_descriptor;
}

void ngp_cartridge::init()
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



void ngp_cartridge::backup_cartridge_game_data(std::ostream& fout, task_controller* controller)
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
      
      // Convenience variables
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Calcualte number of expected bytes
      unsigned int bytes_expected = block->num_bytes;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from cartridge
      if (controller == nullptr)
      {
        buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, buffer, bytes_expected);
      }
      else
      {
        // Create a forwarding controller to pass progress updates to
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, buffer, bytes_expected, &fwd_controller);
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
      if (curr_block >= chip->num_blocks)
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
      while (m_chips[curr_chip]->is_erasing());
    } catch (std::exception& ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_chips[curr_chip]->reset();
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

void ngp_cartridge::restore_cartridge_game_data(std::istream& fin, task_controller* controller)
{
  // Ensure argument type is not the standard input
  if (&fin == &std::cin)
  {
    throw std::invalid_argument("Standard input cannot be used in cartridge operations");
  }
  
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
      
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = block->num_bytes;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) buffer, bytes_expected);
      buffer_size = (unsigned int) fin.gcount();
      
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
      m_chips[curr_chip]->erase_block(block->base_address);
      
      // Wait for erasure to complete
      while (m_chips[curr_chip]->test_erasing())
      {
        if (controller != nullptr)
        {
          controller->on_task_update(task_status::RUNNING,, 0);
        }
      }
      
      // Write buffer to cartridge
      if (controller == nullptr)
      {
        m_chips[curr_chip]->program_bytes(block->base_address, buffer, buffer_size);
      }
      else
      {
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(buffer_size);
        try
        {
          m_chips[curr_chip]->program_bytes(block->base_address, buffer, buffer_size, &fwd_controller);
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
      if (curr_block >= chip->num_blocks)
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
      while (m_chips[curr_chip]->is_erasing());
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_chips[curr_chip]->reset();
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

bool ngp_cartridge::compare_cartridge_game_data(std::istream& fin, task_controller* controller)
{
  // Ensure argument type is not the standard input
  if (&fin == &std::cin)
  {
    throw std::invalid_argument("Standard input cannot be used in cartridge operations");
  }
  
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
      
      // Convenience variables
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = block->num_bytes;
      if (bytes_expected > bytes_total - bytes_compared)
      {
        bytes_expected = bytes_total - bytes_compared;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) f_buffer, bytes_expected);
      f_buffer_size = (unsigned int) fin.gcount();
      
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
        c_buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, c_buffer, bytes_expected);
      }
      else
      {
        // Use a forwarding controller to pass progress updates to our controller
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          c_buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, c_buffer, bytes_expected, &fwd_controller);
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
      if (curr_block >= chip->num_blocks)
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
      while (m_chips[curr_chip]->is_erasing());
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_chips[curr_chip]->reset();
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

void ngp_cartridge::backup_cartridge_save_data(std::ostream& fout, task_controller* controller)
{
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Determine the total number of bytes and blocks to write
  unsigned int bytes_written = 0;
  unsigned int bytes_total = 0;
  unsigned int blocks_total = 0;
  for (unsigned int i = 0; i < descriptor()->num_chips; ++i)
  {
    for (unsigned int j = 0; j < descriptor()->chips[i]->num_blocks; ++j)
    {
      if (!descriptor()->chips[i]->blocks[j]->is_protected)
      {
        bytes_total += descriptor()->chips[i]->blocks[j]->num_bytes;
        blocks_total++;
      }
    }
  }
  
  // Create the file and block header structs and populate with data
  NGFheader file_header;
  NGFblock  block_header;
  file_header.version = NGF_HEADER_VERSION;
  file_header.num_bytes = sizeof(file_header);
  file_header.num_bytes += bytes_total;
  file_header.num_bytes += sizeof(block_header) * blocks_total;
  file_header.num_blocks = blocks_total;
  
  // Write header to file
  fout.write((char*) &file_header, sizeof(file_header));	
  
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
      
      // Convenience variables
      cartridge_descriptor::chip_descriptor* chip;
      cartridge_descriptor::chip_descriptor::block_descriptor* block;
      chip = descriptor()->chips[curr_chip];
      block = chip->blocks[curr_block];
      
      // Only write unprotected blocks
      if (!block->is_protected)
      {
        // Populate the header
        block_header.address = block->base_address;
        block_header.num_bytes = block->num_bytes;
        
        // Adjust for blocks not on the first chip
        for (unsigned int i = curr_chip; i > 0; --i)
        {
          block_header.address += descriptor()->chips[i]->num_blocks;
        }
        
        // Adjust for NGP virtual address offset
        block_header.address += 0x200000;
        
        // Write block header to file
        fout.write((char*) &block_header, sizeof(block_header));
        
        
        // Calcualte number of expected bytes
        unsigned int bytes_expected = block->num_bytes;
        if (bytes_expected > bytes_total - bytes_written)
        {
          bytes_expected = bytes_total - bytes_written;
        }
        
        // Attempt to read bytes from cartridge
        if (controller == nullptr)
        {
          buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, buffer, bytes_expected);
        }
        else
        {
          // Create a forwarding controller to pass progress updates to
          forwarding_task_controller fwd_controller(controller);
          fwd_controller.scale_work_to(bytes_expected);
          try
          {
            buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, buffer, bytes_expected, &fwd_controller);
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
        bytes_written += buffer_size;
      }
      
      // Update markers
      curr_block++;
      if (curr_block >= chip->num_blocks)
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
      while (m_chips[curr_chip]->is_erasing());
    } catch (std::exception& ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_chips[curr_chip]->reset();
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

void ngp_cartridge::restore_cartridge_save_data(std::istream& fin, task_controller* controller)
{
  // Ensure argument type is not the standard input
  if (&fin == &std::cin)
  {
    throw std::invalid_argument("Standard input cannot be used in cartridge operations");
  }
  
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Seek to start of file
  fin.seekg(0, fin.beg);
  
  // Initialize file header structs and read header from file
  NGFheader file_header;
  NGFblock  block_header;
  fin.read((char*) &file_header, sizeof(file_header));
  
  // Determine the number of bytes to read
  unsigned int bytes_written = 0;
  unsigned int bytes_total = file_header.num_bytes;
  bytes_total -= sizeof(file_header);
  bytes_total -= sizeof(block_header) * file_header.num_blocks;
  
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
    
    
    // Loop through all blocks in file
    for (unsigned int i = 0; i < file_header.num_blocks; ++i)
    {
      // Convenience functions
      cartridge_descriptor::chip_descriptor* chip = nullptr;
      cartridge_descriptor::chip_descriptor::block_descriptor* block = nullptr;
      
      // Read in the block header
      fin.read((char*) &block_header, sizeof(block_header));
      
      // Account for Neo Geo Pocket virtual address offset
      block_header.address -= 0x200000;
      
      // Determine the chip on which the block resides
      for (curr_chip = 0; curr_chip < descriptor()->num_chips; ++curr_chip)
      {
        chip = descriptor()->chips[curr_chip];
        if (block_header.address < chip->num_bytes)
        {
          break;
        }
        else
        {
          block_header.address -= chip->num_bytes;
        }
      }
      
      // Ensure integrity
      if (curr_chip >= descriptor()->num_chips)
      {
        throw std::runtime_error("Save file does not fit on this cartridge");
      }
      
      // Determine the block index on which the block resides
      for (curr_block = 0; curr_block < chip->num_blocks; ++curr_block)
      {
        block = chip->blocks[curr_block];
        if (block_header.address <= block->base_address)
        {
          break;
        }
      }
      
      // Ensure integrity
      if (curr_block >= chip->num_blocks
          || block_header.address != block->base_address
          || block_header.num_bytes != block->num_bytes)
      {
        throw std::runtime_error("Save file does not fit on this cartridge");
      }
      
      
      
      // With the destination block and chip found, transfer data from file
#ifdef VERBOSE
      std::cout << "(chip " << curr_chip << ", block " << curr_block << ") " << bytes_written << " B / " << bytes_total << " B (" << (bytes_written * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = block->num_bytes;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) buffer, bytes_expected);
      buffer_size = (unsigned int) fin.gcount();
      
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
      m_chips[curr_chip]->erase_block(block->base_address);
      
      // Wait for erasure to complete
      while (m_chips[curr_chip]->test_erasing())
      {
        if (controller != nullptr)
        {
          controller->on_task_update(task_status::RUNNING, 0);
        }
      }
      
      // Write buffer to cartridge
      if (controller == nullptr)
      {
        m_chips[curr_chip]->program_bytes(block->base_address, buffer, buffer_size);
      }
      else
      {
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(buffer_size);
        try
        {
          m_chips[curr_chip]->program_bytes(block->base_address, buffer, buffer_size, &fwd_controller);
        }
        catch (std::exception& ex)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
          throw;
        }
      }
      
      bytes_written += buffer_size;
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    // Error occured! Clean up and pass error on to caller
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_chips[curr_chip]->is_erasing());
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_chips[curr_chip]->reset();
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

bool ngp_cartridge::compare_cartridge_save_data(std::istream& fin, task_controller* controller)
{
  // Ensure argument type is not the standard input
  if (&fin == &std::cin)
  {
    throw std::invalid_argument("Standard input cannot be used in cartridge operations");
  }
  
  // Ensure class was intiialized
  if (!m_was_init)
  {
    throw std::runtime_error("ERROR"); // TODO
  }
  
  // Seek to start of file
  fin.seekg(0, fin.beg);
  
  // Initialize file header structs and read header from file
  NGFheader file_header;
  NGFblock  block_header;
  fin.read((char*) &file_header, sizeof(file_header));
  
  // Determine the number of bytes to read
  unsigned int bytes_written = 0;
  unsigned int bytes_total = file_header.num_bytes;
  bytes_total -= sizeof(file_header);
  bytes_total -= sizeof(block_header) * file_header.num_blocks;
  
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
  
  // Begin writing data block-by-block
  try
  {
    // Open connection to NGP chip
    m_linkmasta->open();
    
    
    // Loop through all blocks in file
    for (unsigned int i = 0; i < file_header.num_blocks && matched; ++i)
    {
      // Convenience functions
      cartridge_descriptor::chip_descriptor* chip = nullptr;
      cartridge_descriptor::chip_descriptor::block_descriptor* block = nullptr;
      
      // Read in the block header
      fin.read((char*) &block_header, sizeof(block_header));
      
      // Account for Neo Geo Pocket virtual address offset
      block_header.address -= 0x200000;
      
      // Determine the chip on which the block resides
      for (curr_chip = 0; curr_chip < descriptor()->num_chips; ++curr_chip)
      {
        chip = descriptor()->chips[curr_chip];
        if (block_header.address < chip->num_bytes)
        {
          break;
        }
        else
        {
          block_header.address -= chip->num_bytes;
        }
      }
      
      // Ensure integrity
      if (curr_chip >= descriptor()->num_chips)
      {
        throw std::runtime_error("Save file does not fit on this cartridge");
      }
      
      // Determine the block index on which the block resides
      for (curr_block = 0; curr_block < chip->num_blocks; ++curr_block)
      {
        block = chip->blocks[curr_block];
        if (block_header.address <= block->base_address)
        {
          break;
        }
      }
      
      // Ensure integrity
      if (curr_block >= chip->num_blocks
          || block_header.address != block->base_address
          || block_header.num_bytes != block->num_bytes)
      {
        throw std::runtime_error("Save file does not fit on this cartridge");
      }
      
      
      
      // With the destination block and chip found, transfer data from file
#ifdef VERBOSE
      std::cout << "(chip " << curr_chip << ", block " << curr_block << ") " << bytes_written << " B / " << bytes_total << " B (" << (bytes_written * 100 / bytes_total) << "%)" << endl;
#endif
      
      // Calcualte number of expected bytes
      unsigned bytes_expected = block->num_bytes;
      if (bytes_expected > bytes_total - bytes_written)
      {
        bytes_expected = bytes_total - bytes_written;
      }
      
      // Attempt to read bytes from file
      fin.read((char*) f_buffer, bytes_expected);
      f_buffer_size = (unsigned int) fin.gcount();
      
      // Check for errors
      if (f_buffer_size != bytes_expected)
      {
        if (controller != nullptr)
        {
          controller->on_task_end(task_status::ERROR, controller->get_task_work_progress());
        }
        throw std::runtime_error("ERROR");
      }
      if (!fin.good() && (f_buffer_size + bytes_written) < bytes_total)
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
        c_buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, c_buffer, bytes_expected);
      }
      else
      {
        // Use a forwarding controller to pass progress updates to our controller
        forwarding_task_controller fwd_controller(controller);
        fwd_controller.scale_work_to(bytes_expected);
        try
        {
          c_buffer_size = m_chips[curr_chip]->read_bytes(block->base_address, c_buffer, bytes_expected, &fwd_controller);
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
      
      bytes_written += f_buffer_size;
    }
    
    // Clean up before returning
    m_linkmasta->close();
  }
  catch (std::exception& ex)
  {
    // Error occured! Clean up and pass error on to caller
    try {
      // Spinlock while the chip finishes erasing (if it was erasing)
      while (m_chips[curr_chip]->is_erasing());
    } catch (exception ex2) {
      // Well... this is awkward
    }
    
    try {
      // Attempt to reset the chip
      m_chips[curr_chip]->reset();
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



void ngp_cartridge::build_cartridge_destriptor()
{
  if (m_descriptor != nullptr)
  {
    delete m_descriptor;
    m_descriptor = nullptr;
  }
  
  ngp_chip* chip;
  
  for (unsigned int i = 0; i < MAX_NUM_CHIPS; ++i)
  {
    chip = new ngp_chip(m_linkmasta, i);
    
    // Check if chip exists or not
    if (chip->get_manufacturer_id() == 0x90 && chip->get_device_id() == 0x90)
    {
      delete chip;
      m_num_chips = i;
      break;
    }
    
    m_chips[i] = chip;
    m_num_chips = i + 1;
    
    // Initialize chip
    m_chips[i]->test_bypass_support();
  }
  
  // Initialize cartridge descriptor
  m_descriptor = new cartridge_descriptor(m_num_chips);
  m_descriptor->type = NEO_GEO_POCKET;
  m_descriptor->num_bytes = 0;
  
  // Build chips
  for (unsigned int i = 0; i < m_num_chips; ++i)
  {
    build_chip_descriptor(i);
    m_descriptor->num_bytes += m_descriptor->chips[i]->num_bytes;
  }
}

void ngp_cartridge::build_chip_descriptor(unsigned int chip_i)
{
  ngp_chip* chip = m_chips[chip_i];
  cartridge_descriptor::chip_descriptor* chip_desc;
  unsigned int num_bytes;
  unsigned int num_blocks;
  
  ngp_chip::manufact_id_t manufacturer = chip->get_manufacturer_id();
  ngp_chip::device_id_t   device_id    = chip->get_device_id();
  
  // Confirm that chip exists
  if (manufacturer == 0x90 && device_id == 0x90)
  {
    // Stop everything and exit function
    return;
  }
  
  // Determine size of chip based on device id
  switch (device_id)
  {
  case 0x2F:  // 16 Mib (2^24 bits) = 2 MiB (2^21 bytes)
    num_bytes = 0x200000;
    break;
    
  case 0x2C:  // 8 Mib (2^23 bits) = 1 MiB (2^20 bytes)
    num_bytes = 0x100000;
    break;
    
  case 0xAB:  // 4 Mib (2^22 bits) = 0.5 MiB (2^19 bytes)
    num_bytes = 0x80000;
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

void ngp_cartridge::build_block_descriptor(unsigned int chip_i, unsigned int block_i)
{
  ngp_chip* chip = m_chips[chip_i];
  
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

bool ngp_cartridge::test_for_cartridge(linkmasta_device* linkmasta)
{
  bool exists;
  
  // Check the device id and manufacturer id and see if they are invalid
  linkmasta->open();
  ngp_chip chip(linkmasta, 0);
  chip.reset();
  exists = !(chip.get_device_id() == 0x90 || chip.get_manufacturer_id() == 0x90);
  linkmasta->close();
  
  return exists;
}

