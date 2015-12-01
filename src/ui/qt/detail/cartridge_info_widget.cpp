#include "cartridge_info_widget.h"
#include "ui_cartridge_info_widget.h"

#include <QLabel>
#include <QString>

#include "cartridge/cartridge.h"

#include "../device_manager.h"
#include "../flash_masta_app.h"
#include "../main_window.h"



CartridgeInfoWidget::CartridgeInfoWidget(int device_id, cartridge* cart, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::CartridgeInfoWidget), m_cart_chip_widgets(nullptr),
  m_device_id(device_id), m_cart_chip_sizes(nullptr)
{
  ui->setupUi(this);
  
  if (cart == nullptr)
  {
    setGameBackupEnabled(false);
    setGameFlashEnabled(false);
    setGameVerifyEnabled(false);
    setSaveBackupEnabled(false);
    setSaveRestoreEnabled(false);
    setSaveVerifyEnabled(false);
  }
  else
  {
    buildFromCartridge(cart);
  }
  
  MainWindow* mw = FlashMastaApp::getInstance()->getMainWindow();
  connect(FlashMastaApp::getInstance(), SIGNAL(selectedDeviceChanged(int,int)), this, SLOT(onDeviceSelected(int,int)));
  connect(ui->cartActionBackupGameButton, SIGNAL(clicked(bool)), mw, SLOT(triggerActionBackupGame()));
  connect(ui->cartActionFlashGameButton, SIGNAL(clicked(bool)), mw, SLOT(triggerActionFlashGame()));
  connect(ui->cartActionVerifyGameButton, SIGNAL(clicked(bool)), mw, SLOT(triggerActionVerifyGame()));
  connect(ui->cartActionBackupSaveButton, SIGNAL(clicked(bool)), mw, SLOT(triggerActionBackupSave()));
  connect(ui->cartActionRestoreSaveButton, SIGNAL(clicked(bool)), mw, SLOT(triggerActionRestoreSave()));
  connect(ui->cartActionVerifySaveButton, SIGNAL(clicked(bool)), mw, SLOT(triggerActionVerifySave()));
}

CartridgeInfoWidget::~CartridgeInfoWidget()
{
  clearChipData();
  delete ui;
}


void CartridgeInfoWidget::buildFromCartridge(cartridge* cart)
{
  while (!FlashMastaApp::getInstance()->getDeviceManager()->tryClaimDevice(m_device_id));
  
  const cartridge_descriptor* descriptor = cart->descriptor();
  
  setCartridgeSize(descriptor->num_bytes);
  setCartridgeNumSlots(cart->num_slots());
  
  // Special case the number of chips based on cartridge system
  if (descriptor->system == system_type::SYSTEM_WONDERSWAN)
  {
    setCartridgeNumChips(descriptor->num_chips + 1);
  }
  else
  {
    setCartridgeNumChips(descriptor->num_chips);
  }
  
  for (unsigned int chip_i = 0; chip_i < descriptor->num_chips; chip_i++)
  {
    setCartridgeChipSize(chip_i, descriptor->chips[chip_i]->num_bytes);
    
    // Special case chip label if Wonderswan
    if (descriptor->num_chips > 1 && descriptor->system == system_type::SYSTEM_WONDERSWAN)
    {
      ((QLabel*) m_cart_chip_widgets[chip_i][0])->setText(QString("Flash chip ") + QString::number(chip_i+1) + QString(" size:"));
    }
  }
  
  if (descriptor->system == system_type::SYSTEM_WONDERSWAN)
  {
    if (descriptor->num_chips == 1)
    {
      ((QLabel*) m_cart_chip_widgets[0][0])->setText("Flash chip size:");
    }
    
    ((QLabel*) m_cart_chip_widgets[descriptor->num_chips][0])->setText("SRAM chip size:");
    setCartridgeChipSize(descriptor->num_chips, 524288); // SRAM is always 4 MiB
  }
  
  // Decide cartridge capabilities based on the cartridge's chip's identifiers
  switch (descriptor->system)
  {
  default:
  case SYSTEM_UNKNOWN:
    setGameBackupEnabled(false);
    setGameFlashEnabled(false);
    setGameVerifyEnabled(false);
    setSaveBackupEnabled(false);
    setSaveRestoreEnabled(false);
    setSaveVerifyEnabled(false);
    break;
    
  case SYSTEM_NEO_GEO_POCKET:    
    switch (descriptor->type)
    {
    default:
    case CARTRIDGE_UNKNOWN:
      setGameBackupEnabled(false);
      setGameFlashEnabled(false);
      setGameVerifyEnabled(false);
      setSaveBackupEnabled(false);
      setSaveRestoreEnabled(false);
      setSaveVerifyEnabled(false);
      break;
      
    case CARTRIDGE_OFFICIAL:
      setGameBackupEnabled(true);
      setGameFlashEnabled(false);
      setGameVerifyEnabled(true);
      setSaveBackupEnabled(true);
      setSaveRestoreEnabled(true);
      setSaveVerifyEnabled(true);
      break;
      
    case CARTRIDGE_FLASHMASTA:
      setGameBackupEnabled(true);
      setGameFlashEnabled(true);
      setGameVerifyEnabled(true);
      setSaveBackupEnabled(false);
      setSaveRestoreEnabled(true);
      setSaveVerifyEnabled(true);
      break;
    }
    break;
    
  case SYSTEM_WONDERSWAN:
    setGameBackupEnabled(true);
    setGameFlashEnabled(true);
    setGameVerifyEnabled(true);
    setSaveBackupEnabled(true);
    setSaveRestoreEnabled(true);
    setSaveVerifyEnabled(true);
    break;
  }
  
  // Show/hide IU elements based on cartridge type
  bool is_flashmasta = cart->type() == cartridge_type::CARTRIDGE_FLASHMASTA;
  setPromptLabelVisible(is_flashmasta);
  setCartridgeSpecNumSlotsVisible(is_flashmasta);
  
  FlashMastaApp::getInstance()->getDeviceManager()->releaseDevice(m_device_id);
}



