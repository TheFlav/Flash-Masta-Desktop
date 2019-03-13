#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "libusb-1.0/libusb.h"
#include <ctype.h>

//#define DEBUG

#if !defined(OS_MACOSX)
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_MAC)
#define OS_MACOSX
#endif
#endif
#endif

#include "../Common/ids.h"
#include "../Common/messages.h"

#include "../Firmware/WS_RTC.h"
#include "../Firmware/ws_avr_messages.h"

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/ioctl.h>
#include <termios.h>
#elif defined(OS_WINDOWS)
#include <conio.h>
#endif

#define FLASH_ACCESS_DATA_BITS 16

#define ENDPOINT_BULK_IN 0x81 /* endpoint 0x81 address for IN */
#define ENDPOINT_BULK_OUT 0x02 /* endpoint 0x02 address for OUT */

#define USB_RXTX_SIZE 64
#define USB_TIMEOUT 6000

/*#define FLASH_CHIP_SIZE_IN_MBITS 128
#define FLASH_CHIP_SIZE_IN_BYTES (FLASH_CHIP_SIZE_IN_MBITS * 1024 * 1024 / 8)
#define FLASH_CHIP_SIZE_IN_WORDS (FLASH_CHIP_SIZE_IN_BYTES >> 1)*/

// options
#define OPTION_INFO             0x0001
#define OPTION_BACKUP           0x0002
#define OPTION_WRITE            0x0004
#define OPTION_VERIFY           0x0008
#define OPTION_PROTECT          0x0010
#define OPTION_BACKUP_SAVEGAME  0x0020
#define OPTION_RESTORE_SAVEGAME 0x0040
#define OPTION_MICROSD          0x0080
#define OPTION_INITWSMMC        0x0100
#define OPTION_TEST				0x0200

char *filename = NULL;
char *verifyFilename = NULL;

char *saveFilename = NULL;

uint8_t unlockBypassMode = 0;

uint32_t cart_size_override = 0;

uint8_t g_numSlotsPerCart = 0xFF, g_numAddrLinesPerSlot = 0xFF;

//uint32_t write_N = 64*8;//for 64xN mode, this needs to be a multiple of 64
uint32_t write_N = 8192;//was 1
uint32_t skipChip = 0xFFFFFFFF;//if set to 0 or 1, skip operations on that chip

/*
 uint32_t ws1_addr = 0x5555;
uint32_t ws2_addr = 0xAAAA;
uint32_t ws3_addr = 0x5555;*/
uint32_t ws1_addr = 0x2AAA;
uint32_t ws2_addr = 0x5555;
uint32_t ws3_addr = 0x2AAA;

//static char get_keystroke(void);


uint32_t bootBlockStartAddr[2]={0};
char cartRomName[14];
uint8_t prot[2][35], bootBlockStartNum[2]={0}, chipSize[2]={0};
uint16_t manufID[2]={0}, deviceID[2]={0}, factoryProt[2]={0};
void erase_cart();

void ws_rtc_test(uint8_t, uint8_t);
void ws_rtc_file_test(char *);
void ws_rtc_select_cart_slot(uint8_t slot_num);
void ws_gpo_select_cart_slot(uint8_t slot_num);
uint8_t assumeYES = 0;

//timeouts in ms
#define HID_SEND_TIMEOUT 1000      //original was 100ms
#define HID_RECV_TIMEOUT (60*1000) //60s

uint8_t buf[USB_RXTX_SIZE];

#include <time.h>


#if defined(OS_WINDOWS)
/*void usleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}*/
#elif defined(OS_LINUX) || defined(OS_MACOSX)
#else
 #warning unknown platform
#endif

static struct libusb_device_handle *devh = NULL;

static int find_lvr_usb(void)
{
	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	return devh ? 0 : -EIO;
}

static void print_libusb_error(int code){
	fprintf(stdout,"\n");
	switch(code){
	case LIBUSB_ERROR_IO:
		fprintf(stdout,"Error: LIBUSB_ERROR_IO\nInput/output error.\n");
		break;
	case LIBUSB_ERROR_INVALID_PARAM:
		fprintf(stdout,"Error: LIBUSB_ERROR_INVALID_PARAM\nInvalid parameter.\n");
		break;
	case LIBUSB_ERROR_ACCESS:
		fprintf(stdout,"Error: LIBUSB_ERROR_ACCESS\nAccess denied (insufficient permissions).\n");
		break;
	case LIBUSB_ERROR_NO_DEVICE:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_DEVICE\nNo such device (it may have been disconnected).\n");
		break;
	case LIBUSB_ERROR_NOT_FOUND:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_FOUND\nEntity not found.\n");
		break;
	case LIBUSB_ERROR_BUSY:
		fprintf(stdout,"Error: LIBUSB_ERROR_BUSY\nResource busy.\n");
		break;
	case LIBUSB_ERROR_TIMEOUT:
		fprintf(stdout,"Error: LIBUSB_ERROR_TIMEOUT\nOperation timed out.\n");
		break;
	case LIBUSB_ERROR_OVERFLOW:
		fprintf(stdout,"Error: LIBUSB_ERROR_OVERFLOW\nOverflow.\n");
		break;
	case LIBUSB_ERROR_PIPE:
		fprintf(stdout,"Error: LIBUSB_ERROR_PIPE\nPipe error.\n");
		break;
	case LIBUSB_ERROR_INTERRUPTED:
		fprintf(stdout,"Error:LIBUSB_ERROR_INTERRUPTED\nSystem call interrupted (perhaps due to signal).\n");
		break;
	case LIBUSB_ERROR_NO_MEM:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_MEM\nInsufficient memory.\n");
		break;
	case LIBUSB_ERROR_NOT_SUPPORTED:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_SUPPORTED\nOperation not supported or unimplemented on this platform.\n");
		break;
	case LIBUSB_ERROR_OTHER:
		fprintf(stdout,"Error: LIBUSB_ERROR_OTHER\nOther error.\n");
		break;
	default:
		fprintf(stdout, "Error: unkown error\n");
	}
}

unsigned char getBootBlockStart()
{
    switch(chipSize[0])
    {
        case 16:
            return 31;
        case 8:
	        return 15;
        case 4:
	        return 7;
    }
    return 0;
}


unsigned long blockNumToAddr(unsigned char chip, unsigned char blockNum)
{
    unsigned char bootBlockStartNum = getBootBlockStart();
    unsigned long addr;

    if(blockNum >= bootBlockStartNum)
    {
        addr = bootBlockStartNum * 0x10000;

        unsigned char bootBlock = blockNum - bootBlockStartNum;
        if(bootBlock>=1)
            addr+= 0x8000;
        if(bootBlock>=2)
            addr+= 0x2000;
        if(bootBlock>=3)
            addr+= 0x2000;
    }
    else
        addr = blockNum * 0x10000;

    if(chip)
        addr+=0x200000;

	return addr;
}

unsigned char addrToBlockNum(unsigned long addr)
{
    unsigned char blockNum;

    unsigned char chip = addr<0x200000 ? 0 : 1;

    for(blockNum=0;blockNum<34;blockNum++)
    {
        if((addr >= blockNumToAddr(chip, blockNum)) && (addr < blockNumToAddr(chip, blockNum+1)))
            break;
    }
    return blockNum;
}


unsigned long blockSize(unsigned char blockNum)
{
    unsigned char bootBlockStartNum = getBootBlockStart();
    if(blockNum >= bootBlockStartNum)
    {
        unsigned char bootBlock = blockNum - bootBlockStartNum;
        if(bootBlock==3)
            return 0x4000;
        if(bootBlock==2)
            return 0x2000;
        if(bootBlock==1)
            return 0x2000;
        if(bootBlock==0)
            return 0x8000;
    }

    return 0x10000;
}

struct NGFheaderStruct
{
	unsigned short version;		//always 0x53?
	unsigned short numBlocks;	//how many blocks are in the file
	unsigned long fileLen;		//length of the file
} ;

struct blockStruct
{
	unsigned long NGPCaddr;  //where this block starts (in NGPC memory map)
	unsigned long len;  // length of following data
} ;

void blink_linkmasta_led(uint8_t blinkCount)
{
    int num, r;
    build_blink_led_command(buf, blinkCount);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}


    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != 64)
    {
        printf("\nblink_linkmasta_led: libusb_bulk_transfer fail\n");
        fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS)
        {
            printf("blink_linkmasta_led: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}



void check_firmware_version(uint8_t *majVer, uint8_t *minVer)
{
    int num, r;
    build_getversion_command(buf);

    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
	printf("check_firmware_version: transfer to device failed\n");
        exit(-1);
    }

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
	printf("check_firmware_version: response from device failed\n");
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\ncheck_firmware_version: received %d byted, expected 64\n", num);
        fflush(stdout);
        exit(-1);
    }
    else
    {
        get_getversion_reply(buf, majVer, minVer);
    }
}

void get_cartinfo(uint8_t *isSlotNumFixed, uint8_t *isSlotSizeFixed, uint8_t *numSlotsPerCart, uint8_t *numAddrLinesPerSlot)
{
    int num, r;
    build_getcartinfo_command(buf);
    
    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        printf("get_cartinfo: transfer to device failed\n");
        exit(-1);
    }
    
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        printf("get_cartinfo: response from device failed\n");
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nget_cartinfo: received %d byted, expected 64\n", num);
        fflush(stdout);
        exit(-1);
    }
    else
    {
        get_getcartinfo_reply(buf, isSlotNumFixed, isSlotSizeFixed, numSlotsPerCart, numAddrLinesPerSlot);
    }
}

void set_unset_WS_lines(uint8_t setLines, uint8_t unsetLines)
{
    int num, r;
    build_set_unset_lines_command(buf, setLines, unsetLines);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}


    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != 64)
    {
        printf("\nset_unset_WS_lines: libusb_bulk_transfer fail\n");
        fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS)
        {
            printf("set_unset_WS_lines: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}


void flash_write_32_bytes(uint32_t addr, uint8_t *data)
{
    int num, r;

    printf("flash_write_32_bytes: writing 32 bytes to addr=0x%X\n", addr);
    
    build_flash_write_32_command(buf, addr, data);
	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != 64)
    {
        printf("\nflash_write_32_bytes: libusb_bulk_transfer fail (addr=0x%X)\n", addr);
        fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS)
            printf("flash_write_32_bytes: result = 0x%02X\n", result);
            fflush(stdout);
    }
}

void flash_write_N_bytes(uint32_t addr, uint8_t *data, uint8_t n)
{
    int num, r;
    printf("flash_write_N_bytes: writing %d bytes to addr=0x%X\n", n, addr);
    
    build_flash_write_N_command(buf, addr, data, n);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nflash_write_N_bytes: libusb_bulk_transfer fail (addr=0x%X)\n", addr);
        fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS)
            printf("nflash_write_N_bytes: result = 0x%02X\n", result);
            fflush(stdout);
    }
}

void flash_write_64xN_bytes(uint32_t addr, uint8_t *data, uint8_t numPackets)
{
    int num, r;
    uint8_t packetsSent = 0;
    build_flash_write64xN_command(buf, addr, numPackets);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    while(packetsSent < numPackets)
    {
        build_flash_write64xN_data_packet(buf, &data[packetsSent*64]);

        r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
        if(r<0)
        {
            print_libusb_error(r);
            exit(-1);
        }

        if(num != USB_RXTX_SIZE)
            printf("r=%d\n", num);
            fflush(stdout);

        packetsSent++;
    }

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nflash_write_64xN_bytes: libusb_bulk_transfer ENDPOINT_BULK_IN fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t result, packetsProcessed;

        get_write64xN_reply(buf, &result, &packetsProcessed);

        if(result != MSG_WRITE64xN_REPLY) {
            printf("\nflash_write_64xN_bytes: result = 0x%02X\n", result);
            fflush(stdout);
        }
        if(packetsProcessed != numPackets) {
            printf("\nflash_write_64xN_bytes: processed %d packets, but sent %d\n", packetsProcessed, numPackets);
            fflush(stdout);
        }
    }
}


void sram_write_64xN_bytes(uint32_t addr, uint8_t *data, uint8_t numPackets)
{
    int num, r;
    uint8_t packetsSent = 0;
    build_sram_write64xN_command(buf, addr, numPackets);
    
    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    
    while(packetsSent < numPackets)
    {
        build_sram_write64xN_data_packet(buf, &data[packetsSent*64]);
        
        r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
        if(r<0)
        {
            print_libusb_error(r);
            exit(-1);
        }
        
        if(num != USB_RXTX_SIZE)
            printf("r=%d\n", num);
        fflush(stdout);
        
        packetsSent++;
    }
    
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nsram_write_64xN_bytes: libusb_bulk_transfer ENDPOINT_BULK_IN fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t result, packetsProcessed;
        
        get_write64xN_reply(buf, &result, &packetsProcessed);
        
        if(result != MSG_WRITE64xN_REPLY) {
            printf("\nsram_write_64xN_bytes: result = 0x%02X\n", result);
            fflush(stdout);
        }
        if(packetsProcessed != numPackets) {
            printf("\nsram_write_64xN_bytes: processed %d packets, but sent %d\n", packetsProcessed, numPackets);
            fflush(stdout);
        }
    }
}


