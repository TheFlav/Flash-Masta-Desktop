/*! \file
 *  \brief File containing the declaration of the \ref ws_sram_chip class.
 *
 *  File contaiing the header information and declaration of the
 *  \ref ws_sram_chip class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-25
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __WS_SRAM_CHIP__
#define __WS_SRAM_CHIP__

class linkmasta_device;
class task_controller;

/*! \class ws_sram_chip
 *  \brief Class for controlling and interacting with an SRAM storage chip on a
 *         WonderSwan cartridge.
 *  
 *  Class for controlling and interacting with an SRAM storage chip on a
 *  WonderSwan cartridge. Provides an interface for reading and writing data to
 *  the device without the caller needing to know how to perform specific
 *  actions on the device.
 *  
 *  Because SRAM is far simpler than flash memory, this class only provides
 *  methods for reading and writing individual words from and to the device, as
 *  well as lengthy sequences of data.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class ws_sram_chip
{
public:
  /*! \brief Type used to represent data content. */
  typedef unsigned char   data_t;
  
  /*! \brief Type used to represent words on the chip. */
  typedef unsigned char   word_t;
  
  /*! \brief Type used to represent the chip's index on the cartridge. */
  typedef unsigned int    chip_index_t;
  
  /*! \brief Type used to represent an address into the device's storage. */
  typedef unsigned int    address_t;
  
  /*! \brief The constructor for this class.
   *  
   *  The main constructor for this class. Initializes members with supplied
   *  parameters.
   *  
   *  \param[in,out] linkmasta Pointer to the \ref linkmasta_device this chip is
   *                 to use for communication with the hardware. Must be a
   *                 pointer to a valid object in memory.
   *  
   *  \see linkmasta_device
   */
                          ws_sram_chip(linkmasta_device* linkmasta);
  
  /*! \brief The destructor for the class.
   *  
   *  Frees dynamically allocated memory and closes any open connections.
   */
                          ~ws_sram_chip();
  
  word_t                  read(address_t address);
  
  void                    write(address_t address, word_t data);
  
  unsigned int            read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  unsigned int            program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
private:
  linkmasta_device* m_linkmasta;
  chip_index_t      m_chip_num;
};

#endif /* defined(__WS_SRAM_CHIP__) */
