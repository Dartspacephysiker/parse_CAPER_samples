/*

2015/09/07 Beginnings
2015/09/18 Adding time calculation
           NOTE: This assumes that we don't skip the last frame! Otherwise the major frame
                 count is botched.
*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include "TM1_majorframe.h"
#include "TM23_majorframe.h"

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
int iMeasurementInit(struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, uint64_t ullSampsPerMinorFrame, 
		     uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode );
int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo);

uint16_t uParseMeasurementSamples(struct suMeasurementInfo * psuMeasInfo, int iMeasIdx, 
				  uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, uint64_t ullSampsPerMinorFrame, uint32_t * pulMinorFrameSampCount, 
				  uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
				  uint64_t * pullWordsWritten,
				  uint8_t bCombineTM1Meas, uint8_t bAssembleCounter, uint8_t bWriteSamplesToFile);

void vPrintMeasurementInfo (struct suMeasurementInfo * psuMeasInfo);
void vPrintSubFrame (uint16_t * pauMajorFrame, int64_t llMinorFrameIdx);

//uint16_t combine_MSB_LSB_sample(uint16_t uMSBSample, uint16_t uLSBSample, uint16_t uMSBShift, uint16_t uLSBShift, uint16_t uJustification, uint8_t bMSBIsFirst);

uint64_t ullAssembleCounterVal(uint64_t * paullMajorFrameVals, int64_t llMinorFrameIdx, uint16_t uNumMFCounters, uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint64_t * pullMFCVal);

uint64_t ullGetFirstMFCVal(FILE * psuInFile, size_t szInFileSize, 
			   struct suMeasurementInfo ** ppsuMeasInfo, int16_t iNMeasurements,
			   uint16_t * pauMinorFrame, uint64_t ullSampsPerMinorFrame, 
			   uint64_t * pullCounterVal, uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
			   uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint8_t bCombineTM1Meas,uint64_t * pllCurrentGPSWord);

void vCheckForNewGPSWord(struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, 
			 uint16_t * pauMinorFrame,int64_t * pllCurrentGPSWord, uint8_t bCombineTM1Meas);

int iWriteMeasurementTStamps(struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx,
			     int64_t llWordOffset_MajorFrame, int64_t llWordOffset_MinorFrame, int64_t llWordOffset_GPS);

int64_t llGetMinorFrameIdx(uint16_t * pauMinorFrame);

int iBadSFIDIncrement(int64_t llMinorFrameIdx, int64_t llOldMinorFrameIdx);

void vUsage(void);

//global var declarations
int64_t  gllMinorFramesPerMajorFrame;
uint16_t guAsymWRInd;
uint16_t guAsymFRInd;
uint16_t guSFIDIdx;
uint16_t guTMLink;
double   gdWordPeriod;

//struct suMeasurementInfo * psuMeasurementInit(int16_t iMeasIdx);

int main( int argc, char * argv[] )
{
    //************
    //Declare vars

    char         szInFile[DEF_STR_SIZE];		       //input/output file stuff
    char         szUniqCounterFile[DEF_STR_SIZE];

    FILE       * psuInFile;
    FILE       * psuUniqCounterFile;
    
    char         szOutPrefix[DEF_STR_SIZE];

    struct stat  suInFileStat;		       //input stuff
    uint64_t     ullSampBitLength;	       //Sample size (in bytes)
    uint64_t     ullSampsPerMinorFrame;	       //Number of samples to grab at each periodic interval
    //    uint64_t     ullNBytesPerMinorFrame;	       //Number of bits in MinorFrame

    uint16_t   * pauMinorFrame;
    uint64_t     ullBytesPerMinorFrame;

    struct suMeasurementInfo ** ppsuMeasInfo;
    int16_t      iMeasIdx;
    int16_t      iNMeasurements;

    uint16_t     uAsymIdx;

    int64_t      llMinorFrameIdx;
    int64_t      llOldMinorFrameIdx;
    uint64_t     ullBytesPerMajorFrame;
    int64_t      llMajorFrameCount;
    uint16_t  *  pauIsMinorFrameCollected;
    uint16_t **  ppauMajorFrame;
    uint16_t  *  pauMajFTemp;

    uint64_t     ullInFilePos;
    uint64_t     ullWordsRead;
    uint64_t     ullWordsWritten;
    uint64_t     ullNSampsRead;
    uint64_t     ullTotWordsRead;
    uint64_t     ullTotWordsWritten;

    uint32_t     ulMinorFrameSampCount;
    uint64_t     ullMinorFrameCount;
    uint64_t     ullSkippedFrameCount;

    int          iArgIdx;

    uint8_t      bCombineTM1Meas;
    uint8_t      bDoCheckSFIDIncrement;
    uint8_t      bTStampMode;
    uint64_t     ullFirstMFCVal;
    uint64_t     ullCurrentMFCVal;

    uint8_t      bAssembleCounter;
    uint8_t      bWriteCounter;
    uint16_t     uNumMFCounters;
    uint16_t  *  pauMFCIndices;
    uint64_t  *  paullMajorFrameVals;
    uint64_t     ullCounterVal;
    uint16_t     uMinorFrameBitShift;

    uint16_t     uNGPSWords;
    int64_t      llCurrentGPSWord;
    uint16_t  *  pauGPSMeasIdx; //GPS 1pps measurement indices (indexing from zero in the arrays above)
    int64_t      llWordOffset_MajorFrame; //Current major frame offset (in words) relative to first recorded major frame
    int64_t      llWordOffset_MinorFrame;
    int64_t      llWordOffset_GPS;


    uint8_t      bVerbose;

    //*******************
    //Initialize global vars
    gllMinorFramesPerMajorFrame    = DEF_MINOR_PER_MAJOR;
    guAsymWRInd                    = 0;
    guAsymFRInd                    = 0;
    guSFIDIdx                      = 0;
    guTMLink                       = DEF_TM_LINK;
    gdWordPeriod                   = 0;

    //Initialize vars
    ulMinorFrameSampCount          = 0;
    ullMinorFrameCount             = 0;
    ullSkippedFrameCount           = 0;
    llMajorFrameCount              = 0;
			           
    iArgIdx                        = 0;
			           
    bCombineTM1Meas                = DEF_COMBINE_TM1;
    bDoCheckSFIDIncrement          = DEF_DO_CHECK_SFID_INCREMENT;
    bTStampMode                    = DEF_CALC_TSTAMPS;
    ullFirstMFCVal                 = 0;
    ullCurrentMFCVal               = 0;

    bAssembleCounter               = DEF_ASSEMBLE_COUNTER;
    bWriteCounter                  = 0;
    uNumMFCounters                 = 0;
    pauMFCIndices                  = NULL;
    paullMajorFrameVals            = NULL;
    ullCounterVal                  = 0;
    uMinorFrameBitShift            = 0;
			           
    uNGPSWords                     = 0;
    pauGPSMeasIdx                  = NULL;
    llCurrentGPSWord               = 0;
    llWordOffset_MajorFrame        = 0;
    llWordOffset_MinorFrame        = 0;
    llWordOffset_GPS               = 0;
			           
    bVerbose                       = DEF_VERBOSE;

    if (argc < 2)
	{
	vUsage();
	return EXIT_SUCCESS;
	}

    szInFile[0]  = '\0';
    strcpy(szUniqCounterFile,"");		       // Default is stdout

    psuInFile = (FILE *) NULL;
    psuUniqCounterFile = (FILE *) NULL;
    strncpy(szOutPrefix,DEF_OUTPREFIX,DEF_STR_SIZE);

    //    ullSampBitLength           = DEF_N_SAMPBITS;
    //    ullSampBitLength           = 0;
    ullSampsPerMinorFrame      = DEF_SAMPSPERMINORFRAME;
    ullBytesPerMinorFrame      = ullSampsPerMinorFrame * sizeof(uint16_t);

    ppsuMeasInfo = (struct suMeasurementInfo **) NULL;
    iMeasIdx                   = 0;

    llMinorFrameIdx            = 0;
    llOldMinorFrameIdx         = 0;
    ullInFilePos               = 0;
    ullWordsRead               = 0;
    ullWordsWritten            = 0;
    ullNSampsRead              = 0;
    ullTotWordsRead            = 0;
    ullTotWordsWritten         = 0;

    for (iArgIdx=1; iArgIdx<argc; iArgIdx++)   //start with 1 to skip input file
	{

	switch (argv[iArgIdx][0])
	    {
	    int iTmp;
	    case '-' :
		switch (argv[iArgIdx][1])
		    {
		    /* case 's' :                   /\* Sample size *\/ */
		    /* 	iArgIdx++; */
		    /* 	if(iArgIdx >= argc) */
		    /* 	    { */
		    /* 	    vUsage(); */
		    /* 	    return EXIT_FAILURE; */
		    /* 	    } */
		    /* 	sscanf(argv[iArgIdx],"%" PRIu64 ,&ullSampBitLength); */
		    /* 	break; */

		    case 'n' :                   /* # Samples */
			iArgIdx++;
			if(iArgIdx >= argc)
			    {
			    vUsage();
			    return EXIT_FAILURE;
			    }
			sscanf(argv[iArgIdx],"%" PRIu64 ,&ullSampsPerMinorFrame);
			break;

		    case 'L' :                   /* TM Link # */
			iArgIdx++;
			if(iArgIdx >= argc)
			    {
			    vUsage();
			    return EXIT_FAILURE;
			    }
			sscanf(argv[iArgIdx],"%" PRIu16 ,&guTMLink);
			break;

		    case 'P' :                  /* Prefix for output files */
			iArgIdx++;
			strncpy(szOutPrefix, argv[iArgIdx],DEF_STR_SIZE);
			break;

		    case 'C' :                  /* Combine TM1 MSB/LSB measurements */
			bCombineTM1Meas       = !bCombineTM1Meas;
			break;

		    case 'c' :                  /* Check that SFID increments uniformly */
			bDoCheckSFIDIncrement = !bDoCheckSFIDIncrement;
			break;

		    case 'A' :                  /* Assemble and write unique counter */
			bAssembleCounter      = !bAssembleCounter;
			bWriteCounter         = !bWriteCounter;
			break;

		    case 'T' :                  /* Produce timestamps based on GPS pulse */
			bTStampMode           = !bTStampMode;
			bAssembleCounter      = !bAssembleCounter; //Need counter for TStampMode
			break;

		    case 'v' :                  /* Verbosities */
			bVerbose = 1;
			break;

		    case 'h' :                  /* Verbosities */
			vUsage();
			return EXIT_SUCCESS;
			break;

		    default :
			break;
		    } /* end flag switch */
		break;

	    default :
		if (szInFile[0] == '\0') strcpy(szInFile, argv[iArgIdx]);
		//		else                     strcpy(szUniqCounterFile,argv[iArgIdx]);
		break;

	    } /* end command line arg switch */
	} /* end for all arguments */

    if (guTMLink == 1 )
	{
	printf("TM1 : SKIN/ELF/VF/VLF/AGC samples\n");

	iNMeasurements                   = N_TM1_MEASUREMENTS;
        guSFIDIdx                        = TM1_SFID_IDX;
			                 
	uNumMFCounters                   = TM1_NUM_MFCOUNTERS;
	paullMajorFrameVals              = (uint64_t *) malloc(uNumMFCounters * sizeof(uint64_t));
	pauMFCIndices                    = (uint16_t *) malloc(uNumMFCounters * sizeof(uint16_t));
	for ( iArgIdx = 0; iArgIdx < uNumMFCounters; iArgIdx++ )
	    {			         
	    pauMFCIndices[iArgIdx]       = auTM1MFCMeasIdx[iArgIdx];
	    paullMajorFrameVals[iArgIdx] = -1;
		//		printf("MFC Index: %" PRIu16 "\n",pauMFCIndices[iArgIdx]);
	    }			         
				         
	ullSampBitLength                 = TM1_WORD_BITLENGTH;
	uMinorFrameBitShift              = TM1_MINORFRAME_BITSHIFT;
	gdWordPeriod                     = ullSampBitLength/TM1_BPS;
			                   
	if ( bTStampMode )               
	    {		                   
	    uNGPSWords                   = TM1_N_GPS_WORDS;
	    pauGPSMeasIdx                = (uint16_t *) malloc(uNGPSWords * sizeof(uint16_t));
	    for ( iArgIdx = 0; iArgIdx < uNGPSWords; iArgIdx++ )
		{		         
		pauGPSMeasIdx[iArgIdx]   = auTM1GPSMeasIdx[iArgIdx];
		}
	    }

	if ( bCombineTM1Meas ) printf("Combining MSB/LSB measurements on TM1!\n");

	}
    else
	{
	printf("TM2/3: RxDSP samples\n");

	iNMeasurements                   = N_TM23_MEASUREMENTS;
        guSFIDIdx                        = TM23_SFID_IDX;
				         
	uNumMFCounters                   = TM23_NUM_MFCOUNTERS;
	paullMajorFrameVals              = (uint64_t *) malloc(uNumMFCounters * sizeof(uint64_t));
	pauMFCIndices                    = (uint16_t *) malloc(uNumMFCounters * sizeof(uint16_t));
	for ( iArgIdx = 0; iArgIdx < uNumMFCounters; iArgIdx++ )
	    {			         
	    pauMFCIndices[iArgIdx]       = auTM23MFCMeasIdx[iArgIdx];
	    paullMajorFrameVals[iArgIdx] = -1;
		//		printf("MFC Index: %" PRIu16 "\n",pauMFCIndices[iArgIdx]);
	    }			         
				         
	ullSampBitLength                 = TM23_WORD_BITLENGTH;
	uMinorFrameBitShift              = TM23_MINORFRAME_BITSHIFT;
	gdWordPeriod                     = ullSampBitLength/TM23_BPS;
				         
	if ( bTStampMode )               
	    {		                   
	    uNGPSWords                   = TM23_N_GPS_WORDS;
	    pauGPSMeasIdx                = (uint16_t *) malloc(uNGPSWords * sizeof(uint16_t));
	    for ( iArgIdx = 0; iArgIdx < uNGPSWords; iArgIdx++ )
		{		         
		pauGPSMeasIdx[iArgIdx]   = auTM23GPSMeasIdx[iArgIdx];
		}		         
	    }			         
				         
	ullSampsPerMinorFrame            = 400;
	ullBytesPerMinorFrame            = ullSampsPerMinorFrame * sizeof(uint16_t);
	gllMinorFramesPerMajorFrame      = 4;

	}

    if ( ullSampBitLength < 1 || ullSampBitLength > 16 )
	{
	fprintf(stderr,"Invalid sample size provided! Exiting...\n");
	return EXIT_FAILURE;
	}
    printf(" \n");

    //init measurements
    //    psuMeasInfo = (struct suMeasurementInfo *) malloc(sizeof(struct suMeasurementInfo) * iNMeasurements);
    ppsuMeasInfo = malloc(iNMeasurements * sizeof(struct suMeasurementInfo *));
    for ( iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	{
        int err;
	//	    psuMeasInfo[iMeasIdx] = psuMeasurementInit(iMeasIdx);
	ppsuMeasInfo[iMeasIdx] = (struct suMeasurementInfo *) malloc(sizeof(struct suMeasurementInfo));
	//	if (psuMeasInfo == (struct suMeasurementInfo *) NULL)
	if (ppsuMeasInfo[iMeasIdx] == (struct suMeasurementInfo *) NULL)
	    {
	    printf("Couldn't initialize measurement %" PRIi16 "!\n",iMeasIdx);
	    return -1;
	    }

	err = iMeasurementInit(ppsuMeasInfo[iMeasIdx],iMeasIdx,szOutPrefix,ullSampsPerMinorFrame, 
			       bCombineTM1Meas, bDoCheckSFIDIncrement, bTStampMode);
	if (bVerbose) vPrintMeasurementInfo(ppsuMeasInfo[iMeasIdx]);
	}


    //initialize major frame, binary array for keeping track of
    ullBytesPerMinorFrame = ullSampsPerMinorFrame * sizeof(uint16_t);
    ullBytesPerMajorFrame = ullBytesPerMinorFrame * gllMinorFramesPerMajorFrame;

    pauMinorFrame = malloc(ullBytesPerMinorFrame);

    pauIsMinorFrameCollected = (uint16_t *) calloc(gllMinorFramesPerMajorFrame,2);
    ppauMajorFrame = (uint16_t **) malloc(gllMinorFramesPerMajorFrame * sizeof(uint16_t *));
    //    pauMajFTemp = malloc(gllMinorFramesPerMajorFrame * ullSampsPerMinorFrame * sizeof(uint16_t));
    pauMajFTemp = malloc(ullBytesPerMajorFrame);

    for (llMinorFrameIdx = 0; llMinorFrameIdx < gllMinorFramesPerMajorFrame; llMinorFrameIdx++) 
	{
	ppauMajorFrame[llMinorFrameIdx] = pauMajFTemp + (llMinorFrameIdx * ullSampsPerMinorFrame);
	}
    if (bVerbose) printf("Major Frame Size   :\t%" PRIu64 " bytes\n",ullBytesPerMajorFrame);

    int Count;

    //Open input file
    if (strlen(szInFile)==0)
	{
	vUsage();
	return EXIT_FAILURE;
	}

    psuInFile = fopen(szInFile,"rb");
    if (psuInFile == NULL)
	{
	fprintf(stderr, "Error opening input file\n");
	return EXIT_FAILURE;
	}
    printf("Input file: %s\n\n", szInFile);

    //Get input file stats
    ullInFilePos = fseek(psuInFile, 0, SEEK_SET); //go to zero, positively
    stat(szInFile, &suInFileStat);

    // If making a unique counter, open a file
    if ( bAssembleCounter && bWriteCounter )
    	{
	sprintf(szUniqCounterFile,"%s--%s.txt",szOutPrefix,"unique_counter");
    	psuUniqCounterFile = fopen(szUniqCounterFile,"w");
    	if (psuUniqCounterFile == NULL)
    	    {
    	    fprintf(stderr, "Error opening file for unique counter\n");
    	    return EXIT_FAILURE;
    	    }

    	printf("Unique counter file: %s\n", szUniqCounterFile);
    	}

    //To the beginning!
    if ( !bDoCheckSFIDIncrement ) 
	printf("Parsing CAPER samples for TM%" PRIu16 "\n",guTMLink);
    else  
	printf("Checking SFID increment; no output will be produced...\n");
    printf("\n");

    //Get first MFC value, if that's what we need to do
    //    if ( bTStampMode )
    //	{
	    ullFirstMFCVal = ullGetFirstMFCVal(psuInFile,suInFileStat.st_size, 
					       ppsuMeasInfo, iNMeasurements,
					       pauMinorFrame,ullSampsPerMinorFrame, 
					       &ullCounterVal, paullMajorFrameVals, pauMFCIndices, uNumMFCounters,
					       ullSampBitLength, uMinorFrameBitShift, bCombineTM1Meas, &llCurrentGPSWord);
	    //break out if we couldn't get it
	    if ( ullFirstMFCVal == -1 ) 
		{
		printf("TStampMode: Unable to get a first major frame count! Is something wrong with your data?\n");
		printf("Exiting...");
		return -1;
		}
	    //	}

    //Loop over whole file
    llOldMinorFrameIdx = 0;
    llMinorFrameIdx    = 1;
    do
    	{
    	ullWordsWritten       = 0;
	ulMinorFrameSampCount = 0;
	//reset global vars
	guAsymWRInd        = 0;
	guAsymFRInd        = 0;

    	ullWordsRead = fread(pauMinorFrame, 2, ullSampsPerMinorFrame, psuInFile);     	//Get samples from infile
    	if (ullWordsRead != ullSampsPerMinorFrame)
    	    {
    	    fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullWordsRead*2,ullBytesPerMinorFrame);
    	    break;
    	    }

	//Determine which minor frame this is
	llOldMinorFrameIdx = llMinorFrameIdx;
	llMinorFrameIdx = llGetMinorFrameIdx(pauMinorFrame);  //The TM list counts from 1, not zero
	if (bVerbose ) printf("Minor frame: %" PRIX64 "\n",llMinorFrameIdx);

	if (bDoCheckSFIDIncrement)
	    {
	    llOldMinorFrameIdx %= gllMinorFramesPerMajorFrame;

	    if ( ullMinorFrameCount > 0 && iBadSFIDIncrement(llMinorFrameIdx,llOldMinorFrameIdx) )
		{
		ullSkippedFrameCount++;
		printf("Minor frame skipped!\n");
		printf("Minor frame number     : %" PRIi64 "\n",llMinorFrameIdx);
		printf("Old minor frame number : %" PRIi64 "\n",llOldMinorFrameIdx);
		printf("Net minor frame count  : %" PRIu64 "\n",ullMinorFrameCount);
		printf("Number of mismatches   : %" PRIu64 "\n",ullSkippedFrameCount);
		printf("\n");
		}
	    }
	else
	    {
	    if ( bTStampMode )
		{
		llWordOffset_MajorFrame = ullCurrentMFCVal * gllMinorFramesPerMajorFrame * ullSampsPerMinorFrame;
		llWordOffset_MinorFrame = ( llMinorFrameIdx - 1 ) * gllMinorFramesPerMajorFrame;

		//Check for new GPS word in this minor frame, then calculate GPS word offset
		vCheckForNewGPSWord(ppsuMeasInfo[pauGPSMeasIdx[0]],llMinorFrameIdx,pauMinorFrame,&llCurrentGPSWord,bCombineTM1Meas);
		llWordOffset_GPS = llWordOffset_MajorFrame + llCurrentGPSWord;
		
		}

	    for (iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
		{
		    uParseMeasurementSamples(ppsuMeasInfo[iMeasIdx],iMeasIdx,
							  pauMinorFrame,llMinorFrameIdx,ullSampsPerMinorFrame,&ulMinorFrameSampCount,
							  paullMajorFrameVals,pauMFCIndices,uNumMFCounters,&ullWordsWritten,
							  bCombineTM1Meas,bAssembleCounter,1);
		}
	    
	    //assemble unique counter, if requested
	    if ( bAssembleCounter )
		{
		    ullCounterVal = ullAssembleCounterVal(paullMajorFrameVals,llMinorFrameIdx,uNumMFCounters,ullSampBitLength, uMinorFrameBitShift, &ullCurrentMFCVal);
		    ullCurrentMFCVal -= ullFirstMFCVal; //Make current MF count relative to value of first recorded MF count
		    if ( bWriteCounter ) fprintf(psuUniqCounterFile,"%" PRIu64 "\n",ullCounterVal);
		}

	    if ( bTStampMode )
	    	{
	    	for (iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	    	    {
		    if ( ppsuMeasInfo[iMeasIdx]->bTSCalcEnabled && ( ppsuMeasInfo[iMeasIdx]->uOffsetBufCount > 0 ) )
	    		{
			iWriteMeasurementTStamps(ppsuMeasInfo[iMeasIdx], llMinorFrameIdx,
						 llWordOffset_MajorFrame, llWordOffset_MinorFrame, llWordOffset_GPS);
	    		}
	    	    }
	    	}
	    }

	//did we get the whole major frame?
	ullMinorFrameCount++;
	//	if (ullMinorFrameCount % gllMinorFramesPerMajorFrame == 0) llMajorFrameCount++;
	if ( llMinorFrameIdx == gllMinorFramesPerMajorFrame ) llMajorFrameCount++; //NOTE: This assumes that we don't skip the last frame!
	if ( bVerbose ) printf("Major frame #%" PRIu64 "\n", llMajorFrameCount);

    	if (bVerbose) 
    	    {
	    // printf("Read %" PRIu64 " bytes (iteration #%" PRIu64 ")\n",ullInFilePos,ullNSampsRead);
    	    printf("N InSamps read     : %" PRIu64 "\n",ullNSampsRead);
    	    printf("InFile Position (bytes) : %" PRIu64 "\n",ullInFilePos);
    	    printf("Read %" PRIu64 " bytes; Wrote %" PRIu64 " bytes\n",ullWordsRead*2,ullWordsWritten);
    	    }

    	ullTotWordsRead += ullWordsRead;
    	ullTotWordsWritten += ullWordsWritten;

    	}  while( ( ullInFilePos = ftell(psuInFile) ) < suInFileStat.st_size  );
    
    //Summary
    printf("Minor frame count           : %" PRIu64 "\n", ullMinorFrameCount);
    printf("Major frame count           : %" PRIu64 "\n", llMajorFrameCount);
    if (bDoCheckSFIDIncrement) 
	{
	printf("Total number of frame skips : %" PRIu64 "\n",ullSkippedFrameCount);
	if ( ullSkippedFrameCount == 0 )
	    printf("\nShe's clean! No frame skips detected\n");
	else
	    printf("\nFrame skips exist within this file.\n");
	}
    else
	{
	printf("\n");
	printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Total Bytes) \n", ullTotWordsWritten*2, (suInFileStat.st_size));
	printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Total Bytes) \n", ullTotWordsWritten*2, (suInFileStat.st_size));
	printf("Output file size is %.2f%% of input file\n", (float)((float)( ullTotWordsWritten*2 )/(float)(suInFileStat.st_size))*100);
	}
    
    fclose(psuInFile); //close input file

    //release the mem!
    for ( iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	{
	if ( ppsuMeasInfo[iMeasIdx] != NULL ) iMeasurementFree(ppsuMeasInfo[iMeasIdx]);
	}
    free(ppsuMeasInfo);

    //junk mem for major frame
    if ( pauMajFTemp                != NULL ) free(pauMajFTemp);
    if ( ppauMajorFrame             != NULL ) free(ppauMajorFrame);

    //junk minor frame stuff
    if ( pauMinorFrame              != NULL ) free(pauMinorFrame);
    if ( pauIsMinorFrameCollected   != NULL ) free(pauIsMinorFrameCollected);

    //Extras that should go away
    if ( psuUniqCounterFile         != NULL ) fclose(psuUniqCounterFile);
    if ( pauGPSMeasIdx              != NULL ) free( pauGPSMeasIdx );

    return EXIT_SUCCESS;
}

