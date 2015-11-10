#include "ngp_cartridge_verify_task.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include "cartridge/cartridge.h"

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
