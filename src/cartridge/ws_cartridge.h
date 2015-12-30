/*! \file
 *  \brief File containing the declaration of the \ref ws_cartridge class.
 *  
 *  File containing the header information and declaration of the
 *  \ref ws_cartridge class. This file includes the minimal number of files
 *  necessary to use any instance of the \ref ws_cartridge class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-08-17
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __WS_CARTRIDGE_H__
#define __WS_CARTRIDGE_H__

#include "cartridge.h"
#include <vector>

class linkmasta_device;
class ws_rom_chip;
class ws_sram_chip;

/*! \class ws_cartridge
 *  \brief Class representing a WonderSwan game cartridge.
 *  
 *  Class representing a WonderSwan game cartridge. Stores information relevant
 *  to cartridge storage and hardware information, such as the number of onboard
 *  chips and exactly how the chips are divided into blocks. Implements
 *  functions for performing high-level operations on a cartridge.
 *  
 *  This class does not support copying or the copy assignment operator (=).
 *  
 *  This class is *not* thread-safe. Use caution when working in a multithreaded
 *  environment.
 */
class ws_cartridge: public cartridge
{
public:
  /*! \brief A struct for representing and storing the metadata of a Wonderswan
   *         game.
   */
  struct game_metadata
  {
    /*! \brief Expects an array of the last 10 bytes of the game. */
    void read_from_data_array(const unsigned char* data);
    
    /*! \brief Expects an array of at least 10 bytes. */
    void write_to_data_array(unsigned char* data);
    
    unsigned char  developer_id;
    unsigned char  minimum_system;
    unsigned char  game_id;
    unsigned char  mapper_version;
    unsigned char  rom_size;
    unsigned char  save_size;
    unsigned char  flags;
    unsigned char  RTC_present;
    unsigned short checksum;
  };
  
  
  
  /*! \brief Class constructor
   *  
   *  Main constructor for the class. Initializes members with default values
   *  and supplied parameters.
   *  
   *  \param linkmasta Pointer ot valid instance of \ref linkmasta_device to use
   *         to communicate with the cartridge hardware. This class does not
   *         assume responsibility for deallocating the object on deletion, and
   *         thus the responsibility for deleting the \ref linkmasta_device
   *         falls to the caller of this function.
   */
                        ws_cartridge(linkmasta_device* linkmasta);
  
  /*! \brief Class destructor.
   *  
   *  Handles memory cleanup and closes any open connections. However, any
   *  \ref linkmasta_device supplied to this object during construction will
   *  *not* be deallocated, and thus must be freed manually.
   */
                        ~ws_cartridge();
  
  /*!
   *  \see cartridge::system()
   *  
   *  \returns \ref system_type::SYSTEM_WONDERSWAN - The cartridge is compatible with
   *           a WonderSwan or WonderSwan Color system.
   */
  system_type           system() const;
  
  /*! \see cartridge::type() */
  cartridge_type        type() const;
  
  /*!
   *  \see cartridge::descriptor()
   */
  const cartridge_descriptor* descriptor() const;
  
  
  
  /*!
   *  \see cartridge::init()
   */
  void                  init();
  
  /*!
   *  \see cartridge::backup_cartridge_game_data(std::ostream& fout, task_controller* controller = nullptr)
   */
  void                  backup_cartridge_game_data(std::ostream& fout, int slot = SLOT_ALL, task_controller* controller = nullptr);
  
  /*!
   *  \see cartridge::restore_cartridge_game_data(std::istream& fin, task_controller* controller = nullptr)
   */
  void                  restore_cartridge_game_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  
  /*!
   *  \see cartridge::compare_cartridge_game_data(std::istream& fin, task_controller* controller = nullptr)
   */
  bool                  compare_cartridge_game_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  
  /*!
   *  \see cartridge::backup_cartridge_save_data(std::ostream& fout, task_controller* controller = nullptr)
   */
  void                  backup_cartridge_save_data(std::ostream& fout, int slot = SLOT_ALL, task_controller* controller = nullptr);
  