void vPrintSubFrame (uint16_t * pauMajorFrame, int64_t llMinorFrameIdx)
{ 
    //    for

}

int iMeasurementInit(struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, uint64_t ullSampsPerMinorFrame, 
		     uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode )
{
    uint16_t uNAsymWRanges;
    uint16_t uNAsymFRanges;

    uint64_t ullBytesInAsymWRanges;
    uint64_t ullBytesInAsymFRanges;

    uint64_t ullAsymRangeIdx;

    uint16_t uWRangeIdx;
    uint16_t uFRangeIdx;

    size_t   szTempLen;
    size_t   szAbbrevLen;

    int iTmpIdx;

    if ( guTMLink == 1)
	{

	szTempLen = 0;
	while ( szTM1SerialMeasNames[iMeasIdx][szTempLen] != '\0' )
	    szTempLen++;
	strncpy(psuMeasInfo->szName, szTM1SerialMeasNames[iMeasIdx],	                   //Name of measurement, e.g., "Langmuir Probe Measurement 1 MSB"
		szTempLen );    

	szAbbrevLen = 0;
	while ( szTM1SerialMeasAbbrev[iMeasIdx][szAbbrevLen] != '\0' )
		szAbbrevLen++;
	strncpy(psuMeasInfo->szAbbrev, szTM1SerialMeasAbbrev[iMeasIdx],			   //Abbreviation for measurement, e.g., "LP01MSB"
		szAbbrevLen );

	szTempLen = 0;
	while ( szTM1User[iMeasIdx][szTempLen] != '\0' )
		szTempLen++;
        strncpy(psuMeasInfo->szUser, szTM1User[iMeasIdx],				   //Who is the user? E.g., Dartmouth, U Iowa
		szTempLen );

        psuMeasInfo->uWord               = uTM1Word[iMeasIdx] - 1;	                   //Beginning word in the frame, -1 for counting from zero
        psuMeasInfo->uWdInt              = uTM1WdInt[iMeasIdx];				   //Word interval
        psuMeasInfo->uMinorFrame         = uTM1Frame[iMeasIdx];				   //Which minor frame is it in?
        psuMeasInfo->uMinorFrInt         = uTM1FrInt[iMeasIdx];				   //How often does it show up?
        psuMeasInfo->ulSPS               = ulTM1SPS[iMeasIdx];
        			         
        psuMeasInfo->uSampsPerMinorFrame = ullSampsPerMinorFrame/psuMeasInfo->uWdInt;      //How many of these to expect per frame? Most are just one.

	psuMeasInfo->uLSBWord            = uTM1LSBWord[iMeasIdx] - 1;
	if ( ( bCombineTM1Meas && ( psuMeasInfo->uLSBWord != TM_SKIP_LSB - 1 ) ) && ( psuMeasInfo->uLSBWord != TM_NO_LSB -1 ) )
	    psuMeasInfo->szAbbrev[szAbbrevLen-4] = '\0';

        uNAsymWRanges                    = uTM1NAsymWRanges[iMeasIdx];
        uNAsymFRanges                    = uTM1NAsymFRanges[iMeasIdx];

	if ( bTStampMode )
	    psuMeasInfo->bTSCalcEnabled  = abTM1TSCalcEnabled[iMeasIdx];
	else
	    psuMeasInfo->bTSCalcEnabled  = 0;

	}
    else if ( ( guTMLink == 2 ) || ( guTMLink == 3 ) )
	{
	strncpy(psuMeasInfo->szName, szTM23SerialMeasNames[iMeasIdx],	                   //Name of measurement, e.g., "Langmuir Probe Measurement 1 MSB"
		strlen(szTM23SerialMeasNames[iMeasIdx]) );    
	strncpy(psuMeasInfo->szAbbrev, szTM23SerialMeasAbbrev[iMeasIdx],		   //Abbreviation for measurement, e.g., "LP01MSB"
		strlen(szTM23SerialMeasAbbrev[iMeasIdx]) );
        strncpy(psuMeasInfo->szUser, szTM23User[iMeasIdx],				   //Who is the user? E.g., Dartmouth, U Iowa
		strlen(szTM23User[iMeasIdx]) );
        psuMeasInfo->uWord               = uTM23Word[iMeasIdx] - 1;			   //Beginning word in the frame, -1 for counting from zero
        psuMeasInfo->uWdInt              = uTM23WdInt[iMeasIdx];			   //Word interval
        psuMeasInfo->uMinorFrame         = uTM23Frame[iMeasIdx];			   //Which minor frame is it in?
        psuMeasInfo->uMinorFrInt         = uTM23FrInt[iMeasIdx];			   //How often does it show up?
        psuMeasInfo->ulSPS               = ulTM23SPS[iMeasIdx];
        
	//        psuMeasInfo->uSample             = -1;
        psuMeasInfo->ullSampCount        = 0;
        			         
        psuMeasInfo->uSampsPerMinorFrame = ullSampsPerMinorFrame/psuMeasInfo->uWdInt;      //How many of these to expect per frame? Most are just one.

	psuMeasInfo->uLSBWord            = uTM23LSBWord[iMeasIdx] - 1;

        uNAsymWRanges                    = uTM23NAsymWRanges[iMeasIdx];
        uNAsymFRanges                    = uTM23NAsymFRanges[iMeasIdx];

	if ( bTStampMode )
	    psuMeasInfo->bTSCalcEnabled  = abTM23TSCalcEnabled[iMeasIdx];
	else
	    psuMeasInfo->bTSCalcEnabled  = 0;

	}

	//handle asymmetric word ranges
    if (uNAsymWRanges > 0)
	{
	psuMeasInfo->uNAsymWRanges       = uNAsymWRanges;
	psuMeasInfo->ppauAsymWRanges     = (uint16_t **) malloc(uNAsymWRanges * sizeof(uint16_t *));
	
	ullBytesInAsymWRanges            = uNAsymWRanges * sizeof(uint16_t *) * 2;
	
	psuMeasInfo->uSampsPerMinorFrame = 0; //This has to be recalculated

	//initialize memory word ranges
	//		psuMeasInfo->ppauAsymWRanges = (uint16_t **) malloc(ullBytesInAsymWRanges);
	psuMeasInfo->pauWtemp            = malloc(uNAsymWRanges * 2 * sizeof(uint16_t));
	for (ullAsymRangeIdx = 0; ullAsymRangeIdx < uNAsymWRanges; ullAsymRangeIdx++) 
	    {
	    psuMeasInfo->ppauAsymWRanges[ullAsymRangeIdx] = psuMeasInfo->pauWtemp + (ullAsymRangeIdx * 2);
	    }
	
	//Now assign the values
	for (uWRangeIdx = 0; uWRangeIdx < uNAsymWRanges; uWRangeIdx++)
	    {
	    if ( guTMLink == 1)
		{
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] = uTM1AsymWRanges[guAsymWRInd][0];
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] = uTM1AsymWRanges[guAsymWRInd][1];
		}
	    else if ( ( guTMLink == 2 ) || ( guTMLink == 3 ) )
		{
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] = uTM23AsymWRanges[guAsymWRInd][0];
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] = uTM23AsymWRanges[guAsymWRInd][1];
		}
	    psuMeasInfo->uSampsPerMinorFrame += (psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] - psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] + 1);
	    guAsymWRInd++;
	    }
	}
    else
	{
	psuMeasInfo->uNAsymWRanges   = 0;
	psuMeasInfo->ppauAsymWRanges = NULL;
	psuMeasInfo->pauWtemp        = NULL;
	}
    
    //handle asymmetric frame ranges
    if (uNAsymFRanges > 0)
	{
        psuMeasInfo->uNAsymFRanges   = uNAsymFRanges;
	psuMeasInfo->ppauAsymFRanges = (uint16_t **) malloc(uNAsymFRanges * sizeof(uint16_t *));
	
	ullBytesInAsymFRanges      = uNAsymFRanges * sizeof(uint16_t *) * 2;
	
	//initialize memory word ranges
	//		psuMeasInfo->ppauAsymFRanges = (uint16_t **) malloc(ullBytesInAsymFRanges);
	psuMeasInfo->pauFtemp = malloc(uNAsymFRanges * 2 * sizeof(uint16_t));
	for (ullAsymRangeIdx = 0; ullAsymRangeIdx < uNAsymFRanges; ullAsymRangeIdx++) 
	    {
	    psuMeasInfo->ppauAsymFRanges[ullAsymRangeIdx] = psuMeasInfo->pauFtemp + (ullAsymRangeIdx * 2);
	    }
	
	//Now assign the values
	for (uFRangeIdx = 0; uFRangeIdx < uNAsymFRanges; uFRangeIdx++)
	    {
	    if ( guTMLink == 1)
		{
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][0] = uTM1AsymFRanges[guAsymFRInd][0];
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][1] = uTM1AsymFRanges[guAsymFRInd][1];
		}
	    else if ( ( guTMLink == 2 ) || ( guTMLink == 3 ) )
		{
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][0] = uTM23AsymFRanges[guAsymFRInd][0];
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][1] = uTM23AsymFRanges[guAsymFRInd][1];
		}
	    guAsymFRInd++;
	    }
	}
    else
	{
	psuMeasInfo->uNAsymFRanges   = 0;
	psuMeasInfo->ppauAsymFRanges = NULL;
	psuMeasInfo->pauFtemp        = NULL;
	}
    
    //Now handle initialization of buffers
    psuMeasInfo->palSample           = (int32_t *) malloc(psuMeasInfo->uSampsPerMinorFrame * sizeof(int32_t));
    for ( iTmpIdx = 0; iTmpIdx < psuMeasInfo->uSampsPerMinorFrame; iTmpIdx++ )
	psuMeasInfo->palSample[iTmpIdx] = -1;


    if ( ( ( guTMLink > 1 ) || ( ( psuMeasInfo->uLSBWord  != TM_SKIP_LSB - 1 ) || !bCombineTM1Meas ) ) && !bDoCheckSFIDIncrement )
	{
	sprintf(psuMeasInfo->szOutFile,"%s--%s.out",szOutPrefix,psuMeasInfo->szAbbrev);
	psuMeasInfo->psuOutFile       = (FILE *) fopen(psuMeasInfo->szOutFile,"wb");
	}
    else
	{
	psuMeasInfo->szOutFile[0]     = '\0';
	psuMeasInfo->psuOutFile       = NULL;
	}

    if ( psuMeasInfo->bTSCalcEnabled )
	{
	sprintf(psuMeasInfo->szTStampFile,"%s--%s--tstamps.txt",szOutPrefix,psuMeasInfo->szAbbrev);
	psuMeasInfo->psuTStampFile    = (FILE *) fopen(psuMeasInfo->szTStampFile,"w");
	psuMeasInfo->pallWordOffsets  = (int64_t *) malloc(psuMeasInfo->uSampsPerMinorFrame * sizeof(int64_t));
	}
    else
	{
	psuMeasInfo->szTStampFile[0]  = '\0';
	psuMeasInfo->psuTStampFile    = NULL;
	psuMeasInfo->pallWordOffsets  = NULL;
	}
	psuMeasInfo->uOffsetBufCount  = 0;

    return 0;
}

