#include "main.h"
#include "W25Q.h"
#include "stm32f3xx_hal.h"
#include <stdlib.h>


void W25Q_Select(void)
{
HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_RESET);
}


void W25Q_Deselect(void)
{
HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_SET);
}

uint8_t W25Q_RxTx(uint8_t data)
{
#define SPI2_DR_8b          (*(__IO uint8_t *)((uint32_t)SPI2 + 0x0C))

        while (!((SPI2->SR) & SPI_SR_TXE)){};      
        SPI2_DR_8b = data; 
        while (!((SPI2->SR) & SPI_SR_RXNE)){};
				return SPI2_DR_8b;
}




void W25Q_First_Start(void)
{
W25Q_begin(W25Q32);
HAL_Delay(10);
W25Q_SetWriteEnable(W_EN);
HAL_Delay(10);
W25Q_WriteSR(0);
HAL_Delay(10);
//temp1 = W25Q_ReadSR();	
//HAL_Delay(10);
}

void W25Q_WriteSR(uint32_t cmd)
{
  W25Q_Select();
  W25Q_RxTx(W_SR);
	W25Q_RxTx(cmd);
  W25Q_RxTx(cmd>>8);
  W25Q_Deselect();
}

uint16_t W25Q_ReadSR(void)
{
  uint8_t r1,r2;
  W25Q_Select();
  W25Q_RxTx(R_SR1);
  r1 = W25Q_RxTx(0xff);
  W25Q_Deselect();
  W25Q_Deselect();//some delay
  W25Q_Select();
  W25Q_RxTx(R_SR2);
  r2 = W25Q_RxTx(0xff);
  W25Q_Deselect();
  return (((uint16_t)r2)<<8)|r1;
}


uint8_t W25Q_ReadManufacturer(void)
{
  uint8_t c;
  W25Q_Select();
  W25Q_RxTx(R_JEDEC_ID);
  c = W25Q_RxTx(0x00);
  W25Q_RxTx(0x00);
  W25Q_RxTx(0x00);
  W25Q_Deselect();
  return c;
}

uint64_t W25Q_ReadUniqueID(void)
{
  uint64_t uid;
  uint8_t *arr;
  arr = (uint8_t*)&uid;
  W25Q_Select();
  W25Q_RxTx(R_UNIQUE_ID);
  W25Q_RxTx(0x00);
  W25Q_RxTx(0x00);
  W25Q_RxTx(0x00);
  W25Q_RxTx(0x00);
  //for little endian machine only
  for(int i=7;i>=0;i--)
  {
    arr[i] = W25Q_RxTx(0x00);
  }
  W25Q_Deselect();
  return uid;
}

uint16_t W25Q_ReadPartID(void)
{
  uint8_t a,b;
  W25Q_Select();
  W25Q_RxTx(R_JEDEC_ID);
  W25Q_RxTx(0x00);
  a = W25Q_RxTx(0x00);
  b = W25Q_RxTx(0x00);
  W25Q_Deselect();
  return (a<<8)|b;
}

uint8_t W25Q_CheckPartNo(enum partNumber _partno)
{
  uint8_t manuf;
  uint16_t id;
  
  W25Q_Select();
  W25Q_RxTx(R_JEDEC_ID);
  manuf = W25Q_RxTx(0x00);
  id = W25Q_RxTx(0x00) << 8;
  id |= W25Q_RxTx(0x00);
  W25Q_Deselect();

//  Serial.print("MANUF=0x");
//  Serial.print(manuf,HEX);
//  Serial.print(",ID=0x");
//  Serial.print(id,HEX);
//  Serial.println();
  
  if(manuf != WINBOND_MANUF)
    return 0;
//  Serial.println("MANUF OK");

  if(_partno == custom)
    return 1;
//  Serial.println("Not a custom chip type");

//  if(_partno == autoDetect)
//  {
////    Serial.print("Autodetect...");
//    for(int i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
//    {
//      if(id == pgm_read_word(&(pnList[i].id)))
//      {
//        _partno = (enum partNumber)pgm_read_byte(&(pnList[i].pn));
//        //Serial.println("OK");
//        return 1;
//      }
//    }
//    if(_partno == autoDetect)
//    {
////      Serial.println("Failed");
//      return 0;
//    }
//  }

//  //test chip id and partNo
//  for(int i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
//  {
//    if(_partno == (partNumber)pgm_read_byte(&(pnList[i].pn)))
//    {
//      if(id == pgm_read_word(&(pnList[i].id)))//id equal
//        return 1;
//      else
//        return 0;
//    }
//  }
//  Serial.println("partNumber not found");
  return 0;//partNo not found
}


