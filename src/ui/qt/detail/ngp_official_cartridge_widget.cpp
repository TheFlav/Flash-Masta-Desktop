#include "ngp_official_cartridge_widget.h"
#include "ui_ngp_official_cartridge_widget.h"

NgpOfficalCartridgeWidget::NgpOfficalCartridgeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpOfficalCartridgeWidget)
{
  ui->setupUi(this);
}

NgpOfficalCartridgeWidget::~NgpOfficalCartridgeWidget()
{
  delete ui;
}
