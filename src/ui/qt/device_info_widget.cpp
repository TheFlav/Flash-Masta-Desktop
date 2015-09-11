#include "device_info_widget.h"
#include "ui_device_info_widget.h"

#include "flash_masta.h"
#include "device_manager.h"
#include <string>
#include "linkmasta_device/linkmasta_device.h"
#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"
#include <QThread>


worker::worker(unsigned int id) : QObject(), m_id(id) {}
worker::~worker() {}
void worker::process()
{
  std::string product_str = FlashMasta::get_instance()->get_device_manager()->get_product_string(m_id);
  std::string game_str;
  linkmasta_device* linkmasta = FlashMasta::get_instance()->get_device_manager()->get_linkmasta_device(m_id);
  cartridge* cart;
  
  switch (FlashMasta::get_instance()->get_device_manager()->get_product_id(m_id))
  {
  case 0x4178:       // NGP (linkmasta)
  case 0x4256:       // NGP (new flashmasta)
    if (ngp_cartridge::test_for_cartridge(linkmasta))
    {
      game_str = "Game detected";
    }
    else
    {
      game_str = "No cartridge detected";
    }
    break;
    
  case 0x4252:       // WS
    game_str = "Game detected";
    break;
  }
  
  QString s1, s2;
  
  s1 = QString(product_str.c_str());
  s2 = QString(game_str.c_str());
  
  emit finished(s1, s2);
}



DeviceInfoWidget::DeviceInfoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::DeviceInfoWidget)
{
  ui->setupUi(this);
}

DeviceInfoWidget::~DeviceInfoWidget()
{
  delete ui;
}

void DeviceInfoWidget::set_device_id(unsigned int device_id)
{
  m_device_id = device_id;
  
  // display busy indicator
  ui->deviceString->hide();
  ui->gameString->hide();
  ui->progressBar->show();
  
  QThread* thread = new QThread();
  worker* w = new worker(device_id);
  w->moveToThread(thread);
  connect(thread, SIGNAL(started()), w, SLOT(process()));
  connect(w, SIGNAL(finished(QString,QString)), thread, SLOT(quit()));
  connect(w, SIGNAL(finished(QString,QString)), w, SLOT(deleteLater()));
  connect(w, SIGNAL(finished(QString,QString)), this, SLOT(refresh_ui(QString,QString)));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
}



void DeviceInfoWidget::refresh_ui(QString device_name, QString game_name)
{
  m_device_name = device_name;
  m_game_name = game_name;
  
  ui->deviceString->setText(m_device_name);
  ui->gameString->setText(m_game_name);
  
  ui->progressBar->hide();
  ui->deviceString->show();
  ui->gameString->show();
}


