#include "ngp_flashmasta_cartridge_widget.h"
#include "ui_ngp_flashmasta_cartridge_widget.h"
#include "cartridge/ngp_cartridge.h"
#include "ngp_flashmasta_cartridge_slot_widget.h"
#include "../worker/ngp_lm_cartridge_fetching_worker.h"
#include "ngp_fm_cartridge_info_widget.h"
#include "fm_cartridge_slot_widget.h"

#include <QString>
#include <string>
#include <QThread>

NgpFlashmastaCartridgeWidget::NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFlashmastaCartridgeWidget), m_device_id(device_id), m_cartridge(nullptr)
{
  ui->setupUi(this);
  
  m_default_widget = ui->defaultWidget;
  m_current_widget = m_default_widget;
  
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
  m_slot_widgets.reserve(m_cartridge->num_slots() + 1);
  
  ui->slotsComboBox->insertItem(0, "Cartridge Info");
  m_slot_widgets.push_back(new NgpFmCartridgeInfoWidget(m_cartridge, ui->verticalLayout->widget()));
  m_slot_widgets.back()->hide();
  ui->verticalLayout->addWidget(m_slot_widgets.back(), 1);
  
  for (unsigned int i = 0; i < m_cartridge->num_slots(); ++i)
  {
    FmCartridgeSlotWidget* slot_widget = new FmCartridgeSlotWidget(m_cartridge, (int) i, ui->verticalLayout->widget());
    m_slot_widgets.push_back(slot_widget);
    slot_widget->hide();
    
    ui->verticalLayout->addWidget(slot_widget, 1);
    ui->slotsComboBox->insertItem(i+1, "Slot " + QString::number(i+1));
    
    connect(slot_widget, SIGNAL(gameBackupTriggered()), this, SLOT(when_gameBackupTriggered()));
    connect(slot_widget, SIGNAL(gameFlashTriggered()), this, SLOT(when_gameFlashTriggered()));
    connect(slot_widget, SIGNAL(gameVerifyTriggered()), this, SLOT(when_gameVerifyTriggered()));
    connect(slot_widget, SIGNAL(saveBackupTriggered()), this, SLOT(when_saveBackupTriggered()));
    connect(slot_widget, SIGNAL(saveRestoreTriggered()), this, SLOT(when_saveRestoreTriggered()));
    connect(slot_widget, SIGNAL(saveVerifyTriggered()), this, SLOT(when_saveVerifyTriggered()));
  }
  
  ui->slotsComboBox->setCurrentIndex(0);
  on_slotsComboBox_currentIndexChanged(0);
}



// public slots:

void NgpFlashmastaCartridgeWidget::cartridge_loaded(ngp_cartridge* cartridge)
{
  if (m_cartridge != nullptr) delete m_cartridge;
  m_cartridge = cartridge;
  m_worker = nullptr;
  refresh_ui();
}

void NgpFlashmastaCartridgeWidget::when_gameBackupTriggered()
{
  emit gameBackupTriggered();
}

void NgpFlashmastaCartridgeWidget::when_gameFlashTriggered()
{
  emit gameFlashTriggered();
}

void NgpFlashmastaCartridgeWidget::when_gameVerifyTriggered()
{
  emit gameVerifyTriggered();
}

void NgpFlashmastaCartridgeWidget::when_saveBackupTriggered()
{
  emit saveBackupTriggered();
}

void NgpFlashmastaCartridgeWidget::when_saveRestoreTriggered()
{
  emit saveRestoreTriggered();
}

void NgpFlashmastaCartridgeWidget::when_saveVerifyTriggered()
{
  emit saveVerifyTriggered();
}



// private slots:

void NgpFlashmastaCartridgeWidget::on_slotsComboBox_currentIndexChanged(int index)
{
  m_current_widget->hide();
  
  if (index >= 0 && index < (int) m_slot_widgets.size())
  {
    m_current_widget = m_slot_widgets[index];
  }
  else
  {
    m_current_widget = m_default_widget;
  }
  
  m_current_widget->show();
}
