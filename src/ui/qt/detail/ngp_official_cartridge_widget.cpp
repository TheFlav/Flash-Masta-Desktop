#include "ngp_official_cartridge_widget.h"
#include "ui_ngp_official_cartridge_widget.h"

#include <QThread>
#include "../worker/ngp_lm_official_cartridge_info_worker.h"
#include "cartridge/ngp_cartridge.h"

NgpOfficalCartridgeWidget::NgpOfficalCartridgeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpOfficalCartridgeWidget), m_cartridge(nullptr),
  m_cart_thread(nullptr)
{
  ui->setupUi(this);
  
  m_cart_thread = new QThread();
}

NgpOfficalCartridgeWidget::~NgpOfficalCartridgeWidget()
{
  delete ui;
}

void NgpOfficalCartridgeWidget::on_cartridge_loaded(cartridge* cart)
{
  (void) cart;
}

