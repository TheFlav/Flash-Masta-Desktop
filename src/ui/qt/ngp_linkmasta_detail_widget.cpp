#include "ngp_linkmasta_detail_widget.h"
#include "ui_ngp_linkmasta_detail_widget.h"

NgpLinkmastaDetailWidget::NgpLinkmastaDetailWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NgpLinkmastaDetailWidget)
{
  ui->setupUi(this);
}

NgpLinkmastaDetailWidget::~NgpLinkmastaDetailWidget()
{
  delete ui;
}
