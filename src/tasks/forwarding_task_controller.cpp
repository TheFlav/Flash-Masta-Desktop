/*! \file
 *  \brief File containing the implementation of
 *  \ref forwarding_task_controller.
 *  
 *  File containing the implementation of \ref forwarding_task_controller.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-06
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "forwarding_task_controller.h"

forwarding_task_controller::forwarding_task_controller(task_controller* receiver)
  : task_controller(), m_receiver(receiver), m_task_work_target(0), m_mutex(new std::mutex())
{
  // Nothing else to do
}

forwarding_task_controller::forwarding_task_controller(const forwarding_task_controller& other)
  : task_controller(other), m_receiver(other.m_receiver),
    m_task_work_target(other.m_task_work_target), m_mutex(new std::mutex())
{
  // Nothing else to do
}

forwarding_task_controller::~forwarding_task_controller()
{
  // Nothing else to do
  delete m_mutex;
}



void forwarding_task_controller::on_task_update(task_status status, int work_progress)
{
  m_mutex->lock();
  
  unsigned long long prev_progress = get_task_work_progress();
  task_controller::on_task_update(status, work_progress);
  unsigned long long curr_progress = get_task_work_progress();
  
  // Calculate scaled progress value
  int scaled_progress = (int) (curr_progress * m_task_work_target / get_task_expected_work());
  scaled_progress -= (int) (prev_progress * m_task_work_target / get_task_expected_work());
  
  m_receiver->on_task_update(status, scaled_progress);
  
  m_mutex->unlock();
}

bool forwarding_task_controller::is_task_cancelled() const
{
  m_mutex->lock();
  
  auto r = m_receiver->is_task_cancelled();
  
  m_mutex->unlock();
  return r;
}

forwarding_task_controller& forwarding_task_controller::scale_work_to(int work_target)
{
  m_mutex->lock();
  
  m_task_work_target = work_target;
  
  m_mutex->unlock();
  return *this;
}
