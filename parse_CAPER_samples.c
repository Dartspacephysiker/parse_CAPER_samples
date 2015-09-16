/*
2015/09/07 Now we want to grab all samples out of a file to make 'em work
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include "TM1_majorframe.h"
#include "TM23_majorframe.h"

#define DEF_N_SAMPBITS                   16 //in bits
#define MAX_N_MINORFRAMES               256
#define DEF_SAMPSPERMINORFRAME          120
#define DEF_MINOR_PER_MAJOR              32

#define DEF_TM_LINK                       1

#define DEF_OUTPREFIX  "parsed_TM1-samples"
#define DEF_COMBINE_TM1                   0 //combine TM1 channels on the fly
#define DEF_VERBOSE                       0 //please tell me

#define DEF_STR_SIZE                   1024

//struct declarations
struct suMeasurementInfo
{
    char        szName[DEF_STR_SIZE];          //Name of the measurement, e.g., "Langmuir Probe Measurement 1 MSB"
    char        szAbbrev[DEF_STR_SIZE];        //Abbreviation for measurement, e.g., "LP01MSB"
    char        szUser[DEF_STR_SIZE];	      //Who is the user? E.g., Dartmouth, U Iowa
    uint16_t    uWord;		      //Beginning word in the frame
    uint16_t    uWdInt;		      //Word interval
    uint16_t    uSampsPerMinorFrame;     //How many of these to expect per frame? Most are just one.
    uint16_t    uMinorFrame;          //Which minor frame is it in?
    uint16_t    uMinorFrInt;          //How often does it show up?
    uint32_t    ulSPS;

    uint16_t    uSample;
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

    char        szOutFileName[DEF_STR_SIZE];
    FILE *      psuOutFile;
};

//function declarations
int iMeasurementInit(struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, uint64_t ullSampsPerMinorFrame, uint16_t uTMLink, uint8_t bCombineTM1Meas );
int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo);

void vPrintMeasurementInfo (struct suMeasurementInfo * psuMeasInfo);
void vPrintSubFrame (uint16_t * pauMajorFrame, int16_t iMinorFrameIdx);

uint16_t combine_MSB_LSB_sample(uint16_t uMSBSample, uint16_t uLSBSample, uint16_t uMSBShift, uint16_t uLSBShift, uint16_t uJustification, uint8_t bMSBIsFirst);
void vUsage(void);

//global var declarations
uint16_t uGlobAsymWRInd;
uint16_t uGlobAsymFRInd;

//struct suMeasurementInfo * psuMeasurementInit(int16_t iMeasIdx);

int main( int argc, char * argv[] )
    {
    //************
    //Declare vars

    char         szInFile[DEF_STR_SIZE];		       //input/output file stuff
    char         szOutFile[DEF_STR_SIZE];

    FILE       * psuInFile;
    FILE       * psuOutFile;
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

    uint16_t     uSFIDIdx;
    uint16_t     uAsymIdx;

    uint64_t     ullMinorFrameIdx;
    int64_t      llMinorFramesPerMajorFrame;
    uint64_t     ullBytesPerMajorFrame;
    int64_t      llMajorFrameCount;
    uint16_t  *  pauIsMinorFrameCollected;
    uint16_t **  ppauMajorFrame;
    uint16_t  *  temp;

    uint64_t     ullInFilePos;
    uint64_t     ullWordsRead;
    uint64_t     ullWordsWritten;
    uint64_t     ullNSampsRead;
    uint64_t     ullTotWordsRead;
    uint64_t     ullTotWordsWritten;

    uint32_t     ulMinorFrameSampCount;
    uint64_t     ullMinorFrameCount;

    uint16_t     uTMLink;

    int          iArgIdx;

    uint8_t      bCombineTM1Meas;
    uint8_t      bVerbose;

    //*******************
    //Initialize global vars
    uGlobAsymWRInd = 0;
    uGlobAsymFRInd = 0;

    //Initialize vars
    ulMinorFrameSampCount  = 0;
    ullMinorFrameCount     = 0;
    llMajorFrameCount      = 0;

    iArgIdx                = 0;

    bCombineTM1Meas        = DEF_COMBINE_TM1;
    bVerbose               = DEF_VERBOSE;

    if (argc < 2)
	{
	vUsage();
	return EXIT_SUCCESS;
	}

    szInFile[0]  = '\0';
    strcpy(szOutFile,"");		       // Default is stdout

    psuInFile = (FILE *) !NULL;
    psuOutFile = (FILE *) !NULL;
    strncpy(szOutPrefix,DEF_OUTPREFIX,DEF_STR_SIZE);

    ullSampBitLength         = DEF_N_SAMPBITS;
    ullSampsPerMinorFrame   = DEF_SAMPSPERMINORFRAME;
    ullBytesPerMinorFrame  = ullSampsPerMinorFrame * sizeof(uint16_t);

    ppsuMeasInfo = (struct suMeasurementInfo **) !NULL;
    iMeasIdx                  = 0;

    uSFIDIdx                = 0;

    ullMinorFrameIdx = 0;
    llMinorFramesPerMajorFrame = DEF_MINOR_PER_MAJOR;

    ullInFilePos            = 0;
    ullWordsRead            = 0;
    ullWordsWritten         = 0;
    ullNSampsRead           = 0;
    ullTotWordsRead         = 0;
    ullTotWordsWritten      = 0;

    for (iArgIdx=1; iArgIdx<argc; iArgIdx++)   //start with 1 to skip input file
	{

	switch (argv[iArgIdx][0])
	    {
	    int iTmp;
	    case '-' :
		switch (argv[iArgIdx][1])
		    {
		    case 's' :                   /* Sample size */
			iArgIdx++;
			if(iArgIdx >= argc)
			    {
			    vUsage();
			    return EXIT_FAILURE;
			    }
			sscanf(argv[iArgIdx],"%" PRIu64 ,&ullSampBitLength);
			break;

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
			sscanf(argv[iArgIdx],"%" PRIu16 ,&uTMLink);
			break;

		    case 'P' :                  /* Prefix for output files */
			iArgIdx++;
			strncpy(szOutPrefix, argv[iArgIdx],DEF_STR_SIZE);
			break;

		    case 'C' :                  /* Combine TM1 MSB/LSB measurements */
			bCombineTM1Meas = !bCombineTM1Meas;
			break;

		    case 'v' :                  /* Verbosities */
			bVerbose = 1;
			break;

		    default :
			break;
		    } /* end flag switch */
		break;

	    default :
		if (szInFile[0] == '\0') strcpy(szInFile, argv[iArgIdx]);
		else                     strcpy(szOutFile,argv[iArgIdx]);
		break;

	    } /* end command line arg switch */
	} /* end for all arguments */

    if ( ullSampBitLength < 1 || ullSampBitLength > 16 )
	{
	fprintf(stderr,"Invalid sample size provided! Exiting...\n");
	return EXIT_FAILURE;
	}
    printf(" \n");

    if (uTMLink == 1 )
	{
	iNMeasurements      = N_TM1_MEASUREMENTS;
        uSFIDIdx            = TM1_SFID_IDX;
	
	if ( bCombineTM1Meas ) printf("Combining MSB/LSB measurements on TM1!\n");

	}
    else
	{
	iNMeasurements      = N_TM23_MEASUREMENTS;
        uSFIDIdx            = TM23_SFID_IDX;
	ullSampsPerMinorFrame   = 400;
	ullBytesPerMinorFrame  = ullSampsPerMinorFrame * sizeof(uint16_t);
	llMinorFramesPerMajorFrame = 4;

	}

    //init measurements
    //    psuMeasInfo = (struct suMeasurementInfo *) malloc(sizeof(struct suMeasurementInfo) * iNMeasurements);
    ppsuMeasInfo = malloc(iNMeasurements * sizeof(struct suMeasurementInfo *));
    for ( iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	{
        int err;
	//	    psuMeasInfo[iMeasIdx] = psuMeasurementInit(iMeasIdx);
	ppsuMeasInfo[iMeasIdx] = (struct suMeasurementInfo *) malloc(sizeof(struct suMeasurementInfo));
	//	if (psuMeasInfo == (struct suMeasurementInfo *) !NULL)
	if (ppsuMeasInfo[iMeasIdx] == (struct suMeasurementInfo *) !NULL)
	    {
	    printf("Couldn't initialize measurement %" PRIi16 "!\n",iMeasIdx);
	    return -1;
	    }

	err = iMeasurementInit(ppsuMeasInfo[iMeasIdx],iMeasIdx,szOutPrefix,ullSampsPerMinorFrame,uTMLink, bCombineTM1Meas);
	if (bVerbose) vPrintMeasurementInfo(ppsuMeasInfo[iMeasIdx]);
	}


    //initialize major frame, binary array for keeping track of
    ullBytesPerMinorFrame = ullSampsPerMinorFrame * sizeof(uint16_t);
    ullBytesPerMajorFrame = ullBytesPerMinorFrame * llMinorFramesPerMajorFrame;

    pauMinorFrame = malloc(ullBytesPerMinorFrame);

    pauIsMinorFrameCollected = (uint16_t *) calloc(llMinorFramesPerMajorFrame,2);
    ppauMajorFrame = (uint16_t **) malloc(llMinorFramesPerMajorFrame * sizeof(uint16_t *));
    //    temp = malloc(llMinorFramesPerMajorFrame * ullSampsPerMinorFrame * sizeof(uint16_t));
    temp = malloc(ullBytesPerMajorFrame);

    for (ullMinorFrameIdx = 0; ullMinorFrameIdx < llMinorFramesPerMajorFrame; ullMinorFrameIdx++) 
	{
	ppauMajorFrame[ullMinorFrameIdx] = temp + (ullMinorFrameIdx * ullSampsPerMinorFrame);
	}
    printf("Major Frame Size   :\t%" PRIu64 " bytes\n",ullBytesPerMajorFrame);

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
    printf("Input file: %s\n", szInFile);

    //Get input file stats
    ullInFilePos = fseek(psuInFile, 0, SEEK_SET); //go to zero, positively
    stat(szInFile, &suInFileStat);

    // If output file specified then open it
    if (strlen(szOutFile) != 0)
	{
	psuOutFile = fopen(szOutFile,"wb");
	if (psuOutFile == NULL)
	    {
	    fprintf(stderr, "Error opening output file\n");
	    return EXIT_FAILURE;
	    }

	printf("Output file: %s\n", szOutFile);
	}
    else  // No output file name so use stdout
	{
	psuOutFile = stdout;
	printf("Outputting to stdout\n");
	}

    //To the beginning!
    printf("\nConverting %" PRIu64 "-bit to 16-bit samples...\n", ullSampBitLength);

    //Loop over whole file
    do
    	{
    	ullWordsWritten = 0;
	ulMinorFrameSampCount = 0;
	//reset global vars
	uGlobAsymWRInd = 0;
	uGlobAsymFRInd = 0;

    	ullWordsRead = fread(pauMinorFrame, 2, ullSampsPerMinorFrame, psuInFile);     	//Get samples from infile
    	if (ullWordsRead != ullSampsPerMinorFrame)
    	    {
    	    fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullWordsRead*2,ullBytesPerMinorFrame);
    	    break;
    	    }

	//set all samples to -1, reset counters
	//CURRENTLY NOT USED, MAYBE WITH GOOD REASON
	/* for (iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++) */
	/*     { */
	/*     ppsuMeasInfo[iMeasIdx]->uSample = -1; */
	/*     } */
	
	//Determine which minor frame this is
	ullMinorFrameIdx = (pauMinorFrame[uSFIDIdx-1] + 1) & 0b0000011111;  //The TM list counts from 1, not zero
	if (bVerbose) printf("Minor frame: %" PRIX64 "\n",ullMinorFrameIdx);

	//prepare all the samples, man
	//	while(ulMinorFrameSampCount < ullSampsPerMinorFrame)
	//	    {
	
	//check the sync words
	
	//write this minor frame to the appropriate measurement file
	int iCurrAsymWRange;
	int iCurrAsymFRange;
	for (iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	    {
	    int iWdIdx;

	    // If there are no asymmetric word or frame ranges, just write the sample at the specified word and any others within minor frame
	    if ( ( ppsuMeasInfo[iMeasIdx]->uNAsymWRanges == 0 ) && ( ppsuMeasInfo[iMeasIdx]->uNAsymFRanges == 0 ) )
		{
		    
		if ( (ullMinorFrameIdx % ppsuMeasInfo[iMeasIdx]->uMinorFrInt) == ( ppsuMeasInfo[iMeasIdx]->uMinorFrame % ppsuMeasInfo[iMeasIdx]->uMinorFrInt ) )
		    {
		    if ( ( uTMLink == 1 ) && ( bCombineTM1Meas ) )
			{
			int iLSBWdIdx;
			uint16_t uCombinedSample;
			iLSBWdIdx = ppsuMeasInfo[iMeasIdx]->uLSBWord;
			if( iLSBWdIdx != ( TM_SKIP_LSB - 1 ) )
			    {
			    for (iWdIdx = ppsuMeasInfo[iMeasIdx]->uWord; iWdIdx < ullSampsPerMinorFrame; iWdIdx += ppsuMeasInfo[iMeasIdx]->uWdInt)
				{
				if ( iLSBWdIdx == TM_NO_LSB - 1 )
				    {
				    uCombinedSample = pauMinorFrame[iWdIdx];
				    }
				else if ( iLSBWdIdx == TM_UPPER6_MSB_LOWER10_LSB - 1) //GPS and ACS are weird
				    {
				    uCombinedSample = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) << 10 ) | ( pauMinorFrame[iLSBWdIdx] );
				    }
				else
				    {
				    uCombinedSample = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) <<  6 ) | ( pauMinorFrame[iLSBWdIdx] >> 4 );
				    }

				ullWordsWritten += fwrite(&uCombinedSample,2,1,ppsuMeasInfo[iMeasIdx]->psuOutFile) * 2;
				ulMinorFrameSampCount += 2;
				iLSBWdIdx += ppsuMeasInfo[iMeasIdx]->uWdInt;
				}
			    }
			}
		    else
			{
			for (iWdIdx = ppsuMeasInfo[iMeasIdx]->uWord; iWdIdx < ullSampsPerMinorFrame; iWdIdx += ppsuMeasInfo[iMeasIdx]->uWdInt)
			    {
			    ullWordsWritten += fwrite(&pauMinorFrame[iWdIdx],2,1,ppsuMeasInfo[iMeasIdx]->psuOutFile);
			    ppsuMeasInfo[iMeasIdx]->ullSampCount++;
			    ulMinorFrameSampCount++;
			    }
			}

		    }
		}
	    // Otherwise, if we have asymmetric word ranges but no asymmetric frame ranges ...
	    else if ( ( ppsuMeasInfo[iMeasIdx]->uNAsymWRanges > 0 ) && ( ppsuMeasInfo[iMeasIdx]->uNAsymFRanges == 0 ) )
		{
		int iLowerLim;
		int iUpperLim;
		for (uAsymIdx = 0; uAsymIdx < ppsuMeasInfo[iMeasIdx]->uNAsymWRanges; uAsymIdx++)
		    {
		    /* iLowerLim = ppsuMeasInfo[iMeasIdx]->ppauAsymWRanges[uGlobAsymWRInd+uAsymIdx][0]; //these ranges are inclusive */
		    /* iUpperLim = ppsuMeasInfo[iMeasIdx]->ppauAsymWRanges[uGlobAsymWRInd+uAsymIdx][1]; */
		    iLowerLim = ppsuMeasInfo[iMeasIdx]->ppauAsymWRanges[uAsymIdx][0]; //these ranges are inclusive
		    iUpperLim = ppsuMeasInfo[iMeasIdx]->ppauAsymWRanges[uAsymIdx][1];
		    for ( iWdIdx = iLowerLim; iWdIdx <= iUpperLim; iWdIdx++)
			{
			ullWordsWritten += fwrite(&pauMinorFrame[iWdIdx],2,1,ppsuMeasInfo[iMeasIdx]->psuOutFile);
			ppsuMeasInfo[iMeasIdx]->ullSampCount++;
			ulMinorFrameSampCount++;
			}
		    }
		uGlobAsymWRInd += ppsuMeasInfo[iMeasIdx]->uNAsymWRanges;
		}
	    // Otherwise, if we have asymmetric frame ranges but no asymmetric word ranges ...
	    else if ( ( ppsuMeasInfo[iMeasIdx]->uNAsymWRanges == 0 ) && ( ppsuMeasInfo[iMeasIdx]->uNAsymFRanges > 0 ) )
		{
		int iLowerLim;
		int iUpperLim;
		for (uAsymIdx = 0; uAsymIdx < ppsuMeasInfo[iMeasIdx]->uNAsymFRanges; uAsymIdx++)
		    {
		    /* iLowerLim = ppsuMeasInfo[iMeasIdx]->ppauAsymFRanges[uGlobAsymFRInd+uAsymIdx][0]; //these ranges are inclusive */
		    /* iUpperLim = ppsuMeasInfo[iMeasIdx]->ppauAsymFRanges[uGlobAsymFRInd+uAsymIdx][1]; */
		    iLowerLim = ppsuMeasInfo[iMeasIdx]->ppauAsymFRanges[uAsymIdx][0]; //these ranges are inclusive
		    iUpperLim = ppsuMeasInfo[iMeasIdx]->ppauAsymFRanges[uAsymIdx][1];
		    for ( iWdIdx = iLowerLim; iWdIdx <= iUpperLim; iWdIdx++)
			{
			if ( iWdIdx == ullMinorFrameIdx )
			    {
			    ullWordsWritten += fwrite(&pauMinorFrame[ppsuMeasInfo[iMeasIdx]->uWord],2,1,ppsuMeasInfo[iMeasIdx]->psuOutFile);
			    ppsuMeasInfo[iMeasIdx]->ullSampCount++;
			    ulMinorFrameSampCount++;
			    }
			}
		    }
		uGlobAsymFRInd += ppsuMeasInfo[iMeasIdx]->uNAsymFRanges;
		}
	    }


	/* int i = 0; */
	/* for (i = 0; i < ullSampsPerMinorFrame; i++) */
	/*     { */
	/* 	printf("Samp #%i:   0x%" PRIx16 "\n",i,pauMinorFrame[i]); */
	/*     } */

	//did we get the whole major frame?
	ullMinorFrameCount++;
	if (ullMinorFrameCount % llMinorFramesPerMajorFrame == 0) llMajorFrameCount++;
	if (bVerbose) printf("Major frame #%" PRIu64 "\n", llMajorFrameCount);


	for (ullMinorFrameIdx = 0; ullMinorFrameIdx < llMinorFramesPerMajorFrame; ullMinorFrameIdx++)
	    {
	    if (bVerbose) 
		{
		if (pauIsMinorFrameCollected[ullMinorFrameIdx] == 0)
		    printf("Did not collect subframe %" PRIi64 "!",ullMinorFrameIdx);
		}
	    }

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
    printf("Minor frame count:  %" PRIu64 "\n", ullMinorFrameCount);
    printf("Major frame count:  %" PRIu64 "\n", llMajorFrameCount);
    printf("\n");
    //    if (bVerbose) 
    printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Total Bytes) \n", ullTotWordsWritten*2, (suInFileStat.st_size));
    printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Total Bytes) \n", ullTotWordsWritten*2, (suInFileStat.st_size));
    printf("Output file size is %.2f%% of input file\n", (float)((float)( ullTotWordsWritten*2 )/(float)(suInFileStat.st_size))*100);

    //close files
    fclose(psuInFile);
    fclose(psuOutFile);

    //release the mem!
    for ( iMeasIdx = 0; iMeasIdx < iNMeasurements; iMeasIdx++)
	{
	if ( ppsuMeasInfo[iMeasIdx] != NULL )
	    {
	    iMeasurementFree(ppsuMeasInfo[iMeasIdx]);
	    }
	}
    free(ppsuMeasInfo);

    //junk mem for major frame
    free(temp);
    free(ppauMajorFrame);

    //junk minor frame stuff
    free(pauMinorFrame);
    free(pauIsMinorFrameCollected);

    return EXIT_SUCCESS;
    }