  /*!
   *  \see cartridge::restore_cartridge_save_data(std::istream& fin, task_controller* controller = nullptr)
   */
  void                  restore_cartridge_save_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  
  /*!
   *  \see cartridge::compare_cartridge_save_data(std::istream& fin, task_controller* controller = nullptr)
   */
  bool                  compare_cartridge_save_data(std::istream& fin, int slot = SLOT_ALL, task_controller* controller = nullptr);
  
  /*!
   *  \see cartridge::num_slots() const
   */
  unsigned int          num_slots() const;
  
  /*!
   *  \see cartridge::slot_size(int slot) const
   */
  unsigned int          slot_size(int slot) const;
  
  /*!
   *  \see cartridge::fetch_game_name(int slot)
   */
  std::string           fetch_game_name(int slot);
  
  /*!
   * \brief Gets the parsed metadata of the game in the given slot.
   * 
   * Gets a pointer to a struct containing the parsed metadata for the game
   * stored in the given slot. If something went wrong while parsing the metadata
   * or no metadata has been cached for the given slot, then this function returns
   * a nullptr.
   * 
   * This is a non-blocking function as it merely returns a cached value. This cached
   * value is constructed when \ref init() is called on this cartridge.
   * 
   * \param slot The slot of the game to get the metadata for.
   * 
   * \return Pointer to a const \ref game_metadata struct or nullptr something
   *         went wrong.
   * 
   * \see game_metadata
   */
  const game_metadata*  get_game_metadata(int slot) const;
  
  /*!
   *  \brief Gets the calculated number of bytes for a game in the given slot.
   *  
   *  Gets the number of bytes the game in the given slot occupies. This value
   *  is directly derived from the game metadata stored in that slot. This value
   *  may not be the same as that of the slot itself.
   *  
   *  This is a non-blocking function as it merely returns a cached value. This
   *  cached value is constructed when \ref init() is called on this cartridge.
   *  
   *  \param slot The slot of the game to get the metadata for.
   *  
   *  \return The number of bytes the game in slot \ref slot occupies which may
   *          be fewer than the size of the slot itself.
   */
  unsigned int          get_game_size(int slot) const;
  
  /*!
   *  \brief Calculates the number of bytes a game occupies given a numerical
   *        size code.
   *  
   *  Calcualtes the number of bytes a game occupies given a numerical size
   *  code.
   *  
   *  \param [in] size_code The size code to translate to bytes.
   *  
   *  \return The number of bytes the game is expected to occupy. If
   *          \ref size_code is not recognized, a 0 is returned.
   */
  static unsigned int   calculate_game_size(int size_code);
  
  

protected:
  
  /*! \brief Constructs a \ref cartridge_descriptor struct using information
   *         gathered from the associated \ref linkmasta_device.
   *  
   *  Uses the associated \ref linkmasta_device to query for information on the
   *  cartridge and its onboard chips. Gathers information on cartridge size and
   *  the number of flash storage chips on the cartridge. This function calls
   *  \ref build_chip_descriptor(unsigned int chip_i) automatically to gather
   *  information about the onboard hardware.
   *  
   *  Before building the descriptor, this function replaces the internally
   *  cached descriptor with the newly created one. To access the newly created
   *  descriptor, use \ref descriptor().
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. The function is provided as-is and any timeouts should be
   *  configured with the supplied \ref linkmasta_device beforehand.
   *  
   *  \see build_chip_descriptor(unsigned int chip_i)
   *  \see build_block_descriptor(unsigned int chip_i, unsigned int block_i)
   */
  void                  build_cartridge_destriptor();
  
