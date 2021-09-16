#include "main.h"
#include "stm32f3xx_hal.h"
#include "Network.h"
#include "GlobalData_U2.h"
#include "socket.h"
#include "dhcp.h"
#include "httpServer.h"
#include "httpParser.h"
#include "httpUtil.h"
#include "W25Q.h"
#include "ftpd.h"
#include "FLASH.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define _MAIN_DEBUG_

#define SOCK_UDPS        0
#define SOCK_DHCP				 1
#define SOCK_HTTP3			 4 
#define SOCK_HTTP4			 5
#define SOCK_HTTP5			 6
#define SOCK_HTTP6			 7

#define MAX_HTTPSOCK		 4




const char *web_page[]=
	{
		"Accounting.html",
		"Action.html",
		"Battery.html",
		"Calendar.html",
		"CalibrateContact.html",
		"CalibrateCurrentAC.html",
		"CalibrateCurrentDC.html",
		"CalibrateGround.html",
		"CalibrateResistance.html",
		"CalibrateSettings.html",
		"CalibrateVoltageAC.html",
		"CalibrateVoltageDC.html",
		"Cell.html",
		"ClimatMonitor.html",
		"CommonStatus.html",
		"DigitalIoSettings.html",
		"Engine.html",
		"FuelMonitor.html",
		"FuseMonitor.html",
		"Generator.html",
		"GroundingMonitor.html",
		"Help.html",
		"HybridMonitor.html",
		"Interface.html",
		"Mains.html",
		"Outputs.html",
		"RelaySettings.html",
		"SecurityMonitor.html",
		"Statistic.html",
		"System.html",
		"TelemetryMonitor.html",
		"MenuBuilder.js",
		"SrvFunc2.js",
		"SrvFunc4.js",
		"ControlStyles.css",
		"File.txt",
		"index.html",
		"func_test.html",
		"favicon.ico"
	};

	const char *firm_files[]=
	{
		"U1_Firmware.bin",
		"U2_Firmware.bin",
		"U1_Bootloader.bin",
		"U2_Bootloader.bin",
		"Site_Adr.bin"		
	};
	
	const uint16_t adr_firm_files[]=
	{
		ADDR_4KB_W25Q_FIRMWARE_U1,
		ADDR_4KB_W25Q_FIRMWARE_U2,
		ADDR_4KB_W25Q_BOOTLOADER_U1,
		ADDR_4KB_W25Q_BOOTLOADER_U2,
		ADDR_4KB_W25Q_ADDR_SITE_U2	
	};
	
uint16_t adr_web_page[sizeof(web_page)/4];	


/*###############################################################*/
/*###############################################################* Network_HTTP_Read_From_FlashBuf -->*/
/*###############################################################*/
int32_t Network_HTTP_Read_From_FlashBuf(uint32_t addr, uint8_t *buf, uint32_t send_len)
{
		if(!W25Q_read(addr,&buf[0],send_len))return 0;
		*(buf+send_len+1) = 0; // Insert '/0' for indicates the 'End of String' (null terminated)	
return send_len;	
}

/*###############################################################*/
/*###############################################################* Network_HTTP_Check_Response -->*/
/*###############################################################*/
int32_t Network_HTTP_Check_Response(char * uri_name, uint32_t *content_addr, uint32_t *file_len)
{
					int32_t start_4KB=Network_Check_URL_Site(uri_name);					
					if(start_4KB!=-1)
					{
						uint32_t adr_w25q=0x1000*start_4KB;
					  uint8_t size_site[5];
						W25Q_read(adr_w25q,&size_site[0],5);
						*content_addr=adr_w25q+0x100;
						*file_len=atoi((const char *)size_site);
						return 1;
					}	
return 0;	
}

/*###############################################################*/
/*###############################################################* Network_Check_URL_Site -->*/
/*###############################################################*/
int32_t Network_Check_URL_Site(char * url)
{
	for(int i=0; i<(sizeof(adr_web_page)/2); i++)
	{
		if((!strcmp(url, web_page[i]))&&(adr_web_page[i]!=0xFFFF))
		{			
			return (int32_t)adr_web_page[i]; 
		}		
	}	
return -1;	
}


