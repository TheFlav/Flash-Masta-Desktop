#ifndef NGP_OFFICIAL_CARTRIDGE_WIDGET_H
#define NGP_OFFICIAL_CARTRIDGE_WIDGET_H

#include <QWidget>

class QThread;
namespace Ui {
class NgpOfficalCartridgeWidget;
}

class ngp_cartridge;

class NgpOfficalCartridgeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpOfficalCartridgeWidget(QWidget *parent = 0);
  ~NgpOfficalCartridgeWidget();
  
public slots:
  void on_cartridge_loaded(cartridge* cart);
  
private:
  Ui::NgpOfficalCartridgeWidget *ui;
  ngp_cartridge*                m_cartridge;
  
  QThread*                      m_cart_thread;
};

#endif // NGP_OFFICIAL_CARTRIDGE_WIDGET_H
