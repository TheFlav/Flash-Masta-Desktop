//
//  ws_linkmasta_tester.h
//  FlashMasta
//
//  Created by Dan on 7/30/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __WS_LINKMASTA_TESTER_H__
#define __WS_LINKMASTA_TESTER_H__

#include "tester.h"
#include <iosfwd>
#include "usb/usbfwd.h"

class cartridge;
class linkmasta_device;
struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

class ws_linkmasta_tester: public tester
{
public:
  ws_linkmasta_tester(std::istream& in, std::ostream& out, std::ostream& err);
  ~ws_linkmasta_tester();
  
  bool prepare();
  void pretests();
  void posttests();
  void cleanup();

private:
  std::istream& in;
  std::ostream& out;
  std::ostream& err;

  std::ifstream*        m_fin;
  usb::usb_device*      m_usb;
  linkmasta_device*     m_linkmasta;

  libusb_context*       m_libusb;
  libusb_device*        m_device;
  libusb_device_handle* m_handle;
  
  int                   m_test_count[2] = {0};

};

#endif /* defined(__WS_LINKMASTA_TESTER_H__) */
