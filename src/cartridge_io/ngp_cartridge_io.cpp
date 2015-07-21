//
//  ngp_cartridge_io.cpp
//  FlashMasta
//
//  Created by Dan on 7/16/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ngp_cartridge_io.h"

ngp_cartridge_io::ngp_cartridge_io()
  : m_cartridge(NULL)
{
  // Nothing else to do
}

ngp_cartridge_io::~ngp_cartridge_io()
{
  delete m_cartridge;
}

unsigned int ngp_cartridge_io::read_bytes(unsigned int address, char* buffer, unsigned int n)
{
  return 0;
}

unsigned int ngp_cartridge_io::write_bytes(unsigned int address, const char* buffer, unsigned int n)
{
  return 0;
}

unsigned int ngp_cartridge_io::flash_from_stream(std::istream& in)
{
  return 0;
}

unsigned int ngp_cartridge_io::flash_to_stream(std::ostream& out)
{
  return 0;
}

const ngp_cartridge* ngp_cartridge_io::get_cartridge_info()
{
  return m_cartridge;
}
