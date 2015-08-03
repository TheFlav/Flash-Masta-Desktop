//
//  ngp_cartridge_tester.cpp
//  FlashMasta
//
//  Created by Dan on 7/30/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ngp_cartridge_tester.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ngp_linkmasta_device.h"
#include "cartridge/ngp_cartridge.h"
#include "libusb-1.0/libusb.h"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace usb;

ngp_cartridge_tester::ngp_cartridge_tester(std::istream& in, std::ostream& out, std::ostream& err)
  : in(in), out(out), err(err), m_fin(new ifstream)
{
  // Nothing else to do
}

ngp_cartridge_tester::~ngp_cartridge_tester()
{
  delete m_fin;
}

bool ngp_cartridge_tester::prepare()
{
  // Initialize libusb
  out << "Initializing libusb..."; out.flush();
  if (libusb_init(&m_libusb) != 0)
  {
    err << endl;
    err << "ERROR: An error occured when attempting to initialize libusb" << endl;
    return false;
  }
  out << "done." << endl;
  
  
  // Get handle to USB device
  out << "Searching for linkmasta device..."; out.flush();
  m_handle = libusb_open_device_with_vid_pid(m_libusb, 0x20A0, 0x4178);
  if (m_handle == nullptr)
  {
    err << endl;
    err << "ERROR: Unable to find linkmasta device or an error occured" << endl;
    libusb_exit(m_libusb);
    return false;
  }
  else
  {
    m_device = libusb_get_device(m_handle);
    libusb_ref_device(m_device);
    libusb_close(m_handle);
  }
  out << "done." << endl;
  
  
  // Initialize usb device
  out << "Initializing usb device..."; out.flush();
  try
  {
    m_usb = new libusb_usb_device(m_device);
    m_usb->init();
  }
  catch (exception& ex)
  {
    err << endl;
    err << "ERROR: An error occured when attempting to initialize usb device" << endl;
    return false;
  }
  out << "done." << endl;
  
  
  // Initialize linkmasta device
  out << "Initializing linkmasa device..."; out.flush();
  try
  {
    m_linkmasta = new ngp_linkmasta_device(m_usb);
    m_linkmasta->init();
  }
  catch (exception& ex)
  {
    err << endl;
    err << "ERROR: An error occured when attempting to initialize linkmasta device" << endl;
    return false;
  }
  out << "done." << endl;
  
  // Initialize input file
  m_fin->open("/Users/Dan/Documents/ROMs/pbm.ngp", ios::binary);
  
  return true;
}

void ngp_cartridge_tester::pretests()
{
  out << "Beginning ngp_cartridge_tester tests" << endl;
}

bool ngp_cartridge_tester::run_tests()
{
  int test_num = 0;
  bool success = true;
  ifstream& fin = *m_fin;
  ofstream fout;
  
  //////////////////////////////////////////////////////////// 1
  
  success = true;
  
  try
  {
    m_cartridge = new ngp_cartridge(m_linkmasta);
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 2
  
  success = true;
  
  try
  {
    m_cartridge->init();
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 3
  
  success = true;
  
  const cartridge_descriptor* desc;
  try
  {
    desc = m_cartridge->descriptor();
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  out << "    Cartridge system: " << desc->type << endl;
  out << "    Cartridge size:   " << desc->num_bytes << " B (" << ((double) desc->num_bytes / 0x20000) << " Mib)" << endl;
  out << "    Number of chips:  " << desc->num_chips << endl;
  
  for (unsigned int i = 0; i < desc->num_chips; ++i)
  {
    const cartridge_descriptor::chip_descriptor* chip = desc->chips[i];
    out << "    Chip " << chip->chip_num << endl;
    out << "      Manufacturer ID:  0x" << hex << uppercase << chip->manufacturer_id << nouppercase << dec << endl;
    out << "      Device ID:        0x" << hex << uppercase << chip->device_id << nouppercase << dec << endl;
    out << "      Chip size:        " << chip->num_bytes << " B (" << ((double) chip->num_bytes / 0x20000) << " Mib)" << endl;
    out << "      Number of blocks: " << chip->num_blocks << endl;
    
    for (unsigned int j = 0; j < chip->num_blocks; ++j)
    {
      const cartridge_descriptor::chip_descriptor::block_descriptor* block = chip->blocks[j];
      out << "      Block " << block->block_num << endl;
      out << "        Address:   0x" << hex << uppercase << setfill('0') << setw(6) << block->base_address << setfill(' ') << nouppercase << dec << endl;
      out << "        Size:      " << block->num_bytes << " B (" << (block->num_bytes / 0x400) << " KiB)" << endl;
      out << "        Protected: " << (block->is_protected ? "YES" : "NO") << endl;
    }
  }
  
  //////////////////////////////////////////////////////////// 4
  
  success = true;
  
  fout.open("/Users/Dan/Documents/ROMs/bkp.ngp", ios::binary);
  
  try
  {
    if (fout.good())
    {
      m_cartridge->write_cartridge_to_file(fout);
    }
    else
    {
      success = false;
    }
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  fout.close();
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 5
  
  success = true;
  
  fin.seekg(0, fin.beg);
  try
  {
    if (fin.good())
    {
      m_cartridge->write_file_to_cartridge(fin);
    }
    else
    {
      success = false;
    }
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
//  out << "  Test " << ++test_num << " SKIPPED" << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 6
  
  success = true;
  
  fin.seekg(0, fin.beg);
  try
  {
    if (fin.good())
    {
      m_cartridge->compare_file_to_cartridge(fin);
    }
    else
    {
      success = false;
    }
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 7
  
  success = true;
  
  try
  {
    delete m_cartridge;
  }
  catch (exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// END
  
  return (m_test_count[1] == 0 ? true : false);
}

void ngp_cartridge_tester::posttests()
{
  out << "Concluded ngp_cartridge_tester tests" << endl;
}

void ngp_cartridge_tester::cleanup()
{
  m_fin->close();
  delete m_fin;
  
  m_linkmasta->close();
  delete m_linkmasta;
  
  m_usb->close();
  delete m_usb;
  
  libusb_unref_device(m_device);
  libusb_exit(m_libusb);
}

void ngp_cartridge_tester::results()
{
  out << "Passed " << m_test_count[0] << " of " << (m_test_count[0] + m_test_count[1]) << endl << endl;
}