/*###############################################################*/
/*###############################################################* Network_Get_Page_Addrs -->*/
/*###############################################################*/
void Network_Get_Page_Addrs(char * buff){
		uint32_t adr_w25q=0x1000*ADDR_4KB_W25Q_ADDR_SITE_U2+0x100;
		uint16_t size_file=0;
		uint8_t chr;
		uint8_t cnt;
		uint16_t quan_pages=sizeof(web_page)/4;
		uint32_t adr_page_w25q=ADDR_4KB_W25Q_SITE_U2;
	

for(int i=0; i<quan_pages; i++)
{
	adr_web_page[i]=0xFFFF;
}

for(int i=0; i<quan_pages; i++)
{
	cnt=0;
	chr=0;
	while((chr!='\n')&&(cnt<100))
	{
		W25Q_read(adr_w25q++,(uint8_t *)&chr,1);
		*(buff+(cnt++))=chr;
	}
	buff[cnt]=0;
	if(cnt==100){break;}
	char *p=0;
	p=strchr(buff,' ');
	p=strchr(++p,' ');
	p=strchr(++p,' ');
	p=strchr(++p,' ');
	if(p){size_file=atoi(++p);}
	if(size_file==0){break;}
	adr_web_page[i]=adr_page_w25q;
	adr_page_w25q+=ceil(((float)size_file+(float)0x100)/(float)0x1000);
}
}
/*###############################################################*/
/*###############################################################* Network_Init -->*/
/*###############################################################*/
void Network_Init(void)
{
	 uint8_t txsize[8] = {2,2,2,2,2,2,2,2};
   uint8_t rxsize[8] = {2,2,2,2,2,2,2,2};
	 uint8_t socknumlist[] = {SOCK_HTTP3, SOCK_HTTP4, SOCK_HTTP5, SOCK_HTTP6};
	 
														
		W5500_HW_Reset();
    reg_wizchip_cs_cbfunc(W5500_Select, W5500_Deselect);
    reg_wizchip_spi_cbfunc(W5500_Read_SPI, W5500_Write_SPI);
    wizchip_init(txsize,rxsize);


if (NETWORK.gWIZNETINFO.dhcp==NETINFO_DHCP)
{
		DHCP_init(SOCK_DHCP, NETWORK.TX_HTTP_BUF);
		reg_dhcp_cbfunc(Network_DHCP_Assign, Network_DHCP_Assign, Network_DHCP_Conflict);
		if(Network_DHCP_Routine()!=DHCP_IP_LEASED)
		{
			Network_Config((void*) &NETWORK.gWIZNETINFO);
		}
}
else if (NETWORK.gWIZNETINFO.dhcp==NETINFO_STATIC)
{
		Network_Config((void*) &NETWORK.gWIZNETINFO);
}
else
{
		Network_Config((void*) &NETWORK.gWIZNETINFO);
}

httpServer_init(NETWORK.TX_HTTP_BUF, NETWORK.RX_HTTP_BUF, MAX_HTTPSOCK, socknumlist);

ctlnetwork(CN_GET_NETINFO, (void*) &NETWORK.gWIZNETINFO);
ftpd_init(NETWORK.gWIZNETINFO.ip);	// Added by James for FTP

Network_Get_Page_Addrs((char *)NETWORK.TX_HTTP_BUF); //Читаем адреса страниц
}
/*###############################################################*/
/*###############################################################* Network_Routine -->*/
/*###############################################################*/
void Network_Routine(void)
{
for(int i = 0; i < MAX_HTTPSOCK; i++)
	{
		httpServer_run(i);	// HTTP server handler
	}	

	
		ftpd_run(NETWORK.TX_HTTP_BUF);

	
if(Network_UDP_Routine(SOCK_UDPS, NETWORK.TX_HTTP_BUF, 3000) < 0)
	{
			#ifdef _NETWORK_DEBUG_
			 printf("UDP ERROR\r\n");
			#endif
	}				

	
}	