void write_word(uint32_t addr, uint16_t data, uint8_t target)
{
    int num, r;
    build_write16_command(buf, addr, data, target);

    //printf("write_word: writing 0x%04X to 0x%06X\n", data, addr);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nwrite_word: libusb_bulk_transfer fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS) {
            printf("write_word: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}

void write_word_rom(uint32_t addr, uint16_t data)
{
    write_word(addr, data, TARGET_ROM);
}

void write_word_cart(uint32_t addr, uint16_t data)
{
    write_word(addr, data, TARGET_CART);
}

void write_byte_target(uint32_t addr, uint16_t data, uint8_t target)
{
    int num, r;
    build_write8_command(buf, addr, data, target);
    
    //printf("write_word: writing 0x%04X to 0x%06X\n", data, addr);
    
    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nwrite_byte_sram: libusb_bulk_transfer fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t result;
        
        get_result_reply(buf, &result);
        
        if(result != MSG_RESULT_SUCCESS) {
            printf("write_byte_sram: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}

void write_byte_sram(uint32_t addr, uint16_t data)
{
    write_byte_target(addr, data, TARGET_SRAM);
}

void write_byte_port(uint32_t addr, uint16_t data)
{
    //printf("write_byte_port: 0x%06X 0x%02X\n", addr, data);
    write_byte_target(addr, data, TARGET_PORTIO);
}

void write_byte_rom(uint32_t addr, uint16_t data)
{
    write_byte_target(addr, data, TARGET_ROM);
}

void write_bytes_to_avr_via_gpo(uint8_t num_bytes, uint8_t *data)
{
    uint8_t gpo_out;
    uint8_t bytes_sent = 0;
    
    write_byte_port(0xC00CD, 0x06); //wake up the AVR (via GPO3) and set high 3 GPO low (low wakes AVR)
    write_byte_port(0xC00CC, 0x0E); //turn on all 3 GPO outputs (GPO3, 2, 1)
    
    usleep(100000);  //wakeup from powerdown (as opposed to idle) needs more time

    for(bytes_sent = 0; bytes_sent < num_bytes; bytes_sent++)
    {
        uint8_t send_byte = data[bytes_sent];
        for(int i=0; i < 8; i++)
        {
            if(send_byte & 0x80)
                gpo_out = 0x02;        //~GPO3, GPO1, ~GPO2
            else
                gpo_out = 0x00;        //~GPO3, ~GPO1, ~GPO2
            send_byte <<= 1;
            
            write_byte_port(0xC00CD, gpo_out);
            usleep(1000);  //wakeup from powerdown (as opposed to idle) needs more time
            
            gpo_out |= 0x04;            //clock high (GPO2)
            write_byte_port(0xC00CD, gpo_out);
            usleep(1000);  //wakeup from powerdown (as opposed to idle) needs more time
        }
    }
    
    write_byte_port(0xC00CD, 0x0E);  //turn off GPO and sleep AVR (active low wakeups)
    write_byte_port(0xC00CC, 0x00);  //turn off GPO and sleep AVR (AVR has internal pullups for the IRQ lines)
}

uint8_t read_byte_avr_eeprom(uint32_t addr)
{
    int num, r;
    
    
    build_eeprom_read_byte_command(buf, addr);
    
    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nTimeout waiting for reply (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t data;
        uint32_t returnAddr;
        
        if(!get_read8_reply(buf, &returnAddr, &data))
        {
            printf("read_byte_avr_eeprom: get_read_reply failure\n");
            exit(-1);
        }
        return data;
    }
    
    return 0;
}

uint8_t read_byte_target(uint32_t addr, uint8_t target)
{
    int num, r;
   
    
    build_read8_command(buf, addr, target);
    
    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nTimeout waiting for reply (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint16_t data;
        uint32_t returnAddr;
        
        if(!get_read16_reply(buf, &returnAddr, &data))
        {
            printf("read_byte_target: get_read_reply failure\n");
            exit(-1);
        }
        return data;
    }
    
    return 0;
}

uint8_t read_byte_port(uint32_t addr)
{
    return read_byte_target(addr, TARGET_PORTIO);
}

uint8_t read_byte_sram(uint32_t addr)
{
    return read_byte_target(addr, TARGET_SRAM);
}

uint8_t read_byte_rom(uint32_t addr)
{
    return read_byte_target(addr, TARGET_ROM);
}

uint8_t read_byte_cart(uint32_t addr)
{
    return read_byte_target(addr, TARGET_CART);
}

uint16_t read_word_target(uint32_t addr, uint8_t target)
{
    int num, r;
    
    build_read16_command(buf, addr, target);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nTimeout waiting for reply (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint16_t data;
        uint32_t returnAddr;

        if(!get_read16_reply(buf, &returnAddr, &data))
        {
            printf("read_word: get_read_reply failure\n");
            exit(-1);
        }
        return data;
    }

    return 0;
}

uint16_t read_word_cart(uint32_t addr)
{
#if FLASH_ACCESS_DATA_BITS == 16
    return read_word_target(addr, TARGET_CART);
#elif FLASH_ACCESS_DATA_BITS == 8
    return read_byte_target(addr, TARGET_CART) | ((uint16_t)read_byte_target(addr+1, TARGET_CART) << 8);
#else
 #error How many data bits are there?
#endif
}


uint16_t read_word_rom(uint32_t addr)
{
#if FLASH_ACCESS_DATA_BITS == 16
    return read_word_target(addr, TARGET_ROM);
#elif FLASH_ACCESS_DATA_BITS == 8
    return read_byte_rom(addr) | ((uint16_t)read_byte_rom(addr+1) << 8);
#else
 #error How many data bits are there?
#endif
}

void set_address(uint32_t addr)
{
    int num, r;
    build_set_addr_command(buf, addr);

    //printf("set_address: writing 0x%04X to 0x%06X\n", data, addr);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\nset_address: libusb_bulk_transfer fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS) {
            printf("\nset_address: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}

void select_cart_slot(uint8_t data)
{
    int num, r;
    build_set_cartslot_command(buf, data);
    
    r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    
    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\n select_cart_slot: libusb_bulk_transfer fail\n");
        exit(-1);
    }
    else
    {
        uint8_t result;
        
        get_result_reply(buf, &result);
        
        if(result != MSG_RESULT_SUCCESS) {
            printf("\n select_cart_slot: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}

void set_data(uint32_t data)
{
    int num, r;
    build_set_data16_command(buf, data);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != USB_RXTX_SIZE)
    {
        printf("\n set_data: libusb_bulk_transfer fail\n");
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS) {
            printf("\n set_data: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}

/*uint8_t *read_64_bytes(uint32_t addr, uint8_t chip)
{
    int num, r;
    build_read64_command(buf, addr, chip);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // check if any Raw HID packet has arrived
    r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
    {
        print_libusb_error(r);
        exit(-1);
    }
    else if(num != 64)
    {
        printf("\nlibusb_bulk_transfer fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        return buf;
    }

    return NULL;
}
*/

uint8_t read_64xN_bytes(uint8_t *readBuf, uint32_t addr, uint8_t n, uint8_t target_chip)
{
    int num, r;
    int i;
    build_read64xN_command(buf, addr, n, target_chip);

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, buf, USB_RXTX_SIZE, &num, USB_TIMEOUT);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    for(i=0;i<n;i++)
    {
        // check if any Raw HID packet has arrived
        r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, readBuf+(64*i), USB_RXTX_SIZE, &num, USB_TIMEOUT);
        if(r<0)
        {
            print_libusb_error(r);
            exit(-1);
        }
        else if(num != 64)
        {
            printf("\nlibusb_bulk_transfer fail (addr=0x%X)\n", addr);
            exit(-1);
        }
    }

    return 1;
}




void printUsage(char *thisFile)
{
    printf("\n");
    printf("Usage: %s [<option(s)>]\n", thisFile);
    printf("\n");
    printf("  /?                Shows this help screen.\n");
    printf("  /i                Displays cartridge info.\n");
    printf("  /p                Displays cartridge protected blocks.\n");
    printf("  /c                Init WonderSwan cartridge.\n");
    printf("  /b <filename>     Backup/Dump the current cartridge slot to the file (based on detected ROM size).\n");
    printf("  /w <filename>     Write the specified file to the current cart slot.\n");
    printf("  /v <filename>     Verify that the specified file is the same as the current cart slot.\n");
    printf("  /s                Backup official cart save-game (untested).\n");
    printf("  /r <fname.ngf>    Restores save-game (untested).\n");
    printf("  /g 0xADDR         Gets the single byte (from SRAM) at addr\n");
    printf("  /dr 0xADDR        Dumps the 16-bit word (from ROM) at addr\n");
    printf("  /ds 0xADDR        Dumps the 8-bit byte (from SRAM) at addr\n");
    printf("  /dp 0xPT          Dumps the 8-bit byte (from MBC port [see http://daifukkat.su/docs/wsman/#ovr_iomap])\n");
    printf("  /lr 0xADDR 0xDADA Load single word to ROM addr (not necessarily flashing to cart)\n");
    printf("  /ls 0xADDR 0xDA   Load single byte to SRAM addr\n");
    printf("  /lp 0xPT 0xDA     Load single byte to MBC io-mapped port PT\n");
    printf("  /lg 0xDA 0xDA     Load 2 bytes to AVR (via GPO serial)\n");
    printf("  /u                Set unlock bypass write mode (faster for Flashmasta cart) [obsolete?]\n");
    printf("  /n n              Set write_N byte chunks. Use either 1-32 or a multiple of 64.\n");
    printf("  /o n              Override cart size detection to n Mbit.\n");
    printf("  /k 0|1            In write mode, skip chip 0 or 1.\n");
    printf("  /e                Erase cartridge (do nothing else).\n");
    printf("  /q                Set the current slot (via on-cart USB).\n");
    printf("  /t                Perform some tests.\n");
    printf("  /tr               Test the real-time clock.\n");
    printf("  /tf <filename>    Test writing <filename> to SD via RTC overload.\n");
    printf("  /ts n             Test setting slot n via RTC command.\n");
    printf("  /tg n             Test setting slot n via GPO command.\n");
    printf("\n");
}

uint16_t parseCmdLine(int argc, char *argv[])
{
    uint16_t options = 0;
    int a;
    // parse command line
    for (a=1; a<argc; a++)
    {
        if ((argv[a][0] == '/') || (argv[a][0] == '-'))
        {
            // option
            switch (tolower(argv[a][1]))
            {
                default:
                case '?':
                    printUsage(argv[0]);
                    exit(0);
                case 'y':
                    assumeYES = 1;
                    break;
                case 'i':
                    options |= OPTION_INFO;
                    break;
                case 'p':
                    options |= OPTION_PROTECT;
                    break;
                case 'b':
                    if(options & OPTION_WRITE)
                    {
                        printUsage(argv[0]);
                        exit(1);
                    }
                    options |= OPTION_BACKUP;
                    filename = argv[++a];
                    break;
                case 'w':
                    if(options & OPTION_BACKUP)
                    {
                        printUsage(argv[0]);
                        exit(1);
                    }
                    options |= OPTION_WRITE;
                    filename = argv[++a];
                    break;
                case 'v':
                    options |= OPTION_VERIFY;
                    verifyFilename = argv[++a];
                    break;
                case 's':
                    options |= OPTION_BACKUP_SAVEGAME;
                    break;
                case 'r':
                    options |= OPTION_RESTORE_SAVEGAME;
                    saveFilename = argv[++a];
                    break;
                case 'm':
                    options |= OPTION_MICROSD;
                    break;
                case 'd':
                {
                    uint32_t addr32;
                    
                    switch(tolower(argv[a][2]))
                    {
                        default:
                        case 'r':
                            a++;
                            sscanf(argv[a], "%X", &addr32);

                            printf("ROM word at address 0x%06X is 0x%04X\n", addr32, read_word_rom(addr32));
                            break;
                        case 'b':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            
                            printf("ROM byte at address 0x%06X is 0x%04X\n", addr32, read_byte_rom(addr32));
                            break;
                        case 's':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            
                            printf("SRAM byte at address 0x%06X is 0x%04X\n", addr32, read_byte_sram(addr32));
                            break;
                        case 'p':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            
                            if(addr32 <= 0xFF)
                                addr32 |= (addr32 & 0xF0) << 12;
                            
                            printf("PORT byte at address 0x%06X is 0x%04X\n", addr32, read_byte_port(addr32));
                            break;
                        case 'c':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            
                            if((addr32 & 0xF0000) == 0x10000)
                                printf("CART byte at address 0x%06X is 0x%04X\n", addr32, read_byte_cart(addr32));
                            else
                                printf("CART word at address 0x%06X is 0x%04X\n", addr32, read_word_cart(addr32));
                            break;
                        case 'e':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            printf("AVR EEPROM byte at address 0x%06X is 0x%02X\n", addr32, read_byte_avr_eeprom(addr32));
                            break;
                    }
                    
                    exit(0);
                    break;
                }
                case 'g':
                {
                    uint32_t addr32;
                    a++;
                    sscanf(argv[a], "%X", &addr32);

                    
                    printf("SRAM byte at address 0x%06X is 0x%02X\n", addr32, read_byte_sram(addr32));
                    exit(0);
                    break;
                }
                case 'l':
                {
                    uint32_t addr32, data;

                    switch(tolower(argv[a][2]))
                    {
                        default:
                        case 'r':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            a++;
                            sscanf(argv[a], "%X", &data);
                            
                            printf("Loading 0x%04X to ROM address 0x%06X\n", data, addr32);
                            write_word_rom(addr32, data);
                            break;
                        case 's':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            a++;
                            sscanf(argv[a], "%X", &data);
                            
                            printf("Loading 0x%02X to SRAM address 0x%06X\n", data, addr32);
                            write_byte_sram(addr32, data);
                            break;
                        case 'p':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            a++;
                            sscanf(argv[a], "%X", &data);
                            
                            if(addr32 <= 0xFF)
                                addr32 |= (addr32 & 0xF0) << 12;

                            printf("Loading 0x%02X to PORT address 0x%06X\n", data, addr32);
                            write_byte_port(addr32, data);
                            break;
                        case 'g':
                            a++;
                            sscanf(argv[a], "%X", &addr32);
                            a++;
                            sscanf(argv[a], "%X", &data);
                            
                            if(addr32 <=0xFF && data <= 0xFF)
                            {
                                uint8_t send_buf[2];
                                send_buf[0] = addr32;
                                send_buf[1] = data;
                            
                                printf("Loading 0x%02X 0x%02X to AVR\n", send_buf[0], send_buf[1]);
                                write_bytes_to_avr_via_gpo(2, send_buf);
                            }
                            break;
                    }
                    exit(0);
                    break;
                }
                case 'u':
                    unlockBypassMode = 1;
                    break;
                case 'n':
                    a++;
                    sscanf(argv[a], "%d", &write_N);
                    break;
                case 'o':
                    a++;
                    sscanf(argv[a], "%d", &cart_size_override);
                    break;
                case 'k':
                    a++;
                    sscanf(argv[a], "%d", &skipChip);
                    break;
                case 'c':
                    options |= OPTION_INITWSMMC;
                    break;
                case 't':
                    switch(tolower(argv[a][2]))
                    {
                        uint32_t data, data2;
                        case 'r':
                            a++;
                            sscanf(argv[a], "%x", &data);
                            a++;
                            sscanf(argv[a], "%x", &data2);
                            ws_rtc_test(data, data2);
                            exit(0);
                        case 'f':
                            {
                                if(a+1 < argc)
                                {
                                    a++;
                                    ws_rtc_file_test(argv[a]);
                                }
                                exit(0);
                            }
                        case 's':
                            a++;
                            sscanf(argv[a], "%d", &data);
                            if(data <= 0xFF)
                            {
                                printf("Selecting cartridge slot %d (via RTC)\n", data);
                                ws_rtc_select_cart_slot(data);
                            }
                            exit(0);
                        case 'g':
                            a++;
                            sscanf(argv[a], "%d", &data);
                            if(data <= 0xFF)
                            {
                                printf("Selecting cartridge slot %d (via GPO)\n", data);
                                ws_gpo_select_cart_slot(data);
                            }
                            exit(0);
                        default:
                            break;
                    }
                    options |= OPTION_TEST;
                    break;
                case 'e':
                    erase_cart();
                    exit(0);
                case 'q':
                    a++;
                    uint32_t data;
                    sscanf(argv[a], "%d", &data);
                    if(data <= 0xFF)
                    {
                        printf("Selecting cartridge slot %d\n", data);
                        select_cart_slot(data);
                    }
                    //exit(1);

            }
        }
    }
    return options;
}

void erase_cart()
{
    uint16_t word1, word2;
    uint8_t toggle, done = 0;

    write_word_rom(ws1_addr, 0xAA);
    write_word_rom(ws2_addr, 0x55);
    write_word_rom(ws3_addr, 0x80);
    write_word_rom(ws1_addr, 0xAA);
    write_word_rom(ws2_addr, 0x55);
    write_word_rom(ws3_addr, 0x10);

    printf("Waiting for cart to be erased.\n");
    fflush(stdout);

    do{
        word1 = read_byte_rom(0);
        word2 = read_byte_rom(0);
        
        if((word1 & 0x04) == (word2 & 0x04))
        {
            toggle = 0;
            done = 1;
        }
        else
        {
            toggle = 1;
        }
    
        printf("\rErase Operation In Progress: 0x%04X, 0x%04X", word1, word2);
        fflush(stdout);

        if(toggle)
        {
            if(word2 & 0x20)//DQ5 == 1
            {
                //printf("\nToggle with DQ5 = 1\n");
                
                word1 = read_byte_rom(0);
                word2 = read_byte_rom(0);
                if((word1 & 0x04) == (word2 & 0x04))
                {
                    //printf("\nNo toggle.  Done.\n");
                    done = 1;
                }
                else
                {
                    printf("\nErase operation not complete!!!\n");
                }
            }
        }
    } while (!done);

    printf("\n");
    fflush(stdout);
}


void erase_slot()
{
    uint8_t byte1, byte2;
    uint8_t toggle, done = 0;
    
    if(g_numAddrLinesPerSlot == 0xFF)
    {
        printf("Invalid slot size!\n");
        return;
    }
    
    uint32_t block_size = 0x020000;
    uint32_t slot_size = 1 << g_numAddrLinesPerSlot;
    uint32_t num_blocks_per_slot = slot_size / block_size;
    uint32_t curr_block_address = 0;

    printf("Waiting for slot to be erased.\n");
    printf("Block Size = %d / Slot Size = %d / Blocks Per Slot = %d\n", block_size, slot_size, num_blocks_per_slot);
    fflush(stdout);
    
    for(int i=0; i<num_blocks_per_slot; i++)
    {
        write_byte_rom(ws1_addr, 0xAA);
        write_byte_rom(ws2_addr, 0x55);
        write_byte_rom(ws3_addr, 0x80);
        write_byte_rom(ws1_addr, 0xAA);
        write_byte_rom(ws2_addr, 0x55);
    
        curr_block_address = i * block_size;
        write_byte_rom(curr_block_address, 0x30);  //erase block
        
        
        do{
            byte1 = read_byte_rom(curr_block_address);
            byte2 = read_byte_rom(curr_block_address);
            
            if((byte1 & 0x40) == (byte2 & 0x40))
            {
                toggle = 0;
                done = 1;
            }
            else
            {
                toggle = 1;
            }
            
            printf("\rErase Operation In Progress (0x%08X): 0x%02X, 0x%02X", curr_block_address, byte1, byte2);
            fflush(stdout);
            
            if(toggle)
            {
                if(byte2 & 0x20)//DQ5 == 1
                {
                    //printf("\nToggle with DQ5 = 1\n");
                    
                    byte1 = read_byte_rom(curr_block_address);
                    byte2 = read_byte_rom(curr_block_address);
                    if((byte1 & 0x40) == (byte2 & 0x40))
                    {
                        //printf("\nNo toggle.  Done.\n");
                        done = 1;
                    }
                    else
                    {
                        printf("\nErase operation not complete!!!\n");
                    }
                }
            }
        } while (!done);
        
        while((byte1 = read_byte_rom(curr_block_address)) != 0xFF)
        {
            //printf("\rErase Operation In Progress: 0x%02X", byte1);
        }
    }
    
    printf("\n");
    fflush(stdout);
}

int write_file_to_cart_rom(char *filename)
{
    FILE *f = fopen(filename, "rb");
    int num=0, total=0;
    uint32_t addr;
    uint16_t dataBuf[32];
    time_t startTime;

    
    fseek(f, 0L, SEEK_END);
    uint32_t file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    


    printf("\n");

    if (!f)
    {
        printf("File not found!\n");
        fflush(stdout);
        return 0;
    }
    
    addr=0;
    
    if(g_numAddrLinesPerSlot == 0xFF)
    {
        printf("Invalid slot size!\n");
        fclose(f);
        return 0;
    }
    
    uint32_t slot_size = 1 << g_numAddrLinesPerSlot;

    if(file_size > slot_size)
    {
        printf("File \"%s\" is too large for this slot!\n", filename);
        fclose(f);
        return 0;
    }
    
    uint32_t percent = (slot_size-0x100)/100;
    addr = slot_size - file_size;
    
    
    printf("%s = 0x%X bytes\n", filename, file_size);

    startTime = time(NULL);
    //erase chips  FLASHALLERS

    printf("Erase slot?\n");
    if (assumeYES || tolower(getchar()) == 'y')
        erase_slot();


    if(unlockBypassMode && write_N == 1)
    {
        printf("No verification in turbo mode!  Consider using verify mode after writing.\n");
        fflush(stdout);

        while(addr<slot_size)
        {
            if(addr==0)
            {
                write_word_rom(ws1_addr, 0xAA);
                write_word_rom(ws2_addr, 0x55);
                write_word_rom(ws3_addr, 0x20);
            }

            num = fread(dataBuf, 2, 1, f);
            if(num != 1)
                break;

            printf("\rWriting addr 0x%06X  ", addr);
            fflush(stdout);

            write_word_rom(0, 0xA0);
            write_word_rom(addr, dataBuf[0]);

            total+=1;
            addr+=1;
        }

        write_word_rom(0, 0x90);
        write_word_rom(0, 0x00);

        printf("\n");
        fflush(stdout);
    }
    else if((write_N % 64) == 0)
    {
        uint8_t *dataBuf64xN;

        dataBuf64xN = malloc(write_N);
        if(!dataBuf64xN)
        {
            printf("write_file_to_cart: malloc of %d bytes failed!\n", write_N);
            exit(1);
        }

        while (addr < slot_size)
        {
            num = fread(dataBuf64xN, 1, write_N, f);
            printf("\rflash_write_64xN_bytes: addr=0x%06X (%d%%)", addr, addr/percent);

            if(num > 0)
            {
                uint8_t allFF = 1;
                for(int i=0; i< num; i++)
                {
                    if(dataBuf64xN[i] != 0xFF)
                    {
                        allFF = 0;
                        break;
                    }
                }
                if(!allFF)
                {
                    printf("    writing=%d            ", write_N);
                    flash_write_64xN_bytes(addr, dataBuf64xN, write_N/64);
                    printf("    read=%d write_N=%d    ", num, write_N);
                }
                else
                {
                    printf("    skipped writing %d    ", num);
                }
                fflush(stdout);
            }

            total+=num;
            if(num != write_N)
                break;
            addr+=write_N;
        }
        printf("\n");
        fflush(stdout);

        free(dataBuf64xN);
    }
    else if(write_N == 32)
    {
        while (addr < slot_size)
        {
            num = fread(dataBuf, 1, 32, f);
            printf("\rflash_write_32_bytes: addr=0x%06X", addr);
            fflush(stdout);

            flash_write_32_bytes(addr, (uint8_t *)dataBuf);
            total+=num;
            if(num != 32)
                break;
            addr+=32;
        }
        printf("\n");
        fflush(stdout);
    }
    else if(write_N > 1 && write_N < 32)
    {
        uint8_t n = 1;

        while (addr < slot_size)
        {
            num = fread(dataBuf, 1, write_N, f);
            printf("\rflash_write_N_bytes: addr=0x%06X n=%d", addr, write_N);
            fflush(stdout);

            flash_write_N_bytes(addr, (uint8_t *)dataBuf, write_N);
            total+=num;
            if(num != write_N)
                break;
            addr+=n;
        }
        printf("\n");
        fflush(stdout);
    }
    else if(write_N == 1)
    {
        printf("write n = 1\n");
        while(addr<slot_size)
        {
            int wait;

            num = fread(dataBuf, 2, 1, f);
            if(num != 1)
                break;

            printf("\rWriting addr 0x%06X  ", addr);
            fflush(stdout);

            if(dataBuf[0] != 0xFFFF)
            {
                write_word_rom(ws1_addr, 0xAA);
                write_word_rom(ws2_addr, 0x55);
                write_word_rom(ws3_addr, 0xA0);
                write_word_rom(addr, dataBuf[0]);

                //make sure the data has been properly written
                wait = 10000;
                while(dataBuf[0] != read_word_rom(addr))
                {
                    wait--;
                    if(wait == 0)
                    {
                        printf("\nWriting addr 0x%06X failed!  (wrote 0x%04X != read 0x%04X) Retrying.\n", addr, dataBuf[0], read_word_rom(addr));
                        write_word_rom(ws1_addr, 0xAA);
                        write_word_rom(ws2_addr, 0x55);
                        write_word_rom(ws3_addr, 0xA0);
                        write_word_rom(addr, dataBuf[0]);

                        //make sure the data has been properly written
                        uint32_t wait2 = 10000;
                        while(dataBuf[0] != read_word_rom(addr))
                        {
                            wait2--;
                            if(wait2 == 0)
                            {
                                printf("\nWriting addr 0x%06X failed!  (wrote 0x%04X != read 0x%04X) Exiting.\n", addr, dataBuf[0], read_word_rom(addr));
                                exit(1);
                            }
                        }
                    }
                }
            }

            total+=2;
            addr+=2;
        }
        printf("\n");
        fflush(stdout);
    }
    else
    {
        printf("\nERROR write_N=%d should be from 1 to 32 or a multiple of 64\n", write_N);
        fclose(f);
        exit(1);
    }
    fclose(f);
    printf("Erased cart and wrote %d bytes in %ld seconds\n", total, time(NULL)-startTime);
    fflush(stdout);
    return total;
}

int verify_file_with_cart_slot(char *filename)
{
    FILE *f = fopen(filename, "rb");
    int itemsRead=0;
    time_t startTime;
    uint32_t addr=0;
	uint32_t retries = 0;
    
    fseek(f, 0L, SEEK_END);
    uint32_t file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    //use slot size
    uint32_t start_addr = (1 << g_numAddrLinesPerSlot) - file_size;

    addr=start_addr;

    if (!f)
    {
        printf("File not found! (%s)\n", filename);
        return 0;
    }

    startTime = time(NULL);

    printf("Verifying...");
    fflush(stdout);

    if((write_N % 64) != 0)
    {
        uint16_t cartWord, fileWord;
        do
        {
            if(retries)
                printf("\rVerifying address 0x%06X (%05d retries)", addr, retries);
            else
                printf("\rVerifying address 0x%06X", addr);
            fflush(stdout);

            itemsRead = fread(&fileWord, 2, 1, f);
            if(itemsRead != 1)
            {
                printf("\nverify: Success!  End of file reached at addr 0x%06X\n", addr);
                printf("  Verified %d bytes in %ld seconds\n", addr, time(NULL)-startTime);
                if(retries)
                    printf("  %d total retries\n", retries);

                fclose(f);
                return 1;
            }

            cartWord = read_word_rom(addr);

            if(cartWord != fileWord)
            {
                int i;
                for(i=0;i<16 && cartWord != fileWord;i++)
                {
                    retries++;
                    printf("\r Retrying address 0x%06X (%05d retries)", addr, retries);
                    cartWord = read_word_rom(addr);
                }
            }


            if(cartWord == fileWord)
            {
                addr+=2;
            }
        } while(cartWord == fileWord);
        
        printf("\nverify: Failed!  Mismatch detected at addr 0x%06X\n", addr);
        printf("  Cart Data = 0x%04X / File Data = 0x%04X\n (%lds elapsed)\n (%d total retries)", cartWord, fileWord, time(NULL)-startTime, retries);
    }
    else
    {
        
        uint8_t verified = 0;
//        uint8_t cart_buf[8192];
//        uint8_t file_buf[8192];
        
        uint8_t *cart_buf;
        cart_buf = malloc(write_N);
        if(!cart_buf)
        {
            printf("\nMemory allocation error!\n");
            return 0;
        }

        uint8_t *file_buf;
        file_buf = malloc(write_N);
        if(!file_buf)
        {
            printf("\nMemory allocation error!\n");
            free(cart_buf);
            return 0;
        }
        
        
        do
        {
            if(retries)
                printf("\rVerifying address 0x%06X (%05d retries)", addr, retries);
            else
                printf("\rVerifying address 0x%06X", addr);
            fflush(stdout);
            
            itemsRead = fread(file_buf, 1, write_N, f);
            if(itemsRead == 0)
            {
                printf("\nverify: Success!  End of file reached at addr 0x%06X\n", addr);
                printf("  Verified %d bytes in %ld seconds\n", addr, time(NULL)-startTime);
                if(retries)
                    printf("  %d total retries\n", retries);
                
                fclose(f);
                free(cart_buf);
                free(file_buf);
                return 1;
            }

            if((itemsRead % 64) != 0)
            {
                read_64xN_bytes(cart_buf, addr, (itemsRead / 64) + 1, TARGET_ROM);
            }
            else
            {
                read_64xN_bytes(cart_buf, addr, itemsRead / 64, TARGET_ROM);
            }
            
            verified = (memcmp(cart_buf, file_buf, itemsRead) == 0);
            
            if(verified)
            {
                addr+=itemsRead;
            }

        } while(verified);
      
        printf("\nverify: Failed!  Mismatch detected near addr 0x%06X\n", addr);
        printf("  read %d items (write_N = %d) (%d retries)\n", itemsRead, write_N, retries);
        printf("  (%lds elapsed)\n",time(NULL)-startTime);
        printf("  Cart Data = 0x%02X / File Data = 0x%02X\n", cart_buf[0], file_buf[0]);
        printf("  Cart Data = 0x%02X / File Data = 0x%02X\n", cart_buf[1], file_buf[1]);
        printf("  Cart Data = 0x%02X / File Data = 0x%02X\n", cart_buf[2], file_buf[2]);
        printf("  Cart Data = 0x%02X / File Data = 0x%02X\n", cart_buf[3], file_buf[3]);
        printf("  Cart Data = 0x%02X / File Data = 0x%02X\n", cart_buf[4], file_buf[4]);
        
        free(cart_buf);
        free(file_buf);
    }
    

    fclose(f);
    return 0;
}


int backup_cart_slot_rom_to_file(char *bakFile, uint32_t size)
{
    uint32_t addr;
    uint32_t percent = (size-0x100)/100;
    time_t sTime = time(NULL);
	uint8_t *readBuf;
    
    //use slot size
    uint32_t start_addr = (1 << g_numAddrLinesPerSlot) - size;

    FILE *f = fopen(bakFile, "rb");
    if (f)
    {
        fclose(f);
        printf("Overwrite file with backup?\n");
        if (assumeYES || tolower(getchar()) != 'y') return 0;
    }
    f = fopen(bakFile, "wb");
    if (!f) { printf("Can't open file!\n"); return -1; }

	readBuf = malloc(size);
	if(!readBuf)
		return -1;

	int chunkSize = 32;

    // backup
    for (addr=start_addr; addr<(start_addr+size); addr+=(64*chunkSize))
    {
        printf("\rBacking up... 0x%06X (%d%%)", addr-start_addr, (addr-start_addr)/percent);
        fflush(stdout);

		read_64xN_bytes(readBuf, addr, chunkSize, TARGET_ROM);//64bytes better not go over a block boundary
		fwrite(readBuf, chunkSize, 64, f);

		//temp_word = read_word_rom(addr);
        //fwrite(&temp_word, 1, 2, f);
    }
    printf("\r(%d%%) Backed up %d bytes in %ld seconds             \n", addr/percent, addr, time(NULL)-sTime);
    fflush(stdout);

    fclose(f);
	free(readBuf);
    return 0;
}

int backup_cart_sram_to_file(char *bakFile, uint32_t size)
{
    uint32_t addr;
    uint32_t percent = (size-0x100)/100;
    time_t sTime = time(NULL);
    uint8_t *readBuf;
    
    FILE *f = fopen(bakFile, "rb");
    if (f)
    {
        fclose(f);
        printf("Overwrite file with backup?\n");
        if (assumeYES || tolower(getchar()) != 'y') return 0;
    }
    f = fopen(bakFile, "wb");
    if (!f) { printf("Can't open file!\n"); return -1; }
    
    readBuf = malloc(size);
    if(!readBuf)
        return -1;
    
    int chunkSize = 32;
    
    // backup
    for (addr=0; addr<size; addr+=(64*chunkSize))
    {
        printf("\rBacking up... 0x%06X (%d%%)", addr, addr/percent);
        fflush(stdout);
        
        read_64xN_bytes(readBuf, addr, chunkSize, TARGET_SRAM);//64bytes better not go over a block boundary
        fwrite(readBuf, chunkSize, 64, f);
    }
    printf("\r(%d%%) Backed up %d bytes in %ld seconds             \n", addr/percent, addr, time(NULL)-sTime);
    fflush(stdout);
    
    fclose(f);
    free(readBuf);
    return 0;
}

int write_file_to_cart_sram(char *filename)
{
    FILE *f = fopen(filename, "rb");
    int num=0, total=0;
    uint32_t addr;
    time_t startTime = time(NULL);;
    
    
    fseek(f, 0L, SEEK_END);
    uint32_t file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    uint32_t percent = (file_size-0x100)/100;
    
    
    printf("\n");
    
    if (!f)
    {
        printf("File not found!\n");
        fflush(stdout);
        return 0;
    }
    
    printf("%s = 0x%X bytes\n", filename, file_size);

        uint8_t *dataBuf64xN;
        
        dataBuf64xN = malloc(write_N);
        if(!dataBuf64xN)
        {
            printf("write_file_to_cart: malloc of %d bytes failed!\n", write_N);
            exit(1);
        }
    
        addr=0;
        while (addr < file_size)
        {
            num = fread(dataBuf64xN, 1, write_N, f);
            printf("\rsram_write_64xN_bytes: addr=0x%06X (%d%%)", addr, addr/percent);
            
            if(num > 0)
            {
                sram_write_64xN_bytes(addr, dataBuf64xN, write_N/64);
                printf("    read=%d write_N=%d    ", num, write_N);
                fflush(stdout);
            }
            
            total+=num;
            if(num != write_N)
                break;
            addr+=write_N;
        }
        printf("\n");
        fflush(stdout);
        
        free(dataBuf64xN);
    
    fclose(f);
    printf("Wrote %d bytes in %ld seconds\n", total, time(NULL)-startTime);
    fflush(stdout);
    return total;
}

/*
//write all the unprotected blocks out to a file
void backupCartSaveGame()
{
	//find the unprotected blocks and write them to the .NGF file
	int i;
	FILE *ngfFile=NULL;
	char ngfFilename[255];

	int bytes;
	struct NGFheaderStruct NGFheader;
	struct blockStruct block;




	NGFheader.version = 0x53;
	NGFheader.numBlocks = 0;
	NGFheader.fileLen = sizeof(struct NGFheaderStruct);
	printf("version=0x%X blocks=0x%X len=0x%X\n", NGFheader.version, NGFheader.numBlocks, NGFheader.fileLen);
        fflush(stdout);
	//add them all up, first
	for(i=0;i<35;i++)
	{
		if(prot[0][i]==0)//is unprotected?
		{
			NGFheader.numBlocks++;
			NGFheader.fileLen += blockSize(i);
		}
		if(prot[1][i]==0)//is unprotected?
		{
			NGFheader.numBlocks++;
			NGFheader.fileLen += blockSize(i);
		}
	}
	printf("version=0x%X blocks=0x%X len=0x%X\n", NGFheader.version, NGFheader.numBlocks, NGFheader.fileLen);
        fflush(stdout);

	if(NGFheader.fileLen == 0 || NGFheader.numBlocks == 0)
	{
		fprintf(stderr, "No unprotected blocks to save\n");
		return;
	}

    sprintf(ngfFilename, "%s-%d.ngf", cartRomName, time(NULL));
    printf("Backing up save-game to \"%s\"\n", ngfFilename);
    fflush(stdout);

	ngfFile = fopen(ngfFilename, "wb");
	if(!ngfFile)
	{
		fprintf(stderr, "Couldn't open %s file\n", ngfFilename);
		return;
	}

	NGFheader.fileLen += NGFheader.numBlocks * sizeof(struct blockStruct);

	bytes = fwrite(&NGFheader, 1, sizeof(struct NGFheaderStruct), ngfFile);
	if(bytes != sizeof(struct NGFheaderStruct))
	{
		fprintf(stderr, "backupCartSaveGame: wrote %d bytes, but exptected %d bytes\n", bytes, sizeof(struct NGFheaderStruct));
		fclose(ngfFile);
		return;
	}

	for(i=0;i<35;i++)
	{
		if(prot[0][i]==0)//writable block
		{

			block.NGPCaddr = blockNumToAddr(0, i) + 0x200000;//where the NGPC sees this data
			block.len = blockSize(i);

            printf("Backing up save-game from 1st chip block=%d addr=0x%08X len=0x%X\n", i, block.NGPCaddr-0x200000, block.len);
            fflush(stdout);

			bytes = fwrite(&block, 1, sizeof(struct blockStruct), ngfFile);
			if(bytes != sizeof(struct blockStruct))
			{
				fprintf(stderr, "save-game: wrote %d bytes, but exptected %d bytes\n", bytes, sizeof(struct blockStruct));
				fclose(ngfFile);
				return;
			}

            int toWrite = blockSize(i);
            uint32_t addr;
            bytes = 0;
            for (addr=blockNumToAddr(0, i); addr<(blockNumToAddr(0, i)+blockSize(i)); addr+=64)
            {
                uint8_t readBuf[64];
                int j;

                read_64xN_bytes(readBuf, addr&0x1FFFFF, 0, 1);//64bytes better not go over a block boundary

                if(toWrite<64)
                    bytes += fwrite(readBuf, 1, toWrite, ngfFile);
                else
                    bytes += fwrite(readBuf, 1, 64, ngfFile);
                toWrite-=64;
            }

			if(bytes != blockSize(i))
			{
				fprintf(stderr, "save-game: wrote %d bytes, but exptected %d bytes\n", bytes, blockSize(i));
				fclose(ngfFile);
				return;
			}
		}
	}

	if(chipSize[1])  //do the second chip, also
	{
		for(i=0;i<35;i++)
		{
			if(prot[1][i]==0)//writable block
			{
				block.NGPCaddr = blockNumToAddr(1, i) + 0x600000;//where the NGPC sees this data
				block.len = blockSize(i);

                printf("Backing up save-game from 2nd chip block=%d addr=0x%08X len=0x%X\n", i, block.NGPCaddr, block.len);
                fflush(stdout);

				bytes = fwrite(&block, 1, sizeof(struct blockStruct), ngfFile);
				if(bytes != sizeof(struct blockStruct))
				{
					fprintf(stderr, "save-game: wrote %d bytes, but exptected %d bytes\n", bytes, sizeof(struct blockStruct));
					fclose(ngfFile);
					return;
				}

                int toWrite = blockSize(i);
                uint32_t addr;
                bytes = 0;
                for (addr=blockNumToAddr(1, i); addr<(blockNumToAddr(1, i)+blockSize(i)); addr+=64)
                {
                    uint8_t readBuf[64];

                    read_64xN_bytes(readBuf, addr&0x1FFFFF, 1, 1);//64bytes better not go over a block boundary

                    if(toWrite<64)
                        bytes += fwrite(readBuf, 1, toWrite, ngfFile);
                    else
                        bytes += fwrite(readBuf, 1, 64, ngfFile);
                    toWrite-=64;
                }

                if(bytes != blockSize(i))
                {
                    fprintf(stderr, "save-game: wrote %d bytes, but exptected %d bytes\n", bytes, blockSize(i));
                    fclose(ngfFile);
                    return;
                }
			}
		}
	}

	fclose(ngfFile);
}


void restoreSaveGameToCart(char *ngfFilename)
{
	//find the NGF file and read it in
	FILE *ngfFile;
	int bytes, i;
	uint8_t *blocks;
	void *blockMem;
	struct NGFheaderStruct NGFheader;
	struct blockStruct *blockHeader;
	uint32_t totalWritten = 0;
	time_t startTime;
	uint8_t erasedBlocks[2][35];

	ngfFile = fopen(ngfFilename, "rb");
	if(!ngfFile)
	{
		printf("restoreSaveGameToCart: Couldn't open %s file\n", ngfFilename);
		return;
	}

	bytes = fread(&NGFheader, 1, sizeof(struct NGFheaderStruct), ngfFile);

	if(bytes != sizeof(struct NGFheaderStruct))
	{
		printf("restoreSaveGameToCart: Bad NGF file %s\n", ngfFilename);
		fclose(ngfFile);
		return;
	}


	if(NGFheader.version != 0x53)
	{
		printf("restoreSaveGameToCart: Bad NGF file version %s 0x%X\n", ngfFilename, NGFheader.version);
		fclose(ngfFile);
		return;
	}

    blockMem = (uint8_t *) malloc(NGFheader.fileLen - sizeof(struct NGFheaderStruct));
    //error handling?
    if(!blockMem)
    {
		printf("restoreSaveGameToCart: can't malloc %d bytes\n", (NGFheader.fileLen - sizeof(struct NGFheaderStruct)));
        return;
    }


	blocks = (unsigned char *)blockMem;

	bytes = fread(blocks, 1, NGFheader.fileLen - sizeof(struct NGFheaderStruct), ngfFile);
	fclose(ngfFile);

	if(bytes != (NGFheader.fileLen - sizeof(struct NGFheaderStruct)))
	{
		printf("restoreSaveGameToCart: read 0x%X bytes, but exptected 0x%X bytes\n", bytes, (NGFheader.fileLen - sizeof(struct NGFheaderStruct)));
		free(blockMem);
		return;
	}

    if(NGFheader.numBlocks > 35)
    {
		printf("restoreSaveGameToCart: numBlocks=%d overflow\n", NGFheader.numBlocks);
		free(blockMem);
		return;
    }

    memset(erasedBlocks, 0, 35*2);

    startTime = time(NULL);
	//loop through the blocks and insert them into mainrom
	for(i=0; i < NGFheader.numBlocks; i++)
	{
	    blockHeader = (struct blockStruct*) blocks;
		blocks += sizeof(struct blockStruct);

        if(!((blockHeader->NGPCaddr >= 0x200000 && blockHeader->NGPCaddr < 0x400000)
             ||
             (blockHeader->NGPCaddr >= 0x800000 && blockHeader->NGPCaddr < 0xA00000) ))
        {
            printf("restoreSaveGameToCart: invalid blockHeader->NGPCaddr=0x%08X\n", blockHeader->NGPCaddr);
            free(blockMem);
            return;
        }
		if(blockHeader->NGPCaddr >= 0x800000)
		{
			blockHeader->NGPCaddr -= 0x600000;
		}
		else if(blockHeader->NGPCaddr >= 0x200000)
		{
			blockHeader->NGPCaddr -= 0x200000;
		}

        uint8_t chip = (blockHeader->NGPCaddr<0x200000)?0:1;

        uint8_t block = addrToBlockNum(blockHeader->NGPCaddr);
        if(erasedBlocks[chip][block]==0)
        {
            printf("Erasing block %d\n", block);
            //erase the block with this addr
            write_byte(ws1_addr, 0xAA, chip);
            write_byte(ws2_addr, 0x55, chip);
            write_byte(ws3_addr, 0x80, chip);
            write_byte(ws1_addr, 0xAA, chip);
            write_byte(ws2_addr, 0x55, chip);
            write_byte(blockHeader->NGPCaddr&0x1FFFFF, 0x30, chip);
            erasedBlocks[chip][block]=1;
        }

        while(read_byte(blockHeader->NGPCaddr&0x1FFFFF, chip) != 0xFF)
        {
            //wait for the block erase to complete
        }

        printf("Write 0x%X bytes to addr 0x%X of chip %d\n", blockHeader->len, blockHeader->NGPCaddr&0x1FFFFF, chip);
        uint32_t addr, count=0;
        uint32_t percent = (blockHeader->len)/100;
        if(blockHeader->len % 32)
            printf("mod 32 problem\n");
        for(addr = blockHeader->NGPCaddr; addr < (blockHeader->NGPCaddr + blockHeader->len); addr+=32)
        {

            printf("\rRestoring savegame... 0x%06X (%d%%)", addr, count/percent);
            fflush(stdout);

            flash_write_32_bytes(addr&0x1FFFFF, blocks+count, chip);
            count+=32;
        }
        printf("\rRestoring savegame... 0x%06X (%d%%)\n", addr-1, 100);
        fflush(stdout);

		blocks += blockHeader->len;
		totalWritten += blockHeader->len;

        read_byte(0, chip);//make sure we're back to readable state
	}

	printf("Wrote 0x%X bytes in %d seconds.", totalWritten, time(NULL)-startTime);
        fflush(stdout);


	free(blockMem);
}

*/

void init_wonderswan_MMC()
{
    uint16_t word;
    uint32_t addr;
    printf("init_wonderswan_MMC\n");

    set_unset_WS_lines(LINE_PORTIO | LINE_CARTSEL, LINE_CLK | LINE_RESET);
    set_address(0x5000A);

//the current cart has the reset line ONLY going to the WS MMC mem controller chip
//    keep that chip in the reset state!
//    set_unset_WS_lines(LINE_RESET, 0);

    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);//2
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);//3
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);//4
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);//5
    set_unset_WS_lines(LINE_CLK, 0);

    set_address(0xA0005);

    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);

    set_unset_WS_lines(LINE_CLK, 0);//10
        set_unset_WS_lines(0, LINE_CLK);//1
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//4
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//5
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//6
    set_unset_WS_lines(LINE_CLK, 0);//20
        set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);//30
        set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);//40
        set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);//50
        set_unset_WS_lines(0, LINE_CLK);//1
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
    set_unset_WS_lines(LINE_CLK, 0);//60
        set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);//70
        set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);//80
        set_unset_WS_lines(0, LINE_CLK);//1
    set_unset_WS_lines(LINE_CLK, 0);//90
        set_unset_WS_lines(0, LINE_CLK);//1
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
    set_unset_WS_lines(LINE_CLK, 0);//80
        set_unset_WS_lines(0, LINE_CLK);//1
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//2
        set_unset_WS_lines(LINE_CLK, 0);
        set_unset_WS_lines(0, LINE_CLK);//3

    set_unset_WS_lines(LINE_CLK, LINE_PORTIO | LINE_CARTSEL);
    write_byte_port(0xC00C2, 0x00AA);
    write_byte_port(0xC00C3, 0x0055);
    set_unset_WS_lines(LINE_PORTIO | LINE_CARTSEL, LINE_CLK);

    set_unset_WS_lines(LINE_CLK, LINE_PORTIO | LINE_CARTSEL);
    word = read_byte_port(0xC00C2);
    if((word&0xFF) != 0xAA)
        printf("problem 1 read 0x%04X\n", word);
    word = read_byte_port(0xC00C3);
    if((word&0xFF) != 0x55)
        printf("problem 2 read 0x%04X\n", word);
    set_unset_WS_lines(LINE_PORTIO | LINE_CARTSEL, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);


    set_unset_WS_lines(LINE_CLK, LINE_PORTIO | LINE_CARTSEL);
    write_byte_port(0xC00C2, 0x00FF);
    write_byte_port(0xC00C3, 0x00FF);
    set_unset_WS_lines(LINE_PORTIO | LINE_CARTSEL, LINE_CLK);

    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);


    addr = 0x3FFF0;
    set_address(addr);
    set_data(0xFFFF);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, LINE_CARTSEL);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    printf("addr 0x%X = %04X\n", addr, read_word_cart(addr));
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(0, LINE_CLK);
    set_unset_WS_lines(LINE_CLK, 0);
    set_unset_WS_lines(LINE_CARTSEL, LINE_CLK);
}


