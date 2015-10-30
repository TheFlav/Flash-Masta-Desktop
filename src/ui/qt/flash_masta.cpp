#include "flash_masta.h"
#include "libusb_device_manager.h"
#include "main_window.h"

FlashMasta* FlashMasta::instance = nullptr;
const int FlashMasta::NO_DEVICE = -1;
const int FlashMasta::NO_SLOT = -1;

FlashMasta::FlashMasta(int argc, char **argv, int flags)
  : QApplication(argc, argv, flags),
    m_main_window(nullptr), m_device_manager(nullptr),
    m_game_backup_enabled(false), m_game_flash_enabled(false),
    m_game_verify_enabled(false), m_save_backup_enabled(false),
    m_save_restore_enabled(false), m_save_verify_enabled(false),
    m_selected_device(NO_DEVICE), m_selected_slot(NO_SLOT)
{
  if (FlashMasta::instance == nullptr)
  {
    FlashMasta::instance = this;
  }
  
  m_device_manager = new LibusbDeviceManager();
  m_main_window = new MainWindow();
  
  connect(m_main_window, SIGNAL(destroyed(QObject*)), this, SLOT(mainWindowDestroyed(QObject*)));
  m_main_window->show();
}

FlashMasta::~FlashMasta()
{
  // Nothing else to do
}

DeviceManager* FlashMasta::get_device_manager() const
{
  return m_device_manager;
}

MainWindow* FlashMasta::get_main_window() const
{
  return m_main_window;
}

int FlashMasta::get_selected_device() const
{
  return m_selected_device;
}



// public slots:

void FlashMasta::setGameBackupEnabled(bool enabled)
{
  emit gameBackupEnabledChanged(enabled);
}

void FlashMasta::setGameFlashEnabled(bool enabled)
{
  emit gameFlashEnabledChanged(enabled);
}

void FlashMasta::setGameVerifyEnabled(bool enabled)
{
  emit gameVerifyEnabledChanged(enabled);
}

void FlashMasta::setSaveBackupEnabled(bool enabled)
{
  emit saveBackupEnabledChanged(enabled);
}

void FlashMasta::setSaveRestoreEnabled(bool enabled)
{
  emit saveRestoreEnabledChanged(enabled);
}

void FlashMasta::setSaveVerifyEnabled(bool enabled)
{
  emit saveVerifyEnabledChanged(enabled);
}

void FlashMasta::setSelectedDevice(int device_id)
{
  int old_id = m_selected_device;
  m_selected_device = device_id;
  emit selectedDeviceChanged(old_id, device_id);
  
  if (m_selected_device == -1 || m_selected_slot == -1)
  {
    setGameBackupEnabled(false);
    setGameFlashEnabled(false);
    setGameVerifyEnabled(false);
    setSaveBackupEnabled(false);
    setSaveRestoreEnabled(false);
    setSaveVerifyEnabled(false);
  }
}

void FlashMasta::setSelectedSlot(int slot_id)
{
  int old_id = m_selected_slot;
  m_selected_slot = slot_id;
  emit selectedSlotChanged(old_id, slot_id);
  
  if (m_selected_device == -1 || m_selected_slot == -1)
  {
    setGameBackupEnabled(false);
    setGameFlashEnabled(false);
    setGameVerifyEnabled(false);
    setSaveBackupEnabled(false);
    setSaveRestoreEnabled(false);
    setSaveVerifyEnabled(false);
  }
}



// private slots:

void FlashMasta::mainWindowDestroyed(QObject* object)
{
  if (object == m_main_window)
  {
    m_main_window = nullptr;
  }
}



FlashMasta* FlashMasta::get_instance()
{
  return FlashMasta::instance;
}