/*###############################################################*/
/*###############################################################* Network_UDP_Routine -->*/
/*###############################################################*/
int32_t Network_UDP_Routine(uint8_t sn, uint8_t* buf, uint16_t port)
{
		int32_t  ret;
		uint16_t size, sentsize;
		uint8_t  destip[4];
		uint16_t destport;
		const char *str_Conn="Connect";
		const char *str_OK="OK";
		const char *str_ERROR="ERROR";
	
   switch(getSn_SR(sn))
   {
      case SOCK_UDP :
         if((size = getSn_RX_RSR(sn)) > 0)
         {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
            ret = recvfrom(sn, buf, size, destip, (uint16_t*)&destport);
            if(ret <= 0)
            {
#ifdef _LOOPBACK_DEBUG_
               printf("%d: recvfrom() SOCKET UDP ERROR: %d\r\n",sn,ret);
#endif
               return ret;
            }

						if (strcmp((void*)buf,"Connect")==0)
						{
							size = strlen(str_Conn)+1;
							sentsize = 0;
							NETWORK.UDP.Dest_Port=destport;
							NETWORK.UDP.Dest_Ip[0]=destip[0];
							NETWORK.UDP.Dest_Ip[1]=destip[1];
							NETWORK.UDP.Dest_Ip[2]=destip[2];
							NETWORK.UDP.Dest_Ip[3]=destip[3];							
							for(int i=0; i<sizeof(NETWORK.TX_HTTP_BUF);i++){NETWORK.TX_HTTP_BUF[0]=0;}
							strcpy((char *)NETWORK.TX_HTTP_BUF, str_Conn);
							
							while(sentsize != size)
							{
								 ret = sendto(sn, NETWORK.TX_HTTP_BUF+sentsize, size-sentsize, NETWORK.UDP.Dest_Ip, NETWORK.UDP.Dest_Port);
								 if(ret < 0) return ret;
								 sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
							}	
						
						}	
						else if ((!strcmp((void*)buf,"Site"))||(!strcmp((void*)buf,"Firm"))||(!strcmp((void*)buf,"Boot")))
						{
							ret = Write_Net_W25Q(buf,strlen("Site")+1);
							const char *p_str;
							if(ret){p_str=str_ERROR;}else{p_str=str_OK;}
							strcpy((char *)NETWORK.TX_HTTP_BUF, p_str);
							ret = sendto(sn, NETWORK.TX_HTTP_BUF, strlen(p_str), NETWORK.UDP.Dest_Ip, NETWORK.UDP.Dest_Port);
							if(ret < 0) return ret;
						}
						else if (strcmp((void*)buf,"Firmware_U2")==0)
						{
							Reset_MCU();
#ifdef _NETWORK_DEBUG_
         printf("\r\nJump To Bootloader\r\n");
#endif														
							JumpToAddrFlash(ADDR_FLASH_PAGE_BOOTLOADER);							
						}
						else if (strcmp((void*)buf,"Bootloader_U2")==0)
						{
#ifdef _NETWORK_DEBUG_
         printf("\r\nWrite Bootloader\r\n");
#endif								
							Write_Bootloader_Flash(ADDR_FLASH_PAGE_BOOTLOADER, ADDR_4KB_W25Q_BOOTLOADER_U2, (void*)NETWORK.TX_HTTP_BUF, sizeof(NETWORK.TX_HTTP_BUF));
						}
						else if (strcmp((void*)buf,"Change_Addr_Site")==0)
						{
#ifdef _NETWORK_DEBUG_
         printf("\r\nChange Addr Site\r\n");
#endif								
						Network_Get_Page_Addrs((void*)buf);
						strcpy((char *)NETWORK.TX_HTTP_BUF, str_OK);
						ret = sendto(sn, NETWORK.TX_HTTP_BUF, strlen(str_OK), NETWORK.UDP.Dest_Ip, NETWORK.UDP.Dest_Port);
						if(ret < 0) return ret;
						}							
         }
				 
				 
         break;
      case SOCK_CLOSED:
#ifdef _NETWORK_DEBUG_
         //printf("%d:UDP loopback start\r\n",sn);
#endif
         if((ret = socket(sn, Sn_MR_UDP, port, 0x00)) != sn)
            return ret;
#ifdef _NETWORK_DEBUG_
         printf("%d:Opened, UDP loopback, port [%d]\r\n", sn, port);
#endif
         break;
      default :
         break;
   }
   return 1;
}


