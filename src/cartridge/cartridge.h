#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "../common/types.h"

/**
 * Abstract class representing a generic game cartridge for an unknown game
 * system. Contains information relevant to storage capacticy and hardware.
 */
class cartridge
{
public:
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
   * Designated constructor for the cartridge class. Initializes properties to the
   * default value for each type.
   */
  cartridge();
  
  /**
   * Copy constructor for the cartridge class. Initializes properties to corresponding
   * values of other object. Does nto modify the other object.
   * 
   * @param other - The cartridge to copy.
   */
  cartridge(const cartridge& other);
  
  /**
   * Destructor for the cartridge class. Does nothing except exist for extendibility.
   */
  virtual ~cartridge();
  
  /**
   * Gets the storage capacity of the cartridge in bytes.
   *
   * @return The storage capacity of the cartridge in bytes.
   */
  unsigned int size() const;
  
  /**
   * Gets the starting address_t of the cartridge.
   *
   * @return The starting address_t of the cartridge.
   */
  address_t base_address() const;
  
  /**
   * Gets game system for which the game cartridge is intended. This value can
   * be used to determine the subclass of cartridge and how to process it.
   *
   * @return Value of enumeration system_type denoting the game system for which
   *   the cartridge was designed.
   */
  virtual system_type system() const = 0;
  
  unsigned int m_size;    //<! The storage capacity of the cartridge
  address_t m_base_address; //<! The starting address_t of the cartridge
};

#endif // __CARTRIDGE_H__