int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo)
{
    
    //    if (psuMeasInfo->uNAsymWRanges > 0)
    //	{
    if ( psuMeasInfo->pauWtemp        != NULL ) free(psuMeasInfo->pauWtemp);
    if ( psuMeasInfo->ppauAsymWRanges != NULL ) free(psuMeasInfo->ppauAsymWRanges);
	//	}
    
    //free asymm frame ranges
	//    if (psuMeasInfo->uNAsymFRanges > 0)
	//	{
    if ( psuMeasInfo->pauFtemp        != NULL ) free(psuMeasInfo->pauFtemp);
    if ( psuMeasInfo->ppauAsymFRanges != NULL ) free(psuMeasInfo->ppauAsymFRanges);
        //      }

    if ( psuMeasInfo->psuOutFile      != NULL )	fclose(psuMeasInfo->psuOutFile);

    if ( psuMeasInfo->psuTStampFile   != NULL ) fclose(psuMeasInfo->psuTStampFile);

    if ( psuMeasInfo->pallWordOffsets != NULL )	free(psuMeasInfo->pallWordOffsets);

    free(psuMeasInfo);

    return (EXIT_SUCCESS);
}

void vPrintMeasurementInfo (struct suMeasurementInfo * psuMeasInfo)
{
    printf("Measurement Name       :   %s\n",psuMeasInfo->szName);
    printf("Abbrev                 :   %s\n",psuMeasInfo->szAbbrev);
    printf("User                   :   %s\n",psuMeasInfo->szUser);
    printf("Word                   :   %" PRIu16 "\n",psuMeasInfo->uWord);
    printf("Word Interval          :   %" PRIu16 "\n",psuMeasInfo->uWdInt);
    //    printf("N Words in Minor Frame :   %" PRIu16 "\n",psuMeasInfo->uSampsPerMinorFrame);
    printf("Minor Frame            :   %" PRIu16 "\n",psuMeasInfo->uMinorFrame);
    printf("Minor Frame Interval   :   %" PRIu16 "\n",psuMeasInfo->uMinorFrInt);
    printf("Samples per second     :   %" PRIu16 "\n",psuMeasInfo->ulSPS);
    printf("\n");
    printf("# Asym word ranges     :   %" PRIu16 "\n",psuMeasInfo->uNAsymWRanges);
    printf("# Asym frame ranges    :   %" PRIu16 "\n",psuMeasInfo->uNAsymFRanges);
    printf("\n");
    printf("TStamp calc enabled    :   %" PRIu8 "\n",psuMeasInfo->bTSCalcEnabled);
}

