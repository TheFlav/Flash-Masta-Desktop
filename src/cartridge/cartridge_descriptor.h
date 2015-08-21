/*! \file
 *  \brief File containing declaration of the \ref cartridge_descriptor struct
 *         its related structs and enums.
 *  
 *  File containing the delcaration of the \ref cartridge_descriptor struct and
 *  any required components, such as \ref cartridge_descriptor::chip_descriptor,
 *  \ref cartridge_descriptor::chip_descriptor::block_descriptor, and
 *  \ref system_type.
 *  
 *  \author Daniel Andrus
 *  \date 2015-07-28
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __CARTRIDGE_DESCRIPTOR_H__
#define __CARTRIDGE_DESCRIPTOR_H__



/** \enum system_type
 *  \brief Enumeration representing a system to which a game cartridge belongs.
 *  
 *  Enumeration indicating which game system an object is designed to work with.
 *  Can be used to change how a segment of code processes a game cartridge.
 */
enum system_type
{
  /*! \brief Unknown cartridge type */
  UNKNOWN,
  
  /*! \brief Neo Geo Pocket or Neo Geo Pocket Color */
  NEO_GEO_POCKET,
  
  /*! \brief Wonderswan or Wonderswan Color */
  WONDERSWAN
};



/*! \struct cartridge_descriptor
 *  \brief Struct for describing the hardware of a game cartridge.
 *  
 *  This structure describes the hardware of a generic game cartridge. It
 *  contains fields for the system for which it was built, the total storage
 *  capacity of the cartridge in bytes, and the number of accessible chips on
 *  the cartridge.
 *  
 *  This class uses constructors and destructors for dynamic memory allocation.
 *  This allows the cartridge to hold any number of storage chips.
 *  
 *  \see system_type
 *  \see cartridge_descriptor::chip_descriptor
 *  \see cartridge_descriptor::chip_descriptor::block_descriptor
 */
struct cartridge_descriptor
{
  struct                    chip_descriptor;        // Forward declaration
  
  /*! \brief The constructor for this class.
   *  
   *  The main constructor for this class. Performs dynamic memory allocaiton
   *  which will be cleaned up when the object is destroyed.
   *  
   *  \param [in] num_chips The number of chips this cartridge will have. Must
   *         be calculated by the caller ahead of time.
   */
                            cartridge_descriptor(unsigned int num_chips);
  
  /*! \brief The copy constructor for this class.
   *  
   *  The copy constructor for this class. Creates a new instance of this class
   *  and fills it with copies of the contents of another. Performs a deep copy
   *  so that both the new and old instance are each independent of the other.
   *  
   *  \param [in] other The original instance to copy.
   */
                            cartridge_descriptor(const cartridge_descriptor& other);
  
  /*! \brief The destructor fot this class.
   *  
   *  The destructor for this class. Frees dynamically allocated memory,
   *  along with object references in the \ref chips array.
   *  
   *  \see chips
   */
                            ~cartridge_descriptor();
  
  
  
  /*! \brief Stores the system for which the described cartridge was built for.
   *  
   *  \see system_type
   */
  system_type               type;
  
  /*! \brief The storage capacity of the cartridge in bytes.
   */
  unsigned int              num_bytes;
  
  /*! \brief The total number of accessible hardware chips on the cartridge.
   *  
   *  The total number of accessible hardware chips on the cartridge. Can be
   *  used as an upper bound for the \ref chips array.
   *  
   *  \see chip
   */
  const unsigned int        num_chips;
  
  /*! \brief Dynamic array of \ref cartridge_descriptor::chip_descriptor
   *         pointers.
   *  
   *  Dynamic array of \ref cartridge_descriptor::chip_descriptor pointers. The
   *  array is allocated during object construction and is deallocated on object
   *  destruction. The elments of the array will be set to **nullptr** until
   *  manually changed. \ref num_chips can be used as an upper bound for this
   *  array.
   *  
   *  \warning Each element in this array will be destroyed when this object
   *           is destroyed. If an element must be manually deleted from this
   *           array, it is the caller's responsibility to replace it with
   *           either another valid object or **nullptr**.
   *  
   *  \see cartridge_descriptor::chip_descriptor
   *  \see num_chips
   *  \see cartridge_descriptor(unsigned int num_chips)
   *  \see ~cartridge_descriptor()
   */
  chip_descriptor** const   chips;
};



/*! \struct cartridge_descriptor::chip_descriptor
 *  \brief Struct for describing the hardware of a game cartridge's storage
 *         chip.
 *  
 *  This structure describes the hardware of a generic game cartridge's storage
 *  chip. It contains fields for the device's manufacturer id, device id,
 *  storage capacity in bytes, and number of blocks (sectors) the device is
 *  divided into. The struct can also keep track of its own index on the
 *  cartridge, which is useful for self-referencing.
 *  
 *  \see cartridge_descriptor
 *  \see cartridge_descriptor::chip_descriptor::block_descriptor
 */
struct cartridge_descriptor::chip_descriptor
{
  struct                    block_descriptor;         // Forward Declaration
  
  /*! \brief The constructor for this class.
   *  
   *  The main constructor for this class. Performs dynamic memory allocaiton
   *  which will be cleaned up when the object is destroyed.
   *  
   *  \param [in] num_blocks The number of blocks (sectors) this device will
   *         is divided into. Must be calculated by the caller ahead of time.
   */
                            chip_descriptor(unsigned int num_blocks);
  
