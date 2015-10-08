//
//  tester.cpp
//  FlashMasta
//
//  Created by Dan on 10/6/15.
//  Copyright © 2015 7400 Circuits. All rights reserved.
//

#include "tester.h"

#include "test.h"

#include <iostream>

using namespace std;

tester::tester(std::string name)
  : m_name(name), m_tests(), m_passed(0)
{
  // Nothing else to do
}

tester::~tester()
{
  for (test* t : m_tests)
  {
    delete t;
  }
}

std::string tester::name() const
{
  return m_name;
}

bool tester::run_tests(std::ostream& out , std::istream& in, std::ostream& err)
{
  int i = 0;
  m_passed = 0;
  bool good = true;
  
  for (auto t : m_tests)
  {
    i++;
    
    out << "RUNNING TEST " << i << ": " << t->name() << endl;
    
    if (t->run(out, in, err))
    {
      out << "TEST PASSED" << endl;
      m_passed++;
    }
    else
    {
      out << "TEST FAILED" << endl;
      good = false;
      
      if (t->is_mission_critical())
      {
        out << "\nCRITICAL TEST FAILED. ABORTING\n" << endl;
        return false;
      }
    }
  }
  
  return good;
}

void tester::results(std::ostream& out)
{
  out << "Passed " << m_passed << " of " << m_tests.size() << endl << endl;
}

void tester::add_test(test* t)
{
  m_tests.push_back(t);
}
