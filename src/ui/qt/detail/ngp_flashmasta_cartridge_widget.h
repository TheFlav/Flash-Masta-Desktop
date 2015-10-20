#ifndef NGP_FLASHMASTA_CARTRIDGE_WIDGET_H
#define NGP_FLASHMASTA_CARTRIDGE_WIDGET_H

#include <QWidget>

#include "../flash_masta.h"
#include <vector>

namespace Ui {
class NgpFlashmastaCartridgeWidget;
}

class ngp_cartridge;
class NgpLmOfficialCartridgeInfoWorker;

class NgpFlashmastaCartridgeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent = 0);
  ~NgpFlashmastaCartridgeWidget();
  void refresh_ui();
  
public slots:
  void cartridge_loaded(ngp_cartridge* cartridge);
  
private:
  Ui::NgpFlashmastaCartridgeWidget *ui;
  unsigned int m_device_id;
  NgpLmOfficialCartridgeInfoWorker* m_worker;
  ngp_cartridge* m_cartridge;
  std::vector<QWidget*> m_slot_widgets;
};

#endif // NGP_FLASHMASTA_CARTRIDGE_WIDGET_H
