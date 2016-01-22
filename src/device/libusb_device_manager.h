#ifndef __LIBUSB_DEVICE_MANAGER_H__
#define __LIBUSB_DEVICE_MANAGER_H__

#include "device_manager.h"

#include <map>
#include <string>

struct libusb_context;
struct libusb_device;



class libusb_device_manager : public device_manager
{
  /////////////// METHODS ///////////////
public:
  libusb_device_manager();
  ~libusb_device_manager();
  
  std::vector<unsigned int> get_connected_devices();
  bool                      try_get_connected_devices(std::vector<unsigned int>& devices);
  bool                      is_connected(unsigned int id);
  unsigned int              get_vendor_id(unsigned int id);
  unsigned int              get_product_id(unsigned int id);
  std::string               get_manufacturer_string(unsigned int id);
  std::string               get_product_string(unsigned int id);
  std::string               get_serial_number(unsigned int id);
  linkmasta_device*         get_linkmasta_device(unsigned int id);
  bool                      is_device_claimed(unsigned int id);
  bool                      try_claim_device(unsigned int id);
  void                      release_device(unsigned int id);
  
protected:
  void refresh_device_list();
  
private:
  static bool is_supported(unsigned int vendor_id, unsigned int product_id);
  
  //////////////// DATA ////////////////
private:
  libusb_context* m_libusb;
  std::mutex      m_libusb_mutex;
  bool            m_libusb_init;
  
  struct connected_device
  {
    unsigned int id;
    unsigned int vendor_id;
    unsigned int product_id;
    std::string  manufacturer_string;
    std::string  product_string;
    std::string  serial_number;
    libusb_device* device;
    linkmasta_device* linkmasta;
    bool claimed;
  };
  std::map<unsigned int, connected_device> m_connected_devices;
  std::mutex m_connected_devices_mutex;
  std::vector<connected_device> m_disconnected_devices;
};

#endif // __LIBUSB_DEVICE_MANAGER_H__
