#ifndef NGP_FLASHMASTA_CARTRIDGE_WIDGET_H
#define NGP_FLASHMASTA_CARTRIDGE_WIDGET_H

#include <QWidget>

#include "../flash_masta.h"
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
  void refresh_ui();
  
public slots:
  void cartridge_loaded(ngp_cartridge* cartridge);
  void when_gameBackupTriggered();
  void when_gameFlashTriggered();
  void when_gameVerifyTriggered();
  void when_saveBackupTriggered();
  void when_saveRestoreTriggered();
  void when_saveVerifyTriggered();
  
private slots:
  void on_slotsComboBox_currentIndexChanged(int index);
  
signals:
  void gameBackupTriggered();
  void gameFlashTriggered();
  void gameVerifyTriggered();
  void saveBackupTriggered();
  void saveRestoreTriggered();
  void saveVerifyTriggered();
  void slotSelected(int slot);
  void slotDeselected(int slot);
  
private:
  Ui::NgpFlashmastaCartridgeWidget *ui;
  QWidget* m_default_widget;
  QWidget* m_current_widget;
  
  unsigned int m_device_id;
  NgpLmCartridgeFetchingWorker* m_worker;
  ngp_cartridge* m_cartridge;
  std::vector<QWidget*> m_slot_widgets;
};

#endif // NGP_FLASHMASTA_CARTRIDGE_WIDGET_H
