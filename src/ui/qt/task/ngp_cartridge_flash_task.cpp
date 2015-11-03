#include "ngp_cartridge_flash_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

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
  
  if (m_cartridge->type() == cartridge_type::CARTRIDGE_FLASHMASTA)
  {
    unsigned int file_size;
    m_fin->seekg(0, m_fin->end);
    file_size = m_fin->tellg();
    m_fin->seekg(0, m_fin->beg);
    
    // Display a warning if it looks like the file was not made for this cartridge
    if (m_slot != -1 && file_size < m_cartridge->slot_size(m_slot))
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
  }  
  
  if (m_slot == -1)
  {
    set_progress_label(QString("Flashing data from file to entire cartridge"));
  }
  else
  {
    set_progress_label(QString("Flashing data from file to slot ") + QString::number(m_slot+1));
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
