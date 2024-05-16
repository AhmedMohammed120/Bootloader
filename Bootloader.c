
/****************************************************************
 FILE DESCRIPTION
-----------------------------------------------------------------
*        File     :  Bootloader.c
*        Brief    :  Bootloader
*        Details  :  - 
*

*****************************************************************
* INCLUDES
****************************************************************/
#include "Bootloader.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "crc.h"
/****************************************************************
*    LOCAL MACROS CONSTATN\FUNCTIONS
****************************************************************/

/****************************************************************
*    LOCAL DATA
****************************************************************/
static uint8_t BL_Host_Buffer[BL_RECEIVING_DATA_BUFFER];
static char BL_Version_Info[BL_VERSION_DATA_SIZE] = {BL_VERSION_VENDOR_ID,
                                                   BL_VERSION_MAJOR_VERSION,
                                                   BL_VERSION_MINOR_VERSION,
                                                   BL_VERSION_PATCH_VERSION   
                                                  };
	                                                
	                                            
static char Commands[12] = 
{
CBL_GET_VER_CMD           ,     
CBL_GET_HELP_CMD          ,
CBL_GET_CID_CMD           ,
CBL_GET_RDP_STATUS_CMD    ,
CBL_GO_TO_ADDR_CMD        ,
CBL_FLASH_ERASE_CMD       ,
CBL_MEM_WRITE_CMD         ,
CBL_EN_R_W_PROTECT_CMD    ,
CBL_MEM_READ_CMD          ,
CBL_READ_SECTOR_STATUS_CMD,
CBL_OTP_READ_CMD          ,
CBL_CHANGE_ROP_Level_CMD  
};
	
	                                            

	
	
	
	
	
	
	
/****************************************************************
*    GLOBAL DATA
****************************************************************/

/****************************************************************
*    LOCAL FUNCTION PROTOTYPES
****************************************************************/
static HAL_StatusTypeDef Bootloader_Get_Version(uint8_t* Host_Buffer);
static void Bootloader_Get_Help(uint8_t* Host_Buffer);
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
static uint8_t Bootloader_Jump_To_Specific_Address(uint8_t* Host_Buffer);
/*static*/ HAL_StatusTypeDef Bootloader_FLASH_Erase(uint8_t* Host_Buffer);
static BL_Status BootLoader_Erase_Sectors(uint8_t Sector_Number, uint8_t Number_of_Sectors);
/*static*/ HAL_StatusTypeDef Bootloader_Write_Data_in_MEMORY(uint8_t* Host_Buffer);
uint8_t Bootloader_Get_Read_Protection_Level(uint8_t* Host_Buffer);
 HAL_StatusTypeDef Bootloader_Change_Read_Protection_Level(uint8_t* Host_Buffer);

static CRC_Status Bootloader_CRC_Verify(uint32_t Host_CRC,uint8_t* Host_Buffer,uint8_t Data_Length);
static void BL_Send_ACK(uint8_t Data_Length);
static void BL_Send_NACK(void);
static void BL_Send_Data(uint8_t* Data , uint8_t Data_Length);
static void Jump_to_Main_Function(void);
static uint8_t BL_Check_Address_Validation(uint32_t Address);
static HAL_StatusTypeDef BL_Write_Payload(uint8_t *Host_Buffer , uint32_t Payload_Address , uint8_t Payload_Len);
static uint8_t  Get_Read_Protection_Level(void);
static HAL_StatusTypeDef  Change_Read_Protection_Level(uint8_t Protection_Level);






/****************************************************************
*    LOCAL FUNCTIONS
****************************************************************/

/****************************************************************
*   GLOBAL FUNCTIONS
****************************************************************/
/*
    Brief    : Bootloader send message. 
	  Details  : send message by USART.
	  param[in]: Pointer to char.
	  param[in]: Pin Led pin.  
*/



void BL_SendMessage(char* Message , ...)
{	
	 char String[100] = {0};
   va_list Args;
	 
/*Enable Accesses to the Variable arguments*/
	 va_start(Args,Message);
	 
/*Writing formated data from variable arguments to a string*/
	 vsprintf(String,Message,Args);

#if (SELECTED_PROTOCOL == BL_USART_ENABLE_DEBUG_MESSAGE)
/*Transmitted the data through the defined USART */
	 HAL_UART_Transmit(BOOTLOADER_USART1,(uint8_t*)String,sizeof(String),HAL_MAX_DELAY );
	 
#elif (SELECTED_PROTOCOL == BL_SPI_ENABLE_DEBUG_MESSAGE)
/*Transmitted the data through the defined SPI */	 
	 
#elif (SELECTED_PROTOCOL == BL_CAN_ENABLE_DEBUG_MESSAGE) 
	 
/*Transmitted the data through the defined CAN */		 
	 
#endif	 
/*Perform a clean up for an App object Initialized by a call to va_start*/
   va_end(Args);
}


