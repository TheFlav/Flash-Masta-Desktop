#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
  void on_button_backup_rom_clicked();
  
  void on_button_flash_rom_clicked();
  
private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
