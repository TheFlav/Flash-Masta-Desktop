//
//  ws_linkmasta_tester.cpp
//  FlashMasta
//
//  Created by Dan on 7/30/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ws_linkmasta_tester.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ws_linkmasta_device.h"
#include "libusb-1.0/libusb.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;
using namespace usb;

ws_linkmasta_tester::ws_linkmasta_tester(std::istream& in, std::ostream& out, std::ostream& err)
  : in(in), out(out), err(err), m_fin(new ifstream)
{
  // Nothing else to do
}

ws_linkmasta_tester::~ws_linkmasta_tester()
{
  delete m_fin;
}

bool ws_linkmasta_tester::prepare()
{
  out << "Beginning ws_linkmasta_tester test preparationsx" << endl;
  
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
  out << "Searching for target device..."; out.flush();
  m_handle = libusb_open_device_with_vid_pid(m_libusb, 0x20A0, 0x4252);
  if (m_handle == nullptr)
  {
    err << endl;
    err << "ERROR: Unable to find target device or an error occured" << endl;
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
  catch (std::exception& ex)
  {
    err << endl;
    err << ex.what();
    err << "ERROR: An error occured when attempting to initialize usb device" << endl;
    return false;
  }
  out << "done." << endl;
  
  // Initialize input file
  m_fin->open("/Users/Dan/Documents/ROMs/pacman_cut.ngp", ios::binary);
  
  return true;
}

void ws_linkmasta_tester::pretests()
{
  out << "Beginning ws_linkmasta_tester tests" << endl;
}

bool ws_linkmasta_tester::run_tests()
{
  int test_num = 0;
  bool success = true;
  //ifstream& fin = *m_fin;
  ofstream fout;
  
  ////////////////////////////////////////////////////////////
  
  // Create test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Constructing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta = new ws_linkmasta_device(m_usb);
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to construct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Delete test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Destructing ws_linkmasta_device..."; out.flush();
    try
    {
      delete m_linkmasta;
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to destruct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Create usb object for test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    // Initialize usb device
    out << "    Initializing usb device..."; out.flush();
    try
    {
      m_usb = new libusb_usb_device(m_device);
      m_usb->init();
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to initialize usb device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Create test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Constructing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta = new ws_linkmasta_device(m_usb);
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to construct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Initialize test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Initializing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->init();
    }
    catch (std::exception& e)
    {
      err << endl;
      err << e.what();
      err << "    ERROR: An error occured when attempting ot initialize ws_linkmasta_device" << endl;
      success = false;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Delete test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Destructing ws_linkmasta_device..."; out.flush();
    try
    {
      delete m_linkmasta;
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to destruct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Create usb object for test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    // Initialize usb device
    out << "    Initializing usb device..."; out.flush();
    try
    {
      m_usb = new libusb_usb_device(m_device);
      m_usb->init();
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to initialize usb device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Create test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Constructing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta = new ws_linkmasta_device(m_usb);
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to construct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Initialize test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Initializing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->init();
    }
    catch (std::exception& e)
    {
      err << endl;
      err << e.what();
      err << "    ERROR: An error occured when attempting ot initialize ws_linkmasta_device" << endl;
      success = false;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Opening test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Opening connection to ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->open();
    }
    catch (std::exception& e)
    {
      out << e.what() << endl;
      success = false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Closing test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Closing connection to ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->close();
    }
    catch (std::exception& e)
    {
      out << e.what() << endl;
      success = false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Delete test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Destructing ws_linkmasta_device..."; out.flush();
    try
    {
      delete m_linkmasta;
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to destruct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Create usb object for test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    // Initialize usb device
    out << "    Initializing usb device..."; out.flush();
    try
    {
      m_usb = new libusb_usb_device(m_device);
      m_usb->init();
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to initialize usb device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Create test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Constructing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta = new ws_linkmasta_device(m_usb);
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to construct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Initialize test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Initializing ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->init();
    }
    catch (std::exception& e)
    {
      err << endl;
      err << e.what();
      err << "    ERROR: An error occured when attempting ot initialize ws_linkmasta_device" << endl;
      success = false;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Opening test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Opening connection to ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->open();
    }
    catch (std::exception& e)
    {
      out << e.what() << endl;
      success = false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Fetching firmware version from test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Fetching firmware version from ws_linkmasta_device..."; out.flush();
    ws_linkmasta_device::version_t version;
    try
    {
      version = m_linkmasta->firmware_version();
    }
    catch (std::exception& e)
    {
      out << e.what() << endl;
      success = false;
    }
    out << "done." << endl;
    out << "    firmware version: " << version << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Fetching firmware version from test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Fetching firmware version from ws_linkmasta_device..."; out.flush();
    ws_linkmasta_device::version_t version;
    try
    {
      version = m_linkmasta->firmware_version();
    }
    catch (std::exception& e)
    {
      out << e.what() << endl;
      success = false;
    }
    out << "done." << endl;
    out << "    firmware version: " << version << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Closing test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    out << "    Closing connection to ws_linkmasta_device..."; out.flush();
    try
    {
      m_linkmasta->close();
    }
    catch (std::exception& e)
    {
      out << e.what() << endl;
      success = false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  // Delete test subject
  {
    success = true;
    test_num++;
    out << "  RUNNING TEST " << test_num << endl;
    
    // Create new ws_linkmasta_device
    out << "    Destructing ws_linkmasta_device..."; out.flush();
    try
    {
      delete m_linkmasta;
    }
    catch (std::exception& ex)
    {
      err << endl;
      err << ex.what();
      err << "    ERROR: An error occured when attempting to destruct ws_linkmasta_device" << endl;
      return false;
    }
    out << "done." << endl;
    
    out << "  TEST " << test_num << " " << (success ? "PASSED" : "FAILED") << endl;
    m_test_count[success ? 0 : 1]++;
  }
  
  ////////////////////////////////////////////////////////////
  
  //// Backup cartridge save game ////
  
  //////////////////////////////////////////////////////////// END
  
  return (m_test_count[1] == 0 ? true : false);
}

void ws_linkmasta_tester::posttests()
{
  out << "Concluded ws_linkmasta_tester tests" << endl;
}

void ws_linkmasta_tester::cleanup()
{
  m_fin->close();
  delete m_fin;
  
  m_usb->close();
  delete m_usb;
  
  libusb_unref_device(m_device);
  libusb_exit(m_libusb);
}

void ws_linkmasta_tester::results()
{
  out << "Passed " << m_test_count[0] << " of " << (m_test_count[0] + m_test_count[1]) << endl << endl;
}


