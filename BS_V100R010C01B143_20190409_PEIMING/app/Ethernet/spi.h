
#ifndef __SPI_H
#define __SPI_H
/* Includes ------------------------------------------------------------------*/


/* Exported Functions --------------------------------------------------------*/

void SPI_Configuration(void);
void SPI_WriteByte(uint8_t TxData);
uint8_t SPI_ReadByte(void);
void SPI_CrisEnter(void);
void SPI_CrisExit(void);
void SPI_CS_Select(void);
void SPI_CS_Deselect(void);

#endif /* __SPI_H */

/*********************************END OF FILE**********************************/
