#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <mutex>
#include <thread>
#include <vector>

namespace usb {
class usb_device;
}
class linkmasta_device;



class DeviceManager
{
  //////////////// METHODS ////////////////
public:
                                    DeviceManager();
  virtual                           ~DeviceManager();
  
  virtual std::vector<unsigned int> getConnectedDevices() = 0;
  virtual bool                      tryGetConnectedDevices(std::vector<unsigned int>& devices) = 0;
  virtual bool                      isConnected(unsigned int id) = 0;
  virtual unsigned int              getVendorId(unsigned int id) = 0;
  virtual unsigned int              getProductId(unsigned int id) = 0;
  virtual std::string               getManufacturerString(unsigned int id) = 0;
  virtual std::string               getProductString(unsigned int id) = 0;
  virtual std::string               getSerialNumber(unsigned int id) = 0;
  virtual linkmasta_device*         getLinkmastaDevice(unsigned int id) = 0;
  virtual bool                      isDeviceClaimed(unsigned int id) = 0;
  virtual bool                      tryClaimDevice(unsigned int id) = 0;
  virtual void                      releaseDevice(unsigned int id) = 0;
  
protected:
  unsigned int                      generateId();
  void                              startAutoRefresh();
  void                              stopAutoRefreshAndWait();
  virtual void                      refreshDeviceList() = 0;
  static linkmasta_device*          buildLinkmastaDevice(usb::usb_device* device);
  
private:
                                    DeviceManager(const DeviceManager& other) = delete;
  void                              refreshThreadFunction();
  
  
  //////////////// DATA ////////////////
private:
  std::thread                       m_refresh_thread;
  bool                              m_thread_kill_flag;
  bool                              m_thread_dead;
  
  unsigned int                      curr_id;
};

#endif // __DEVICE_MANAGER_H__


