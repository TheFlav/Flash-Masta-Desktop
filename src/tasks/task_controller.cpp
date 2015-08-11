//
//  task_controller.cpp
//  FlashMasta
//
//  Created by Dan on 8/6/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "task_controller.h"

task_controller::task_controller()
  : m_task_status(NOT_STARTED), m_task_work_expected(0), m_task_work_total(0),
    m_task_is_cancelled(false), m_mutex(new std::mutex())
{
  // Nothing else to do
}

task_controller::task_controller(const task_controller& other)
  : m_task_status(other.m_task_status),
    m_task_work_expected(other.m_task_work_expected),
    m_task_work_total(other.m_task_work_total),
    m_task_is_cancelled(other.m_task_is_cancelled),
    m_mutex(new std::mutex())
{
  // Nothing else to do
}

task_controller::~task_controller()
{
  // Nothing else to do
  delete m_mutex;
}

void task_controller::on_task_start(int work_expected)
{
  m_mutex->lock();
  m_task_status = RUNNING;
  m_task_work_expected = work_expected;
  m_task_work_total = 0;
  m_mutex->unlock();
}

void task_controller::on_task_update(task_status status, int work_progress)
{
  m_mutex->lock();
  m_task_status = status;
  m_task_work_total += work_progress;
  m_mutex->unlock();
}

void task_controller::on_task_end(task_status status, int work_total)
{
  m_mutex->lock();
  m_task_status = status;
  m_task_work_total = work_total;
  m_mutex->unlock();
}

bool task_controller::is_task_cancelled() const
{
  m_mutex->lock();
  auto r = m_task_is_cancelled;
  m_mutex->unlock();
  return r;
}

float task_controller::get_task_progress_percentage() const
{
  m_mutex->lock();
  float r;
  if (m_task_work_expected == 0)
  {
    r = 0.0f;
  }
  else
  {
    r = ((float) m_task_work_total / (float) m_task_work_expected);
  }
  m_mutex->unlock();
  return r;
}



task_status task_controller::get_task_status() const
{
  m_mutex->lock();
  auto r = m_task_status;
  m_mutex->unlock();
  return r;
}

int task_controller::get_task_expected_work() const
{
  m_mutex->lock();
  auto r = m_task_work_expected; 
  m_mutex->unlock();
  return r;
}

int task_controller::get_task_work_progress() const
{
  m_mutex->lock();
  auto r = m_task_work_total;
  m_mutex->unlock();
  return r;
}

void task_controller::cancel_task()
{
  m_mutex->lock();
  m_task_is_cancelled = true;
  m_mutex->unlock();
}


