#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "libusb-1.0/libusb.h"


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

#define ENDPOINT_BULK_IN 0x81 /* endpoint 0x81 address for IN */
#define ENDPOINT_BULK_OUT 0x02 /* endpoint 1 address for OUT */

#define USB_RXTX_SIZE 64
#define USB_TIMEOUT 2000



// options
#define OPTION_INFO             0x01
#define OPTION_BACKUP           0x02
#define OPTION_WRITE            0x04
#define OPTION_VERIFY           0x08
#define OPTION_PROTECT          0x10
#define OPTION_BACKUP_SAVEGAME  0x20
#define OPTION_RESTORE_SAVEGAME 0x40
#define OPTION_MICROSD          0x80
char *filename = NULL;
char *verifyFilename = NULL;

char *saveFilename = NULL;

uint8_t unlockBypassMode = 0;
uint32_t write_N = 64*8;//for 64xN mode, this needs to be a multiple of 64
uint32_t skipChip = 0xFFFFFFFF;//if set to 0 or 1, skip operations on that chip

uint32_t ws1_addr = 0x5555;
uint32_t ws2_addr = 0x2AAA;
uint32_t ws3_addr = 0x5555;

//static char get_keystroke(void);


uint32_t bootBlockStartAddr[2]={0};
char cartRomName[14];
uint8_t prot[2][35], manufID[2]={0}, deviceID[2]={0}, factoryProt[2]={0}, bootBlockStartNum[2]={0}, chipSize[2]={0};

void erase_cart();


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
 //#error unknown platform
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
    unsigned char blockNum, i;

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
    unsigned char bootBlockStartNum = getBootBlockStart(chipSize);
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

void build_linkmasta_led(uint8_t blinkCount)
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
        printf("\build_linkmasta_led: libusb_bulk_transfer fail\n");
	fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS)
            printf("build_linkmasta_led: result = 0x%02X\n", result);
            fflush(stdout);
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
        printf("\ncheck_firmware_version: received %d byted, expected 64\n", num);
        fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_getversion_reply(buf, majVer, minVer);
    }
}


void flash_write_32_bytes(uint32_t addr, uint8_t *data, uint8_t chip)
{
    int num, r;
    build_flash_write_32_command(buf, addr, data, chip, unlockBypassMode);
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

void flash_write_N_bytes(uint32_t addr, uint8_t *data, uint8_t chip, uint8_t n)
{
    int num, r;
    build_flash_write_N_command(buf, addr, data, chip, n, unlockBypassMode);

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

void flash_write_64xN_bytes(uint32_t addr, uint8_t *data, uint8_t chip, uint8_t numPackets)
{
    int num, retVal, r;
    uint8_t packetsSent = 0;
    build_flash_write64xN_command(buf, addr, chip, numPackets, unlockBypassMode);

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

        get_flash_write64xN_reply(buf, &result, &packetsProcessed);

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


void write_byte(uint32_t addr, uint8_t data, uint8_t chip)
{
    int num, r;
    build_write_command(buf, addr, data, chip);

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
        printf("\nwrite_byte: libusb_bulk_transfer fail (addr=0x%X)\n", addr);
        exit(-1);
    }
    else
    {
        uint8_t result;

        get_result_reply(buf, &result);

        if(result != MSG_RESULT_SUCCESS) {
            printf("write_byte: result = 0x%02X\n", result);
            fflush(stdout);
        }
    }
}

uint8_t reset_to_dfu_mode(void)
{
    int num, r;
    build_dfu_reset_command(buf);
    
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
        printf("\reset_to_dfu_mode: libusb_bulk_transfer fail\n");
        fflush(stdout);
        exit(-1);
    }
    else
    {
        uint8_t result;
        
        get_result_reply(buf, &result);
        
        if(result != MSG_RESULT_SUCCESS)
        {
            printf("reset_to_dfu_mode: result = 0x%02X\n", result);
            return 0;
        }
        fflush(stdout);
    }
    
    return 1;
}


uint8_t read_byte(uint32_t addr, uint8_t chip)
{
    int num, r;
    build_read_command(buf, addr, chip);

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

        if(!get_read_reply(buf, &returnAddr, &data))
        {
            printf("read_byte: get_read_reply failure\n");
            exit(-1);
        }
        return data;
    }

    return 0;
}

uint8_t *read_64_bytes(uint32_t addr, uint8_t chip)
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

