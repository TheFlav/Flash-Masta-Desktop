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

Classes extending Qt objects are named using camel casing with now characters
separating the words. The first letter of each distinct word in the class's name
is capitalized and all other letters in the word are lowercase.

    class GladosBot;

## Directory Structure

*Note: Paths are relative to the project root directory.*

**/build** -
Dedicated build directory, used to store object files and to be used as a work
space by IDEs. This directory and its contents should never be tracked by source
control.

**/src** -
Contains all source code for the project. No IDE project files. All files in
this directory should be tracked by source control, even if not all files are
used in all builds of the project.

**/src/cartridge** -
Contains files related to cartridges and cartridge information. Classes in this
directory contain functionality for manipulating game cartridges and
communicating with the chips on these cartridges.

**/src/common** -
Contains common files that are found throughout the program. These files
contain global type defines and preprocessor macros that are class-agnostic.

**/src/hardware** -
Contains the original LinkMasta code written by Ed Mandy. The files contained
in this directory build messages to communicate with the firmware on the
LinkMasta and performing *libusb* library function calls to send and receive
data via USB.

**/src/test** -
Contains files and classes for unit testing.

**/src/hardware/AVR** -
Contains some header files used by the original LinkMasta. These files actually
contain a lot of source code despite their header status, these functions
existing in the global scope. They should not be included or used except by the
original LinkMasta code.

**/src/hardware/PC-App-CLI** -
Contains the original LinkMasta code and corresponding header. The code in these
files serves multiple functions; everything from command line argument handling
to interacting with the LinkMasta firmware to making *libusb* library calls to
communicating with the hardware via USB.

**/src/linkmasta_device** -
Contains classes responsible for handling communications with various types of
linkmasta devices. These classes are not supposed to concern themselves with the
details of usb communication, although they often use usb libraries and classes
to interface with linkmasta devices.

**/src/tasks** -
Classes that aid in communicating the progress and status of long-running tasks.

**/src/test** -
Contains code that will help automatically test other systems. Also known as
unit testing.

**/src/ui** –
Contains code and template files necessary for user interfaces. This includes
graphical user interfaces and command line interfaces.

**/src/ui/cl** -
Contains code related to a command line-based interface.

**/src/ui/qt** -
Contains files related to the Qt graphical user interface.

**/src/ui/qt/task** -
Contains classes designed to perform a task over a long period of time.
Contains feedback functionality so that users can know the progress of the
current task and can cancel the task at any time.

**/src/usb** -
Contains files for classes that are used as "handles" for USB devices. These
classes contain functionality for opening a connection and transmitting data
to and from USB devices.

**/src/usb/exception** -
Exception classes thrown by usb objects when an error occurs.
