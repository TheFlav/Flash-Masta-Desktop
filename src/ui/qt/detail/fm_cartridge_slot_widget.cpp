#include "fm_cartridge_slot_widget.h"
#include "ui_fm_cartridge_slot_widget.h"
#include "cartridge/cartridge.h"
#include "cartridge/ngp_cartridge.h"
#include "cartridge/ws_cartridge.h"



FmCartridgeSlotWidget::FmCartridgeSlotWidget(cartridge* cart, int slot, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FmCartridgeSlotWidget)
{
  ui->setupUi(this);
  
  if (cart != nullptr && slot != -1)
  {
    buildFromCartridge(cart, slot);
  }
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
}

void FmCartridgeSlotWidget::buildFromNgpCartridge(ngp_cartridge* cart, int slot)
{
  (void) slot;
  
  std::string game_name = cart->fetch_game_name(m_slot);
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



//////// HELPER FUNCTIONS ////////


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
