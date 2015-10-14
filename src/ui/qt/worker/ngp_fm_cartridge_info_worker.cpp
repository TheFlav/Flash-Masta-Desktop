//
//  ngp_fm_cartridge_info_worker.cpp
//  FlashMasta
//
//  Created by Dan on 10/14/15.
//  Copyright © 2015 7400 Circuits. All rights reserved.
//

#include "ngp_fm_cartridge_info_worker.h"
#include "../flash_masta.h"
#include "../device_manager.h"

NgpFmCartridgeInfoWorker:NgpFmCartridgeInfoWorker(unsigned int device_id, QObject *parent)
  : QObject(parent), m_id(device_id), m_cancelled(false)
{
  // Nothing else to do
}

NgpFmCartridgeInfoWorker:~NgpFmCartridgeInfoWorker()
{
  cancel();
}

void NgpFmCartridgeInfoWorker:run()
{
  ngp_cartridge* cartridge = nullptr;
  
  m_mutex.lock();
  if (!m_cancelled)
  {
    DeviceManager* device_manager = FlashMasta::get_instance()->get_device_manager();
    if (device_manager->is_connected(m_id))
    {
      while (!device_manager->claim_device(m_id));
      
      cartridge = new ngp_cartridge(device_manager->get_linkmasta_device(m_id));
      cartridge->init();
      
      device_manager->release_device(m_id);
    }
  }
  m_mutex.unlock();
  
  emit finished(cartridge);
}

void NgpFmCartridgeInfoWorker:cancel()
{
  m_mutex.lock();
  m_cancelled = true;
  m_mutex.unlock();
}