/*
 C2 C1 C0 Description
 0  0  0  Reset (00 (year), 01 (month), 01 (day), 0 (day of week),
 00 (minute), 00 (second)) (*1)
 0  0  1  Status register access
 0  1  0  Real-time data access 1 (year data to)
 0  1  1  Real-time data access 2 (hour data to)
 1  0  0  Alarm time/frequency duty setting 1
 1  0  1  Alarm time/frequency duty setting 2
 1  1  0  Test mode start (*2)
 1  1  1  Test mode end (*2)
 (*1) Don't care the R/W bit of this command.
 (*2) This command is access-disabled due to specific use for the IC test.
 */
#define WS_RTC_FLAG_READY               0x80
#define WS_RTC_FLAG_CMD_IN_PROGRESS     0x10

#define WS_RTC_READ_BIT                 0x01
#define WS_RTC_CMD_SET_DONE             0x00
#define WS_RTC_CMD_SET_RESET            0x10  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b000 << 1))
#define WS_RTC_CMD_SET_STATUS_REG       0x12  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b001 << 1))
#define WS_RTC_CMD_GET_STATUS_REG       0x13  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b001 << 1) | WS_RTC_READ_BIT)
#define WS_RTC_CMD_SET_DATE_AND_TIME    0x14  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b010 << 1))
#define WS_RTC_CMD_GET_DATE_AND_TIME    0x15  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b010 << 1) | WS_RTC_READ_BIT)
#define WS_RTC_CMD_SET_TIME_OF_DAY      0x16  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b011 << 1))
#define WS_RTC_CMD_GET_TIME_OF_DAY      0x17  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b011 << 1) | WS_RTC_READ_BIT)
#define WS_RTC_CMD_SET_ALARM_1          0x18  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b100 << 1))
#define WS_RTC_CMD_GET_ALARM_1          0x19  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b100 << 1) | WS_RTC_READ_BIT)
#define WS_RTC_CMD_SET_ALARM_2          0x1A  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b101 << 1))
#define WS_RTC_CMD_GET_ALARM_2          0x1B  //(WS_RTC_FLAG_CMD_IN_PROGRESS | (0b101 << 1) | WS_RTC_READ_BIT)

