#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qfontdatabase.h>
#include <qfiledialog.h>
#include "../../hardware/PC-App-CLI/NeoLinkmasta.h"



MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  ui->textBrowser->setFont(fixedFont);
  
  print_to_console("");
  print_to_console("-=> NeoLinkMasta GUI v%d.%d build %d <=-", 1, 2, 1001);
  print_to_console("");
  print_to_console("     (c) Mr.Spiv & Flavor in 2011.");
  print_to_console("");
}

MainWindow::~MainWindow()
{
  delete ui;
}



void MainWindow::print_to_console(const char* message, ...)
{
  char buf[1024];
  va_list args;
  
  // Format string and arguments and store in buffer
  va_start(args, message);
  vsprintf(buf, message, args);
  
  // Append message to console
  ui->textBrowser->append(buf);
  
  va_end(args);
}

void MainWindow::on_button_backup_rom_clicked()
{
  char* argv[3] = {0};
  char argv1[] = "";
  char argv2[] = "/b";
  char argv3[256] = "";
  argv[0] = argv1;
  argv[1] = argv2;
  argv[2] = argv3;
  
  QString filename = QFileDialog::getSaveFileName(
        this, tr("Save File"), "backup.ngp",
        tr("Neo Geo Pocket (*.ngp)"));
  
  if (filename == QString::null)
  {
    // Do nothing. fail.
  }
  else
  {
    strcpy(argv3, filename.toStdString().c_str());
    print_to_console("%d", _main(3, argv));
  }
}

void MainWindow::on_button_flash_rom_clicked()
{
  char* argv[3] = {0};
  char argv1[] = "";
  char argv2[] = "/w";
  char argv3[256] = "";
  argv[0] = argv1;
  argv[1] = argv2;
  argv[2] = argv3;
  
  QString filename = QFileDialog::getOpenFileName(
        this, tr("Open File"), "backup.ngp",
        tr("Neo Geo Pocket (*.ngp)"));
  
  if (filename == QString::null)
  {
    // Do nothing. fail.
  }
  else
  {
    strcpy(argv3, filename.toStdString().c_str());
    print_to_console("%d", _main(3, argv));
  }
}