void vPrintSubFrame (uint16_t * pauMajorFrame, int16_t ullMinorFrameIdx)
{ 
    //    for

}

 int iMeasurementInit(struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, uint64_t ullSampsPerMinorFrame, uint16_t uTMLink, uint8_t bCombineTM1Meas )
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

    if ( uTMLink == 1)
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

        psuMeasInfo->uWord            = uTM1Word[iMeasIdx] - 1;		                   //Beginning word in the frame, -1 for counting from zero
        psuMeasInfo->uWdInt           = uTM1WdInt[iMeasIdx];				   //Word interval
        psuMeasInfo->uSampsPerMinorFrame = ullSampsPerMinorFrame/psuMeasInfo->uWdInt;      //How many of these to expect per frame? Most are just one.
        psuMeasInfo->uMinorFrame      = uTM1Frame[iMeasIdx];				   //Which minor frame is it in?
        psuMeasInfo->uMinorFrInt      = uTM1FrInt[iMeasIdx];				   //How often does it show up?
        psuMeasInfo->ulSPS            = ulTM1SPS[iMeasIdx];
        
        psuMeasInfo->uSample          = -1;
        psuMeasInfo->ullSampCount     = 0;
        
	psuMeasInfo->uLSBWord         = uTM1LSBWord[iMeasIdx] - 1;
	if ( ( bCombineTM1Meas && ( psuMeasInfo->uLSBWord != TM_SKIP_LSB - 1 ) ) && ( psuMeasInfo->uLSBWord != TM_NO_LSB -1 ) )
	    psuMeasInfo->szAbbrev[szAbbrevLen-4] = '\0';


        uNAsymWRanges = uTM1NAsymWRanges[iMeasIdx];
        uNAsymFRanges = uTM1NAsymFRanges[iMeasIdx];
	}
    else if ( ( uTMLink == 2 ) || ( uTMLink == 3 ) )
	{
	strncpy(psuMeasInfo->szName, szTM23SerialMeasNames[iMeasIdx],	                   //Name of measurement, e.g., "Langmuir Probe Measurement 1 MSB"
		strlen(szTM23SerialMeasNames[iMeasIdx]) );    
	strncpy(psuMeasInfo->szAbbrev, szTM23SerialMeasAbbrev[iMeasIdx],		   //Abbreviation for measurement, e.g., "LP01MSB"
		strlen(szTM23SerialMeasAbbrev[iMeasIdx]) );
        strncpy(psuMeasInfo->szUser, szTM23User[iMeasIdx],				   //Who is the user? E.g., Dartmouth, U Iowa
		strlen(szTM23User[iMeasIdx]) );
        psuMeasInfo->uWord            = uTM23Word[iMeasIdx] - 1;			   //Beginning word in the frame, -1 for counting from zero
        psuMeasInfo->uWdInt           = uTM23WdInt[iMeasIdx];				   //Word interval
        psuMeasInfo->uSampsPerMinorFrame = ullSampsPerMinorFrame/psuMeasInfo->uWdInt;      //How many of these to expect per frame? Most are just one.
        psuMeasInfo->uMinorFrame      = uTM23Frame[iMeasIdx];				   //Which minor frame is it in?
        psuMeasInfo->uMinorFrInt      = uTM23FrInt[iMeasIdx];				   //How often does it show up?
        psuMeasInfo->ulSPS            = ulTM23SPS[iMeasIdx];
        
        psuMeasInfo->uSample          = -1;
        psuMeasInfo->ullSampCount     = 0;
        
	psuMeasInfo->uLSBWord         = uTM23LSBWord[iMeasIdx] - 1;

        uNAsymWRanges = uTM23NAsymWRanges[iMeasIdx];
        uNAsymFRanges = uTM23NAsymFRanges[iMeasIdx];
	}

	//handle asymmetric word ranges
    if (uNAsymWRanges > 0)
	{
	psuMeasInfo->uNAsymWRanges   = uNAsymWRanges;
	psuMeasInfo->ppauAsymWRanges = (uint16_t **) malloc(uNAsymWRanges * sizeof(uint16_t *));
	
	ullBytesInAsymWRanges      = uNAsymWRanges * sizeof(uint16_t *) * 2;
	
	//initialize memory word ranges
	//		psuMeasInfo->ppauAsymWRanges = (uint16_t **) malloc(ullBytesInAsymWRanges);
	psuMeasInfo->pauWtemp = malloc(uNAsymWRanges * 2 * sizeof(uint16_t));
	for (ullAsymRangeIdx = 0; ullAsymRangeIdx < uNAsymWRanges; ullAsymRangeIdx++) 
	    {
	    psuMeasInfo->ppauAsymWRanges[ullAsymRangeIdx] = psuMeasInfo->pauWtemp + (ullAsymRangeIdx * 2);
	    }
	
	//Now assign the values
	for (uWRangeIdx = 0; uWRangeIdx < uNAsymWRanges; uWRangeIdx++)
	    {
	    if ( uTMLink == 1)
		{
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] = uTM1AsymWRanges[uGlobAsymWRInd][0];
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] = uTM1AsymWRanges[uGlobAsymWRInd][1];
		}
	    else if ( ( uTMLink == 2 ) || ( uTMLink == 3 ) )
		{
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] = uTM23AsymWRanges[uGlobAsymWRInd][0];
		psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] = uTM23AsymWRanges[uGlobAsymWRInd][1];
		}
	    
	    uGlobAsymWRInd++;
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
	    if ( uTMLink == 1)
		{
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][0] = uTM1AsymFRanges[uGlobAsymFRInd][0];
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][1] = uTM1AsymFRanges[uGlobAsymFRInd][1];
		}
	    else if ( ( uTMLink == 2 ) || ( uTMLink == 3 ) )
		{
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][0] = uTM23AsymFRanges[uGlobAsymFRInd][0];
		psuMeasInfo->ppauAsymFRanges[uFRangeIdx][1] = uTM23AsymFRanges[uGlobAsymFRInd][1];
		}
	    uGlobAsymFRInd++;
	    }
	}
    else
	{
	psuMeasInfo->uNAsymFRanges   = 0;
	psuMeasInfo->ppauAsymFRanges = NULL;
	psuMeasInfo->pauFtemp        = NULL;
	}
    
    if ( ( uTMLink > 1 ) || ( ( psuMeasInfo->uLSBWord  != TM_SKIP_LSB - 1 ) || !bCombineTM1Meas ) )
	{
	sprintf(psuMeasInfo->szOutFileName,"%s--%s.out",szOutPrefix,psuMeasInfo->szAbbrev);
	psuMeasInfo->psuOutFile       = fopen(psuMeasInfo->szOutFileName,"wb");
	}
    else
	psuMeasInfo->psuOutFile       = NULL;

    return 0;
}

