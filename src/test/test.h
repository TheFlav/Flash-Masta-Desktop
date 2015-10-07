//
//  test.h
//  FlashMasta
//
//  Created by Dan on 10/6/15.
//  Copyright © 2015 7400 Circuits. All rights reserved.
//

#ifndef __TEST_H__
#define __TEST_H__

#include <string>
#include <list>
#include <functional>
#include <iosfwd>

class test
{
public:
  test(std::string name, bool mission_critical, std::function<bool (std::ostream& out, std::istream& in, std::ostream& err)> action);
  ~test();
  
  std::string name() const;
  bool is_mission_critical() const;
  test* add_pretest(test* t);
  test* add_posttest(test* t);
  
  bool run(std::ostream& out, std::istream& in, std::ostream& err);
  
protected:
  bool run_pretests(std::ostream& out, std::istream& in, std::ostream& err);
  bool run_posttests(std::ostream& out, std::istream& in, std::ostream& err);
  
private:
  test(const test&) = delete;
  test& operator=(const test&) = delete;
  
  const std::string                m_name;
  const bool                       m_critical;
  const std::function<bool (std::ostream& out, std::istream& in, std::ostream& err)> m_action;
  
  std::list<test*> m_pretests;
  std::list<test*> m_posttests;
};


#endif /* __TEST_H__ */
