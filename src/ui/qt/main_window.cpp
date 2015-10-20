#include "main_window.h"
#include "ui_main_window.h"

#include <vector>
#include <QString>
#include <QLayout>
#include <string>
#include <QMessageBox>
#include "flash_masta.h"
#include "device_manager.h"
#include "device_info_widget.h"
#include "detail/ngp_linkmasta_detail_widget.h"
#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"
#include "linkmasta_device/linkmasta_device.h"
#include "task/ngp_cartridge_backup_task.h"
#include "task/ngp_cartridge_verify_task.h"
#include "task/ngp_cartridge_flash_task.h"
#include "task/ngp_cartridge_backup_save_task.h"
#include "task/ngp_cartridge_restore_save_task.h"
#include "task/ws_cartridge_backup_task.h"
#include "task/ws_cartridge_verify_task.h"
#include "task/ws_cartridge_flash_task.h"
#include "task/ws_cartridge_backup_save_task.h"
#include "task/ws_cartridge_restore_save_task.h"
#include "libusb-1.0/libusb.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) 
  : QMainWindow(parent), ui(new Ui::MainWindow),
    m_target_system(system_type::UNKNOWN), m_timer(this), m_device_ids(),
    m_device_info_widgets(), m_default_widget(nullptr)
{
  ui->setupUi(this);

  // Hide toolbar if on windows
#ifdef OS_WINDOWS
  ui->mainToolBar->hide();
#endif
  
  // Start the automatic list refresh timer
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(refreshDeviceList_timeout()));
  m_timer.start(10);
}

MainWindow::~MainWindow()
{
  delete ui;
}



cartridge* build_cartridge_for_device(int id)
{
  linkmasta_device* linkmasta;
  cartridge* cart;
  
  linkmasta = FlashMasta::get_instance()->get_device_manager()->get_linkmasta_device(id);
  if (linkmasta == nullptr)
  {
    return nullptr;
  }
  
  switch (FlashMasta::get_instance()->get_device_manager()->get_product_id(id))
  {
  case 0x4256:       // NGP (new flashmasta)
  case 0x4178:       // NGP (linkmasta)
    cart = new ngp_cartridge(linkmasta);
    break;
    
  case 0x4252:       // WS
    cart = new ws_cartridge(linkmasta);
    break;
  }
  
  cart->init();
  return cart;
}