//write samples from this minor frame to the appropriate measurement files
uint16_t uParseMeasurementSamples(struct suMeasurementInfo * psuMeasInfo, int iMeasIdx, 
				  uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, uint64_t ullSampsPerMinorFrame, uint32_t * pulMinorFrameSampCount, 
				  uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
				  uint64_t * pullWordsWritten,
				  uint8_t bCombineTM1Meas, uint8_t bAssembleCounter, uint8_t bWriteSamplesToFile)
{
    int iArgIdx;
    int iWdIdx;
    int iFrIdx;

    uint16_t uAsymIdx;
    // If there are no asymmetric word or frame ranges, just write the sample at the specified word and any others within minor frame
    if ( ( psuMeasInfo->uNAsymWRanges == 0 ) && ( psuMeasInfo->uNAsymFRanges == 0 ) )
	{
	if ( (llMinorFrameIdx % psuMeasInfo->uMinorFrInt) == ( psuMeasInfo->uMinorFrame % psuMeasInfo->uMinorFrInt ) )
	    {
	    if ( ( guTMLink == 1 ) && ( bCombineTM1Meas ) )
		{
		int iLSBWdIdx;
		uint16_t uCombinedSample;
		iLSBWdIdx = psuMeasInfo->uLSBWord;
		if( iLSBWdIdx != ( TM_SKIP_LSB - 1 ) )
		    {
		    for (iWdIdx = psuMeasInfo->uWord; iWdIdx < ullSampsPerMinorFrame; iWdIdx += psuMeasInfo->uWdInt)
			{
			if ( iLSBWdIdx == TM_NO_LSB - 1 )
			    {
			    uCombinedSample = pauMinorFrame[iWdIdx];
			    if ( bAssembleCounter )
				{
				for ( iArgIdx = 0; iArgIdx < uNumMFCounters; iArgIdx++ )
				    {
				    if ( iMeasIdx == pauMFCIndices[iArgIdx] )
					{
					paullMajorFrameVals[iArgIdx] = uCombinedSample;
					break;
					}
				    }
				}
			    }
			else if ( iLSBWdIdx == TM_UPPER6_MSB_LOWER10_LSB - 1) //GPS and ACS are weird
			    {
			    uCombinedSample = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) << 10 ) | ( pauMinorFrame[iLSBWdIdx] );
			    }
			else
			    {
			    uCombinedSample = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) <<  6 ) | ( pauMinorFrame[iLSBWdIdx] >> 4 );
			    }
			
			if ( bWriteSamplesToFile )
			    {
			    (*pullWordsWritten) += fwrite(&uCombinedSample,2,1,psuMeasInfo->psuOutFile) * 2;
			    psuMeasInfo->ullSampCount++;
			    (*pulMinorFrameSampCount) += 2;
			    }
			if ( psuMeasInfo->bTSCalcEnabled ) 
			    {
			    psuMeasInfo->pallWordOffsets[psuMeasInfo->uOffsetBufCount] = iWdIdx;
			    psuMeasInfo->uOffsetBufCount++;
			    }
			iLSBWdIdx += psuMeasInfo->uWdInt;
			}
		    }
		}
	    else
		{
		for (iWdIdx = psuMeasInfo->uWord; iWdIdx < ullSampsPerMinorFrame; iWdIdx += psuMeasInfo->uWdInt)
		    {
		    if ( bWriteSamplesToFile )
			{
			(*pullWordsWritten) += fwrite(&pauMinorFrame[iWdIdx],2,1,psuMeasInfo->psuOutFile);
			psuMeasInfo->ullSampCount++;
			(*pulMinorFrameSampCount)++;
			}
		    if ( psuMeasInfo->bTSCalcEnabled ) 
			{
			psuMeasInfo->pallWordOffsets[psuMeasInfo->uOffsetBufCount] = iWdIdx;
			psuMeasInfo->uOffsetBufCount++;
			}
		    if ( bAssembleCounter )
			{
			for ( iArgIdx = 0; iArgIdx < uNumMFCounters; iArgIdx++ )
			    {
			    if ( iMeasIdx == pauMFCIndices[iArgIdx] )
				{
				paullMajorFrameVals[iArgIdx] = pauMinorFrame[iWdIdx];
				break;
				}
			    }
			}
		    }
		}
	    
	    }
	}
    // Otherwise, if we have asymmetric word ranges but no asymmetric frame ranges ...
    else if ( ( psuMeasInfo->uNAsymWRanges > 0 ) && ( psuMeasInfo->uNAsymFRanges == 0 ) )
	{
	int iLowerLim;
	int iUpperLim;
	for (uAsymIdx = 0; uAsymIdx < psuMeasInfo->uNAsymWRanges; uAsymIdx++)
	    {
	    iLowerLim = psuMeasInfo->ppauAsymWRanges[uAsymIdx][0]; //these ranges are inclusive
	    iUpperLim = psuMeasInfo->ppauAsymWRanges[uAsymIdx][1];
	    for ( iWdIdx = iLowerLim; iWdIdx <= iUpperLim; iWdIdx++)
		{
		if ( bWriteSamplesToFile )
		    {
		    (*pullWordsWritten) += fwrite(&pauMinorFrame[iWdIdx],2,1,psuMeasInfo->psuOutFile);
		    psuMeasInfo->ullSampCount++;
		    (*pulMinorFrameSampCount)++;
		    }
		if ( psuMeasInfo->bTSCalcEnabled ) 
		    {
		    psuMeasInfo->pallWordOffsets[psuMeasInfo->uOffsetBufCount] = iWdIdx;
		    psuMeasInfo->uOffsetBufCount++;
		    }
		}
	    }
	guAsymWRInd += psuMeasInfo->uNAsymWRanges;
	}
    // Otherwise, if we have asymmetric frame ranges but no asymmetric word ranges ...
    else if ( ( psuMeasInfo->uNAsymWRanges == 0 ) && ( psuMeasInfo->uNAsymFRanges > 0 ) )
	{
	int iLowerLim;
	int iUpperLim;
	for (uAsymIdx = 0; uAsymIdx < psuMeasInfo->uNAsymFRanges; uAsymIdx++)
	    {
	    iLowerLim = psuMeasInfo->ppauAsymFRanges[uAsymIdx][0]; //these ranges are inclusive
	    iUpperLim = psuMeasInfo->ppauAsymFRanges[uAsymIdx][1];
	    for ( iFrIdx = iLowerLim; iFrIdx <= iUpperLim; iFrIdx++)
		{
		if ( iFrIdx == llMinorFrameIdx )
		    {
		    if ( bWriteSamplesToFile )
			{
			(*pullWordsWritten) += fwrite(&pauMinorFrame[psuMeasInfo->uWord],2,1,psuMeasInfo->psuOutFile);
			psuMeasInfo->ullSampCount++;
			(*pulMinorFrameSampCount)++;
			}
		    if ( psuMeasInfo->bTSCalcEnabled ) 
			{
			psuMeasInfo->pallWordOffsets[psuMeasInfo->uOffsetBufCount] = iWdIdx;
			psuMeasInfo->uOffsetBufCount++;
			}
		    if ( bAssembleCounter )
			{
			for ( iArgIdx = 0; iArgIdx < uNumMFCounters; iArgIdx++ )
			    {
			    if ( iMeasIdx == pauMFCIndices[iArgIdx] )
				{
				paullMajorFrameVals[iArgIdx] = pauMinorFrame[psuMeasInfo->uWord];
				break;
				}
			    }
			}
		    }
		}
	    }
	guAsymFRInd += psuMeasInfo->uNAsymFRanges;
	}

    return 0;
}

