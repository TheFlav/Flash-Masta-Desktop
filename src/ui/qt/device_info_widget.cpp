#include "device_info_widget.h"
#include "ui_device_info_widget.h"

#include "flash_masta.h"
#include "device_manager.h"
#include "linkmasta_device/linkmasta_device.h"
#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"
#include <QThread>
#include <QTimer>


worker::worker(unsigned int id) : QObject(), m_id(id), product_str(), game_str(), m_full(true) {}
worker::~worker() {}
void worker::process()
{
  if (FlashMasta::get_instance()->get_device_manager()->claim_device(m_id))
  {
  
    try
    {
      if (m_full)
      {
        product_str = FlashMasta::get_instance()->get_device_manager()->get_product_string(m_id);
      }
      linkmasta_device* linkmasta = FlashMasta::get_instance()->get_device_manager()->get_linkmasta_device(m_id);
      cartridge* cart;
      
      switch (FlashMasta::get_instance()->get_device_manager()->get_product_id(m_id))
      {
      case 0x4256:       // NGP (new flashmasta)
        if (!m_full) break;
        
      case 0x4178:       // NGP (linkmasta)
        if (ngp_cartridge::test_for_cartridge(linkmasta))
        {
          cart = new ngp_cartridge(linkmasta);
          cart->init();
          game_str = cart->fetch_game_name(0);
        }
        else
        {
          cart = nullptr;
          game_str = "No cartridge detected";
        }
        break;
        
      case 0x4252:       // WS
        if (!m_full) break;
        
        cart = new ws_cartridge(linkmasta);
        cart->init();
        game_str = cart->fetch_game_name(0);
        break;
      }
      
      if (cart != nullptr) delete cart;
      
    }
    catch (std::exception& ex)
    {
      product_str = "An error occured";
      m_full = true;
    }
    
    m_full = false;
    
    FlashMasta::get_instance()->get_device_manager()->release_device(m_id);    
  }
  
  QString s1, s2;
  s1 = QString(product_str.c_str());
  s2 = QString(game_str.c_str());
  emit finished(s1, s2);
}



DeviceInfoWidget::DeviceInfoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::DeviceInfoWidget),
  m_thread(new QThread()),
  m_worker(nullptr)
{
  ui->setupUi(this);
}

DeviceInfoWidget::~DeviceInfoWidget()
{
  m_worker->deleteLater();
  m_thread->deleteLater();
  delete ui;
}

void DeviceInfoWidget::set_device_id(unsigned int device_id)
{
  m_device_id = device_id;
  
  // display busy indicator
  ui->deviceString->hide();
  ui->gameString->hide();
  ui->progressBar->show();
  
  if (m_worker != nullptr)
  {
    m_worker->deleteLater();
  }
  m_worker = new worker(device_id);
  m_worker->moveToThread(m_thread);
  connect(m_thread, SIGNAL(started()), m_worker, SLOT(process()));
  connect(m_worker, SIGNAL(finished(QString,QString)), m_thread, SLOT(quit()));
  connect(m_worker, SIGNAL(finished(QString,QString)), this, SLOT(refresh_ui(QString,QString)));
  m_thread->start();
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
  
  QTimer::singleShot(1000, m_thread, SLOT(start()));
}


