//
//  ws_linkmasta_tester.cpp
//  FlashMasta
//
//  Created by Dan on 7/30/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ws_linkmasta_tester.h"

#include "test.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ws_linkmasta_device.h"
#include "libusb-1.0/libusb.h"

#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;
using namespace usb;

ws_linkmasta_tester::ws_linkmasta_tester(std::istream& in, std::ostream& out, std::ostream& err)
  : tester("ws_linkmasta_tester"), in(in), out(out), err(err), m_fin(new ifstream)
{
  typedef function<bool (std::ostream&, std::istream&, std::ostream&)> action;
  
  // REUSABLE ACTIONS
  action a_usbdevice = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Constructing usb device..."; out.flush();
    m_usb = new libusb_usb_device(m_device);
    out << "done." << endl;
    
    out << "  Initializing usb device..."; out.flush();
    m_usb->init();
    out << "done." << endl;
    
    return true;
  };
  
  action a_construct = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    if (!a_usbdevice(out, in, err)) return false;
    
    out << "  Constructing instance..."; out.flush();
    m_linkmasta = new ws_linkmasta_device(m_usb);
    out << "done." << endl;
    return true;
  };
  
  action a_destruct = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Destructing instance..."; out.flush();
    delete m_linkmasta;
    m_linkmasta = nullptr;
    out << "done." << endl;
    return true;
  };
  
  action a_initialize = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Initializing device..."; out.flush();
    m_linkmasta->init();
    out << "done." << endl;
    return true;
  };
  
  action a_open = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Opening device..."; out.flush();
    m_linkmasta->open();
    out << "done." << endl;
    return true;
  };
  
  action a_close = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Closing device..."; out.flush();
    m_linkmasta->close();
    out << "done." << endl;
    return true;
  };
  
  action a_firmware = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Fetching firmware version..."; out.flush();
    auto version = m_linkmasta->firmware_version();
    out << "done." << endl;
    out << "    Firmware version: " << version << endl;
    return true;
  };
  
  // CONSTRUCTION
  add_test(new test("construct ws_linkmasta_device object", false, a_construct));
  
  // DESTRUCTION
  add_test(new test("destruct ws_linkmasta_device object", false, a_destruct));
  
  // INITIALIZATION
  add_test(new test("initialize ws_linkmasta_device object", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  // OPENING
  add_test(new test("open ws_linkmasta_device object", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_open(out, in, err));
  }));
  
  // CLOSING
  add_test(new test("close ws_linkmasta_device object", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_close(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  // FETCHING FIRMWARE VERSION
  add_test(new test("fetch device firmware version", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_open(out, in, err)
            && a_firmware(out, in, err)
            && a_close(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  // FETCHING FIRMWARE VERSION TWICE
  add_test(new test("fetch device firmware version twice", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_open(out, in, err)
            && a_firmware(out, in, err)
            && a_firmware(out, in, err)
            && a_close(out, in, err)
            && a_destruct(out, in, err));
  }));
}

ws_linkmasta_tester::~ws_linkmasta_tester()
{
  delete m_fin;
}

bool ws_linkmasta_tester::prepare()
{
  out << "Beginning ws_linkmasta_tester test preparations" << endl;
  
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
  
  // Initialize input file
  m_fin->open("/Users/Dan/Documents/ROMs/pacman_cut.ngp", ios::binary);
  
  return true;
}

void ws_linkmasta_tester::pretests()
{
  out << "Beginning ws_linkmasta_tester tests" << endl;
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