  /*! \brief Creates and populates a \ref cartridge_descriptor::chip_descriptor
   *         struct using information gathered from the associated
   *         \ref linkmasta_device.
   *  
   *  Uses the associated \ref linkmasta_device to query for information on the
   *  cartridge's onboard flash storage chips. Gathers information on the chip's
   *  device id, manufacturer id, storge capacity, and sector layout. Calls
   *  \ref build_block_descriptor(unsigned int chip_i, unsigned int block_i)
   *  automatically to gather information about each sector. This function is
   *  automatically called by \ref build_cartridge_descriptor().
   *
   *  After building the descriptor, this function updates the internally
   *  cached descriptor with the newly created one. To access the result of this
   *  function, use \ref descriptor() to access the cartridge's descriptor, then
   *  access the structure at the desired index of
   *  \ref cartridge_descriptor::chips.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. The function is provided as-is and any timeouts should be
   *  configured with the supplied \ref linkmasta_device beforehand.
   *  
   *  \param [in] chip_i The index of the chip to build the struct from. Index
   *         0 will refer to the first chip on the cartridge.
   *  
   *  \see build_cartridge_descriptor()
   *  \see build_block_descriptor(unsigned int chip_i, unsigned int block_i)
   */
  void                  build_chip_descriptor(unsigned int chip_i);
  
  /*! \brief Creats and populates a
   *         \ref cartridge_descriptor::chip_descriptor::block_descriptor
   *         struct using information gathered from the associated
   *         \ref linkmasta_device.
   *  
   *  Uses the associated \ref linkmasta_device to query for information on the
   *  specified chip's sectors. Gathers information on the sector's storage
   *  capacity, base address, and write protection status. This function is
   *  automatically called by \ref build_chip_descriptor(unsigned int chip_i).
   *  
   *  After building the descriptor, this function updates the internally
   *  cached descriptor with the newly created one. To access the result of this
   *  function, use \ref descriptor() to access the cartridge's descriptor, then
   *  access the structure at the desired index of
   *  \ref cartridge_descriptor::chips, and form there access the structure
   *  at the desired index of
   *  \ref cartridge_descriptor::chip_descriptor::blocks.
   *  
   *  This function is a blocking function that can take several seconds to
   *  complete. The function is provided as-is and any timeouts should be
   *  configured with the supplied \ref linkmasta_device beforehand.
   *  
   *  \param [in] chip_i The index of the chip to build the struct from. Index
   0 will refer to the first chip on the cartridge.
   *  \param [in] block_i The index of the block on the chip to build the
   *         descriptor from. Index 0 will refer to the first sector on the
   *         chip.
   *  
   *  \see build_cartridge_descriptor()
   *  \see build_chip_descriptor(unsigned int chip_i)
   */
  void                  build_block_descriptor(unsigned int chip_i, unsigned int block_i);
  
  /*! \brief Reads data from the cartridge to determine the game slot layout of
   *         the chip, caching the results to avoid future cartridge queries.
   *  
   *  Reads data from the cartridge to determine the game slot layout of the
   *  chip, caching the results to avoid future cartridge queries.
   */
  void                  build_slots_layout();
  
  /*! \brief Reads game metadata from the cartridge and caches it for later use.
   * 
   *  Reads data from the cartridge to get game metadata from all game slots on
   *  the cartridge, caching the results to avoid future cartridge queries.
   *  
   *  \param [in] slot Target game slot to fetch metadata for. -1 will fetch
   *                   metadata from all available slots on the cartridge.
   */
  void                  build_game_metadata(int slot = -1);
  
  
  
private:
  
  /*! \brief Disabled copy constructor.
   *  
   *  The copy constructor for this class. Because this class cannot be copied
   *  due to its nature, this method has been disabled. Instead, use one of the
   *  main constructors.
   *
   *  \param [in] other The original instance of the class to copy.
   *  
   *  \see ws_cartridge(linkmasta_device* linkmasta)
   */
                        ws_cartridge(const ws_cartridge& other) = delete;
  
