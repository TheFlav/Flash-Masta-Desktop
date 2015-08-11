//
//  tester.h
//  FlashMasta
//
//  Created by Dan on 7/22/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __TESTER_H__
#define __TESTER_H__

class tester
{
public:
  virtual ~tester(){};
  
  virtual bool prepare() = 0;
  virtual void pretests() = 0;
  virtual bool run_tests() = 0;
  virtual void posttests() = 0;
  virtual void cleanup() = 0;
  virtual void results() = 0;
};

#endif /* defined(__TESTER_H__) */
