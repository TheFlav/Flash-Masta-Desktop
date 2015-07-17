#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <stdint.h>

#if defined(OS_WINDOWS)
 #include <winsock2.h>
#elif ARCH==ARCH_AVR8
#elif defined(OS_LINUX) || defined(OS_MACOSX)
#else
 #error Undefined hardware/software platform
#endif

#define MSG_GETVERSION              0x00
#define MSG_READ_CMD                0x01
#define MSG_WRITE_CMD               0x02
#define MSG_READ_REPLY              0x03
#define MSG_READ64xN_CMD            0x04
#define MSG_FLASHWRITE32_CMD        0x05
#define MSG_FLASHWRITE_N_CMD        0x06
#define MSG_FLASHWRITE64xN_CMD      0x07
#define MSG_WRITE64xN_REPLY         0x08
#define MSG_BLINK_LED               0x09
#define MSG_SPI_SEND_RECV_CMD       0x0A

#define MSG_RESULT_FAIL             0x10
#define MSG_RESULT_SUCCESS          0x11

#define MSG_TYPE_OFFSET             0

#define MSG_MAJORVER_OFFSET         1
#define MSG_MINORVER_OFFSET         2

#define MSG_ADDRHB_OFFSET           1
#define MSG_ADDRMB_OFFSET           2
#define MSG_ADDRLB_OFFSET           3

#define MSG_DATA_OFFSET             4
#define MSG_CHIP_SELECT_OFFSET      5
#define MSG_64B_PACKET_COUNT        6
#define MSG_FWRITE_BYTE_COUNT       7
#define MSG_FWRITE_UBYPASS_MODE     8
#define MSG_FWRITE_PAYLOAD_OFFSET  32

//when calling build_read64xN_command, this is the recommended value for N
#define COUNT_64xN (0x80)  //should be a power of 2 and fit into a uint8_t


void build_blink_led_command(uint8_t *buf, uint8_t blinkCount)
{
    buf[MSG_TYPE_OFFSET] = MSG_BLINK_LED;
    buf[MSG_DATA_OFFSET] = blinkCount;
}

void build_getversion_command(uint8_t *buf)
{
    buf[MSG_TYPE_OFFSET] = MSG_GETVERSION;
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

//message structure
//byte  value
//0     msgType
//1     address high byte   ((addr>>16)&0xFF)
//2     address middle byte ((addr>> 8)&0xFF)
//3     address low byte    (addr&0xFF)
//4     data (8bits)

void build_write_command(uint8_t *buf, uint32_t addr_host, uint8_t data, uint8_t chip)
{
    buf[MSG_TYPE_OFFSET] = MSG_WRITE_CMD;
    buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
    buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
    buf[MSG_ADDRLB_OFFSET] = addr_host;

    buf[MSG_DATA_OFFSET] = data;
    buf[MSG_CHIP_SELECT_OFFSET] = chip;

#if defined(OS_WINDOWS) || defined(OS_MACOSX)
    //printf("write addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif
}

void build_flash_write_N_command(uint8_t *buf, uint32_t addr_host, uint8_t *data, uint8_t chip, uint8_t n, uint8_t uBypassMode)
{
    buf[MSG_TYPE_OFFSET] = MSG_FLASHWRITE_N_CMD;
    buf[MSG_FWRITE_BYTE_COUNT] = n;

    buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
    buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
    buf[MSG_ADDRLB_OFFSET] = addr_host;

    buf[MSG_CHIP_SELECT_OFFSET] = chip;
    buf[MSG_FWRITE_UBYPASS_MODE] = uBypassMode;

#ifdef OS_WINDOWS
    //printf("addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif

    int i;
    for(i=0;i<n;i++)
        buf[MSG_FWRITE_PAYLOAD_OFFSET+i] = data[i];
}

void build_flash_write_32_command(uint8_t *buf, uint32_t addr_host, uint8_t *data, uint8_t chip, uint8_t uBypassMode)
{
    buf[MSG_TYPE_OFFSET] = MSG_FLASHWRITE32_CMD;

    buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
    buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
    buf[MSG_ADDRLB_OFFSET] = addr_host;

    buf[MSG_CHIP_SELECT_OFFSET] = chip;
    buf[MSG_FWRITE_UBYPASS_MODE] = uBypassMode;

#ifdef OS_WINDOWS
    //printf("addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif

    int i;
    for(i=0;i<32;i++)
        buf[MSG_FWRITE_PAYLOAD_OFFSET+i] = data[i];
}

void build_flash_write64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t chip, uint8_t n, uint8_t uBypassMode)
{
    buf[MSG_TYPE_OFFSET] = MSG_FLASHWRITE64xN_CMD;

    buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
    buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
    buf[MSG_ADDRLB_OFFSET] = addr_host;

    buf[MSG_CHIP_SELECT_OFFSET] = chip;
    buf[MSG_64B_PACKET_COUNT] = n;
    buf[MSG_FWRITE_UBYPASS_MODE] = uBypassMode;
}

