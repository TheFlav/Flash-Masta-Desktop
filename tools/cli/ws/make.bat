@rem # Windoze
gcc -o WSLinkmasta.exe WSLinkmasta.c -lusb-1.0 -lwsock32 -DOS_WINDOWS

@rem # OSX.. assumes you got libusb headers somewhere.. and a libusb-1.0.a compiled
@rem # for i386 and x86_64 in a local directory
@rem #gcc -DOS_MACOSX -I/usr/local/inclide -framework IOKit -framework CoreFoundation -arch i386 -arch x86_64 -lusb-1.0 NeoLinkmasta.c -o NeoLinkmasta


