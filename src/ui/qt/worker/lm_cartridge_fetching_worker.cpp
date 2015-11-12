#include "lm_cartridge_fetching_worker.h"

#include "../flash_masta_app.h"
#include "../device_manager.h"
#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"

LmCartridgeFetchingWorker::LmCartridgeFetchingWorker(unsigned int device_id, QObject *parent) :
  QObject(parent), m_device_id(device_id), m_cancelled(false)
{
  // Nothing else to do
}



void LmCartridgeFetchingWorker::run()
{
  bool cancel = false;
  cartridge* cart = nullptr;
  QString game_name = "";
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(m_device_id));
  
  m_mutex.lock();
  if (m_cancelled) cancel = true;
  m_mutex.unlock();
  
  linkmasta_device* linkmasta;
  
  if (!cancel)
  {
    linkmasta = FlashMastaApp::getInstance()->getDeviceManager()->getLinkmastaDevice(m_device_id);
    m_mutex.lock();
    if (m_cancelled) cancel = true;
    m_mutex.unlock();
  }
  
  if (!cancel)
  {
    cart = linkmasta->build_cartridge();
    m_mutex.lock();
    if (m_cancelled) cancel = true;
    m_mutex.unlock();
  }
  
  if (!cancel)
  {
    game_name = cart->fetch_game_name(0).c_str();
    m_mutex.lock();
    if (m_cancelled) cancel = true;
    m_mutex.unlock();
  }
  
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(m_device_id);
  m_mutex.lock();
  if (m_cancelled) cancel = true;
  m_mutex.unlock();
  
  if (cancel && cart != nullptr)
  {
    delete cart;
    cart = nullptr;
  }
  
  emit finished(cart, game_name);
}

void LmCartridgeFetchingWorker::cancel()
{
  m_mutex.lock();
  m_cancelled = true;
  m_mutex.unlock();
}
