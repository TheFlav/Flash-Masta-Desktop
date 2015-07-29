//
//  libusb_usb_device_tester.cpp
//  FlashMasta
//
//  Created by Dan on 7/22/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "libusb_usb_device_tester.h"
#include "libusb-1.0/libusb.h"
#include "hardware/AVR/messages.h"
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
  const libusb_usb_device::device_description* desc;
  
  //////////////////////////////////////////////////////////// 1 
  
  success = true;
  
  try
  {
    m_test_subject = new libusb_usb_device(m_device);
  }    
  catch (std::exception& e)
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
    m_test_subject->init();
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 3
  
  success = true;
  
  try
  {
    desc = m_test_subject->get_device_description();
    success = desc != nullptr;
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  out << "    Device class:   " << desc->device_class << endl;
  out << "    Vendor ID:      " << hex << uppercase << desc->vendor_id << nouppercase << dec << endl;
  out << "    Product ID:     " << hex << uppercase << desc->product_id << nouppercase << dec << endl;
  out << "    Configurations: " << desc->num_configurations << endl;
  
  for (unsigned int i = 0; i < desc->num_configurations; ++i)
  {
    const libusb_usb_device::device_configuration* conf = desc->configurations[i];
    out << "    Configuration " << i << endl;
    out << "      Interfaces: " << conf->num_interfaces << endl;
    
    for (unsigned int j = 0; j < conf->num_interfaces; ++j)
    {
      const libusb_usb_device::device_interface* inter = conf->interfaces[j];
      out << "      Interface " << j << endl;
      out << "        Alt settings: " << inter->num_alt_settings << endl;
      
      for (unsigned int k = 0; k < inter->num_alt_settings; ++k)
      {
        const libusb_usb_device::device_alt_setting* alt = inter->alt_settings[k];
        out << "        Alt setting " << k << endl;
        out << "          Interface num: " << alt->interface_num << endl;
        out << "          Endpoints:     " << alt->num_endpoints << endl;
        
        for (unsigned int l = 0; l < alt->num_endpoints; ++l)
        {
          const libusb_usb_device::device_endpoint* ep = alt->endpoints[l];
          out << "          Endpoint " << (l + 1) << endl;
          out << "            Address:       " << ep->address << endl;
          out << "            Transfer Type: " << ep->transfer_type << endl;
          out << "            Direction:     " << ((ep->address & LIBUSB_ENDPOINT_IN) == 0 ? "OUT" : "IN") << endl;
        }
      }
    }
  }

  //////////////////////////////////////////////////////////// 4
  
  success = true;
  
  try
  {
    m_test_subject->open();
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 5
  
  success = true;
  
  try
  {
    m_test_subject->set_interface(0);
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 6
  
  success = true;
  
  libusb_usb_device::data_t data[65535] = "";
  
  build_read_command(data, 0, 0);
  
  try
  {
    if (m_test_subject->write(data, 64, 2000) != 64)
    {
      success = false;
    }
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 7
  
  success = true;
  int r;
  
  try
  {
    r  = m_test_subject->read(data, 64, 2000);
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  out << "    Data received (64 bytes, hex):" << hex << uppercase << setfill('0');
  for (int i = 0; i < r; ++i)
  {
    if (i % 8 == 0)
    {
      out << "\n     ";
    }
    out << ' ' << setw(2) << (int) data[i];
  }
  out << setfill(' ') << nouppercase << dec << endl;
  
  //////////////////////////////////////////////////////////// 8
  
  success = true;
  unsigned char dataByte;
  unsigned int returnAddress;
  
  try
  {
    get_read_reply(data, &returnAddress, &dataByte);
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  out << "    byte: 0x" << hex << uppercase << (int) dataByte << nouppercase << dec << endl;
  out << "    addr: 0x" << hex << uppercase << returnAddress << uppercase << dec << endl;
  
  //////////////////////////////////////////////////////////// 9
  
  success = true;
  
  try
  {
    m_test_subject->close();
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// 10
  
  success = true;
  
  try
  {
    delete m_test_subject;
  }
  catch (std::exception& e)
  {
    out << e.what() << endl;
    success = false;
  }
  
  out << "  Test " << ++test_num << " " << (success ? "PASSED" : "FAILED") << endl;
  m_test_count[success ? 0 : 1]++;
  
  //////////////////////////////////////////////////////////// END
  
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
  out << "Passed " << m_test_count[0] << " of " << (m_test_count[0] + m_test_count[1]) << endl << endl;
}

