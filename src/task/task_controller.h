/*! \file
 *  \brief File containing the declaration of the \ref task_controller class.
 *  
 *  File containing the header information and declaration of the
 *  \ref task_controller class. This file includes the minimal number of files
 *  necessary to use any instance of the \ref task_controller class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-06
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __TASK_CONTROLLER_H__
#define __TASK_CONTROLLER_H__

#include <mutex>

/*!
 *  \brief Enum indicating the current status of an operation. Can be used to
 *         indicate error states or state-transisions.
 */
enum task_status
{
  /*! \brief Task is in an irrecoverable error state. */
  ERROR,
  
  /*! \brief Task has not yet been started. */
  NOT_STARTED,
  
  /*! \brief Task is currently starting but has not yet started. */
  STARTING,
  
  /*! \brief Task has begun and is currently running as expected. */
  RUNNING,
  
  /*! \brief Task is in the process of stopping and performing cleanup. */
  STOPPING,
  
  /*! \brief Task has completed and is not in an error state. */
  COMPLETED,
  
  /*! \brief Task has been cancelled and may not have completed as expected. */
  CANCELLED
};



/*!
 *  \brief Class that provides long-running tasks an interface through which to
 *         communicate status updates and task progress.
 *  
 *  Class that provides long-running tasks an interface through which to
 *  communicate status updates and task progress.
 *  
 *  This class is thread-safe and thus can be used for communication between
 *  threads.
 */
class task_controller
{
public:
  
  /*!
   *  \brief The default class constructor.
   *  
   *  The default class constructor. Initializes member variables to their
   *  default states.
   */
  task_controller();
  
  /*!
   *  \brief The class copy constructor.
   *  
   *  The class copy constructor. Allows an existing \ref task_controller object
   *  to be deep-copied, ensuring the new instance is completely independent of
   *  the original instance.
   *  
   *  \param [in] other The instance to copy.
   */
  task_controller(const task_controller& other);
  
  /*!
   *  \brief The class destructor.
   *  
   *  The class destructor. Releases allocated memory and destroyes the class
   *  safely.
   */
  virtual ~task_controller();
  
  
  
  /*!
   *  \brief Callback for when the task begins execution.
   *  
   *  Callback for when the task has begun execution. Allows the task to
   *  communicate a numerical value of the total amount of expected work to
   *  accomplish, allowing the \ref task_controller to adjust any UI elements.
   *  This method should not be called more than once.
   *  
   *  \param [in] work_expected A numerical value indicating the total amount of
   *         work that the task expects to accomplish.
   */
  virtual void on_task_start(int work_expected);
  
  /*!
   *  \brief Callback for the task to communicate periodic status updates.
   *  
   *  Callback for the task to communicate periodic status updates. It is the
   *  tasks's responsibility to call this method when it can, which means that
   *  if the task for some reason is locked up or stuck, this method may not
   *  be called for a long time. This method can be called multiple times over
   *  the course of a task's execution.
   *  
   *  \param [in] status The current status of the task.
   *  \param [in] work_progress Progress made towards the expected work total
   *         since the last call to this method. This parameter will not
   *         indicate total progress. See \ref get_task_work_progress().
   */
  virtual void on_task_update(task_status status, int work_progress);
  
  /*!
   *  \brief Callback for when the task ends execution, either successful or
   *         unsuccessful.
   *  
   *  Callback for when the task ends execution, both successfully and
   *  unsuccessfully. Allows the task to communicate the final status of the
   *  task, as well as the total mount of work completed over the course of
   *  execution. This method should not be called more than once.
   *  
   *  To determine whether or not the task executed with or without errors,
   *  check the \ref status parameter.
   *  
   *  \param [in] status The final status code of the task.
   *  \param [in] work_total The total amount of work completed over the course
   *         of the task's execution.
   */
  virtual void on_task_end(task_status status, int work_total);
  
  /*!
   *  \brief Simple getter that allows the task to determine whether or not it
   *         should prematurely terminate. This method is primarily used for
   *         communication from the \ref task_controller to the task itself.
   *  
   *  Simple getter that allows the task to determine whether or not it should
   *  prematurely terminate. This method is primarily used for communcation from
   *  the \ref task_controller to the task itself. This method should not be
   *  overridden unless a more complicated method for determining if the task
   *  has been cancelled is required. Otherwise, the method will return `false`
   *  until the \ref cancel_task() method is called at least once. This method
   *  may be called multiple times over a task's execution.
   *  
   *  This method is to be used by the task as a way of determining if it should
   *  cancel itself, not as a way of determining if the task has completed
   *  cancelling.
   *  
   *  \return true if the task should terminate, false if not.
   */
  virtual bool is_task_cancelled() const;
  
  /*!
   *  \brief Gets a calculated percentage value of the amount of work that has
   *         been completed relative to the total expected work.
   *  
   *  Gets a calculated percentage value of the amount of work that has been
   *  completed since the task began relative to the expected total amount of
   *  work. This method is a convenience value that may not be perfectly
   *  accurate due to the nature of floating point values.
   *  
   *  \return A floating point value between 0 and 1 representing the
   *          approximate amount of work completed over the course of the task
   *          execution, where 0 indicates that no progress has been made and
   *          1 indicates that the task is fully complete.
   *  
   *  \see get_task_expected_work()
   *  \see get_task_work_progress()
   */
  virtual float get_task_progress_percentage() const;
  
  /*!
   *  \brief Gets the last reported status code of the task.
   *  
   *  Gets the last reported status code of the task. Because it is the task's
   *  responsibility for reporting its status, it is possible for the task's
   *  status to have changed since last reporting its status, rendering this
   *  method's return value out-of-date.
   *  
   *  \return The last reported status code of the task.
   */
  virtual task_status get_task_status() const;
  
  /*!
   *  \brief Gets the total work value estimated by the task.
   *  
   *  Gets the total work value estimated by the task when
   *  \ref on_task_start(int work_expected) was first called. This method should
   *  only be called after \ref on_task_start(int work_expected) has been
   *  called.
   *  
   *  \return Integer representation of the total amount of work expected to
   *          be accomplished by the task.
   */
  virtual int get_task_expected_work() const;
  
  /*!
   *  \brief Gets the current total work completed by the task.
   *  
   *  Gets the current total work completed by the task. This value may change
   *  multiple times over the course of a task's execution.
   *  
   *  \return Integer representation of the total amount of work completed by
   *          the task over the course of its execution.
   */
  virtual int get_task_work_progress() const;
  
  /*!
   *  \brief Cancels a running task.
   *  
   *  Cancels a running task by setting an internal flag that causes
   *  \ref is_task_cancelled() to return true. This method does not guarantee
   *  that the task will terminate in any reasonable amount of time if at all,
   *  as it is the responsibility of the task itself to check if it should
   *  self-terminate.
   */
  virtual void cancel_task();
  
  
  
private:
  
  /*!
   *  \brief The last reported status of the task.
   */
  task_status m_task_status;
  
  /*!
   *  \brief The total amount of work expected to be accomplished by the task.
   */
  int m_task_work_expected;
  
  /*!
   *  \brief The total amount of work accomplished by the task thus far.
   */
  int m_task_work_total;
  
  /*!
   *  \brief Flag indicating whether or not the task should self-terminate.
   */
  bool m_task_is_cancelled;
  
  /*!
   *  \brief Data lock used to make this class thread-safe.
   */
  std::mutex* m_mutex;
};

#endif /* defined(__TASK_CONTROLLER_H__) */
