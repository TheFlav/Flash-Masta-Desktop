//
//  test.cpp
//  FlashMasta
//
//  Created by Dan on 10/6/15.
//  Copyright © 2015 7400 Circuits. All rights reserved.
//

#include "test.h"

#include <iostream>

using namespace std;

test::test(std::string name, bool mission_critical, std::function<bool (std::ostream& out, std::istream& in, std::ostream& err)> action)
  : m_name(name), m_critical(mission_critical), m_action(action)
{
  // Nothing else to do
}

test::~test()
{
  // Nothing else to do
}

std::string test::name() const
{
  return m_name;
}

bool test::is_mission_critical() const
{
  return m_critical;
}

test* test::add_pretest(test* t)
{
  m_pretests.push_back(t);
  return this;
}

test* test::add_posttest(test* t)
{
  m_pretests.push_back(t);
  return this;
}

bool test::run(std::ostream& out, std::istream& in, std::ostream& err)
{
  bool success;
  
  if (m_pretests.size() > 0 && !run_pretests(out, in, err))
  {
    return false;
  }
  
  try
  {
    success = m_action(out, in, err);
  }
  catch (std::exception& ex)
  {
    err << "  ERROR ENCOUNTERED\n  " << ex.what() << endl;
    success = false;
  }
  
  if (m_posttests.size() > 0 && !run_posttests(out, in, err))
  {
    return false;
  }
  
  return success;
}

bool test::run_pretests(std::ostream& out, std::istream& in, std::ostream& err)
{
  out << "  executing pretests" << endl;
  
  int i = 0;
  for (auto t : m_pretests)
  {
    i++;
    try
    {
      out << "  RUNNING PRETEST " << i << ": " << t->m_name;
      if (t->run(out, in, err))
      {
        out << "  PRETEST PASSED" << endl;
      }
      else
      {
        out << "  PRETEST FAILED" << endl;
        return false;
      }
    }
    catch (std::exception& ex)
    {
      err << "  ERROR OCCURED DURING PRETEST " << i << ": " << t->m_name << endl;
      return false;
    }
  }
  
  out << "  all pretests passed" << endl;
  return true;
}

bool test::run_posttests(std::ostream& out, std::istream& in, std::ostream& err)
{
  out << "  executing posttests" << endl;
  
  int i = 0;
  for (auto t : m_posttests)
  {
    i++;
    try
    {
      out << "  RUNNING POSTTEST " << i << ": " << t->m_name;
      if (t->run(out, in, err))
      {
        out << "  POSTTEST PASSED" << endl;
      }
      else
      {
        out << "  POSTTEST FAILED" << endl;
        return false;
      }
    }
    catch (std::exception& ex)
    {
      err << "  ERROR OCCURED DURING POSTTEST " << i << ": " << t->m_name << endl;
      return false;
    }
  }
  
  out << "  all posttests passed" << endl;
  return true;
}
