#ifndef NGP_LINKMASTA_DETAIL_WIDGET_H
#define NGP_LINKMASTA_DETAIL_WIDGET_H

#include <QWidget>
#include <QThread>

namespace Ui {
class NgpLinkmastaDetailWidget;
}

class NgpLinkmastaDetailWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpLinkmastaDetailWidget(unsigned int device_id, QWidget *parent = 0);
  ~NgpLinkmastaDetailWidget();
  
  void start_polling();
  void stop_polling();
  
public slots:
  void cartridge_removed();
  void cartridge_inserted();
  void selected_device_changed(int old_device, int new_device);
  
private:
  Ui::NgpLinkmastaDetailWidget *ui;
  const unsigned int m_device_id;
  
  QWidget* m_default_widget;
  QWidget* m_cartridge_widget;
  QThread* m_pooling_thread;
};

#endif // NGP_LINKMASTA_DETAIL_WIDGET_H
