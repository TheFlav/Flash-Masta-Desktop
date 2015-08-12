#include "cartridge_flash_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

CartridgeFlashTask::CartridgeFlashTask(QWidget* parent): CartridgeTask(parent)
{
  // Nothing else to do
}

CartridgeFlashTask::~CartridgeFlashTask()
{
  // Nothing else to do
}



void CartridgeFlashTask::run_task()
{
  // Get source file from user
  QString filename = QFileDialog::getOpenFileName(
    (QWidget*) this->parent(), tr("Open File"), QString(),
    tr("Neo Geo Pocket (*.ngp)"));
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
    m_cartridge->write_file_to_cartridge(*m_fin, this);
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
