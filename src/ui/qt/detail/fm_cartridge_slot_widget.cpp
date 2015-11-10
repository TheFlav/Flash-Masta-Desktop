#include "fm_cartridge_slot_widget.h"
#include "ui_fm_cartridge_slot_widget.h"
#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"
#include "../flash_masta_app.h"
#include "../device_manager.h"
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
  
  FlashMastaApp* app = FlashMastaApp::get_instance();
  MainWindow* win = app->get_main_window();
  
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
  (void) slot;
  
  while (!FlashMastaApp::get_instance()->get_device_manager()->claim_device(m_device_id));
  std::string game_name = cart->fetch_game_name(m_slot);
  FlashMastaApp::get_instance()->get_device_manager()->release_device(m_device_id);
  
  if (game_name.empty())
  {
    game_name = "Unknown";
  }
  setSlotGameName(QString(game_name.c_str()));
}

void FmCartridgeSlotWidget::buildFromWsCartridge(ws_cartridge* cart, int slot)
{
  (void) cart;
  (void) slot;
  
  std::string game_name = "";
  setSlotGameName(QString(game_name.c_str()));
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

void FmCartridgeSlotWidget::setSlotGameName(QString name)
{
  m_slot_game_name = name;
  
  ui->slotInfoGameTitleOutputLabel->setText(name);
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
