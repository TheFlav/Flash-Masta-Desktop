/*! \file
 *  \brief File containing the declaration of the \ref ngp_chip class.
 *  
 *  File containing the header information and declaration of the
 *  \ref ngp_chip class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-29
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __NGP_CHIP_H__
#define __NGP_CHIP_H__

class linkmasta_device;
class task_controller;

/*! \class ngp_chip
 *  \brief Class for controlling and interacting with a flash storage chip on a
 *         Neo Geo Pocket cartridge.
 *  
 *  Class for controlling and interacting with a flash storage chip on a Neo Geo
 *  Pocket cartridge. Contains logic for manipulating the device's current mode,
 *  erasing sectors on the device, programming the device, and getting device
 *  information.
 *  
 *  This class is intended to be used as a communication utility
 *  and stores very little information about the current state of the device
 *  that it represents.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class ngp_chip
{
public:
  /*! \brief Type used for data content. */
  typedef unsigned char    data_t;
  
  /*! \brief Type used for sending commands and reading words from the chip. */
  typedef unsigned char    word_t;
  
  /*!
   * \brief Type used for specifying the index of the device on the cartridge.
   */
  typedef unsigned int     chip_index_t;
  
  /*! \brief Type used for the device's manufacturer id. */
  typedef unsigned int     manufact_id_t;
  
  /*! \brief Type used for the device's device id. */
  typedef unsigned int     device_id_t;
  
  /*!
   *  \brief Type used for indicating whether a sector on the device is
   *         write protected.
   */
  typedef bool             protect_t;
  
  /*! \brief Type used for specifying an address on the device. */
  typedef unsigned int     address_t;
  
  /*! \enum chip_mode
   *  \brief Enumeration representing the assumed state of the device.
   *  
   *  Enumeration representing the current assumed state of the device.
   *  
   *  \see current_mode()
   */
  enum chip_mode
  {
    /*! \brief Device is in the default read mode. */
    READ,
    
    /*! \brief Device is in autoselect mode, ready for metadata queries. */
    AUTOSELECT,
    
    /*! \brief Device is in bypass mode, ready for swift programming. */
    BYPASS,
    
    /*! \brief Device is busy erasing. */
    ERASE
  };
  
  /*! \brief The constructor for this class.
   *  
   *  The main constructor for the class. Initializes members with supplied
   *  parameters.
   *  
   *  \param linkmasta_device Reference to the linkmasta device this chip is to
   *         use for communication with the hardware. Must be a pointer to a
   *         valid object in memory.
   *  \param [in] chip_num This device's index on the cartridge.
   *  
   *  \see linkmasta_device
   */
                          ngp_chip(linkmasta_device* linkmasta_device, chip_index_t chip_num);
  
  /*! \brief The destructor for the class.
   *  
   *  Frees dynamically allocated memory and closes any open connections.
   *  
   *  To prevent the device being in an inconsistent state, be sure to call
   *  \ref reset() before allowing this object to be destroyed.
   *  
   *  \see reset()
   */
                          ~ngp_chip();
  
  
  
  /*! \brief Reads a single word from the chip.
   *  
   *  Reads a single word from the chip from the provided address. The exact
   *  result of this method can vary depending on the current state of the chip.
   *  For example, if the chip is in \ref chip_mode::READ mode, then the device
   *  should give the data stored at the given address. If the device is in
   *  \ref chip_mode::AUTOSELECT, then this method can be used to request
   *  specific metadata about the chip. See the device's specification sheet to
   *  learn what commands are valid.
   *  
   *  \param [in] address The address from which to request a word.
   *  
   *  \returns The word returned from the device.
   *  
   *  \see address_t
   *  \see chip_mode
   *  \see current_mode()
   */
  word_t                  read(address_t address);
  
  /*! \brief Writes a single word to the chip.
   *  
   *  Writes a single word from the chip. This does not program the data in the
   *  chip's storage, but instead can be used to send commands directly to the
   *  device. See the device's specification sheet to learn what commands are
   *  valid.
   *  
   *  \param [in] address The address to which to write.
   *  \param [in] data The word of data to send to the chip.
   *  
   *  \see program_byte(address_t address, data_t data);
   */
  void                    write(address_t address, word_t data);
  
  
  
  /*! \brief Commands the device to enter the default \ref chip_mode::READ mode.
   *  
   *  Sends the reset command sequence to the hardware device. Whether or not
   *  the operation is successful is not guaranteed.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::READ mode.
   *  
   *  \see chip_mode::READ
   *  \see current_mode()
   */
  void                    reset();
  
  /*! \brief Commands the device to fetch the manufacturer id.
   *  
   *  Sends the command sequence necessary to enter \ref chip_mode::AUTOSELECT
   *  mode before requesting the manufacturer's id. The success of this
   *  operation is not guaranteed.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::AUTOSELECT mode.
   *  
   *  \returns If the operation executes as expected, will return the
   *           manufacturer's id. Otherwise, the result is undefined.
   *  
   *  \see chip_mode::AUTOSELECT
   *  \see current_mode()
   */
  manufact_id_t           get_manufacturer_id();
  
  /*! \brief Commands the device to fetch the device id.
   *  
   *  Sends the command sequence necessary to enter \ref chip_mode::AUTOSELECT
   *  mode before requesting the device id. The success of this operation is not
   *  guaranteed.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::AUTOSELECT mode.
   *  
   *  \returns If the operation executes as expected, will return the device id.
   *           Otherwise, the result is undefined.
   *  
   *  \see chip_mode::AUTOSELECT
   *  \see current_mode()
   */
  device_id_t             get_device_id();
  
  /*! \brief Queries the device on the protection status of a specific sector.
   *  
   *  Sends the command sequence necessary to enter \ref chip_mode::AUTOSELECT
   *  mode before requesting the protection status of a sector.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::AUTOSELECT mode.
   *  
   *  \param [in] sector_address The base address of the block (sector) to
   *         test for protection.
   *  
   *  \returns If successful, returns **true** if the sector is protected and
   *           **false** if it is unprotected.
   *  
   *  \see chip_mode::AUTOSELECT
   *  \see current_mode()
   */
  protect_t               get_block_protection(address_t sector_address);
  
  /*! \brief Attempts to program a word at a specific address on the chip.
   *  
   *  Attepts to program a word at a specific address on the chip. See note
   *  below on how to use this functionality when working with flash-based
   *  storage.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::READ mode.
   *  
   *  \note
   *  This function must be used with knowledge about how flash storage chips
   *  work. Writing to flash storage can only reset bits. That means that only
   *  **0**'s can be written to flash storage. In order to write **1**'s to
   *  flash storage, the entire sector containing the destination address must
   *  be erased.
   *  
   *  \par
   *  Before calling this function, ensure that the data at the destination
   *  address has been erased.
   *  
   *  \param [in] address The address on the chip to program.
   *  \param [in] data The data to write to flash storage.
   */
  void                    program_byte(address_t address, data_t data);
  
  /*! \brief Attempts to cause the device to enter bypass mode.
   *  
   *  Sends the command sequence to the device that will cause it to enter
   *  \ref chip_mode::BYPASS mode. When in \ref chip_mode::BYPASS mode, the
   *  device can program data much faster. See note below on how to use this
   *  functionality.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::BYPASS mode.
   *  
   *  \note
   *  This functionality is only available on certain devices. If this object
   *  determines that the current device does not support bypass mode, then this
   *  funtion will do nothing.
   */
  void                    unlock_bypass();
  
  /*! \brief Sends the command sequence to erase all data in the chip's flash
   *         storage.
   *  
   *  Sends the command sequence to erase all data in the chip's flash storage,
   *  setting all bits to **1**. See note below on how to use this
   *  functionality.
   *  
   *  \note
   *  Once this method has been called, it is the
   *  responsibility of the caller to poll the chip until the erase operation
   *  is complete. To test if the chip is erasing, the caller must call the
   *  \ref test_erasing() function. *Calls to \ref is_erasing() or testing the
   *  device's current mode with \ref current_mode() will not work!*
   *  
   *  Causes the device to enter \ref chip_mode::ERASE.
   */
  void                    erase_chip();
  
  /*! \brief Sends the command sequence to erase a single sector in the chip's
   *         flash storage.
   *  
   *  Sends the command sequence to erase an individual sector in the chip's
   *  flash storage, setting all bits within to **1**. See note below on how to
   *  use this functionality.
   *  
   *  \note
   *  Once this method has been called, it is the
   *  responsibility of the caller to poll the chip until the erase operation
   *  is complete. To test if the chip is erasing, the caller must call the
   *  \ref test_erasing() function. *Calls to \ref is_erasing() or testing the
   *  device's current mode with \ref current_mode() will not work!*
   *  
   *  Causes the device to enter \ref chip_mode::ERASE.
   *  
   *  \param [in] block_address The base address of the sector to erase. If this
   *         address is not the exact base address of a valid sector, then the
   *         erase operation will likely never initialize or will never
   *         complete.
   */
  void                    erase_block(address_t block_address);
  
  
  
  /*! \brief Gets the currently assumed mode that the chip is in.
   *  
   *  Gets the currently assumed mode that the chip is in. This value is not
   *  guaranteed to be true because the device may have been in an unknown state
   *  when this class was created.
   *  
   *  \returns The current \ref chip_mode that the device is assumed to be in.
   *  
   *  \see chip_mode
   */
  chip_mode               current_mode() const;
  
  /*! \brief Gets whether or not the current chip supports
   *         \ref chip_mode::BYPASS mode.
   *  
   *  Gets whether or not the current chip supports \ref chip_mode::BYPASS mode.
   *  Not all chips can support this faster means of programming memory, and
   *  thus it is important to not assume that the given chip supports this
   *  functionality.
   *  
   *  Rarely will an outside function need to know if the chip supports bypass
   *  mode. Once a test has been made, this class will store the result and will
   *  use it in future program operations.
   *  
   *  This function does not query the chip to determine if it supports bypass
   *  mode. Instead, it checks an internally cached variable. To perform a test
   *  on the device to see if it supports bypass mode, use
   *  \ref test_bypass_support().
   *  
   *  \returns **true** if the chip supports bypass mode, and **false** if the
   *           chip does not.
   *  
   *  \see chip_mode
   *  \see test_bypass_support()
   */
  bool                    supports_bypass() const;
  
  /*! \brief Tests the chip for whether or not it supports
   *         \ref chip_mode::BYPASS mode.
   *  
   *  Tests the chip for whether or not it supports \ref chip_mode::BYPASS mode.
   *  Uses various command sequences to query metadata on the chip to make this
   *  determination. The result is then cached internally and can be retrieved
   *  without querying the chip using \ref supports_bypass().
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::AUTOSELECT mode.
   *  
   *  \returns **true** if the chip supports \ref chip_mode::BYPASS mode,
   *           **false** if it does not.
   *  
   *  \see chip_mode
   *  \see supports_bypass()
   */
  bool                    test_bypass_support();
  
  /*! \brief Gets whether or not the chip was last determined to be in
   *         \ref chip_mode::ERASE mode.
   *  
   *  Gets whether or not the chip was last determined to be in
   *  \ref chip_mode::ERASE mode. This function does not query the chip, and
   *  thus may not be a true representation of whether or not the chip is
   *  actually erasing anything at the moment. To explicitly query the chip to
   *  determine if it is currently erasing, use \ref test_erasing(). The result
   *  of this function will not change until a call to \ref test_erasing()
   *  determines that the chip is no longer erasing or if a call to
   *  \ref erase_chip() or \ref erase_block(address_t block_address) puts the
   *  chip into \ref chip_mode::ERASE mode.
   *  
   *  \returns **true** if the chip was last determined to be in
   *           \ref chip_mode::ERASE mode, **false** if not.
   *  
   *  \see chip_mode
   *  \see test_erasing()
   *  \see erase_chip()
   *  \see erase_block(address_t block_address)
   */
  bool                    is_erasing() const;
  
  /*! \brief Tests the chip to determine whether it is in \ref chip_mode::ERASE
   *         mode or not.
   *  
   *  Tests the chip to determine whether it is in \ref chip_mode::ERASE mode or
   *  not. It is the responsibility of whatever controls this class to call this
   *  function repeatedly after making a call to \ref erase_chip() or
   *  \ref erase_blocK(address_t block_address) until this function returns
   *  **false**.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  If this function detects that the chip has just finished erasing, it will
   *  put the chip into \ref chip_mode::READ mode.
   *  
   *  \code
   *  // Begin erasing chip
   *  erase_chip();
   *  
   *  // Wait here until erasing ends
   *  while (test_erasing());
   *  \endcode
   *  
   *  \returns **true** if the chip is still erasing, **false** if not.
   *  
   *  \see chip_mode
   *  \see erase_chip()
   *  \see erase_block(address_t block_address)
   */
  bool                    test_erasing();
  
  /*! \brief Reads a series of sequential bytes of data from the chip.
   *  
   *  Reads a series of sequential bytes of data from the chip. Does not modify
   *  the chip's contents. If the chip is not already in \ref chip_mode::READ
   *  mode, will cause the chip to enter \ref chip_mode::READ mode.
   *  
   *  Will read the specified number of bytes from the chip starting at the
   *  given address and will write them to the given data array.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  Causes the device to enter \ref chip_mode::READ mode.
   *  
   *  \param [in] address The address on the chip to begin reading bytes.
   *  \param [out] address The buffer to write the results of the operation to.
   *         This value must be a valid pointer to an already allocated array
   *         of \ref data_t objects. These objects do not need to be initialized
   *         before hand.
   *  \param [in] num_bytes The number of bytes to read from the chip.
   *  \param [in,out] controller The \ref task_controller to report progress to.
   *         This value must be a valid pointer a \ref task_controller object or
   *         **nullptr**.
   *  
   *  \returns The number of bytes successfully read from the chip.
   *  
   *  \see address_t
   *  \see data_t
   *  \see task_controller
   *  \see chip_mode
   */
  unsigned int            read_bytes(address_t address, data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
  /*! \brief Programs a series of bytes of data sequentially to the chip.
   *  
   *  Programs the flash data on the chip given a sequence of bytes, starting at
   *  the given address. This modifies the chips contents. Will program the
   *  specified number of bytes to the chip starting at the given address and
   *  will return the number of bytes successfully written.
   *  
   *  The exact result of this function is dependent on the behavior of flash
   *  memory. Flash chips can only program 0's, meaning that 1's can only be
   *  flipped to 0's, and 0's cannot be changed. The only way to "reset" these
   *  0's to 1's is to erase the entire sector in which the desired address
   *  resides. Before calling this function, be sure to call \ref erase_chip()
   *  or \ref erase_block(address_t block_address) first to guarantee expected
   *  behavior.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  Causes the device to enter \ref chip_mode::BYPASS mode if supported by the
   *  chip, and \ref chip_mode::READ mode otherwise.
   *  
   *  \param [in] address The address on the chipo to begin programming bytes.
   *  \param [in] data A buffer containing the data to program to the chip.
   *  \param [in] num_bytes The size in bytes of the \ref data buffer.
   *  \param [in,out] controller The \ref task_controller to report progress to.
   *         This value must be a valid pointer a \ref task_controller object or
   *         **nullptr**.
   *  
   *  \returns The number of bytes successfully programmed to the chip.
   *  
   *  \see address_t
   *  \see data_t
   *  \see task_controller
   *  \see chip_mode
   */
  unsigned int            program_bytes(address_t address, const data_t* data, unsigned int num_bytes, task_controller* controller = nullptr);
  
  
  
private:
  /*! \brief Send the command sequence to enter \ref chip_mode::AUTOSELECT mode
   *         to the chip.
   *  
   *  Sends the \ref chip_mode::AUTOSELECT mode command sequence to the chip.
   *  This will send reset the device if necessary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  Causes the device to enter \ref chip_mode::AUTOSELECT mode.
   *  
   *  \see chip_mode
   */
  void                    enter_autoselect();
  
  
  
  /*! \brief The currently predicted mode of the chip.
   *  
   *  The currently predicted mode the hardware device is in. This value is
   *  to track the state of the chip and must be actively maintained when
   *  passing commands that could alter the state of the device. This value is
   *  initialized to \ref chip_mode::READ.
   *  
   *  \see chip_mode
   */
  chip_mode               m_mode;
  
  /*! \brief The address of the last block erased.
   *  
   *  The last address passed to the \ref erase_block(address_t block_address)
   *  function. This value is used for testing whether the chip has finished
   *  erasing or not.
   *  
   *  \ref address_t
   *  \ref erase_block(address_t block_address)
   */
  address_t               m_last_erased_addr;
  
  /*! \brief Boolean value indicating whether or not the device supports bypass
   *         mode.
   *  
   *  Boolean value indicating whether or not the device supports bypass mode.
   *  This value is set in the \ref init() and automatically detected based on
   *  the device's manufacturer and product IDs.
   *  
   *  \ref init()
   */
  bool                    m_supports_bypass;
  
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
  linkmasta_device* const m_linkmasta;
  
  /*! \brief The index number of the chip on the cartridge.
   *  
   *  The index number of this device on the cartridge. Used for specifying the
   *  hardware device to communicated with through the assigned \ref m_linkmasta
   *  device.
   *  
   *  \see chip_index_t
   */
  chip_index_t const      m_chip_num;
};

#endif /* defined(__NGP_CHIP_H__) */