//these should be above 0x99 (start at 0xA0)
#define WS_RTC_AVR_CMD_SET_SLOT         0xA0  //to set the slot, send the WS_RTC_AVR_CMD_GET_SLOT command via WS_RTC_CMD_SET_DATE_AND_TIME (year = WS_RTC_AVR_CMD_SET_SLOT)
#define WS_RTC_AVR_CMD_GET_SLOT         0xA1  //to get the slot, you must first send the WS_RTC_AVR_CMD_GET_SLOT command via WS_RTC_CMD_SET_DATE_AND_TIME, then use WS_RTC_CMD_GET_DATE_AND_TIME (year will be slot)


void ws_strobe_cart_clock(void)
{
    //usleep(100);  //plenty of delay in the USB calls
    set_unset_WS_lines(LINE_CLK, 0);
    //usleep(100);
    set_unset_WS_lines(0, LINE_CLK);
}

void ws_strobe_cart_clock_times(uint8_t n)
{
    for(int i=0; i<n;i++)
    {
        ws_strobe_cart_clock();
    }
}


uint8_t ws_rtc_wait_for_ready(void)
{
    uint8_t ca = read_byte_port(0xC00CA);
    int i = 0;
    
    if((ca & WS_RTC_FLAG_CMD_IN_PROGRESS) == 0)
    {
        //printf("ws_rtc_wait_for_ready: No command in progress, not waiting. CA = 0x%02X\n", ca);
        return 1;
    }
    
    while((ca & WS_RTC_FLAG_READY) == 0)
    {
        //printf("ws_rtc_wait_for_ready: CA = 0x%02X (%d)\n", ca, i);
        if(i > 255)
        {
            printf("ws_rtc_wait_for_ready: FAIL CA = 0x%02X (%d)\n", ca, i);
            //printf("ws_rtc_wait_for_ready: CB = 0x%02X\n", read_byte_port(0xC00CB));
            return 0;
        }
        
        ws_strobe_cart_clock();
        
        i++;
        ca = read_byte_port(0xC00CA);
    }
    
    //printf("ws_rtc_wait_for_ready: CA = 0x%02X (%d)\n", ca, i);
    //printf("ws_rtc_wait_for_ready: CB = 0x%02X\n", read_byte_port(0xC00CB));
    return 1;
}

