#include "cartridge_verify_task.h"
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

CartridgeVerifyTask::CartridgeVerifyTask(QWidget *parent) : CartridgeTask(parent)
{
  // Nothing else to do
}

CartridgeVerifyTask::~CartridgeVerifyTask()
{
  // Nothing else to do
}

void CartridgeVerifyTask::run_task()
{
  // Get destination file from user
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
  
  set_progress_label("Verifying cartridge");
  
  // Begin task
  try
  {
    if (m_cartridge->compare_file_to_cartridge(*m_fin, this))
    {
      QMessageBox msgBox;
      msgBox.setText("Cartridge matches the chosen file 100%.");
      msgBox.exec();
    }
    else
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
