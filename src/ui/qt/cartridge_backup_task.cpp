#include "cartridge_backup_task.h"
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

CartridgeBackupTask::CartridgeBackupTask(QWidget *parent) : CartridgeTask(parent)
{
  // Nothing else to do
}

CartridgeBackupTask::~CartridgeBackupTask()
{
  // Nothing else to do
}

void CartridgeBackupTask::run_task()
{
  // Get destination file from user
  QString filename = QFileDialog::getSaveFileName(
    (QWidget*) this->parent(), tr("Save File"), "backup.ngp",
    tr("Neo Geo Pocket (*.ngp)"));
  if (filename == QString::null)
  {
    // Quietly fail
    return;
  }
  
  // Initialize output file
  m_fout = new std::ofstream(filename.toStdString().c_str(), std::ios::binary);
  if (!m_fout->is_open())
  {
    delete m_fout;
    QMessageBox msgBox;
    msgBox.setText("Unable to open destination file");
    msgBox.exec();
    return;
  }
  
  set_progress_label("Creating cartridge backup");
  
  // Begin task
  try
  {
    m_cartridge->write_cartridge_to_file(*m_fout, this);
  }
  catch (std::exception& ex)
  {
    (void) ex;
    m_fout->close();
    delete m_fout;
    throw;
  }
  
  // Cleanup
  m_fout->close();
  delete m_fout;
}
