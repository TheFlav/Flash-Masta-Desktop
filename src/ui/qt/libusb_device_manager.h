#ifndef __LIBUSB_DEVICE_MANAGER_H__
#define __LIBUSB_DEVICE_MANAGER_H__

#include "device_manager.h"
#include <map>

struct libusb_context;
struct libusb_device;



class LibusbDeviceManager : public DeviceManager
{
  /////////////// METHODS ///////////////
public:
  LibusbDeviceManager();
  ~LibusbDeviceManager();
  
  std::vector<unsigned int> get_connected_devices();
  bool                      try_get_connected_devices(std::vector<unsigned int>& devices);
  bool                      is_connected(unsigned int id);
  unsigned int              get_vendor_id(unsigned int id);
  unsigned int              get_product_id(unsigned int id);
  linkmasta_device*         get_linkmasta_device(unsigned int id);
  
protected:
  void refresh_device_list();
  
private:
  static bool is_supported(unsigned int vendor_id, unsigned int product_id);
  
  //////////////// DATA ////////////////
private:
  libusb_context* m_libusb;
  
  struct connected_device
  {
    unsigned int id;
    unsigned int vendor_id;
    unsigned int product_id;
    libusb_device* device;
    linkmasta_device* linkmasta;
  };
  std::map<unsigned int, connected_device> m_connected_devices;
  std::mutex m_connected_devices_mutex;
};

#endif // __LIBUSB_DEVICE_MANAGER_H__
