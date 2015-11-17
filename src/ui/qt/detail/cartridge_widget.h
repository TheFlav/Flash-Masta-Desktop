#ifndef __CARTRIDGE_WIDGET_H__
#define __CARTRIDGE_WIDGET_H__

#include <QWidget>

#include <vector>
#include <string>

#include "../flash_masta_app.h"

namespace Ui {
class CartridgeWidget;
}

class cartridge;
class LmCartridgeFetchingWorker;
class QLayoutItem;

class CartridgeWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit CartridgeWidget(unsigned int device_id, QWidget *parent = 0);
  ~CartridgeWidget();
  
  void refreshUi();
  void setCartridgeName(std::string label);
  void setCartridgeNameVisible(bool visible);
  
private:
  void setSlotsComboBoxVisible(bool visible);
  
public slots:
  void cartridgeLoaded(cartridge* cartridge, std::string cartridge_game_name);
  void deviceSelected(int old_device_id, int new_device_id);
  void slotSelected(int old_slot_id, int new_slot_id);
  void updateEnabledActions();
  
private slots:
  void on_slotsComboBox_currentIndexChanged(int index);
  
private:
  Ui::CartridgeWidget *ui;
  QWidget* m_default_widget;
  QWidget* m_current_widget;
  int m_current_slot;
  bool m_is_selected;
  
  unsigned int m_device_id;
  LmCartridgeFetchingWorker* m_worker;
  cartridge* m_cartridge;
  std::string m_cartridge_game_name;
  std::vector<QWidget*> m_slot_widgets;
  
  QLayoutItem* m_slotsComboBoxHorizontalLayout;
};

#endif // __NGP_FLASHMASTA_CARTRIDGE_WIDGET_H__
