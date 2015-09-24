#ifndef NGP_OFFICIAL_CARTRIDGE_WIDGET_H
#define NGP_OFFICIAL_CARTRIDGE_WIDGET_H

#include <QWidget>

namespace Ui {
class NgpOfficalCartridgeWidget;
}

class NgpOfficalCartridgeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpOfficalCartridgeWidget(QWidget *parent = 0);
  ~NgpOfficalCartridgeWidget();
  
private:
  Ui::NgpOfficalCartridgeWidget *ui;
};

#endif // NGP_OFFICIAL_CARTRIDGE_WIDGET_H
