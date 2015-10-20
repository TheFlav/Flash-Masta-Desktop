#include "ngp_flashmasta_cartridge_widget.h"
#include "ui_ngp_flashmasta_cartridge_widget.h"
#include "cartridge/ngp_cartridge.h"
#include "ngp_flashmasta_cartridge_slot_widget.h"
#include "../worker/ngp_lm_cartridge_fetching_worker.h"

#include <QString>
#include <string>
#include <QThread>

NgpFlashmastaCartridgeWidget::NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFlashmastaCartridgeWidget), m_device_id(device_id), m_cartridge(nullptr)
{
  ui->setupUi(this);
  
  QThread* thread = new QThread();
  m_worker = new NgpLmCartridgeFetchingWorker(m_device_id);
  m_worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), m_worker, SLOT(run()));
  connect(m_worker, SIGNAL(finished(ngp_cartridge*)), this, SLOT(cartridge_loaded(ngp_cartridge*)));
  connect(m_worker, SIGNAL(finished(ngp_cartridge*)), thread, SLOT(quit()));
  connect(m_worker, SIGNAL(finished(ngp_cartridge*)), m_worker, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
}

NgpFlashmastaCartridgeWidget::~NgpFlashmastaCartridgeWidget()
{
  if (m_worker != nullptr) m_worker->cancel();
  if (m_cartridge != nullptr) delete m_cartridge;
  
  delete ui;
}

void NgpFlashmastaCartridgeWidget::refresh_ui()
{
  // Reset everything and erase cached data
  ui->slotsComboBox->clear();
  for (QWidget* widget : m_slot_widgets)
  {
    delete widget;
  }
  m_slot_widgets.clear();
  
  ui->slotsComboBox->insertItem(0, "Cartridge Info");
  
  for (unsigned int i = 0; i < m_cartridge->num_slots(); ++i)
  {
    ui->slotsComboBox->insertItem(i+1, "Slot " + QString::number(i+1));
  }
  
  ui->slotsComboBox->setCurrentIndex(0);
}

void NgpFlashmastaCartridgeWidget::cartridge_loaded(ngp_cartridge* cartridge)
{
  if (m_cartridge != nullptr) delete m_cartridge;
  m_cartridge = cartridge;
  m_worker = nullptr;
  refresh_ui();
}