/*
      Brief  : Bootloader send message. 
	  Details  : send message by USART.
	  param[in]: char.
	    
*/

HAL_StatusTypeDef BL_Get_Host_Command(void)
{

	ACK_Status   Status = BL_ACK;
	uint8_t Data_Length = 0;
	HAL_StatusTypeDef Hal_Return = HAL_ERROR;
	HAL_StatusTypeDef Hal_Status = HAL_ERROR;
	/*1. Clear Data buffer */
  memset(BL_Host_Buffer,0,BL_RECEIVING_DATA_BUFFER);
	/*Receive First byte */
	Hal_Status = HAL_UART_Receive(BOOTLOADER_USART1, BL_Host_Buffer, 1, HAL_MAX_DELAY);
	if(Hal_Status != HAL_OK)
	{
		Status = BL_NACK;
		Hal_Return = HAL_ERROR;
	}
	else
	{
	/*Receive All data*/
	Data_Length = BL_Host_Buffer[0];
	Hal_Status = HAL_UART_Receive(BOOTLOADER_USART1, &BL_Host_Buffer[1], Data_Length, HAL_MAX_DELAY);
	}
	if(Hal_Status != HAL_OK)
	{
		Status = BL_NACK;
	}
	else
	{
		Hal_Return = HAL_OK;
	/*Check Command*/
	  uint8_t	Command = (BL_Host_Buffer[1]);
		switch(Command)
		{
			case CBL_GET_VER_CMD:
			BL_SendMessage("Read the Bootloader version from MCU \r \n");
			Bootloader_Get_Version( BL_Host_Buffer);
			break;
			
			case CBL_GET_HELP_CMD:
			BL_SendMessage("Read the Commands Supported by Bootloader \r \n");
			Bootloader_Get_Help(BL_Host_Buffer);
			break;
			
			case CBL_GET_CID_CMD:
			BL_SendMessage("Read the Bootloader Identification number \r \n");
			Bootloader_Get_Chip_Identification_Number(BL_Host_Buffer);
			break;
			
			case CBL_GET_RDP_STATUS_CMD:
			BL_SendMessage("Read the Flash protection level \r \n");
						
			break;
			
			case CBL_GO_TO_ADDR_CMD:
			BL_SendMessage("Bootloader jumb to a specific address \r \n");
			Bootloader_Jump_To_Specific_Address(BL_Host_Buffer);
				
			break;
			
			case CBL_FLASH_ERASE_CMD:
			BL_SendMessage("Mass erase or sector erase of the user flash \r \n");
			Bootloader_FLASH_Erase(BL_Host_Buffer);
			break;
			
			case CBL_MEM_WRITE_CMD:
			BL_SendMessage("Writing data into different memories of MCU \r \n");
			Bootloader_Write_Data_in_MEMORY(BL_Host_Buffer);
			
			break;
			
			case CBL_EN_R_W_PROTECT_CMD:
			BL_SendMessage("Enable read/write protection on different sectors of flash \r \n");

			break;
			
			case CBL_MEM_READ_CMD:
			BL_SendMessage("Read data from Different memories of the MCU \r \n");

			break;
			
			case CBL_READ_SECTOR_STATUS_CMD:
			BL_SendMessage("Read all sectors protection status \r \n");
			break;
			
			case CBL_OTP_READ_CMD:
			BL_SendMessage("Read OTP contents \r \n");
			break;
			
			
			case CBL_CHANGE_ROP_Level_CMD:
			BL_SendMessage("Change ROP level \r \n");
			break;
					
			default:
			BL_SendMessage(" \n Invalid Command %d\n",Command);
			Hal_Return = HAL_OK;
			break;	
		}
	
	}
	
return Hal_Return;
}



