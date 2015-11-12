#ifndef __LM_OFFICIAL_CARTRIDGE_INFO_WORKER_H__
#define __LM_OFFICIAL_CARTRIDGE_INFO_WORKER_H__

#include <QObject>
#include <QMutex>

class cartridge;

class LmCartridgeFetchingWorker : public QObject
{
  Q_OBJECT
public:
  explicit LmCartridgeFetchingWorker(unsigned int device_id, QObject *parent = 0);
  
public slots:
  void run();
  void cancel();
  
signals:
  void finished(cartridge* cart, QString cartridge_game_name);
  
private:
  unsigned int m_device_id;
  QMutex m_mutex;
  bool m_cancelled;
};

#endif // __LM_OFFICIAL_CARTRIDGE_INFO_WORKER_H__
