//
//  not_found_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "not_found_exception.h"

using namespace usb;

not_found_exception::not_found_exception(const std::string& item_not_found)
  : exception(item_not_found + " not found"), m_item_not_found(item_not_found)
{
  // Nothing else to do
}

not_found_exception::not_found_exception(const char* item_not_found)
  : exception(std::string(item_not_found))
{
  // Nothing else to do
}

not_found_exception::not_found_exception(const not_found_exception& other)
  : exception(other.m_item_not_found)
{
  // Nothing else to do
}

const char* not_found_exception::item_not_found() const
{
  return m_item_not_found.c_str();
}
