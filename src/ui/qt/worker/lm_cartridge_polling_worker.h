#ifndef __LM_CARTRIDGE_POLLING_WORKER_H__
#define __LM_CARTRIDGE_POLLING_WORKER_H__

#include <QObject>
#include <QTimer>

class LmCartridgePollingWorker : public QObject
{
  Q_OBJECT
public:
  explicit LmCartridgePollingWorker(unsigned int id, QObject *parent = 0);
  
private:
  static const int INTERVAL;
  
public slots:
  void start();
  void stop();
  void run();
  
signals:
  void cartridgeInserted();
  void cartridgeRemoved();
  
private:
  unsigned int m_id;
  bool m_device_connected;
  bool m_running;
  
  QTimer m_timer;
};

#endif // __LM_CARTRIDGE_POLLING_WORKER_H__
