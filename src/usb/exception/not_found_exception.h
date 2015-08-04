//
//  not_found_exception.h
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __NOT_FOUND_EXCEPTION_H__
#define __NOT_FOUND_EXCEPTION_H__

#include "exception.h"
#include <string>

namespace usb
{

class not_found_exception: public exception
{
public:
  explicit not_found_exception(const std::string& item_not_found);
  explicit not_found_exception(const char* item_not_found);
  not_found_exception(const not_found_exception& other);
  
  const char* item_not_found() const;
  
private:
  std::string m_item_not_found;
};

};

#endif /* defined(__NOT_FOUND_EXCEPTION_H__) */
