#ifndef __LIBUSB_DEVICE_MANAGER_H__
#define __LIBUSB_DEVICE_MANAGER_H__

#include "device_manager.h"
#include <map>
#include <string>

struct libusb_context;
struct libusb_device;



class LibusbDeviceManager : public DeviceManager
{
  /////////////// METHODS ///////////////
public:
  LibusbDeviceManager();
  ~LibusbDeviceManager();
  
  std::vector<unsigned int> getConnectedDevices();
  bool                      tryGetConnectedDevices(std::vector<unsigned int>& devices);
  bool                      isConnected(unsigned int id);
  unsigned int              getVendorId(unsigned int id);
  unsigned int              getProductId(unsigned int id);
  std::string               getManufacturerString(unsigned int id);
  std::string               getProductString(unsigned int id);
  std::string               getSerialNumber(unsigned int id);
  linkmasta_device*         getLinkmastaDevice(unsigned int id);
  bool                      isDeviceClaimed(unsigned int id);
  bool                      tryClaimDevice(unsigned int id);
  void                      releaseDevice(unsigned int id);
  
protected:
  void refreshDeviceList();
  
private:
  static bool isSupported(unsigned int vendor_id, unsigned int product_id);
  
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