/*
    Brief    : Bootloader Get Version Message. 
	  Details  : send message by USART.
	  param[in]: pointer to uint8_t.   
*/
static HAL_StatusTypeDef Bootloader_Get_Version(uint8_t* Host_Buffer)
{
	HAL_StatusTypeDef Hal_Status = HAL_ERROR;
	uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
  ACK_Status Status = BL_NACK;	

  /*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
  /*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}
	else
	{
	BL_SendMessage("CRC Verification bassed\n");
	/*3. Send ACK*/
		Hal_Status = HAL_OK;
   BL_Send_ACK(BL_VERSION_DATA_SIZE);
	/*4. Send Version Info*/ 
   Status = BL_ACK;	
	/*4. Send Version Info*/
			BL_SendMessage("Bootloader Vendor       : %d     \n",BL_Version_Info[BL_VERSION_VENDOR_ID_POS]);
			BL_SendMessage("Bootloader Major Version: %d     \n",BL_Version_Info[BL_VERSION_MAJOR_VERSION_POS]);
			BL_SendMessage("Bootloader Minor Version: %d     \n",BL_Version_Info[BL_VERSION_MINOR_VERSION_POS]);
			BL_SendMessage("Bootloader Patch Version: %d     \n",BL_Version_Info[BL_VERSION_PATCH_VERSION_POS]);
			BL_SendMessage("System Info done :)");
	}
	return Hal_Status;
}

static CRC_Status Bootloader_CRC_Verify(uint32_t Host_CRC,uint8_t* Host_Buffer,uint8_t Data_Length)
{
	uint32_t Accumulated_CRC = 0;
	CRC_Status Status = CRC_NOT_PASSED;
	uint32_t Data = 0;
	for(unsigned int Counter = 0 ; Counter < Data_Length ; Counter++)
	{
		Data  = (uint32_t)Host_Buffer[Counter];
	  Accumulated_CRC = HAL_CRC_Accumulate(BL_CRC,&Data ,1);
	}

	if(Accumulated_CRC == Host_CRC)
	{
		Status = CRC_PASSED;
		
	}
	else
	{
	Status = CRC_NOT_PASSED;
	}
	return Status;
}



static void BL_Send_ACK(uint8_t Data_Length)
{
   uint8_t Arr[2] = {0};    
   Arr[0] = ACK;
   Arr[1] = Data_Length;	
   BL_Send_Data(Arr,0x2);   
}
static void BL_Send_NACK(void)
{
   uint8_t NACK_Val  = NACK;  
   BL_Send_Data(&NACK_Val,0x1);
}


static void BL_Send_Data(uint8_t* Data , uint8_t Data_Length)
{
HAL_UART_Transmit(BOOTLOADER_USART1,Data,Data_Length,HAL_MAX_DELAY);
}

static void Bootloader_Get_Help(uint8_t* Host_Buffer)
{
  uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
  ACK_Status Status = BL_NACK;	
/*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
/*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}
	else
	{
	/*3. Send ACK*/
	BL_Send_ACK(BL_COMMANDS_SIZE);
	/*Sending Data*/
  BL_Send_Data((uint8_t*)Commands,12);	
		Status = BL_ACK;
	}
}


static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer)
{
  uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
	uint16_t Identification_Number = 0;
  ACK_Status Status = BL_NACK;	
/*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
/*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*3. Send ACK*/
	Status = BL_ACK;
  BL_Send_ACK(2);
  Identification_Number = (uint16_t)((DBGMCU->IDCODE)&0xFFF);
	BL_Send_Data((uint8_t*)&Identification_Number,2);	
	}
	else
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}
}




static void Jump_to_Main_Function(void)
{
	/*1. Getting MSP Address*/
   uint32_t MSP_Address = *((volatile uint32_t*)APPLICATION_BASE_ADDRESS);
	/*2. Getting Reset Handler Address*/
   uint32_t Reset_Handler_ = *((volatile uint32_t*)(APPLICATION_BASE_ADDRESS + 4));
	/*3. DeInit Peripherals*/
	
	/*4. Init MSP*/
	__set_MSP(MSP_Address);
	/*4. Jump to main*/
   Main_Function main_;
	 main_ = (Main_Function)Reset_Handler_;
	 main_();
}

static uint8_t Bootloader_Jump_To_Specific_Address(uint8_t* Host_Buffer)
{
Jump_to_Address Jump;
uint8_t Address_Status = INVALID_ADDRESS;
  uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
	uint32_t Address = 0;
  ACK_Status Status = BL_NACK;	
	
/*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
/*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*3. Send ACK*/
	Status = BL_ACK;
  BL_Send_ACK(2);
	/*4. Check Address Validation*/		
	Address = *((uint32_t*)(&Host_Buffer[2]));	
	BL_Check_Address_Validation(Address);
		if(INVALID_ADDRESS!=BL_Check_Address_Validation(Address))
		{
  /*Jump to Address*/
		
		Jump = (Jump_to_Address)Address;
		Jump();
		}	
		else
		{
			/*Invalid Address*/
		}

	}
	else
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}


