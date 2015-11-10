#include "ngp_cartridge_backup_save_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

NgpCartridgeBackupSaveTask::NgpCartridgeBackupSaveTask(QWidget *parent, cartridge* cart, int slot)
  : NgpCartridgeTask(parent, cart, slot)
{
  // Nothing else to do
}

NgpCartridgeBackupSaveTask::~NgpCartridgeBackupSaveTask()
{
  // Nothing else to do
}

void NgpCartridgeBackupSaveTask::go()
{
  if (m_cartridge->type() == cartridge_type::CARTRIDGE_FLASHMASTA)
  {
    QMessageBox msgBox;
    msgBox.setText("Unable to backup save game data.\n\nBecause of the cartridge type, this software is unable to detect where the game save data is stored.");
    msgBox.exec();
  }
  else
  {
    NgpCartridgeTask::go();
  }
}

void NgpCartridgeBackupSaveTask::run_task()
{
  // Get destination file from user
  QString filename = QFileDialog::getSaveFileName(
    (QWidget*) this->parent(), tr("Save File"), "save_backup.ngf",
    tr("Neo Geo File (*.ngf);;All files (*)"));
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
  
  if (m_slot == -1)
  {
    setProgressLabel(QString("Backing up save data in entire cartridge to file"));
  }
  else
  {
    setProgressLabel(QString("Backing up save data in slot ") + QString::number(m_slot+1) + QString(" to file"));
  }
  
  // Begin task
  try
  {
    m_cartridge->backup_cartridge_save_data(*m_fout, (m_slot == -1 ? cartridge::SLOT_ALL : m_slot), this);
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
