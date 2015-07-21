#ifndef __NGP_CARTRIDGE_H__
#define __NGP_CARTRIDGE_H__

#include "mc_cartridge.h"
#include <vector>

/**
 * Class representing a Neo Geo Pocket game cartridge. Stores information
 * relevant to cartridge storage and hardware information, such as the number
 * of onboard chips and exactly how the chips are divided into blocks.
 */
class ngp_cartridge: public mc_cartridge
{
public:
  
  /**
   * Class representing a chip on a Neo Geo Pocket cartridge. Stores information
   * relevant to storage chips, such as the number of bytes on the chip and
   * information on the memory blocks the chip is divided into.
   */
  class chip: public mc_cartridge::chip
  {
  public:
    typedef unsigned int block_index;  //<! Data type used to represent the index of a block of memory
    
    /**
     * Class representing a block of memory on a Neo Geo Pocket chip. Contains
     * methods to get the number of bytes in the block, the base address of the
     * block, and whether or not the block is marked as being protected.
     */
    class block
    {
    public:
      /**
       * Designated constructor for the block class. Initializes properties
       * to the default value of each type.
       */
      block();
      
      /**
       * Copy constructor for the block class. Performs a deep copy of the
       * properties of the other supplied block. Does not modify the data of
       * the original object.
       *
       * @param[in] other - The original block object to create a copy of.
       */
      block(const block& other);
      
      /**
       * Gets the size of the memory block in bytes.
       *
       * @return The number of bytes in a block.
       */
      unsigned int size() const;
      
      /**
       * Gets the value of the starting address of the block.
       *
       * @return The starting address of the block.
       */
      address_t     base_address() const;
      
      /**
       * Gets whether or not the block is declared as "protected" by the Neo
       * Geo catridge. This does not necessarily an accurate indicator as to
       * whether or not the block can be written to.
       *
       * @return True if the block is protected, false if not.
       */
      bool         is_protected() const;
      
      unsigned int m_size;          //<! The number of bytes
      address_t    m_base_address;  //<! The starting memory address
      bool         m_is_protected;  //<! Whether or not the block is protected
    };
    
    
    
    /**
     * Designated constructor for the gnp_cartridge::chip class. Initializes
     * properties to default values of each type.
     */
    chip();
    
    /**
     * Copy constructor for the gnp_cartridge::chip class. Initializes with
     * information copied from another object. Does not modify other object.
     *
     * @param[in] other - Original chip instance to create a copy of.
     */
    chip(const chip& other);
    
    /**
     * Gets the id of the manufacturer of the chip.
     * 
     * @return The id of the manufacturer of the chip.
     */
    unsigned int manufacturer_id() const;
    
    /**
     * Gets the device id of the chip itself.
     * @return The device id of the chip itself.
     */
    unsigned int device_id() const;
    
    /**
     * Gets the number of blocks making up the chip.
     *
     * @return The number of blocks the chip is divided into.
     */
    unsigned int num_blocks() const;
    
    /**
     * Gets the information of the block with the given index. Does not provide
     * checks against invalid block indexes, so programmers must take care to
     * validate parameters before calling this function.
     *
     * @param[in] block - The index of the block to get.
     *
     * @return Reference to the block object found at the given index. This
     * object cannot be manipulated or modified.
     */
    const block& get_block(block_index block) const;
    
    unsigned int       m_manufacturer_id;  //<! Manufacturer's id
    unsigned int       m_device_id;        //<! Device's id
    unsigned int       m_num_blocks;       //<! The number of blocks the chip is divided into
    std::vector<block> m_blocks;           //<! List of block objects containing information on each individual block
  };
  
  
  
  /**
   * Designated constructor for the ngp_cartridge. Initializes properties
   * with default values of each type.
   */
  ngp_cartridge();
  
  /**
   * Copy constructor for the ngp_cargridge class. Initializes with information
   * copied from another instance. Does not modify the original object.
   *
   * @param[in] other - Original instance to copy data from.
   */
  ngp_cartridge(const ngp_cartridge& other);
  
  // Note: documentation contained in super class mc_cartridge
  const chip* get_chip(chip_index chip) const;
  
  // Note: documentation contained in super class cartridge
  system_type system() const;
  
  std::vector<chip> m_chips;   //<! List of chips in cartridge.
};

#endif // __NGP_CARTRIDGE_H__
