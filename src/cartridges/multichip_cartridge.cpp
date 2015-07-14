#include "multichip_cartridge.h"

typedef cartridge::address address;
typedef multichip_cartridge::chip_index chip_index;

address __multichip_cartridge_sum_array(chip_index num_chips, const address* size_chips)
{
  address base = 0;
  for (chip_index i = 0; i < num_chips; ++i)
  {
    base += size_chips[i];
  }
  return base;
}

multichip_cartridge::multichip_cartridge(chip_index num_chips, const address* size_chips)
  : cartridge(__multichip_cartridge_sum_array(num_chips, size_chips)), size_chips(num_chips)
{
  // Allocate arrays for data
  size_bytes_chips = new address[num_chips];
  base_address_chips = new address[num_chips];
  
  chip_index i = 0;
  address base = 0;
  
  for (; i < num_chips; ++i)
  {
    base_address_chips[i] = base;
    size_bytes_chips[i] = size_chips[i];
    base += size_chips[i];
  }
}

multichip_cartridge::~multichip_cartridge()
{
  delete[] size_bytes_chips;
  delete[] base_address_chips;
}

inline chip_index multichip_cartridge::num_chips() const
{
  return size_chips;
}

inline address multichip_cartridge::size_chip(chip_index chip) const
{
  return size_bytes_chips[chip];
}

inline address multichip_cartridge::base_address_chip(chip_index chip) const
{
  return base_address_chips[chip];
}
