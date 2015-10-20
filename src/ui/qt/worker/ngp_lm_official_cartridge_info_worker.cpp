#include "ngp_lm_official_cartridge_info_worker.h"

#include "../flash_masta.h"
#include "../device_manager.h"
#include "cartridge/ngp_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"

NgpLmOfficialCartridgeInfoWorker::NgpLmOfficialCartridgeInfoWorker(unsigned int device_id, QObject *parent) :
  QObject(parent), m_device_id(device_id), m_cancelled(false)
{
  
}



void NgpLmOfficialCartridgeInfoWorker::run()
{
  bool cancel = false;
  ngp_cartridge* cart = nullptr;
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(m_device_id));
  
  m_mutex.lock();
  if (m_cancelled) cancel = true;
  m_mutex.unlock();
  
  linkmasta_device* linkmasta;
  
  if (!cancel)
  {
    linkmasta = FlashMasta::get_instance()->get_device_manager()->get_linkmasta_device(m_device_id);
    m_mutex.lock();
    if (m_cancelled) cancel = true;
    m_mutex.unlock();
  }
  
  if (!cancel)
  {
    cart = new ngp_cartridge(linkmasta);
    m_mutex.lock();
    if (m_cancelled) cancel = true;
    m_mutex.unlock();
  }
  
  if (!cancel)
  {
    cart->init();
    m_mutex.lock();
    if (m_cancelled) cancel = true;
    m_mutex.unlock();
  }
  
  FlashMasta::get_instance()->get_device_manager()->release_device(m_device_id);
  m_mutex.lock();
  if (m_cancelled) cancel = true;
  m_mutex.unlock();
  
  if (cancel && cart != nullptr)
  {
    delete cart;
    cart = nullptr;
  }
  
  emit finished(cart);
}

void NgpLmOfficialCartridgeInfoWorker::cancel()
{
  m_mutex.lock();
  m_cancelled = true;
  m_mutex.unlock();
}
