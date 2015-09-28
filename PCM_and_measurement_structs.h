//2015/09/21
//Structures containing information on  the PCM stream and measurements within the PCM stream

#ifndef _PCM_AND_MEAS_STRUCTS
#define _PCM_AND_MEAS_STRUCTS

static char   * PCM_CONF_VALS[]   = {"NAME",
				     "N_MEASUREMENTS",
				     "SFID_IDX",
				     "BPS",
				     "SAMPLE_BITLENGTH",
				     "N_SAMPLES_PER_MINOR",
				     "N_MINOR_PER_MAJOR",
				     "N_MAJFRAMECOUNTERS",
				     "N_MINFRAME_BITPOS",
				     "N_GPS_WORDS"};

static char  * PCM_MEAS_ARRAYS[] = {"MEAS_NAME",
				    "MEAS_ABBREV",
				    "MEAS_USER",
				    "MEAS_SAMPLE_LOC_IN_MINFRAME",
				    "MEAS_SAMPLE_INTERVAL_IN_MINFRAME",
				    "MEAS_SAMPLE_FRAME",
				    "MEAS_FRAME_INTERVAL",
				    "MEAS_SAMPLE_RATE",
				    "MEAS_N_ASYM_WORD_RANGES",
				    "MEAS_ASYM_WORD_RANGES",
				    "MEAS_N_ASYM_FRAME_RANGES",
				    "MEAS_ASYM_FRAME_RANGES",
				    "MEAS_LSB_WORD",
				    "MEAS_TSTAMP_CALC_TYPE",
				    "MEAS_TSTAMP_SEARCH_WORD",
				    "MEAS_INTERN_SAMP_RATE",
				    "MAJFRAME_INDEX",
				    "GPS_INDEX"};


struct suPCMInfo
{

    char            szName[128];
    uint16_t        uTMLink;			       //Variables defined by NASA's PCM measurement doc
    uint64_t        ullBitRate;
    uint16_t        uAsymWRInd;
    uint16_t        uAsymFRInd;
    int64_t         llMinorFramesPerMajorFrame;
    uint16_t        uSFIDIdx;
    double          dWordPeriod;

    int16_t         iNMeasurements;

    uint64_t        ullSampBitLength;		       //Sample size (in bytes)
    uint64_t        ullSampsPerMinorFrame;	       //Number of samples to grab at each periodic interval
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

    uint64_t        ullCounterVal;		       //Master counter, composed of all major frame counters and the minor frame counter

    uint16_t        uNGPSWordsInPCM;		       //How many GPS words are defined by the PCM measurement list?
    int64_t         llFirstGPSWord;		       //The first GPS word in the file
    int64_t         llCurrentGPSWord;		       //The current GPS word as we loop through the file
    uint64_t        ullGPSWordCount;		       //How many non-zero GPS words seen so far (critical for time calculation)
    uint64_t        ullGPSWordStreakCount;             //How long this word has persisted (measures drift time within PCM stream)
    uint16_t  *     pauGPSMeasIdx;	               //GPS 1pps measurement indices (indexing measurements in PCM header from zero )
    
    //    uint8_t         bDoSearchFrameForWord;             //Perform a search of each minor frame for a particular word in order to produce a timestamp
};


struct suMeasurementInfo			       //Struct for keeping track of a measurement within the PCM stream
{
    /* Variables defined in NASA's PCM measurement list */
    char            szName[DEF_STR_SIZE];	       //Name of the measurement, e.g., "Langmuir Probe Measurement 1 MSB"
    char            szAbbrev[DEF_STR_SIZE];	       //Abbreviation for measurement, e.g., "LP01MSB"
    char            szUser[DEF_STR_SIZE];	       //Who is the user? E.g., Dartmouth, U Iowa
    uint16_t        uWord;			       //Beginning word in the frame
    uint16_t        uWdInt;			       //Word interval
    uint16_t        uSampsPerMinorFrame;	       //How many of these to expect per frame? Most are just one.
    uint16_t        uMinorFrame;		       //Which minor frame is it in?
    uint16_t        uMinorFrInt;		       //How often does it show up?
    uint32_t        ulSPS;
		    
    uint64_t   *    paullSample;		       //Buffer for samples, or a sample count if uTSCalcType == 3
    uint64_t        llSampIdx;
    int64_t         llTotalSampCount;
		    
    uint16_t        uNAsymWRanges;		       //Vars for asymmetric word ranges defined by NASA PCM doc
    uint16_t **     ppauAsymWRanges;
    uint16_t  *     pauWtemp;
    		    
    uint16_t        uNAsymFRanges;		       //Vars for asymmetric minor-frame ranges defined by NASA PCM doc
    uint16_t **     ppauAsymFRanges;
    uint16_t  *     pauFtemp;
		    
    uint8_t         uTSCalcType;                       //Variables for calculation of timestamps (NOTE: "TSSW" = timestamp search word)
    int64_t  *      pallPCMWdOffsets;		       //CalcType = 1: TStamps calculated relative to GPS word, samples output to binary
    uint16_t        uOffsetBufCount;		       //CalcType = 2: TStamps calculated relative to GPS word, samples and TStamps output to ASCII
    char            szTSSearchWord[DEF_STR_SIZE];      //Calctype = 3: TStamps calculated rel. to TSSW, using measurement's word period.
    int64_t         llPCMWdOffset_TSSW;                //Calctype = 4: TStamps for searchword are outputted along with TSSW sample number
    int64_t         llTSIdx;
    uint16_t        uTSSWIdx;
    int64_t         llTSSW_SampleNum;
    int64_t         llTSSW_MFCVal;
    double          dInternalWordPeriod;
    char            szTStampFile[DEF_STR_SIZE];
    FILE *          psuTStampFile;

    char            szOutFile[DEF_STR_SIZE];	       //Stuff for output files
    FILE *          psuOutFile;

    uint16_t        uLSBWord;			       //If this measurement is the MSB word of a data consisting of both 
						       //an MSB and and LSB word, this var points to the corresponding LSB
						       //word in the frame
};

#endif //end #ifndef _PCM_AND_MEAS_STRUCTS
