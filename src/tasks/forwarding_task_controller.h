/*! \file
 *  \brief File containing the declaration of the
 *         \ref forwarding_task_controller class.
 *  
 *  File containing the header information and declaration of the
 *  \ref forwardign_task_controller class. This file includes the minimal number
 *  of files necessary to use any instance of the
 *  \ref forwarding_task_controller class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-06
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __FORWARDING_TASK_CONTROLLER_H__
#define __FORWARDING_TASK_CONTROLLER_H__

#include "task_controller.h"
#include <mutex>

/*!
 *  \brief A specialized implementation of \ref task_controller that allows
 *         sub-tasks to report their status to another task_controller without
 *         skewing the expected work values.
 *  
 *  A specialized implementation of the \ref task_controller that allows
 *  sub-tasks to report their status to another task_controller without skewing
 *  the expected work values of the parent task. Automatically scales progress
 *  updates to a predetermined work total.
 *  
 *  This class is thread-safe and thus can be used for communication between
 *  threads.
 *  
 *  \see task_controller
 */
class forwarding_task_controller: public task_controller
{
public:
  
  /*!
   *  \brief The default class constructor.
   *  
   *  The default class constructor. Performs data setup and initializes
   *  properties to default values. Accepts a pointer to an existing parent
   *  \ref task_controller object to which to forward task status updates.
   *  
   *  \param [in,out] receiver Pointer to an excisting parent
   *         \ref task_controller object to which to forward task status
   *         updates. This value should not be null.
   */
  forwarding_task_controller(task_controller* receiver);
  
  /*!
   *  \brief The class copy constructor.
   *  
   *  The class copy constructor. Constructs a new object to be a deep copy
   *  of another \ref forwarding_task_controller, allowing the new instance to
   *  be completely independent from the original instance.
   *  
   *  \param [in] other The original instance to copy.
   */
  forwarding_task_controller(const forwarding_task_controller& other);
  
  /*!
   *  \brief The class destructor.
   *  
   *  The class destructor. Performs cleanup and releases allocated memory for
   *  safe deletion.
   */
  virtual ~forwarding_task_controller();
  
  
  
  /*!
   *  \brief Callback for task status updates.
   *  
   *  Callback for task status updates. Automatically scales the reported work
   *  progress and invokes the
   *  \ref on_task_update(task_status status, int work_progress) method of the
   *  parent \ref task_controller object passed during construction.
   *  
   *  \param [in] status The reported status of the task.
   *  \param [in] work_progress The work progress reported by the task made
   *         since the last call to this method.
   *  
   *  \see task_controller::on_task_update(task_status status, int work_progress)
   */
  virtual void on_task_update(task_status status, int work_progress);
  
  /*!
   *  \brief Method used by the task to determine if it should self-terminate.
   *  
   *  Method used by the task to determine if it should self-terminate. Calls
   *  to this method are propagated up to this object's parent
   *  \ref task_controller object's \ref task_controller::is_task_cancelled()
   *  method. Therefore, this method truly reports whether or not the parent
   *  \ref task_controller has been cancelled.
   *  
   *  \return true if the parent \ref task_controller has been cancelled, false
   *          if not.
   *  
   *  \see task_controller::is_task_cancelled()
   */
  virtual bool is_task_cancelled() const;
  
  /*!
   *  \brief Sets the internal scaling values to use when forwarding calls to
   *  \ref on_task_update(task_status status, int work_progress).
   *  
   *  Sets the internal scaling values ot use when forwarding calls to
   *  \ref on_task_update(task_status status, int work_progress). This method
   *  should be called at least once before the task is started and should not
   *  be called once the task has begun.
   *  
   *  \param The total amount of work this object is allowed to report, allowing
   *         this task's total expected work to be scaled to a predetermined
   *         value.
   *  
   *  \return A reference to this same object as a convenience, allowing this
   *          class to be both constructed and the scaling value to be set on
   *          the same line.
   */
  forwarding_task_controller& scale_work_to(int work_target);
  
  
  
private:
  
  /*! \brief This object's parent \ref task_controller to forward updates to. */
  task_controller* const m_receiver;
  
  /*! \brief The adjusted expected work value to scale progress updates to. */
  int m_task_work_target;
  
  /*! \brief A thread lock used to keep this class thread-safe. */
  std::mutex* m_mutex;
};

#endif /* defined(__FORWARDING_TASK_CONTROLLER_H__) */
