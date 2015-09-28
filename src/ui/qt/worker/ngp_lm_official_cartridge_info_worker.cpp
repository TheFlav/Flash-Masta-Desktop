#include "ngp_lm_official_cartridge_info_worker.h"

#include "../flash_masta.h"
#include "../device_manager.h"
#include "cartridge/ngp_cartridge.h"

NgpLmOfficialCartridgeInfoWorker::NgpLmOfficialCartridgeInfoWorker(unsigned int device_id, QObject *parent) :
  QObject(parent), m_device_id(device_id)
{
  
}



void NgpLmOfficialCartridgeInfoWorker::run()
{
  ngp_cartridge* cart;
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(m_device_id));
  
  auto linkmasta = FlashMasta::get_instance()->get_device_manager()->get_linkmasta_device(m_device_id);
  cart = new ngp_cartridge(linkmasta);
  cart->init();
  
  FlashMasta::get_instance()->get_device_manager()->release_device(m_device_id);
  emit finished(cart);
}