/*For combining samples. Clearly unfinished*/
uint16_t combine_MSB_LSB_sample(uint16_t uMSBSample, uint16_t uLSBSample, 
				uint16_t uMSBShift, uint16_t uLSBShift, 
				uint16_t uJustification, uint8_t bMSBIsFirst)
{
    uint16_t uCombinedSample;

    uCombinedSample = ( uMSBSample << uMSBShift );

    return uCombinedSample;
}

uint64_t ullAssembleCounterVal(uint64_t * paullMajorFrameVals, int64_t llMinorFrameIdx, uint16_t uNumMFCounters, uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint64_t * pullMFCVal)
{
    uint64_t ullCounterVal;
    int      iMFCIdx;

    ullCounterVal = 0;
    for ( iMFCIdx = 0; iMFCIdx < uNumMFCounters; iMFCIdx++ )
	{
	    ullCounterVal = ullCounterVal | ( paullMajorFrameVals[iMFCIdx] << ullSampBitLength*iMFCIdx ); //	    ullCounterVal = ullCounterVal | ( paullMajorFrameVals[iMFCIdx] << ullSampBitLength*(uNumMFCounters-iMFCIdx-1) );
	//printf("Major frame counter %i: %" PRIu64 "\n",iMFCIdx, paullMajorFrameVals[iMFCIdx]);
	}
    if ( pullMFCVal != NULL ) 
	(*pullMFCVal) = ullCounterVal;

	ullCounterVal = (ullCounterVal << uMinorFrameBitShift) | (llMinorFrameIdx - 1); //last, tack on minor frame

    return ullCounterVal;
}

