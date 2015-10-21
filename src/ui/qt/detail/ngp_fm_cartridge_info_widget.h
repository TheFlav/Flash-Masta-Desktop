#ifndef NGP_FM_CARTRIDGE_INFO_WIDGET_H
#define NGP_FM_CARTRIDGE_INFO_WIDGET_H

#include <QWidget>

namespace Ui {
class NgpFmCartridgeInfoWidget;
}

class NgpFmCartridgeInfoWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpFmCartridgeInfoWidget(QWidget *parent = 0);
  ~NgpFmCartridgeInfoWidget();
  
private:
  Ui::NgpFmCartridgeInfoWidget *ui;
};

#endif // NGP_FM_CARTRIDGE_INFO_WIDGET_H
