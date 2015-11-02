#ifndef NGP_FM_CARTRIDGE_INFO_WIDGET_H
#define NGP_FM_CARTRIDGE_INFO_WIDGET_H

#include <QWidget>

namespace Ui {
class NgpFmCartridgeInfoWidget;
}

class ngp_cartridge;

class NgpFmCartridgeInfoWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit NgpFmCartridgeInfoWidget(int device_id, ngp_cartridge* cartridge = 0, QWidget *parent = 0);
  ~NgpFmCartridgeInfoWidget();
  
  void buildFromCartridge(ngp_cartridge* cartridge);
  
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
  Ui::NgpFmCartridgeInfoWidget *ui;
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

#endif // NGP_FM_CARTRIDGE_INFO_WIDGET_H

