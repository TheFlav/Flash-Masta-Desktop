#include "main_window.h"
#include "ui_main_window.h"

#include <vector>
#include <QString>
#include <QLayout>
#include <string>
#include <QMessageBox>
#include "flash_masta.h"
#include "device_manager.h"
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
    m_target_system(system_type::SYSTEM_UNKNOWN), m_timer(this), m_device_ids(),
    m_device_detail_widgets(), m_default_widget(nullptr), m_current_widget(nullptr)
{
  ui->setupUi(this);
  
  // connect ui to actions
  FlashMasta* app = FlashMasta::get_instance();
  connect(ui->actionBackupROM, SIGNAL(triggered(bool)), this, SLOT(triggerActionBackupGame()));
  connect(ui->actionRestoreROM, SIGNAL(triggered(bool)), this, SLOT(triggerActionFlashGame()));
  connect(ui->actionVerifyROM, SIGNAL(triggered(bool)), this, SLOT(triggerActionVerifyGame()));
  connect(ui->actionBackupSave, SIGNAL(triggered(bool)), this, SLOT(triggerActionBackupSave()));
  connect(ui->actionRestoreSave, SIGNAL(triggered(bool)), this, SLOT(triggerActionRestoreSave()));
  connect(ui->actionVerifySave, SIGNAL(triggered(bool)), this, SLOT(triggerActionVerifySave()));
  connect(app, SIGNAL(gameBackupEnabledChanged(bool)), this, SLOT(setGameBackupEnabled(bool)));
  connect(app, SIGNAL(gameFlashEnabledChanged(bool)), this, SLOT(setGameFlashEnabled(bool)));
  connect(app, SIGNAL(gameVerifyEnabledChanged(bool)), this, SLOT(setGameVerifyEnabled(bool)));
  connect(app, SIGNAL(saveBackupEnabledChanged(bool)), this, SLOT(setSaveBackupEnabled(bool)));
  connect(app, SIGNAL(saveRestoreEnabledChanged(bool)), this, SLOT(setSaveRestoreEnabled(bool)));
  connect(app, SIGNAL(saveVerifyEnabledChanged(bool)), this, SLOT(setSaveVerifyEnabled(bool)));
  
  // Disable all actions
  setGameBackupEnabled(false);
  setGameFlashEnabled(false);
  setGameVerifyEnabled(false);
  setSaveBackupEnabled(false);
  setSaveRestoreEnabled(false);
  setSaveVerifyEnabled(false);
  
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



cartridge* MainWindow::build_cartridge_for_device(int id)
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
  
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(id));
  cart->init();
  FlashMasta::get_instance()->get_device_manager()->release_device(id);
  return cart;
}



// public slots:

void MainWindow::setGameBackupEnabled(bool enabled)
{
  ui->actionBackupROM->setEnabled(enabled);
}

void MainWindow::setGameFlashEnabled(bool enabled)
{
  ui->actionRestoreROM->setEnabled(enabled);
}

void MainWindow::setGameVerifyEnabled(bool enabled)
{
  ui->actionVerifyROM->setEnabled(enabled);
}

void MainWindow::setSaveBackupEnabled(bool enabled)
{
  ui->actionBackupSave->setEnabled(enabled);
}

void MainWindow::setSaveRestoreEnabled(bool enabled)
{
  ui->actionRestoreSave->setEnabled(enabled);
}

void MainWindow::setSaveVerifyEnabled(bool enabled)
{
  ui->actionVerifySave->setEnabled(enabled);
}



// private slots:

#define PRE_ACTION \
  int device_index = FlashMasta::get_instance()->get_selected_device();\
  int slot_index = FlashMasta::get_instance()->get_selected_slot();\
  cartridge* cart = (device_index != -1 && slot_index != -1 ? build_cartridge_for_device(device_index) : nullptr);\
  \
  if (cart == nullptr)\
  {\
    QMessageBox msgBox(this);\
    msgBox.setText("Please select a Flash Masta and a game slot.");\
    msgBox.exec();\
    return;\
  }\
  \
  /* Mark device as in-use, wait until available */\
  while (!FlashMasta::get_instance()->get_device_manager()->claim_device(device_index));

#define POST_ACTION \
  FlashMasta::get_instance()->get_device_manager()->release_device(device_index);\
  delete cart;


