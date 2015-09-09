#include "libusb_device_manager.h"
#include <string>
#include "libusb-1.0/libusb.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/linkmasta_device.h"

using namespace std;



LibusbDeviceManager::LibusbDeviceManager()
  : DeviceManager()
{
  libusb_init(&m_libusb);
}

LibusbDeviceManager::~LibusbDeviceManager()
{
  for (auto entry : m_connected_devices)
  {
    delete entry.second.linkmasta;
    libusb_unref_device(entry.second.device);
  }
  
  libusb_exit(m_libusb);
}



std::vector<unsigned int> LibusbDeviceManager::get_connected_devices()
{
  vector<unsigned int> list;
  
  m_connected_devices_mutex.lock();
  list.reserve(m_connected_devices.size());
  
  for (auto entry : m_connected_devices)
  {
    list.push_back(entry.first);
  }
  m_connected_devices_mutex.unlock();
  
  return list;
}

bool LibusbDeviceManager::try_get_connected_devices(std::vector<unsigned int>& devices)
{
  // Only continue if background process isn't running
  if (m_connected_devices_mutex.try_lock())
  {
    devices.clear();
    devices.reserve(m_connected_devices.size());
    
    for (auto entry : m_connected_devices)
    {
      devices.push_back(entry.first);
    }
    
    m_connected_devices_mutex.unlock();
    return true;
  }
  else
  {
    return false;
  }
}

bool LibusbDeviceManager::is_connected(unsigned int id)
{
  m_connected_devices_mutex.lock(); // LOCK m_connected_devices
  
  bool r = (m_connected_devices.find(id) != m_connected_devices.end());
  
  m_connected_devices_mutex.unlock(); // UNLOCK m_connected_devices
  
  return r;
}

unsigned int LibusbDeviceManager::get_vendor_id(unsigned int id)
{
  m_connected_devices_mutex.lock(); // LOCK m_connected_devices
  
  auto it = m_connected_devices.find(id);
  
  if (it == m_connected_devices.end())
  {
    m_connected_devices_mutex.unlock(); // UNLOCK m_connected_devices
    throw std::invalid_argument("Unknown connected device ID " + std::to_string(id));
  }
  
  id = it->second.vendor_id;
  
  m_connected_devices_mutex.unlock(); // UNLOCK m_connected_devices
  
  return id;
}

unsigned int LibusbDeviceManager::get_product_id(unsigned int id)
{
  m_connected_devices_mutex.lock(); // LOCK m_connected_devices
  
  auto it = m_connected_devices.find(id);
  
  if (it == m_connected_devices.end())
  {
    m_connected_devices_mutex.unlock(); // UNLOCK m_connected_devices
    throw std::invalid_argument("Unknown connected device ID " + std::to_string(id));
  }
  
  id = it->second.product_id;
  
  m_connected_devices_mutex.unlock(); // UNLOCK m_connected_devices
  
  return id;
}

linkmasta_device* LibusbDeviceManager::get_linkmasta_device(unsigned int id)
{
  m_connected_devices_mutex.lock();
  
  auto it = m_connected_devices.find(id);
  
  if (it == m_connected_devices.end())
  {
    m_connected_devices_mutex.unlock();
    throw std::invalid_argument("Unknown connected device ID " + std::to_string(id));
  }
  
  auto r = it->second.linkmasta;
  
  m_connected_devices_mutex.unlock();
  
  return r;
}



void LibusbDeviceManager::refresh_device_list()
{
  libusb_device** device_list;
  int num_devices = libusb_get_device_list(m_libusb, &device_list);
  std::map<unsigned int, bool> device_status;
  
  m_connected_devices_mutex.lock(); // LOCK m_connected_devices
  
  // mark all devices as not found
  for (auto entry : m_connected_devices)
  {
    device_status[entry.first] = false;
  }
  
  for (int i = 0; i < num_devices; ++i)
  {
    bool found = false;
    libusb_device_descriptor desc;
    libusb_get_device_descriptor(device_list[i], &desc);
    
    // Only deal with supported devices
    if (!is_supported(desc.idVendor, desc.idProduct))
    {
      continue;
    }
    
    // See if we already know about the device
    for (auto entry : m_connected_devices)
    {
      if (entry.second.device == device_list[i])
      {
        found = true;
        device_status[entry.first] = true;
        break;
      }
    }
    
    // Create new entry if necessary
    if (!found)
    {
      connected_device new_device;
      new_device.id = generate_id();
      new_device.vendor_id = desc.idVendor;
      new_device.product_id = desc.idProduct;
      new_device.device = device_list[i];
      
      usb::libusb_usb_device* usb_device = new usb::libusb_usb_device(new_device.device);
      usb_device->init();
      new_device.linkmasta = build_linkmasta_device(new usb::libusb_usb_device(new_device.device));
      
      m_connected_devices[new_device.id] = new_device;
      libusb_ref_device(device_list[i]);
    }
  }
  
  // Remove devices that were not found
  for (auto entry : device_status)
  {
    if (!entry.second)
    {
      delete m_connected_devices[entry.first].linkmasta;
      libusb_unref_device(m_connected_devices[entry.first].device);
      m_connected_devices.erase(entry.first);
    }
  }
  
  m_connected_devices_mutex.unlock(); // UNLOCK m_connected_devices
  
  // Free the libusb list
  libusb_free_device_list(device_list, 1);
}

bool LibusbDeviceManager::is_supported(unsigned int vendor_id, unsigned int product_id)
{
  return ((vendor_id == 0x20A0 && product_id == 0x4178)       // NGP (linkmasta)
          || (vendor_id == 0x20A0 && product_id == 0x4256)    // NGP (new flashmasta)
          || (vendor_id == 0x20A0 && product_id == 0x4252));  // WS
}


