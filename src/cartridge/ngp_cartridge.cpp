#include "ngp_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"
#include "ngp_chip.h"



#define DEFAULT_BLOCK_SIZE 0x2000



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
  
  m_was_init = true;
  
  m_linkmasta->init();
  m_linkmasta->open();
  build_cartridge_destriptor();
  m_linkmasta->close();
}

/*
void ngp_cartridge::restore_cartridge_from_file(std::ifstream& fin);
void ngp_cartridge::backup_cartridge_to_file(std::ofstream& fout);
*/

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
    chip->test_bypass_support();
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
    num_bytes = 0x1000000;
    break;
    
  case 0x2C:  // 8 Mib (2^23 bits) = 1 MiB (2^20 bytes)
    num_bytes = 0x800000;
    break;
    
  case 0xAB:  // 4 Mib (2^22 bits) = 0.5 MiB (2^19 bytes)
    num_bytes = 0x400000;
    break;
    
  default:    // unknown chip? too bad.
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


