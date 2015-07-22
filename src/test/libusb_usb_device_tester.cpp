//
//  libusb_usb_device_tester.cpp
//  FlashMasta
//
//  Created by Dan on 7/22/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "libusb_usb_device_tester.h"
#include "libusb-1.0/libusb.h"
#include <iomanip>

using namespace std;

libusb_usb_device_tester::libusb_usb_device_tester(istream& in, ostream& out, ostream& err)
  : in(in), out(out), err(err)
{
  // Nothing else to do
}

bool libusb_usb_device_tester::prepare()
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
  if (m_handle == NULL)
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
  
  
  return true;
}

void libusb_usb_device_tester::pretests()
{
  out << "Beginning libusb_usb_device_tester tests" << endl;
}

bool libusb_usb_device_tester::run_tests()
{
  int test_num = 0;
  bool success = true;
  
  ////////////////////////////////////////////////////////////  
  
  out << "  Test " << ++test_num << ": "; out.flush();
  success = true;
  
  m_test_subject = new libusb_usb_device(m_device);
  
  out << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  ////////////////////////////////////////////////////////////
  
  out << "  Test " << ++test_num << ": "; out.flush();
  success = true;
  
  m_test_subject->open();
  
  out << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  ////////////////////////////////////////////////////////////
  
  out << "  Test " << ++test_num << ": "; out.flush();
  success = true;
  
  m_test_subject->close();
  
  out << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  ////////////////////////////////////////////////////////////
  
  out << "  Test " << ++test_num << ": "; out.flush();
  success = true;
  
  delete m_test_subject;
  
  out << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  ////////////////////////////////////////////////////////////
  
  return (m_test_count[1] == 0 ? true : false);
}

void libusb_usb_device_tester::posttests()
{
  out << "Concluded libusb_usb_device_tester tests" << endl;
}

void libusb_usb_device_tester::cleanup()
{
  libusb_unref_device(m_device);
  libusb_exit(m_libusb);
}

void libusb_usb_device_tester::results()
{
  out << "Passed " << m_test_count[0] << " of " << m_test_count[1] << endl << endl;
}

