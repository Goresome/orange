#ifndef _UTILS_H_
#define _UTILS_H_

bool isFile(const char * file);
void MsgBox(const char * str);
void xor3(unsigned char* ptr, unsigned int len);

extern short RoadPathTable[16];
extern int8 RoadX[8];
extern int8 RoadY[8];

#endif