return Address_Status;
}
static uint8_t BL_Check_Address_Validation(uint32_t Address)
{
  uint8_t Status = INVALID_ADDRESS;
 if((FLASH_BASE<=Address) && (Address<=FLASH_MEMORY_END))
 {
	 Status = VALID_ADDRESS;
 }
 else if((SRAM1_BASE<=Address) && (Address<=SRAM1_MEMORY_END))
 {
	 Status = VALID_ADDRESS;
 }
 else if((SRAM2_BASE<=Address) && (Address<=SRAM2_MEMORY_END))
 {
	 Status = VALID_ADDRESS;
 }
  else if((CCMDATARAM_BASE<=Address) && (Address<=CCRAM_MEMORY_END))
 {
	 Status = VALID_ADDRESS;
 }
 return Status;
}

/*static*/ HAL_StatusTypeDef Bootloader_FLASH_Erase(uint8_t* Host_Buffer)
{
 HAL_StatusTypeDef HAL_Status  = HAL_ERROR;
	uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
  ACK_Status Status = BL_NACK;	
	

  /*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
  /*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
   /*3.Send ACK*/
	Status = BL_ACK;
  BL_Send_ACK(0);
   /*Start Erasing Process*/
	HAL_Status = BootLoader_Erase_Sectors(Host_Buffer[2],Host_Buffer[3]);
	}
	else
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}

return HAL_Status;
}

static BL_Status BootLoader_Erase_Sectors(uint8_t Sector_Number, uint8_t Number_of_Sectors)
{

FLASH_EraseInitTypeDef Flash_Erase;	
BL_Status Status = BL_Not_Passed;
uint32_t Sector_Error = 0;
uint8_t Reminding_Sectors = 0;
/*1. Init Flash Structure*/
Flash_Erase.Banks = FLASH_BANK_1;
Flash_Erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	
/*2.Check Erase Type*/	
	if(FLASH_MASS_ERASE == Number_of_Sectors)
	{
	Flash_Erase.TypeErase = FLASH_TYPEERASE_MASSERASE;
	Status = BL_Passed;

	}
/*3.Check Erasing Sectors Validity*/
else if(FLASH_SECTORS_NUM <= Number_of_Sectors)
{
	/*Error*/
	BL_Status Status = BL_Not_Passed;
}
else
{

	Reminding_Sectors =  (FLASH_SECTORS_NUM - Sector_Number);
  if(Reminding_Sectors < Number_of_Sectors)
	{
	Number_of_Sectors = Reminding_Sectors;
	}
	else
	{
		/*Nothing*/
	}
	Flash_Erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	Flash_Erase.NbSectors =  Number_of_Sectors;
	Flash_Erase.Sector    = Sector_Number;
	Status = BL_Passed;
}
if(BL_Passed == Status)
{
/*4. Unlock Flash Control Register*/
  HAL_FLASH_OB_Unlock();
/*5. Configure User configuration bits*/
	HAL_FLASHEx_Erase(&Flash_Erase,&Sector_Error);
/*6. Lock Flash Control Register*/
	HAL_FLASH_OB_Lock();
}

return Status;
}

/*static*/ HAL_StatusTypeDef Bootloader_Write_Data_in_MEMORY(uint8_t* Host_Buffer)
{
HAL_StatusTypeDef HAL_Status = HAL_ERROR;
  uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
	uint32_t Base_Address = 0;
	uint8_t Payload_Len = 0;
  ACK_Status Status = BL_NACK;	
/*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
/*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*3. Send ACK*/
	Status = BL_ACK;
  BL_Send_ACK(2);
	/*getting Base memory address & End Address*/
	Base_Address = *((uint32_t*)(&Host_Buffer[2]));
	Payload_Len = Host_Buffer[6];
	/*Checking Validation of addresses range*/
		uint8_t X = BL_Check_Address_Validation(Base_Address);
		uint8_t Y = (BL_Check_Address_Validation(Base_Address + Payload_Len) );
		if((X == VALID_ADDRESS)&& Y == VALID_ADDRESS)
		{
	  	HAL_Status  = BL_Write_Payload( Host_Buffer ,  Base_Address ,  Payload_Len);
		   			
		}	
		else
		{
		HAL_Status = HAL_ERROR;
		}
	}
	else
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}

return HAL_Status;
}


