#include "flash_masta_app.h"
#include "common/log.h"

int main(int argc, char *argv[])
{
  log_init();
  log_start("program start...");
  FlashMastaApp* app = new FlashMastaApp(argc, argv);
  int r = app->exec();
  delete app;
  log_end("program end");
  log_deinit();
  return r;
}
