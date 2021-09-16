#ifndef W25Q_H_
#define W25Q_H_

#include <string.h>
#include <stdint.h>


enum partNumber {
    custom = -1,
    autoDetect = 0,
    W25Q80 = 1,
    W25Q16 = 2,
    W25Q32 = 4,
    W25Q64 = 8,
    W25Q128 = 16
};

//COMMANDS
#define W_EN 	0x06	//write enable
#define W_DE	0x04	//write disable
#define R_SR1	0x05	//read status reg 1
#define R_SR2	0x35	//read status reg 2
#define W_SR	0x01	//write status reg
#define PAGE_PGM	0x02	//page program
#define QPAGE_PGM	0x32	//quad input page program
#define BLK_E_64K	0xD8	//block erase 64KB
#define BLK_E_32K	0x52	//block erase 32KB
#define SECTOR_E	0x20	//sector erase 4KB
#define CHIP_ERASE	0xc7	//chip erase
#define CHIP_ERASE2	0x60	//=CHIP_ERASE
#define E_SUSPEND	0x75	//erase suspend
#define E_RESUME	0x7a	//erase resume
#define PDWN		0xb9	//power down
#define HIGH_PERF_M	0xa3	//high performance mode
#define CONT_R_RST	0xff	//continuous read mode reset
#define RELEASE		0xab	//release power down or HPM/Dev ID (deprecated)
#define R_MANUF_ID	0x90	//read Manufacturer and Dev ID (deprecated)
#define R_UNIQUE_ID	0x4b	//read unique ID (suggested)
#define R_JEDEC_ID	0x9f	//read JEDEC ID = Manuf+ID (suggested)
#define READ		0x03
#define FAST_READ	0x0b

#define SR1_BUSY_MASK	0x01
#define SR1_WEN_MASK	0x02

#define WINBOND_MANUF	0xef

#define DEFAULT_TIMEOUT 200


void W25Q_Select(void);
void W25Q_Deselect(void);
uint8_t W25Q_RxTx(uint8_t data);
uint16_t W25Q_ReadSR(void);
uint8_t W25Q_ReadManufacturer(void);
uint64_t W25Q_ReadUniqueID(void);
uint16_t W25Q_ReadPartID(void);
uint8_t W25Q_CheckPartNo(enum partNumber _partno);
uint8_t W25Q_Busy(void);
void W25Q_SetWriteEnable(uint8_t cmd);
long W25Q_bytes(void);
uint16_t W25Q_pages(void);
uint16_t W25Q_sectors(void);
uint16_t W25Q_blocks(void);
uint8_t W25Q_begin(enum partNumber _partno);
void W25Q_end(void);
uint16_t W25Q_read (uint32_t addr,uint8_t *buf, uint16_t n);
void W25Q_writePage(uint32_t addr_start,uint8_t *buf);
void W25Q_EraseSector(uint32_t addr_start);
void W25Q_Erase32kBlock(uint32_t addr_start);
void W25Q_Erase64kBlock(uint32_t addr_start);
void W25Q_EraseAll(void);
void W25Q_EraseSuspend(void);
void W25Q_EraseResume(void);
void W25Q_WriteBytes(uint32_t addr_start,uint8_t *buf,uint32_t size);
void W25Q_First_Start(void);
void W25Q_WriteSR(uint32_t cmd);
uint32_t W25Q_EraseSector_Wait_Busy(uint32_t addr);
uint32_t Write_Net_W25Q(uint8_t *data, uint32_t size_cmd);
uint32_t W25Q_WriteBytes_Wait_Busy(uint32_t addr_start,uint8_t *buf,uint32_t size);
uint32_t W25Q_EraseChip_Wait_Busy(void);
#endif /* W25Q_H_ */
