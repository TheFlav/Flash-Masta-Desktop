#include "ws_linkmasta_messages.h"
#include <stdio.h>

#if defined(OS_WINDOWS)
#include <winsock2.h>
#elif ARCH==ARCH_AVR8
#elif defined(OS_LINUX) || defined(OS_MACOSX)
#else
#error Undefined hardware/software platform
#endif

#define MSG_GETVERSION              0x00
#define MSG_READ8_CMD               0x01
#define MSG_WRITE8_CMD              0x02
#define MSG_READ_REPLY              0x03
#define MSG_READ64xN_CMD            0x04
#define MSG_FLASHWRITE32_CMD        0x05
#define MSG_FLASHWRITE_N_CMD        0x06
#define MSG_FLASHWRITE64xN_CMD      0x07
#define MSG_WRITE64xN_REPLY         0x08
#define MSG_BLINK_LED               0x09
#define MSG_SPI_SEND_RECV_CMD       0x0A
#define MSG_SET_UNSET_LINES_CMD     0x0B
#define MSG_SET_ADDR_LINES_CMD      0x0C
#define MSG_SET_DATA_LINES_CMD      0x0D
#define MSG_WRITE16_CMD             0x0E
#define MSG_READ16_CMD              0x0F
#define MSG_SRAMWRITE64xN_CMD       0x10
#define MSG_GET_CARTINFO_CMD        0x11
#define MSG_SET_CARTSLOT_CMD        0x12

#define MSG_EEPROMWRITE_N_CMD       0x80
#define MSG_EEPROMREAD_N_CMD        0x81

#define MSG_RESULT_FAIL             0x10
#define MSG_RESULT_SUCCESS          0x11

#define MSG_TYPE_OFFSET             0

#define MSG_MAJORVER_OFFSET         1
#define MSG_MINORVER_OFFSET         2

#define MSG_SLOTS_FIXEDNUM_OFFSET   1
#define MSG_SLOTS_FIXEDSIZE_OFFSET  2
#define MSG_CARTSLOTS_OFFSET        3
#define MSG_SLOTADDRLINES_OFFSET    4

#define MSG_SETBITS_OFFSET          1
#define MSG_UNSETBITS_OFFSET        2

#define MSG_ADDRHB_OFFSET           1
#define MSG_ADDRMB_OFFSET           2
#define MSG_ADDRLB_OFFSET           3

#define MSG_DATA16HB_OFFSET         4
#define MSG_DATA16LB_OFFSET         5

#define MSG_DATA8_OFFSET            5

#define MSG_64B_PACKET_COUNT        6
#define MSG_FWRITE_BYTE_COUNT       7
#define MSG_FWRITE_UBYPASS_MODE     8
#define MSG_ADDRNEGATIVEONE_OFFSET  9

#define MSG_TARGET_OFFSET           10

#define MSG_FWRITE_PAYLOAD_OFFSET  32

//when calling build_read64xN_command, this is the recommended value for N
#define COUNT_64xN (0x80)  //should be a power of 2 and fit into a uint8_t

#define LINE_RESET      0x01
#define LINE_CART41     0x02
#define LINE_CART42     0x04
#define LINE_CART45     0x08
#define LINE_CLK        0x10