uint8_t W25Q_Busy(void)
{
  uint8_t r1;
  W25Q_Select();
  W25Q_RxTx(R_SR1);
  r1 = W25Q_RxTx(0xff);
  W25Q_Deselect();
  if(r1 & SR1_BUSY_MASK) return 1;
  return 0;
}

void W25Q_SetWriteEnable(uint8_t cmd)
{
  W25Q_Select();
  W25Q_RxTx( cmd ? W_EN : W_DE );
  W25Q_Deselect();
}


uint8_t W25Q_begin(enum partNumber _partno)
{
    W25Q_Select();
    W25Q_RxTx(RELEASE);
    W25Q_Deselect();
    HAL_Delay(5);//>3us
    //  Serial.println("Chip Released");

    //if(!W25Q_CheckPartNo(_partno)) return 0;
	return 1;
}

void W25Q_end(void)
{
  W25Q_Select();
  W25Q_RxTx(PDWN);
  W25Q_Deselect();
  HAL_Delay(5);//>3us
}

uint16_t W25Q_read (uint32_t addr,uint8_t *buf, uint16_t n)
{
  if(W25Q_Busy())
    return 0;
  
  W25Q_Select();
  W25Q_RxTx(READ);
  W25Q_RxTx(addr>>16);
  W25Q_RxTx(addr>>8);
  W25Q_RxTx(addr);
  for(uint16_t i=0;i<n;i++)
  {
    buf[i] = W25Q_RxTx(0x00);
  }
  W25Q_Deselect();
  
  return n;
}


void W25Q_writePage(uint32_t addr_start,uint8_t *buf)
{
	W25Q_SetWriteEnable(1);
  W25Q_Select();
  W25Q_RxTx(PAGE_PGM);
  W25Q_RxTx(addr_start>>16);
  W25Q_RxTx(addr_start>>8);
  W25Q_RxTx(addr_start);
  uint8_t i=0;
  do {
    W25Q_RxTx(buf[i]);
    i++;
  }while(i!=0);
  W25Q_Deselect();
}


void W25Q_EraseSector(uint32_t addr_start)
{
	W25Q_SetWriteEnable(1);
  W25Q_Select();
  W25Q_RxTx(SECTOR_E);
  W25Q_RxTx(addr_start>>16);
  W25Q_RxTx(addr_start>>8);
  W25Q_RxTx(addr_start);
  W25Q_Deselect();
}

void W25Q_Erase32kBlock(uint32_t addr_start)
{
	W25Q_SetWriteEnable(1);
  W25Q_Select();
  W25Q_RxTx(BLK_E_32K);
  W25Q_RxTx(addr_start>>16);
  W25Q_RxTx(addr_start>>8);
  W25Q_RxTx(addr_start);
  W25Q_Deselect();
}

void W25Q_Erase64kBlock(uint32_t addr_start)
{
	W25Q_SetWriteEnable(1);
  W25Q_Select();
  W25Q_RxTx(BLK_E_64K);
  W25Q_RxTx(addr_start>>16);
  W25Q_RxTx(addr_start>>8);
  W25Q_RxTx(addr_start);
  W25Q_Deselect();
}

void W25Q_EraseAll(void)
{
  W25Q_Select();
  W25Q_RxTx(CHIP_ERASE);
  W25Q_Deselect();
}

void W25Q_EraseSuspend(void)
{
  W25Q_Select();
  W25Q_RxTx(E_SUSPEND);
  W25Q_Deselect();
}

void W25Q_EraseResume(void)
{
  W25Q_Select();
  W25Q_RxTx(E_RESUME);
  W25Q_Deselect();
}

