#include "main_window.h"
#include "flash_masta.h"

int main(int argc, char *argv[])
{
  FlashMasta a(argc, argv);
  MainWindow w;
  w.show();
  
  return a.exec();
}
