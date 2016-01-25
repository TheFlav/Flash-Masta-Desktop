#include "lm_cartridge_polling_worker.h"

#include "../flash_masta_app.h"
#include "linkmasta/device_manager.h"
#include "cartridge/ngp_cartridge.h"
#include "linkmasta/linkmasta_device.h"

const int LmCartridgePollingWorker::INTERVAL = 2000; // 1k milliseconds = 1 second

LmCartridgePollingWorker::LmCartridgePollingWorker(unsigned int id, QObject *parent) :
  QObject(parent),
  m_id(id), m_device_connected(false), m_running(false), m_timer(this)
{
  // Nothing else to do
}



void LmCartridgePollingWorker::start()
{
  // Configure the timer to trigger periodically
  m_timer.setInterval(INTERVAL);
  m_timer.setSingleShot(false);
  
  // Connect all our slots
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(run()));
  
  // Let's get this party started
  m_timer.start();
}

void LmCartridgePollingWorker::stop()
{
  m_timer.stop();
}

void LmCartridgePollingWorker::run()
{
  if (!FlashMastaApp::getInstance()->getDeviceManager()->try_claim_device(m_id))
  {
    // Do nothing
    return;
  }
  
  // This function simply tests if a cartridge was connected or disconnected
  linkmasta_device* linkmasta = FlashMastaApp::getInstance()->getDeviceManager()->get_linkmasta_device(m_id);
  bool device_connected;
  
  try
  {
    device_connected = linkmasta->test_for_cartridge();
  }
  catch (std::runtime_error& ex)
  {
    (void) ex;
    // Do nothing; fail quietly
    FlashMastaApp::getInstance()->getDeviceManager()->release_device(m_id);
    return;
  }
  
  // Must release device when done using it so as to not block other functions
  FlashMastaApp::getInstance()->getDeviceManager()->release_device(m_id);
  
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


