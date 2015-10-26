#include "ngp_flashmasta_cartridge_slot_widget.h"
#include "ui_ngp_flashmasta_cartridge_slot_widget.h"

#include "cartridge/ngp_cartridge.h"

NgpFlashMastaCartridgeSlotWidget::NgpFlashMastaCartridgeSlotWidget(unsigned int device_id, ngp_cartridge* cartridge, unsigned int slot, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFlashMastaCartridgeSlotWidget), m_device_id(device_id), m_cartridge(cartridge), m_slot(slot)
{
  ui->setupUi(this);
}

NgpFlashMastaCartridgeSlotWidget::~NgpFlashMastaCartridgeSlotWidget()
{
  delete ui;
}
