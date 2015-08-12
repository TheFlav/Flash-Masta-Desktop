#include "flash_masta.h"

FlashMasta* FlashMasta::instance = nullptr;



FlashMasta::FlashMasta(int argc, char **argv, int flags)
  : QApplication(argc, argv, flags)
{
  if (FlashMasta::instance == nullptr)
  {
    FlashMasta::instance = this;
  }
}

FlashMasta::~FlashMasta()
{
  // Nothing else to do
}



FlashMasta* FlashMasta::get_instance()
{
  return FlashMasta::instance;
}