unsigned int CartridgeInfoWidget::cartridgeSize() const
{
  return m_cart_num_bytes;
}

unsigned int CartridgeInfoWidget::cartridgeNumSlots() const
{
  return m_cart_num_slots;
}

unsigned int CartridgeInfoWidget::cartridgeNumChips() const
{
  return m_cart_num_chips;
}

unsigned int CartridgeInfoWidget::cartridgeChipSize(unsigned int chip_index) const
{
  return (chip_index >= m_cart_num_slots ? 0 : m_cart_chip_sizes[chip_index]);
}

bool CartridgeInfoWidget::gameBackupEnabled() const
{
  return m_game_backup_enabled;
}

bool CartridgeInfoWidget::gameFlashEnabled() const
{
  return m_game_flash_enabled;
}

bool CartridgeInfoWidget::gameVerifyEnabled() const
{
  return m_game_verify_enabled;
}

bool CartridgeInfoWidget::saveBackupEnabled() const
{
  return m_save_backup_enabled;
}

bool CartridgeInfoWidget::saveRestoreEnabled() const
{
  return m_save_restore_enabled;
}

bool CartridgeInfoWidget::saveVerifyEnabled() const
{
  return m_save_verify_enabled;
}



// private:

void CartridgeInfoWidget::setPromptLabelVisible(bool visible)
{
  ui->promptLabel->setVisible(visible);
}

void CartridgeInfoWidget::setCartridgeSpecNumSlotsVisible(bool visible)
{
  ui->cartridgeSpecNumSlotsLabel->setVisible(visible);
  ui->cartridgeSpecNumSlotsOutputLabel->setVisible(visible);
}

void CartridgeInfoWidget::clearChipData()
{
  // Delete existing widgets for chips
  if (m_cart_chip_widgets != nullptr)
  {
    for (unsigned int chip_i = 0; m_cart_chip_widgets[chip_i] != nullptr; chip_i++)
    {
      for (unsigned int widget_i = 0; m_cart_chip_widgets[chip_i][widget_i] != nullptr; widget_i++)
      {
        m_cart_chip_widgets[chip_i][widget_i]->hide();
        delete m_cart_chip_widgets[chip_i][widget_i];
      }
      delete [] m_cart_chip_widgets[chip_i];
    }
    delete [] m_cart_chip_widgets;
    m_cart_chip_widgets = nullptr;
  }
  
  // Clear chip size array
  if (m_cart_chip_sizes != nullptr)
  {
    delete [] m_cart_chip_sizes;
  }
}

