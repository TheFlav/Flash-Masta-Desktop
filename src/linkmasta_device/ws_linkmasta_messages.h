//
//  ws_linkmasta_messages.h
//  FlashMasta
//
//  Created by Dan on 8/14/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __WS_LINKMASTA_MESSAGES_H__
#define __WS_LINKMASTA_MESSAGES_H__

#include <stdint.h>

#define MSG_WRITE64xN_REPLY         0x08
#define MSG_RESULT_FAIL             0x10
#define MSG_RESULT_SUCCESS          0x11

namespace wsmsg
{

enum target_enum
{
  TARGET_ROM = 0,             //this is a 0-based word address into ROM
  TARGET_SRAM = 1,            //this is a 0-based byte address into SRAM
  TARGET_PORTIO = 2,          //this is a PORT address
  TARGET_CART = 3             //this is a raw address that the MBC will map to SRAM/ROM/etc. using its bank register(s)
  //TARGET_MICROSD someday?
};


void build_blink_led_command(uint8_t *buf, uint8_t blinkCount);
void build_getversion_command(uint8_t *buf);
void build_set_unset_lines_command(uint8_t *buf, uint8_t setBits, uint8_t unsetBits);
void get_set_unset_lines_command(uint8_t *buf, uint8_t *setBits, uint8_t *unsetBits);
void build_getversion_reply(uint8_t *buf, uint8_t majVer, uint8_t minVer);
void get_getversion_reply(uint8_t *buf, uint8_t *majVer, uint8_t *minVer);

void build_write8_command(uint8_t *buf, uint32_t addr_host, uint8_t data, uint8_t target);
void build_write8_SRAM_command(uint8_t *buf, uint32_t addr_host, uint8_t data);
void build_write16_command(uint8_t *buf, uint32_t addr_host, uint16_t data, uint8_t target);
void build_flash_write_N_command(uint8_t *buf, uint32_t addr_host, const uint8_t *data, uint8_t n);
void build_flash_write_32_command(uint8_t *buf, uint32_t addr_host, const uint8_t *data);

void build_flash_write64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t n);
void build_flash_write64xN_data_packet(uint8_t *buf, const uint8_t *data);
void build_sram_write64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t n);
void build_sram_write64xN_data_packet(uint8_t *buf, const uint8_t *data);
void build_read8_command(uint8_t *buf, uint32_t addr_host, uint8_t target);
void build_read8_SRAM_command(uint8_t *buf, uint32_t addr_host);
void build_read16_command(uint8_t *buf, uint32_t addr_host, uint8_t target);
void build_read64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t n, uint8_t target_chip);
void build_read64_command(uint8_t *buf, uint32_t addr, uint8_t target_chip);
void build_eeprom_write_N_command(uint8_t *buf, uint32_t addr_host, uint8_t *data, uint8_t n);

void build_read8_reply(uint8_t *buf, uint8_t addrHB, uint8_t addrMB, uint8_t addrLB, uint8_t addrNO, uint8_t data);
void build_read16_reply(uint8_t *buf, uint8_t addrHB, uint8_t addrMB, uint8_t addrLB, uint8_t dataHigh, uint8_t dataLow);
void build_write64xN_reply(uint8_t *buf, uint8_t packetsProcessed);
void build_reply_success(uint8_t *buf);
void build_reply_fail(uint8_t *buf);
uint8_t *get_flash_write_32_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB);
uint8_t *get_flash_write_N_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *n);
void get_flash_write64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *n);
void get_sram_write64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *n);
void get_read64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *n, uint8_t *targetChip);
uint8_t *get_eeprom_write_N_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *n);

void build_set_addr_command(uint8_t *buf, uint32_t addr_host);
void get_set_addr_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO);
void build_set_data16_command(uint8_t *buf, uint16_t data);
void get_set_data16_message(uint8_t *buf, uint8_t *dataHB, uint8_t *dataLB);

void get_read_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *targetChip);
void get_write8_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *data8, uint8_t *targetChip);
void get_write16_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *dataHB, uint8_t *dataLB, uint8_t *targetChip);

int get_read8_reply(uint8_t *buf, uint32_t *addr32, uint8_t *data);
int get_read16_reply(uint8_t *buf, uint32_t *addr32, uint16_t *data);
void get_write64xN_reply(uint8_t *buf, uint8_t *msgType, uint8_t *packetsProcessed);
void get_result_reply(uint8_t *buf, uint8_t *result);
void get_blink_led_message(uint8_t *buf, uint8_t *blinkCount);

};

#endif /* defined(__WS_LINKMASTA_MESSAGES_H__) */
