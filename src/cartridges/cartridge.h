#ifndef __FLASHMASTA_CARTRIDGES_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_CARTRIDGE_H__

/**
 * Abstract class representing a generic game cartridge for an unknown game
 * system. Contains information relevant to storage capacticy and hardware.
 */
class cartridge
{
public:
  typedef unsigned int address;   //<! Data type representing a storage address
  
  /**
   * Enumeration representing a system for which the game cartridge belongs.
   * Can be used to change how a segment of code processes a game cartridge.
   */
  enum system_type
  {
    UNKNOWN,          //<! Unknown cartridge type
    NEO_GEO_POCKET    //<! Cartridge made for the Neo Geo Pocket
  };
  
  /**
   * Designated constructor for the cartridge class. Initializes with cartridge
   * storage capacity and base address.
   *
   * @param[in] size - The storage size of the cartridge in bytes.
   * @param[in] base_address - The base memory address of the cartridge. Used in
   *   memory address offsets.
   */
  cartridge(unsigned int size, address base_address);
  
  /**
   * Gets the storage capacity of the cartridge in bytes.
   *
   * @return The storage capacity of the cartridge in bytes.
   */
  unsigned int size() const;
  
  /**
   * Gets the starting address of the cartridge.
   *
   * @return The starting address of the cartridge.
   */
  address base_address() const;
  
  /**
   * Gets game system for which the game cartridge is intended. This value can
   * be used to determine the subclass of cartridge and how to process it.
   *
   * @return Value of enumeration system_type denoting the game system for which
   *   the cartridge was designed.
   */
  virtual system_type system() const = 0;
  
protected:
  const unsigned int m_size;    //<! The storage capacity of the cartridge
  const address m_base_address; //<! The starting address of the cartridge
};

#endif
