//
//  forwarding_task_controller.h
//  FlashMasta
//
//  Created by Dan on 8/6/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __FORWARDING_TASK_CONTROLLER_H__
#define __FORWARDING_TASK_CONTROLLER_H__

#include "task_controller.h"
#include <mutex>

class forwarding_task_controller: public task_controller
{
public:
  forwarding_task_controller(task_controller* receiver);
  forwarding_task_controller(const forwarding_task_controller& other);
  virtual ~forwarding_task_controller();
  
  virtual void on_task_update(task_status status, int work_progress);
  virtual bool is_task_cancelled() const;
  
  forwarding_task_controller& scale_work_to(int work_target);
  
private:
  task_controller* const m_receiver;
  int m_task_work_target;
  std::mutex* m_mutex;
};

#endif /* defined(__FORWARDING_TASK_CONTROLLER_H__) */