uint8_t read_64xN_bytes(uint8_t *readBuf, uint32_t addr, uint8_t chip, uint8_t n)
{
    int num, r;
    int i;
    build_read64xN_command(buf, addr, chip, n);

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

int write_file_to_savemasta(char *filename)
{
    FILE *f = fopen(filename, "rb");
    int num=0, total=0;
    uint32_t addr;
    uint8_t dataBuf[32];
    uint8_t chip=0;
    time_t startTime;


    if (!f)
    {
        printf("File not found!\n");
        return 0;
    }

    startTime = time(NULL);

    addr=0;


    while(addr<0x400000)
    {
        num = fread(dataBuf, 1, 1, f);
        if(num != 1)
            break;

        if((addr % 0x100) == 0)
            printf("\rWriting... 0x%06X", addr);

        int tries=0;

        while(read_byte(addr&0x1FFFFF, chip) != dataBuf[0])
        {
            write_byte(addr&0x1FFFFF, dataBuf[0], chip);
            if(tries > 1000)
            {
                printf("\nProblem writing addr 0x%X\n", addr);
                exit(-1);
            }
        }

        total+=1;
        addr+=1;
    }
    printf("\n");

    fclose(f);
    printf("Wrote %d bytes in %d seconds\n", total, time(NULL)-startTime);
    return total;
}

int backup_savemasta_to_file(char *bakFile, uint32_t size)
{
    uint8_t currChip = 0;
    uint32_t addr;
    uint32_t percent = (size-0x100)/100;
    uint8_t readBuf[64*COUNT_64xN];
    time_t sTime = time(NULL);

    FILE *f = fopen(bakFile, "rb");
    if (f)
    {
        fclose(f);
        printf("Overwrite file with backup?\n");
        if (tolower(getchar()) != 'y') return 0;
    }
    f = fopen(bakFile, "wb");
    if (!f) { printf("Can't open file!\n"); return -1; }

    int goFast = 0;

    // backup
    if(goFast)
    {
        for (addr=0; addr<size; addr+=(64*COUNT_64xN))
        {
            printf("\rBacking up... 0x%06X (%d%%)", addr, addr/percent);

            read_64xN_bytes(readBuf, addr&0x1FFFFF, currChip, COUNT_64xN);//64bytes better not go over a chip boundary
            fwrite(readBuf, COUNT_64xN, 64, f);
        }
    }
    else
    {
        for (addr=0; addr<size; addr++)
        {
            printf("\rBacking up... 0x%06X (%d%%)", addr, addr/percent);

            readBuf[0] = read_byte(addr&0x1FFFFF, currChip);
            fwrite(readBuf, 1, 1, f);
        }
    }

    printf("\r(%d%%) Backed up %d bytes in %d seconds             \n", addr/percent, addr, time(NULL)-sTime);

    fclose(f);
    return 0;
}


void printUsage(char *thisFile)
{
    printf("\n");
    printf("Usage: %s [<option(s)>]\n", thisFile);
    printf("\n");
    printf("  /?             Shows this help screen.\n");
    printf("  /i             Displays cartridge info.\n");
    printf("  /p             Displays cartridge protected blocks.\n");
    printf("  /b <filename>  Backup/Dump the cartridge to the file.\n");
    printf("  /w <filename>  Write the specified file to the cart.\n");
    printf("  /v <filename>  Verify that the specified file is the same as the cart.\n");
    printf("  /s             Backup official cart save-game.\n");
    printf("  /r <fname.ngf> Restores save-game.\n");
    printf("  /d 0xADDR      Dumps the single byte at addr\n");
    printf("  /l 0xADDR 0xDA Load single byte to addr (not necessarily flashing to cart)\n");
    printf("  /u             Set unlock bypass write mode (faster for Flashmasta cart)\n");
//    printf("  /j             Select Spansion S29A016J chip mode\n");
    printf("  /n n           Set write_N byte chunks.  Use either 1-32 or a multiple of 64.\n");
    printf("  /k 0|1         In write mode, skip chip 0 or 1.\n");
    printf("  /e             Erase cartridge (do nothing else).\n");
    printf("  /f             Firmware update mode (reset cart to DFU)\n");
    printf("  /z b <filename>  Backup from Savemasta-slot memory card to file.\n");
    printf("  /z w <filename>  Write file to Savemasta-slot memory card.\n");
    printf("\n");
}

uint8_t parseCmdLine(int argc, char *argv[])
{
    uint8_t options = 0;
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
                    exit(1);
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
                    uint8_t chip = 0;
                    a++;
                    sscanf(argv[a], "%X", &addr32);
                    if(addr32 >= 0x400000)
                        break;
                    if(addr32 >= 0x200000)
                    {
                        addr32 -= 0x200000;
                        chip = 1;
                    }
                    printf("Byte at address 0x%06X is 0x%02X\n", addr32, read_byte(addr32, chip));
                    exit(1);
                    break;
                }
                case 'l':
                {
                    uint32_t addr32, data;
                    uint8_t chip = 0;
                    a++;
                    sscanf(argv[a], "%X", &addr32);
                    a++;
                    sscanf(argv[a], "%X", &data);
                    if(addr32 >= 0x400000)
                        break;
                    if(addr32 >= 0x200000)
                    {
                        addr32 -= 0x200000;
                        chip = 1;
                    }
                    printf("Loading 0x%02X to address 0x%06X\n", data, addr32);
                    write_byte(addr32, data, chip);
                    exit(1);
                    break;
                }
                case 'j':
                    ws1_addr = 0xAAA;
                    ws2_addr = 0x555;
                    ws3_addr = 0xAAA;
                    write_N = 1;
                    break;
                case 'u':
                    unlockBypassMode = 1;
                    break;
                case 'n':
                    a++;
                    sscanf(argv[a], "%d", &write_N);
                    break;
                case 'f':
                    printf("Resetting cart to device firmware update mode.\n");
                    if(reset_to_dfu_mode())
                        printf("  Reset Successful!\n");
                    else
                        printf("  Reset FAILED (maybe old device/firmware)\n");
                    exit(1);
                    break;
                case 'k':
                    a++;
                    sscanf(argv[a], "%d", &skipChip);
                    break;

                case 'e':
                    erase_cart();
                    exit(1);

                case 'z':
                    printf("Savemasta mode!\n");
                    ++a;
                    switch(argv[a][0])
                    {
                        case 'b':
                            backup_savemasta_to_file(argv[++a], 16*1024);
                            break;
                        case 'w':
                            write_file_to_savemasta(argv[++a]);
                            break;
                        default:
                            printf("Unknown Savemasta mode %s, exiting!\n", argv[a]);
                            break;
                    }
                    exit(1);
                    break;
            }
        }
    }
    return options;
}

