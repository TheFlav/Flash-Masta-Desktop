//
//  forwarding_task_controller.cpp
//  FlashMasta
//
//  Created by Dan on 8/6/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "forwarding_task_controller.h"

forwarding_task_controller::forwarding_task_controller(task_controller* receiver)
  : task_controller(), m_receiver(receiver), m_task_work_target(0)
{
  // Nothing else to do
}

forwarding_task_controller::forwarding_task_controller(const forwarding_task_controller& other)
  : task_controller(other), m_receiver(other.m_receiver),
    m_task_work_target(other.m_task_work_target)
{
  // Nothing else to do
}

forwarding_task_controller::~forwarding_task_controller()
{
  // Nothing else to do
}



void forwarding_task_controller::on_task_update(task_status status, int work_progress)
{
  int prev_progress = get_task_progress_percentage();
  task_controller::on_task_update(status, work_progress);
  int curr_progress = get_task_progress_percentage();
  
  // Calculate scaled progress value
  int scaled_progress = (curr_progress * m_task_work_target / get_task_expected_work());
  scaled_progress -= (prev_progress * m_task_work_target / get_task_expected_work());
  
  m_receiver->on_task_update(status, scaled_progress);
}

bool forwarding_task_controller::is_task_cancelled() const
{
  return m_receiver->is_task_cancelled();
}

forwarding_task_controller& forwarding_task_controller::scale_work_to(int work_target)
{
  m_task_work_target = work_target;
  
  return *this;
}