void MainWindow::triggerActionBackupGame()
{
  PRE_ACTION
  
  try
  {
    switch (cart->system())
    {
    case system_type::SYSTEM_NEO_GEO_POCKET:
      NgpCartridgeBackupTask(this, cart).go();
      break;
      
    case system_type::SYSTEM_WONDERSWAN:
      WsCartridgeBackupTask(this, cart).go();
      break;
      
    default:
      // Too bad, so sad
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  POST_ACTION
}

void MainWindow::triggerActionFlashGame()
{
  PRE_ACTION
  
  try
  {
    switch (cart->system())
    {
    case system_type::SYSTEM_NEO_GEO_POCKET:
      NgpCartridgeFlashTask(this, cart).go();
      break;
      
    case system_type::SYSTEM_WONDERSWAN:
      WsCartridgeFlashTask(this, cart).go();
      break;
      
    default:
      // Too bad, so sad
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  POST_ACTION
}

void MainWindow::triggerActionVerifyGame()
{
  PRE_ACTION
  
  try
  {
    switch (cart->system())
    {
    case system_type::SYSTEM_NEO_GEO_POCKET:
      NgpCartridgeVerifyTask(this, cart).go();
      break;
      
    case system_type::SYSTEM_WONDERSWAN:
      WsCartridgeVerifyTask(this, cart).go();
      break;
      
    default:
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  POST_ACTION
}

void MainWindow::triggerActionBackupSave()
{
  PRE_ACTION
  
  try
  {
    switch (cart->system())
    {
    case system_type::SYSTEM_NEO_GEO_POCKET:
      NgpCartridgeBackupSaveTask(this, cart).go();
      break;
      
    case system_type::SYSTEM_WONDERSWAN:
      WsCartridgeBackupSaveTask(this, cart).go();
      break;
      
    default:
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  POST_ACTION
}

void MainWindow::triggerActionRestoreSave()
{
  PRE_ACTION
  
  try
  {
    switch (cart->system())
    {
    case system_type::SYSTEM_NEO_GEO_POCKET:
      NgpCartridgeRestoreSaveTask(this, cart).go();
      break;
      
    case system_type::SYSTEM_WONDERSWAN:
      WsCartridgeRestoreSaveTask(this, cart).go();
      break;
      
    default:
      break;
    }
  }
  catch (std::runtime_error& ex)
  {
    QMessageBox msgBox(this);
    msgBox.setText(ex.what());
    msgBox.exec();    
  }
  
  POST_ACTION
}

void MainWindow::triggerActionVerifySave()
{
  // TODO
}

void MainWindow::refreshDeviceList_timeout()
{
  vector<unsigned int> devices;
  
  if (FlashMasta::get_instance()->get_device_manager()->try_get_connected_devices(devices))
  {
    // Compare known devices with new devices and update list
    for (unsigned int i = 0, j = 0; i < m_device_ids.size() || j < devices.size();)
    {
      if (i < m_device_ids.size() && j < devices.size())
      {
        if (m_device_ids[i] < devices[j])
        {
          // Device has been disconnected
          delete ui->deviceListWidget->takeItem(i);
          auto it = m_device_detail_widgets.find(m_device_ids[i]);
          if (it != m_device_detail_widgets.end())
          {
            delete it->second;
            m_device_detail_widgets.erase(it);
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
          auto widget = new NgpLinkmastaDetailWidget(devices[j], ui->scrollAreaWidgetContents);
          m_device_detail_widgets[devices[j]] = widget;
          //widget->set_device_id(devices[j]);
          widget->hide();
          ui->scrollAreaWidgetContents->layout()->addWidget(widget);
          
          widget->start_polling();
          
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
        auto it = m_device_detail_widgets.find(m_device_ids[i]);
        if (it != m_device_detail_widgets.end())
        {
          delete it->second;
          m_device_detail_widgets.erase(it);
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
        auto widget = new NgpLinkmastaDetailWidget(devices[j], ui->scrollAreaWidgetContents);
        m_device_detail_widgets[devices[j]] = widget;
        //widget->set_device_id(devices[j]);
        widget->hide();
        ui->scrollAreaWidgetContents->layout()->addWidget(widget);
        
        widget->start_polling();
        
        ++i;
        ++j;
      }
    }
  }
  
  m_timer.start(10);
}

void MainWindow::on_deviceListWidget_currentRowChanged(int currentRow)
{
  if (m_current_widget != nullptr)
  {
    m_current_widget->hide();
    m_current_widget = nullptr;
  }
  
  if (currentRow >= 0)
  {
    m_current_widget = m_device_detail_widgets[m_device_ids[currentRow]];
    m_current_widget->show();
    FlashMasta::get_instance()->setSelectedDevice(m_device_ids[currentRow]);
  }
  else
  {
    FlashMasta::get_instance()->setSelectedDevice(-1);
  }
}