QString CartridgeInfoWidget::stringifyBytesToBits(unsigned int num_bytes, bool reduce)
{
  QString text;
  if (reduce && (num_bytes & 0b111111111111111111111111111) == 0)
  {
    text = QString::number(num_bytes >> 27) + QString(" Gib");
  }
  else if (reduce && (num_bytes & 0b11111111111111111) == 0)
  {
    text = QString::number(num_bytes >> 17) + QString(" Mib");
  }
  else if (reduce && (num_bytes & 0xb1111111) == 0)
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

void CartridgeInfoWidget::setCartridgeSize(unsigned int num_bytes)
{
  m_cart_num_bytes = num_bytes;
  
  QString text = stringifyBytesToBits(num_bytes);
  
  ui->cartridgeSpecCapacityOutputLabel->setText(text);
}

void CartridgeInfoWidget::setCartridgeNumSlots(unsigned int num_slots)
{
  m_cart_num_slots = num_slots;
  ui->cartridgeSpecNumSlotsOutputLabel->setText(QString::number(num_slots));
}

void CartridgeInfoWidget::setCartridgeNumChips(unsigned int num_chips)
{
  m_cart_num_chips = num_chips;
  
  clearChipData();
  
  // Recreate widgets for chips
  m_cart_chip_widgets = new QWidget**[m_cart_num_chips + 1];
  for (unsigned int chip_i = 0; chip_i <= m_cart_num_chips; chip_i++)
  {
    m_cart_chip_widgets[chip_i] = nullptr;
    if (chip_i >= m_cart_num_chips) continue;
    
    m_cart_chip_widgets[chip_i] = new QWidget*[2 + 1];
    for (unsigned int widget_i = 0; widget_i <= 2 + 1; widget_i++)
    {
      m_cart_chip_widgets[chip_i][widget_i] = nullptr;
    }
    
    // Build label text
    QString label_text;
    if (m_cart_num_chips == 1)
    {
      label_text = "Flash chip size:";
    }
    else
    {
      label_text = "Flash chip " + QString::number(chip_i + 1) + " size:";
    }
    
    // Create widgets for chip
    QWidget* chip_size_label = new QLabel(label_text, ui->cartridgeSpecsFormLayout->widget());
    QWidget* chip_size_field = new QLabel(QString(""), ui->cartridgeSpecsFormLayout->widget());
    QFont font = QFont(chip_size_field->font());
    font.setBold(true);
    chip_size_field->setFont(font);
      
    m_cart_chip_widgets[chip_i][0] = chip_size_label;
    m_cart_chip_widgets[chip_i][1] = chip_size_field;
    
    ui->cartridgeSpecsFormLayout->addRow(chip_size_label, chip_size_field);
  }
  
  // Rebuild chip size array
  m_cart_chip_sizes = new unsigned int[m_cart_num_chips];
  for (unsigned int chip_i = 0; chip_i < m_cart_num_chips; chip_i++)
  {
    setCartridgeChipSize(chip_i, 0);
  }
}

void CartridgeInfoWidget::setCartridgeChipSize(unsigned int chip_index, unsigned int num_bytes)
{
  if (chip_index >= m_cart_num_chips) return;
  
  m_cart_chip_sizes[chip_index] = num_bytes;
  
  // Update widget display
  QString text = stringifyBytesToBits(m_cart_chip_sizes[chip_index]);
  ((QLabel*) m_cart_chip_widgets[chip_index][1])->setText(text);
}

void CartridgeInfoWidget::setGameBackupEnabled(bool enabled)
{
  m_game_backup_enabled = enabled;
  
  if (m_game_backup_enabled)
  {
    ui->gameBackupDisabledLabel->hide();
    ui->gameBackupEnabledLabel->show();
  }
  else
  {
    ui->gameBackupDisabledLabel->show();
    ui->gameBackupEnabledLabel->hide();
  }
  ui->cartActionBackupGameButton->setEnabled(m_game_backup_enabled);
}

void CartridgeInfoWidget::setGameFlashEnabled(bool enabled)
{
  m_game_flash_enabled = enabled;
  
  if (m_game_flash_enabled)
  {
    ui->gameFlashDisabledLabel->hide();
    ui->gameFlashEnabledLabel->show();
  }
  else
  {
    ui->gameFlashDisabledLabel->show();
    ui->gameFlashEnabledLabel->hide();
  }
  ui->cartActionFlashGameButton->setEnabled(enabled);
}

void CartridgeInfoWidget::setGameVerifyEnabled(bool enabled)
{
  m_game_verify_enabled = enabled;
  ui->cartActionVerifyGameButton->setEnabled(enabled);
}

void CartridgeInfoWidget::setSaveBackupEnabled(bool enabled)
{
  m_save_backup_enabled = enabled;
  
  if (m_save_backup_enabled)
  {
    ui->saveBackupDisabledLabel->hide();
    ui->saveBackupEnabledLabel->show();
  }
  else
  {
    ui->saveBackupDisabledLabel->show();
    ui->saveBackupEnabledLabel->hide();
  }
  ui->cartActionBackupSaveButton->setEnabled(enabled);
}

void CartridgeInfoWidget::setSaveRestoreEnabled(bool enabled)
{
  m_save_restore_enabled = enabled;
  
  if (m_save_restore_enabled)
  {
    ui->saveRestoreDisabledLabel->hide();
    ui->saveRestoreEnabledLabel->show();
  }
  else
  {
    ui->saveRestoreDisabledLabel->show();
    ui->saveRestoreEnabledLabel->hide();
  }
  ui->cartActionRestoreSaveButton->setEnabled(enabled);
}

void CartridgeInfoWidget::setSaveVerifyEnabled(bool enabled)
{
  m_save_verify_enabled = enabled;
  ui->cartActionVerifySaveButton->setEnabled(enabled);
}



// private slots:

void CartridgeInfoWidget::onDeviceSelected(int old_device_id, int new_device_id)
{
  (void) old_device_id;
  if (new_device_id != m_device_id) return;
  
  FlashMastaApp* app = FlashMastaApp::getInstance();
  app->setGameBackupEnabled(m_game_backup_enabled);
  app->setGameFlashEnabled(m_game_flash_enabled);
  app->setGameVerifyEnabled(m_game_verify_enabled);
  app->setSaveBackupEnabled(m_save_backup_enabled);
  app->setSaveRestoreEnabled(m_save_restore_enabled);
  app->setSaveVerifyEnabled(m_save_verify_enabled);
}


