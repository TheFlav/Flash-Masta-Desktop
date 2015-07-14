#ifndef __FLASHMASTA_CARTRIDGES_CARTRIDGE_H__
#define __FLASHMASTA_CARTRIDGES_CARTRIDGE_H__

class cartridge
{
public:
  typedef unsigned int address;
  
  enum system_type
  {
    UNKNOWN,
    NEO_GEO
  };
  
  cartridge(unsigned int size, address base_address);
  
  unsigned int size() const;
  address base_address() const;
  virtual system_type system() const = 0;
  
protected:
  const unsigned int m_size;
  const address m_base_address;
};

#endif
