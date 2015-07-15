#ifndef __FLASHMASTA_CARTRIDGES_MC_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_MC_CARTRIDGE_H__

#include "cartridge.h"

/**
 * Abstract class representing a mult-chip cartridge. Contains information
 * related to the hardware of cartridges with multiple storage chips.
 */
class mc_cartridge: public cartridge
{
public:
  typedef unsigned int chip_index;  //<! Data type used to represent the index of a storage chip.
  
  /**
   * Class representing a storage chip on a multi-chip cartridge.
   */
  class chip
  {
  public:
    
    /**
     * Designated constructor for the mc_cartridge::chip class. Initializes
     * information with the storage capacity of the chip and the base address.
     *
     * @param[in] size - The storage size in bytes of the chip.
     * @param[in] base_address - The starting address of the chip.
     */
    chip(unsigned int size, address base_address);
    
    /**
     * Copy constructor for the mc_cartridge::chip class. Initializes
     * information with data from another instance of the class. Does not
     * modify original object.
     *
     * @param[in] other - Another insance to copy data from.
     */
    chip(const chip& other);
    
    /**
     * Gets the size of the storage chip in bytes.
     *
     * @return The size of the storage chip in bytes.
     */
    unsigned int size() const;
    
    /**
     * Gets the starting address of the storage chip.
     *
     * @return The starting address of the storage chip.
     */
    address base_address() const;
    
  protected:
    const unsigned int m_size;     //<! The storage capacity in bytes
    const address m_base_address;  //<! The starting address
  };
  
  /**
   * The designated constructor for the mc_cartridge class. Initializes
   * information with the storage capacity, the base address, and number of
   * storage chips on the cartridge.
   *
   * @param[in] size - The storage capacity of the cartridge in bytes.
   * @param[in] base_address - The starting address of the cartridge.
   * @param[in] num_chips - The number of storage chips in the cartridge.
   */
  mc_cartridge(unsigned int size, address base_address, unsigned int num_chips);
  
  /**
   * Gets the number of storage chips in the cartridge.
   *
   * @return The nuber of storage chips in the cartridge.
   */
  unsigned int num_chips() const;
  
  /**
   * Gets information for the chip at the given index in the form of a chip
   * object that can supply further chip-specific information. Does not perform
   * input validation or check for valid input, so it is the responsibility of
   * the programmer to provide a valid index. Any errors or exceptions raised
   * will be passed back.
   *
   * @param[in] chip - The index of the chip to get.
   *
   * @return Reference to the chip object containing information pertaining to
   * the chip with the request index. This is bassed-by-reference and cannot be
   * modified or manipulated.
   */
  virtual const chip& get_chip(chip_index chip) const = 0;
  
protected:
  const unsigned int m_num_chips;   //<! The number of onboard storage chips
};

#endif
