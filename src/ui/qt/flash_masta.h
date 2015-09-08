#ifndef FLASHMASTA_H
#define FLASHMASTA_H

#include <QApplication>

class DeviceManager;



class FlashMasta: public QApplication
{
public:
  FlashMasta(int argc, char** argv, int flags = ApplicationFlags);
  ~FlashMasta();
  
  static FlashMasta* get_instance();
  
  DeviceManager* get_device_manager() const;
  
private:
  Q_DISABLE_COPY(FlashMasta)
  
  static FlashMasta* instance;
  
  DeviceManager* m_device_manager;
};

#endif // FLASHMASTA_H
