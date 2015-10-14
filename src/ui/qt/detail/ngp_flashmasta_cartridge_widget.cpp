#include "ngp_flashmasta_cartridge_widget.h"
#include "ui_ngp_flashmasta_cartridge_widget.h"
#include "cartridge/ngp_cartridge.h"
#include "ngp_flashmasta_cartridge_slot_widget.h"

#include <QString>
#include <string>

NgpFlashmastaCartridgeWidget::NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFlashmastaCartridgeWidget), m_device_id(device_id), m_cartridge(nullptr)
{
  ui->setupUi(this);
}

NgpFlashmastaCartridgeWidget::~NgpFlashmastaCartridgeWidget()
{
  if (m_cartridge != nullptr) delete m_cartridge;
  
  delete ui;
}

void NgpFlashmastaCartridgeWidget::refresh_ui()
{
  // Reset everything and erase cached data
  ui->slotsComboBox->clear();
  for (QWidget* widget : m_slot_widgets)
  {
    delete widget;
  }
  m_slot_widgets.clear();
  
  ui->slotsComboBox->insertItem(0, "Cartridge Info");
  
  for (int i = 0; i < m_cartridge->num_slots(); ++i)
  {
    ui->slotsComboBox->insertItem(i+1, "Slot " + QString(i+1));
  }
  
  ui->slotsComboBox->setCurrentIndex(0);
}

void NgpFlashmastaCartridgeWidget::on_cartridge_loaded(ngp_cartridge* cartridge)
{
  if (m_cartridge != nullptr) delete m_cartridge;
  m_cartridge = cartridge;
  refresh_ui();
}