uint64_t ullGetFirstMFCVal(FILE * psuInFile, size_t szInFileSize, 
			   struct suMeasurementInfo ** ppsuMeasInfo, int16_t iNMeasurements,
			   uint16_t * pauMinorFrame, uint64_t ullSampsPerMinorFrame, 
			   uint64_t * pullCounterVal, uint64_t * paullMajorFrameVals, uint16_t * pauMFCIndices, uint16_t uNumMFCounters,
			   uint64_t ullSampBitLength, uint16_t uMinorFrameBitShift, uint8_t bCombineTM1Meas,uint64_t * pllCurrentGPSWord)
{
    int iMeasIdx;
    int iArgIdx;

    uint64_t             ullWordsWritten;
    uint32_t             ulMinorFrameSampCount;
    int64_t              llMinorFrameIdx;
    uint64_t             ullInFilePos;
    uint64_t             ullTemp;
    uint16_t             uMFCValCount;
    uint8_t              bAllMFCValsCollected;

    uint64_t             ullFirstMFCVal;

    ullWordsWritten      = 0;
    ulMinorFrameSampCount= 0;
    llMinorFrameIdx      = 0;
    ullInFilePos         = 0;
    iMeasIdx             = 0;
    ullTemp              = 0;
    uMFCValCount         = 0;
    bAllMFCValsCollected = 0;
	
    while ( !bAllMFCValsCollected && ( ( ullInFilePos = ftell(psuInFile) ) < szInFileSize ) )
	{
	guAsymWRInd = 0;
	guAsymFRInd = 0;

	ullTemp = fread(pauMinorFrame, 2, ullSampsPerMinorFrame, psuInFile);
	if (ullTemp != ullSampsPerMinorFrame)
	    {
	    fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullTemp*2,ullSampsPerMinorFrame*2);
	    return -1;
	    }
	
	llMinorFrameIdx = llGetMinorFrameIdx(pauMinorFrame);  //The TM list counts from 1, not zero
	
	for (iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	    {
	    uParseMeasurementSamples(ppsuMeasInfo[iMeasIdx],iMeasIdx,
				     pauMinorFrame,llMinorFrameIdx,ullSampsPerMinorFrame,&ulMinorFrameSampCount,
				     paullMajorFrameVals,pauMFCIndices,uNumMFCounters,&ullWordsWritten,
				     bCombineTM1Meas,1,0);
	    }
	
	for ( iArgIdx = 0; iArgIdx < uNumMFCounters; iArgIdx++ )
	    {			         
	    if ( paullMajorFrameVals[iArgIdx] != -1 )
		uMFCValCount++;
	    }			         
	
	if ( uMFCValCount == uNumMFCounters )
	    bAllMFCValsCollected = 1;
	}

    if ( bAllMFCValsCollected == 1 )
	{
	//Now we pick up the first MFC Value and go back to the beginning of the file
	(*pullCounterVal) = ullAssembleCounterVal(paullMajorFrameVals,llMinorFrameIdx,uNumMFCounters,ullSampBitLength, uMinorFrameBitShift, &ullFirstMFCVal);
	rewind(psuInFile);
	ullInFilePos = 0;
	}
    else
	{
	return -1;
	}
    return ullFirstMFCVal;
}

