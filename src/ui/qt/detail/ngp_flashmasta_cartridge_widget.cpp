#include "ngp_flashmasta_cartridge_widget.h"
#include "ui_ngp_flashmasta_cartridge_widget.h"
#include "cartridge/ngp_cartridge.h"

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
