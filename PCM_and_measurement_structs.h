//2015/09/21
//Structures containing information on  the PCM stream and measurements within the PCM stream

#ifndef _PCM_AND_MEAS_STRUCTS
#define _PCM_AND_MEAS_STRUCTS

struct suPCMInfo
{

    uint16_t        uTMLink;
    uint64_t        ullBitRate;

    uint16_t        uAsymWRInd;
    uint16_t        uAsymFRInd;
    int64_t         llMinorFramesPerMajorFrame;
    uint16_t        uSFIDIdx;
    double          dWordPeriod;

    int16_t         iNMeasurements;

    uint64_t        ullSampBitLength;	       //Sample size (in bytes)
    uint64_t        ullSampsPerMinorFrame;     //Number of samples to grab at each periodic interval
    uint64_t        ullBytesPerMinorFrame;
    uint32_t        ulMinorFrameSampCount;
    uint64_t        ullMinorFrameCount;
    uint64_t        ullSkippedFrameCount;
    uint16_t        uMinorFrameBitShift;

    uint64_t        ullBytesPerMajorFrame;
    int64_t         llMajorFrameCount;
    uint16_t        uNumMFCounters;
    uint16_t  *     pauMFCIndices;
    uint64_t  *     paullMajorFrameVals;
    uint64_t        ullFirstMFCVal;
    uint64_t        ullCurrentMFCVal;
    uint64_t        ullGPSMFCVal;

    uint64_t        ullCounterVal;

    uint16_t        uNGPSWordsInPCM;
    int64_t         llFirstGPSWord;
    int64_t         llCurrentGPSWord;
    uint64_t        ullGPSWordCount;
    uint64_t        ullGPSWordStreakCount;     //How long this word has persisted (measures drift time within PCM stream)
    uint16_t  *     pauGPSMeasIdx;	       //GPS 1pps measurement indices (indexing measurements in PCM header from zero )
    
    uint8_t         bDoSearchFrameForWord;     //Perform a search of each minor frame for a particular word in order to produce a timestamp
};

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
    char        szTSSearchWord[DEF_STR_SIZE]; 
    char        szTStampFile[DEF_STR_SIZE];
    FILE *      psuTStampFile;
};

#endif //end #ifndef _PCM_AND_MEAS_STRUCTS