namespace wsmsg
{

void build_blink_led_command(uint8_t *buf, uint8_t blinkCount)
{
  buf[MSG_TYPE_OFFSET] = MSG_BLINK_LED;
  buf[MSG_DATA8_OFFSET] = blinkCount;
}

void build_getversion_command(uint8_t *buf)
{
  buf[MSG_TYPE_OFFSET] = MSG_GETVERSION;
}

void build_getcartinfo_command(uint8_t *buf)
{
  buf[MSG_TYPE_OFFSET] = MSG_GET_CARTINFO_CMD;
}

void build_set_unset_lines_command(uint8_t *buf, uint8_t setBits, uint8_t unsetBits)
{
  buf[MSG_TYPE_OFFSET] = MSG_SET_UNSET_LINES_CMD;
  buf[MSG_SETBITS_OFFSET]     = setBits;
  buf[MSG_UNSETBITS_OFFSET]   = unsetBits;
}

void get_set_unset_lines_command(uint8_t *buf, uint8_t *setBits, uint8_t *unsetBits)
{
  *setBits = buf[MSG_SETBITS_OFFSET];
  *unsetBits = buf[MSG_UNSETBITS_OFFSET];
}

void build_getversion_reply(uint8_t *buf, uint8_t majVer, uint8_t minVer)
{
  buf[MSG_TYPE_OFFSET] = MSG_GETVERSION;
  buf[MSG_MAJORVER_OFFSET] = majVer;
  buf[MSG_MINORVER_OFFSET] = minVer;
}

void get_getversion_reply(uint8_t *buf, uint8_t *majVer, uint8_t *minVer)
{
  if(buf[MSG_TYPE_OFFSET] == MSG_GETVERSION)
  {
    *majVer = buf[MSG_ADDRHB_OFFSET];
    *minVer = buf[MSG_ADDRMB_OFFSET];
  }
  else
  {
    *majVer = 0;
    *minVer = 0;
  }
  
  return;
}

void build_getcartinfo_reply(uint8_t *buf, uint8_t isSlotNumFixed, uint8_t isSlotSizeFixed, uint8_t numSlotsPerCart, uint8_t numAddrLinesPerSlot)
{
  buf[MSG_TYPE_OFFSET] = MSG_GET_CARTINFO_CMD;
  buf[MSG_SLOTS_FIXEDNUM_OFFSET] = isSlotNumFixed;                            //fixed=1 variable=0
  buf[MSG_SLOTS_FIXEDSIZE_OFFSET] = isSlotSizeFixed;                          //fixed=1 variable=0
  buf[MSG_CARTSLOTS_OFFSET] = numSlotsPerCart;
  buf[MSG_SLOTADDRLINES_OFFSET] = numAddrLinesPerSlot;
}

void get_getcartinfo_reply(uint8_t *buf, uint8_t *isSlotNumFixed, uint8_t *isSlotSizeFixed, uint8_t *numSlotsPerCart, uint8_t *numAddrLinesPerSlot)
{
  if(buf[MSG_TYPE_OFFSET] == MSG_GET_CARTINFO_CMD)
  {
    *isSlotNumFixed = buf[MSG_SLOTS_FIXEDNUM_OFFSET];
    *isSlotSizeFixed = buf[MSG_SLOTS_FIXEDSIZE_OFFSET];
    *numSlotsPerCart = buf[MSG_CARTSLOTS_OFFSET];
    *numAddrLinesPerSlot = buf[MSG_SLOTADDRLINES_OFFSET];
  }
  else
  {
    *isSlotNumFixed = 0xFF;
    *isSlotSizeFixed = 0xFF;
    *numSlotsPerCart = 0xFF;
    *numAddrLinesPerSlot = 0xFF;
  }
  
  return;
}

void build_set_cartslot_command(uint8_t *buf, uint8_t slot_num)
{
  buf[MSG_TYPE_OFFSET] = MSG_SET_CARTSLOT_CMD;
  buf[MSG_DATA8_OFFSET] = slot_num;
}


//message structure
//byte  value
//0     msgType
//1     address high byte   ((addr>>16)&0xFF)
//2     address middle byte ((addr>> 8)&0xFF)
//3     address low byte    (addr&0xFF)
//4     data (8bits)

void build_write8_command(uint8_t *buf, uint32_t addr_host, uint8_t data, uint8_t target)
{
  buf[MSG_TYPE_OFFSET] = MSG_WRITE8_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_DATA8_OFFSET] = data;
  
  buf[MSG_TARGET_OFFSET] = target;//SRAM, PORT_IO, etc.
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("write 0x%02X to addr %02X %02X %02X %s (target %d)\n", buf[MSG_DATA8_OFFSET], buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET], buf[MSG_ADDRNEGATIVEONE_OFFSET] ? "1" : "0", target);
#endif
}

void build_write8_SRAM_command(uint8_t *buf, uint32_t addr_host, uint8_t data)
{
  build_write8_command(buf, addr_host, data, TARGET_SRAM);
}