  /*! \brief Disabled copy assignment constructor.
   *  
   *  The copy assignment operator for this class. Because this class cannot be
   *  copied due to its nature, this method has been disabled.
   *  
   *  \param [in] other The original instance of the class to copy.
   *  
   *  \returns The instance of the class on which this method was called. I.e.,
   *           the left-hand side of the assignment operator.
   */
  ws_cartridge&         operator=(const ws_cartridge& other) = delete;
  
  
  
  /*! \brief Flag indicating that \ref init() has been previously called on this
   *         instance.
   *  
   *  Flag indicating that \ref init() has been previously called on this
   *  instance. This variable is set to **false** when the object is first
   *  constructed, then set to **true** once \ref init() has been called. This
   *  variable should not be set to **false** after it has previously been set
   *  to **true**.
   *  
   *  \see init()
   */
  bool                  m_was_init;
  
  /*! \brief Pointer to the \ref linkmasta_device that the object will use for
   *         communication with the hardware.
   *  
   *  Pointer to the \ref linkmasta_device that the object will use for
   *  communication with the hardware. This variable must be initialized with
   *  a valid pointer during object creation.
   *  
   *  \see linkmasta_device
   *  \see ws_cartridge(linkmasta_device* linkmasta)
   */
  linkmasta_device*     m_linkmasta;
  
  /*! \brief A pointer to the cached \ref cartridge_descriptor struct that
   *         describes the hardware of the cartridge.
   *  
   *  A pointer to the cached \ref cartridge_descriptor struct that describes
   *  the hardware of the cartridge. This variable is set to **nullptr** until
   *  \ref init() or \ref build_cartridge_descriptor() is called.
   *  
   *  \see cartridge_descriptor
   *  \see init()
   *  \see build_cartridge_descriptor()
   */
  cartridge_descriptor* m_descriptor;
  
  /*! \brief A pointer to the \ref ws_rom_chip instance that handles
   *         communications with the cartridge's ROM device.
   *  
   *  A pointer to the \ref ws_rom_chip instance that handles communications
   *  with the cartridge's ROM device. This member is initialized when the
   *  \ref init() function is called and is deallocated on deconstruction. It
   *  should be used for any and all communications with the ROM chip of the
   *  cartridge, as it contains all of the core logic for interacting with flash
   *  storage.
   *  
   *  \see ws_rom_chip
   *  \see init()
   */
  ws_rom_chip*          m_rom_chip;
  
  /*! \brief A pointer ot the \ref ws_sram_chip object that handles
   *         communications with the cartridge's SRAM device.
   *  
   *  A pointer ot the \ref ws_sram_chip object that handles communications with
   *  the cartridge's SRAM device. This member is initialized when the
   *  \ref init() function is called and is deallocated on deconstruction. It
   *  should be used for any and all communications with the SRAM chip on the
   *  cartridge, as it contains all of the necessary logic for interacting with
   *  SRAM storage.
   */
  ws_sram_chip*         m_sram_chip;
  
  /*! \brief The vector used for caching the slot layout of the cartridge.
   *  
   *  The vector used for caching the slot layout of the cartridge. Stored as an
   *  array of slot sizes, where the base address of each slot is calculated by
   *  summing all previous slot's sizes. This vector should be used when making
   *  slot-based calculations instead of polling the cartridge itself.
   *  
   *  This member is initialized and calculated when calling the
   *  \ref build_slots_layout() function.
   */
  std::vector<unsigned int> m_slots;
  
  /*! \brief The vector used for caching game metadata for each slot.
   *  
   *  The vector used for caching the game metdata for each slot on the
   *  cartridge.
   *  
   *  This member is initialized and filled with values when calling the
   *  \ref build_game_metadata() function.
   *  
   *  \see game_metadata
   */
  std::vector<game_metadata> m_metadata;
};

#endif /* defined(__WS_CARTRIDGE_H__) */
