#include "fm_cartridge_slot_widget.h"
#include "ui_fm_cartridge_slot_widget.h"

#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"
#include "games/game_catalog.h"
#include "games/game_descriptor.h"

#include "../device_manager.h"
#include "../flash_masta_app.h"
#include "../main_window.h"



// public:

FmCartridgeSlotWidget::FmCartridgeSlotWidget(int device_id, cartridge* cart, int slot, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FmCartridgeSlotWidget), m_device_id(device_id)
{
  ui->setupUi(this);
  
  if (cart != nullptr && slot != -1)
  {
    buildFromCartridge(cart, slot);
  }
  
  FlashMastaApp* app = FlashMastaApp::getInstance();
  MainWindow* win = app->getMainWindow();
  
  connect(app, SIGNAL(gameBackupEnabledChanged(bool)), this, SLOT(setGameBackupEnabled(bool)));
  connect(app, SIGNAL(gameFlashEnabledChanged(bool)), this, SLOT(setGameFlashEnabled(bool)));
  connect(app, SIGNAL(gameVerifyEnabledChanged(bool)), this, SLOT(setGameVerifyEnabled(bool)));
  connect(app, SIGNAL(saveBackupEnabledChanged(bool)), this, SLOT(setSaveBackupEnabled(bool)));
  connect(app, SIGNAL(saveRestoreEnabledChanged(bool)), this, SLOT(setSaveRestoreEnabled(bool)));
  connect(app, SIGNAL(saveVerifyEnabledChanged(bool)), this, SLOT(setSaveVerifyEnabled(bool)));
  
  connect(ui->slotActionBackupGameButton, SIGNAL(clicked(bool)), win, SLOT(triggerActionBackupGame()));
  connect(ui->slotActionFlashGameButton, SIGNAL(clicked(bool)), win, SLOT(triggerActionFlashGame()));
  connect(ui->slotActionVerifyGameButton, SIGNAL(clicked(bool)), win, SLOT(triggerActionVerifyGame()));
  connect(ui->slotActionBackupSaveButton, SIGNAL(clicked(bool)), win, SLOT(triggerActionBackupSave()));
  connect(ui->slotActionFlashSaveButton, SIGNAL(clicked(bool)), win, SLOT(triggerActionRestoreSave()));
  connect(ui->slotActionVerifySaveButton, SIGNAL(clicked(bool)), win, SLOT(triggerActionVerifySave()));
}

FmCartridgeSlotWidget::~FmCartridgeSlotWidget()
{
  delete ui;
}



void FmCartridgeSlotWidget::buildFromCartridge(cartridge* cart, int slot)
{
  if (cart == nullptr || slot == -1 || (unsigned int) slot >= cart->num_slots()) return;
  
  m_slot = slot;
  
  // Set the slot size
  setSlotSize(cart->slot_size(m_slot));
  
  // Let each cartridge type resolve itself
  switch (cart->system())
  {
  case system_type::SYSTEM_NEO_GEO_POCKET:
    buildFromNgpCartridge((ngp_cartridge*) cart, slot);
    break;
    
  case system_type::SYSTEM_WONDERSWAN:
    buildFromWsCartridge((ws_cartridge*) cart, slot);
    break;
    
  default:
    break;
  }
  
  setGameBackupEnabled(false);
  setGameFlashEnabled(false);
  setGameVerifyEnabled(false);
  setSaveBackupEnabled(false);
  setSaveRestoreEnabled(false);
  setSaveVerifyEnabled(false);
}



// private:

void FmCartridgeSlotWidget::buildFromNgpCartridge(ngp_cartridge* cart, int slot)
{
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(m_device_id));
  const game_descriptor* descriptor = FlashMastaApp::getInstance()->getNeoGeoGameCatalog()->identify_game(cart, slot);
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(m_device_id);
  
  std::string game_name = cart->fetch_game_name(slot);
  if (game_name.empty())
  {
    game_name = "Unknown";
  }
  
  // Set fields based on contents of descriptor
  setSlotGameNameVisible(true);
  setSlotGameName(QString(descriptor != nullptr ? descriptor->name : "Unknown"));
  setSlotGameSizeVisible(false);
  setSlotDeveloperNameVisible(false);
  setSlotCartNameVisible(true);
  setSlotCartName(QString(game_name.c_str()));
  
  // Delete our dynamically allocated descriptor
  if (descriptor != nullptr)
  {
    delete descriptor;
  }
}

