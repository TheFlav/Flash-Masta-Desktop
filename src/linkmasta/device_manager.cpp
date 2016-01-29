/*! \file
 *  \brief File containing the implementation of \ref device_manager.
 *  
 *  File containing a partial implementation of \ref device_manager.
 *  Pure virtual functions must be implemented by subclasses.
 *  
 *  See corrensponding header file to view documentation for struct, its
 *  methods, and its member variables.
 *  
 *  \author Daniel Andrus
 *  \date 2015-09-08
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "device_manager.h"

#include <chrono>

#include "common/log.h"
#include "usb/usb_device.h"
#include "ngp_linkmasta_device.h"
#include "ws_linkmasta_device.h"

using namespace std;
using namespace usb;



device_manager::device_manager()
  : m_thread_kill_flag(false), m_thread_dead(true), curr_id(0)
{
  // Nothing else to do
}

device_manager::~device_manager()
{
  log_start(log_level::DEBUG, "~DeviceManager() {");
  stop_auto_refresh_and_wait();
  log_end("}");
}



unsigned int device_manager::generate_id()
{
  return curr_id++;
}

void device_manager::start_auto_refresh()
{
  if (m_thread_dead)
  {
    m_thread_dead = false;
    m_thread_kill_flag = false;
    m_refresh_thread = thread(&device_manager::refresh_thread_function, this);
    m_refresh_thread.detach();
  }
}

void device_manager::stop_auto_refresh_and_wait()
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

linkmasta_device* device_manager::build_linkmasta_device(usb::usb_device* device)
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



void device_manager::refresh_thread_function()
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
    refresh_device_list();
    
    // Update refresh timer
    target_time = target_time = chrono::steady_clock::now();
    target_time += chrono::seconds(1);
  }
  
  m_thread_dead = true;
}
