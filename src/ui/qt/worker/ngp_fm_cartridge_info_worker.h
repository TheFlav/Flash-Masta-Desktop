//
//  ngp_fm_cartridge_info_worker.hpp
//  FlashMasta
//
//  Created by Dan on 10/14/15.
//  Copyright © 2015 7400 Circuits. All rights reserved.
//

#ifndef __NGP_FM_CARTRIDGE_INFO_WORKER_H__
#define __NGP_FM_CARTRIDGE_INFO_WORKER_H__

#include <QObject>
#include <QMutex>

class ngp_cartridge;

class NgpFmCartridgeInfoWorker : public QObject
{
  Q_OBJECT
public:
  explicit NgpFmCartridgeInfoWorker(unsigned int device_id, QObject *parent = 0);
  ~NgpFmCartridgeInfoWorker();
  
signals:
  void finished(ngp_cartridge* cartridge);
  
public slots:
  void run();
  void cancel();
  
private:
  unsigned int m_id;
  QMutex m_mutex;
  bool   m_cancelled;
};

#endif /* __NGP_FM_CARTRIDGE_INFO_WORKER_H__ */
