#ifndef NETWORK_H_
#define NETWORK_H_
#include "wizchip_conf.h"


















//#define _NETWORK_DEBUG_


int32_t Network_DHCP_Routine(void);
void Network_Init(void);
void Network_Config(wiz_NetInfo* wiz_netinfo);
void Network_DHCP_Assign(void);
void Network_DHCP_Conflict(void);
void Network_Routine(void);
int32_t Network_UDP_Routine(uint8_t sn, uint8_t* buf, uint16_t port);
int32_t Network_Check_URL_Site(char * url);
int32_t Network_HTTP_Check_Response(char * uri_name, uint32_t *content_addr, uint32_t *file_len);
int32_t Network_HTTP_Read_From_FlashBuf(uint32_t addr, uint8_t *buf, uint32_t send_len);
void Network_Get_Page_Addrs(char * buff);
uint32_t Network_FTP_Answer_MLSD_CMD(int32_t (*send)(uint8_t sn, uint8_t * buf, uint16_t len), uint8_t * buf);
uint32_t Network_FTP_Answer_Recycle_MLSD_CMD(int32_t (*send_ftp)(uint8_t sn, uint8_t * buf, uint16_t len), uint8_t * pbuf);
int32_t Network_Check_URL_Firm(char * url);
uint32_t Network_FTP_Answer_RETR_CMD(int32_t (*send_ftp)(uint8_t sn, uint8_t * buf, uint16_t len), uint8_t * ftp_buf, uint8_t * ftp_filename);
int32_t Network_FTP_Save_File(uint8_t * pbuf, char * file_name, uint32_t pbuf_size);
int16_t Network_FTP_Save_File_Size(uint8_t * pbuf, char * file_name);
int32_t Network_Check_Size_File(char * url);

#endif /* NETWORK_H_ */