int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo)
{
    
    if (psuMeasInfo->uNAsymWRanges > 0)
	{
	free(psuMeasInfo->pauWtemp);
	free(psuMeasInfo->ppauAsymWRanges);
	}
    
    //free asymm frame ranges
    if (psuMeasInfo->uNAsymFRanges > 0)
	{
	free(psuMeasInfo->pauFtemp);
	free(psuMeasInfo->ppauAsymFRanges);
	}

    if ( psuMeasInfo->psuOutFile != NULL )
	fclose(psuMeasInfo->psuOutFile);

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

void vUsage(void)
    {
    printf("\n");
    printf("parse_CAPER_samples\n");
    printf("Convert a filed outputted by bust_nBit_into_16bit_file into separate measurement files!\n");
    printf("\n");
    printf("Usage: parse_CAPER_samples <input file> <output file> [flags]   \n");
    printf("                                                                      \n");
    printf("   <filename>   Input/output file names                               \n");
    printf("                                                                      \n");
    printf("   INPUT FILE PARAMETERS                                              \n");
    printf("   -s SIZE      Size of samples                    (in bits)  [%i]    \n",DEF_N_SAMPBITS);
    printf("   -n           Number of samples per minor frame  (optional) [%i]    \n",DEF_SAMPSPERMINORFRAME);
    printf("   -L           TM link number (can be 1-4)                   [%i]    \n",DEF_TM_LINK);
    printf("                                                                      \n");
    printf("                                                                      \n");
    printf("   OPTIONAL PARAMETERS                                                \n");
    printf("   -P           Output file prefix                            [%s]    \n",DEF_OUTPREFIX);
    printf("   -C           Combine MSB/LSB channels on the fly(TM1 Only!)[%i]    \n",DEF_COMBINE_TM1);
    printf("   -v           Verbose                                       [%i]    \n",DEF_VERBOSE);
    printf("                                                                      \n");
    }
