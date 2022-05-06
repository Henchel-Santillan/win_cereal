#ifndef WIN_CEREAL_SERIAL_WRAPPER_H
#define WIN_CEREAL_SERIAL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdio.h>
#include <string.h>

#include <windows.h>


BOOL OpenCOMPort(int, int);
BOOL CloseCOMPort(int); 
int ReadByte(int, unsigned char);
int ReadBuffer(int, unsigned char*, int);
int WriteByte(int, unsigned char);
int WriteBuffer(int, unsigned char*, int);
BOOL GetPortNumber(const char*);

void FlushRX(int);
void FlushTX(int);
void FlushRXTX(int);


#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif // WIN_CEREAL_SERIAL_WRAPPER_H