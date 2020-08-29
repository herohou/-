/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2009   
   �Ͷ˵Ĵ���I/Oģ��Ǽ�Petit FatFs��C����2009   */
/*-----------------------------------------------------------------------*/

#include  "diskio.h"
#include  "mcusys.h"
#include  "spi.h"




u8 dest_tbuf[512];
//u16
/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive      
   ��ʼ������������    
   ����0 Ϊ�ɹ�  ����1 Ϊʧ��                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	DSTATUS stat;

	// Put your code here	   ����Ĵ���д������
	 stat=SD_Init();	  //��ʼ��SD�� ��������ֵ��stat


	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector
   ������                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* dest,			/* Pointer to the destination object ������ݵĵ�ַ*/
	DWORD sector,		/* Sector number (LBA)  Ҫ����������*/
	WORD sofs,			/* Offset in the sector ����ƫ��ֵ*/
	WORD count			/* Byte count (bit15:destination) ���������ݸ���*/
)
{
	DRESULT res;

    // Put your code here	 ����Ĵ���д������

	u16 i;
	u8 r1;
    res=RES_ERROR;

	
	r1=SD_ReadDisk(dest_tbuf,sector,1);//ֱ�Ӷ�ȡһ��������������

	if(r1==0)//ָ��ͳɹ�
	 {
//			 if(sofs)  //�����ƫ����
//			 {		  
//			 for(i=0;i<sofs;i++)Moni_SPI_ReadWriteByte(0xff);  //��ƫ��ֵѭ����
			 			      
			 for(i=0;i<count;i++) *(dest++)=dest_tbuf[sofs++];  //����Ҫ������������		   
//			 }
//			 else   //û��ƫ��ֵ ��ƫ��ֵΪ0
//			 {
//			   //Ҫ�������ݸ������Ƹ�tmp
//			   for(i=0;i<count;i++) *(dest++)=Moni_SPI_ReadWriteByte(0xff);  //����Ҫ������������
//			 }
//			 tmp=512-sofs-count;  //ʣ��Ϊ������   ������-ƫ��ֵ-��ȡ����
//			 for(i=0;i<tmp;i++)Moni_SPI_ReadWriteByte(0xff);// ��ʣ������ѭ���� ��������������и���
//	

	 }
	else  return res;	 //��ָ��ʧ��

	res=RES_OK;	   //���ݶ�ȡ�ɹ�
	return res;


//	u16 tmp,i;
//	u8 r1;
//    res=RES_ERROR;
//
//	r1=SD_SendCmd(CMD17,sector<<9,0X01);//������
//	
//	if(r1==0)//ָ��ͳɹ�
//		{
//		    while(Moni_SPI_ReadWriteByte(0xff)!=0xfe);  //�ȴ���������
//			 if(sofs)  //�����ƫ����
//			 {		  
//			 for(i=0;i<sofs;i++)Moni_SPI_ReadWriteByte(0xff);  //��ƫ��ֵѭ����
//			 			      
//			 for(i=0;i<count;i++) *(dest++)=Moni_SPI_ReadWriteByte(0xff);  //����Ҫ������������		   
//			 }
//			 else   //û��ƫ��ֵ ��ƫ��ֵΪ0
//			 {
//			   //Ҫ�������ݸ������Ƹ�tmp
//			   for(i=0;i<count;i++) *(dest++)=Moni_SPI_ReadWriteByte(0xff);  //����Ҫ������������
//			 }
//			 tmp=512-sofs-count;  //ʣ��Ϊ������   ������-ƫ��ֵ-��ȡ����
//			 for(i=0;i<tmp;i++)Moni_SPI_ReadWriteByte(0xff);// ��ʣ������ѭ���� ��������������и���
//
//
//			//������2��αCRC��dummy CRC��
//			  Moni_SPI_ReadWriteByte(0xff); 
//              Moni_SPI_ReadWriteByte(0xff); 		   
//		}
//	  else  return res;	 //��ָ��ʧ��
//
//	SD_DisSelect();//ȡ��Ƭѡ
//	res=RES_OK;	   //���ݶ�ȡ�ɹ�
//	return res;

//	u8 res1=0; 
//    if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
////	switch(pdrv)
////	{
////		case SD_CARD://SD��
//			res1=SD_ReadDisk(dest,sector,count);	 
//			while(res1)//������
//			{
//				SD_Init();	//���³�ʼ��SD��
//				res1=SD_ReadDisk(dest,sector,count);	
//				//printf("sd rd error:%d\r\n",res);
//			}
////			break;
////		case EX_FLASH://�ⲿflash
////			for(;count>0;count--)
////			{
////				W25QXX_Read(buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
////				sector++;
////				buff+=FLASH_SECTOR_SIZE;
////			}
////			res=0;
////			break;
////		default:
////			res=1; 
////	}
//   //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
//    if(res1==0x00)return RES_OK;	 
//    else return RES_ERROR;	 


}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector 
   д����                                                 
   
   disk_writep��д��������������ֻ������������
   д������ݵ�ַ* buff��ڼ�������sc�������ݡ�

   ���ڱ�д��������ʱ��Ҫע�⣬����FatFs�ڲ����ú�������Ҫ�����밴�����µ�˳������
   �� buff ָ��һ����ָ�룬��sc ��Ϊ0 ʱ�����ʾ�����������д�������г�ʼ����
   ��sc Ϊ0 ʱ�����ʾ�����������д�������н���������
   �� buff ָ��һ���ڴ滺���������ǽ��������Ķ�д��*/
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
const BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation ָ������ݱ�д�룬�գ�����/���д����*/
	DWORD sc		/* Sector number (LBA) or Number of bytes to send ��������LBA����Ҫ���͵��ֽ���*/
)
{
//	DRESULT res;

//	 u8 r1;
//	 u16 t;
//	 u32 m;
//	res = RES_ERROR;
//
//	if (!buff) {						  //���ָ���ָ��
//		if (sc) {						  //sc��Ϊ0 ����д������ʼ��
//
//			// Initiate write process		  ��ʼд����
//				  r1=SD_SendCmd(CMD24,sc<<9,0X01);
//				  if(r1!=0) return res;	 //Ӧ����ȷ
//			      do
//		          {
//			        if(Moni_SPI_ReadWriteByte(0XFF)==0XFF)break;//OK	  �ſ����ݵȴ�SD��׼����
//			          m++;		  	
//		           }while(m<0XFFFFFF);//�ȴ� 
//	
//		          Moni_SPI_ReadWriteByte(0XFE);		  //��������
//				  res=RES_OK;
//
//		} else {							 //scΪ0  ��ʾд��������
//
//			// Finalize write process		  ���д����
//
//		           	//������2��αCRC��dummy CRC��
//				 	Moni_SPI_ReadWriteByte(0xFF);
//					Moni_SPI_ReadWriteByte(0xFF);
//					r1 = Moni_SPI_ReadWriteByte(0xFF);
//		
//					if( (r1&0x1f) != 0x05)//�ȴ�SD��Ӧ��
//					{
//						SD_DisSelect();
//						return res;
//					}
//					//�ȴ�������
//					while(!Moni_SPI_ReadWriteByte(0xFF));
//		
//					SD_DisSelect();	
//					res = RES_OK;
//
//
//
//
//		}
//	} else {
//
//		// Send data to the disk			  �������ݵ�����
//			for(t=0;t<512;t++)Moni_SPI_ReadWriteByte(*buff++);//����ٶ�,���ٺ�������ʱ��
//		    res = RES_OK;
//
//
//	}
//
//	return res;


	u8 res1=0;  
    if (!buff)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
//	switch(pdrv)
//	{
//		case SD_CARD://SD��
			res1=SD_WriteDisk((u8*)buff,sc,1);
			while(res1)//д����
			{
				SD_Init();	//���³�ʼ��SD��
				res1=SD_WriteDisk((u8*)buff,sc,1);	
				//printf("sd wr error:%d\r\n",res);
			}
//			break;
//		case EX_FLASH://�ⲿflash
//			for(;count>0;count--)
//			{										    
//				W25QXX_Write((u8*)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
//				sector++;
//				buff+=FLASH_SECTOR_SIZE;
//			}
//			res=0;
//			break;
//		default:
//			res=1; 
//	}
    //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res1 == 0x00)return RES_OK;	 
    else return RES_ERROR;	








}