void erase_cart()
{
    uint8_t byte;

    int sleep = 0;

    if(skipChip != 0)
    {
        write_byte(ws1_addr, 0xAA, 0);
        usleep(sleep);
        write_byte(ws2_addr, 0x55, 0);
        usleep(sleep);
        write_byte(ws3_addr, 0x80, 0);
        usleep(sleep);
        write_byte(ws1_addr, 0xAA, 0);
        usleep(sleep);
        write_byte(ws2_addr, 0x55, 0);
        usleep(sleep);
        write_byte(ws3_addr, 0x10, 0);
        usleep(sleep);
    }

    if(skipChip != 1)
    {
        write_byte(ws1_addr, 0xAA, 1);
        usleep(sleep);
        write_byte(ws2_addr, 0x55, 1);
        usleep(sleep);
        write_byte(ws3_addr, 0x80, 1);
        usleep(sleep);
        write_byte(ws1_addr, 0xAA, 1);
        usleep(sleep);
        write_byte(ws2_addr, 0x55, 1);
        usleep(sleep);
        write_byte(ws3_addr, 0x10, 1);
        usleep(sleep);
    }

    if(skipChip != 0 && skipChip != 1)
    {
        uint8_t byte2;
        printf("Waiting for chips to be erased.\n");
        fflush(stdout);
        
        do{
            byte = read_byte(0, 0);
            byte2 = read_byte(0, 1);
            printf("\rchip0 byte 0x000000 = 0x%02X / chip1 byte 0x000000 = 0x%02X", byte, byte2);
            fflush(stdout);
        } while ((~byte & 0x80) || (~byte2 & 0x80));
    }
    else if(skipChip != 0)
    {
        printf("Waiting for chip 0 to be erased.\n");
        fflush(stdout);

        do{
          byte = read_byte(0, 0);
          printf("\rbyte 0x000000 = 0x%02X", byte);
          fflush(stdout);
        } while (~byte & 0x80);
    }
    else if(skipChip != 1)
    {
        printf("\nWaiting for chip 1 to be erased.\n");
        fflush(stdout);
        do{
          byte = read_byte(0, 1);
          printf("\rbyte 0x200000 = 0x%02X", byte);
          fflush(stdout);
        } while (~byte & 0x80);
    }

    printf("\n");
    fflush(stdout);
}