uint8_t ws_rtc_wait_for_cmd(uint8_t cmd)
{
    return 1;
    
    uint8_t ca = read_byte_port(0xC00CA);
    int i = 0;
    
    //printf("ws_rtc_wait_for_cmd: initial CA = 0x%02X (%d)\n", ca, i);
    
    while(ca != cmd)
    {
        if(i > 255)
        {
            printf("ws_rtc_wait_for_cmd: FAIL CA = 0x%02X (%d)\n", ca, i);
            //printf("ws_rtc_wait_for_cmd: CB = 0x%02X\n", read_byte_port(0xC00CB));
            return 0;
        }
        
        ws_strobe_cart_clock();
        
        i++;
        ca = read_byte_port(0xC00CA);
    }
    
    //printf("ws_rtc_wait_for_cmd: CA = 0x%02X (%d)\n", ca, i);
    //printf("ws_rtc_wait_for_cmd: CB = 0x%02X\n", read_byte_port(0xC00CB));
    return 1;
}


uint8_t ws_rtc_send_cmd(uint8_t cmd)
{
    //wait until the register is ready
    ws_rtc_wait_for_ready();
    
    //write the command
    write_byte_port(0xC00CA, cmd);
    
    ws_strobe_cart_clock();
    
/*    if(cmd == WS_RTC_CMD_SET_DONE)
    {
        ws_strobe_cart_clock();
        return 1;
    }*/
    
    /*
    //wait until the command is accepted
    if(!ws_rtc_wait_for_cmd(cmd))
        return 0;
     */
    
    //ws_rtc_wait_for_ready();
    
    return 1;
}

uint8_t ws_rtc_set_data(uint8_t data)
{
    uint8_t ca = read_byte_port(0xC00CA);
    uint8_t is_cmd_in_progress = ca & WS_RTC_FLAG_CMD_IN_PROGRESS;
    
    //wait until the register is ready
    ws_rtc_wait_for_ready();
    
    write_byte_port(0xC00CB, data);
    ws_strobe_cart_clock();
    
    if(!is_cmd_in_progress)  //nothing to wait for
        return 1;
    
    //wait until the data is accepted
    ws_rtc_wait_for_cmd(read_byte_port(0xC00CA) & 0x7F);
    
    //wait until the register is ready
    //ws_rtc_wait_for_ready();
    
    return 1;
}

uint8_t ws_rtc_get_data()
{
    //uint8_t ca = read_byte_port(0xC00CA);
    //uint8_t is_cmd_in_progress = ca & WS_RTC_FLAG_CMD_IN_PROGRESS;
    
    //wait until the register is ready
    ws_rtc_wait_for_ready();
   
    uint8_t data = read_byte_port(0xC00CB);
    ws_strobe_cart_clock_times(2);
    
/*    uint8_t ca = read_byte_port(0xC00CA);
    
    int i=0;
    while((ca & WS_RTC_FLAG_READY) == 0)
    {
        printf("ws_rtc_get_data: CA = 0x%02X (%d)\n", ca, i);
        
        if(i > 255)
        {
            printf("ws_rtc_get_data: FAIL CA = 0x%02X (%d)\n", ca, i);
            //printf("ws_rtc_wait_for_ready: CB = 0x%02X\n", read_byte_port(0xC00CB));
            return 0;
        }
        
        ws_strobe_cart_clock();

     
        i++;
        ca = read_byte_port(0xC00CA);
    }
    
    printf("ws_rtc_get_data: exiting CA = 0x%02X (%d)\n", ca, i);*/
    //printf("ws_rtc_get_data: received 0x%02X\n", data);
    return data;
}


uint8_t ws_rtc_set_date_and_time(uint8_t year, uint8_t month, uint8_t day, uint8_t day_of_week, uint8_t hour, uint8_t minute, uint8_t second)
{
    uint8_t cmd_accepted = 0;
    int i;
    
    ws_rtc_set_data(year);
    
    for(i=0; i<10;i++)
    {
        if(ws_rtc_send_cmd(WS_RTC_CMD_SET_DATE_AND_TIME))
        {
            //printf("ws_rtc_set_date_and_time: WS_RTC_CMD_SET_DATE_AND_TIME accepted after %d tries\n", i);
            cmd_accepted = 1;
            break;
        }
    }
    if(!cmd_accepted)
    {
        printf("ws_rtc_set_date_and_time: ERROR!    WS_RTC_CMD_SET_DATE_AND_TIME not accepted after %d tries\n", i);
        return 0;
    }
    
    ws_rtc_set_data(month);
    ws_rtc_set_data(day);
    ws_rtc_set_data(day_of_week);
    ws_rtc_set_data(hour);
    ws_rtc_set_data(minute);
    ws_rtc_set_data(second);
    ws_rtc_wait_for_ready();
    
    ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);
    
    return 1;
}

uint8_t ws_rtc_get_date_and_time(uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *day_of_week, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    uint8_t cmd_accepted = 0;
    int i;
    
    for(i=0; i<10;i++)
    {
        if(ws_rtc_send_cmd(WS_RTC_CMD_GET_DATE_AND_TIME))
        {
            //printf("ws_rtc_get_date_and_time: WS_RTC_CMD_GET_DATE_AND_TIME accepted after %d tries\n", i);
            cmd_accepted = 1;
            break;
        }
    }
    if(!cmd_accepted)
    {
        printf("ws_rtc_get_date_and_time: ERROR!    WS_RTC_CMD_GET_DATE_AND_TIME not accepted after %d tries\n", i);
        return 0;
    }

    
    *year = ws_rtc_get_data();
    
    *month = ws_rtc_get_data();
    
    *day = ws_rtc_get_data();
    
    *day_of_week = ws_rtc_get_data();
    
    *hour = ws_rtc_get_data();
    
    *minute = ws_rtc_get_data();
    
    *second = ws_rtc_get_data();

    ws_rtc_wait_for_ready();
    
    return 1;
}

void ws_rtc_send_data(uint8_t *buffer, uint16_t buffer_len)
{
    uint16_t bytes_sent = 0;
    while((buffer_len - bytes_sent) >= 6)
    {
        ws_rtc_set_date_and_time(WS_RTC_AVR_CMD_DATA_6, buffer[bytes_sent+0], buffer[bytes_sent+1], buffer[bytes_sent+2], buffer[bytes_sent+3], buffer[bytes_sent+4], buffer[bytes_sent+5]);
        //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
        bytes_sent += 6;
    }
    
    if((buffer_len - bytes_sent) > 0)
        ws_rtc_set_date_and_time(WS_RTC_AVR_CMD_DATA_0 + (buffer_len - bytes_sent), buffer[bytes_sent+0], buffer[bytes_sent+1], buffer[bytes_sent+2], buffer[bytes_sent+3], buffer[bytes_sent+4], buffer[bytes_sent+5]);
    //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
}

uint16_t ws_rtc_get_avr_data(uint8_t *get_buffer, uint16_t get_bytes)
{
    uint16_t bytes_received = 0;
    uint8_t cmd_received;
    
    printf("ws_rtc_get_avr_data: get_bytes=%d\n", get_bytes);
    
    if(get_bytes >= 6)
    {
        //only need to send one WS_RTC_AVR_CMD_DATA_REQUEST of 6 until we need to request less
        ws_rtc_set_date_and_time(WS_RTC_AVR_CMD_DATA_REQUEST, 6, 0, 0, 0, 0, 0);
        //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
    
        while((get_bytes - bytes_received) >= 6)
        {
            memset(&get_buffer[bytes_received+0], 0, 6);
            ws_rtc_get_date_and_time(&cmd_received, &get_buffer[bytes_received+0], &get_buffer[bytes_received+1], &get_buffer[bytes_received+2],
                                     &get_buffer[bytes_received+3], &get_buffer[bytes_received+4], &get_buffer[bytes_received+5]);
            //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
            
            if(cmd_received < WS_RTC_AVR_CMD_DATA_0 || cmd_received > WS_RTC_AVR_CMD_DATA_6)
            {
                printf("ws_rtc_get_avr_data: error cmd_received(0x%02X)\n", cmd_received);
                return bytes_received;
            }
            
            if(cmd_received == WS_RTC_AVR_CMD_DATA_0)
            {
                return bytes_received;
            }
            
            /*printf("ws_rtc_get_avr_data: cmd_received=0x%02X bytes_received=%d (get_bytes=%d) {0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}\n", cmd_received, bytes_received + (cmd_received - WS_RTC_AVR_CMD_DATA_0), get_bytes,
                   get_buffer[bytes_received+0], get_buffer[bytes_received+1], get_buffer[bytes_received+2],
                   get_buffer[bytes_received+3], get_buffer[bytes_received+4], get_buffer[bytes_received+5]);*/
            
            bytes_received += (cmd_received - WS_RTC_AVR_CMD_DATA_0);
        }
    }
    
    if((get_bytes - bytes_received) > 0)
    {
        uint8_t in_buffer[6];
        
        printf("ws_rtc_get_avr_data: calling ws_rtc_set_date_and_time to request %d bytes (bytes_received=%d get_bytes=%d)\n", (get_bytes - bytes_received), bytes_received, get_bytes);
        ws_rtc_set_date_and_time(WS_RTC_AVR_CMD_DATA_REQUEST, (get_bytes - bytes_received), 0, 0, 0, 0, 0);
        //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
        
        ws_rtc_get_date_and_time(&cmd_received, &in_buffer[0], &in_buffer[1], &in_buffer[2],
                                 &in_buffer[3], &in_buffer[4], &in_buffer[5]);
        //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
        
        if(cmd_received < WS_RTC_AVR_CMD_DATA_0 || cmd_received > WS_RTC_AVR_CMD_DATA_6)
        {
            printf("ws_rtc_get_avr_data: error cmd_received(0x%02X)\n", cmd_received);
            return bytes_received;
        }
        
        if(cmd_received == WS_RTC_AVR_CMD_DATA_0)
        {
            return bytes_received;
        }
        
        memcpy(get_buffer + bytes_received, in_buffer, (get_bytes - bytes_received));
        
        //printf("ws_rtc_get_avr_data: received %d bytes\n", cmd_received - WS_RTC_AVR_CMD_DATA_0);
        bytes_received += (cmd_received - WS_RTC_AVR_CMD_DATA_0);
    }
    
    printf("ws_rtc_get_avr_data: returning bytes_received=%d\n", bytes_received);
    
    return bytes_received;
}

