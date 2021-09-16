
#ifndef GLOBALDATA_U2_H
#define GLOBALDATA_U2_H
#include <stdint.h>
#include "wizchip_conf.h"

#define DATA_BUF_SIZE   2048
extern uint8_t FTP_DBUF[512];


typedef enum FTP_STATUS_SAVE_FILE {
	//NETWORK_FTP_SAVE_NONE =0,
	SAVE_NONE =0,
	CHECK_FILE,
	ERASE_FIRST_SECTOR_Q25W,
	SAVE_FIRST_PACKET,
	ERASE_NEXT_SECTOR_Q25W,
	SAVE_NEXT_PACKET,
	WAIT_NEXT_PACKET,
}FTP_STATUS_SAVE_FILE;


struct  _NETWORK {
	/*MCU*/
	uint16_t f_Download_Firmware;
	/*Ethernet*/
	wiz_NetInfo gWIZNETINFO;
	/* IP*/

	/*UDP*/
	struct  UDP {
	 uint8_t  Dest_Ip[4];
   uint16_t Dest_Port;
	 uint16_t Len;
	 uint8_t *p_Buf;		
	}__attribute__ ((__packed__)) UDP;	

	/*FTP*/
	struct  FTP {
	 uint8_t  Password[8];
	 uint8_t  Login[8];
	 FTP_STATUS_SAVE_FILE  fl_Save_File;
	 uint32_t  adr_Start_Save_File_W25Q;
	 uint32_t  adr_Save_File_W25Q;
	 uint32_t  Num_Sector_Erase_W25Q;			
	 uint32_t  Size_Save_File_W25Q;		
	}__attribute__ ((__packed__)) FTP;

	
uint8_t RX_HTTP_BUF[DATA_BUF_SIZE];
uint8_t TX_HTTP_BUF[DATA_BUF_SIZE];
//uint8_t TX_UDP_BUF[DATA_BUF_SIZE];	
//uint8_t FTP_DBUF[512];	
};
extern struct _NETWORK NETWORK;






#define ADDR_FLASH_PAGE_BOOTLOADER  	ADDR_FLASH_PAGE_120
#define ADDR_4KB_W25Q_FIRMWARE_U1		0
#define ADDR_4KB_W25Q_FIRMWARE_U2		65
#define ADDR_4KB_W25Q_BOOTLOADER_U1		130
#define ADDR_4KB_W25Q_BOOTLOADER_U2		135
#define ADDR_4KB_W25Q_ADDR_SITE_U2		141
#define ADDR_4KB_W25Q_SITE_U2			143

#endif /* GLOBALDATA_U2_H */















