#include "ngp_cartridge_verify_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"
#include "../flash_masta_app.h"
#include "game/game_catalog.h"

NgpCartridgeVerifyTask::NgpCartridgeVerifyTask(QWidget *parent, cartridge* cart, int slot)
  : NgpCartridgeTask(parent, cart, slot)
{
  // Nothing else to do
}

NgpCartridgeVerifyTask::~NgpCartridgeVerifyTask()
{
  // Nothing else to do
}

void NgpCartridgeVerifyTask::run_task()
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
                                    "Would you like to verify the entire cartridge instead?",
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
  
  if (m_slot == -1)
  {
    setProgressLabel("Comparing cartridge data to file");
  }
  else
  {
    setProgressLabel(QString("Comparing slot ") + QString::number(m_slot+1) + QString(" data to file"));
  }
  
  // Begin task
  try
  {
    if (m_cartridge->compare_cartridge_game_data(*m_fin, (m_slot == -1 ? cartridge::SLOT_ALL : m_slot), this) && !is_task_cancelled())
    {
      QMessageBox msgBox;
      msgBox.setText("Cartridge and file match.");
      msgBox.exec();
    }
    else if(!is_task_cancelled())
    {
      QMessageBox msgBox;
      msgBox.setText("Cartridge data does not match the chosen file.");
      msgBox.exec();
    }
  }
  catch (std::exception& ex)
  {
    (void) ex;
    m_fin->close();
    delete m_fin;
    throw;
  }
  
  // Cleanup
  m_fin->close();
  delete m_fin;
}