void * ws_rtc_get_message(uint16_t *msg_length)
{
    uint8_t in_buffer[6];
    uint8_t rtc_avr_cmd_received;
    
    void *get_message;
    
    uint16_t header_length = sizeof(struct ws_avr_msg_generic_struct);
    
    printf("ws_rtc_get_message: calling ws_rtc_set_date_and_time to request %d bytes\n", header_length);
    
    ws_rtc_set_date_and_time(WS_RTC_AVR_CMD_DATA_REQUEST, header_length, 0, 0, 0, 0, 0);
    //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
    
    ws_rtc_get_date_and_time(&rtc_avr_cmd_received, &in_buffer[0], &in_buffer[1], &in_buffer[2],
                             &in_buffer[3], &in_buffer[4], &in_buffer[5]);
    //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);  //ERMERM TODO: necessary?
    
    if(rtc_avr_cmd_received != (WS_RTC_AVR_CMD_DATA_0 + sizeof(struct ws_avr_msg_generic_struct)))
    {
        printf("ERROR ws_rtc_get_message: rtc_avr_cmd_received=0x%02X (expected 0x%lX)\n", rtc_avr_cmd_received, (WS_RTC_AVR_CMD_DATA_0 + sizeof(struct ws_avr_msg_generic_struct)));
        printf("ERROR ws_rtc_get_message: in buffer = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", in_buffer[0], in_buffer[1], in_buffer[2], in_buffer[3], in_buffer[4], in_buffer[5]);
        return NULL;
    }
       
    struct ws_avr_msg_generic_struct *msg_header = (struct ws_avr_msg_generic_struct *) in_buffer;
//    struct ws_avr_msg_generic_struct *msg;
    
    printf("ws_rtc_get_message: sizeof(struct ws_avr_msg_generic_struct) = %lu, header len=0x%X, header type=0x%02X, cmd = 0x%02X\n", sizeof(struct ws_avr_msg_generic_struct), msg_header->length, msg_header->type, rtc_avr_cmd_received);
    
    get_message = malloc(msg_header->length);
    if(!get_message)
        return NULL;
        
    memcpy(get_message, msg_header, sizeof(struct ws_avr_msg_generic_struct));
    
    uint16_t get_length = ((struct ws_avr_msg_generic_struct*)get_message)->length - sizeof(struct ws_avr_msg_generic_struct);
    
    uint16_t received_length = ws_rtc_get_avr_data(((struct ws_avr_msg_generic_struct*)get_message)->buffer, get_length);
    
    if(received_length != get_length)
    {
        printf("ERROR ws_rtc_get_message: received_length=%d (expected %d)\n", received_length, get_length);
        free(get_message);
        return NULL;
    }
    
    *msg_length = received_length + header_length;
    return get_message;
}

void ws_rtc_free_message(void *msg)
{
    free(msg);
    msg = NULL;
}

int ws_rtc_sd_filesystem_open(void)
{
    uint16_t struct_len = sizeof(struct ws_avr_msg_generic_struct);
    
    struct ws_avr_msg_generic_struct          msg;
    
    
    msg.type = WS_AVR_MSG_SD_OPEN;
    msg.length = struct_len;
    
    ws_rtc_send_data((uint8_t*)&msg, msg.length);
    
    struct ws_avr_msg_generic_struct *reply = NULL;
    uint16_t reply_len;
    
    reply = ws_rtc_get_message(&reply_len);//get the result
    
    if(!reply_len || reply == NULL)
    {
        printf("ERROR ws_rtc_sd_filesystem_open: ws_rtc_get_message returned %d bytes and ptr=0x%X\n", reply_len, (uint32_t)reply);
        return 0;
    }
    
    if(reply_len != sizeof(struct ws_avr_msg_generic_struct))
    {
        printf("ERROR ws_rtc_sd_filesystem_open: ws_rtc_get_message returned unexpected result (ret=0x%04X expect=0x%lX)\n", reply_len, sizeof(struct ws_avr_msg_generic_struct));
        ws_rtc_free_message(reply);
        return 0;
    }
    
    if(reply->type != WS_AVR_MSG_SD_OPEN_RESULT)
    {
        printf("ERROR ws_rtc_sd_filesystem_open: ws_rtc_get_message returned unexpected msg type (ret=0x%04X expect=0x%04X)\n", reply->type, WS_AVR_MSG_SD_OPEN_RESULT);
        ws_rtc_free_message(reply);
        return 0;
    }
    
    ws_rtc_free_message(reply);
    return 1;
}

int ws_rtc_sd_filesystem_close(void)
{
    uint16_t struct_len = sizeof(struct ws_avr_msg_generic_struct);
    
    struct ws_avr_msg_generic_struct          msg;
    
    
    msg.type = WS_AVR_MSG_SD_CLOSE;
    msg.length = struct_len;
    
    ws_rtc_send_data((uint8_t*)&msg, msg.length);
    
    struct ws_avr_msg_generic_struct *reply = NULL;
    uint16_t reply_len;
    
    reply = ws_rtc_get_message(&reply_len);//get the result
    
    if(!reply_len || reply == NULL)
    {
        printf("ERROR ws_rtc_sd_filesystem_close: ws_rtc_get_message returned %d bytes and ptr=0x%X\n", reply_len, (uint32_t)reply);
        return  0;
    }
    
    if(reply_len != sizeof(struct ws_avr_msg_generic_struct))
    {
        printf("ERROR ws_rtc_sd_filesystem_close: ws_rtc_get_message returned unexpected result (ret=0x%04X expect=0x%lX)\n", reply_len, sizeof(struct ws_avr_msg_generic_struct));
        ws_rtc_free_message(reply);
        return  0;
    }
    
    if(reply->type != WS_AVR_MSG_SD_CLOSE_RESULT)
    {
        printf("ERROR ws_rtc_sd_filesystem_close: ws_rtc_get_message returned unexpected msg type (ret=0x%04X expect=0x%04X)\n", reply->type, WS_AVR_MSG_SD_CLOSE_RESULT);
        ws_rtc_free_message(reply);
        return  0;
    }
    
    ws_rtc_free_message(reply);
    return 1;
}


uint8_t ws_rtc_sd_file_open(char *path, char *mode)
{
    uint8_t avr_sd_file_index = 0xFF;
    
    uint16_t path_len = strlen(path);
    uint16_t struct_len = sizeof(struct ws_avr_msg_fopen_struct) + path_len + 1;//+1 for NULL termination
    
    if(path_len >= 255)
        return 0xFF;//???
    
    struct ws_avr_msg_fopen_struct          *fopen_msg;
    
    fopen_msg = malloc(struct_len);
    if(!fopen_msg)
        return 0xFF;
    
    fopen_msg->type = WS_AVR_MSG_F_OPEN;
    fopen_msg->length = struct_len;
    strcpy(fopen_msg->mode, mode);
    strcpy(fopen_msg->filename, path);
    
    ws_rtc_send_data((uint8_t*)fopen_msg, fopen_msg->length);
    free(fopen_msg);
    
    //opening the file seems to take a little time, so give it a moment
    //if we don't do this, it seems to fail the first time we open a file, so this works better
    sleep(1);

    struct ws_avr_msg_fopen_result_struct *fopen_reply = NULL;
    uint16_t reply_len;
    
    fopen_reply = ws_rtc_get_message(&reply_len);//get the result
    
    if(!reply_len || fopen_reply == NULL)
    {
        printf("ERROR ws_rtc_sd_file_open: ws_rtc_get_message returned %d bytes and ptr=0x%X\n", reply_len, (uint32_t)fopen_reply);
        return 0xFF;
    }

    if(reply_len != sizeof(struct ws_avr_msg_fopen_result_struct))
    {
        printf("ERROR ws_rtc_sd_file_open: ws_rtc_get_message returned unexpected result (ret=0x%04X expect=0x%lX)\n", reply_len, sizeof(struct ws_avr_msg_fopen_result_struct));
        ws_rtc_free_message(fopen_reply);
        return 0xFF;
    }
    
    if(fopen_reply->type != WS_AVR_MSG_F_OPEN_RESULT)
    {
        printf("ERROR ws_rtc_sd_file_open: ws_rtc_get_message returned unexpected msg type (ret=0x%04X expect=0x%04X)\n", fopen_reply->type, WS_AVR_MSG_F_OPEN_RESULT);
        ws_rtc_free_message(fopen_reply);
        return 0xFF;
    }

    avr_sd_file_index = fopen_reply->file_index;
    ws_rtc_free_message(fopen_reply);
    return avr_sd_file_index;
}

uint16_t ws_rtc_sd_file_read(uint8_t avr_sd_file_index, void *buffer, uint16_t buffer_len)
{
    uint16_t struct_len = sizeof(struct ws_avr_msg_fread_struct);
    
    struct ws_avr_msg_fread_struct          *fread_msg;
    
    printf("ws_rtc_sd_file_read: malloc(%d)\n", struct_len);
    fread_msg = malloc(struct_len);
    if(!fread_msg)
        return 0;
    
    fread_msg->type = WS_AVR_MSG_F_READ;
    fread_msg->length = struct_len;
    fread_msg->file_index = avr_sd_file_index;
    fread_msg->num_bytes = buffer_len;
    
    printf("ws_rtc_sd_file_read: call ws_rtc_send_data with %d bytes\n", fread_msg->length);
    ws_rtc_send_data((uint8_t*)fread_msg, fread_msg->length);
    free(fread_msg);
    
    struct ws_avr_msg_fread_result_struct *fread_reply = NULL;
    uint16_t reply_len;

    sleep(2);  //wait a bit????
    fread_reply = ws_rtc_get_message(&reply_len);//get the result
    
    if(!reply_len || fread_reply == NULL)
    {
        printf("ERROR ws_rtc_sd_file_read: ws_rtc_get_message returned %d bytes and ptr=0x%X\n", reply_len, (uint32_t)fread_reply);
        return 0;
    }

    if(reply_len > (sizeof(struct ws_avr_msg_fread_result_struct) + buffer_len) || reply_len < sizeof(struct ws_avr_msg_fread_result_struct))
    {
        printf("ERROR ws_rtc_sd_file_read: ws_rtc_get_message returned unexpected result (ret=0x%04X structsize=0x%lX)\n", reply_len, sizeof(struct ws_avr_msg_fread_result_struct));
        ws_rtc_free_message(fread_reply);
        return 0;
    }
    
    if(fread_reply->type != WS_AVR_MSG_F_READ_RESULT)
    {
        printf("ERROR ws_rtc_sd_file_read: ws_rtc_get_message returned unexpected msg type (ret=0x%04X expect=0x%04X)\n", fread_reply->type, WS_AVR_MSG_F_READ_RESULT);
        ws_rtc_free_message(fread_reply);
        return 0;
    }
    
    printf("ws_rtc_sd_file_read: fresult=0x%02X %d bytes read (type=0x%02X, msglen=%d)\n", fread_reply->fresult, fread_reply->num_bytes, fread_reply->type, fread_reply->length);
    
    uint16_t offset = fread_reply->length - fread_reply->num_bytes;  //don't need the metadata in the read buffer
    memcpy(buffer, fread_reply->buffer, fread_reply->num_bytes);
    
    uint16_t num_bytes_read = fread_reply->num_bytes;
    ws_rtc_free_message(fread_reply);
    return num_bytes_read;
}

uint16_t ws_rtc_sd_file_write(uint8_t avr_sd_file_index, void *buffer, uint16_t buffer_len)
{
    uint16_t struct_len = sizeof(struct ws_avr_msg_fwrite_struct) + buffer_len;
    
    struct ws_avr_msg_fwrite_struct          *fwrite_msg;
    
    printf("ws_rtc_sd_file_write: malloc(%d)\n", struct_len);
    fwrite_msg = malloc(struct_len);
    if(!fwrite_msg)
        return 0;
    
    fwrite_msg->type = WS_AVR_MSG_F_WRITE;
    fwrite_msg->length = struct_len;
    fwrite_msg->file_index = avr_sd_file_index;
    printf("ws_rtc_sd_file_write: memcpy %d bytes\n", buffer_len);
    memcpy(fwrite_msg->buffer, buffer, buffer_len);
    
    printf("ws_rtc_sd_file_write: call ws_rtc_send_data with %d bytes\n", fwrite_msg->length);
    ws_rtc_send_data((uint8_t*)fwrite_msg, fwrite_msg->length);
    free(fwrite_msg);
    
    struct ws_avr_msg_fwrite_result_struct *fwrite_reply = NULL;
    uint16_t reply_len;
    
    fwrite_reply = ws_rtc_get_message(&reply_len);//get the result
    
    if(!reply_len || fwrite_reply == NULL)
    {
        printf("ERROR ws_rtc_sd_file_write: ws_rtc_get_message returned %d bytes and ptr=0x%X\n", reply_len, (uint32_t)fwrite_reply);
        return 0;
    }
    
    if(reply_len != sizeof(struct ws_avr_msg_fwrite_result_struct))
    {
        printf("ERROR ws_rtc_sd_file_write: ws_rtc_get_message returned unexpected result (ret=0x%04X expect=0x%lX)\n", reply_len, sizeof(struct ws_avr_msg_fwrite_result_struct));
        ws_rtc_free_message(fwrite_reply);
        return 0;
    }
    
    if(fwrite_reply->type != WS_AVR_MSG_F_WRITE_RESULT)
    {
        printf("ERROR ws_rtc_sd_file_write: ws_rtc_get_message returned unexpected msg type (ret=0x%04X expect=0x%04X)\n", fwrite_reply->type, WS_AVR_MSG_F_WRITE_RESULT);
        ws_rtc_free_message(fwrite_reply);
        return 0;
    }
    
    printf("ws_rtc_sd_file_write: fresult=0x%02X %d bytes written\n", fwrite_reply->fresult, fwrite_reply->num_bytes);
    
    uint16_t num_bytes_written = fwrite_reply->num_bytes;
    ws_rtc_free_message(fwrite_reply);
    return num_bytes_written;
}


uint8_t ws_rtc_sd_file_close(uint8_t avr_sd_file_index)
{
    struct ws_avr_msg_fclose_struct          fclose_msg;
    
    fclose_msg.type = WS_AVR_MSG_F_CLOSE;
    fclose_msg.length = sizeof(fclose_msg);
    fclose_msg.file_index = avr_sd_file_index;
    
    ws_rtc_send_data((uint8_t*)&fclose_msg, fclose_msg.length);
    
    struct ws_avr_msg_fclose_result_struct *fclose_reply = NULL;
    uint16_t reply_len;
    
    fclose_reply = ws_rtc_get_message(&reply_len);//get the result
    
    if(!reply_len || fclose_reply == NULL)
    {
        printf("ERROR ws_rtc_sd_file_close: ws_rtc_get_message returned %d bytes and ptr=0x%X\n", reply_len, (uint32_t)fclose_reply);
        return 0;
    }
    
    if(reply_len != sizeof(struct ws_avr_msg_fclose_result_struct))
    {
        printf("ERROR ws_rtc_sd_file_close: ws_rtc_get_message returned unexpected result (ret=0x%04X expect=0x%lX)\n", reply_len, sizeof(struct ws_avr_msg_fclose_result_struct));
        ws_rtc_free_message(fclose_reply);
        return 0;
    }
    
    if(fclose_reply->type != WS_AVR_MSG_F_CLOSE_RESULT)
    {
        printf("ERROR ws_rtc_sd_file_close: ws_rtc_get_message returned unexpected msg type (ret=0x%04X expect=0x%04X)\n", fclose_reply->type, WS_AVR_MSG_F_CLOSE_RESULT);
        ws_rtc_free_message(fclose_reply);
        return 0;
    }
    
    printf("ws_rtc_sd_file_close: fresult=0x%02X\n", fclose_reply->fresult);
    
    uint8_t fresult = fclose_reply->fresult;
    ws_rtc_free_message(fclose_reply);
    return fresult;
}

