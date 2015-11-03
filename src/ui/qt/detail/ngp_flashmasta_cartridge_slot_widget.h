#ifndef NGPFLASHMASTACARTRIDGESLOTWIDGET_H
#define NGPFLASHMASTACARTRIDGESLOTWIDGET_H

#include <QWidget>

namespace Ui {
class NgpFlashMastaCartridgeSlotWidget;
}

class ngp_cartridge;

class NgpFlashMastaCartridgeSlotWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpFlashMastaCartridgeSlotWidget(unsigned int device_id, ngp_cartridge* cartridge, unsigned int slot, QWidget *parent = 0);
  ~NgpFlashMastaCartridgeSlotWidget();
  
private:
  Ui::NgpFlashMastaCartridgeSlotWidget *ui;
  unsigned int m_device_id;
  ngp_cartridge* m_cartridge;
  unsigned int m_slot;
};

#endif // NGPFLASHMASTACARTRIDGESLOTWIDGET_H
