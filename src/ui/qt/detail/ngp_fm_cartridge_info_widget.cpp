#include "ngp_fm_cartridge_info_widget.h"
#include "ui_ngp_fm_cartridge_info_widget.h"

NgpFmCartridgeInfoWidget::NgpFmCartridgeInfoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpFmCartridgeInfoWidget)
{
  ui->setupUi(this);
}

NgpFmCartridgeInfoWidget::~NgpFmCartridgeInfoWidget()
{
  delete ui;
}
