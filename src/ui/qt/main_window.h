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
  
  void print_to_console(const char* message, ...);
  
private slots:
  void on_combobox_system_type_currentIndexChanged(int index);
  void on_button_backup_rom_clicked();
  void on_button_verify_rom_clicked();
  void on_button_flash_rom_clicked();
  void on_button_backup_save_clicked();
  void on_button_restore_save_clicked();
  
private:
  enum button_flags {
    ROM_FLASH = 1 << 0,
    ROM_BACKUP = 1 << 1,
    ROM_VERIFY = 1 << 2,
    SAVE_RESTORE = 1 << 3,
    SAVE_BACKUP = 1 << 4,
    CARTRIDGE_INFO = 1 << 5,
    PREFERENCES = 1 << 6
  };
  
  void enable_buttons(int buttons);
  
  Ui::MainWindow *ui;
  system_type m_target_system;
  
};

#endif // __MAINWINDOW_H__
