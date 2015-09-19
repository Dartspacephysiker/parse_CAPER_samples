#ifndef _PARSE_PCM_SAMPLES
#define PARSE_PCM_SAMPLES

//#define DEF_N_SAMPBITS                   16 //in bits
#define DEF_SAMPSPERMINORFRAME          120
#define DEF_MINOR_PER_MAJOR              32

#define DEF_TM_LINK                       1

#define DEF_OUTPREFIX  "parsed_TM1-samples"
#define DEF_COMBINE_TM1                   0    //combine TM1 channels on the fly
#define DEF_DO_CHECK_SFID_INCREMENT       0    //Check that SFID increments uniformly
#define DEF_ASSEMBLE_COUNTER              0    //Create and output unique counter produced by major and minor frame counters
#define DEF_CALC_TSTAMPS                  0    //Create and output timestamps based on GPS 1 pps
#define DEF_VERBOSE                       0    //please tell me

#define DEF_STR_SIZE                   1024

#define MAX_N_MINORFRAMES               256
#define MAX_GPS_WORDS                     2

//struct declarations
struct suMeasurementInfo
{
    char        szName[DEF_STR_SIZE];          //Name of the measurement, e.g., "Langmuir Probe Measurement 1 MSB"
    char        szAbbrev[DEF_STR_SIZE];        //Abbreviation for measurement, e.g., "LP01MSB"
    char        szUser[DEF_STR_SIZE];	       //Who is the user? E.g., Dartmouth, U Iowa
    uint16_t    uWord;			       //Beginning word in the frame
    uint16_t    uWdInt;			       //Word interval
    uint16_t    uSampsPerMinorFrame;	       //How many of these to expect per frame? Most are just one.
    uint16_t    uMinorFrame;		       //Which minor frame is it in?
    uint16_t    uMinorFrInt;		       //How often does it show up?
    uint32_t    ulSPS;

    int32_t   * palSample;
    uint64_t    ullSampCount;

    uint16_t    uNAsymWRanges;
    uint16_t ** ppauAsymWRanges;
//    uint16_t  * uAsymWRanges[2];
    uint16_t  * pauWtemp;
    
    uint16_t    uNAsymFRanges;
    uint16_t ** ppauAsymFRanges;
//    uint16_t  * uAsymFRanges[2];
    uint16_t  * pauFtemp;

    uint16_t    uLSBWord;

    char        szOutFile[DEF_STR_SIZE];
    FILE *      psuOutFile;

    uint8_t     bTSCalcEnabled;
    uint64_t  * pallWordOffsets;
    uint16_t    uOffsetBufCount;
    char        szTStampFile[DEF_STR_SIZE];
    FILE *      psuTStampFile;
};

//function declarations
int iMeasurementInit(struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, uint64_t gullSampsPerMinorFrame, 
		     uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode );
int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo);

uint16_t uParseMeasurementSamples(struct suMeasurementInfo * psuMeasInfo, int iMeasIdx, 
				  uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, uint64_t gullSampsPerMinorFrame, uint32_t * pulMinorFrameSampCount, 
				  uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
				  uint64_t * pullWordsWritten,
				  uint8_t bCombineTM1Meas, uint8_t bAssembleCounter, uint8_t bWriteSamplesToFile);

void vPrintMeasurementInfo (struct suMeasurementInfo * psuMeasInfo);
void vPrintSubFrame (uint16_t * pauMajorFrame, int64_t llMinorFrameIdx);

//uint16_t combine_MSB_LSB_sample(uint16_t uMSBSample, uint16_t uLSBSample, uint16_t uMSBShift, uint16_t uLSBShift, uint16_t uJustification, uint8_t bMSBIsFirst);

uint64_t ullAssembleCounterVal(uint64_t * paullMajorFrameVals, int64_t llMinorFrameIdx, uint16_t uNumMFCounters, uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint64_t * pullMFCVal);

uint8_t bFoundFirstMFCValAndGPSWord(FILE * psuInFile, size_t szInFileSize, 
				    struct suMeasurementInfo ** ppsuMeasInfo, int16_t iNMeasurements,
				    uint16_t * pauMinorFrame, uint64_t gullSampsPerMinorFrame, uint64_t * pullCounterVal, 
				    uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
				    uint64_t * pullFirstMFCVal,
				    uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint8_t bCombineTM1Meas,
				    uint64_t * pllCurrentGPSWord, uint64_t * pullGPSWordCount, uint64_t * pullGPSWordStreakCount,
				    uint16_t uGPSMeasIdx, int64_t * pllWordOffset_GPS);

uint64_t ullGetFirstMFCValAndGPSWord(FILE * psuInFile, size_t szInFileSize, 
				     struct suMeasurementInfo ** ppsuMeasInfo, int16_t iNMeasurements,
				     uint16_t * pauMinorFrame, uint64_t gullSampsPerMinorFrame, 
				     uint64_t * pullCounterVal, uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
				     uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint8_t bCombineTM1Meas, 
				     uint64_t * pllCurrentGPSWord, uint64_t * pullGPSWordCount, uint64_t * pullGPSWordStreakCount,
				     uint16_t uGPSMeasIdx, int64_t * pllWordOffset_GPS);

int bCheckForNewGPSWord(struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, uint16_t * pauMinorFrame,
			int64_t * pllCurrentGPSWord, uint64_t * pullGPSWordCount, uint64_t * pullGPSWordStreakCount, 
			uint8_t bCombineTM1Meas);

int iWriteMeasurementTStamps(struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx,
			     int64_t llWordOffset_MajorFrame, int64_t llWordOffset_MinorFrame,
			     int64_t llWordOffset_GPS, uint64_t ullGPSWordCount);

int64_t llGetMinorFrameIdx(uint16_t * pauMinorFrame);

uint8_t bBadSFIDIncrement(int64_t llMinorFrameIdx, int64_t llOldMinorFrameIdx);

void vUsage(void);

//global var declarations
int64_t  gllMinorFramesPerMajorFrame;
uint64_t gullSampsPerMinorFrame;	       //Number of samples to grab at each periodic interval
uint16_t guAsymWRInd;
uint16_t guAsymFRInd;
uint16_t guSFIDIdx;
uint16_t guTMLink;
double   gdWordPeriod;

#endif //end #ifndef _PARSE_PCM_SAMPLES
