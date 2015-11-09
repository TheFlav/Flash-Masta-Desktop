#include "ngp_flashmasta_cartridge_widget.h"
#include "ui_ngp_flashmasta_cartridge_widget.h"
#include "cartridge/ngp_cartridge.h"
#include "../worker/ngp_lm_cartridge_fetching_worker.h"
#include "ngp_fm_cartridge_info_widget.h"
#include "fm_cartridge_slot_widget.h"
#include "../device_manager.h"

#include <QString>
#include <string>
#include <QThread>

NgpFlashmastaCartridgeWidget::NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFlashmastaCartridgeWidget), m_current_slot(-1),
  m_device_id(device_id), m_cartridge(nullptr)
{
  ui->setupUi(this);
  
  m_default_widget = ui->defaultWidget;
  m_current_widget = m_default_widget;
  
  connect(FlashMasta::get_instance(), SIGNAL(selectedDeviceChanged(int,int)), this, SLOT(device_selected(int,int)));
  connect(FlashMasta::get_instance(), SIGNAL(selectedSlotChanged(int,int)), this, SLOT(slot_selected(int,int)));
  
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
  m_slot_widgets.push_back(new NgpFmCartridgeInfoWidget((int) m_device_id, m_cartridge, ui->verticalLayout->widget()));
  m_slot_widgets.back()->hide();
  ui->verticalLayout->addWidget(m_slot_widgets.back(), 1);
  
  for (unsigned int i = 0; i < m_cartridge->num_slots(); ++i)
  {
    FmCartridgeSlotWidget* slot_widget = new FmCartridgeSlotWidget(m_device_id, m_cartridge, (int) i, ui->verticalLayout->widget());
    m_slot_widgets.push_back(slot_widget);
    slot_widget->hide();
    
    ui->verticalLayout->addWidget(slot_widget, 1);
    ui->slotsComboBox->insertItem(i+1, "Slot " + QString::number(i+1) + " - " + slot_widget->slotGameName());
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

void NgpFlashmastaCartridgeWidget::device_selected(int old_device_id, int new_device_id)
{
  (void) old_device_id;
  if (new_device_id != (int) m_device_id) return;
  FlashMasta::get_instance()->setSelectedSlot(m_current_slot);
}

void NgpFlashmastaCartridgeWidget::slot_selected(int old_slot_id, int new_slot_id)
{
  (void) old_slot_id;
  (void) new_slot_id;
  if (FlashMasta::get_instance()->get_selected_device() != (int) m_device_id) return;
  update_enabled_actions();
}

void NgpFlashmastaCartridgeWidget::update_enabled_actions()
{
  FlashMasta* app = FlashMasta::get_instance();
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

void NgpFlashmastaCartridgeWidget::on_slotsComboBox_currentIndexChanged(int index)
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
  FlashMasta::get_instance()->setSelectedSlot(index - 1);
}
