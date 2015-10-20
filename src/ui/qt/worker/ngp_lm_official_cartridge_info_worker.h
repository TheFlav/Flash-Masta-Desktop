#ifndef NGPLMOFFICIALCARTRIDGEINFOWORKER_H
#define NGPLMOFFICIALCARTRIDGEINFOWORKER_H

#include <QObject>
#include <QMutex>

class ngp_cartridge;

class NgpLmOfficialCartridgeInfoWorker : public QObject
{
  Q_OBJECT
public:
  explicit NgpLmOfficialCartridgeInfoWorker(unsigned int device_id, QObject *parent = 0);
  
signals:
  void finished(ngp_cartridge* cart);
  
public slots:
  void run();
  void cancel();
  
private:
  unsigned int m_device_id;
  QMutex m_mutex;
  bool m_cancelled;
};

#endif // NGPLMOFFICIALCARTRIDGEINFOWORKER_H