void build_flash_write64xN_data_packet(uint8_t *buf, uint8_t *data)
{
    int i;
    for(i=0;i<64;i++)
        buf[i] = data[i];
}

void build_read_command(uint8_t *buf, uint32_t addr_host, uint8_t chip)
{
    buf[MSG_TYPE_OFFSET] = MSG_READ_CMD;

    buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
    buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
    buf[MSG_ADDRLB_OFFSET] = addr_host;

#if defined(OS_WINDOWS) || defined(OS_MACOSX)
    //printf("read addr %02X %02X %02X\n", buf[MSG_ADDRHB_OFFSET], buf[MSG_ADDRMB_OFFSET], buf[MSG_ADDRLB_OFFSET]);
#endif

    buf[MSG_CHIP_SELECT_OFFSET] = chip;
}

void build_read64xN_command(uint8_t *buf, uint32_t addr_host, uint8_t chip, uint8_t n)
{
    //count better not be 0!
    buf[MSG_TYPE_OFFSET] = MSG_READ64xN_CMD;

    buf[MSG_ADDRHB_OFFSET] = addr_host>>16;
    buf[MSG_ADDRMB_OFFSET] = addr_host>>8;
    buf[MSG_ADDRLB_OFFSET] = addr_host;

    buf[MSG_CHIP_SELECT_OFFSET] = chip;

    buf[MSG_64B_PACKET_COUNT] = n;
}

void build_SPI_send_recv_command(uint8_t *buf, uint8_t data)
{
    buf[MSG_TYPE_OFFSET] = MSG_SPI_SEND_RECV_CMD;
    buf[MSG_DATA_OFFSET] = data;
}

void build_read64_command(uint8_t *buf, uint32_t addr, uint8_t chip)
{
    build_read64xN_command(buf, addr, chip, 1);
}

void build_read_reply(uint8_t *buf, uint8_t addrHB, uint8_t addrMB, uint8_t addrLB, uint8_t data, uint8_t chip)
{
    buf[MSG_TYPE_OFFSET] = MSG_READ_REPLY;
    buf[MSG_ADDRHB_OFFSET] = addrHB;
    buf[MSG_ADDRMB_OFFSET] = addrMB;
    buf[MSG_ADDRLB_OFFSET] = addrLB;
    buf[MSG_DATA_OFFSET] = data;
    buf[MSG_CHIP_SELECT_OFFSET] = chip;
}

void build_flash_write64xN_reply(uint8_t *buf, uint8_t packetsProcessed)
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


uint8_t *get_flash_write_32_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *uBypassMode)
{
    *addrHB = buf[MSG_ADDRHB_OFFSET];
    *addrMB = buf[MSG_ADDRMB_OFFSET];
    *addrLB = buf[MSG_ADDRLB_OFFSET];
    *chip = buf[MSG_CHIP_SELECT_OFFSET];
    *uBypassMode = buf[MSG_FWRITE_UBYPASS_MODE];

    return &buf[MSG_FWRITE_PAYLOAD_OFFSET];
}