/*###############################################################*/
/*###############################################################* Network_DHCP_Routine -->*/
/*###############################################################*/
int32_t Network_DHCP_Routine(void)
{
	int32_t ret = DHCP_FAILED;
	int32_t dhcp_retry = 0;
	
  while (ret != DHCP_IP_LEASED)
  {	
			ret = DHCP_run();

			if(ret == DHCP_IP_LEASED)
			{
#ifdef _NETWORK_DEBUG_
				printf(" - DHCP Success: DHCP Leased time : %d Sec.\r\n\r\n", getDHCPLeasetime());
#endif
				return ret;
			}
			else if(ret == DHCP_FAILED)
			{
				dhcp_retry++;
#ifdef _NETWORK_DEBUG_
				if(dhcp_retry <= 4) printf(" - DHCP Timeout occurred and retry [%d]\r\n", dhcp_retry);
#endif
			}

			if(dhcp_retry > 4)
			{
#ifdef _NETWORK_DEBUG_
				printf(" - DHCP Failed\r\n\r\n");
#endif
				//Network_Config((wiz_NetInfo*)arg);
				return ret;
			}			
	}
	
return ret;
}
/*###############################################################*/
/*###############################################################* Network_Config -->*/
/*###############################################################*/
void Network_Config(wiz_NetInfo* wiz_netinfo)
{	
	ctlnetwork(CN_SET_NETINFO, wiz_netinfo);
	ctlnetwork(CN_GET_NETINFO, wiz_netinfo);

#ifdef _NETWORK_DEBUG_
	// Display Network Information
	uint8_t tmpstr[6];
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
	printf("\r\n=== %s NET CONF ===\r\n",(char*)tmpstr);
	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",wiz_netinfo->mac[0],wiz_netinfo->mac[1],wiz_netinfo->mac[2],
		  wiz_netinfo->mac[3],wiz_netinfo->mac[4],wiz_netinfo->mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", wiz_netinfo->ip[0],wiz_netinfo->ip[1],wiz_netinfo->ip[2],wiz_netinfo->ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", wiz_netinfo->gw[0],wiz_netinfo->gw[1],wiz_netinfo->gw[2],wiz_netinfo->gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", wiz_netinfo->sn[0],wiz_netinfo->sn[1],wiz_netinfo->sn[2],wiz_netinfo->sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", wiz_netinfo->dns[0],wiz_netinfo->dns[1],wiz_netinfo->dns[2],wiz_netinfo->dns[3]);
	printf("======================\r\n");
