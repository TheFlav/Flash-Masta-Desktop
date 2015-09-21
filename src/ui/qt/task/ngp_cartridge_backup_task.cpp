#include "ngp_cartridge_backup_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

NgpCartridgeBackupTask::NgpCartridgeBackupTask(QWidget *parent, cartridge* cart) : NgpCartridgeTask(parent, cart)
{
  // Nothing else to do
}

NgpCartridgeBackupTask::~NgpCartridgeBackupTask()
{
  // Nothing else to do
}

void NgpCartridgeBackupTask::run_task()
{
  // Get destination file from user
  QString filename = QFileDialog::getSaveFileName(
    (QWidget*) this->parent(), tr("Save File"), "backup.ngp",
    tr("Neo Geo Pocket (*.ngp);;All files (*)"));
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
    m_cartridge->backup_cartridge_game_data(*m_fout, cartridge::SLOT_ALL, this);
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
