#ifndef FLASHMASTA_H
#define FLASHMASTA_H

#include <QApplication>

class FlashMasta: public QApplication
{
public:
  FlashMasta(int argc, char** argv, int flags = ApplicationFlags);
  ~FlashMasta();
  
  static FlashMasta* get_instance();
  
private:
  Q_DISABLE_COPY(FlashMasta)
  
  static FlashMasta* instance;
};

#endif // FLASHMASTA_H
