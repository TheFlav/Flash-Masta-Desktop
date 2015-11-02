#include "ngp_linkmasta_detail_widget.h"
#include "ui_ngp_linkmasta_detail_widget.h"
#include "../worker/ngp_lm_cartridge_polling_worker.h"
#include "ngp_flashmasta_cartridge_widget.h"

NgpLinkmastaDetailWidget::NgpLinkmastaDetailWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpLinkmastaDetailWidget), m_device_id(device_id), m_cartridge_widget(nullptr),
  m_pooling_thread(nullptr)
{
  ui->setupUi(this);
  m_default_widget = ui->contentWidget;
}

NgpLinkmastaDetailWidget::~NgpLinkmastaDetailWidget()
{
  stop_polling();
  delete ui;
}



void NgpLinkmastaDetailWidget::start_polling()
{
  if (m_pooling_thread != nullptr) return;
  
  // Spin up a new thread with a worker to periodically pool linkmasta for cartridge
  m_pooling_thread = new QThread();
  auto worker = new NgpLmCartridgePollingWorker(m_device_id);
  
  worker->moveToThread(m_pooling_thread);
  connect(worker, SIGNAL(cartridge_removed()), this, SLOT(cartridge_removed()));
  connect(worker, SIGNAL(cartridge_inserted()), this, SLOT(cartridge_inserted()));
  connect(m_pooling_thread, SIGNAL(started()), worker, SLOT(start()));
  connect(m_pooling_thread, SIGNAL(finished()), worker, SLOT(stop()));
  connect(m_pooling_thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  
  m_pooling_thread->start();
}

void NgpLinkmastaDetailWidget::stop_polling()
{
  if (m_pooling_thread == nullptr) return;
  
  m_pooling_thread->quit();
  m_pooling_thread->deleteLater();
  m_pooling_thread = nullptr;
}



void NgpLinkmastaDetailWidget::cartridge_removed()
{
  m_default_widget->show();
  
  if (m_cartridge_widget != nullptr)
  {
    m_cartridge_widget->hide();
    delete m_cartridge_widget;
    m_cartridge_widget = nullptr;
  }
  
  FlashMasta::get_instance()->setSelectedSlot(-1);
}

void NgpLinkmastaDetailWidget::cartridge_inserted()
{
  if (m_cartridge_widget != nullptr)
  {
    m_cartridge_widget->hide();
    delete m_cartridge_widget;
  }
  
  m_cartridge_widget = new NgpFlashmastaCartridgeWidget(m_device_id, ui->verticalLayout->widget());
  ui->verticalLayout->addWidget(m_cartridge_widget, 1);
  m_cartridge_widget->show();
  m_default_widget->hide();
}


