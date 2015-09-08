#include "flash_masta.h"
#include "libusb_device_manager.h"

FlashMasta* FlashMasta::instance = nullptr;



FlashMasta::FlashMasta(int argc, char **argv, int flags)
  : QApplication(argc, argv, flags),
    m_device_manager(new LibusbDeviceManager())
{
  if (FlashMasta::instance == nullptr)
  {
    FlashMasta::instance = this;
  }
}

FlashMasta::~FlashMasta()
{
  // Nothing else to do
}



FlashMasta* FlashMasta::get_instance()
{
  return FlashMasta::instance;
}



DeviceManager* FlashMasta::get_device_manager() const
{
  return m_device_manager;
}