void ws_rtc_file_test___(char *write_string)
{
    //wake up the AVR on the cart
    write_byte_port(0xC00CD, 0x08);  //set GPO3 high
    write_byte_port(0xC00CC, 0x08);  //turn on GPO3 output
    write_byte_port(0xC00CD, 0x00);  //set GPO3 low so we generate a falling edge

    uint8_t file_index = ws_rtc_sd_file_open("ws_rtc.txt", "a");    //append mode
    
    printf("ws_rtc_file_test: file index = 0x%02X\n", file_index);
    
    if(file_index < 0x10)
    {
        uint16_t bytes_written = ws_rtc_sd_file_write(file_index, write_string, strlen(write_string));
        
        printf("ws_rtc_file_test: bytes_written = 0x%04X\n", bytes_written);
        
        uint8_t close_result = ws_rtc_sd_file_close(file_index);
        
        printf("ws_rtc_file_test: file closed with result 0x%02X\n", close_result);
    }
    
    
    //sleep the AVR on the cart
    write_byte_port(0xC00CD, 0x08);//GP3 high
    write_byte_port(0xC00CC, 0x00);
}

/* read bytes from local write_filename and write them to the WS_AVR file of the same name
 */
void ws_rtc_file_test(char *write_filename)
{
    FILE *f = fopen(write_filename, "rb");
    uint8_t io_buf[512];
    uint8_t file_buf[512];
    uint8_t fail = 0;

    //wake up the AVR on the cart
    write_byte_port(0xC00CD, 0x08);  //set GPO3 high
    write_byte_port(0xC00CC, 0x08);  //turn on GPO3 output
    write_byte_port(0xC00CD, 0x00);  //set GPO3 low so we generate a falling edge
    
    fail = 1;
    for(int i=0; i<10; i++)
    {
        if(ws_rtc_sd_filesystem_open())
        {
            fail = 0;
            break;
        }
    }
    if(fail)
    {
        printf("ERROR: ws_rtc_file_test: Unable to open SD card on WS Flash Masta cartridge.\n");
        return;
    }
    
    fseek(f, 0L, SEEK_END);
    uint32_t file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    
    uint8_t file_index = ws_rtc_sd_file_open(write_filename, "w");    //append mode
    
    printf("ws_rtc_file_test: file index = 0x%02X\n", file_index);

    if(file_index < 0x10)
    {
        uint32_t file_size_temp = file_size;
        while(file_size_temp && !fail)
        {
            uint16_t bytes_read = fread(file_buf, 1, 512, f);
            uint16_t bytes_written = ws_rtc_sd_file_write(file_index, file_buf, bytes_read);
            
            if(bytes_read != bytes_written)
                printf("ERROR: ws_rtc_file_test: could not write bytes (bytes_read = 0x%04X / bytes_written = 0x%04X)\n", bytes_read, bytes_written);
            
            file_size_temp -= bytes_read;
            printf("ws_rtc_file_test: bytes_read = 0x%04X / bytes_written = 0x%04X (0x%04X bytes remain)\n", bytes_read, bytes_written, file_size_temp);
                   
            if(bytes_read != bytes_written)
                fail = 1;
        }

        
        uint8_t close_result = ws_rtc_sd_file_close(file_index);
        printf("ws_rtc_file_test: file closed with result 0x%02X\n", close_result);
        file_index = ws_rtc_sd_file_open(write_filename, "r");
        printf("ws_rtc_file_test: file index = 0x%02X\n", file_index);
        
        if(file_index >= 0x10)
        {
            fail = 1;
        }
        
        FILE *of = fopen("tempfile.out", "w+b");
        file_size_temp = file_size;
        fseek(f, 0L, SEEK_SET);
        while(file_size_temp && !fail)
        {
            uint16_t bytes_to_read = fread(file_buf, 1, 512, f);
            uint16_t bytes_read = ws_rtc_sd_file_read(file_index, io_buf, bytes_to_read);
            fwrite(io_buf, 1, bytes_read, of);
            
            file_size_temp -= bytes_read;
            printf("ws_rtc_file_test: bytes_read = 0x%04X / bytes_to_read = 0x%04X (0x%04X bytes remain)\n", bytes_read, bytes_to_read, file_size_temp);
            
            if(bytes_read != bytes_to_read)
            {
                printf("ERROR: ws_rtc_file_test: could not read bytes (bytes_read = 0x%04X / bytes_to_read = 0x%04X)\n", bytes_read, bytes_to_read);
                fail = 1;
            }
            else if(memcmp(file_buf, io_buf, bytes_read) != 0)
            {
                printf("ERROR: ws_rtc_file_test: memcmp mismatch\n");
                fail = 1;
            }
        }
        fclose(of);
        
        
        
        close_result = ws_rtc_sd_file_close(file_index);
        printf("ws_rtc_file_test: file closed with result 0x%02X\n", close_result);
    }
    else
    {
        printf("ws_rtc_file_test: file open error 0x%02X\n", file_index);
    }
    
    ws_rtc_sd_filesystem_close();
    
    
    //sleep the AVR on the cart
    write_byte_port(0xC00CD, 0x08);//GP3 high
    write_byte_port(0xC00CC, 0x00);
    
    fclose(f);
}


void ws_rtc_test(uint8_t data0, uint8_t data1)
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t day_of_week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    
    //wake up the AVR on the cart
    write_byte_port(0xC00CD, 0x08);  //set GPO3 high
    write_byte_port(0xC00CC, 0x08);  //turn on GPO3 output
    write_byte_port(0xC00CD, 0x00);  //set GPO3 low so we generate a falling edge

    //ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);

//    ws_rtc_set_date_and_time(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01);
    ws_rtc_set_date_and_time(data0, data1, data1+1, data1+2, data1+3, data1+4, data1+5);
//    ws_rtc_set_date_and_time(0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);

    ws_rtc_get_date_and_time(&year, &month, &day, &day_of_week, &hour, &minute, &second);

    ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);
    
    printf("Y=0x%02X, Mo=0x%02X, D=0x%02X, DoW=0x%02X, H=0x%02X, Min=0x%02X, S=0x%02X\n", year, month, day, day_of_week, hour, minute, second);
    //sleep the AVR on the cart
    write_byte_port(0xC00CD, 0x08);//GP3 high
    write_byte_port(0xC00CC, 0x00);
}

uint8_t ws_rtc_send_avr_cmd_plus_data_buffer(uint8_t avr_command, uint8_t data_len, uint8_t *data)
{
    uint8_t cmd_accepted = 0;
    int i;
    
    ws_rtc_set_data(avr_command);
    
    for(i=0; i<10;i++)
    {
        if(ws_rtc_send_cmd(WS_RTC_CMD_SET_DATE_AND_TIME))
        {
            printf("ws_rtc_send_avr_cmd_plus_data_buffer: WS_RTC_CMD_SET_DATE_AND_TIME accepted after %d tries\n", i);
            cmd_accepted = 1;
            break;
        }
    }
    if(!cmd_accepted)
    {
        printf("ws_rtc_send_avr_cmd_plus_data_buffer: ERROR!    WS_RTC_CMD_SET_DATE_AND_TIME not accepted after %d tries\n", i);
        return 0;
    }
    
    ws_rtc_set_data(data_len);
    
    uint8_t bytes_sent = 0;
    while(bytes_sent < data_len && bytes_sent < 6)
    {
        ws_rtc_set_data(data[bytes_sent]);
        bytes_sent++;
    }
    ws_rtc_wait_for_ready();
    ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);
    
    return bytes_sent;
}

uint8_t ws_rtc_send_avr_cmd_plus_data(uint8_t avr_command, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4, uint8_t data5, uint8_t data6)
{
    uint8_t cmd_accepted = 0;
    int i;
    
    ws_rtc_set_data(avr_command);
    
    for(i=0; i<10;i++)
    {
        if(ws_rtc_send_cmd(WS_RTC_CMD_SET_DATE_AND_TIME))
        {
            //printf("ws_rtc_send_avr_cmd_plus_data: WS_RTC_CMD_SET_DATE_AND_TIME accepted after %d tries (AVR cmd = 0x%02X)\n", i, avr_command);
            cmd_accepted = 1;
            break;
        }
    }
    if(!cmd_accepted)
    {
        printf("ws_rtc_send_avr_cmd_plus_data: ERROR!    WS_RTC_CMD_SET_DATE_AND_TIME not accepted after %d tries (AVR cmd = 0x%02X)\n", i, avr_command);
        return 0;
    }
    
    ws_rtc_set_data(data0);
    ws_rtc_set_data(data1);
    ws_rtc_set_data(data2);
    ws_rtc_set_data(data3);
    ws_rtc_set_data(data4);
    ws_rtc_set_data(data5);
    ws_rtc_set_data(data6);
    
    ws_rtc_wait_for_ready();
    ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);
    
    return 1;
}


uint8_t ws_rtc_get_avr_reply_data(uint8_t *data0, uint8_t *data1, uint8_t *data2, uint8_t *data3, uint8_t *data4, uint8_t *data5, uint8_t *data6)
{
    uint8_t cmd_accepted = 0;
    int i;
    
    for(i=0; i<10;i++)
    {
        if(ws_rtc_send_cmd(WS_RTC_CMD_GET_DATE_AND_TIME))
        {
            //printf("ws_rtc_get_avr_reply_data: WS_RTC_CMD_GET_DATE_AND_TIME accepted after %d tries\n", i);
            cmd_accepted = 1;
            break;
        }
    }
    if(!cmd_accepted)
    {
        printf("ws_rtc_get_avr_reply_data: ERROR!    WS_RTC_CMD_GET_DATE_AND_TIME not accepted after %d tries\n", i);
        return 0;
    }
    
    
    *data0 = ws_rtc_get_data();
    
    if(data1 == NULL)
        ws_rtc_get_data();
    else
        *data1 = ws_rtc_get_data();
    
    if(data2 == NULL)
        ws_rtc_get_data();
    else
        *data2 = ws_rtc_get_data();
    
    if(data3 == NULL)
        ws_rtc_get_data();
    else
        *data3 = ws_rtc_get_data();
    
    if(data4 == NULL)
        ws_rtc_get_data();
    else
        *data4 = ws_rtc_get_data();
    
    if(data5 == NULL)
        ws_rtc_get_data();
    else
        *data5 = ws_rtc_get_data();
    
    if(data6 == NULL)
        ws_rtc_get_data();
    else
        *data6 = ws_rtc_get_data();
    
    ws_rtc_wait_for_ready();
    ws_rtc_send_cmd(WS_RTC_CMD_SET_DONE);
    
    return 1;
}

void ws_rtc_select_cart_slot(uint8_t slot_num)
{
    uint8_t old_slot=0xFF, new_slot = 0xFF;
    
    write_byte_port(0xC00CD, 0x08);  //set GPO3 high
    write_byte_port(0xC00CC, 0x08);  //turn on GPO3 output
    write_byte_port(0xC00CD, 0x00);  //set GPO3 low so we generate a falling edge
    
    ws_rtc_send_avr_cmd_plus_data(WS_RTC_AVR_CMD_GET_SLOT, 0, 0, 0, 0, 0, 0, 0);
    ws_rtc_get_avr_reply_data(&old_slot, NULL, NULL, NULL, NULL, NULL, NULL);

    ws_rtc_send_avr_cmd_plus_data(WS_RTC_AVR_CMD_SET_SLOT, slot_num, 0, 0, 0, 0, 0, 0);
    
    ws_rtc_send_avr_cmd_plus_data(WS_RTC_AVR_CMD_GET_SLOT, 0, 0, 0, 0, 0, 0, 0);
    ws_rtc_get_avr_reply_data(&new_slot, NULL, NULL, NULL, NULL, NULL, NULL);
    
    printf("Old Slot = %d / Set Slot = %d / New Slot = %d\n", old_slot, slot_num, new_slot);
    
    write_byte_port(0xC00CD, 0x08);  //set GPO3 high
    write_byte_port(0xC00CC, 0x00);  //turn off GPO3 output
}

#define WS_GPO_AVR_CMD_SET_SLOT         0xA0
void ws_gpo_select_cart_slot(uint8_t slot_num)
{
    uint8_t send_buf[2];
    send_buf[0] = WS_GPO_AVR_CMD_SET_SLOT;
    send_buf[1] = slot_num;
    write_bytes_to_avr_via_gpo(2, send_buf);
}


void close_libusb(void)
{
    //blink the LED 2 times to signal that we're exiting
    blink_linkmasta_led(2);

	libusb_release_interface(devh, 0);
    libusb_reset_device(devh);
	libusb_close(devh);
	libusb_exit(NULL);
}

