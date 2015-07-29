#include "ngp_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"

ngp_cartridge::ngp_cartridge(linkmasta_device* linkmasta)
  : m_was_init(false),
    m_linkmasta(linkmasta), m_descriptor(nullptr)
{
  // Nothing else to do
}

ngp_cartridge::~ngp_cartridge()
{
  if (m_descriptor != nullptr)
  {
    delete m_descriptor;
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
  build_destriptor();
}

/*
void ngp_cartridge::restore_cartridge_from_file(std::ifstream& fin);
void ngp_cartridge::backup_cartridge_to_file(std::ofstream& fout);
*/

void ngp_cartridge::build_destriptor()
{
  
}
