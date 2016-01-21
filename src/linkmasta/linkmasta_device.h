/*! \file
 *  \brief File containing the declarartion of the \ref linkmasta_device
 *         abstract class.
 *  
 *  File containing the header information and declaration of the
 *  \ref linkmasta_device abstract class. This file includes the minimal number
 *  of files necessary to use any instance of the \ref linkmasta_device class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-24
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __LINKMASTSA_DEVICE_H__
#define __LINKMASTSA_DEVICE_H__

#include "common/types.h"
#include <string>

class cartridge;
class task_controller;



/*! \enum linkmasta_system
 *  \brief Enumeration representing a system to which the linkmasta belongs.
 *  
 *  Enumeration indicating which game system a linkmasta is designed to work
 *  with. Can be used to determine how to cast a pointer.
 */
enum linkmasta_system
{
  /*! \brief Unknown linkmasta system. */
  LINKMASTA_UNKNOWN,
  
  /*! \brief Neo Geo Pocket or Neo Geo Pocket Color. */
  LINKMASTA_NEO_GEO_POCKET,
  
  /*! \brief Wonderswan or Wonderswan Color. */
  LINKMASTA_WONDERSWAN
};



/*! \class linkmasta_device
 *  \brief Provides functionality for communicating with a linkmasta device and
 *         interfaces for determining what features the implementation supports.
 *  
 *  Abstract class representing a LinkMasta device. Allows for easy interaction
 *  and communication with the device and abstraction of implementation details.
 *  Provides convenient methods for message passing and for interacting with
 *  any connected cartridge.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class linkmasta_device
{
public:
  
  /*! \brief Type used for data transfer to and from the device. */
  typedef unsigned char    data_t;
  
  /*! \brief Type used for communicating with the device using single words. */
  typedef unsigned int     word_t;
  
  /*! \brief Type used for setting device timeouts. */
  typedef unsigned int     timeout_t;
  
  /*! \brief Type used for communicating version information. */
  typedef std::string      version_t;
  
  /*! \brief Type used for indicating chip indexes in some operations. */
  typedef unsigned int     chip_index;
  
  
  
  /*!
   *  \brief Class destructor.
   *  
   *  Destructor for the class. Exists to surpress warnings.
   */
  virtual                  ~linkmasta_device() {};
  
  /*!
   *  \brief Initializes the device using default settings.
   *  
   *  Initializes the linkmasta with default settings. This is a blocking
   *  function that can take several seconds to complete.
   *  
   *  If this function has been called previously on the current instance of the
   *  class, then nothing will happen.
   */
  virtual void             init() = 0;
  
  
  
  /*!
   *  \brief Checks if the connection to the underlying USB device is open.
   *  
   *  Checks if the linkmasta currently has an open connection to the underlying
   *  USB device.
   *  
   *  \return true if the connection is open, false if not.
   */
  virtual bool             is_open() const = 0;
  
  /*!
   *  \brief Gets the current timeout setting used for all communication with
   *         the underlying USB device.
   *  
   *  Gets the current timeout setting used for all communication with the
   *  underlying USB device. This value is used when sending and recieving data
   *  over USB, but does not apply to operations in the class. In other words,
   *  if a single operation requires the sending and recieving of multiple
   *  messages over USB, then this timeout will be multiplied by the number of
   *  messages sent or recieved. This value will fluctuate depending on the
   *  underlying implementation.
   *  
   *  \return Number of milliseconds that all USB operations will be capped at.
   *  
   *  \see set_timeout(timeout_t timeout)
   */
  virtual timeout_t        timeout() const = 0;
  
  /*!
   *  \brief Gets the version of the firmware running on the device.
   *  
   *  Gets the firmware version string of the LinkMasta.
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init() or opened via a call to \ref open(), an exception will
   *  be thrown.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  \return String containing the firmware version of the firmware running on
   *          the LinkMasta device.
   */
  virtual version_t        firmware_version() = 0;
  
  /*!
   *  \brief Checks if the LinkMasta device is a standalone device or if it is
   *  integrated with a cartridge.
   *  
   *  Checks if a LinkMasta device is a standalone device or if it is integrated
   *  into a game cartridge. This can be used to determine whether or not a call
   *  \ref test_for_cartridge() is necessary.
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init(), an exception will be thrown.
   *  
   *  \return true if the device is built-in to a cartridge and a call to
   *          \ref test_for_cartridge() is unnecessary, false if it is a
   *          standalone device and the call to \ref test_for_cartridge() is
   *          necessary.
   */
  virtual bool             is_integrated_with_cartridge() const = 0;
  
  
  
  /*!
   *  \brief Sets the maximum number of milliseconds that USB operations should
   *         be allowed to take before being terminated.
   *  
   *  Sets the maximum number of milliseconds that USB operations should be
   *  allowed to take before being terminated. If a negative number is given,
   *  then all timeouts will be removed, potentially allowing USB operations to
   *  hang indefinitely.
   *  
   *  \param [in] timeout The number of milliseconds to allow USB operations to
   *                      take.
   *  
   *  \see timeout()
   */
  virtual void             set_timeout(timeout_t timeout) = 0;
  
  /*!
   *  \brief Opens a connection to the underlying USB device.
   *  
   *  Opens a connection to the underlying USB device. This method must be
   *  called before attempting to perform any operations on or communicating
   *  with the USB device.
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init(), an exception will be thrown.
   *  
   *  If this method is called when the device's connection has already been
   *  opened, then nothing will happen.
   */
  virtual void             open() = 0;
  
  /*!
   *  \brief Closes a connection to the underlying USB device.
   *  
   *  Closes any open connection with the underlying USB device. This method
   *  should be called after completing any series of operations on or
   *  communicating with the USB device.
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init(), an exception will be thrown.
   *  
   *  If this method is called when the device's connection has already been
   *  closed, then nothing will happen.
   */
  virtual void             close() = 0;
  
  /*!
   *  \brief Reads an individual word from the underlying USB device which may
   *         be data or control information.
   *  
   *  Reads a word from the indicated chip on the cartridge connected to this
   *  device, if any. This word may be stored data or it may be control data.
   *  This method can be used to communicate and send commands to or receive
   *  feedback from a hardware chip and should be used with caution. It is
   *  highly recommend that other operations are used first if they are supprted
   *  by this device before attempting to manually implement them using this
   *  method.
   *  
   *  Accessing this method directly can also result in other classes entering
   *  an invalid state or becoming out-of-sync with the underlying hardware
   *  device.
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init() or opened via a call to \ref open(), an exception will
   *  be thrown.
   *  
   *  If an operation fails or an error occures, this method will throw an
   *  exception.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \param [in] chip    The index of the hardware chip on the connected
   *                      cartridge to read the word from.
   *  \param [in] address The memory address on the hardware chip on the
   *                      connected cartridge of the word to read.
   *  
   *  \return The result of the read operation.
   */
  virtual word_t           read_word(chip_index chip, address_t address) = 0;
  
  /*!
   *  \brief Writes an individual word to the underlying USB device. This does
   *         not attempt to program the data on the chip.
   *  
   *  Writes a word to the indicated chip on the cartridge connected to this
   *  device, if any. This method can be used to send commands to a hardware
   *  device and should be used with caution and only if you know exactly what
   *  you are doing. This method should only be used for directly communicating
   *  with the hardware chip and thus can result in unrecoverable data loss if
   *  used incorrectly.
   *  
   *  Accessing this method directly can also result in other classes entering
   *  an invalid state or becoming out-of-sync with the underlying hardware
   *  device.
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init() or opened via a call to \ref open(), an exception will
   *  be thrown.
   *  
   *  If an operation fails or an error occures, this method will throw an
   *  exception.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \param [in] chip    The index of the hardware chip on the connected cartridge
   *                      to read the word from.
   *  \param [in] address The memory address on the hardware chip on the
   *                      connected cartridge of the word to read.
   *  \param [in] data    The word of data to write to the hardware device.
   */
  virtual void             write_word(chip_index chip, address_t address, word_t data) = 0;
  
  /*!
   *  \brief Tests for the existance of a connected cartridge.
   *  
   *  Tests for the existance of a connected cartridge. For LinkMastas that are
   *  integrated in with a cartridge, this method may still attempt to
   *  communicate with the cartridge, and so calls to this method should be
   *  avoided if possible by first calling \ref is_integrated_with_cartridge().
   *  
   *  If this method is called before the instance has been initialized via a
   *  call to \ref init() or opened via a call to \ref open(), an exception will
   *  be thrown.
   *  
   *  If an operation fails or an error occures, this method will throw an
   *  exception.
   *  
   *  This is a blocking function that can take several seconds to complete.
   *  
   *  \return true if the LinkMasta detects a connected cartridge, false if not.
   */
  virtual bool             test_for_cartridge() = 0;
  
  /*!
   *  \brief Builds a \ref cartridge object that can be used to perform
   *         high-level cartidge operations on.
   *  
   *  Builds a \ref cartridge object that can be used to perform high-level
   *  cartridge operations. This convenience function allows a specific
   *  implementation of a LinkMasta to instantiate a subclass of \ref cartridge
   *  that is specific to the system this LinkMasta is designed to work with.
   *  
   *  \returns A pointer to a \ref cartridge object that can be used to perform
   *           high-level operations, or nullptr if an error occured. This
   *           object may not be initialized when returned.
   */
  virtual cartridge*       build_cartridge() = 0;
  
  /*!
   *  \brief Gets the \ref linkmasta_system enum indicating the game system with
   *         which this object was designed to work with.
   *  
   *  Gets the \ref linkmastaa_system enum indicating the game system with which
   *  this object was designed to work with. This value can be used to determine
   *  how to cast a pointer to this object.
   *  
   *  \return \ref linkmasta_system enumeration indicating the type of system
   *          this implementation supports.
   */
  virtual linkmasta_system system() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *  to \ref read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr).
   *  
   *  Gets whether or not this particular implementation supports reading bytes.
   *  If this method returns false, then calls to \ref read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr),
   *          false if not. Unless overridden, this function returns false.
   *  
   *  \see read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr)
   */
  virtual bool             supports_read_bytes() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_program_bytes() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref erase_chip(chip_index chip).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref erase_chip(chip_index chip)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref erase_chip(chip_index chip),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_erase_chip() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref erase_chip_block(chip_index chip, address_t block_address).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref erase_chip_block(chip_index chip, address_t block_address)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref erase_chip_block(chip_index chip, address_t block_address),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_erase_chip_block() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref read_manufacturer_id(chip_index chip).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref read_manufacturer_id(chip_index chip)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref read_manufacturer_id(chip_index chip),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_read_manufacturer_id() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref read_device_id(chip_index chip).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref read_device_id(chip_index chip)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref read_device_id(chip_index chip),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_read_device_id() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref read_block_protection(chip_index chip, address_t block_address).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref read_block_protection(chip_index chip, address_t block_address)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref read_block_protection(chip_index chip, address_t block_address),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_read_block_protection() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref read_num_slots().
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref read_num_slots()
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref read_num_slots(),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_read_num_slots() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref read_slot_size(unsigned int slot_num).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref read_slot_size(unsigned int slot_num)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref read_slot_size(unsigned int slot_num),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_read_slot_size() const;
  
  /*!
   *  \brief Gets whether or not this particular implementation supports calls
   *         to \ref switch_slot(unsigned int slot_num).
   *  
   *  Gets whether or not this particular implementation supports programming
   *  bytes. If this method returns false, then calls to \ref switch_slot(unsigned int slot_num)
   *  are likely to throw an exception.
   *  
   *  \return true if this implementation supports calls to \ref switch_slot(unsigned int slot_num),
   *          false if not. Unless overridden, this function returns false.
   */
  virtual bool             supports_switch_slot() const;
  
  
  
  /*!
   *  \brief Reads a sequence of bytes from the indicated chip on the connected
   *         cartridge into the provided array.
   *  
   *  Reads a sequence of bytes from the indicated chip on the connected
   *  cartridge into the provided array. Does not perform bounds checking, thus
   *  it is the responsibility of the caller to understand the limits of the
   *  hardware device before misusing this function.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_read_bytes() first before calling this method.
   *  
   *  This is a blocking function that can take a long time to complete. An
   *  optional \ref task_controller can be provided to track the progress of the
   *  operation.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip to read the bytes from.
   *  \param [in] start_address The start address to begin reading the sequence
   *         of bytes in increasing order.
   *  \param [out] buffer Output array to which all read bytes will be written.
   *         Must be allocated and at least as large as the value of
   *         \ref num_bytes
   *  \param [in] num_bytes Number of sequential bytes to read from the device.
   *  \param [out] controller Optional \ref task_controller object to report
   *         progress of the operation to. If nullptr is given, then this
   *         parameter will be ignored.
   *  
   *  \return The number of bytes successfully read from the device.
   *  
   *  \see supports_read_bytes()
   */
  virtual unsigned int     read_bytes(chip_index chip, address_t start_address, data_t* buffer, unsigned int num_bytes, task_controller* controller = nullptr);
  
  /*!
   *  \brief Programs a sequence of bytes to the indicated chip on the connected
   *         cartridge at the provided address.
   *  
   *  Programs a sequence of bytes to the indicated chip on the connected
   *  cartridge at the provided address. Does not perform bounds checking, thus
   *  it is the responsibility of the caller to understand the limits of the
   *  hardware device before misusing this function.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_program_bytes() first before calling this method.
   *  
   *  This is a blocking function that can take a long time to complete. An
   *  optional \ref task_controller can be provided to track the progress of the
   *  operation.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip to program the bytes to.
   *  \param [in] start_address The starting address to begin programming data
   *         in ascending order.
   *  \param [in] buffer Array of data to program to the device.
   *  \param [in] num_bytes The number of bytes to program to the device.
   *  \param [out] controller An optional \ref task_controller to report
   *         progress of the operation to. If nullptr is given, then this
   *         parameter will be ignored.
   *  
   *  \return The number of bytes successfully programmed to the device.
   *  
   *  \see supports_program_bytes()
   */
  virtual unsigned int     program_bytes(chip_index chip, address_t start_address, const data_t* buffer, unsigned int num_bytes, bool bypass_mode, task_controller* controller = nullptr);
  
  /*!
   *  \brief Erases an entire chip on the connected cartridge.
   *  
   *  Erases and the contents of an entire chip.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_erase_chip() first before calling this method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip to erase.
   *  
   *  \see supports_erase_chip()
   */
  virtual void             erase_chip(chip_index chip);
  
  /*!
   *  \brief Erases an individual block of memory on a chip on the connected
   *         cartridge.
   *  
   *  Erase the contents of an individual block of memory on a chip on the
   *  connected cartridge.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_erase_chip_block() first before calling this method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip on which the target block is
   *         located.
   *  \param [in] block_address The base address of the memory block to be
   *         erased.
   *  
   *  \see supports_erase_chip_block()
   */
  virtual void             erase_chip_block(chip_index chip, address_t block_address);
  
  /*!
   *  \brief Reads the manufacturer's ID from the specified chip on the
   *         connected cartridge.
   *  
   *  Reads the manufacturer's ID from the specified chip on the connected
   *  cartridge.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_read_manufacturer_id() first before calling this
   *  method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip from which to fetch the
   *         manufacturer's ID.
   *  
   *  \return The numerical ID of the device's manufacturer.
   *  
   *  \see supports_read_manufacturer_id()
   */
  virtual unsigned int     read_manufacturer_id(chip_index chip);
  
  /*!
   *  \brief Reads the device's ID from the specified chip on the connected
   *         cartridge.
   *  
   *  Reads the device's ID from the specified chip on the connecteed cartridge.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_read_device_id() first before calling this
   *  method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip from which to fetch the device's
   *         ID.
   *  
   *  \return The numerical ID of the device.
   *  
   *  \see supports_read_device_id()
   */
  virtual unsigned int     read_device_id(chip_index chip);
  
  /*!
   *  \brief Reads the block protection status of a particular block on a chip
   *         on the connected cartridge.
   *  
   *  Reads the block protection status of a particular block of memory on a
   *  chip on the connected cartridge.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_read_block_protection() first before calling this
   *  method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] chip The index of the chip from which to fetch the device's
   *         ID.
   *  \param [in] block_address The base address of the block of memory to get
   *         protection information on.
   *  
   *  \return true if the memory block is write-protected, false if not.
   *  
   *  \see supports_read_block_protection()
   */
  virtual bool             read_block_protection(chip_index chip, address_t block_address);
  
  /*!
   *  \brief Reads the number of game slots that exist on the connected
   *         cartridge.
   *  
   *  Reads the number of game slots that exist on the connected cartridge.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_read_num_slots() first before calling this
   *  method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \return The number of game slots on the cartridge.
   *  
   *  \see supports_read_num_slots()
   */
  virtual unsigned int     read_num_slots();
  
  /*!
   *  \brief Reads the size in bytes of a specific game slot on the cartridge.
   *  
   *  Reads the size in bytes of the specific game slot on the cartridge.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_read_slot_size() first before calling this
   *  method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] slot_num The index of the slot to get the size of.
   *  
   *  \return The number of bytes that slot with index \ref slot_num occupies.
   *  
   *  \see supports_read_slot_size()
   */
  virtual unsigned int     read_slot_size(unsigned int slot_num);
  
  /*!
   *  \brief Switches the current working slot.
   *  
   *  Switches the current working slot. In some implementations, this actually
   *  changes the address frame on the device, and thus it is important that
   *  the caller understand the underlying implementation of this method.
   *  
   *  Not all implementations will support this method and some may throw an
   *  exception if they do not. Check if the implementation supports this method
   *  by calling \ref supports_switch_slot() first before calling this method.
   *  
   *  This is a blocking function that can take a long time to complete.
   *  
   *  If an error occurs during this operation, an exception will be thrown and
   *  the device may be in an unresponsive state. This may mean the device will
   *  need to be physically disconnected from the system in order to be reset.
   *  
   *  \param [in] slot_num The index of the slot to switch to.
   *  
   *  \return true if the operation was successful, false if not.
   *  
   *  \see supports_switch_slot()
   */
  virtual bool             switch_slot(unsigned int slot_num);
};

#endif /* defined(__LINKMASTSA_DEVICE_H__) */
