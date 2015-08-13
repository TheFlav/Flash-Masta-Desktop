#include "cartridge_backup_save_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

CartridgeBackupSaveTask::CartridgeBackupSaveTask(QWidget *parent) : CartridgeTask(parent)
{
  // Nothing else to do
}

CartridgeBackupSaveTask::~CartridgeBackupSaveTask()
{
  // Nothing else to do
}

void CartridgeBackupSaveTask::run_task()
{
  // Get destination file from user
  QString filename = QFileDialog::getSaveFileName(
    (QWidget*) this->parent(), tr("Save File"), "save_backup.ngp",
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
  
  set_progress_label("Creating game save backup");
  
  // Begin task
  try
  {
    m_cartridge->backup_cartridge_save_data(*m_fout, this);
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