void MainWindow::on_actionBackupROM_triggered()
{
  int index = ui->deviceListWidget->currentRow();
  
  if (index >= 0)
  {
    index = m_device_ids[index];
  }
  else
  {
    return;
  }
  
  // Mark device as in-use, wait until available
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(index));
  
  cartridge* cart = build_cartridge_for_device(index);
  cart->init();
  
  try
  {
    switch (cart->system())
    {
    case system_type::NEO_GEO_POCKET:
      NgpCartridgeBackupTask(this, cart).go();
      break;
      
    case system_type::WONDERSWAN:
      WsCartridgeBackupTask(this, cart).go();
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  delete cart;
  FlashMasta::get_instance()->get_device_manager()->release_device(index);
}

void MainWindow::on_actionRestoreROM_triggered()
{
  int index = ui->deviceListWidget->currentRow();
  
  if (index >= 0)
  {
    index = m_device_ids[index];
  }
  else
  {
    return;
  }
  
  // Mark device as in-use, wait until available
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(index));
  
  cartridge* cart = build_cartridge_for_device(index);
  cart->init();
  
  try
  {
    switch (cart->system())
    {
    case system_type::NEO_GEO_POCKET:
      NgpCartridgeFlashTask(this, cart).go();
      break;
      
    case system_type::WONDERSWAN:
      WsCartridgeFlashTask(this, cart).go();
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  delete cart;
  FlashMasta::get_instance()->get_device_manager()->release_device(index);
}

void MainWindow::on_actionVerifyROM_triggered()
{
  int index = ui->deviceListWidget->currentRow();
  
  if (index >= 0)
  {
    index = m_device_ids[index];
  }
  else
  {
    return;
  }
  
  // Mark device as in-use, wait until available
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(index));
  
  cartridge* cart = build_cartridge_for_device(index);
  cart->init();
  
  try
  {
    switch (cart->system())
    {
    case system_type::NEO_GEO_POCKET:
      NgpCartridgeVerifyTask(this, cart).go();
      break;
      
    case system_type::WONDERSWAN:
      WsCartridgeVerifyTask(this, cart).go();
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  delete cart;
  FlashMasta::get_instance()->get_device_manager()->release_device(index);
}

void MainWindow::on_actionBackupSave_triggered()
{
  int index = ui->deviceListWidget->currentRow();
  
  if (index >= 0)
  {
    index = m_device_ids[index];
  }
  else
  {
    return;
  }
  
  // Mark device as in-use, wait until available
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(index));
  
  cartridge* cart = build_cartridge_for_device(index);
  cart->init();
  
  try
  {
    switch (cart->system())
    {
    case system_type::NEO_GEO_POCKET:
      NgpCartridgeBackupSaveTask(this, cart).go();
      break;
      
    case system_type::WONDERSWAN:
      WsCartridgeBackupSaveTask(this, cart).go();
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  delete cart;
  FlashMasta::get_instance()->get_device_manager()->release_device(index);
}

void MainWindow::on_actionRestoreSave_triggered()
{
  int index = ui->deviceListWidget->currentRow();
  
  if (index >= 0)
  {
    index = m_device_ids[index];
  }
  else
  {
    return;
  }
  
  // Mark device as in-use, wait until available
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(index));
  
  cartridge* cart = build_cartridge_for_device(index);
  cart->init();
  
  try
  {
    switch (cart->system())
    {
    case system_type::NEO_GEO_POCKET:
      NgpCartridgeRestoreSaveTask(this, cart).go();
      break;
      
    case system_type::WONDERSWAN:
      WsCartridgeRestoreSaveTask(this, cart).go();
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  delete cart;
  FlashMasta::get_instance()->get_device_manager()->release_device(index);
}

void MainWindow::refreshDeviceList_timeout()
{
  vector<unsigned int> devices;
  
  if (FlashMasta::get_instance()->get_device_manager()->try_get_connected_devices(devices))
  {
    // Compare known devices with new devices and update list
    for (int i = 0, j = 0; i < m_device_ids.size() || j < devices.size();)
    {
      if (i < m_device_ids.size() && j < devices.size())
      {
        if (m_device_ids[i] < devices[j])
        {
          // Device has been disconnected
          delete ui->deviceListWidget->takeItem(i);
          auto it = m_device_info_widgets.find(m_device_ids[i]);
          if (it != m_device_info_widgets.end())
          {
            delete it->second;
            m_device_info_widgets.erase(it);
          }
          m_device_ids.erase(m_device_ids.begin() + i);
        }
        else if (m_device_ids[i] > devices[j])
        {
          // Devices was skipped/added in the middle
          QListWidgetItem *item = new QListWidgetItem(QString(FlashMasta::get_instance()->get_device_manager()->get_product_string(devices[j]).c_str()));
          auto size = item->sizeHint();
          size.setHeight(40);
          item->setSizeHint(size);
          ui->deviceListWidget->insertItem(i, item);
          m_device_ids.insert(m_device_ids.begin() + i, devices[j]);
          
          //auto widget = new DeviceInfoWidget(ui->scrollAreaWidgetContents->parentWidget());
          auto widget = new NgpLinkmastaDetailWidget(devices[j], ui->scrollAreaWidgetContents->parentWidget());
          widget->start_polling();
          
          m_device_info_widgets[devices[j]] = widget;
          //widget->set_device_id(devices[j]);
          widget->hide();
          
          ++i;
          ++j;
        }
        else
        {
          ++i;
          ++j;
        }
      }
      else if (i < m_device_ids.size())
      {
        // Device was disconnected
        delete ui->deviceListWidget->takeItem(i);
        auto it = m_device_info_widgets.find(m_device_ids[i]);
        if (it != m_device_info_widgets.end())
        {
          delete it->second;
          m_device_info_widgets.erase(it);
        }
        m_device_ids.erase(m_device_ids.begin() + i);
      }
      else if (j < devices.size())
      {
        // Device was connected
        QListWidgetItem *item = new QListWidgetItem(QString(FlashMasta::get_instance()->get_device_manager()->get_product_string(devices[j]).c_str()));
        auto size = item->sizeHint();
        size.setHeight(40);
        item->setSizeHint(size);
        ui->deviceListWidget->insertItem(i, item);
        m_device_ids.insert(m_device_ids.begin() + i, devices[j]);
        
        //auto widget = new DeviceInfoWidget(ui->scrollAreaWidgetContents->parentWidget());
        auto widget = new NgpLinkmastaDetailWidget(devices[j], ui->scrollAreaWidgetContents->parentWidget());
        widget->start_polling();
        
        m_device_info_widgets[devices[j]] = widget;
        //widget->set_device_id(devices[j]);
        widget->hide();
        
        ++i;
        ++j;
      }
    }
  }
  
  m_timer.start(10);
}

void MainWindow::on_deviceListWidget_currentRowChanged(int currentRow)
{
  if (m_default_widget == nullptr)
  {
    m_default_widget = ui->scrollAreaWidgetContents;
  }
  
  if (currentRow >= 0)
  {
    ui->scrollAreaWidgetContents->hide();
    ui->scrollAreaWidgetContents = m_device_info_widgets[m_device_ids[currentRow]];
    ui->scrollAreaWidgetContents->show();
  }
  else
  {
    ui->scrollAreaWidgetContents = m_default_widget;
  }
}


