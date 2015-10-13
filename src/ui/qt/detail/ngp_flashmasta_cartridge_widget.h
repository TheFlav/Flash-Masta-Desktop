#ifndef NGP_FLASHMASTA_CARTRIDGE_WIDGET_H
#define NGP_FLASHMASTA_CARTRIDGE_WIDGET_H

#include <QWidget>

#include "../flash_masta.h"

namespace Ui {
class NgpFlashmastaCartridgeWidget;
}

class ngp_cartridge;

class NgpFlashmastaCartridgeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent = 0);
  ~NgpFlashmastaCartridgeWidget();
  
private:
  Ui::NgpFlashmastaCartridgeWidget *ui;
  unsigned int m_device_id;
  ngp_cartridge* m_cartridge;
};

#endif // NGP_FLASHMASTA_CARTRIDGE_WIDGET_H
