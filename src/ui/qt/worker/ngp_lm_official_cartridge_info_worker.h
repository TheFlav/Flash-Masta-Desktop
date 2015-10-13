#ifndef NGPLMOFFICIALCARTRIDGEINFOWORKER_H
#define NGPLMOFFICIALCARTRIDGEINFOWORKER_H

#include <QObject>

class cartridge;

class NgpLmOfficialCartridgeInfoWorker : public QObject
{
  Q_OBJECT
public:
  explicit NgpLmOfficialCartridgeInfoWorker(unsigned int device_id, QObject *parent = 0);
  
signals:
  void finished(cartridge* cart);
  
public slots:
  void run();
  
private:
  unsigned int m_device_id;
};

#endif // NGPLMOFFICIALCARTRIDGEINFOWORKER_H
