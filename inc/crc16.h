/*
 * crc16.h
 *
 *  Created on: 18/03/2014
 *      Author: Renato Coral Sampaio
 */

#ifndef CRC16_H_
#define CRC16_H_

unsigned short CRC16(unsigned short crc, char data);
unsigned short calcula_CRC(unsigned char *commands, int size);

#endif /* CRC16_H_ */