  /*! \brief The copy constructor for this class.
   *  
   *  The copy constructor for this class. Creates a new instance of this class
   *  and fills it with copies of the contents of another. Performs a deep copy
   *  so that both the new and old instance are each independent of the other.
   *  
   *  \param [in] other The original instance to copy.
   */
                            chip_descriptor(const chip_descriptor& other);
  
  /*! \brief The destructor for this class.
   *  
   *  The destructor for this class. Frees dynamically allocated memory,
   *  along with object references in the \ref chips array.
   *  
   *  \see blocks
   */
                            ~chip_descriptor();
  
  
  
  /*! \brief The index of the chip on the cartridge.
   *  
   *  The index of the chip on the cartridge. Useful for self-referencing.
   *  Intended to be used as an index into \ref cartridge_descriptor::chips
   *  to get a pointer to itself.
   *  
   *  \see cartridge_descriptor::chips
   */
  unsigned int              chip_num;
  
  /*! \brief The id of the manufacturer of the device.
   *  
   *  The id of the manufacturer who created the device. When used in
   *  conjunction with \ref device_id, can be used to identify the exact model
   *  of a device.
   *  
   *  \see device_id
   */
  unsigned int              manufacturer_id;
  
  /*! \brief The id of the device.
   *  
   *  The id number of the device. Useful for looking up how to interact with
   *  the device. When used in conjunction with \ref manufacturer_id, can be
   *  used to identify the exact model of a device.
   *  
   *  \see manufacturer_id
   */
  unsigned int              device_id;
  
  /*! \brief The storage capacity of the chip in bytes.
   *  
   *  The total capacity of the chip in bytes.
   */
  unsigned int              num_bytes;
  
  /*! \brief The number of blocks that the device is divided into.
   *  
   *  The number of blocks, also known as sectors, that the device is divided
   *  into. Can be used as an upper bound into the \ref blocks array. This value
   *  is initialized during object construction.
   *  
   *  \see blocks
   */
  const unsigned int        num_blocks;
  
  /*! \brief Dynamic array of
   *         \ref cartridge_descriptor::chip_descriptor::block_descriptor
   *         pointers.
   *  
   *  Dynamic array of
   *  \ref cartridge_descriptor::chip_descriptor::block_descriptor pointers. The
   *  array is allocated during object construction and is deallocated on object
   *  desctruction. The elements of the array will be set to **nullptr** until
   *  manually changed. \ref num_blocks can be used as an upper bound for this
   *  array.
   *  
   *  \warning Each element in this array will be destroyed when this object
   *           is destroyed. If an element must be manually deleted from this
   *           array, it is the caller's responsibility to replace it with
   *           either another valid object or **nullptr**.
   *  
   *  \see cartridge_descriptor::chip_descriptor::block_descriptor
   *  \see num_blocks
   *  \see chip_descriptor(unsigned int num_blocks)
   *  \see ~chip_descriptor()
   */
  block_descriptor** const  blocks;
};



/*! \struct cartridge_descriptor::chip_descriptor::block_descriptor
 *  \brief Struct for describing an individual block on a game cartridge's
 *         storage chip.
 *  
 *  This structure describes an individual block (a.k.a. sector) on a game
 *  cartridge's storage chip. It contains fields for the starting address of the
 *  block, the size of the block in bytes, and whether or not the block is
 *  read-only. The struct can also keep track of its own index on the chip,
 *  which is useful for self-referencing.
 *  
 *  \see cartridge_descriptor
 *  \see cartridge_descriptor::chip_descriptor
 */
struct cartridge_descriptor::chip_descriptor::block_descriptor
{
  /*! \brief The constructor for this class.
   *  
   *  The main constructor for this class.
   */
                            block_descriptor();
  
  /*! \brief The copy constructor for the class.
   *  
   *  The copy constructor for this class. Creates a new instance of this class
   *  and fills it with copies of the contents of another.
   *  
   *  \param [in] other The original instance to copy.
   */
                            block_descriptor(const block_descriptor& other);
  
  /*! \brief the destructor for this class.
   *  
   *  The destructor for this class.
   */
                            ~block_descriptor();
  
  
  
  /*! \brief The index of the block on the chip.
   *  
   *  The index of the block on the chip. useful for self-referencing. Indended
   *  to be used as an index into
   *  \ref cartridge_descriptor::chip_descriptor::blocks to get a pointer to
   *  itself.
   *  
   *  \see cartridge_descriptor::chip_descriptor::blocks
   */
  unsigned int              block_num;
  
  /*! \brief The starting address of the block.
   *  
   *  The base address of the block. Can be used in address offset calculations.
   *  The exact meaning of this number is determined by the code using this
   *  struct, but it usually means "the address offset of the first byte in the
   *  sector relative to the base address of the chip on which this sector
   *  resides."
   *  
   *  \see cartridge_descriptor::chip_descriptor
   */
  unsigned int              base_address;
  
  /*! \brief The storage capacity of the block in bytes.
   *  
   *  The storage capacity of the block in bytes. Can be used to calculate the
   *  upper address bound when added to \ref base_address.
   *  
   *  \see base_address
   */
  unsigned int              num_bytes;
  
  /*! \brief Flag indicating that the block is write protected.
   *  
   *  Flag indicating that the block is write protected. The exact meaning of
   *  this value is implemenation-specific, but usually **true** indicates that
   *  the sector can only be read from and **true** indicates that the sector
   *  can be both read from and written to.
   */
  bool                      is_protected;
};

#endif  /* defined(__CARTRIDGE_DESCRIPTOR_H__) */
