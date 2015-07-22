# FlashMasta

--------------------------------------------------------------------------------

## Conventions

### Naming

#### Variables

Variables are camel-cased with the first letter lower-cased.

    int    iAmCake;
    double testSubjects[2000];

#### Functions

Functions are all lower-cased with individual words separated by underscores
(`_`).

    void this_was_a_triumph();
    int  im_making_a_note_here(int hugeSuccess);

Function names need to reflect their purpose and use and should follow the
conventions of the environment they are defined in. In this case, function names
should follow the patterns of the standard library such as using `size()` to
indicate lengths/counts/sizes and `set_<property>` so set a property value.

    unsigned int size();
    void         set_x(int x);

#### Classes

Classes are all lower-cased with individual words spearated by underscores
(`_`).

    class glados_bot;

## Directory Structure

*Note: Paths are relative to the project root directory.*

**/build** – untracked  
Dedicated build directory, used to store object files and to be used as a work
space by IDEs. This directory and its contents are not tracked by source
control.

**/src** – tracked  
Contains all source code for the project. No IDE project files. All files in
this directory should be relevant to source control.

**/src/cartridge** - tracked  
Contains files related to cartridges and cartridge information.

**/src/cartridge_io** - tracked
Contains file related to cartridge reading and writing. Code in here should
primarily contain gaming system and cartridge-specific functionality.

**/src/common** - tracked
Contains common files that are found throughout the program. These files
contain global type defines and preprocessor macros that are class-agnostic.

**/src/hardware** - tracked
Contains the original LinkMasta code written by Ed Mandy. The files contained
in this directory build messages to communicate with the firmware on the
LinkMasta and performing *libusb* library function calls to send and receive
data via USB.

**/src/hardware/AVR** - tracked
Contains some header files used by the original LinkMasta. These files actually
contain a lot of source code despite their header status, these functions
existing in the global scope. They should not be included or used except by the
original LinkMasta code.

**/src/hardware/PC-App-CLI** - tracked
Contains the original LinkMasta code and corresponding header. The code in these
files serves multiple functions; everything from command line argument handling
to interacting with the LinkMasta firmware to making *libusb* library calls to
communicating with the hardware via USB.

**/src/ui** – tracked   
Contains code and template files necessary for user interfaces. This includes
graphical user interfaces and command line interfaces.

**/src/ui/cl** - tracked
Contains code related to a command line-based interface.

**/src/ui/qt** - tracked  
Contains files related to the Qt graphical user interface.

**/src/usb_communicator** - tracked
Contains files for classes that are intermediaries between the USB libraries
used and the rest of the program. These classes contain functionality for
monitering USB ports, getting handles for connected USB devices, and listing
devices connected to the computer.

**/src/usb_device** - tracked
Contains files for classes that are used as "handles" for USB devices. These
classes contain functionality for opening a connection and transmitting data
to and from USB devices.
