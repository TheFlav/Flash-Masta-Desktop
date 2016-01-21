/*! \file
 *  \brief File containing the definitions for logging functions.
 *
 *  File containing the definitions for logging functions.
 *  
 *  \author Daniel Andrus
 *  \date 2016-01-20
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#include "log.h"

#ifndef DISABLE_LOGGING
#include <fstream>
#include <iomanip>
#include <ctime>
#include <stack>

#ifndef MIN_LOG_LEVEL

#ifdef DEBUG
#define MIN_LOG_LEVEL log_level::DEBUG
#else
#define MIN_LOG_LEVEL log_level::INFO
#endif // defined(DEBUG)
#endif // defined(MIN_LOG_LEVEL)

#define INDENT_STRING "  "
static int        log_prev_level = -1;
static int        log_indent_level = 0;
static bool       log_current_indent_level_empty = true;
static std::stack<log_level> log_level_stack;
static std::ofstream lout;

#endif // defined(DISABLE_LOGGING)



void log_init()
{
#ifndef DISABLE_LOGGING
  lout.open("log.txt", std::ofstream::out | std::ofstream::app);
#endif
}

void log_deinit()
{
#ifndef DISABLE_LOGGING
  lout << '\n';
  lout.flush();
  lout.close();
#endif
}



void log(const char* message)
{
#ifndef DISABLE_LOGGING
  log(log_level::INFO, message);
#else
  (void) message;
#endif
}

void log(log_level level, const char* message)
{
#ifndef DISABLE_LOGGING
  if (level >= MIN_LOG_LEVEL)
  {
    // Print newline
    lout << '\n';
    
    // Print timestamp
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    lout << std::put_time(&tm, "%c") << ": ";
    
    // Display indentation
    for (int i = 0; i < log_indent_level; i++)
    {
      lout << INDENT_STRING;
    }
    
    // Print to log and flush output
    lout << message;
    lout.flush();
    
    log_current_indent_level_empty = false;
  }
  log_prev_level = level;
#else
  (void) level;
  (void) message;
#endif
}

void log_cont(const char* message)
{
#ifndef DISABLE_LOGGING
 if (log_prev_level >= MIN_LOG_LEVEL)
 {
   // Print to log and flush output
   lout << message;
   lout.flush();
 }
#else
  (void) message;
#endif
}

void log_start(const char* message)
{
#ifndef DISABLE_LOGGING
  log_start(log_level::INFO, message);
#else
  (void) message;
#endif
}

void log_start(log_level level, const char* message)
{
#ifndef DISABLE_LOGGING
  log(level, message);
  log_indent_level++;
  log_current_indent_level_empty = true;
  log_level_stack.push(level);
#else
  (void) level;
  (void) message;
#endif
}

void log_end()
{
#ifndef DISABLE_LOGGING
  log_end(nullptr);
#endif
}

void log_end(const char* message)
{
#ifndef DISABLE_LOGGING
  if (log_indent_level > 0) log_indent_level--;
  if (message != nullptr)
  {
    if (log_current_indent_level_empty)
    {
      log_cont(message);
    }
    else
    {
      log(log_level_stack.top(), message);
    }
  }
  log_level_stack.pop();
  log_current_indent_level_empty = false;
#else
  (void) message;
#endif
}
