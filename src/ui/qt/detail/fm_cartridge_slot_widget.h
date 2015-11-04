#ifndef FM_CARTRIDGE_SLOT_WIDGET_H
#define FM_CARTRIDGE_SLOT_WIDGET_H

#include <QWidget>
#include <QString>

namespace Ui {
class FmCartridgeSlotWidget;
}

class cartridge;
class ngp_cartridge;
class ws_cartridge;

class FmCartridgeSlotWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit FmCartridgeSlotWidget(int device_id, cartridge* cart = 0, int slot = -1, QWidget *parent = 0);
  ~FmCartridgeSlotWidget();
  
  void buildFromCartridge(cartridge* cart, int slot);
private:
  void buildFromNgpCartridge(ngp_cartridge* cart, int slot);
  void buildFromWsCartridge(ws_cartridge* cart, int slot);
  
public:
  int slotNumber() const;
  unsigned int slotSize() const;
  QString slotGameName() const;
  bool gameBackupEnabled() const;
  bool gameFlashEnabled() const;
  bool gameVerifyEnabled() const;
  bool saveBackupEnabled() const;
  bool saveRestoreEnabled() const;
  bool saveVerifyEnabled() const;
  
private:
  QString stringifyBytesToBits(unsigned int num_bytes, bool reduce = true);
  
public slots:
  void setSlotSize(unsigned int num_bytes);
  void setSlotGameName(QString name);
  void setGameBackupEnabled(bool enabled);
  void setGameFlashEnabled(bool enabled);
  void setGameVerifyEnabled(bool enabled);
  void setSaveBackupEnabled(bool enabled);
  void setSaveRestoreEnabled(bool enabled);
  void setSaveVerifyEnabled(bool enabled);
  
private:
  Ui::FmCartridgeSlotWidget *ui;
  
  int m_slot;
  int m_device_id;
  unsigned int m_slot_num_bytes;
  QString m_slot_game_name;
  
  bool m_game_backup_enabled;
  bool m_game_flash_enabled;
  bool m_game_verify_enabled;
  bool m_save_backup_enabled;
  bool m_save_restore_enabled;
  bool m_save_verify_enabled;
};

#endif // FM_CARTRIDGE_SLOT_WIDGET_H