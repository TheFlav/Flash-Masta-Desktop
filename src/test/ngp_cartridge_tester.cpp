//
//  ngp_cartridge_tester.cpp
//  FlashMasta
//
//  Created by Dan on 7/30/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#include "ngp_cartridge_tester.h"

#include "test.h"
#include "usb/libusb_usb_device.h"
#include "linkmasta_device/ngp_linkmasta_device.h"
#include "cartridge/ngp_cartridge.h"
#include "libusb-1.0/libusb.h"

#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>

using namespace std;
using namespace usb;

ngp_cartridge_tester::ngp_cartridge_tester(std::istream& in, std::ostream& out, std::ostream& err)
  : tester("ngp_cartridge_tester"), in(in), out(out), err(err), m_fin(new ifstream)
{
  typedef function<bool (std::ostream&, std::istream&, std::ostream&)> action;
  
  // REUSABLE ACTIONS
  action a_construct = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Constructing instance..."; out.flush();
    m_cartridge = new ngp_cartridge(m_linkmasta);
    out << "done." << endl;
    return true;
  };
  
  action a_destruct = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Destructing instance..."; out.flush();
    delete m_cartridge;
    m_cartridge = nullptr;
    out << "done." << endl;
    return true;
  };
  
  action a_initialize = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Initializing device..."; out.flush();
    m_cartridge->init();
    out << "done." << endl;
    return true;
  };
  
  action a_describe = [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    out << "  Getting and cartridge description...";
    auto desc = m_cartridge->descriptor();
    if (desc == nullptr) return false;
    out << "done." << endl;
    
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
        out << "        Block " << block->block_num << ",  ";
        out << "Address: 0x" << hex << uppercase << setfill('0') << setw(6) << block->base_address << setfill(' ') << nouppercase << dec << ",  ";
        out << "Size: " << block->num_bytes << " B (" << (block->num_bytes / 0x400) << " KiB)" << ",  ";
        out << "Protected: " << (block->is_protected ? "YES" : "NO") << endl;
      }
    }
    
    return true;
  };
  
  auto a_backup_rom = [=](std::ostream& out, std::istream& in, std::ostream& err, int slot = ngp_cartridge::SLOT_ALL)->bool
  {
    for (int i = 0; i < m_cartridge->num_slots(); ++i)
    {
      if (slot != ngp_cartridge::SLOT_ALL && i != slot) continue;
      
      stringstream s;
      s << "/Users/Dan/Documents/ROMs/backup" << i << ".ngp";
      out << "  Opening file \"" << s.str() << "\" for output..."; out.flush();
      std::ofstream fout(s.str());
      out << "done." << endl;
      
      out << "  Backing up slot " << i << " game data..." << endl;
      m_cartridge->backup_cartridge_game_data(fout, i);
      out << "done." << endl;
      
      fout.close();
      out << "  Closed file \"" << s.str() << "\"." << endl;
    }
    
    return true;
  };
  
  auto a_compare_rom = [=](std::ostream& out, std::istream& in, std::ostream& err, int slot = ngp_cartridge::SLOT_ALL)->bool
  {
    for (int i = 0; i < m_cartridge->num_slots(); ++i)
    {
      if (slot != ngp_cartridge::SLOT_ALL && i != slot) continue;
      
      stringstream s;
      s << "/Users/Dan/Documents/ROMs/backup" << i << ".ngp";
      out << "  Opening file \"" << s.str() << "\" for input..."; out.flush();
      std::ifstream fin(s.str());
      out << "done." << endl;
      
      out << "  Verifying slot " << i << " game data..." << endl;
      bool success = m_cartridge->compare_cartridge_game_data(fin, i);
      out << "done." << endl;
      
      fin.close();
      out << "  Closed file \"" << s.str() << "\"." << endl;
      
      if (!success) return false;
    }
    
    return true;
  };
  
  auto a_restore_rom = [=](std::ostream& out, std::istream& in, std::ostream& err, int slot = ngp_cartridge::SLOT_ALL)->bool
  {
    for (int i = 0; i < m_cartridge->num_slots(); ++i)
    {
      if (slot != ngp_cartridge::SLOT_ALL && i != slot) continue;
      
      stringstream s;
      s << "/Users/Dan/Documents/ROMs/backup" << i << ".ngp";
      out << "  Opening file \"" << s.str() << "\" for input..."; out.flush();
      std::ifstream fin(s.str());
      out << "done." << endl;
      
      out << "  Restoring slot " << i << " game data..." << endl;
      m_cartridge->restore_cartridge_game_data(fin, i);
      out << "done." << endl;
      
      fin.close();
      out << "  Closed file \"" << s.str() << "\"." << endl;
    }
    
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
  
  // DESCRIBING CARTRIDGE HARDWARE
  add_test(new test("get cartridge description", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_describe(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  // BACKING UP CARTRIDGE
  add_test(new test("backup rom to disk", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_backup_rom(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  // VERIFYING CARTRIDGE
  add_test(new test("verify rom", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_compare_rom(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  // RESTORING CARTRIDGE
  add_test(new test("restore rom from disk", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    return (a_construct(out, in, err)
            && a_initialize(out, in, err)
            && a_restore_rom(out, in, err)
            && a_compare_rom(out, in, err)
            && a_destruct(out, in, err));
  }));
  
  
  // FLASH METAL SLUG 2
  add_test(new test("flash metal slug 2 from disk", false, [=](std::ostream& out, std::istream& in, std::ostream& err)->bool
  {
    if (!(a_construct(out, in, err) && a_initialize(out, in, err))) return false;
    
    
    stringstream s;
    s << "/Users/Dan/Documents/ROMs/metalslug2nd.ngp";
    out << "  Opening file \"" << s.str() << "\" for input..."; out.flush();
    std::ifstream fin(s.str());
    out << "done." << endl;
    
    out << "  Restoring cartridge game data..." << endl;
    m_cartridge->restore_cartridge_game_data(fin, ngp_cartridge::SLOT_ALL);
    out << "done." << endl;
    
    fin.seekg(0, fin.beg);
    
    out << "  Verifying cartridge game data..." << endl;
    bool success = m_cartridge->compare_cartridge_game_data(fin, ngp_cartridge::SLOT_ALL);
    out << "done." << endl;
    
    fin.close();
    out << "  Closed file \"" << s.str() << "\"." << endl;
    
    if (!success) return false;
    if (!a_destruct(out, in, err)) return false;
    
    return true;
  }));
}

ngp_cartridge_tester::~ngp_cartridge_tester()
{
  delete m_fin;
}

bool ngp_cartridge_tester::prepare()
{
  out << "Beginning " << name() << " test preparations" << endl;
  
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
  m_handle = libusb_open_device_with_vid_pid(m_libusb, 0x20A0, 0x4256);
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
  catch (std::exception& ex)
  {
    err << endl;
    err << ex.what();
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
  catch (std::exception& ex)
  {
    err << endl;
    err << ex.what();
    err << "ERROR: An error occured when attempting to initialize linkmasta device" << endl;
    return false;
  }
  out << "done." << endl;
  
  // Initialize input file
  m_fin->open("/Users/Dan/Documents/ROMs/pacman_cut.ngp", ios::binary);
  
  return true;
}

void ngp_cartridge_tester::pretests()
{
  out << "Beginning " << name() << " tests" << endl;
}

void ngp_cartridge_tester::posttests()
{
  out << "Concluded " << name() << " tests" << endl;
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