void build_write16_command(uint8_t *buf, uint32_t addr_host, uint16_t data, uint8_t target)
{
  buf[MSG_TYPE_OFFSET] = MSG_WRITE16_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit addresses, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_DATA16HB_OFFSET] = data>>8;
  buf[MSG_DATA16LB_OFFSET] = data;
  
  buf[MSG_TARGET_OFFSET] = target;//SRAM, PORT_IO, etc.
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("write addr 0x%04X to %02X %02X %02X\n", (buf[MSG_DATA16HB_OFFSET] << 8) | buf[MSG_DATA16LB_OFFSET], buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
}

void build_flash_write_N_command(uint8_t *buf, uint32_t addr_host, const uint8_t *data, uint8_t n)
{
  buf[MSG_TYPE_OFFSET] = MSG_FLASHWRITE_N_CMD;
  buf[MSG_FWRITE_BYTE_COUNT] = n;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit addresses, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
  
  int i;
  for(i=0;i<n;i++)
    buf[MSG_FWRITE_PAYLOAD_OFFSET+i] = data[i];
}

void build_flash_write_32_command(uint8_t *buf, uint32_t addr_host, const uint8_t *data)
{
  buf[MSG_TYPE_OFFSET] = MSG_FLASHWRITE32_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit addresses, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  //printf("addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
  
  int i;
  for(i=0;i<32;i++)
    buf[MSG_FWRITE_PAYLOAD_OFFSET+i] = data[i];
}


void build_flash_write64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t n)
{
  buf[MSG_TYPE_OFFSET] = MSG_FLASHWRITE64xN_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit addresses, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_64B_PACKET_COUNT] = n;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("flash write addr 0x%02X %02X %02X (%d 64-byte packets = %d bytes)\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET],
         buf[MSG_64B_PACKET_COUNT], buf[MSG_64B_PACKET_COUNT] * 64);
#endif
  
}

void build_flash_write64xN_data_packet(uint8_t *buf, const uint8_t *data)
{
  int i;
  for(i=0;i<64;i++)
    buf[i] = data[i];
}

void build_sram_write64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t n)
{
  buf[MSG_TYPE_OFFSET] = MSG_SRAMWRITE64xN_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit addresses, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_64B_PACKET_COUNT] = n;
}

void build_sram_write64xN_data_packet(uint8_t *buf, const uint8_t *data)
{
  int i;
  for(i=0;i<64;i++)
    buf[i] = data[i];
}

void build_read8_command(uint8_t *buf, uint32_t addr_host, uint8_t target)
{
  buf[MSG_TYPE_OFFSET] = MSG_READ8_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit reads, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_TARGET_OFFSET] = target;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("read8 addr %02X %02X %02X %s\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET], buf[MSG_ADDRNEGATIVEONE_OFFSET] ? "1" : "0");
#endif
}

void build_read8_SRAM_command(uint8_t *buf, uint32_t addr_host)
{
  build_read8_command(buf, addr_host, TARGET_SRAM);
}

void build_read16_command(uint8_t *buf, uint32_t addr_host, uint8_t target)
{
  //a 16bit read should only be to
  buf[MSG_TYPE_OFFSET] = MSG_READ16_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit reads, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_TARGET_OFFSET] = target;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("read16 addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
}

void build_read64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t n, uint8_t target_chip)
{
  //count better not be 0!
  buf[MSG_TYPE_OFFSET] = MSG_READ64xN_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit reads, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
  buf[MSG_TARGET_OFFSET] = target_chip;
  
  buf[MSG_64B_PACKET_COUNT] = n;
}

void build_read64_command(uint8_t *buf, uint32_t addr, uint8_t target_chip)
{
  build_read64xN_command(buf, addr, 1, target_chip);
}

void build_eeprom_write_N_command(uint8_t *buf, uint32_t addr_host, uint8_t *data, uint8_t n)
{
  buf[MSG_TYPE_OFFSET] = MSG_EEPROMWRITE_N_CMD;
  buf[MSG_FWRITE_BYTE_COUNT] = n;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX))
  if((addr_host + n) > 4096)//4096 bytes of EEPROM on AT90USB1286
    printf("addr %02X %02X %02X num bytes %d out of range\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET], buf[MSG_FWRITE_BYTE_COUNT]);
  if(n > 32)//packet can hold up to 32 bytes
    printf("num_bytes %d out of range\n", buf[MSG_FWRITE_BYTE_COUNT]);
#endif
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  //printf("addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
  
  int i;
  for(i=0;i<n;i++)
    buf[MSG_FWRITE_PAYLOAD_OFFSET+i] = data[i];
}