#endif
}
/*###############################################################*/
/*###############################################################* Network_DHCP_Assign -->*/
/*###############################################################*/
void Network_DHCP_Assign(void)
{
	wiz_NetInfo gWIZNETINFO;

	getIPfromDHCP(gWIZNETINFO.ip);
	getGWfromDHCP(gWIZNETINFO.gw);
	getSNfromDHCP(gWIZNETINFO.sn);
	getDNSfromDHCP(gWIZNETINFO.dns);
	Network_Config((void*) &gWIZNETINFO);
}
/*###############################################################*/
/*###############################################################* Network_DHCP_Conflict -->*/
/*###############################################################*/
void Network_DHCP_Conflict(void)
{
	// TODO
	//printf("DHCP CONFLICT \r\n");
	while(1)
		{
			
		}
}
/*###############################################################*/
/*###############################################################* Network_FTP_Answer_MLSD_CMD -->*/
/*###############################################################*/
uint32_t Network_FTP_Answer_MLSD_CMD(int32_t (*send_ftp)(uint8_t sn, uint8_t * buf, uint16_t len), uint8_t * pbuf)
{
	uint32_t size;
	uint8_t chr=0;
	uint32_t adr_w25q=0x1000*ADDR_4KB_W25Q_ADDR_SITE_U2+0x100;

	
	size = sprintf((char *)pbuf, "drwxr-xr-x 1 ftp ftp 0 Jul 23 2017 $Recycle.Bin\r\n");
	size = strlen((char *)pbuf);
	send_ftp(DATA_SOCK, pbuf, size);
	
for(int i=0; i<(sizeof(adr_web_page)/2); i++)
{
	size=0;
	chr=0;
	while((chr!='\n')&&(size<256))
	{
		W25Q_read(adr_w25q++,(uint8_t *)&chr,1);
		*(pbuf+(size++))=chr;
	}
	pbuf[size]=0;
	if(size==256){break;}
	send_ftp(DATA_SOCK, pbuf, size);
}
return size;		
}
/*###############################################################*/
/*###############################################################* Network_Check_URL_Firm -->*/
/*###############################################################*/
int32_t Network_Check_URL_Firm(char * url)
{
	for(int i=0; i<(sizeof(adr_firm_files)/2); i++)
	{
		if((!strcmp(url, firm_files[i]))&&(adr_firm_files[i]!=0xFFFF))
		{			
			return (int32_t)adr_firm_files[i]; 
		}		
	}	
return -1;	
}
/*###############################################################*/
/*###############################################################* Network_FTP_Answer_Recycle_MLSD_CMD -->*/
/*###############################################################*/
uint32_t Network_FTP_Answer_Recycle_MLSD_CMD(int32_t (*send_ftp)(uint8_t sn, uint8_t * buf, uint16_t len), uint8_t * pbuf)
{
	uint32_t size=0;
	int32_t start_4KB,p;
	

	for(int i=0; i<(sizeof(adr_firm_files)/2); i++)
	{
			start_4KB=Network_Check_URL_Firm((char *)firm_files[i]);
		if(start_4KB!=-1)
		{
			uint32_t adr_w25q=0x1000*start_4KB;
			uint8_t size_file[6];
			size_file[5]=0;
			W25Q_read(adr_w25q,&size_file[0],5);
			uint32_t file_len=atoi((const char *)&size_file[0]);
			if(file_len<=0){for(int i=0; i<5; i++)size_file[i]='0';}
			p=0;
			while((size_file[p]=='0')&&(p<5)){p++;}
			size = sprintf((char *)pbuf, "-rwxr-xr-x 1 ftp ftp %s Jul 23 2017 %s\r\n",&size_file[p],(char *)firm_files[i]);
			size = strlen((char *)pbuf);
			send_ftp(DATA_SOCK, pbuf, size);
		}
	}	

return size;		
}
/*###############################################################*/
/*###############################################################* Network_FTP_Answer_RETR_CMD -->*/
/*###############################################################*/
uint32_t Network_FTP_Answer_RETR_CMD(int32_t (*send_ftp)(uint8_t sn, uint8_t * buf, uint16_t len), uint8_t * ftp_buf, uint8_t * ftp_filename)
{
	uint32_t file_len=0;
	int32_t start_4KB;

	start_4KB=-1;
	for(int i=0; i<(sizeof(adr_web_page)/2); i++)
	{
		if((!strcmp((char *)(ftp_filename+strlen("/")), web_page[i])))
			{
				start_4KB=Network_Check_URL_Site((char *)web_page[i]);
			}			
	}
	for(int i=0; i<(sizeof(adr_firm_files)/2); i++)
	{
		if((!strcmp((char *)(ftp_filename+strlen("/$Recycle.Bin/")), firm_files[i])))
			{
				start_4KB=Network_Check_URL_Firm((char *)firm_files[i]);
			}			
	}	
	
		if(start_4KB!=-1)
		{
			uint32_t adr_w25q=0x1000*start_4KB;
			uint8_t size_file[6];
			size_file[5]=0;
			W25Q_read(adr_w25q,&size_file[0],5);
			adr_w25q+=0x100;
			file_len=atoi((const char *)&size_file[0]);
			if(file_len<=0){return 0;}
			
			uint32_t buff_len=0;
			int i=file_len;
			while(i)
			{
				if(i>_MAX_SS){buff_len=_MAX_SS;}else{buff_len=i;}
				if(!W25Q_read(adr_w25q,&ftp_buf[0],buff_len))return 0;
				send_ftp(DATA_SOCK, ftp_buf, buff_len);
				adr_w25q+=buff_len;
				i-=buff_len;
			}
		}


return file_len;		
}

