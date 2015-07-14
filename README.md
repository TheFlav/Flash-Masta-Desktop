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

**/src/cartridges** - tracked  
Contains files related to cartridges, cartridge information, and cartridge
reading and writing.

**/src/ui** – tracked   
Contains code and tempalte files necessary for user interfaces. This includes
graphical user interfaces and command line interfaces.

**/src/ui/qt** - tracked  
Contains files related to the Qt graphical user interface.
