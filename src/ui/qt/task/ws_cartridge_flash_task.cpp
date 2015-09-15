#include "ws_cartridge_flash_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

WsCartridgeFlashTask::WsCartridgeFlashTask(QWidget* parent, cartridge* cart): WsCartridgeTask(parent, cart)
{
  // Nothing else to do
}

WsCartridgeFlashTask::~WsCartridgeFlashTask()
{
  // Nothing else to do
}



void WsCartridgeFlashTask::run_task()
{
  // Get source file from user
  QString filename = QFileDialog::getOpenFileName(
    (QWidget*) this->parent(), tr("Open File"), QString(),
    tr("WonderSwan Color (*.wsc);;WonderSwan (*.ws)"));
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
  
  set_progress_label("Writing data to cartridge");
  
  // Begin task
  try
  {
    m_cartridge->restore_cartridge_game_data(*m_fin, cartridge::SLOT_ALL, this);
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