int write_file_to_cart(char *filename)
{
    FILE *f = fopen(filename, "rb");
    int num=0, total=0;
    uint32_t addr;
    uint8_t dataBuf[32];
    uint8_t chip=0;
    time_t startTime;

   uint32_t size;

    /* get the cart size.. */
    size = (chipSize[0] + chipSize[1]) * 0x20000;


    printf("\n");

    if (!f)
    {
        printf("File not found!\n");
        fflush(stdout);
        return 0;
    }

    startTime = time(NULL);
    //erase chips  FLASHALLERS
    erase_cart();

    addr=0;

    if(unlockBypassMode && write_N == 1)
    {
        printf("No verification in turbo mode!  Consider using verify mode after writing.\n");
        fflush(stdout);

        //while(addr<0x400000)
        while (addr < size)
        {
            if(chip==0 && addr==0)
            {
                if(chip != skipChip)
                {
                    write_byte(ws1_addr, 0xAA, chip);
                    write_byte(ws2_addr, 0x55, chip);
                    write_byte(ws3_addr, 0x20, chip);
                }
            }
            else if(chip==0 && addr>=0x200000)
            {
                if(chip != skipChip)
                {
                    write_byte(0, 0x90, chip);
                    write_byte(0, 0x00, chip);
                }

                chip = 1;

                if(chip != skipChip)
                {
                    write_byte(ws1_addr, 0xAA, chip);
                    write_byte(ws2_addr, 0x55, chip);
                    write_byte(ws3_addr, 0x20, chip);
                }
            }

            num = fread(dataBuf, 1, 1, f);
            if(num != 1)
                break;

            printf("\rWriting addr 0x%06X  ", addr);
            fflush(stdout);

            if(chip != skipChip)
            {
                write_byte(0, 0xA0, chip);
                write_byte(addr&0x1FFFFF, dataBuf[0], chip);
            }

            total+=1;
            addr+=1;
        }

        if(chip != skipChip)
        {
            write_byte(0, 0x90, chip);
            write_byte(0, 0x00, chip);
        }

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

        //while(addr<0x400000)
        while (addr < size)
        {
            if(chip==0 && addr>=0x200000)
                chip = 1;

            num = fread(dataBuf64xN, 1, write_N, f);
            printf("\rflash_write_64xN_bytes: addr=0x%06X chip=%d", addr&0x1FFFFF, chip);
            fflush(stdout);

            if(chip != skipChip)
                flash_write_64xN_bytes(addr&0x1FFFFF, dataBuf64xN, chip, write_N/64);

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
        //while(addr<0x400000)
        while (addr < size)
        {
            if(chip==0 && addr>=0x200000)
                chip = 1;

            num = fread(dataBuf, 1, 32, f);
            printf("\rflash_write_32_bytes: addr=0x%06X chip=%d", addr&0x1FFFFF, chip);
            fflush(stdout);

            if(chip != skipChip)
                flash_write_32_bytes(addr&0x1FFFFF, dataBuf, chip);
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

        //while(addr<0x400000)
        while (addr < size)
        {
            if(chip==0 && addr>=0x200000)
                chip = 1;

            num = fread(dataBuf, 1, write_N, f);
            printf("\rflash_write_N_bytes: addr=0x%06X chip=%d n=%d", addr&0x1FFFFF, chip, write_N);
            fflush(stdout);

            if(chip != skipChip)
                flash_write_N_bytes(addr&0x1FFFFF, dataBuf, chip, write_N);
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
        //while(addr<0x400000)
        while (addr < size)
        {
            int wait;
            if(chip==0 && addr>=0x200000)
                chip = 1;

            num = fread(dataBuf, 1, 1, f);
            if(num != 1)
                break;

            printf("\rWriting addr 0x%06X  ", addr);
            fflush(stdout);

            if(chip != skipChip)
            {
                write_byte(ws1_addr, 0xAA, chip);
                write_byte(ws2_addr, 0x55, chip);
                write_byte(ws3_addr, 0xA0, chip);
                write_byte(addr&0x1FFFFF, dataBuf[0], chip);
            }

            //make sure the data has been properly written
            wait = 2000;
            while(dataBuf[0] != read_byte(addr&0x1FFFFF, chip))
            {
                wait--;
                if(wait == 0)
                {
                    printf("\nWriting addr 0x%06X failed!\n", addr);
                    exit(1);
                }
            }

            total+=1;
            addr+=1;
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
    printf("Erased cart and wrote %d bytes in %d seconds\n", total, time(NULL)-startTime);
    fflush(stdout);
    return total;
}



int verify_file_with_cart(char *filename)
{
    FILE *f = fopen(filename, "rb");
    int i, bytes=0;
    uint8_t chip=0;
    time_t startTime;
    uint32_t /*blockAddr,*/ blockLen, addr=0;
    uint8_t fileBytes[64], *cartBytes;
    uint8_t unprotDiffered = 0;
    int memcmpRetval = 0;



    if (!f)
    {
        printf("File not found! (%s)\n", filename);
        return 0;
    }

    startTime = time(NULL);

    printf("Verifying...");
    fflush(stdout);

    for(chip = 0; chip<2;chip++)
    {
        if(!chipSize[chip]) //no chip here
            continue;

        for(i=0;i<35;i++)
        {
            //blockAddr = blockNumToAddr(chip, i);//where the NGPC sees this data
            blockLen = blockSize(i);

            while(blockLen)
            {
                printf("\rVerifying address 0x%06X", addr);
                fflush(stdout);

                bytes = fread(fileBytes, 1, 64, f);
                if(bytes != 64)
                {
                    fprintf(stderr, "\nverify: end of file reached at addr 0x%06X\nNo differences found, but cart was larger than file\n", addr+bytes);
                    fclose(f);
                    return 1;
                }

                cartBytes = read_64_bytes(addr, chip);

                memcmpRetval = memcmp(cartBytes, fileBytes, 64);

                if(memcmpRetval != 0)
                {
                    int deeperInspectionPassed = 1;
                    if(prot[chip][i]==1)
                    {
                        int j;
                        for(j=0;j<64;j++)
                            if(cartBytes[j] != fileBytes[j])
                            {
                                fprintf(stderr, "\nverify: mismatch at addr 0x%06X file=0x%02X cart=0x%02X", addr+j, fileBytes[j], cartBytes[j]);
                                cartBytes[j] = read_byte((addr+j)&0x1FFFFF, (addr+j)>0x1FFFFF ? 1 : 0);
                                if(cartBytes[j] == fileBytes[j])
                                    fprintf(stderr, "\n  but deeper verification passed!");
                                else
                                    deeperInspectionPassed = 0;
                            }
                        printf("\n");
                        if(deeperInspectionPassed == 0)
                        {
                            fclose(f);
                            return 0;
                        }
                    }
                    if(prot[chip][i]==0)
                        unprotDiffered = 1;
                }
/*
                {
                    int j;
                    cartBytes = malloc(64);
                    for(j=0;j<64;j++)
                    {
                        cartBytes[j] = read_byte((addr+j), chip);
                        if(cartBytes[j] != fileBytes[j])
                        {
                            fprintf(stderr, "\nverify: mismatch at addr 0x%06X file=0x%02X cart=0x%02X p=%d c=%d i=%d", addr+j, fileBytes[j], cartBytes[j], prot[chip][i], chip, i);

                            if(prot[chip][i]==1)
                            {
                                fclose(f);
                                return 0;
                            }

                            if(prot[chip][i]==0)
                                unprotDiffered = 1;
                        }
                    }
                    free(cartBytes);
                }
*/
                addr+=64;
                blockLen-=64;
            }
        }
    }

    if(unprotDiffered)
    {
        printf("\rUnprotected blocks differed, but protected blocks verified.  This may be due to changes in savegame data.\n");
        printf("Verified %d bytes in %d seconds.\n", addr, time(NULL)-startTime);
        fflush(stdout);
    }
    else
    {
        printf("\rSuccess!  Verified %d bytes in %d seconds.\n", addr, time(NULL)-startTime);
        fflush(stdout);
    }

    fclose(f);

    return 1;
}


int backup_cart_to_file(char *bakFile, uint32_t size)
{
    uint8_t currChip = 0;
    uint32_t addr;
    uint32_t percent = (size-0x100)/100;
    uint8_t readBuf[64*COUNT_64xN];
    time_t sTime = time(NULL);

    FILE *f = fopen(bakFile, "rb");
    if (f)
    {
        fclose(f);
        printf("Overwrite file with backup?\n");
        if (tolower(getchar()) != 'y') return 0;
    }
    f = fopen(bakFile, "wb");
    if (!f) { printf("Can't open file!\n"); return -1; }

    // backup
    for (addr=0; addr<size; addr+=(64*COUNT_64xN))
    {
        if(currChip == 0 && addr >= 0x200000)
            currChip = 1;
        printf("\rBacking up... 0x%06X (%d%%)", addr, addr/percent);
        fflush(stdout);

        read_64xN_bytes(readBuf, addr&0x1FFFFF, currChip, COUNT_64xN);//64bytes better not go over a chip boundary
        fwrite(readBuf, COUNT_64xN, 64, f);
    }
    printf("\r(%d%%) Backed up %d bytes in %d seconds             \n", addr/percent, addr, time(NULL)-sTime);
    fflush(stdout);

    fclose(f);
    return 0;
}




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

                read_64xN_bytes(readBuf, addr&0x1FFFFF, 0/*chip*/, 1);//64bytes better not go over a block boundary

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

                    read_64xN_bytes(readBuf, addr&0x1FFFFF, 1/*chip*/, 1);//64bytes better not go over a block boundary

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
            /*write_byte(ws1_addr, 0xAA, chip);
            write_byte(ws2_addr, 0x55, chip);
            write_byte(ws3_addr, 0xA0, chip);

            if((count&0xFF) == 0)
                printf("\rRestoring savegame... 0x%06X (%d%%)", addr, count/percent);

            write_byte(addr&0x1FFFFF, blocks[count++], chip);*/
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






int Dump32MbitCart()
{
    uint8_t readBuf[64];
    uint32_t addr=0;
    int counter = 0;
    time_t sTime = time(NULL);


	uint8_t sendCommand[64];
	int r, num;
	char bakFile[] = "testdump.ngp";

    printf("opening %s\n", bakFile);
    FILE *f = fopen(bakFile, "rb");
    if (f)
    {
        fclose(f);
        printf("Overwrite file with backup?\n");
        fflush(stdout);
        if (tolower(getchar()) != 'y') return 0;
    }
    f = fopen(bakFile, "wb");
    if (!f) { printf("Can't open file!\n"); return -1; }

    sendCommand[0] = 0x08;
    printf("calling libusb_bulk_transfer\n");
    fflush(stdout);
	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, sendCommand, 64, &num, 1000);
    if(r<0)
	{
        print_libusb_error(r);
        exit(-1);
	}

    // backup
    for (addr=0; addr<0x400000; addr+=64)
    {
        if(addr%0x1000 == 0) {
            printf("\rBacking up... 0x%06X", addr);
            fflush(stdout);
        }

        r=libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, buf, 64, &num, 1000);
        if(r<0)
        {
            print_libusb_error(r);
            return -1;
        }
        if(num != 64) {
            printf("\nRead Mismatch: %d bytes read\n", num);
            fflush(stdout);
        }
        fwrite(readBuf, 1, 64, f);
    }
    printf("\rBacked up %d bytes in %d seconds\n", addr, time(NULL)-sTime);
    fflush(stdout);

    fclose(f);

    return 0;
}



/**************************
   Start Micro SD functions
**************************/
uint8_t in_SD_SPI_mode = 0;

uint8_t SD_SPI_send_byte(uint8_t sendByte)
{
    uint8_t recvByte = 0, i;
//    static uint8_t bitBuffer = 1;
    uint32_t addr=0;

    if(!in_SD_SPI_mode)
        return 0;

    for(i=0;i<8;i++)
    {
        if(sendByte & 0x80)
            addr = 0x000001;
        else
            addr = 0x000000;

        recvByte <<= 1;

        if(read_byte(addr, 0))
            recvByte |= 0x01;

        write_byte(addr, 0x00, 0);

        sendByte <<= 1;
    }

    return recvByte;
}

void printBuf(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    printf("0x");
    for(i=0; i<len; i++)
    {
        printf("%02X", buf[i]);
    }
}

void SD_SPI_transmit(uint8_t *sendBuffer, uint8_t *recvBuffer, uint8_t len)
{
    uint8_t i;
    uint8_t sendByte;

    printf("Sending:");
    printBuf(sendBuffer, len);
    printf(" ");

    for(i=0; i<len; i++)
    {
        recvBuffer[i] = SD_SPI_send_byte(sendBuffer[i]);
    }
    printf(" Received:");
    printBuf(recvBuffer, len);
    printf("\n");
}

uint8_t SD_SPI_wait_for_response(uint8_t response)
{
    uint8_t wait = 0xFF;
    while(SD_SPI_send_byte(0xFF) != response)
    {
        if(wait-- == 0)
        {
            printf("ERROR 0x41!\n");
            return 0;
        }
    }
    return 1;
}

void enter_SD_SPI_mode(void)
{
    write_byte(ws1_addr, 0xAA, 0);
    write_byte(ws2_addr, 0x55, 0);
    write_byte(ws3_addr, 0xF0, 0);

    write_byte(ws1_addr, 0xAA, 0);
    write_byte(ws2_addr, 0x55, 0);
    write_byte(ws3_addr, 0xED, 0);

    in_SD_SPI_mode = 1;
}

void exit_SD_SPI_mode(void)
{
    write_byte(ws3_addr, 0xF0, 0);

    in_SD_SPI_mode = 0;
}


void queryMicroSD(void)
{
    uint8_t i, sendBuf[256], recvBuf[256];

    enter_SD_SPI_mode();

    //init SD SPI with 10 0xFF's
    for(i=0;i<10;i++)
    {
        SD_SPI_send_byte(0xFF);
    }

    //send reset command with checksum 0x95
    SD_SPI_transmit("\x40\x00\x00\x00\x00\x95", recvBuf, 6);
    if(!SD_SPI_wait_for_response(0x01))
    {
        printf("ERROR 0x40!\n");
        return;
    }

    //send reset command with checksum faked 0xFF
    SD_SPI_transmit("\x41\x00\x00\x00\x00\xFF", recvBuf, 6);
    if(!SD_SPI_wait_for_response(0x00))
    {
        printf("ERROR 0x41!\n");
        return;
    }

    SD_SPI_transmit("\x77\x00\x00\x00\x00\x95\xFF\xFF", recvBuf, 8);

    SD_SPI_transmit("\x69\x00\x00\x00\x00\x95\xFF\xFF", recvBuf, 8);

    exit_SD_SPI_mode();
}
/**************************
   End Micro SD functions
**************************/

void close_libusb(void)
{
    //blink the LED 2 times to signal that we're exiting
    build_linkmasta_led(2);

	libusb_release_interface(devh, 0);
    libusb_reset_device(devh);
	libusb_close(devh);
	libusb_exit(NULL);
}

int main(int argc, char *argv[])
{
    uint8_t i, currChip=0, majVer, minVer;
    uint32_t addr;

	uint8_t options;

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
    //printf("Successfully found device\n");

	libusb_detach_kernel_driver(devh, 0);

	r = libusb_set_configuration(devh, 1);
	if (r < 0) {
		fprintf(stderr, "libusb_set_configuration error %d\n", r);
		exit(-1);
	}
	//printf("Successfully set usb configuration 1\n");

	r = libusb_claim_interface(devh, 0);
	if (r < 0) {
		fprintf(stderr, "libusb_claim_interface error %d\n", r);
		exit(-1);
	}
	//printf("Successfully claimed interface\n");

    atexit(close_libusb);


    check_firmware_version(&majVer, &minVer);
    if(majVer != MAJOR_VERSION || minVer != MINOR_VERSION)
    {
        printf("Version mismatch!  Expected v%d.%02d  Firmware returned v%d.%02d.  Exiting.\n", MAJOR_VERSION, MINOR_VERSION, majVer, minVer);
        return -1;
    }

    options = parseCmdLine(argc, argv);

    memset(prot[0], 0x01, 35);
    memset(prot[1], 0x01, 35);

    for(currChip=0;currChip<2;currChip++)
    {

        write_byte(ws1_addr, 0xAA, currChip);
        write_byte(ws2_addr, 0x55, currChip);
        write_byte(ws3_addr, 0xF0, currChip);

        write_byte(ws1_addr, 0xAA, currChip);
        write_byte(ws2_addr, 0x55, currChip);
        write_byte(ws3_addr, 0x90, currChip);

//        printf(" 0x%02X\n", read_byte(0, currChip));//make sure we're back to readable state

        manufID[currChip] = read_byte(0, currChip);
        deviceID[currChip] = read_byte(1, currChip);
        factoryProt[currChip] = read_byte(3, currChip);

        if(manufID[currChip] == 0x90 && deviceID[currChip] == 0x90 /*&& factoryProt[currChip] == 0x90*/)
        {
            //this currChip is nonexistant
            continue;
        }

        switch(deviceID[currChip])
        {
            default:
                /*chipSize[currChip] = 0;
                bootBlockStartAddr[currChip] = 0;
                bootBlockStartNum[currChip] = 0;
                break;*/
            case 0x2F:  //16mbit
                chipSize[currChip] = 16;
                bootBlockStartAddr[currChip] = 0x1F0000;
                bootBlockStartNum[currChip] = 31;
                break;
            case 0x2C://8mbit
                chipSize[currChip] = 8;
                bootBlockStartAddr[currChip] = 0xF0000;
                bootBlockStartNum[currChip] = 15;
                break;
            case 0xAB://4mbit
                chipSize[currChip] = 4;
                bootBlockStartAddr[currChip] = 0x70000;
                bootBlockStartNum[currChip] = 7;
                break;
        }

        if(options & OPTION_INFO)
        {
            printf("\nChip #%d info:\n", currChip);
            printf(" ManufID  = 0x%02X\n", manufID[currChip]);
            printf(" DeviceID = 0x%02X\n", deviceID[currChip]);
            printf(" FactoryProt = 0x%02X\n", factoryProt[currChip]);
            printf("\n");
        }

        if(options & OPTION_PROTECT)
            printf("\nChip #%d block protection:\n", currChip);

        i=0;
        for(addr=0;addr<=bootBlockStartAddr[currChip];addr+=0x10000)
        {
            prot[currChip][i] = read_byte(addr+2, currChip);
            if(options & OPTION_PROTECT)
                printf(" Block %02d (0x%06X) %s\n", i, addr, prot[currChip][i]?"Protected":"Unprotected");
            i++;
        }
        addr-=0x10000;
        addr+=0x8000;
        prot[currChip][i] = read_byte(addr+2, currChip);
        if(options & OPTION_PROTECT)
            printf(" Block %02d (0x%06X) %s\n", i, addr, prot[currChip][i]?"Protected":"Unprotected");
        i++;
        addr+=0x2000;
        prot[currChip][i] = read_byte(addr+2, currChip);
        if(options & OPTION_PROTECT)
            printf(" Block %02d (0x%06X) %s\n", i, addr, prot[currChip][i]?"Protected":"Unprotected");
        i++;
        addr+=0x2000;
        prot[currChip][i] = read_byte(addr+2, currChip);
        if(options & OPTION_PROTECT)
            printf(" Block %02d (0x%06X) %s\n", i, addr, prot[currChip][i]?"Protected":"Unprotected");

        write_byte(ws1_addr, 0xAA, currChip);
        usleep(20);
        write_byte(ws2_addr, 0x55, currChip);
        usleep(20);
        write_byte(ws3_addr, 0xF0, currChip);
        usleep(20);
        usleep(1);
        read_byte(0, currChip);//make sure we're back to readable state
        usleep(20);
    }


    i=0;
    for(addr=0x24;addr<0x24+12;addr++)
        cartRomName[i++] = read_byte(addr, 0); //read the cart name from chip 1
    cartRomName[13]=0;

    printf("%dmbit cart detected - %s\n", chipSize[0]+chipSize[1], cartRomName);

    if(options & OPTION_BACKUP)
    {
        backup_cart_to_file(filename, (chipSize[0]+chipSize[1])*1024*1024/8);
    }

    if(options & OPTION_WRITE)
    {
        write_file_to_cart(filename);
    }

    if(options & OPTION_VERIFY)
    {
        verify_file_with_cart(verifyFilename);
    }

    if(options & OPTION_BACKUP_SAVEGAME)
    {
        backupCartSaveGame();
    }

    if(options & OPTION_RESTORE_SAVEGAME)
    {
        restoreSaveGameToCart(saveFilename);
    }

    if(options & OPTION_MICROSD)
    {
        queryMicroSD();
    }

	printf("\n");
	return 1;
}

