#ifndef DEVICE_INFO_WIDGET_H
#define DEVICE_INFO_WIDGET_H

#include <QWidget>
#include <QMutex>
#include <string>

namespace Ui {
class DeviceInfoWidget;
}



/// Worker object for fetching the device info and passing it on to be displayed in the DeviceInfoWidget
class worker : public QObject
{
  Q_OBJECT
  
public:
  explicit worker(unsigned int id);
  ~worker();
  
public slots:
  void process();
  
signals:
  void finished(QString product_str, QString game_str);
  
private:
  unsigned int m_id;
  std::string  product_str;
  std::string  game_str;
  bool         m_full;
};



class DeviceInfoWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit DeviceInfoWidget(QWidget *parent = 0);
  ~DeviceInfoWidget();
  
  void set_device_id(unsigned int device_id);
  
  
private slots:
  void refresh_ui(QString device_name, QString game_name);
  
private:
  Ui::DeviceInfoWidget *ui;
  
  unsigned int m_device_id;
  QString m_device_name;
  QString m_game_name;
  
  QThread* m_thread;
  worker* m_worker;
};

#endif // DEVICE_INFO_WIDGET_H
