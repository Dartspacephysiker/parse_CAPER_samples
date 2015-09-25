#ifndef _PARSE_PCM
#define _PARSE_PCM

//function declarations
int iPCMInit(struct suPCMInfo * psuPCMInfo, uint16_t uTMLink, uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode );

int iMeasurementInit(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, 
		     uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode );


uint16_t uParseMeasurementSamples(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int iMeasIdx, 
				  uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, 
				  uint64_t * pullWordsWritten,
				  uint8_t bCombineTM1Meas, uint8_t bAssembleCounter, uint8_t iWriteSamplesToFile);
void vWriteSample(uint64_t ullSample, int64_t llSampleIdx, struct suMeasurementInfo * psuMeasInfo, int iWriteMode, uint8_t bIsCombined, uint64_t * pullWordsWritten);

uint16_t combine_MSB_LSB_sample(uint16_t uMSBSample, uint16_t uLSBSample, 
				uint16_t uMSBShift, uint16_t uLSBShift, 
				uint16_t uJustification, uint8_t bMSBIsFirst);


uint64_t ullAssembleCounterVal(struct suPCMInfo * psuPCMInfo, int64_t llMinorFrameIdx, uint64_t * pullMFCVal);


uint8_t bFoundFirstMFCValGPSWordAndSearchWords(FILE * psuInFile, size_t szInFileSize, 
					       struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo,
					       uint16_t * pauMinorFrame, uint8_t bCombineTM1Meas, int64_t * pllPCMWdOffset_GPS);

//GPS word routines
int bGotNewGPSWord(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, uint16_t * pauMinorFrame,
			uint8_t bCombineTM1Meas);
void vUpdateGPSWord(struct suPCMInfo * psuPCMInfo, int64_t * pllPCMWdOffset_MajorFrame, int64_t * pllPCMWdOffset_GPS);

//Timestamp routines
void vSetTStampInfo(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, int iWdIdx );
int iCalcTStamps(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo, uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, int64_t * pllPCMWdOffset_MajorFrame, int64_t * pllPCMWdOffset_MinorFrame, int64_t * pllPCMWdOffset_GPS, uint64_t * pullWordsWritten, uint8_t bCombineTM1Meas);
int iWriteMeasurementTStamps(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, 
			     int64_t llPCMWdOffset_MajorFrame, int64_t llPCMWdOffset_MinorFrame, int64_t llPCMWdOffset_GPS, uint64_t * pullWordsWritten);

int64_t llGetMinorFrameIdx(struct suPCMInfo * psuPCMInfo, uint16_t * pauMinorFrame);
uint8_t bBadSFIDIncrement(struct suPCMInfo * psuPCMInfo, int64_t llMinorFrameIdx, int64_t llOldMinorFrameIdx);

//Routines for finding search words
uint8_t bIsSearchWordInMinorFrame(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, uint16_t * pauMinorFrame, int64_t llMinorFrameIdx);
uint8_t bIsSearchWord(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, uint16_t * pauMinorFrame, uint16_t uWdIdx, int64_t llMinorFrameIdx);
void vUpdateSearchWord(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, int iWdIdx);

//Routines for freeing memory
int iPCMFree(struct suPCMInfo * psuPCMInfo);
int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo);

void vPrintSubFrame (uint16_t * pauMajorFrame, int64_t llMinorFrameIdx);
void vPrintPCMInfo (struct suPCMInfo * psuPCMInfo);
void vPrintMeasurementInfo (struct suMeasurementInfo * psuMeasInfo);

void vUsage(void);

#endif //end #ifndef _PARSE_PCM
