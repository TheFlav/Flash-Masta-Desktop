#include "ngp_fm_cartridge_info_widget.h"
#include "ui_ngp_fm_cartridge_info_widget.h"
#include "cartridge/ngp_cartridge.h"
#include <QString>
#include <QLabel>

QString stringifyBytesToBits(unsigned int num_bytes, bool reduce = true);



NgpFmCartridgeInfoWidget::NgpFmCartridgeInfoWidget(ngp_cartridge* cartridge, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFmCartridgeInfoWidget), m_cart_chip_widgets(nullptr), m_cart_chip_sizes(nullptr)
{
  ui->setupUi(this);
  
  if (cartridge == nullptr)
  {
    setGameBackupEnabled(false);
    setGameFlashEnabled(false);
    setSaveBackupEnabled(false);
    setSaveRestoreEnabled(false);
  }
  else
  {
    buildFromCartridge(cartridge);
  }
}

NgpFmCartridgeInfoWidget::~NgpFmCartridgeInfoWidget()
{
  clearChipData();
  delete ui;
}


void NgpFmCartridgeInfoWidget::buildFromCartridge(ngp_cartridge* cartridge)
{
  const cartridge_descriptor* descriptor = cartridge->descriptor();
  
  setCartridgeSize(descriptor->num_bytes);
  setCartridgeNumSlots(cartridge->num_slots());
  setCartridgeNumChips(descriptor->num_chips);
  for (unsigned int chip_i = 0; chip_i < descriptor->num_chips; chip_i++)
  {
    setCartridgeChipSize(chip_i, descriptor->chips[chip_i]->num_bytes);
  }
  
  // TODO: Decide cartridge capabilities based on the cartridge's chip's identifiers
  setGameBackupEnabled(true);
  setGameFlashEnabled(true);
  setSaveBackupEnabled(true);
  setSaveRestoreEnabled(true);
}


unsigned int NgpFmCartridgeInfoWidget::cartridgeSize() const
{
  return m_cart_num_bytes;
}

unsigned int NgpFmCartridgeInfoWidget::cartridgeNumSlots() const
{
  return m_cart_num_slots;
}

unsigned int NgpFmCartridgeInfoWidget::cartridgeNumChips() const
{
  return m_cart_num_chips;
}

unsigned int NgpFmCartridgeInfoWidget::cartridgeChipSize(unsigned int chip_index) const
{
  return (chip_index >= m_cart_num_slots ? 0 : m_cart_chip_sizes[chip_index]);
}

bool NgpFmCartridgeInfoWidget::gameBackupEnabled() const
{
  return m_game_backup_enabled;
}

bool NgpFmCartridgeInfoWidget::gameFlashEnabled() const
{
  return m_game_flash_enabled;
}

bool NgpFmCartridgeInfoWidget::saveBackupEnabled() const
{
  return m_save_backup_enabled;
}

bool NgpFmCartridgeInfoWidget::saveRestoreEnabled() const
{
  return m_save_restore_enabled;
}



void NgpFmCartridgeInfoWidget::setCartridgeSize(unsigned int num_bytes)
{
  m_cart_num_bytes = num_bytes;
  
  QString text = stringifyBytesToBits(num_bytes);
  
  ui->cartridgeSpecCapacityOutputLabel->setText(text);
}

void NgpFmCartridgeInfoWidget::setCartridgeNumSlots(unsigned int num_slots)
{
  m_cart_num_slots = num_slots;
  ui->cartridgeSpecNumSlotsOutputLabel->setText(QString::number(num_slots));
}

void NgpFmCartridgeInfoWidget::setCartridgeNumChips(unsigned int num_chips)
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
    
    // Create widgets for chip
    QWidget* chip_size_label = new QLabel(QString("Chip ") + QString::number(chip_i + 1) + QString(" size:"), ui->cartridgeSpecsFormLayout->widget());
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
  
  ui->cartridgeSpecNumChipsOutputLabel->setText(QString::number(m_cart_num_chips));
}

void NgpFmCartridgeInfoWidget::setCartridgeChipSize(unsigned int chip_index, unsigned int num_bytes)
{
  if (chip_index >= m_cart_num_chips) return;
  
  m_cart_chip_sizes[chip_index] = num_bytes;
  
  // Update widget display
  QString text = stringifyBytesToBits(m_cart_chip_sizes[chip_index]);
  ((QLabel*) m_cart_chip_widgets[chip_index][1])->setText(text);
}

void NgpFmCartridgeInfoWidget::setGameBackupEnabled(bool enabled)
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
}

void NgpFmCartridgeInfoWidget::setGameFlashEnabled(bool enabled)
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
}

void NgpFmCartridgeInfoWidget::setSaveBackupEnabled(bool enabled)
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
}

void NgpFmCartridgeInfoWidget::setSaveRestoreEnabled(bool enabled)
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
}



void NgpFmCartridgeInfoWidget::clearChipData()
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


//////// HELPER FUNCTIONS ////////


QString stringifyBytesToBits(unsigned int num_bytes, bool reduce)
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

