#include "ngp_linkmasta_detail_widget.h"
#include "ui_ngp_linkmasta_detail_widget.h"
#include "../worker/ngp_lm_cartridge_polling_worker.h"

NgpLinkmastaDetailWidget::NgpLinkmastaDetailWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpLinkmastaDetailWidget), m_device_id(device_id), m_pooling_thread(nullptr)
{
  ui->setupUi(this);
  m_default_widget = ui->verticalLayout->itemAt(1)->widget();
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
  connect(worker, SIGNAL(cartridge_removed()), this, SLOT(on_cartridge_removed()));
  connect(worker, SIGNAL(cartridge_inserted()), this, SLOT(on_cartridge_inserted()));
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



void NgpLinkmastaDetailWidget::on_cartridge_removed()
{
  ui->noCartridgeLabel->setText("No cartridge detected");
}

void NgpLinkmastaDetailWidget::on_cartridge_inserted()
{
  ui->noCartridgeLabel->setText("Cartridge detected");
}


