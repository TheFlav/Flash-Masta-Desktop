#ifndef __FLASHMASTA_CARTRIDGES_NGP_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_NGP_CARTRIDGE_H__

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
       * Designated constructor for the block class. Initializes with
       * information about the size of the block (in bytes), the base address of
       * the block, and whether or not the block is protected from write
       * acceess. Note that if a block is reported as being protected, that does
       * not necessarily mean that it is impossible to write to that block.
       *
       * @param[in] size - The number of bytes in this block of memory.
       * @param[in] base_address - The base address of the block. Can be used
       *   for address offsets.
       * @param[in] is_protected - Boolean indicating whether the memory block
       *   is protected, true for protected, false for not. This property
       *   does not strictly mean read-only, since it is possible for the
       *   cartridge to report that a block is protected when in fact allows
       *   writes to that block.
       */
      block(unsigned int size, address base_address, bool is_protected);
      
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
      address      base_address() const;
      
      /**
       * Gets whether or not the block is declared as "protected" by the Neo
       * Geo catridge. This does not necessarily an accurate indicator as to
       * whether or not the block can be written to.
       *
       * @return True if the block is protected, false if not.
       */
      bool         is_protected() const;
      
    protected:
      const unsigned int m_size;          //<! The number of bytes
      const address      m_base_address;  //<! The starting memory address
      const bool         m_is_protected;  //<! Whether or not the block is protected
    };
    
    /**
     * Designated constructor for the gnp_cartridge::chip class. Initializes
     * with information about the size, base address, and the memory blocks of
     * the chip.
     *
     * @param[in] size - The size of the chip in bytes, taking all blocks into
     *   account.
     * @param[in] base_address - The starting address of the chip.
     * @param[in] num_blocks - The number of blocks the chip is divided into.
     * @param[in] blocks - Vector of blocks composing the chip.
     */
    chip(unsigned int size, address base_address, unsigned int num_blocks, const std::vector<block>& blocks);
    
    /**
     * Copy constructor for the gnp_cartridge::chip class. Initializes with
     * information copied from another object. Does not modify other object.
     *
     * @param[in] other - Original chip instance to create a copy of.
     */
    chip(const chip& other);
    
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
    
  protected:
    const unsigned int       m_num_blocks; //<! The number of blocks the chip is divided into
    const std::vector<block> m_blocks;     //<! List of block objects containing information on each individual block
  };
  
  /**
   * Designated constructor for the ngp_cartridge. Initializes with information
   * on cartridge capacity, the base address, the number of on-board storage
   * chips, and a list actual chip objects containing chip information.
   *
   * @param[in] size - The capacity of the cartridge in bytes.
   * @param[in] address - The starting address of the cartridge, used for any
   *   memory access offsets. Will typically be 0.
   * @param[in] num_chips - The number of chips the cartridge contains. Usually
   *   between 1 and 2.
   * @param[in] chips - List containing initialized chip objects representing
   *   the cartridge's on-board chips and containing information on each.
   */
  ngp_cartridge(unsigned int size, address base_address, unsigned int num_chips, const std::vector<chip>& chips);
  
  /**
   * Copy constructor for the ngp_cargridge class. Initializes with information
   * copied from another instance. Does not modify the original object.
   *
   * @param[in] other - Original instance to copy data from.
   */
  ngp_cartridge(const ngp_cartridge& other);
  
  // Note: documentation contained in super class mc_cartridge
  const chip& get_chip(chip_index chip) const;
  
  // Note: documentation contained in super class cartridge
  system_type system() const;
  
private:
  const std::vector<chip> m_chips;   //<! List of chips in cartridge.
};

#endif
