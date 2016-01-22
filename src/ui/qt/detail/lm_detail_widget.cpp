#include "lm_detail_widget.h"
#include "ui_lm_detail_widget.h"

#include "linkmasta/linkmasta_device.h"

#include "cartridge_widget.h"
#include "device/device_manager.h"
#include "../worker/lm_cartridge_polling_worker.h"

LmDetailWidget::LmDetailWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::LmDetailWidget), m_device_id(device_id), m_cartridge_widget(nullptr),
  m_pooling_thread(nullptr)
{
  ui->setupUi(this);
  m_default_widget = ui->contentWidget;
  
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedDeviceChanged(int,int)), this, SLOT(selectedDeviceChanged(int,int)));
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedSlotChanged(int,int)), this, SLOT(selectedSlotChanged(int,int)));
  
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(device_id));
  linkmasta_device* linkmasta = FlashMastaApp::getInstance()->getDeviceManager()->getLinkmastaDevice(device_id);
  linkmasta->open();
  std::string ver = linkmasta->firmware_version();
  linkmasta->close();
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(device_id);
  
  if (linkmasta->is_integrated_with_cartridge())
  {
    cartridgeInserted();
  }
  else
  {
    startPolling();
  }
  
  // Display name of device
  QString device_name = "";
  switch (linkmasta->system())
  {
  default:
  case LINKMASTA_UNKNOWN:
    device_name = "Unknown Device";
    break;
    
  case LINKMASTA_NEO_GEO_POCKET:
    if (linkmasta->is_integrated_with_cartridge())
    {
      device_name = "Neo Geo USB Flash Masta";
    }
    else
    {
      device_name = "Neo Geo Link Masta";
    }
    break;
    
  case LINKMASTA_WONDERSWAN:
    device_name = "Wonderswan Flash Masta";
    break;
  }
  ui->deviceNameLabel->setText(device_name);
  
  // Display device firmware version
  QString device_version = "v";
  device_version += QString(ver.c_str());
  ui->deviceFirmwareVersionLabel->setText(device_version);
}

LmDetailWidget::~LmDetailWidget()
{
  stopPolling();
  delete ui;
}



void LmDetailWidget::startPolling()
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

void LmDetailWidget::stopPolling()
{
  if (m_pooling_thread == nullptr) return;
  
  m_pooling_thread->quit();
  m_pooling_thread->deleteLater();
  m_pooling_thread = nullptr;
}



void LmDetailWidget::disableActions()
{
  FlashMastaApp* app = FlashMastaApp::getInstance();
  app->setGameBackupEnabled(false);
  app->setGameFlashEnabled(false);
  app->setGameVerifyEnabled(false);
  app->setSaveBackupEnabled(false);
  app->setSaveRestoreEnabled(false);
  app->setSaveVerifyEnabled(false);
}



void LmDetailWidget::cartridgeRemoved()
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

void LmDetailWidget::cartridgeInserted()
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

void LmDetailWidget::selectedDeviceChanged(int old_device, int new_device)
{
  (void) old_device;
  if (new_device == (int) m_device_id && m_cartridge_widget == nullptr)
  {
    disableActions();
  }
}

void LmDetailWidget::selectedSlotChanged(int old_slot, int new_slot)
{
  (void) old_slot;
  (void) new_slot;
  if (FlashMastaApp::getInstance()->getSelectedDevice() == (int) m_device_id && m_cartridge_widget == nullptr)
  {
    disableActions();
  }
}


