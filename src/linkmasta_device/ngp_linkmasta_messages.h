//
//  ngp_linkmasta_messages.h
//  FlashMasta
//
//  Created by Dan on 7/24/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __NGP_LINKMASTA_MESSAGES_H__
#define __NGP_LINKMASTA_MESSAGES_H__

#include <stdint.h>

#define MSG_WRITE64xN_REPLY         0x08
#define MSG_RESULT_FAIL             0x10
#define MSG_RESULT_SUCCESS          0x11

void build_blink_led_command(uint8_t *buf, uint8_t blinkCount);
void build_getversion_command(uint8_t *buf);
void build_getversion_reply(uint8_t *buf, uint8_t majVer, uint8_t minVer);
void get_getversion_reply(uint8_t *buf, uint8_t *majVer, uint8_t *minVer);
void build_write_command(uint8_t *buf, uint32_t addr_host, uint8_t data, uint8_t chip);
void build_flash_write_N_command(uint8_t *buf, uint32_t addr_host, const uint8_t *data, uint8_t chip, uint8_t n, uint8_t uBypassMode);
void build_flash_write_32_command(uint8_t *buf, uint32_t addr_host, const uint8_t *data, uint8_t chip, uint8_t uBypassMode);
void build_flash_write64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t chip, uint8_t n, uint8_t uBypassMode);
void build_flash_write64xN_data_packet(uint8_t *buf, const uint8_t *data);
void build_read_command(uint8_t *buf, uint32_t addr_host, uint8_t chip);
void build_read64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t chip, uint8_t n);
void build_SPI_send_recv_command(uint8_t *buf, uint8_t data);
void build_read64_command(uint8_t *buf, uint32_t addr, uint8_t chip);
void build_read_reply(uint8_t *buf, uint8_t addrHB, uint8_t addrMB, uint8_t addrLB, uint8_t data, uint8_t chip);
void build_flash_write64xN_reply(uint8_t *buf, uint8_t packetsProcessed);

void build_reply_success(uint8_t *buf);
void build_reply_fail(uint8_t *buf);

uint8_t *get_flash_write_32_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *uBypassMode);
uint8_t *get_flash_write_N_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *n, uint8_t *uBypassMode);
void get_flash_write64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *n, uint8_t *uBypassMode);
void get_read64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *n);
void get_read_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip);
void get_write_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *data, uint8_t *chip);

void get_SPI_send_recv_message(uint8_t *buf, uint8_t *data);
int get_read_reply(uint8_t *buf, uint32_t *addr21, uint8_t *data);
void get_flash_write64xN_reply(uint8_t *buf, uint8_t *msgType, uint8_t *packetsProcessed);
void get_result_reply(uint8_t *buf, uint8_t *result);
void get_blink_led_message(uint8_t *buf, uint8_t *blinkCount);

#endif /* defined(__NGP_LINKMASTA_MESSAGES_H__) */
