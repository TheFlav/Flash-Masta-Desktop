#include "ngp_cartridge_flash_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"
#include "../flash_masta_app.h"
#include "games/game_catalog.h"

NgpCartridgeFlashTask::NgpCartridgeFlashTask(QWidget* parent, cartridge* cart, int slot)
  : NgpCartridgeTask(parent, cart, slot)
{
  // Nothing else to do
}

NgpCartridgeFlashTask::~NgpCartridgeFlashTask()
{
  // Nothing else to do
}



void NgpCartridgeFlashTask::go()
{
  if (m_cartridge->type() != cartridge_type::CARTRIDGE_FLASHMASTA)
  {
    QMessageBox msgBox;
    msgBox.setText("Unable to flash data to cartridge.\n\nBecause of the cartridge type, this software is unable to overwrite the data stored on this cartridge.");
    msgBox.exec();
  }
  else
  {
    NgpCartridgeTask::go();
  }
}

void NgpCartridgeFlashTask::run_task()
{
  // Detect if this would overwrite part of game
  if (m_slot != -1 && m_cartridge->num_slots() > 1)
  {
    int curr_slot = m_slot;
    int othr_slot = (m_slot + m_cartridge->num_slots() - 1) % m_cartridge->num_slots();
    const game_descriptor* curr_game = FlashMastaApp::getInstance()->getNeoGeoGameCatalog()->identify_game(m_cartridge, curr_slot);
    const game_descriptor* othr_game = FlashMastaApp::getInstance()->getNeoGeoGameCatalog()->identify_game(m_cartridge, othr_slot);
    
    if (curr_game == nullptr && othr_game != nullptr && othr_game->num_bytes > m_cartridge->slot_size(othr_slot))
    {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question((QWidget*) parent(), "Confirm",
                                    "Flashing a game to this slot may make slot " +
                                      QString::number(othr_slot+1) + " (" + QString(othr_game->name) +
                                      ") unplayable. Are you sure you want to continue?",
                                    QMessageBox::Cancel|QMessageBox::Yes, QMessageBox::Yes);
      
      delete curr_game;
      delete othr_game;
      
      switch (reply)
      {
      case QMessageBox::Yes:
        // User's ok with it, so we continue
        break;
        
      case QMessageBox::Cancel:
      default:
        // User decides to cancel, so we cancel
        return;
      }
    }
  }
  
  // Get source file from user
  QString filename = QFileDialog::getOpenFileName(
    (QWidget*) this->parent(), tr("Open File"), QString(),
    tr("Neo Geo Pocket (*.ngp);;All files (*)"));
  if (filename == QString::null)
  {
    // Quietly fail
    return;
  }
  
  // Initialize inputfile
  m_fin = new std::ifstream(filename.toStdString().c_str(), std::ios::binary);
  if (!m_fin->is_open())
  {
    delete m_fin;
    QMessageBox msgBox;
    msgBox.setText("Unable to open file");
    msgBox.exec();
    return;
  }
  
  // Gather size of file to flash
  unsigned int file_size;
  m_fin->seekg(0, m_fin->end);
  file_size = m_fin->tellg();
  m_fin->seekg(0, m_fin->beg);
  
  if (file_size > m_cartridge->descriptor()->num_bytes)
  {
    QMessageBox::information((QWidget*) parent(), "File Too Large",
                             "The selected file is too large to fit on this cartridge.",
                             QMessageBox::Ok);
    m_fin->close();
    delete m_fin;
    return;
  }
  else if (m_slot != -1 && file_size > m_cartridge->slot_size(m_slot))
  {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question((QWidget*) parent(), "File Too Large",
                                  "The selected file is too large to fit in the selected slot. "
                                  "Overwrite entire cartridge?",
                                  QMessageBox::Cancel|QMessageBox::Ok, QMessageBox::Ok);
    
    switch (reply)
    {
    case QMessageBox::Ok:
      // User is ok with it, change target slot and continue
      m_slot = -1;
      break;
      
    case QMessageBox::Cancel:
    default:
      // User decides to cancel, so we cancel;
      m_fin->close();
      delete m_fin;
      return;
    }
  }
  
  // Display a warning if it looks like the file was not made for this cartridge
  if (m_cartridge->type() == cartridge_type::CARTRIDGE_FLASHMASTA
      && m_slot != -1
      && file_size < m_cartridge->slot_size(m_slot))
  {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question((QWidget*) parent(), "Compatibility Warning",
                                  "The selected file is smaller than the selected slot on this cartridge. "
                                  "There is a chance that the game will not work as expected due to hardware differences. "
                                  "Continue?", QMessageBox::Cancel|QMessageBox::Ok, QMessageBox::Ok);
    
    switch (reply)
    {
    case QMessageBox::Ok:
      // User's ok with it, so we continue
      break;
      
    case QMessageBox::Cancel:
    default:
      // User decides to cancel, so we cancel
      m_fin->close();
      delete m_fin;
      return;
      break;
    }
  }
  
  if (m_slot == -1)
  {
    setProgressLabel(QString("Flashing data from file to entire cartridge"));
  }
  else
  {
    setProgressLabel(QString("Flashing data from file to slot ") + QString::number(m_slot+1));
  }
  
  // Begin task
  try
  {
    m_cartridge->restore_cartridge_game_data(*m_fin, (m_slot == -1 ? cartridge::SLOT_ALL : m_slot), this);
  }
  catch (std::exception& ex)
  {
    (void) ex;
    m_fin->close();
    delete m_fin;
    
    if (is_task_cancelled())
    {
      QMessageBox msgBox;
      msgBox.setText("Operation aborted: cartridge may be in an unplayable state.");
      msgBox.exec();
    }
    throw;
  }
  
  if (is_task_cancelled())
  {
    QMessageBox msgBox;
    msgBox.setText("Operation aborted: cartridge may be in an unplayable state.");
    msgBox.exec();
  }
  
  // Cleanup
  m_fin->close();
  delete m_fin;
}
