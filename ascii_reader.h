#ifndef _ASCII_READER
#define _ASCII_READER

#include "PCM_and_measurement_structs.h"


int iPCMInitNUMTWO(struct suPCMInfo * psuPCMInfo, uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode );
char * trimwhitespace(char * str);


#endif //end #ifndef _ASCII_READER
