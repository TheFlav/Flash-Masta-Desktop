#include "ngp_cartridge_backup_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"
#include "../flash_masta_app.h"
#include "games/game_catalog.h"

NgpCartridgeBackupTask::NgpCartridgeBackupTask(QWidget *parent, cartridge* cart, int slot)
  : NgpCartridgeTask(parent, cart, slot)
{
  // Nothing else to do
}

NgpCartridgeBackupTask::~NgpCartridgeBackupTask()
{
  // Nothing else to do
}

void NgpCartridgeBackupTask::run_task()
{
  // Handle multi-slot games
  if (m_slot != -1 && m_cartridge->num_slots() > 1)
  {
    int curr_slot = m_slot;
    int othr_slot = (m_slot + m_cartridge->num_slots() - 1) % m_cartridge->num_slots();
    const game_descriptor* curr_game = FlashMastaApp::getInstance()->getNeoGeoGameCatalog()->identify_game(m_cartridge, curr_slot);
    const game_descriptor* othr_game = FlashMastaApp::getInstance()->getNeoGeoGameCatalog()->identify_game(m_cartridge, othr_slot);
    
    if ((curr_game == nullptr && othr_game != nullptr && othr_game->num_bytes > m_cartridge->slot_size(othr_slot))
        || (othr_game == nullptr && curr_game != nullptr && curr_game->num_bytes > m_cartridge->slot_size(curr_slot)))
    {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question((QWidget*) parent(), "Confirm",
                                    "It appears that the cartridge contains a game that spans multiple slots. "
                                    "Would you like to back up the entire cartridge instead?",
                                    QMessageBox::No|QMessageBox::Yes, QMessageBox::Yes);
      
      delete curr_game;
      delete othr_game;
      
      switch (reply)
      {
      case QMessageBox::Yes:
        // User's ok with it, so we change target slot
        m_slot = -1;
        break;
        
      case QMessageBox::No:
      default:
        // User decides to continue, so no action
        break;
      }
    }
  }
  
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
  
  // Let user know what's happening
  if (m_slot == -1)
  {
    setProgressLabel("Backing up entire cartridge to file");
  }
  else
  {
    setProgressLabel(QString("Backing up slot " + QString::number(m_slot + 1) + QString(" to file")));
  }
  
  // Begin task
  try
  {
    m_cartridge->backup_cartridge_game_data(*m_fout, (m_slot == -1 ? cartridge::SLOT_ALL : m_slot), this);
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