int main(int argc, char *argv[])
{
    uint8_t currChip=0, majVer, minVer;

	uint16_t options;

	int r = 1;

	r = libusb_init(NULL);
	if (r < 0) {
		fprintf(stderr, "Failed to initialise libusb\n");
		exit(1);
	}

	r = find_lvr_usb();
	if (r < 0) {
		fprintf(stderr, "Could not find/open device\n");
		exit(-1);
	}
//    printf("Successfully found device\n");

	libusb_detach_kernel_driver(devh, 0);

	r = libusb_set_configuration(devh, 1);
	if (r < 0) {
		fprintf(stderr, "libusb_set_configuration error %d\n", r);
		exit(-1);
	}
//	printf("Successfully set usb configuration 1\n");

	r = libusb_claim_interface(devh, 0);
	if (r < 0) {
		fprintf(stderr, "libusb_claim_interface error %d\n", r);
		exit(-1);
	}
//	printf("Successfully claimed interface\n");

    atexit(close_libusb);


    check_firmware_version(&majVer, &minVer);
    if(majVer != MAJOR_VERSION || minVer != MINOR_VERSION)
    {
        printf("Version mismatch!  Expected v%d.%02d  Firmware returned v%d.%02d.  Exiting.\n", MAJOR_VERSION, MINOR_VERSION, majVer, minVer);
        return -1;
    }

	//set_unset_WS_lines(LINE_RESET, 0);
    
	options = parseCmdLine(argc, argv);


    if(options & OPTION_INITWSMMC)
    {
        init_wonderswan_MMC();
        exit(0);
    }


    memset(prot[0], 0x01, 35);
    memset(prot[1], 0x01, 35);

    
    uint8_t isSlotNumFixed, isSlotSizeFixed;
    get_cartinfo(&isSlotNumFixed, &isSlotSizeFixed, &g_numSlotsPerCart, &g_numAddrLinesPerSlot);
    
    if((isSlotNumFixed != 1) || (isSlotSizeFixed != 1))
    {
        printf("get_cartinfo returned invalid info: %d %d %d %d\n", isSlotNumFixed, isSlotSizeFixed, g_numSlotsPerCart, g_numAddrLinesPerSlot);
        exit(1);
    }

    if(options & OPTION_INFO)
    {
        printf("Detected %d slots (%d bytes each)\n", g_numSlotsPerCart, 1 << g_numAddrLinesPerSlot);
        
        write_word_rom(ws1_addr, 0xAA);
		write_word_rom(ws2_addr, 0x55);
		write_word_rom(ws3_addr, 0xF0);

		write_word_rom(ws1_addr, 0xAA);
		write_word_rom(ws2_addr, 0x55);
	    write_word_rom(ws3_addr, 0x90);

//        printf(" 0x%02X\n", read_byte(0, currChip));//make sure we're back to readable state

        //remember that these are all byte addresses, even though we are reading 16-bits (hence the shift << 1)
#if FLASH_ACCESS_DATA_BITS == 8
        manufID[currChip] = read_byte_rom(0x00 << 1);
        deviceID[currChip] = read_byte_rom(0x01 << 1);
        
        //factoryProt[currChip] = read_byte_rom(0x02);
#elif FLASH_ACCESS_DATA_BITS == 16
		manufID[currChip] = read_word_rom(0x00 << 1);
		deviceID[currChip] = read_word_rom(0x01 << 1);
	    //factoryProt[currChip] = read_word_rom(0x02 << 1);
#else
 #error FLASH_ACCESS_DATA_BITS mismatch
#endif
		//printf("\nChip #%d info:\n", currChip);
        printf(" ManufID  = 0x%04X\n", manufID[currChip]);
        printf(" DeviceID = 0x%04X\n", deviceID[currChip]);

		if(manufID[currChip] == 0x0001 && deviceID[currChip] == 0x227E)
		{
#if FLASH_ACCESS_DATA_BITS == 8
            uint16_t size = read_byte_rom(0x0E);
            uint16_t unlockbypassSupported = read_byte_rom(0x51);
#elif FLASH_ACCESS_DATA_BITS == 16
            uint16_t size = read_word_rom(0x0E<<1);
            uint16_t unlockbypassSupported = read_word_rom(0x51<<1);
#endif
            printf("  S29GL-S Chip Detected\n");
            
            switch(size)
			{
			case 0x2228:
				printf("   Size = 1 Gb\n");
				break;
			case 0x2223:
				printf("   Size = 512 Mb\n");
				break;
			case 0x2222:
				printf("   Size = 256 Mb\n");
				break;
			case 0x2221:
				printf("   Size = 128 Mb\n");
				break;
			default:
				printf("   Size UNKNOWN\n");
				break;
			}
            
            printf("   Unlock Bypass %s (0x%04X)\n", unlockbypassSupported ? "Supported" : "NOT Supported", unlockbypassSupported);
		}
        else if(manufID[currChip] == 0x0089 && (deviceID[currChip] == 0x007E || deviceID[currChip] == 0x227E))
        {
#if FLASH_ACCESS_DATA_BITS == 8
            uint16_t size = read_byte_rom(0x0E<<1);
            uint16_t code3 = read_byte_rom(0x0F<<1);
            if(code3 != 0x01)
                printf("Device Code 3 mismatch! (0x%02X)\n", code3);
#elif FLASH_ACCESS_DATA_BITS == 16
            uint16_t size = read_word_rom(0x0E<<1);
            uint16_t code3 = read_word_rom(0x0F<<1);
            if(code3 != 0x2201 && code3 != 0x01)
                printf("Device Code 3 mismatch! (0x%04X)\n", code3);
#endif
            printf("  M29EW Chip Detected\n");
            
            switch(size)
            {
                case 0x2228:
                case 0x0028:
                    printf("   Size = 1 Gb\n");
                    break;
                case 0x2223:
                case 0x0023:
                    printf("   Size = 512 Mb\n");
                    break;
                case 0x2222:
                case 0x0022:
                    printf("   Size = 256 Mb\n");
                    break;
                case 0x2221:
                case 0x0021:
                    printf("   Size = 128 Mb\n");
                    break;
                default:
                    printf("   Size UNKNOWN (code 0x%04X)\n", size);
                    break;
            }
            
            printf("   Unlock Bypass support assumed for this chip\n");
        }
        
        //printf(" FactoryProt = 0x%04X\n", factoryProt[currChip]);
        printf("\n");

		write_word_rom(ws1_addr, 0xAA);
		write_word_rom(ws2_addr, 0x55);
		write_word_rom(ws3_addr, 0xF0);
		read_word_rom(0x00 << 1);//make sure we're back to readable state
        
        
        
        uint32_t addr = 0xFFFFF6;
        /*for(int i = 0; i < 10; i+=2)
         {
         printf("Addr 0x%6X = 0x%04X\n", addr+i, read_word_rom(addr+i));
         }*/
        
        char *developer_name = NULL;
        
        uint8_t developer = read_word_rom(addr + 0) & 0xFF;
        switch(developer)
        {
            case 0x00:
                developer_name = "Misc.";
                break;
            case 0x01:
                developer_name = "Bandai (and misc.)";
                break;
            case 0x02:
                developer_name = "Taito";
                break;
            case 0x03:
                developer_name = "Tomy";
                break;
            case 0x04:
                developer_name = "Koei";
                break;
            case 0x05:
                developer_name = "Data East";
                break;
            case 0x06:
                developer_name = "Asmik";
                break;
            case 0x07:
                developer_name = "Media Entertainment";
                break;
            case 0x08:
                developer_name = "Nichibutsu";
                break;
            case 0xA:
                developer_name = "Coconuts Japan";
                break;
            case 0xB:
                developer_name = "Sammy";
                break;
            case 0xC:
                developer_name = "Sunsoft";
                break;
            case 0xD:
                developer_name = "Mebius";
                break;
            case 0xE:
                developer_name = "Banpresto";
                break;
            case 0x10:
                developer_name = "Jaleco";
                break;
            case 0x11:
                developer_name = "Imagineer";
                break;
            case 0x12:
                developer_name = "Konami";
                break;
            case 0x16:
                developer_name = "Kobunsha";
                break;
            case 0x17:
                developer_name = "Bottom Up";
                break;
            case 0x18:
                developer_name = "Naxat (Mechanic Arms? Media Entertainment?)";
                break;
            case 0x19:
                developer_name = "Sunrise";
                break;
            case 0x1A:
                developer_name = "Cyberfront";
                break;
            case 0x1B:
                developer_name = "Megahouse";
                break;
            case 0x1D:
                developer_name = "Interbec";
                break;
            case 0x1E:
                developer_name = "NAC";
                break;
            case 0x1F:
                developer_name = "Emotion (Bandai Visual?)";
                break;
            case 0x20:
                developer_name = "Athena";
                break;
            case 0x21:
                developer_name = "KID";
                break;
            case 0x24:
                developer_name = "Omega Micott";
                break;
            case 0x25:
                developer_name = "Upstar";
                break;
            case 0x26:
                developer_name = "Kadokawa/Megas";
                break;
            case 0x27:
                developer_name = "Cocktail Soft";
                break;
            case 0x28:
                developer_name = "Squaresoft";
                break;
            case 0x2B:
                developer_name = "TomCreate";
                break;
            case 0x2D:
                developer_name = "Namco";
                break;
            case 0x2F:
                developer_name = "Gust";
                break;
            case 0x36:
                developer_name = "Capcom";
                break;
        }
        
        if(developer_name == NULL)
            printf("Error detecting developer name.  Cart read problem?\n");
        else
            printf("Developer = %s\n", developer_name);
        
        uint8_t is_color = read_word_rom(addr + 0) >> 8;
        if(is_color > 1)
            printf("Error detecting BW/Color.  0x%02X\n", is_color);
        else
            printf("%s\n", is_color ? "Color" : "Black & White");
        
        uint8_t ws_flags = read_word_rom(addr + 6) & 0xFF;
        if(!(ws_flags & 0x04))
            printf("Error: Always-on flag not set!\n");
        
        if(ws_flags & 0x01)
            printf("Vertical\n");
        else
            printf("Horizontal\n");
        
        uint8_t has_rtc = read_word_rom(addr + 6) >> 8;
        if(has_rtc > 1)
            printf("Error: RTC value unrecognized\n");
        else if(has_rtc == 0)
            printf("RTC not used\n");
        else
            printf("RTC in use\n");

    }
    

    if(options & OPTION_TEST)
    {
        
        /*
		write_word_rom(ws1_addr, 0xAA);
		write_word_rom(ws2_addr, 0x55);
		write_word_rom(ws3_addr, 0xF0);

		write_word_rom(ws1_addr, 0xAA);
		write_word_rom(ws2_addr, 0x55);

		uint32_t write_address = 0;
	    write_word_rom(write_address, 0x25);		//write buffer command (sector, 0x25)
	    write_word_rom(write_address, 2-1);			//word count (minus 1) number of locations at sector (sector, num_words)

		write_word_rom(write_address,	0xEDED);		//first word to write
		write_word_rom(write_address+2, 0xF00D);		//second word to write

		write_word_rom(write_address, 0x29);		//write buffer program confirm (must follow last write buffer location or ABORT)
		//device busy (programming)*/
	
	}
/*
    i=0;
    for(addr=0x24;addr<0x24+12;addr++)
        cartRomName[i++] = read_byte(addr, 0); //read the cart name from chip 1
    cartRomName[13]=0;

    printf("%dmbit cart detected - %s\n", chipSize[0]+chipSize[1], cartRomName);
*/

    if(options & OPTION_WRITE)
    {
        write_file_to_cart_rom(filename);
    }
    if(options & OPTION_VERIFY)
    {
        if(!verify_file_with_cart_slot(verifyFilename))
            exit(1);//fail
    }

    if(options & OPTION_BACKUP)
    {
        uint32_t addr = 0xFFFFF6;
        /*for(int i = 0; i < 10; i+=2)
        {
            printf("Addr 0x%6X = 0x%04X\n", addr+i, read_word_rom(addr+i));
        }*/
        
        char *developer_name = NULL;
        
        uint8_t developer = read_word_rom(addr + 0) & 0xFF;
        switch(developer)
        {
            case 0x00:
                developer_name = "Misc.";
                break;
            case 0x01:
                developer_name = "Bandai (and misc.)";
                break;
            case 0x02:
                developer_name = "Taito";
                break;
            case 0x03:
                developer_name = "Tomy";
                break;
            case 0x04:
                developer_name = "Koei";
                break;
            case 0x05:
                developer_name = "Data East";
                break;
            case 0x06:
                developer_name = "Asmik";
                break;
            case 0x07:
                developer_name = "Media Entertainment";
                break;
            case 0x08:
                developer_name = "Nichibutsu";
                break;
            case 0xA:
                developer_name = "Coconuts Japan";
                break;
            case 0xB:
                developer_name = "Sammy";
                break;
            case 0xC:
                developer_name = "Sunsoft";
                break;
            case 0xD:
                developer_name = "Mebius";
                break;
            case 0xE:
                developer_name = "Banpresto";
                break;
            case 0x10:
                developer_name = "Jaleco";
                break;
            case 0x11:
                developer_name = "Imagineer";
                break;
            case 0x12:
                developer_name = "Konami";
                break;
            case 0x16:
                developer_name = "Kobunsha";
                break;
            case 0x17:
                developer_name = "Bottom Up";
                break;
            case 0x18:
                developer_name = "Naxat (Mechanic Arms? Media Entertainment?)";
                break;
            case 0x19:
                developer_name = "Sunrise";
                break;
            case 0x1A:
                developer_name = "Cyberfront";
                break;
            case 0x1B:
                developer_name = "Megahouse";
                break;
            case 0x1D:
                developer_name = "Interbec";
                break;
            case 0x1E:
                developer_name = "NAC";
                break;
            case 0x1F:
                developer_name = "Emotion (Bandai Visual?)";
                break;
            case 0x20:
                developer_name = "Athena";
                break;
            case 0x21:
                developer_name = "KID";
                break;
            case 0x24:
                developer_name = "Omega Micott";
                break;
            case 0x25:
                developer_name = "Upstar";
                break;
            case 0x26:
                developer_name = "Kadokawa/Megas";
                break;
            case 0x27:
                developer_name = "Cocktail Soft";
                break;
            case 0x28:
                developer_name = "Squaresoft";
                break;
            case 0x2B:
                developer_name = "TomCreate";
                break;
            case 0x2D:
                developer_name = "Namco";
                break;
            case 0x2F:
                developer_name = "Gust";
                break;
            case 0x36:
                developer_name = "Capcom";
                break;
        }
        
        if(developer_name == NULL)
            printf("Error detecting developer name.  Cart read problem?\n");
        else
            printf("Developer = %s\n", developer_name);
        
        uint8_t is_color = read_word_rom(addr + 0) >> 8;
        if(is_color > 1)
            printf("Error detecting BW/Color.  0x%02X\n", is_color);
        else
            printf("%s\n", is_color ? "Color" : "Black & White");
        
        uint8_t ws_flags = read_word_rom(addr + 6) & 0xFF;
        if(!(ws_flags & 0x04))
            printf("Error: Always-on flag not set!\n");
        
        if(ws_flags & 0x01)
            printf("Vertical\n");
        else
            printf("Horizontal\n");

        uint8_t has_rtc = read_word_rom(addr + 6) >> 8;
        if(has_rtc > 1)
            printf("Error: RTC value unrecognized\n");
        else if(has_rtc == 0)
            printf("RTC not used\n");
        else
            printf("RTC in use\n");
        
        uint16_t temp_word = read_word_rom(0xFFFFFA);
		uint8_t size_id = temp_word & 0xFF;
		uint32_t cart_size = 0;
		switch(size_id)
		{
		case 0x02:
			cart_size = 4;//Mbit
			break;
		case 0x03:
			cart_size = 8;//Mbit
			break;
		case 0x04:
			cart_size = 16;//Mbit
			break;
		case 0x06:
			cart_size = 32;//Mbit
			break;
		case 0x08:
			cart_size = 64;//Mbit
			break;
		case 0x09:
			cart_size = 128;//Mbit
			break;
		default:
			cart_size = 0;//fail
			break;
		}
        printf("Detected %d Mbit cart.\n", cart_size);
        
        if(cart_size_override)
        {
            cart_size = cart_size_override;
            printf("Detection overridden to %d Mbit.\n", cart_size);
        }
        
        cart_size = cart_size * 1024 * 1024 / 8;  //convert to bytes

		if(cart_size)
	        backup_cart_slot_rom_to_file(filename, cart_size);
		else
			printf("Cart size not detected!\n");
    }

    if(options & OPTION_BACKUP_SAVEGAME)
    {
        backup_cart_sram_to_file("savegame.sram", 4*1024*1024);//assume 4mbit SRAM
        //backupCartSaveGame();
    }

    if(options & OPTION_RESTORE_SAVEGAME)
    {
        write_file_to_cart_sram(saveFilename);
        //restoreSaveGameToCart(saveFilename);
    }

/*    if(options & OPTION_MICROSD)
    {
        queryMicroSD();
    }*/

	printf("\n");
	return 0;
}

