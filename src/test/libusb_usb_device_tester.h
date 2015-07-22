//
//  libusb_usb_device_tester.h
//  FlashMasta
//
//  Created by Dan on 7/22/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __LIBUSB_USB_DEVICE_TESTER_H__
#define __LIBUSB_USB_DEVICE_TESTER_H__

#include "tester.h"
#include "usb_device/libusb_usb_device.h"
#include <iostream>
#include "libusb-1.0/libusb.h"

class libusb_usb_device_tester: public tester
{
public:
  libusb_usb_device_tester(std::istream& in, std::ostream& out, std::ostream& err);
  ~libusb_usb_device_tester(){};
  
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
  
  libusb_context*       m_libusb;
  libusb_device*        m_device;
  libusb_device_handle* m_handle;
  libusb_usb_device*    m_test_subject;
  
  int                   m_test_count[2] = {0};
};

#endif /* defined(__LIBUSB_USB_DEVICE_TESTER_H__) */
