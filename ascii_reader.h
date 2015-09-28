#ifndef _ASCII_READER
#define _ASCII_READER

#include "PCM_and_measurement_structs.h"


int iPCMInitNUMTWO(struct suPCMInfo * psuPCMInfo,struct suMeasurementInfo ** ppsuMeasInfo,
		   uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode );

void vInitPCMASCIIArrays(struct suPCMInfo * psuPCMInfo);
void vFreePCMASCIIArrays(struct suPCMInfo * psuPCMInfo);

char * trimwhitespace(char * str);


#endif //end #ifndef _ASCII_READER
