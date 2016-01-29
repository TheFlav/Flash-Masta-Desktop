/*! \file
 *  \brief File containing the declaration of the \ref ws_sram_chip class.
 *
 *  File containing the header information and declaration of the
 *  \ref ws_sram_chip class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-25
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __WS_SRAM_CHIP_H__
#define __WS_SRAM_CHIP_H__

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
  
  
  
  /*! \brief Reads a single word from the chip.
   *  
   *  Reads a single word from the chip at the provided address. Due to the
   *  nature of SRAM, the result of this operation, if successful, should always
   *  return the exact data stored at the supplied address.
   *  
   *  \param[in] address The address of the data to fetch.
   *  
   *  \returns The word of data stored at the supplied address.
   */
  word_t                  read(address_t address);
  
  /*! \brief Writes a single word of data to the chip.
   *  
   *  Writes a single word of data to the chip. Due to the nature of SRAM, the
   *  result of this operation, if successfull, will always result in stored
   *  data being overwritten with the supplied data.
   *  
   *  \param[in] address The address at which to store \ref data.
   *  \param[in] data The data to store at \ref address.
   */
  void                    write(address_t address, word_t data);
  
  /*! \param Reads multiple sequential bytes from storage.
   *  
   *  Reads multiple sequential bytes from storage and outputs the results to
   *  a supplied location.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  \param[in] address The starting address to begin reading data.
   *  \param[out] data Array to output data from the device.
   *  \param[in] num_bytes The number of bytes of data to read from the device.
   *  \param[in|out] controller The \ref task_controller to report to while
   *                 reading data.
   *  
   *  \returns The number of bytes written to \ref data output location.
   *  
   *  \see task_controller
   */
  unsigned int            read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
  /*! \brief Writes a sequence of bytes to the device.
   *  
   *  Writes a sequence of bytes to the device in order, starting at the
   *  specified address.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  \param[in] address The starting address to begin writing data.
   *  \param[in] data Array of data to write to the device.
   *  \param[in] num_bytes The number of bytes of data to write to the device.
   *  \param[in|out] controller The \ref task_controller to report to while
   *                 writing data.
   *  
   *  \returns The number of bytes of data successfully written to the device.
   *  
   *  \see task_controller
   */
  unsigned int            program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
  
  
private:
  
  /*! \brief A pointer to the \ref linkmasta_device used for communicating with
   *         the chip.
   *  
   *  A constant pointer to the \ref linkmasta_device used for communicating
   *  with the chip. The pointer to this device is const but the device itself
   *  is not protected from modification. When this object is deallocated, this
   *  pointer's target will not be deleted.
   *  
   *  \see linkmasta_device
   */
  linkmasta_device* m_linkmasta;
  
  /*! \brief The index number of the chip on the cartridge.
   *  
   *  The index number of this device on the cartridge. Used for specifying the
   *  hardware device to communicated with through the assigned \ref m_linkmasta
   *  device.
   *  
   *  \see chip_index_t
   */
  chip_index_t      m_chip_num;
};

#endif /* defined(__WS_SRAM_CHIP_H__) */
