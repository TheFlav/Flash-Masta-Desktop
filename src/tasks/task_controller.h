//
//  task_controller.h
//  FlashMasta
//
//  Created by Dan on 8/6/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __TASK_CONTROLLER_H__
#define __TASK_CONTROLLER_H__

enum task_status
{
  ERROR,
  NOT_STARTED,
  STARTING,
  RUNNING,
  STOPPING,
  COMPLETED
};

class task_controller
{
public:
  task_controller();
  virtual ~task_controller();
  virtual void on_task_start(int work_expected);
  virtual void on_task_update(task_status status, int work_progress);
  virtual void on_task_complete(task_status status, int work_total);
  virtual bool is_task_cancelled() const;
  virtual float get_task_progress_percentage() const;

protected:
  virtual task_status get_task_status() const;
  virtual int get_task_expected_work() const;
  virtual int get_task_work_progress() const;
  virtual void cancel_task();
  
private:
  task_status m_task_status;
  int m_task_work_expected;
  int m_task_work_total;
  bool m_task_is_cancelled;
};

#endif /* defined(__TASK_CONTROLLER_H__) */