void vCheckForNewGPSWord(struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, 
			 uint16_t * pauMinorFrame,int64_t * pllCurrentGPSWord, uint8_t bCombineTM1Meas)
{
    //    int iMeasIdx;
    int iWdIdx;

    uint16_t uNonZeroGPSWordCount;
    int64_t  llTestGPSWord;

    //    iMeasIdx             = pauGPSMeasIdx[0];
    uNonZeroGPSWordCount = 0;
    llTestGPSWord        = -1;
    

    if ( (llMinorFrameIdx % psuMeasInfo->uMinorFrInt) == ( psuMeasInfo->uMinorFrame % psuMeasInfo->uMinorFrInt ) )
	{
	if ( ( guTMLink == 1 ) && ( bCombineTM1Meas ) )
	    {
	    int iLSBWdIdx;
	    iWdIdx = psuMeasInfo->uWord;
	    iLSBWdIdx = psuMeasInfo->uLSBWord;

	    if( iLSBWdIdx != ( TM_SKIP_LSB - 1 ) )
		{
		//		    for ( iWdIdx = psuMeasInfo->uWord; iWdIdx < ullSampsPerMinorFrame; iWdIdx += psuMeasInfo->uWdInt )
		//			{
		if ( iLSBWdIdx == TM_NO_LSB - 1 )
		    {
		    llTestGPSWord = pauMinorFrame[iWdIdx];
		    }
		else if ( iLSBWdIdx == TM_UPPER6_MSB_LOWER10_LSB - 1) //GPS and ACS are weird
		    {
		    llTestGPSWord = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) << 10 ) | ( pauMinorFrame[iLSBWdIdx] );
		    }
		else
		    {
		    llTestGPSWord = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) <<  6 ) | ( pauMinorFrame[iLSBWdIdx] >> 4 );
		    }
		//			iLSBWdIdx += psuMeasInfo->uWdInt;
		//			}
		}
	    }
	else
	    {
	    llTestGPSWord = pauMinorFrame[psuMeasInfo->uWord];
	    }
	}

    if ( ( llTestGPSWord != -1 ) && ( llTestGPSWord != (*pllCurrentGPSWord) ) )
	{
	(*pllCurrentGPSWord) = llTestGPSWord;
	}
}

