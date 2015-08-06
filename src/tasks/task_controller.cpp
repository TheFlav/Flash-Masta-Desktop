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
    m_task_is_cancelled(false)
{
  // Nothing else to do
}

task_controller::task_controller(const task_controller& other)
  : m_task_status(other.task_status),
    m_task_work_expected(other.m_task_work_expected),
    m_task_work_total(other.m_task_work_total),
    m_task_is_cancelled(other.m_task_is_cancelled)
{
  // Nothing else to do
}

task_controller::~task_controller()
{
  // Nothing else to do
}

void task_controller::on_task_start(int work_expected)
{
  m_task_status = RUNNING;
  m_task_work_expected = work_expected;
}

void task_controller::on_task_update(task_status status, int work_progress)
{
  m_task_status = status;
  m_task_work_total += work_progress;
}

void task_controller::on_task_complete(task_status status, int work_total)
{
  m_task_status = status;
  m_task_work_total = work_total;
}

bool task_controller::is_task_cancelled() const
{
  return m_task_is_cancelled;
}

float task_controller::get_task_progress_percentage() const
{
  if (m_task_work_expected == 0)
  {
    return 0.0f;
  }
  else
  {
    return ((float) m_task_work_total / (float) m_task_work_expected);
  }
}



task_status task_controller::get_task_status() const
{
  return m_task_status;
}

int task_controller::get_task_expected_work() const
{
  return m_task_work_expected;
}

int task_controller::get_task_work_progress() const
{
  return m_task_work_total;
}

void task_controller::cancel_task()
{
  m_task_is_cancelled = true;
}


