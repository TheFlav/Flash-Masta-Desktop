/*! \file
 *  \brief File containing the declaration of the \ref cartridge class.
 *  
 *  File containing the header information and declaration of the \ref cartridge
 *  class. This file includes the minimal number of files necessary to use any
 *  instance of the \ref cartridge class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-28
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "../common/types.h"
#include "cartridge_descriptor.h"
#include <iosfwd>
#include <string>

class task_controller;



/*! \class cartridge
 *  \brief Provides functionality for working with data on a game cartridge.
 *  
 *  Abstract class representing a generic game cartridge for an unknown game
 *  system. Provides functions for performing high-level operations on a
 *  cartridge.
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class cartridge
{
public:
  
  /*!
   *  \brief Constant sentinal value for indicating an operation is to be
   *         performed on an entire cartridge's contants rather than on a
   *         specific slot.
   */
  static int const    SLOT_ALL = -1;
  
  
  
  /*!
   *  \brief Class destructor.
   *  
   *  Destructor for the class. Cleans up and frees resources.
   */
  virtual             ~cartridge() { /* Provides virtual destructor */ };
  
  
  
  /*! \brief Get the game system to which the cartridge belongs.
   *  
   *  Gets an enum for the cartridge system for which this cartridge was built
   *  for. This enum can be used for determining how to treat this object and
   *  what to cast it to. If the system type us unknown or is undefined, this
   *  function will return \ref system_type::SYSTEM_UNKNOWN.
   *  
   *  \returns \ref system_type::SYSTEM_UNKNOWN - The system type is unknown or
   *           undefined.
   *  \returns \ref system_type::SYSTEM_NEO_GEO_POCKET - The cartridge is
   *           compatible with a Neo Geo Pocket or Neo Geo Pocket Color system.
   *  \returns \ref system_type::SYSTEM_WONDERSWAN - The cartridge is compatible
   *           with a WonderSwan or a WonderSwan color system.
   *  
   *  \see system_type
   */
  virtual system_type system() const = 0;
  
  /*! \brief Get the type of cartridge. Used to determine whether the cartridge
   *         is an official cartridge, a Flash Masta, or unrecognized.
   *  
   *  Gets an enum representing the cartridge type. This enum can be used for
   *  determining how to operate on the object and what methods are available or
   *  unavailable. If the cartridge type is unknown or undefined, this function
   *   will return \ref cartridge_type::CARTRIDGE_UNKNOWN.
   *  
   *  \return \ref cartridge_type::CARTRIDGE_UNKOWN - The cartridge type is
   *          unknown, unrecognized, or undefined.
   *  \return \ref cartridge_type::CARTRIDGE_OFFICIAL - The cartridge is an
   *          official, unmodified cartridge.
   *  \return \ref cartridge_type::CARTRIDGE_FLASHMASTA - The cartridge is a
   *          Flash Masta, which is a modified official cartridge that can be
   *          written to and behaves differently.
   */
  virtual cartridge_type type() const = 0;
  
  /*! \brief Gets a [cartridge_descriptor](\ref cartridge_descriptor) that
   *  provies information about the hardware of this cartridge instance.
   *  
   *  Gets a \ref cartridge_descriptor struct that contains cartridge hardware
   *  informaiton, including the capacity of the cartridge and each individual
   *  storage chip on the cartridge. This function only fetches the cached
   *  version that is generated on initialization.
   *  
   *  If a call to this function is made before a call to \ref init() is made,
   *  the function will return a **nullptr**.
   *  
   *  \see cartridge_descriptor
   */
  virtual const cartridge_descriptor* descriptor() const = 0;
  
  
  
  /*! \brief Initializes the cartridge using default settings.
   *  
   *  Initializes the cartridge with default settings. This is a blocking
   *  function that could take several seconds to complete depending on timeout
   *  configurations and the state of the environment in which it is called.
   *  
   *  If this function has been called previously on the current instance of the
   *  class, then nothing will happen.
   */
  virtual void        init() = 0;
  
  /*! \brief Writes a cartridge's game data to an output stream.
   *
   *  Extracts the game data from a cartridge and writes its contents to an
   *  output stream. This data may also contain save data or garbage; its exact
   *  format is implementation- and cartridge-specific.
   *  
   *  The stream must already be open and ready to read. This function will have
   *  full control of the stream, meaning that there is no guarantee to the
   *  state of the stream when this function returns control. If the stream is
   *  an instance of \ref std::fstream, then the file should be opened in binary
   *  mode using \ref std::ios::binary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [out] fout The output stream to write to.
   *  \param [in] slot The game slot on the cartridge to back up in the case of
   *         multiple games on a single cartridge. Set to \ref SLOT_ALL to
   *         backup the entire cartridge.
   *  \param [in,out] controller (optional) The controller object to send
   *         progress updates. **nullptr** is an accepted value.
   *  
   *  \see std::ofstream
   *  \see task_controller
   */
  virtual void        backup_cartridge_game_data(std::ostream& fout, int slot = SLOT_ALL, task_controller* controller = nullptr) = 0;
  
  /*! \brief Overwrites a cartridge's game data with data from an input stream.
   *  
   *  Replaces game data on the cartridge with the content of an input stream.
   *  The format of the content in the input stream is implementation- and
   *  cartridge specific and it is the responsibility of the caller to provide
   *  data in the correct format.
   *
   *  The stream must already be open and ready to read. This function will have
   *  full control of the stream, meaning that there is no guarantee to the
   *  state of the stream when this function returns control. The stream cannot
   *  be a reference to any of the standard input or output streams, e.g.
   *  \ref std::cin. If the stream is an instance of \ref std::fstream, then the
   *  file should be opened in binary mode using \ref std::ios::binary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [in] fin The input stream to read from. Cannot be any of the
   *         standard input streams, e.g. \ref std::cin.
   *  \param [in] slot The game slot on the cartridge to write to in the case of
   *         multiple games on a single cartridge. Set to \ref SLOT_ALL to
   *         overwrite the entire cartridge.
   *  \param [in,out] controller (optional) The controller object to send
   *         progress updates. **nullptr** is an accepted value.
   *  
   *  \throws std::invalid_argument Input stream is a standard input stream.
   *  
   *  \see std::istream
   *  \see task_controller
   */
  virtual void        restore_cartridge_game_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr) = 0;
  
  /*! \brief Compares the cartridge's game data with the contents of an input
   *         stream.
   *  
   *  Compares the game data on the cartridge to the game data in an input
   *  stream. The format of the content in the input stream is implementation-
   *  and cartridge-specific and it is the responsibility of the caller to
   *  provide data in the correct format.
   *  
   *  The stream must already be open and ready to read. This function will have
   *  full control of the stream, meaning that there is no guarantee to the
   *  state of the stream when this function returns control. The stream cannot
   *  be a reference to any of the standard input or output streams, e.g.
   *  \ref std::cin. If the stream is an instance of \ref std::ifstream, then
   *  the file should be opened in binary mode using \ref std::ios::binary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [in] fout The input stream to read from. Cannot be any of the
   *         standard input streams, e.g. \ref std::cin.
   *  \param [in] slot The game slot on the cartridge to compare in the case of
   *         multiple games on a single cartridge. Set to \ref SLOT_ALL to
   *         compare the entire cartridge.
   *  \param [in,out] controller (optional) The controller object to send
   *         progress updates. **nullptr** is an accepted value.
   *  
   *  \returns **true** The cartridge game data and input stream contents match.
   *  \returns **false** The cartridge game data and input stream contents do
   *           not match.
   *  
   *  \throws std::invalid_argument Input stream is a standard input stream.
   *  
   *  \see std::istream
   *  \see task_controller
   */
  virtual bool        compare_cartridge_game_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr) = 0;
  
  /*! \brief Saves a cartridge's game save data to an output stream.
   *
   *  Extracts the game save data from a cartridge and writes its contents to an
   *  output stream. This data may also contain game data or garbage; its exact
   *  format is implementation- and cartridge-specific.
   *  
   *  The stream must already be open and ready to read. This function will have
   *  full control of the stream, meaning that there is no guarantee to the
   *  state of the stream when this function returns control. If the stream is
   *  an instance of \ref std::fstream, then the file should be opened in binary
   *  mode using \ref std::ios::binary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [out] fout The output stream to write to.
   *  \param [in] slot The game slot on the cartridge to backup save data from
   *         in the case of multiple games on a single cartridge. Set to
   *         \ref SLOT_ALL to backup save data from all slots on the cartridge.
   *  \param [in,out] controller (optional) The controller object to send
   *         progress updates. **nullptr** is an accepted value.
   *  
   *  \see std::ofstream
   *  \see task_controller
   */
  virtual void        backup_cartridge_save_data(std::ostream& fout, int slot = SLOT_ALL, task_controller* controller = nullptr) = 0;
  
  /*! \brief Overwrites a cartridge's game save data with the contents of an
   *         input stream.
   *  
   *  Replaces game save data on the cartridge with the content of an input
   *  stream. The format of the content in the input stream is implementation-
   *  and cartridge-specific and it is the responsibility of the caller to
   *  provide data in the correct format.
   *
   *  The stream must already be open and ready to read. This function will have
   *  full control of the stream, meaning that there is no guarantee to the
   *  state of the stream when this function returns control. The stream cannot
   *  be a reference to any of the standard input or output streams, e.g.
   *  \ref std::cin. If the stream is an instance of \ref std::fstream, then the
   *  file should be opened in binary mode using \ref std::ios::binary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [in] fin The input stream to read from. Cannot be any of the
   *         standard input streams, e.g. \ref std::cin.
   *  \param [in] slot The game slot on the cartridge to restore save data to
   *         in the case of multiple games on a single cartridge. Set to
   *         \ref SLOT_ALL to restore save data to all slots on the cartridge.
   *  \param [in,out] controller (optional) The controller object to send
   *         progress updates. **nullptr** is an accepted value.
   *  
   *  \throws std::invalid_argument Input stream is a standard input stream.
   *  
   *  \see std::istream
   *  \see task_controller
   */
  virtual void        restore_cartridge_save_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr) = 0;
  
  /*! \brief Compares the cartridge's game save data with the contents of an
   *         input stream.
   *  
   *  Compares the game save data on the cartridge to the game save data in an
   *  input stream. The format of the content in the input stream is
   *  implementation- and cartridge-specific and it is the responsibility of the
   *  caller to provide data in the correct format.
   *  
   *  The stream must already be open and ready to read. This function will have
   *  full control of the stream, meaning that there is no guarantee to the
   *  state of the stream when this function returns control. The stream cannot
   *  be a reference to any of the standard input or output streams, e.g.
   *  \ref std::cin. If the stream is an instance of \ref std::ifstream, then
   *  the file should be opened in binary mode using \ref std::ios::binary.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. A \ref task_controller object may be optionally provided to
   *  allow for mid-process communication and progress updates. If no controller
   *  is supplied or **nullptr** is given, then this feature will be ignored.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [in] fout The input stream to read from. Cannot be any of the
   *         standard input streams, e.g. \ref std::cin.
   *  \param [in] slot The game slot on the cartridge to compare save data to
   *         in the case of multiple games on a single cartridge. Set to
   *         \ref SLOT_ALL to compare against save data from all slots on the
   *         cartridge.
   *  \param [in,out] controller (optional) The controller object to send
   *         progress updates. **nullptr** is an accepted value.
   *  
   *  \returns **true** The cartridge game save data and input stream contents
   *           match.
   *  \returns **false** The cartridge game save data and input stream contents
   *           do not match.
   *  
   *  \throws std::invalid_argument Input stream is a standard input stream.
   *  
   *  \see std::istream
   *  \see task_controller
   */
  virtual bool        compare_cartridge_save_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr) = 0;
  
  /*! \brief Gets the number of game data slots exist on the cartridge.
   *  
   *  Reports the number of game "slots" that the cartridge can hold. This can
   *  be used for determining the maximum number of games a cartridge can hold.
   *  
   *  \returns The maximum number of slots a cartridge can support.
   */
  virtual unsigned int num_slots() const = 0;
  
  /*! \brief Gets the number of bytes in a given game slot.
   *  
   *  Gets the number of bytes in a given game slot. Can be used to check if a
   *  file will fit in a single slot size on a cartridge.
   */
  virtual unsigned int slot_size(int slot) const = 0;
  
  /*! \brief Fetches the name of a game located in a given slot.
   *  
   *  Gets the name of the game on the cartridge in the given slot. If no game
   *  exists at the given slot, this function may return garbage.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete.
   *  
   *  If a call to this funtion is made before a call to \ref init() is made,
   *  this function will throw an exception and no other action will be taken.
   *  
   *  \param [in] slot The game slot on the cartridge to fetch the game name
   *         from.
   *  
   *  \returns A string of 12 or fewer characters that represent the name of the
   *           gone on the cartridge.
   */
  virtual std::string  fetch_game_name(int slot) = 0;
};

#endif // defined(__CARTRIDGE_H__)
