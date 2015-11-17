#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QMainWindow>
#include <QTimer>

#include "cartridge/cartridge.h"

namespace Ui {
class MainWindow;
}
class DeviceInfoWidget;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
private:
  cartridge* buildCartridgeForDevice(int id);
  
public slots:
  void setGameBackupEnabled(bool enabled);
  void setGameFlashEnabled(bool enabled);
  void setGameVerifyEnabled(bool enabled);
  void setSaveBackupEnabled(bool enabled);
  void setSaveRestoreEnabled(bool enabled);
  void setSaveVerifyEnabled(bool enabled);
  void triggerActionBackupGame();
  void triggerActionFlashGame();
  void triggerActionVerifyGame();
  void triggerActionBackupSave();
  void triggerActionRestoreSave();
  void triggerActionVerifySave();
  void refreshDeviceList_timeout();
  
private slots:
  void on_deviceListWidget_currentRowChanged(int currentRow);
  
private:
  Ui::MainWindow *ui;
  system_type m_target_system;
  QTimer m_timer;
  
  std::vector<unsigned int> m_device_ids;
  std::map<unsigned int, QWidget*> m_device_detail_widgets;
  QWidget* m_prompt_no_devices;
  QWidget* m_prompt_none_selected;
  QWidget* m_current_widget;
};

#endif // __MAIN_WINDOW_H__
