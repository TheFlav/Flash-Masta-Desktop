# <a name="top"></a>FlashMasta

## Contents

1. [Project Directory Structure](#pds)
2. [Coding Conventions](#coding-conventions)
  1. [Naming](#naming)
  2. [Indentation](#indentation)
3. [Building](#building)

--------------------------------------------------------------------------------

## <a name="project-directory-structure"></a>[1. Project Directory Structure](#top)

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

**/src/games** -
Contains classes related to game identification, such as taking a connected
cartridge and determining the games installed on it, typically referencing
a SQLite database.

**/src/linkmasta** -
Contains classes responsible for handling communications with various types of
linkmasta devices. These classes are not supposed to concern themselves with the
details of usb communication, although they often use usb libraries and classes
to interface with linkmasta devices.

**/src/sqlite** -
Contains SQLite 3 libraries, source code, and headers.

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
Contains files related to the Qt graphical user interface component of this
project.

**/src/ui/qt/detail** -
Contains classes for displaying detailed LinkMasta, FlashMasta, and game
information in the Qt GUI parts of this project.

**/src/ui/qt/task** -
Contains classes designed to perform a task over a long period of time.
Contains feedback functionality so that users can know the progress of the
current task and can cancel the task at any time.

**/src/ui/qt/worker** -
Contains classes designed to perform asynchronous tasks that usually involve
communications with connected FlashMastas and LinkMastas.

**/src/usb** -
Contains files for classes that are used as "handles" for USB devices. These
classes contain functionality for opening a connection and transmitting data
to and from USB devices.

**/src/usb/exception** -
Exception classes thrown by usb objects when an error occurs.

--------------------------------------------------------------------------------

## <a name="coding-conventions"></a>[2. Coding Conventions](#top)

Coding conventions are important to the maintainability and readability of a
programming project. Outlining and agreeing upon clear guidelines on naming
and style conventions will make development run smoother and can help decrease
developer frustration.

As a rule when developing for a shared project, it is the developer's
responsibility to understand and follow the project's conventions. The best way
of doing so is to review existing code examples in the project and follow
whatever patterns they appear to be following.

For those cases where such examples cannot be found or simply do not exist, this
document may be used as a difinitive guide style guide on the project. However,
because conventions may change and because no programming project is ever truly
dead, this document may be updated to reflect the current coding standards of
the project.

### <a name="naming"></a>[Naming](#top)

#### Variables

Variables are all lower-cased with individual words separated by underscores
(`_`).

    int    i_am_cake;
    double test_subjects[2000];

Member variables of classes should begin with the `m_` prefix in order to
distinguish between member variables and member functions that are closely
related and similarly named.

    class my_class
    {
    public:
      int size();
      
    private:
      int m_size;
    };

#### Functions

Functions are all lower-cased with individual words separated by underscores
(`_`).

    void this_was_a_triumph();
    int  im_making_a_note_here(int hugeSuccess);

Function names need to reflect their purpose and use and should follow the
conventions of the environment they are defined in. In this case, function names
should follow the patterns of the standard library such as using `size()` to
indicate lengths/counts/sizes and `set_<property>` to set a property value.

    unsigned int size();
    void         set_x(int x);

#### Classes and Structs

Because C++ structs are functionally identical to classes with one minor
exception (a default access level of `public` instead of `private`), structs
and classes both follow the same naming conventions.

Classes and structs are all lower-cased with individual words spearated by
underscores (`_`).

    class glados_bot
    {
      /* class definition */
    };

There is one exception to the above rule; classes extending Qt objects are named
using camel casing with no characters separating the words. The first letter of
each distinct word in the class's name is capitalized and all other letters in
the word are lowercase. *Structs will never follow this exception.*

    class GladosBot extends QObject
    {
      /* class definition */
      
      struct glados_bot_struct
      {
        /* struct definition */
      };
    };

### <a name="indentation"></a>[Indentation](#top)

Indentation is important for clarity in all programs. Although having
non-uniform indentation is less consequential than having non-uniform naming
conventions, there is value in having consistency in coding style across an
entire project.

#### Spaces

This project uses two (2) spaces per indentation level. Whether using some other
indentation scheme is preferable is irrelevant, as two spaces is what this
project currently uses for indentation. Should a future developer decide that
another indentation scheme is preferable, then they will need to update all
project files with the new scheme and modify this document to match.

    if (true)
    {
      // check out my indentation
    }

#### Indent Braces

As a rule-of-thumb, indent all code that is contained within a pair of braces
(`{}`, often mislabeled as "curly braces" or "brackets"). This includes the body
of `if`, `else`, `while`, `for`, `do while`, `switch`, and `try catch`
statements, as well as functions, classes, and structs. A nested pair of braces
should increase the indentation level by one from the level in which they are
contained.

    class my_class
    {
      void my_function(bool test)
      {
        if (test)
        {
          // do something
        }
        else
        {
          // do something else
        }
      }
    };

#### Namespaces

Namespaces are are an exception to the indentation rule-of-thumb; do not indent
the body of a `namespace` statement. It is arguably tedious and ultimately
unnecessary.

#### Precompile Directives

Precompile directives should never be indented. Although this may be visually
unappealing, some compilers will not recognize or correcly parse precompile
directives not beginning on column zero.

    if (true)
    {
      if (true)
      {
    #define YES true
        if (YES)
        {
          // you get the idea
        }
      }
    }

#### Statement Labels

Labels are used by `switch` statements and `goto` statements. Regarding the use
of labels with `goto`s, don't. Just don't.

**Switch labels** should be at the same indentation level as the `switch`
statement to which they belong. That is, they will be *less* indented than the
rest of the statements within the `switch` block.

    switch (n)
    {
    case 0:
      // do something
      break;
      
    default:
      // do something
      break;
    }

**General labels** used for `goto` statements, if used (they shouldn't be),
should be placed at the same indentation level as the function to which they
belong. That means their indentation level should match that of the nearest
function header above them.

    void my_function()
    {
    my_start_label:
      // do things
      goto my_start_label;
    }
    
    class my_class()
    {
      void my_member_function()
      {
      my_start_label:
        // do things
        goto my_start_label;
      }
    }
## <a name="building"></a>[Building](#top)
**Manjaro Linux**

git clone https://github.com/TheFlav/Flash-Masta-Desktop.git

cd Flash-Masta-Desktop

mkdir build

cd build

qmake ../FlashMasta.pro

make

cd ../tools

make

./build-database

mv *.db ../build

cd ../build

./FlashMasta

**Ubuntu Linux**
sudo apt install git qtchooser qt5-default qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools build-essential libqt5svg5* libusb-1.0-0-dev

git clone https://github.com/TheFlav/Flash-Masta-Desktop.git

cd Flash-Masta-Desktop

mkdir build

cd build

qmake ../FlashMasta.pro

make

cd ../tools

make

./build-database

mv *.db ../build

cd ../build

sudo ./FlashMasta
