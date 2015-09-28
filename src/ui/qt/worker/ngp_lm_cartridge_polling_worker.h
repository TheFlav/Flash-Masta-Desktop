#ifndef NGP_LM_CARTRIDGE_POLLING_WORKER_H
#define NGP_LM_CARTRIDGE_POLLING_WORKER_H

#include <QObject>
#include <QTimer>

class NgpLmCartridgePollingWorker : public QObject
{
  Q_OBJECT
public:
  explicit NgpLmCartridgePollingWorker(unsigned int id, QObject *parent = 0);
  
signals:
  void cartridge_inserted();
  void cartridge_removed();
  
public slots:
  void start();
  void stop();
  void run();
  
private:
  static const int INTERVAL;
  
  unsigned int m_id;
  bool m_device_connected;
  bool m_running;
  
  QTimer m_timer;
};

#endif // NGP_LM_CARTRIDGE_WORKER_H
