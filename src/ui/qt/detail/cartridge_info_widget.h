#ifndef __CARTRIDGE_INFO_WIDGET_H__
#define __CARTRIDGE_INFO_WIDGET_H__

#include <QWidget>

namespace Ui {
class CartridgeInfoWidget;
}

class cartridge;
class QLayoutItem;

class CartridgeInfoWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit CartridgeInfoWidget(int device_id, cartridge* cart = 0, QWidget *parent = 0);
  ~CartridgeInfoWidget();
  
  void buildFromCartridge(cartridge* cart);
  
  unsigned int cartridgeSize() const;
  unsigned int cartridgeNumSlots() const;
  unsigned int cartridgeNumChips() const;
  unsigned int cartridgeChipSize(unsigned int chip_index) const;
  bool gameBackupEnabled() const;
  bool gameFlashEnabled() const;
  bool gameVerifyEnabled() const;
  bool saveBackupEnabled() const;
  bool saveRestoreEnabled() const;
  bool saveVerifyEnabled() const;
  
private:
  void setPromptLabelVisible(bool visible);
  void setCartridgeSpecNumSlotsVisible(bool visible);
  void clearChipData();
  QString stringifyBytesToBits(unsigned int num_bytes, bool reduce = true);
  
public slots:
  void setCartridgeSize(unsigned int num_bytes);
  void setCartridgeNumSlots(unsigned int num_slots);
  void setCartridgeNumChips(unsigned int num_chips);
  void setCartridgeChipSize(unsigned int chip_index, unsigned int num_bytes);
  void setGameBackupEnabled(bool enabled);
  void setGameFlashEnabled(bool enabled);
  void setGameVerifyEnabled(bool enabled);
  void setSaveBackupEnabled(bool enabled);
  void setSaveRestoreEnabled(bool enabled);
  void setSaveVerifyEnabled(bool enabled);
  
private slots:
  void onDeviceSelected(int old_device_id, int new_device_id);
  
private:
  Ui::CartridgeInfoWidget *ui;
  QWidget*** m_cart_chip_widgets;
  int m_device_id;
  
  unsigned int m_cart_num_bytes;
  unsigned int m_cart_num_slots;
  unsigned int m_cart_num_chips;
  unsigned int* m_cart_chip_sizes;
  bool m_game_backup_enabled;
  bool m_game_flash_enabled;
  bool m_game_verify_enabled;
  bool m_save_backup_enabled;
  bool m_save_restore_enabled;
  bool m_save_verify_enabled;
};

#endif // __CARTRIDGE_INFO_WIDGET_H__
