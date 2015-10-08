//
//  tester.h
//  FlashMasta
//
//  Created by Dan on 7/22/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __TESTER_H__
#define __TESTER_H__

#include <string>
#include <iosfwd>
#include <list>

class test;

class tester
{
public:
  tester(std::string name);
  virtual ~tester();
  
  std::string name() const;
  
  virtual bool prepare() = 0;
  virtual void pretests() = 0;
  virtual bool run_tests(std::ostream& out , std::istream& in, std::ostream& err);
  virtual void posttests() = 0;
  virtual void cleanup() = 0;
  virtual void results(std::ostream& out);
  
protected:
  virtual void add_test(test* t);
  
  const std::string m_name;
  std::list<test*> m_tests;
  int m_passed;
};

#endif /* defined(__TESTER_H__) */