static HAL_StatusTypeDef BL_Write_Payload(uint8_t *Host_Buffer , uint32_t Payload_Address , uint8_t Payload_Len)
{
	     HAL_StatusTypeDef HAL_Status = HAL_ERROR;
       HAL_FLASH_Unlock();
			for(unsigned int Counter = 0; Counter < Payload_Len ; Counter++ )
			{
      HAL_Status  = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,  (Payload_Address + Counter) , Host_Buffer[ 7 + Counter]);	
    	if(HAL_Status != HAL_OK)
			{
				HAL_Status = HAL_ERROR;
			  break;
			}				
			else
			{
				HAL_Status = HAL_ERROR;
				
			}
			}	
			HAL_FLASH_Lock();
return HAL_Status;
}



uint8_t Bootloader_Get_Read_Protection_Level(uint8_t* Host_Buffer)
{
  HAL_StatusTypeDef HAL_Status = HAL_ERROR;
  uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
  ACK_Status Status = BL_NACK;	
/*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
/*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*3. Send ACK*/
	Status = BL_ACK;
	Get_Read_Protection_Level();
  BL_Send_ACK(1);
	
	}
	else
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}


return HAL_Status;
}


static uint8_t  Get_Read_Protection_Level(void)
{
	uint8_t RDP = 0;
	FLASH_OBProgramInitTypeDef RDP_Config;
  HAL_FLASHEx_OBGetConfig(&RDP_Config);
	RDP = RDP_Config.RDPLevel;
	if(FLASH_READ_PROTECTION_LEVEL0 == RDP )
	{
	RDP = 0x0;
	}
	else if(FLASH_READ_PROTECTION_LEVEL2 == RDP )
	{
	RDP = 0x2;
	}
	
	else
	{
	RDP = 0x1;
	}
	return RDP;
}


 HAL_StatusTypeDef Bootloader_Change_Read_Protection_Level(uint8_t* Host_Buffer)
{
  HAL_StatusTypeDef HAL_Status = HAL_ERROR;
  uint8_t  Data_Size = 0;
	uint32_t Received_CRC = 0;
  ACK_Status Status = BL_NACK;	
/*1. Get Data Size & Received CRC*/	
	Data_Size = Host_Buffer[0] + 1;
	Received_CRC = *((uint32_t*)((Host_Buffer + Data_Size) - BL_CRC_SIZE_BYTE));
/*2. Check CRC Verification*/		
	if(Bootloader_CRC_Verify(Received_CRC,Host_Buffer,Data_Size - BL_CRC_SIZE_BYTE) != CRC_NOT_PASSED)
	{
	/*3. Send ACK*/ 
	Status = BL_ACK;
  BL_Send_ACK(1);
/*Change Read Protection level*/		
	HAL_Status = Change_Read_Protection_Level(Host_Buffer[2]);
	}
	else
	{
	/*Send NACK*/
	Status = BL_NACK;	
  BL_Send_NACK();
	}
return HAL_Status;
}

static HAL_StatusTypeDef  Change_Read_Protection_Level(uint8_t Protection_Level)
{
uint8_t Level = 0;
HAL_StatusTypeDef HAL_Status = HAL_ERROR;
FLASH_OBProgramInitTypeDef OB_Program;	
if((FLASH_READ_PROTECTION_LEVEL0 == Protection_Level) || (FLASH_READ_PROTECTION_LEVEL0 == Protection_Level))
{		HAL_FLASH_OB_Lock();
	/*1. Unlock FLASH_OPTCR*/
   HAL_Status	= HAL_FLASH_OB_Unlock();
	if(HAL_ERROR != HAL_Status)
	{
	/*2. Program option bytes*/
    OB_Program.OptionType = OPTIONBYTE_RDP;
		OB_Program.RDPLevel = Protection_Level;
		HAL_Status = HAL_FLASHEx_OBProgram(&OB_Program);
		if(HAL_ERROR != HAL_Status)
		{
	 /*3. Launch program*/
	 HAL_Status = HAL_FLASH_OB_Launch();
		if(HAL_ERROR != HAL_Status)
		{
	/*4. lock FLASH_OPTCR*/
	 HAL_Status = HAL_FLASH_OB_Lock();	
		}
  else
    {
	   HAL_Status = HAL_FLASH_OB_Lock();
	   HAL_Status = HAL_ERROR;
    }
		
		}
	  else
    {
	   HAL_Status = HAL_FLASH_OB_Lock();
	   HAL_Status = HAL_ERROR;
    }

	}
	else
{
	HAL_Status = HAL_FLASH_OB_Lock();
	HAL_Status = HAL_ERROR;
}
}

else
{
	HAL_Status = HAL_FLASH_OB_Lock();
	HAL_Status = HAL_ERROR;
}

return HAL_Status;
}


/****************************************************************
*  END OF  FILE: Bootloader.c
****************************************************************/
