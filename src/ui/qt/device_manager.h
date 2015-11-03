#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <thread>
#include <mutex>
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
  
  virtual std::vector<unsigned int> get_connected_devices() = 0;
  virtual bool                      try_get_connected_devices(std::vector<unsigned int>& devices) = 0;
  virtual bool                      is_connected(unsigned int id) = 0;
  virtual unsigned int              get_vendor_id(unsigned int id) = 0;
  virtual unsigned int              get_product_id(unsigned int id) = 0;
  virtual std::string               get_manufacturer_string(unsigned int id) = 0;
  virtual std::string               get_product_string(unsigned int id) = 0;
  virtual std::string               get_serial_number(unsigned int id) = 0;
  virtual linkmasta_device*         get_linkmasta_device(unsigned int id) = 0;
  virtual bool                      is_device_claimed(unsigned int id) = 0;
  virtual bool                      claim_device(unsigned int id) = 0;
  virtual void                      release_device(unsigned int id) = 0;
  
protected:
  unsigned int                      generate_id();
  void                              start_auto_refresh();
  virtual void                      refresh_device_list() = 0;
  static linkmasta_device*          build_linkmasta_device(usb::usb_device* device);
  
private:
                                    DeviceManager(const DeviceManager& other) = delete;
  void                              refresh_thread_function();
  
  
  //////////////// DATA ////////////////
private:
  std::thread                       m_refresh_thread;
  bool                              m_thread_kill_flag;
  bool                              m_thread_dead;
  
  unsigned int                      curr_id;
};

#endif // __DEVICE_MANAGER_H__


