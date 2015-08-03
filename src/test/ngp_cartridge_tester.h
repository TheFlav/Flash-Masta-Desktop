//
//  ngp_cartridge_tester.h
//  FlashMasta
//
//  Created by Dan on 7/30/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __NGP_CARTRIDGE_TESTER_H__
#define __NGP_CARTRIDGE_TESTER_H__

#include "tester.h"
#include <iosfwd>
#include "usb/usbfwd.h"

class cartridge;
class linkmasta_device;
struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

class ngp_cartridge_tester: public tester
{
public:
  ngp_cartridge_tester(std::istream& in, std::ostream& out, std::ostream& err);
  ~ngp_cartridge_tester();
  
  bool prepare();
  void pretests();
  bool run_tests();
  void posttests();
  void cleanup();
  void results();

private:
  std::istream& in;
  std::ostream& out;
  std::ostream& err;

  std::ifstream*        m_fin;
  usb::usb_device*      m_usb;
  linkmasta_device*     m_linkmasta;
  cartridge*            m_cartridge;

  libusb_context*       m_libusb;
  libusb_device*        m_device;
  libusb_device_handle* m_handle;
  
  int                   m_test_count[2] = {0};

};

#endif /* defined(__NGP_CARTRIDGE_TESTER_H__) */
