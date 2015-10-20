#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

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
  
private slots:
  void on_actionBackupROM_triggered();
  void on_actionRestoreROM_triggered();
  void on_actionVerifyROM_triggered();
  void on_actionBackupSave_triggered();
  void on_actionRestoreSave_triggered();
  void refreshDeviceList_timeout();
  
  void on_deviceListWidget_currentRowChanged(int currentRow);
  
private:
  Ui::MainWindow *ui;
  system_type m_target_system;
  QTimer m_timer;
  
  std::vector<unsigned int> m_device_ids;
  std::map<unsigned int, QWidget*> m_device_detail_widgets;
  QWidget* m_default_widget;
};

#endif // __MAINWINDOW_H__
