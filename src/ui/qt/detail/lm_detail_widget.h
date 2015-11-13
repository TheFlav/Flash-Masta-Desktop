#ifndef __NGP_LINKMASTA_DETAIL_WIDGET_H__
#define __NGP_LINKMASTA_DETAIL_WIDGET_H__

#include <QWidget>

#include <QThread>

namespace Ui {
class LmDetailWidget;
}

class LmDetailWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit LmDetailWidget(unsigned int device_id, QWidget *parent = 0);
  ~LmDetailWidget();
  
  void startPolling();
  void stopPolling();
  
private:
  void disableActions();
  
public slots:
  void cartridgeRemoved();
  void cartridgeInserted();
  void selectedDeviceChanged(int old_device, int new_device);
  void selectedSlotChanged(int old_slot, int new_slot);
  
private:
  Ui::LmDetailWidget *ui;
  const unsigned int m_device_id;
  
  QWidget* m_default_widget;
  QWidget* m_cartridge_widget;
  QThread* m_pooling_thread;
};

#endif // __NGP_LINKMASTA_DETAIL_WIDGET_H__
