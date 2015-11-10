#include "ngp_lm_cartridge_polling_worker.h"

#include "../flash_masta_app.h"
#include "../device_manager.h"
#include "cartridge/ngp_cartridge.h"

const int NgpLmCartridgePollingWorker::INTERVAL = 2000; // 1k milliseconds = 1 second

NgpLmCartridgePollingWorker::NgpLmCartridgePollingWorker(unsigned int id, QObject *parent) :
  QObject(parent),
  m_id(id), m_device_connected(false), m_running(false), m_timer(this)
{
  // Nothing else to do
}



void NgpLmCartridgePollingWorker::start()
{
  // Configure the timer to trigger periodically
  m_timer.setInterval(INTERVAL);
  m_timer.setSingleShot(false);
  
  // Connect all our slots
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(run()));
  
  // Let's get this party started
  m_timer.start();
}

void NgpLmCartridgePollingWorker::stop()
{
  m_timer.stop();
}

void NgpLmCartridgePollingWorker::run()
{
  if (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(m_id))
  {
    // Do nothing
    return;
  }
  
  // This function simply tests if a cartridge was connected or disconnected
  linkmasta_device* linkmasta = FlashMastaApp::getInstance()->getDeviceManager()->getLinkmastaDevice(m_id);
  bool device_connected;
  
  try
  {
    device_connected = ngp_cartridge::test_for_cartridge(linkmasta);
  }
  catch (std::runtime_error& e)
  {
    // Do nothing; fail quietly
    FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(m_id);
    return;
  }
  
  // Must release device when done using it so as to not block other functions
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(m_id);
  
  if (m_device_connected == device_connected)
  {
    // No change; do nothing
    return;
  }
  
  m_device_connected = device_connected;
  
  if (m_device_connected)
  {
    emit cartridgeInserted();
  }
  else
  {
    emit cartridgeRemoved();
  }
}


