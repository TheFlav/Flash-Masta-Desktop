#include "ngp_lm_detail_widget.h"
#include "ui_ngp_linkmasta_detail_widget.h"
#include "../worker/lm_cartridge_polling_worker.h"
#include "cartridge_widget.h"

NgpLinkmastaDetailWidget::NgpLinkmastaDetailWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpLinkmastaDetailWidget), m_device_id(device_id), m_cartridge_widget(nullptr),
  m_pooling_thread(nullptr)
{
  ui->setupUi(this);
  m_default_widget = ui->contentWidget;
  
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedDeviceChanged(int,int)), this, SLOT(selectedDeviceChanged(int,int)));
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedSlotChanged(int,int)), this, SLOT(selectedSlotChanged(int,int)));
}

NgpLinkmastaDetailWidget::~NgpLinkmastaDetailWidget()
{
  stopPolling();
  delete ui;
}



void NgpLinkmastaDetailWidget::startPolling()
{
  if (m_pooling_thread != nullptr) return;
  
  // Spin up a new thread with a worker to periodically pool linkmasta for cartridge
  m_pooling_thread = new QThread();
  auto worker = new LmCartridgePollingWorker(m_device_id);
  
  worker->moveToThread(m_pooling_thread);
  connect(worker, SIGNAL(cartridgeRemoved()), this, SLOT(cartridgeRemoved()));
  connect(worker, SIGNAL(cartridgeInserted()), this, SLOT(cartridgeInserted()));
  connect(m_pooling_thread, SIGNAL(started()), worker, SLOT(start()));
  connect(m_pooling_thread, SIGNAL(finished()), worker, SLOT(stop()));
  connect(m_pooling_thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  
  m_pooling_thread->start();
}

void NgpLinkmastaDetailWidget::stopPolling()
{
  if (m_pooling_thread == nullptr) return;
  
  m_pooling_thread->quit();
  m_pooling_thread->deleteLater();
  m_pooling_thread = nullptr;
}



void NgpLinkmastaDetailWidget::disableActions()
{
  FlashMastaApp* app = FlashMastaApp::getInstance();
  app->setGameBackupEnabled(false);
  app->setGameFlashEnabled(false);
  app->setGameVerifyEnabled(false);
  app->setSaveBackupEnabled(false);
  app->setSaveRestoreEnabled(false);
  app->setSaveVerifyEnabled(false);
}



void NgpLinkmastaDetailWidget::cartridgeRemoved()
{
  m_default_widget->show();
  
  if (m_cartridge_widget != nullptr)
  {
    m_cartridge_widget->hide();
    delete m_cartridge_widget;
    m_cartridge_widget = nullptr;
  }
  
  FlashMastaApp::getInstance()->setSelectedSlot(-1);
}

void NgpLinkmastaDetailWidget::cartridgeInserted()
{
  if (m_cartridge_widget != nullptr)
  {
    m_cartridge_widget->hide();
    delete m_cartridge_widget;
  }
  
  m_cartridge_widget = new CartridgeWidget(m_device_id, ui->verticalLayout->widget());
  ui->verticalLayout->addWidget(m_cartridge_widget, 1);
  m_cartridge_widget->show();
  m_default_widget->hide();
}

void NgpLinkmastaDetailWidget::selectedDeviceChanged(int old_device, int new_device)
{
  (void) old_device;
  if (new_device == (int) m_device_id && m_cartridge_widget == nullptr)
  {
    disableActions();
  }
}

void NgpLinkmastaDetailWidget::selectedSlotChanged(int old_slot, int new_slot)
{
  (void) old_slot;
  (void) new_slot;
  if (FlashMastaApp::getInstance()->getSelectedDevice() == (int) m_device_id && m_cartridge_widget == nullptr)
  {
    disableActions();
  }
}


