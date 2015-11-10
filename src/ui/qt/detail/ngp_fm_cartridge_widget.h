#ifndef __NGP_FLASHMASTA_CARTRIDGE_WIDGET_H__
#define __NGP_FLASHMASTA_CARTRIDGE_WIDGET_H__

#include <QWidget>

#include "../flash_masta_app.h"
#include <vector>

namespace Ui {
class NgpFlashmastaCartridgeWidget;
}

class ngp_cartridge;
class NgpLmCartridgeFetchingWorker;

class NgpFlashmastaCartridgeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpFlashmastaCartridgeWidget(unsigned int device_id, QWidget *parent = 0);
  ~NgpFlashmastaCartridgeWidget();
  
  void refreshUi();
  
public slots:
  void cartridgeLoaded(ngp_cartridge* cartridge);
  void deviceSelected(int old_device_id, int new_device_id);
  void slotSelected(int old_slot_id, int new_slot_id);
  void updateEnabledActions();
  
private slots:
  void on_slotsComboBox_currentIndexChanged(int index);
  
private:
  Ui::NgpFlashmastaCartridgeWidget *ui;
  QWidget* m_default_widget;
  QWidget* m_current_widget;
  int m_current_slot;
  bool m_is_selected;
  
  unsigned int m_device_id;
  NgpLmCartridgeFetchingWorker* m_worker;
  ngp_cartridge* m_cartridge;
  std::vector<QWidget*> m_slot_widgets;
};

#endif // __NGP_FLASHMASTA_CARTRIDGE_WIDGET_H__
