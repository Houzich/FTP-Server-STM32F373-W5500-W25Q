#include "FLASH.h"
#include "W25Q.h"
#include "stm32f3xx_hal.h"
#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FLASH_EraseInitTypeDef EraseInitStruct;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*###############################################################*/
/*###############################################################* Write_Word_Flash -->*/
/*###############################################################*/
uint32_t Write_Word_Flash(uint32_t address, uint32_t word){
		uint32_t PageError;
	
		/* Unlock the Flash to enable the flash control register access *************/ 
	  HAL_FLASH_Unlock();
    /* Fill EraseInit structure************************************************/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = address;
    EraseInitStruct.NbPages     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){HAL_FLASH_Lock(); return ERROR_ERASE_FLASH;}/* Error occurred while erase page in Flash memory.*/

    /* FLASH Word program of word at addresses*/
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word) != HAL_OK){HAL_FLASH_Lock(); return ERROR_WRITE_FLASH;}/* Error occurred while writing data in Flash memory.*/

    /* Check the correctness of written data */
      if((*(__IO uint32_t*) address) != word){HAL_FLASH_Lock(); return ERROR_CHECK_FLASH;}
			
	 HAL_FLASH_Lock();			
return NO_ERROR_FLASH;
}

/*###############################################################*/
/*###############################################################* Read_Word_Flash -->*/
/*###############################################################*/
uint32_t Read_Word_Flash(uint32_t address){
return (*(__IO uint32_t*) address);
}

/*###############################################################*/
/*###############################################################* Write_Block_Flash -->*/
/*###############################################################*/
uint32_t Write_Block_Flash(uint32_t size, uint32_t address_flash, uint32_t *address_data){
		uint32_t PageError,NbPages; 
	__IO uint32_t Addr_flash; 
	__IO uint32_t *Addr_data;
	
		/* Unlock the Flash to enable the flash control register access *************/ 
	  HAL_FLASH_Unlock();

		NbPages=(uint32_t)(address_flash+size + FLASH_PAGE_SIZE-1)/FLASH_PAGE_SIZE-(uint32_t)address_flash/FLASH_PAGE_SIZE;
    /* Fill EraseInit structure************************************************/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = address_flash;
    EraseInitStruct.NbPages     = NbPages;

	
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){HAL_FLASH_Lock(); return ERROR_ERASE_FLASH;}/* Error occurred while erase page in Flash memory.*/

		/* FLASH Block program */
    Addr_flash = address_flash;
		Addr_data = address_data;	
    while (Addr_flash < address_flash+size)
    {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Addr_flash, *Addr_data) == HAL_OK){Addr_flash += 4;Addr_data += 1;}
      else {HAL_FLASH_Lock(); return ERROR_WRITE_FLASH;}/* Error occurred while writing data in Flash memory.*/
    }
			
    /* Check the correctness of written data */
    Addr_flash = address_flash;
		Addr_data = address_data;	
    while (Addr_flash < address_flash+size)
    {
      if((*(__IO uint32_t*) Addr_flash) != *Addr_data){HAL_FLASH_Lock(); return ERROR_CHECK_FLASH;}
      Addr_flash += 4; Addr_data += 1;
    }	

	 HAL_FLASH_Lock();		
return NO_ERROR_FLASH;
}

/*###############################################################*/
/*###############################################################* Read_Block_Flash -->*/
/*###############################################################*/
void Read_Block_Flash(uint32_t size, uint32_t address_flash, uint32_t *address_data){
	__IO uint32_t Addr_flash; 
	__IO uint32_t *Addr_data;

    Addr_flash = address_flash;
		Addr_data = address_data;	
    while (Addr_flash < address_flash+size)
    {
			*Addr_data=*(__IO uint32_t*) Addr_flash;
      Addr_flash += 4; Addr_data += 1;
    }	
}
/*###############################################################*/
/*###############################################################* Reset_MCU -->*/
/*###############################################################*/
void Reset_MCU(void)
{

	
}
/*###############################################################*/
/*###############################################################* JumpToAddrFlash -->*/
/*###############################################################*/
void JumpToAddrFlash(uint32_t addr) {
    void (*SysMemJump)(void);

    HAL_RCC_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
 
    __disable_irq();
    
    __HAL_SYSCFG_REMAPMEMORY_FLASH();
    
    SysMemJump = (void (*)(void)) (*((uint32_t *)(addr + 4)));

    __set_MSP(*(uint32_t *)addr);
		
    SysMemJump();
}

/*###############################################################*/
/*###############################################################* Write_Bootloader_Flash -->*/
/*###############################################################*/
uint32_t Write_Bootloader_Flash(uint32_t address_flash, uint32_t start_4kb_w25q, uint8_t *buff, uint32_t size_buff){
		uint32_t PageError,NbPages;
		uint32_t Size_File;	
		uint8_t Size_File_Arr[6];
	__IO uint32_t Addr_W25Q=0x1000*start_4kb_w25q;
	__IO uint32_t Addr_flash = address_flash;
	
		Size_File_Arr[5]=0;
		W25Q_read(Addr_W25Q,&Size_File_Arr[0],5);
		Addr_W25Q+=0x100;
		Size_File=atoi((const char *)Size_File_Arr);
#ifdef _FLASH_DEBUG_
    printf("\r\nBOOT LEN:%d\r\n",Size_File);
#endif	
		if(!Size_File){return ERROR_WRITE_FLASH;}
		
		/* Unlock the Flash to enable the flash control register access *************/ 
	  HAL_FLASH_Unlock();

		NbPages=(uint32_t)(address_flash + Size_File + FLASH_PAGE_SIZE-1)/FLASH_PAGE_SIZE-(uint32_t)address_flash/FLASH_PAGE_SIZE;
    /* Fill EraseInit structure************************************************/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = address_flash;
    EraseInitStruct.NbPages     = NbPages;

	
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){HAL_FLASH_Lock(); return ERROR_ERASE_FLASH;}/* Error occurred while erase page in Flash memory.*/

		/* FLASH program */		
		uint32_t Size_Write_Block=0;
		while(Size_File>0)
		{
			if(Size_File>=size_buff){Size_Write_Block=size_buff;}else{Size_Write_Block=Size_File;}
			W25Q_read(Addr_W25Q,buff,Size_Write_Block);
			
			if (Write_Block_Flash(Size_Write_Block, Addr_flash, (uint32_t *)buff) == NO_ERROR_FLASH)
			{
				Addr_W25Q+=Size_Write_Block;
				Addr_flash+=Size_Write_Block;
				Size_File-=Size_Write_Block;
			}
			else 
			{
				#ifdef _FLASH_DEBUG_				
				printf("\r\nERROR WRITE FLASH\r\n");
				#endif					
				HAL_FLASH_Lock(); 
				return ERROR_WRITE_FLASH;
			}
		}
	 HAL_FLASH_Lock();

#ifdef _FLASH_DEBUG_				
printf("\r\nNO ERROR WRITE FLASH\r\n");
#endif				
return NO_ERROR_FLASH;
}
