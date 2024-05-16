/****************************************************************
 FILE DESCRIPTION
----------------------------------------------------------------
*        File     :  Bootloader.h
*        Brief    :  Bootloader.
*        Details  :  - 
*
****************************************************************/
#ifndef BOOTLOADER_H
#define BOOTLOADER_H
/****************************************************************
* INCLUDES
****************************************************************/
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include "crc.h"
/****************************************************************
*    GLOBAL DATA TYPE AND INSTRUCTIONS Union, Enum & Struct
****************************************************************/
typedef enum
{
   BL_NACK = 0,
	 BL_ACK

}ACK_Status;

typedef enum
{
   BL_Not_Passed = 0,
	 BL_Passed

}BL_Status;

typedef enum
{
   CRC_NOT_PASSED= 0,
	 CRC_PASSED

}CRC_Status;



typedef void (*Main_Function) (void);
typedef void (*Jump_to_Address) (void);
/****************************************************************
*    GLOBAL CONSTANT MACROS
****************************************************************/
#define APPLICATION_BASE_ADDRESS          0x08008000UL
#define BL_CRC                            &hcrc 
#define BOOTLOADER_USART1                 &huart1
#define BOOTLOADER_USART2                 &huart2

#define BL_USART_ENABLE_DEBUG_MESSAGE     0x0
#define BL_SPI_ENABLE_DEBUG_MESSAGE       0x1
#define BL_CAN_ENABLE_DEBUG_MESSAGE       0x2
#define SELECTED_PROTOCOL                 (BL_USART_ENABLE_DEBUG_MESSAGE)

#define BL_RECEIVING_DATA_BUFFER          0x200
#define BL_CRC_SIZE_BYTE                  0x4

#define BL_VERSION_DATA_SIZE              0x4
#define BL_VERSION_VENDOR_ID              0xff
#define BL_VERSION_MAJOR_VERSION          0x1
#define BL_VERSION_MINOR_VERSION          0x0
#define BL_VERSION_PATCH_VERSION          0x0

#define BL_VERSION_VENDOR_ID_POS          0x0
#define BL_VERSION_MAJOR_VERSION_POS      0x1
#define BL_VERSION_MINOR_VERSION_POS      0x2
#define BL_VERSION_PATCH_VERSION_POS      0x3

#define BL_COMMANDS_SIZE                  0xC

#define ACK                               0xAA
#define NACK                              0xBB

/*Command Information*/
#define CBL_GET_VER_CMD                   0x10
#define CBL_GET_HELP_CMD                  0x11
#define CBL_GET_CID_CMD                   0x12
/* Get Read Protection Status */	     
#define CBL_GET_RDP_STATUS_CMD            0x13
#define CBL_GO_TO_ADDR_CMD                0x14
#define CBL_FLASH_ERASE_CMD               0x15
#define CBL_MEM_WRITE_CMD                 0x16
/* Enable/Disable Write Protection */
#define CBL_EN_R_W_PROTECT_CMD            0x17
#define CBL_MEM_READ_CMD                  0x18
/* Get Sector Read/Write Protection Status */
#define CBL_READ_SECTOR_STATUS_CMD        0x19
#define CBL_OTP_READ_CMD                  0x20
/* Change Read Out Protection Level*/
#define CBL_CHANGE_ROP_Level_CMD          0x21


#define VALID_ADDRESS                     0x1
#define INVALID_ADDRESS                   0x0


/*Memories Base Sizes*/
#define FLASH_MEMORY_SIZE                 (0x400 * 0x400)
#define SRAM1_MEMORY_SIZE                 (0x70 * 0x400)
#define SRAM2_MEMORY_SIZE                 (0x10 * 0x400)
#define CCRAM_MEMORY_SIZE                 (0x40 * 0x400)

#define FLASH_MEMORY_END                  (FLASH_BASE + FLASH_MEMORY_SIZE)
#define SRAM1_MEMORY_END                  (SRAM1_BASE + SRAM1_MEMORY_SIZE)
#define SRAM2_MEMORY_END                  (SRAM2_BASE + SRAM2_MEMORY_SIZE)
#define CCRAM_MEMORY_END                  (CCMDATARAM_BASE + FLASH_MEMORY_SIZE)

#define FLASH_SECTORS_NUM                 0xC
#define FLASH_MASS_ERASE                  0xFF



#define FLASH_READ_PROTECTION_LEVEL0      0xAA
#define FLASH_READ_PROTECTION_LEVEL1      0x55
#define FLASH_READ_PROTECTION_LEVEL2      0xCC

 
/****************************************************************
*    GLOBAL DATA PROTOTYPES
****************************************************************/
/*
      Brief  : Bootloader send message. 
	  Details  : send message by USART.
	  param[in]: Pointer to char.
	  param[in]: Pin Led pin.  
*/
void BL_SendMessage(char* Message , ...);
void Test(void);


/*
    Brief    : Bootloader send message. 
	  Details  : send message by USART.
	  param[in]: char.
	    
*/
HAL_StatusTypeDef BL_Get_Host_Command(void);

/*
    Brief    : Jump to main function. 
	  Details  : Jump to main function.
	  param[in]: None.
	    
*/

/*static*/ HAL_StatusTypeDef Bootloader_Mass_Erase(uint8_t* Host_Buffer);

#endif
/****************************************************************
*  END OF  FILE: Bootloader.h
****************************************************************/



