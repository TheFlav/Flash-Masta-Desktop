//
//  unconfigured_exception.cpp
//  FlashMasta
//
//  Created by Dan on 8/4/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "unconfigured_exception.h"

namespace usb
{

unconfigured_exception::unconfigured_exception(const std::string& missing_config)
  : exception(missing_config + " not configured"), m_missing_config(missing_config)
{
  // Nothing else to do
}

unconfigured_exception::unconfigured_exception(const char* missing_config)
  : unconfigured_exception(std::string(missing_config))
{
  // Nothing else to do
}

unconfigured_exception::unconfigured_exception(const unconfigured_exception& other)
  : unconfigured_exception(other.m_missing_config)
{
  // Nothing else to do
}

const char* unconfigured_exception::missing_config() const
{
  return m_missing_config.c_str();
}

};
