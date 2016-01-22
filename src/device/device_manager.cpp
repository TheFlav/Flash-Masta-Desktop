#include "device_manager.h"

#include <chrono>

#include "common/log.h"
#include "linkmasta/ngp_linkmasta_device.h"
#include "linkmasta/ws_linkmasta_device.h"
#include "usb/usb_device.h"

using namespace std;
using namespace usb;



DeviceManager::DeviceManager()
  : m_thread_kill_flag(false), m_thread_dead(true), curr_id(0)
{
  // Nothing else to do
}

DeviceManager::~DeviceManager()
{
  log_start(log_level::DEBUG, "~DeviceManager() {");
  stopAutoRefreshAndWait();
  log_end("}");
}



unsigned int DeviceManager::generateId()
{
  return curr_id++;
}

void DeviceManager::startAutoRefresh()
{
  if (m_thread_dead)
  {
    m_thread_dead = false;
    m_thread_kill_flag = false;
    m_refresh_thread = thread(&DeviceManager::refreshThreadFunction, this);
    m_refresh_thread.detach();
  }
}

void DeviceManager::stopAutoRefreshAndWait()
{
  log_start(log_level::DEBUG, "DeviceManager::stopAutoRefreshAndWait() {");
  
  m_thread_kill_flag = true;
  if (m_refresh_thread.joinable())
  {
    log(log_level::DEBUG, "waiting to join refresh_thread");
    m_refresh_thread.join();
    log_cont("refresh_thread joined");
  }
  else
  {
    log(log_level::DEBUG, "refresh_thread already joined");
  }
  
  log_end("}");
}

linkmasta_device* DeviceManager::buildLinkmastaDevice(usb::usb_device* device)
{
  device->init();
  
  int vendor_id = device->get_device_description()->vendor_id;
  int product_id = device->get_device_description()->product_id;
  
  if ((vendor_id == 0x20A0 && product_id == 0x4178)
      || (vendor_id == 0x20A0 && product_id == 0x4256))
  {
    auto r = new ngp_linkmasta_device(device);
    r->init();
    return r;
  }
  else if (vendor_id == 0x20A0 && product_id == 0x4252)
  {
    auto r = new ws_linkmasta_device(device);
    r->init();
    return r;
  }
  else
  {
    return nullptr;
  }
}



void DeviceManager::refreshThreadFunction()
{
  // Set target time to refresh
  auto target_time = chrono::steady_clock::now();
  target_time += chrono::seconds(1);
  
  // Loop as long as object exists
  while (!m_thread_kill_flag)
  {
    // Only execute if we've reached our target time
    if (chrono::steady_clock::now() < target_time)
    {
      continue;
    }
    
    // Make call to child
    refreshDeviceList();
    
    // Update refresh timer
    target_time = target_time = chrono::steady_clock::now();
    target_time += chrono::seconds(1);
  }
  
  m_thread_dead = true;
}