uint8_t *get_flash_write_N_command(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *n, uint8_t *uBypassMode)
{
    *addrHB = buf[MSG_ADDRHB_OFFSET];
    *addrMB = buf[MSG_ADDRMB_OFFSET];
    *addrLB = buf[MSG_ADDRLB_OFFSET];
    *chip = buf[MSG_CHIP_SELECT_OFFSET];
    *n = buf[MSG_FWRITE_BYTE_COUNT];
    *uBypassMode = buf[MSG_FWRITE_UBYPASS_MODE];

    return &buf[MSG_FWRITE_PAYLOAD_OFFSET];
}

void get_flash_write64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *n, uint8_t *uBypassMode)
{
    *addrHB = buf[MSG_ADDRHB_OFFSET];
    *addrMB = buf[MSG_ADDRMB_OFFSET];
    *addrLB = buf[MSG_ADDRLB_OFFSET];
    *chip = buf[MSG_CHIP_SELECT_OFFSET];
    *n = buf[MSG_64B_PACKET_COUNT];
    *uBypassMode = buf[MSG_FWRITE_UBYPASS_MODE];
}

void get_read64xN_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip, uint8_t *n)
{
    *addrHB = buf[MSG_ADDRHB_OFFSET];
    *addrMB = buf[MSG_ADDRMB_OFFSET];
    *addrLB = buf[MSG_ADDRLB_OFFSET];
    *chip = buf[MSG_CHIP_SELECT_OFFSET];
    *n = buf[MSG_64B_PACKET_COUNT];
}

void get_read_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *chip)
{
    *addrHB = buf[MSG_ADDRHB_OFFSET];
    *addrMB = buf[MSG_ADDRMB_OFFSET];
    *addrLB = buf[MSG_ADDRLB_OFFSET];
    *chip = buf[MSG_CHIP_SELECT_OFFSET];
}

void get_write_message(uint8_t *buf, uint8_t *addrHB, uint8_t *addrMB, uint8_t *addrLB, uint8_t *data, uint8_t *chip)
{
    *addrHB = buf[MSG_ADDRHB_OFFSET];
    *addrMB = buf[MSG_ADDRMB_OFFSET];
    *addrLB = buf[MSG_ADDRLB_OFFSET];
    *data   = buf[MSG_DATA_OFFSET];
    *chip = buf[MSG_CHIP_SELECT_OFFSET];
}

void get_SPI_send_recv_message(uint8_t *buf, uint8_t *data)
{
    *data   = buf[MSG_DATA_OFFSET];
}

int get_read_reply(uint8_t *buf, uint32_t *addr21, uint8_t *data)
{
    uint32_t addr;

    if(buf[MSG_TYPE_OFFSET] != MSG_READ_REPLY)  {
        return 0;
    }

    addr = buf[MSG_ADDRHB_OFFSET] << 16;
    addr |= buf[MSG_ADDRMB_OFFSET] << 8;
    addr |= buf[MSG_ADDRLB_OFFSET];
    *addr21 = addr;

#if 0
    *addr21 = ntohl(((uint32_t)buf[MSG_ADDRHB_OFFSET] << 8) | ((uint32_t)buf[MSG_ADDRMB_OFFSET] << 16) | ((uint32_t)buf[MSG_ADDRLB_OFFSET] << 24));
#endif

    *data   = buf[MSG_DATA_OFFSET];

#if defined(OS_WINDOWS) || defined(OS_MACOSX)
    //printf("read reply addr %02X %02X %02X, data %02X\n", (addr>>16) & 0xff, (addr>>8) & 0xff, addr & 0xff, *data);
#endif


    return 1;
}

void get_flash_write64xN_reply(uint8_t *buf, uint8_t *msgType, uint8_t *packetsProcessed)
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
    *blinkCount = buf[MSG_DATA_OFFSET];
}

#endif //_MESSAGES_H_
