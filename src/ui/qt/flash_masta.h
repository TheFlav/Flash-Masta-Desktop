#ifndef FLASHMASTA_H
#define FLASHMASTA_H

#include <QApplication>

class DeviceManager;
class MainWindow;

class FlashMasta: public QApplication
{
public:
  FlashMasta(int argc, char** argv, int flags = ApplicationFlags);
  ~FlashMasta();
  
  DeviceManager* get_device_manager() const;
  MainWindow* get_main_window() const;
  
public slots:
  void setGameBackupEnabled(bool enabled);
  void setGameFlashEnabled(bool enabled);
  void setGameVerifyEnabled(bool enabled);
  void setSaveBackupEnabled(bool enabled);
  void setSaveRestoreEnabled(bool enabled);
  void setSaveVerifyEnabled(bool enabled);
  void setSelectedDevice(int device_id);
  void setSelectedSlot(int slot_id);
  
private slots:
  void mainWindowDestroyed(QObject*);
  
signals:
  void gameBackupEnabledChanged(bool);
  void gameFlashEnabledChanged(bool);
  void gameVerifyEnabledChanged(bool);
  void saveBackupEnabledChanged(bool);
  void saveRestoreEnabledChanged(bool);
  void saveVerifyEnabledChanged(bool);
  void selectedDeviceChanged(int, int);
  void selectedSlotChanged(int, int);
  
public:
  static FlashMasta* get_instance();
  
private:
  Q_DISABLE_COPY(FlashMasta)
  
  MainWindow* m_main_window;
  DeviceManager* m_device_manager;
  bool m_game_backup_enabled;
  bool m_game_flash_enabled;
  bool m_game_verify_enabled;
  bool m_save_backup_enabled;
  bool m_save_restore_enabled;
  bool m_save_verify_enabled;
  int m_selected_device;
  int m_selected_slot;
  
  static FlashMasta* instance;
  static const int NO_DEVICE;
  static const int NO_SLOT;
};

#endif // FLASHMASTA_H