int iWriteMeasurementTStamps(struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx,
			     int64_t llWordOffset_MajorFrame, int64_t llWordOffset_MinorFrame, int64_t llWordOffset_GPS)
{
    int iArgIdx;
    int64_t llBaseOffset;
    int64_t llWordOffset_Measurement;
    double  dTimeOffset_Measurement;

    llBaseOffset = llWordOffset_MajorFrame + llWordOffset_MinorFrame;

    for (iArgIdx = 0; iArgIdx < psuMeasInfo->uOffsetBufCount; iArgIdx++ )
	{
	    llWordOffset_Measurement = (llBaseOffset + psuMeasInfo->pallWordOffsets[iArgIdx]) - llWordOffset_GPS;
	    dTimeOffset_Measurement = llWordOffset_Measurement * gdWordPeriod;
	    fprintf(psuMeasInfo->psuTStampFile,"%.6f\n",dTimeOffset_Measurement);
	}

    //reset the count of the offset buffer
    psuMeasInfo->uOffsetBufCount = 0;

    return 0;

    /* if ( (llMinorFrameIdx % psuMeasInfo->uMinorFrInt) == ( psuMeasInfo->uMinorFrame % psuMeasInfo->uMinorFrInt ) ) */
    /* 	{ */
    /* 	    for (iWdIdx = psuMeasInfo->uWord; iWdIdx < ullSampsPerMinorFrame; iWdIdx += psuMeasInfo->uWdInt) */
    /* 		{ */
    /* 		    dTimeOffset_Measurement = (llWordOffset_MajorFrame + llWordOffset_MinorFrame + iWdIdx) * gdWordPeriod; */
    /* 		} */
    /* 	} */
    //    dTimeOffset_Measurement = llWordOffset_Measurement * gdWordPeriod;

}

int64_t llGetMinorFrameIdx(uint16_t * pauMinorFrame){
    return (pauMinorFrame[guSFIDIdx-1] + 1) & 0b0000111111;  //The TM list counts from 1, not zero
}

int iBadSFIDIncrement(int64_t llMinorFrameIdx, int64_t llOldMinorFrameIdx)
{
    return ( ( ( llMinorFrameIdx - llOldMinorFrameIdx ) != 1 ) && ( ( llMinorFrameIdx - llOldMinorFrameIdx ) != 1 - gllMinorFramesPerMajorFrame ) );
}

void vUsage(void)
{
    printf("\n");
    printf("parse_CAPER_samples\n");
    printf("Convert a filed outputted by bust_nBit_into_16bit_file into separate measurement files!\n");
    printf("\n");
    printf("Usage: parse_CAPER_samples [flags]                                         \n");
    printf("                                                                           \n");
    printf("   <filename>   Input/output file names                                    \n");
    printf("                                                                           \n");
    printf("   INPUT FILE PARAMETERS                                                   \n");
    printf("   -n           Number of samples per minor frame                  [%i]    \n",DEF_SAMPSPERMINORFRAME);
    printf("   -L           TM link number (can be 1-4)                        [%i]    \n",DEF_TM_LINK);
    printf("                                                                           \n");
    printf("                                                                           \n");
    printf("   OPTIONAL PARAMETERS                                                     \n");
    printf("   -P           Output file prefix                                 [%s]    \n",DEF_OUTPREFIX);
    printf("   -C           Combine MSB/LSB channels on the fly(TM1 Only!)     [%i]    \n",DEF_COMBINE_TM1);
    printf("   -c           Check integrity of data by following SFID,         [%i]    \n",DEF_DO_CHECK_SFID_INCREMENT);
    printf("                    (no parsed output is produced)                         \n");
    printf("   -A           Assemble unique counter based on major/minor       [%i]    \n",DEF_ASSEMBLE_COUNTER);
    printf("                    frames, and output to file                             \n");
    printf("   -T           Produce timestamps for each measurement based      [%i]    \n",DEF_CALC_TSTAMPS);
    printf("                    on GPS pulse data                                      \n");
    printf("   -v           Verbose                                            [%i]    \n",DEF_VERBOSE);
    printf("                                                                           \n");
    printf("   -h           Help (this menu)                                           \n");
}
