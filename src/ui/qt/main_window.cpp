#include "main_window.h"
#include "ui_main_window.h"

#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <QString>
#include <QLayout>
#include <string>
#include <QMessageBox>
#include "flash_masta_app.h"
#include "device_manager.h"
#include "detail/ngp_lm_detail_widget.h"
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


#define PRE_ACTION \
  int device_index = FlashMastaApp::getInstance()->getSelectedDevice();\
  cartridge* cart = (device_index != -1 ? buildCartridgeForDevice(device_index) : nullptr);\
  \
  if (cart == nullptr)\
  {\
    QMessageBox msgBox(this);\
    msgBox.setText("Please select a Flash Masta and a game slot.");\
    msgBox.exec();\
    return;\
  }\
  \
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(device_index));

#define POST_ACTION \
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(device_index);\
  delete cart;



MainWindow::MainWindow(QWidget *parent) 
  : QMainWindow(parent), ui(new Ui::MainWindow),
    m_target_system(system_type::SYSTEM_UNKNOWN), m_timer(this), m_device_ids(),
    m_device_detail_widgets(), m_prompt_no_devices(nullptr),
    m_current_widget(nullptr)
{
  // Set up UI
  ui->setupUi(this);
  m_prompt_no_devices = ui->promptNoDevices;
  
  // Remove blue glow aroudn QListView on Macs
  ui->deviceListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
  
  // connect ui to actions
  FlashMastaApp* app = FlashMastaApp::getInstance();
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
  
  // Refresh action states
  app->setSelectedDevice(app->getSelectedDevice());
  app->setSelectedSlot(app->getSelectedSlot());
  
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



cartridge* MainWindow::buildCartridgeForDevice(int id)
{
  linkmasta_device* linkmasta;
  cartridge* cart;
  
  linkmasta = FlashMastaApp::getInstance()->getDeviceManager()->getLinkmastaDevice(id);
  if (linkmasta == nullptr)
  {
    return nullptr;
  }
  
  switch (FlashMastaApp::getInstance()->getDeviceManager()->getProductId(id))
  {
  case 0x4256:       // NGP (new flashmasta)
  case 0x4178:       // NGP (linkmasta)
    cart = new ngp_cartridge(linkmasta);
    break;
    
  case 0x4252:       // WS
    cart = new ws_cartridge(linkmasta);
    break;
  }
  
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(id));
  cart->init();
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(id);
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

void MainWindow::triggerActionBackupGame()
{
  PRE_ACTION
  
  try
  {
    switch (cart->system())
    {
    case system_type::SYSTEM_NEO_GEO_POCKET:
      NgpCartridgeBackupTask(this, cart, FlashMastaApp::getInstance()->getSelectedSlot()).go();
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
      NgpCartridgeFlashTask(this, cart, FlashMastaApp::getInstance()->getSelectedSlot()).go();
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
      NgpCartridgeVerifyTask(this, cart, FlashMastaApp::getInstance()->getSelectedSlot()).go();
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
      NgpCartridgeBackupSaveTask(this, cart, FlashMastaApp::getInstance()->getSelectedSlot()).go();
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
      NgpCartridgeRestoreSaveTask(this, cart, FlashMastaApp::getInstance()->getSelectedSlot()).go();
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
  vector<unsigned int> connected_devices;
  
  set<unsigned int> known_devices;    // devices we knew about previously
  set<unsigned int> current_devices;  // devices that are currently connected
  set<unsigned int> new_devices;      // newly connected devices
  set<unsigned int> removed_devices;  // recently removed devices
  
  if (FlashMastaApp::getInstance()->getDeviceManager()->tryGetConnectedDevices(connected_devices))
  {
    // new row selection after updating list
    int selection = -1;
    
    // Construct sets. Use STL algorithms to generate set of removed devices and
    // set of newly connected devices.
    for (auto device_id : m_device_ids) known_devices.insert(device_id);
    for (auto device_id : connected_devices) current_devices.insert(device_id);
    set_difference(current_devices.begin(), current_devices.end(),
                   known_devices.begin(), known_devices.end(),
                   inserter(new_devices, new_devices.end()));
    set_difference(known_devices.begin(), known_devices.end(),
                   current_devices.begin(), current_devices.end(),
                   inserter(removed_devices, removed_devices.end()));
    
    // Handle disconnected devices
    for (unsigned int i = 0; i < m_device_ids.size();)
    {
      auto device_id = m_device_ids[i];
      auto item = ui->deviceListWidget->item(i);
      if (item->isSelected())
      {
        selection = i;
      }
      
      if (removed_devices.find(device_id) != removed_devices.end())
      {
        // Remove listing from device list
        if (item->isSelected()) m_current_widget = nullptr;
        ui->deviceListWidget->takeItem(i);
        delete item;
        
        // Find and delete device's widget
        auto it = m_device_detail_widgets.find(m_device_ids[i]);
        if (it != m_device_detail_widgets.end())
        {
          delete it->second;
          m_device_detail_widgets.erase(it);
        }
        
        // Forget device id
        m_device_ids.erase(m_device_ids.begin() + i);
      }
      else
      {
        i++;
      }
    }
    
    // Handle newly connected devices
    for (auto device_id : new_devices)
    {
      // Instantiate a new item and append it to the list widget
      QListWidgetItem *item = new QListWidgetItem(QString(FlashMastaApp::getInstance()->getDeviceManager()->getProductString(device_id).c_str()));
      auto size = item->sizeHint();
      size.setHeight(40);
      item->setSizeHint(size);
      ui->deviceListWidget->insertItem(ui->deviceListWidget->count(), item);
      
      // Remember this device id
      m_device_ids.push_back(device_id);
      
      // Instantiate widget object for connected device
      auto widget = new NgpLinkmastaDetailWidget(device_id, ui->scrollAreaWidgetContents);
      m_device_detail_widgets[device_id] = widget;
      widget->hide();
      ui->scrollAreaWidgetContents->layout()->addWidget(widget);
      widget->startPolling();
    }
    
    // Update selection if selected device was disconnected
    if (selection >= ui->deviceListWidget->count())
    {
      selection = ui->deviceListWidget->count() - 1;
    }
    if (selection < 0 && ui->deviceListWidget->count() > 0)
    {
      selection = 0;
    }
    
    // Select device in list and act as if user selected it
    if (selection < 0 || !ui->deviceListWidget->item(selection)->isSelected())
    {
      if (selection >= 0)
      {
        ui->deviceListWidget->item(selection)->setSelected(true);
      }
      on_deviceListWidget_currentRowChanged(selection);
    }
  }
  
  m_timer.start(10);
}



// private slots:

void MainWindow::on_deviceListWidget_currentRowChanged(int currentRow)
{
  if (m_current_widget != nullptr)
  {
    m_current_widget->hide();
    m_current_widget = nullptr;
  }
  else
  {
    m_prompt_no_devices->hide();
  }
  
  if (currentRow >= 0)
  {
    m_current_widget = m_device_detail_widgets[m_device_ids[currentRow]];
    m_current_widget->show();
    FlashMastaApp::getInstance()->setSelectedDevice(m_device_ids[currentRow]);
  }
  else
  {
    FlashMastaApp::getInstance()->setSelectedDevice(-1);
  }
}


