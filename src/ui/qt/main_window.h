#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QMainWindow>
#include "cartridge/cartridge.h"

namespace Ui {
class MainWindow;
}

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
  
private:
  Ui::MainWindow *ui;
  system_type m_target_system;
  
};

#endif // __MAINWINDOW_H__