/*void build_SPI_send_recv_command(uint8_t *buf, uint8_t data)
 {
 buf[MSG_TYPE_OFFSET] = MSG_SPI_SEND_RECV_CMD;
 buf[MSG_DATA_OFFSET] = data;
 }*/


void build_read8_reply(uint8_t *buf, uint8_t addrHB, uint8_t addrMB, uint8_t addrLB, uint8_t addrNO, uint8_t data)
{
  buf[MSG_TYPE_OFFSET] = MSG_READ_REPLY;
  buf[MSG_ADDRHB_OFFSET] = addrHB;
  buf[MSG_ADDRMB_OFFSET] = addrMB;
  buf[MSG_ADDRLB_OFFSET] = addrLB;
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addrNO;
  buf[MSG_DATA8_OFFSET] = data;
}

void build_read16_reply(uint8_t *buf, uint8_t addrHB, uint8_t addrMB, uint8_t addrLB, uint8_t dataHigh, uint8_t dataLow)
{
  buf[MSG_TYPE_OFFSET] = MSG_READ_REPLY;
  buf[MSG_ADDRHB_OFFSET] = addrHB;
  buf[MSG_ADDRMB_OFFSET] = addrMB;
  buf[MSG_ADDRLB_OFFSET] = addrLB;
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = 0;
  buf[MSG_DATA16HB_OFFSET] = dataHigh;
  buf[MSG_DATA16LB_OFFSET] = dataLow;
}

void build_write64xN_reply(uint8_t *buf, uint8_t packetsProcessed)
{
  buf[MSG_TYPE_OFFSET] = MSG_WRITE64xN_REPLY;
  buf[MSG_64B_PACKET_COUNT] = packetsProcessed;
}

void build_reply_success(uint8_t *buf)
{
  buf[MSG_TYPE_OFFSET] = MSG_RESULT_SUCCESS;
}

void build_reply_fail(uint8_t *buf)
{
  buf[MSG_TYPE_OFFSET] = MSG_RESULT_FAIL;
}

uint8_t *get_flash_write_32_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  
  return &buf[MSG_FWRITE_PAYLOAD_OFFSET];
}

uint8_t *get_flash_write_N_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *n)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  *n = buf[MSG_FWRITE_BYTE_COUNT];
  
  return &buf[MSG_FWRITE_PAYLOAD_OFFSET];
}

void get_flash_write64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *n)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  *n = buf[MSG_64B_PACKET_COUNT];
}

void get_sram_write64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *n)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  
  *n = buf[MSG_64B_PACKET_COUNT];
}

void get_read64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *n, uint8_t *targetChip)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  
  *targetChip = buf[MSG_TARGET_OFFSET];
  
  *n = buf[MSG_64B_PACKET_COUNT];
}

uint8_t *get_eeprom_write_N_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *n)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *n = buf[MSG_FWRITE_BYTE_COUNT];
  
  return &buf[MSG_FWRITE_PAYLOAD_OFFSET];
}


void build_set_addr_command(uint8_t *buf, uint32_t addr_host)
{
  buf[MSG_TYPE_OFFSET] = MSG_SET_ADDR_LINES_CMD;
  
  buf[MSG_ADDRNEGATIVEONE_OFFSET] = addr_host & 1;  //for 8bit addresses, we need to save this bit and pass it on
  addr_host >>= 1;
  
  buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
  buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
  buf[MSG_ADDRLB_OFFSET] = addr_host;
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("read addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
}

void get_set_addr_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
}

void build_set_data16_command(uint8_t *buf, uint16_t data)
{
  buf[MSG_TYPE_OFFSET] = MSG_SET_DATA_LINES_CMD;
  
  buf[MSG_DATA16HB_OFFSET] = data>>8;
  buf[MSG_DATA16LB_OFFSET] = data;
}

void get_set_data16_message(uint8_t *buf, uint8_t *dataHB, uint8_t *dataLB)
{
  *dataHB = buf[MSG_DATA16HB_OFFSET];
  *dataLB = buf[MSG_DATA16LB_OFFSET];
}