void FmCartridgeSlotWidget::buildFromWsCartridge(ws_cartridge* cart, int slot)
{
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(m_device_id));
  const game_descriptor* descriptor = FlashMastaApp::getInstance()->getWonderswanGameCatalog()->identify_game(cart, slot);
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(m_device_id);
  
  // Set fields based on contents of descriptor
  setSlotGameNameVisible(true);
  setSlotGameName(QString(descriptor != nullptr ? descriptor->name : "Unknown"));
  setSlotGameSizeVisible(true);
  setSlotGameSize(cart->get_game_size(slot));
  setSlotDeveloperNameVisible(true);
  setSlotDeveloperName(QString(descriptor != nullptr ? descriptor->developer_name : "Unknown"));
  setSlotCartNameVisible(false);
  
  // Delete our dynamically allocated descriptor
  if (descriptor != nullptr)
  {
    delete descriptor;
  }  
}



// public:

int FmCartridgeSlotWidget::slotNumber() const
{
  return m_slot;
}

unsigned int FmCartridgeSlotWidget::slotSize() const
{
  return m_slot_num_bytes;
}

QString FmCartridgeSlotWidget::slotGameName() const
{
  return m_slot_game_name;
}

unsigned int FmCartridgeSlotWidget::slotGameSize() const
{
  return m_slot_game_bytes;
}

bool FmCartridgeSlotWidget::gameBackupEnabled() const
{
  return m_game_flash_enabled;
}

bool FmCartridgeSlotWidget::gameFlashEnabled() const
{
  return m_game_backup_enabled;
}

bool FmCartridgeSlotWidget::gameVerifyEnabled() const
{
  return m_game_verify_enabled;
}

bool FmCartridgeSlotWidget::saveBackupEnabled() const
{
  return m_save_backup_enabled;
}

bool FmCartridgeSlotWidget::saveRestoreEnabled() const
{
  return m_save_restore_enabled;
}

bool FmCartridgeSlotWidget::saveVerifyEnabled() const
{
  return m_save_verify_enabled;
}



// private:

QString FmCartridgeSlotWidget::stringifyBytesToBits(unsigned int num_bytes, bool reduce)
{
  QString text;
  if (reduce && (num_bytes & 0x7FFFFFF) == 0)
  {
    text = QString::number(num_bytes >> 27) + QString(" Gib");
  }
  else if (reduce && (num_bytes & 0x1FFFF) == 0)
  {
    text = QString::number(num_bytes >> 17) + QString(" Mib");
  }
  else if (reduce && (num_bytes & 0x7F) == 0)
  {
    text = QString::number(num_bytes >> 7) + QString(" Kib");
  }
  else
  {
    text = QString::number(((unsigned long long) num_bytes) << 3) + QString(" b");
  }
  return text;
}



// public slots:

void FmCartridgeSlotWidget::setSlotSize(unsigned int num_bytes)
{
  m_slot_num_bytes = num_bytes;
  
  QString text = stringifyBytesToBits(num_bytes);
  
  ui->slotInfoCapacityOutputLabel->setText(text);
}

void FmCartridgeSlotWidget::setSlotSizeVisible(bool visible)
{
  if (slotSizeVisible() && visible == false)
  {
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoCapacityLabel);
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoCapacityOutputLabel);
  }
  else if (!slotSizeVisible() && visible == true)
  {
    int row = 0;
    ui->slotInfoFormLayout->insertRow(row, ui->slotInfoCapacityLabel, ui->slotInfoCapacityOutputLabel);
  }
  
  ui->slotInfoCapacityLabel->setVisible(visible);
  ui->slotInfoCapacityOutputLabel->setVisible(visible);
}

bool FmCartridgeSlotWidget::slotSizeVisible() const
{
  return !!ui->slotInfoCapacityLabel->isHidden();
}

void FmCartridgeSlotWidget::setSlotGameName(QString name)
{
  m_slot_game_name = name;
  
  ui->slotInfoGameTitleOutputLabel->setText(name);
}

void FmCartridgeSlotWidget::setSlotGameNameVisible(bool visible)
{
  if (slotGameNameVisible() && visible == false)
  {
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoGameTitleLabel);
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoGameTitleOutputLabel);
  }
  else if (!slotGameNameVisible() && visible == true)
  {
    int row = 0;
    if (slotSizeVisible()) row++;
    ui->slotInfoFormLayout->insertRow(row, ui->slotInfoGameTitleLabel, ui->slotInfoGameTitleOutputLabel);
  }
  
  ui->slotInfoGameTitleLabel->setVisible(visible);
  ui->slotInfoGameTitleOutputLabel->setVisible(visible);
}

bool FmCartridgeSlotWidget::slotGameNameVisible() const
{
  return !ui->slotInfoGameTitleLabel->isHidden();
}

void FmCartridgeSlotWidget::setSlotCartName(QString name)
{
  m_slot_cart_name = name;
  
  ui->slotInfoCartNameOutputLabel->setText(m_slot_cart_name);
}