//bool begin(partNumber _partno,SPIClass &_spi,uint8_t _nss)
//{
//    spi = _spi;
//    nss = _nss;  
//    pinMode (nss, OUTPUT);
//    // pinMode(MISO,INPUT_PULLUP);
//    _spi.begin();
//    _spi.setBitOrder(MSBFIRST);
//    _spi.setClockDivider(SPI_CLOCK_DIV2);
//    _spi.setDataMode(SPI_MODE0);
//    deselect();

//  return winbondFlashClass::begin(_partno);
//}

//void winbondFlashSPI::end()
//{
//  winbondFlashClass::end();
//  spi.end();
//}

uint32_t Write_Net_W25Q(uint8_t *data, uint32_t size_cmd){
uint8_t *p;
uint8_t buff_temp[7];
uint32_t adr,start_KB,erase,num_packet,size_packet;
uint32_t ret=1;
	
buff_temp[6]=0;	//для atoi
p=data+size_cmd;
memcpy(buff_temp,p,4);
start_KB=atoi((const char *)buff_temp);
p+=4;
memcpy(buff_temp,p,4);
erase=atoi((const char *)buff_temp);
p+=4;
memcpy(buff_temp,p,4);
num_packet=atoi((const char *)buff_temp);
p+=4;	
memcpy(buff_temp,p,4);	
size_packet=atoi((const char *)buff_temp);	
p+=4;	

adr=0x1000*start_KB+0x100*num_packet;	

	if(erase==4){W25Q_EraseSector(adr);}
	else if (erase==32){W25Q_Erase32kBlock(adr);}
	else if (erase==64){W25Q_Erase64kBlock(adr);}


	if(erase)
		{
			for(int i=0;i<25;i++)
				{
					HAL_Delay(100);
					ret=W25Q_Busy();
					if(!ret)break;
				}
		}

W25Q_WriteBytes(adr,p,size_packet);
			for(int i=0;i<10;i++)
				{
					HAL_Delay(3);
					ret=W25Q_Busy();
					if(!ret)break;
				}
		
		
//for(int i=0;i<256;i++){buff[i]=5;}
//W25Q_read(adr,&buff[0],256);
return ret;
}

void W25Q_WriteBytes(uint32_t addr_start,uint8_t *buf,uint32_t size)
{
	W25Q_SetWriteEnable(1);
  W25Q_Select();
  W25Q_RxTx(PAGE_PGM);
  W25Q_RxTx(addr_start>>16);
  W25Q_RxTx(addr_start>>8);
  W25Q_RxTx(addr_start);
	int i=0;
	for(i=0; i<size; i++){W25Q_RxTx(buf[i]);}
	//if (i<256){for(int x=i; x<256; x++){W25Q_RxTx(0);}} //остальные заполняем нулями
  W25Q_Deselect();
}

uint32_t W25Q_EraseSector_Wait_Busy(uint32_t addr)
{
	uint32_t ret;
	
	W25Q_EraseSector(addr);
		for(int i=0;i<25;i++)
		{
			HAL_Delay(100);
			ret=W25Q_Busy();
			if(!ret)break;
		}
return ret;
}

uint32_t W25Q_EraseChip_Wait_Busy(void)
{
	uint32_t ret=0;
	uint32_t addr=0;

for(int i=0;i<20;i++)
	{
		W25Q_Erase64kBlock(addr);
			for(int i=0;i<25;i++)
			{
				HAL_Delay(100);
				ret=W25Q_Busy();
				if(!ret)break;
			}
		addr+=0x10000;
	}
return ret;
}


uint32_t W25Q_WriteBytes_Wait_Busy(uint32_t addr_start,uint8_t *buf,uint32_t size)
{
	uint32_t ret;
	uint32_t size_page;
	
	//uint32_t temp=0;	
		do
		{		
			uint32_t temp=addr_start-(addr_start/0x100)*0x100;
			if(size<(0x100-temp)){size_page=size;}else{size_page=0x100-temp;}
			W25Q_WriteBytes(addr_start,buf,size_page);
			for(int i=0;i<25;i++)
			{
				HAL_Delay(3);
				ret=W25Q_Busy();
				if(!ret)break;
			}
			size-=size_page;
			addr_start+=size_page;
			buf+=size_page;
		}	
	while(size);		
			
return ret;
}