/*###############################################################*/
/*###############################################################* Network_FTP_Save_File -->*/
/*###############################################################*/
int32_t Network_FTP_Save_File(uint8_t * pbuf, char * file_name, uint32_t pbuf_size)
{
	uint32_t file_len=0;
	int32_t start_4KB;

	if(NETWORK.FTP.fl_Save_File==SAVE_NONE){NETWORK.FTP.fl_Save_File=CHECK_FILE;}
	if(NETWORK.FTP.fl_Save_File==CHECK_FILE)
	{
		
			start_4KB=-1;
	for(int i=0; i<(sizeof(adr_web_page)/2); i++)
	{
		if((!strcmp((char *)(file_name+strlen("/")), web_page[i])))
			{
				start_4KB=Network_Check_URL_Site((char *)web_page[i]);
			}			
	}
	for(int i=0; i<(sizeof(adr_firm_files)/2); i++)
	{
		if((!strcmp((char *)(file_name+strlen("/")), firm_files[i])))
			{
				start_4KB=Network_Check_URL_Firm((char *)firm_files[i]);
			}			
	}	
	for(int i=0; i<(sizeof(adr_firm_files)/2); i++)
	{
		if((!strcmp((char *)(file_name+strlen("/$Recycle.Bin/")), firm_files[i])))
			{
				start_4KB=Network_Check_URL_Firm((char *)firm_files[i]);
			}
	}
			if(start_4KB!=-1)
			{
				NETWORK.FTP.adr_Start_Save_File_W25Q=start_4KB*0x1000;
				NETWORK.FTP.adr_Save_File_W25Q=start_4KB*0x1000+0x100;
				NETWORK.FTP.Size_Save_File_W25Q=0;
				NETWORK.FTP.Num_Sector_Erase_W25Q=NETWORK.FTP.adr_Start_Save_File_W25Q/0x1000;
				NETWORK.FTP.fl_Save_File=ERASE_FIRST_SECTOR_Q25W;
			}	
			else
			{
				NETWORK.FTP.adr_Start_Save_File_W25Q=0;
				NETWORK.FTP.adr_Save_File_W25Q=0;
				NETWORK.FTP.Size_Save_File_W25Q=0;
				NETWORK.FTP.Num_Sector_Erase_W25Q=0;
				NETWORK.FTP.fl_Save_File=SAVE_NONE;
				return -1;				
			}
	}
	
	if(NETWORK.FTP.fl_Save_File==WAIT_NEXT_PACKET)
	{
		NETWORK.FTP.fl_Save_File=SAVE_NEXT_PACKET;	
	}
	
	if(NETWORK.FTP.fl_Save_File==ERASE_FIRST_SECTOR_Q25W)
	{	
			if(W25Q_EraseSector_Wait_Busy(NETWORK.FTP.adr_Start_Save_File_W25Q)){return -1;}
			NETWORK.FTP.Num_Sector_Erase_W25Q++;
			NETWORK.FTP.fl_Save_File=SAVE_FIRST_PACKET;
	}
	
	if(NETWORK.FTP.fl_Save_File==SAVE_FIRST_PACKET)
	{	
			if(W25Q_WriteBytes_Wait_Busy(NETWORK.FTP.adr_Save_File_W25Q,pbuf,pbuf_size)){return -1;}
			NETWORK.FTP.adr_Save_File_W25Q+=pbuf_size;
			NETWORK.FTP.Size_Save_File_W25Q+=pbuf_size;
			NETWORK.FTP.fl_Save_File=WAIT_NEXT_PACKET;		
	}	
	
	if(NETWORK.FTP.fl_Save_File==SAVE_NEXT_PACKET)
	{	
		uint32_t temp;
		temp=(NETWORK.FTP.adr_Save_File_W25Q+pbuf_size)/0x1000;
		
		if(temp==NETWORK.FTP.Num_Sector_Erase_W25Q)
		{
			if(W25Q_EraseSector_Wait_Busy(0x1000*NETWORK.FTP.Num_Sector_Erase_W25Q)){return -1;}
			NETWORK.FTP.Num_Sector_Erase_W25Q++;
		}
		else if (temp>NETWORK.FTP.Num_Sector_Erase_W25Q)
		{
			return -1;	
		}			
	}	
	
	if(NETWORK.FTP.fl_Save_File==SAVE_NEXT_PACKET)
	{	
			if(W25Q_WriteBytes_Wait_Busy(NETWORK.FTP.adr_Save_File_W25Q,pbuf,pbuf_size)){return -1;}
			W25Q_read(NETWORK.FTP.adr_Start_Save_File_W25Q-0x100,(uint8_t * )&NETWORK.TX_HTTP_BUF[0],pbuf_size);
			NETWORK.FTP.adr_Save_File_W25Q+=pbuf_size;
			NETWORK.FTP.Size_Save_File_W25Q+=pbuf_size;
			NETWORK.FTP.fl_Save_File=WAIT_NEXT_PACKET;
	}	
return file_len;		
}
/*###############################################################*/
/*###############################################################* Network_FTP_Save_File_Size -->*/
/*###############################################################*/
int16_t Network_FTP_Save_File_Size(uint8_t * pbuf, char * file_name)
{
	char size_file[10];
	if(NETWORK.FTP.fl_Save_File==WAIT_NEXT_PACKET)
	{
		sprintf((char *)&size_file[0], "%ld", NETWORK.FTP.Size_Save_File_W25Q);
		if(W25Q_WriteBytes_Wait_Busy(NETWORK.FTP.adr_Start_Save_File_W25Q,(uint8_t * )&size_file[0],5)){return -1;}
		NETWORK.FTP.adr_Start_Save_File_W25Q=0;
		NETWORK.FTP.adr_Save_File_W25Q=0;
		NETWORK.FTP.Size_Save_File_W25Q=0;
		NETWORK.FTP.Num_Sector_Erase_W25Q=0;
		NETWORK.FTP.fl_Save_File=SAVE_NONE;
	}
	
	if(NETWORK.FTP.fl_Save_File==SAVE_NONE)
	{
		if((!strcmp((char *)(file_name+strlen("/")), "Site_Adr.bin")))
			{
				#ifdef _NETWORK_DEBUG_
         printf("\r\nChange Addr Site\r\n");
				#endif								
				Network_Get_Page_Addrs((char *)pbuf);
			}	
		if((!strcmp((char *)(file_name+strlen("/$Recycle.Bin/")), "Site_Adr.bin")))
			{
				#ifdef _NETWORK_DEBUG_
         printf("\r\nChange Addr Site\r\n");
				#endif
				Network_Get_Page_Addrs((char *)pbuf);
			}
		if((!strcmp((char *)(file_name+strlen("/")), "U2_Firmware.bin")))
			{
				NETWORK.f_Download_Firmware=1;
			}		
		if((!strcmp((char *)(file_name+strlen("/")), "U2_Bootloader.bin")))
			{
				#ifdef _NETWORK_DEBUG_
				printf("\r\nWrite Bootloader\r\n");
				#endif								
				Write_Bootloader_Flash(ADDR_FLASH_PAGE_BOOTLOADER, ADDR_4KB_W25Q_BOOTLOADER_U2, (void*)NETWORK.TX_HTTP_BUF, sizeof(NETWORK.TX_HTTP_BUF));
			}		
	}	
	
return 0;
}


/*###############################################################*/
/*###############################################################* Network_Check_URL_Site -->*/
/*###############################################################*/
int32_t Network_Check_Size_File(char * url)
{
	int32_t filesize=0;
	int32_t start_4KB=-1;
	for(int i=0; i<(sizeof(adr_web_page)/2); i++)
	{
		if((!strcmp((char *)url, web_page[i])))
			{
				start_4KB=Network_Check_URL_Site((char *)web_page[i]);
			}			
	}
	for(int i=0; i<(sizeof(adr_firm_files)/2); i++)
	{
		if((!strcmp((char *)url, firm_files[i])))
			{
				start_4KB=Network_Check_URL_Firm((char *)firm_files[i]);
			}			
	}	
			if(start_4KB!=-1)
			{
				uint32_t adr_w25q=0x1000*start_4KB;
				uint8_t size_site[5];
				if(!W25Q_read(adr_w25q,&size_site[0],5)){return -1;}
				filesize=atoi((const char *)size_site);
			}	
			else
			{
				return -1;				
			}
	
return filesize;	
}	