void get_read_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *targetChip)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  *targetChip = buf[MSG_TARGET_OFFSET];
}

void get_write8_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *addrNO, uint8_t *data8, uint8_t *targetChip)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *addrNO = buf[MSG_ADDRNEGATIVEONE_OFFSET];
  *data8 = buf[MSG_DATA8_OFFSET];
  *targetChip = buf[MSG_TARGET_OFFSET];
}

void get_write16_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *dataHB, uint8_t *dataLB, uint8_t *targetChip)
{
  *addrHB = buf[MSG_ADDRHB_OFFSET];
  *addrMB = buf[MSG_ADDRMB_OFFSET];
  *addrLB = buf[MSG_ADDRLB_OFFSET];
  *dataHB = buf[MSG_DATA16HB_OFFSET];
  *dataLB = buf[MSG_DATA16LB_OFFSET];
  *targetChip = buf[MSG_TARGET_OFFSET];
}
/*
 void get_SPI_send_recv_message(uint8_t *buf, uint8_t *data)
 {
 *data   = buf[MSG_DATA_OFFSET];
 }
 */
int get_read8_reply(uint8_t *buf, uint32_t *addr32, uint8_t *data)
{
  uint32_t addr;
  
  if(buf[MSG_TYPE_OFFSET] != MSG_READ_REPLY)  {
    return 0;
  }
  
  addr = (uint32_t)buf[MSG_ADDRHB_OFFSET] << 16;
  addr |= buf[MSG_ADDRMB_OFFSET] << 8;
  addr |= buf[MSG_ADDRLB_OFFSET];
  addr <<= 1;
  addr |= buf[MSG_ADDRNEGATIVEONE_OFFSET] & 0x01;
  
  *addr32 = addr;
  
  *data   = buf[MSG_DATA8_OFFSET];
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("read reply addr %02X %02X %02X, data %02X\n", (addr>>16) & 0xff, (addr>>8) & 0xff, addr & 0xff, *data);
#endif
  
  
  return 1;
}

int get_read16_reply(uint8_t *buf, uint32_t *addr32, uint16_t *data)
{
  uint32_t addr;
  
  if(buf[MSG_TYPE_OFFSET] != MSG_READ_REPLY)  {
    return 0;
  }
  
  addr = (uint32_t) buf[MSG_ADDRHB_OFFSET] << 16;
  addr |= buf[MSG_ADDRMB_OFFSET] << 8;
  addr |= buf[MSG_ADDRLB_OFFSET];
  addr <<= 1;
  addr |= buf[MSG_ADDRNEGATIVEONE_OFFSET] & 0x01;  //this really needs to be 0
  *addr32 = addr;
  
#if 0
  *addr32 = ntohl(((uint32_t)buf[MSG_ADDRHB_OFFSET] << 8) | ((uint32_t)buf[MSG_ADDRMB_OFFSET] << 16) | ((uint32_t)buf[MSG_ADDRLB_OFFSET] << 24));
#endif
  
  *data   = buf[MSG_DATA16LB_OFFSET] | (buf[MSG_DATA16HB_OFFSET] << 8);
  
#if (defined(OS_WINDOWS) || defined(OS_MACOSX)) && defined(DEBUG)
  printf("read reply addr %02X %02X %02X, data %02X\n", (addr>>16) & 0xff, (addr>>8) & 0xff, addr & 0xff, *data);
#endif
  
  
  return 1;
}

void get_write64xN_reply(uint8_t *buf, uint8_t *msgType, uint8_t *packetsProcessed)
{
  *msgType = buf[MSG_TYPE_OFFSET];
  *packetsProcessed = buf[MSG_64B_PACKET_COUNT];
}

void get_result_reply(uint8_t *buf, uint8_t *result)
{
  *result = buf[MSG_TYPE_OFFSET];
}

void get_blink_led_message(uint8_t *buf, uint8_t *blinkCount)
{
  *blinkCount = buf[MSG_DATA8_OFFSET];
}

void get_set_cartslot_command(uint8_t *buf, uint8_t *slot_num)
{
  *slot_num = buf[MSG_DATA8_OFFSET];
}
};