void FmCartridgeSlotWidget::setSlotCartNameVisible(bool visible)
{
  if (slotCartNameVisible() && visible == false)
  {
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoCartNameLabel);
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoCartNameOutputLabel);
  }
  else if (!slotCartNameVisible() && visible == true)
  {
    int row = 0;
    if (slotSizeVisible()) row++;
    if (slotGameNameVisible()) row++;
    ui->slotInfoFormLayout->insertRow(row, ui->slotInfoCartNameLabel, ui->slotInfoCartNameOutputLabel);
  }
  
  ui->slotInfoCartNameLabel->setVisible(visible);
  ui->slotInfoCartNameOutputLabel->setVisible(visible);
}

bool FmCartridgeSlotWidget::slotCartNameVisible() const
{
  return !ui->slotInfoCartNameLabel->isHidden();
}

void FmCartridgeSlotWidget::setSlotDeveloperName(QString name)
{
  m_slot_developer_name = name;
  
  ui->slotInfoDeveloperOutputLabel->setText(m_slot_developer_name);
}

void FmCartridgeSlotWidget::setSlotDeveloperNameVisible(bool visible)
{
  if (slotDeveloperNameVisible() && visible == false)
  {
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoDeveloperLabel);
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoDeveloperOutputLabel);
  }
  else if (!slotDeveloperNameVisible() && visible == true)
  {
    int row = 0;
    if (slotSizeVisible()) row++;
    if (slotGameNameVisible()) row++;
    if (slotCartNameVisible()) row++;
    ui->slotInfoFormLayout->insertRow(row, ui->slotInfoDeveloperLabel, ui->slotInfoDeveloperOutputLabel);
  }
  
  ui->slotInfoDeveloperLabel->setVisible(visible);
  ui->slotInfoDeveloperOutputLabel->setVisible(visible);
}

bool FmCartridgeSlotWidget::slotDeveloperNameVisible() const
{
  return !ui->slotInfoDeveloperLabel->isHidden();
}

void FmCartridgeSlotWidget::setSlotGameSize(unsigned int num_bytes)
{
  m_slot_game_bytes = num_bytes;
  
  QString text = stringifyBytesToBits(num_bytes);
  
  ui->slotInfoGameSizeOutputLabel->setText(text);
}

void FmCartridgeSlotWidget::setSlotGameSizeVisible(bool visible)
{
  if (slotGameSizeVisible() && visible == false)
  {
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoGameSizeLabel);
    ui->slotInfoFormLayout->removeWidget(ui->slotInfoGameSizeOutputLabel);
  }
  else if (!slotGameSizeVisible() && visible == true)
  {
    int row = 0;
    if (slotSizeVisible()) row++;
    if (slotGameNameVisible()) row++;
    if (slotCartNameVisible()) row++;
    if (slotDeveloperNameVisible()) row++;
    ui->slotInfoFormLayout->insertRow(row, ui->slotInfoGameSizeLabel, ui->slotInfoGameSizeOutputLabel);
  }
  
  ui->slotInfoGameSizeLabel->setVisible(visible);
  ui->slotInfoGameSizeOutputLabel->setVisible(visible);
}

bool FmCartridgeSlotWidget::slotGameSizeVisible() const
{
  return !ui->slotInfoGameSizeLabel->isHidden();
}

void FmCartridgeSlotWidget::setGameBackupEnabled(bool enabled)
{
  m_game_backup_enabled = enabled;
  ui->slotActionBackupGameButton->setEnabled(m_game_backup_enabled);
}

void FmCartridgeSlotWidget::setGameFlashEnabled(bool enabled)
{
  m_game_flash_enabled = enabled;
  ui->slotActionFlashGameButton->setEnabled(m_game_flash_enabled);
}

void FmCartridgeSlotWidget::setGameVerifyEnabled(bool enabled)
{
  m_game_verify_enabled = enabled;
  ui->slotActionVerifyGameButton->setEnabled(m_game_verify_enabled);
}

void FmCartridgeSlotWidget::setSaveBackupEnabled(bool enabled)
{
  m_save_backup_enabled = enabled;
  ui->slotActionBackupSaveButton->setEnabled(m_save_backup_enabled);
}

void FmCartridgeSlotWidget::setSaveRestoreEnabled(bool enabled)
{
  m_save_restore_enabled = enabled;
  ui->slotActionFlashSaveButton->setEnabled(m_save_restore_enabled);
}

void FmCartridgeSlotWidget::setSaveVerifyEnabled(bool enabled)
{
  m_save_verify_enabled = enabled;
  ui->slotActionVerifySaveButton->setEnabled(m_save_verify_enabled);
}
