#include "cartridge_widget.h"
#include "ui_cartridge_widget.h"
#include "cartridge/cartridge.h"
#include "../worker/lm_cartridge_fetching_worker.h"
#include "ngp_fm_cartridge_info_widget.h"
#include "fm_cartridge_slot_widget.h"
#include "../device_manager.h"
#include "cartridge/ngp_cartridge.h"

#include <QString>
#include <string>
#include <QThread>

CartridgeWidget::CartridgeWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::CartridgeWidget), m_current_slot(-1),
  m_device_id(device_id), m_cartridge(nullptr)
{
  ui->setupUi(this);
  hideSlotSelection();
  
  m_default_widget = ui->defaultWidget;
  m_current_widget = m_default_widget;
  
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedDeviceChanged(int,int)), this, SLOT(deviceSelected(int,int)));
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedSlotChanged(int,int)), this, SLOT(slotSelected(int,int)));
  
  QThread* thread = new QThread();
  m_worker = new LmCartridgeFetchingWorker(m_device_id);
  m_worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), m_worker, SLOT(run()));
  connect(m_worker, SIGNAL(finished(cartridge*)), this, SLOT(cartridgeLoaded(cartridge*)));
  connect(m_worker, SIGNAL(finished(cartridge*)), thread, SLOT(quit()));
  connect(m_worker, SIGNAL(finished(cartridge*)), m_worker, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
}

CartridgeWidget::~CartridgeWidget()
{
  if (m_worker != nullptr) m_worker->cancel();
  if (m_cartridge != nullptr) delete m_cartridge;
  
  delete ui;
}



void CartridgeWidget::refreshUi()
{
  if (m_cartridge->type() == cartridge_type::CARTRIDGE_FLASHMASTA)
  {
    showSlotSelection();
  }
  else
  {
    hideSlotSelection();
  }
  
  // Reset everything and erase cached data
  ui->slotsComboBox->clear();
  for (QWidget* widget : m_slot_widgets)
  {
    delete widget;
  }
  m_slot_widgets.clear();
  m_slot_widgets.reserve(m_cartridge->num_slots() + 1);
  
  ui->slotsComboBox->insertItem(0, "Cartridge Info");
  m_slot_widgets.push_back(new NgpFmCartridgeInfoWidget((int) m_device_id, (ngp_cartridge*) m_cartridge, ui->verticalLayout->widget()));
  m_slot_widgets.back()->hide();
  ui->verticalLayout->addWidget(m_slot_widgets.back(), 1);
  
  if (m_cartridge->type() == cartridge_type::CARTRIDGE_FLASHMASTA)
  {
    for (unsigned int i = 0; i < m_cartridge->num_slots(); ++i)
    {
      FmCartridgeSlotWidget* slot_widget = new FmCartridgeSlotWidget(m_device_id, m_cartridge, (int) i, ui->verticalLayout->widget());
      m_slot_widgets.push_back(slot_widget);
      slot_widget->hide();
      
      ui->verticalLayout->addWidget(slot_widget, 1);
      ui->slotsComboBox->insertItem(i+1, "Slot " + QString::number(i+1) + " - " + slot_widget->slotGameName());
    }
  }
  
  ui->slotsComboBox->setCurrentIndex(0);
  on_slotsComboBox_currentIndexChanged(0);
}



// private:

void CartridgeWidget::showSlotSelection()
{
  ui->slotsComboBox->show();
}

void CartridgeWidget::hideSlotSelection()
{
  ui->slotsComboBox->hide();
}



// public slots:

void CartridgeWidget::cartridgeLoaded(cartridge* cartridge)
{
  if (m_cartridge != nullptr) delete m_cartridge;
  m_cartridge = cartridge;
  m_worker = nullptr;
  refreshUi();
}

void CartridgeWidget::deviceSelected(int old_device_id, int new_device_id)
{
  (void) old_device_id;
  if (new_device_id != (int) m_device_id) return;
  FlashMastaApp::getInstance()->setSelectedSlot(m_current_slot);
}

void CartridgeWidget::slotSelected(int old_slot_id, int new_slot_id)
{
  (void) old_slot_id;
  (void) new_slot_id;
  if (FlashMastaApp::getInstance()->getSelectedDevice() != (int) m_device_id) return;
  updateEnabledActions();
}

void CartridgeWidget::updateEnabledActions()
{
  FlashMastaApp* app = FlashMastaApp::getInstance();
  if (m_cartridge == nullptr || m_slot_widgets.empty() || m_slot_widgets[0] == nullptr)
  {
    app->setGameBackupEnabled(false);
    app->setGameFlashEnabled(false);
    app->setGameVerifyEnabled(false);
    app->setSaveBackupEnabled(false);
    app->setSaveRestoreEnabled(false);
    app->setSaveVerifyEnabled(false);
  }
  else
  {
    NgpFmCartridgeInfoWidget* widget = (NgpFmCartridgeInfoWidget*) m_slot_widgets[0];
    
    app->setGameBackupEnabled(widget->gameBackupEnabled());
    app->setGameFlashEnabled(widget->gameFlashEnabled());
    app->setGameVerifyEnabled(widget->gameVerifyEnabled());
    app->setSaveBackupEnabled(widget->saveBackupEnabled());
    app->setSaveRestoreEnabled(widget->saveRestoreEnabled());
    app->setSaveVerifyEnabled(widget->saveVerifyEnabled());
  }
}



// private slots:

void CartridgeWidget::on_slotsComboBox_currentIndexChanged(int index)
{
  m_current_widget->hide();
  
  if (index >= 0 && index < (int) m_slot_widgets.size())
  {
    m_current_widget = m_slot_widgets[index];
    m_current_slot = index-1;
  }
  else
  {
    m_current_widget = m_default_widget;
    m_current_slot = -1;
  }
  
  m_current_widget->show();
  FlashMastaApp::getInstance()->setSelectedSlot(index - 1);
}
