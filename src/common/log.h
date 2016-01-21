/*! \file
 *  \brief File containing declarations of logging functions.
 *
 *  File containing declarations of logging functions and any prerequisites,
 *  such as the \ref log_level enum.
 *  
 *  \author Daniel Andrus
 *  \date 2016-01-20
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __LOG_H__
#define __LOG_H__

/*!
 *  \brief Enum representing the logging level for a log message.
 */
enum log_level
{
  /*!
   *  \brief Debug-level log messages that should not appear in the final
   *         product, but should be used to aid in debugging.
   */
  DEBUG,
  
  /*!
   *  \brief Log messages for verbose logging that should not appear in the
   *         final product, but should be used for higher-level debugging.
   */
  VERBOSE,
  
  /*!
   *  \brief Generic log messages that can appear in the final product and can
   *         should be used for high-level process logging. This is the default
   *         logging level if no log level is specified.
   */
  INFO
};



/*!
 *  \brief Initialize the logging system.
 */
void log_init();

/*!
 *  \brief Deinitialize the logging system, closing files, and flushing streams.
 */
void log_deinit();



/*!
 *  \brief Starts a new log entry and flushes it to the output. Inserts a
 *         newline at the beginning in order to separate this message from
 *         previous messages. Log entries are made at level \ref log_level::INFO
 *         by default.
 *  
 *  \param [in] message The new log entry.
 */
void log(const char* message); /* level = INFO */

/*!
 *  \brief Starts a new log entry and flushes it to the output. Inserts a
 *         newline at the beginning in order to separate this message from
 *         previous messages.
 * 
 *  \param [in] level The maximum level to display this log entry at. Dependant
 *         on the MIN_LOG_LEVEL constant.
 *  \param [in] message The new log entry.
 */
void log(log_level level, const char* message);

/*!
 *  \brief Continues log message from previous \ref log(const char*) or
 *         \ref log(log_level, const char*) call.
 * 
 *  \param message Text to append to previous entry.
 */
void log_cont(const char* message);

/*!
 *  \brief Prints a log message and indents the log level by 1, allowing
 *         for layered log entries. Log entries are made at level
 *         \ref log_level::INFO by default.
 * 
 *  \param [in] message The new log entry.
 */
void log_start(const char* message);

/*!
 *  \brief Prints a log message and indents the log level by 1, allowing for
 *         layered log entries.
 * 
 *  \param [in] level The maximum level to display this log entry at. Dependant
 *         on the MIN_LOG_LEVEL constant.
 *  \param [in] message The new log entry.
 */
void log_start(log_level level, const char* message);

/*!
 *  \brief Decreases log indentation level and prints no end message.
 */
void log_end();

/*!
 *  \brief Decreases log indentation level and prints an end message. If no log
 *         entries at the current log level, then this message will be displayed
 *         on the same level as the message passed to
 *         \ref log_start(const char*), otherwise it will be printed on a
 *         separate line.
 * 
 *  \param [in] message The ending message to print to the log.
 */
void log_end(const char* message);

#endif /* defined(__LOG_H__) */
