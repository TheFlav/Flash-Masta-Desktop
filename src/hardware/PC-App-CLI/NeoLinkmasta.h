//
//  NeoLinkmasta.h
//  FlashMasta
//
//  Created by Dan on 7/16/15.
//  Copyright (c) 2015 7400 Circuits. All rights reserved.
//

#ifndef __NEOLINKMASTA_H__
#define __NEOLINKMASTA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

unsigned char getBootBlockStart();


uint32_t blockNumToAddr(unsigned char chip, unsigned char blockNum);

uint32_t blockSize(unsigned char blockNum);

void build_linkmasta_led(uint8_t blinkCount);

void check_firmware_version(uint8_t *majVer, uint8_t *minVer);

void flash_write_32_bytes(uint32_t addr, uint8_t *data, uint8_t chip);

void flash_write_N_bytes(uint32_t addr, uint8_t *data, uint8_t chip, uint8_t n);

void flash_write_64xN_bytes(uint32_t addr, uint8_t *data, uint8_t chip, uint8_t numPackets);


void write_byte(uint32_t addr, uint8_t data, uint8_t chip);

uint8_t read_byte(uint32_t addr, uint8_t chip);

uint8_t *read_64_bytes(uint32_t addr, uint8_t chip);

uint8_t read_64xN_bytes(uint8_t *readBuf, uint32_t addr, uint8_t chip, uint8_t n);

int write_file_to_savemasta(char *filename);

int backup_savemasta_to_file(char *bakFile, uint32_t size);


void printUsage(char *thisFile);

uint8_t parseCmdLine(int argc, char *argv[]);

void erase_cart();

int write_file_to_cart(char *filename);



int verify_file_with_cart(char *filename);


int backup_cart_to_file(char *bakFile, uint32_t size);




//write all the unprotected blocks out to a file
void backupCartSaveGame();


void restoreSaveGameToCart(char *ngfFilename);






int Dump32MbitCart();



/**************************
 Start Micro SD functions
 **************************/
uint8_t SD_SPI_send_byte(uint8_t sendByte);

void printBuf(uint8_t *buf, uint8_t len);

void SD_SPI_transmit(uint8_t *sendBuffer, uint8_t *recvBuffer, uint8_t len);

uint8_t SD_SPI_wait_for_response(uint8_t response);

void enter_SD_SPI_mode(void);

void exit_SD_SPI_mode(void);


void queryMicroSD(void);
/**************************
 End Micro SD functions
 **************************/

int _main(int argc, char *argv[]);

  
#ifdef __cplusplus
}
#endif

#endif // __NEOLINKMASTA_H__
