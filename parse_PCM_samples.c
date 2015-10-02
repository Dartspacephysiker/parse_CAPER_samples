/*
     parse_PCM_samples - A program for decoding binary dumps of PCM streams from IRIG-106
     =================   Chapter 10 files produced by the Wideband Recorders at the NASA
			 Wallops Flight Facility in Wallops Island, VA.

     Author:             Spencer M. Hatch, Dartmouth College
     ======= 

     Revision history:
     =================
     2015/09/07           Beginnings
		          
     2015/09/18           Adding time calculation
                          NOTE: This assumes that we don't skip the last frame! Otherwise the major frame
                                count is botched.
		          
     2015/09/19           Made a structure for PCM info. Ultimately it would be nice to read in a text file
                          to define PCM information.
		          
     2015/09/21           Separated structure definitions and defaults from parse_PCM_samples.h
		          
     2015/09/30 (v0.3)    The program now exclusively runs with .pcmconf files (using newfangled 
                          ascii_reader.h), and many error guards are now in place. Cool updated 
		          usage screen. Maybe it's ready to give to Jim Diehl?
*/


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include "defaults.h"
#include "ascii_reader.h"
#include "PCM_and_measurement_structs.h"
#include "parse_PCM_samples.h"

int main( int argc, char * argv[] )
{
    //************
    //Declare vars

    char                        szInFile[DEF_STR_SIZE];		       //input/output file stuff
    char                        szUniqCounterFile[DEF_STR_SIZE];
		                
    FILE       *                psuInFile;
    FILE       *                psuUniqCounterFile;
    		                
    char                        szOutPrefix[DEF_STR_SIZE];
		                
    char                        szPCMConfFile[DEF_STR_SIZE];
    uint8_t                     bHavePCMConfFile;

    struct stat                 suInFileStat;		       //input stuff
		                
    uint16_t   *                pauMinorFrame;
		                
    struct suPCMInfo	      * psuPCMInfo;   
    struct suMeasurementInfo ** ppsuMeasInfo;
    int16_t                     iMeasIdx;
		                
    //    uint16_t                    uAsymIdx;
		                
    int64_t                     llMinorFrameIdx;
    int64_t                     llOldMinorFrameIdx;

    //    uint16_t  *                 pauIsMinorFrameCollected;
    uint16_t **                 ppauMajorFrame;
    uint16_t  *                 pauMajFTemp;
		                
    uint64_t                    ullInFilePos;
    uint64_t                    ullWordsRead;
    uint64_t                    ullWordsWritten;
    uint64_t                    ullNSampsRead;
    uint64_t                    ullTotWordsRead;
    uint64_t                    ullTotWordsWritten;
		                
    int                         iArgIdx;
		                
    //    uint16_t                    uTMLink;
    uint8_t                     bCombineMSBLSBMeas;
    uint8_t                     bDoCheckSFIDIncrement;
    uint8_t                     bTStampMode;
    uint8_t                     bAssembleCounter;
    uint8_t                     bWriteCounter;

    int64_t                     llPCMWdOffset_MajorFrame;       //Current major frame offset (in words) relative to first recorded major frame
    int64_t                     llPCMWdOffset_MinorFrame;
    int64_t                     llPCMWdOffset_GPS;

    uint8_t                     bVerbose;
    uint8_t                     bDebug;
    int                         err;

    if (argc < 2)
	{
	vUsage();
	return EXIT_SUCCESS;
	}

    //Initialize vars			           
    szInFile[0]                    = '\0';
    szUniqCounterFile[0]           = '\0';

    psuInFile                      = (FILE *) NULL;
    psuUniqCounterFile             = (FILE *) NULL;

    szOutPrefix[0]                 = '\0';

    szPCMConfFile[0]               = '\0';
    bHavePCMConfFile               = 0;

    pauMinorFrame                  = NULL;

    psuPCMInfo                     = NULL;
    ppsuMeasInfo                   = NULL;
    iMeasIdx                       = 0;
    
    //    strcpy(szUniqCounterFile,"");		       // Default is stdout


    llMinorFrameIdx                = 0;
    llOldMinorFrameIdx             = 0;

    ppauMajorFrame                 = NULL;
    pauMajFTemp                    = NULL;

    ullInFilePos                   = 0;
    ullWordsRead                   = 0;
    ullWordsWritten                = 0;
    ullNSampsRead                  = 0;
    ullTotWordsRead                = 0;
    ullTotWordsWritten             = 0;

    iArgIdx                        = 0;
			           
    //    uTMLink                        = DEF_TM_LINK;
    bCombineMSBLSBMeas             = DEF_COMBINE_MSBLSB;
    bDoCheckSFIDIncrement          = DEF_DO_CHECK_SFID_INCREMENT;
    bTStampMode                    = DEF_CALC_TSTAMPS;
    bAssembleCounter               = DEF_ASSEMBLE_COUNTER;
    bWriteCounter                  = 0;

    llPCMWdOffset_MajorFrame       = 0;
    llPCMWdOffset_MinorFrame       = 0;
    llPCMWdOffset_GPS              = 0;
			           
    bVerbose                       = DEF_VERBOSE;
    bDebug                         = 0;
    err                            = 0;

    for (iArgIdx=1; iArgIdx<argc; iArgIdx++)   //start with 1 to skip input file
	{

	switch (argv[iArgIdx][0])
	    {
	    int iTmp;
	    case '-' :
		switch (argv[iArgIdx][1])
		    {

		    /* case 'L' :                   /\* TM Link # *\/ */
		    /* 	iArgIdx++; */
		    /* 	if(iArgIdx >= argc) */
		    /* 	    { */
		    /* 	    vUsage(); */
		    /* 	    return EXIT_FAILURE; */
		    /* 	    } */
		    /* 	sscanf(argv[iArgIdx],"%" PRIu16 ,&uTMLink); */
		    /* 	break; */

		    case 'P' :                  /* PCM conf file */
			iArgIdx++;
			bHavePCMConfFile = 1;
			strncpy(szPCMConfFile, argv[iArgIdx],DEF_STR_SIZE);
			break;

		    case 'p' :                  /* Prefix for output files */
			iArgIdx++;
			strncpy(szOutPrefix, argv[iArgIdx],DEF_STR_SIZE);
			break;

		    case 'C' :                  /* Combine TM1 MSB/LSB measurements */
			bCombineMSBLSBMeas       = 1;
			break;

		    case 'c' :                  /* Check that SFID increments uniformly */
			bDoCheckSFIDIncrement = 1;
			break;

		    case 'A' :                  /* Assemble and write unique counter */
			bAssembleCounter      = 1;
			bWriteCounter         = 1;
			break;

		    case 'T' :                  /* Produce timestamps based on GPS pulse */
			bTStampMode           = 1;
			bAssembleCounter      = 1; //Need counter for TStampMode
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
		break;

	    } /* end command line arg switch */
	} /* end for all arguments */

    /* if ( szPCMConfFile[0] == '\0' ) */
    /* 	strncpy(szPCMConfFile,DEF_PCMCONF_FILE,DEF_STR_SIZE); */

    if ( szOutPrefix[0] == '\0' )
	strncpy(szOutPrefix,DEF_OUTPREFIX,DEF_STR_SIZE);

    //init PCM channel info
    psuPCMInfo = (struct suPCMInfo * ) malloc( sizeof(struct suPCMInfo) );
    ppsuMeasInfo = (struct suMeasurementInfo ** ) malloc(psuPCMInfo->iNMeasurements * sizeof(struct suMeasurementInfo *));
    if ( bHavePCMConfFile )
	err = iInitPCMFromASCII(szPCMConfFile, psuPCMInfo, ppsuMeasInfo, bCombineMSBLSBMeas, bDoCheckSFIDIncrement, bTStampMode, bDebug );
    else
	{
	err = EXIT_FAILURE;
	printf("No .pcmconf file specified! Please specify with -P <pcmconf file> at the command line.\n");
	}
    /* else */
    /* 	err = iInitPCM(psuPCMInfo, uTMLink, bCombineMSBLSBMeas, bDoCheckSFIDIncrement, bTStampMode); */
    if ( err == EXIT_FAILURE )
	{
	printf("Couldn't init PCM! Exiting...\n");
	return EXIT_FAILURE;
	}

    if ( bTStampMode )
	{
	uint8_t bDoExit;
	bDoExit = 0;

	if ( ( psuPCMInfo->pauGPSMeasIdx == NULL ) || ( psuPCMInfo->pauMFCIndices == NULL ) )
	    {
	    printf("Neither GPS word location nor major frame counter location(s) specified in %s!\n",szPCMConfFile);
	    printf("Please edit %s to include these and try re-running, or disable timestamp calculation.\n",szPCMConfFile);
	    bDoExit = 1;
	    }
	else if ( psuPCMInfo->pauGPSMeasIdx == NULL )
	    {
	    printf("Can't calculate timestamps if GPS word location is not specified in %s!\n",szPCMConfFile);
	    printf("Please edit %s to include the GPS word location and try re-running, or disable timestamp calculation.\n",szPCMConfFile);
	    bDoExit = 1;
	    }
	else if ( psuPCMInfo->pauMFCIndices == NULL ) 
	    {
	    printf("Can't calculate major frame counter location(s) not specified in %s!\n",szPCMConfFile);
	    printf("Please edit %s to include major frame counter location(s) and try re-running, or disable timestamp calculation.\n",szPCMConfFile);
	    bDoExit = 1;
	    }
	if ( bDoExit )
	    {
	    printf("Exiting...\n");
	    return EXIT_FAILURE;
	    }
	}

    if ( bAssembleCounter && ( psuPCMInfo->pauMFCIndices == NULL ) )
	{
	printf("Can't assemble unique counter if major frame counter location(s) not specified in %s!\n",szPCMConfFile);
	printf("Please edit %s to include major frame counter location(s) and try re-running, or disable timestamp calculation.\n",szPCMConfFile);
	printf("Exiting...\n");
	return EXIT_FAILURE;
	}

    if (bVerbose) vPrintPCMInfo (psuPCMInfo);
    //	return EXIT_SUCCESS;

    if ( psuPCMInfo->ullSampBitLength < 1 || psuPCMInfo->ullSampBitLength > 16 )
	{
	fprintf(stderr,"Invalid sample size provided! Exiting...\n");
	return EXIT_FAILURE;
	}
    printf(" \n");

    //init measurements
    ppsuMeasInfo = (struct suMeasurementInfo ** ) malloc(psuPCMInfo->iNMeasurements * sizeof(struct suMeasurementInfo *));
    for ( iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++)
	{
	ppsuMeasInfo[iMeasIdx] = (struct suMeasurementInfo *) malloc(sizeof(struct suMeasurementInfo));
	if (ppsuMeasInfo[iMeasIdx] == (struct suMeasurementInfo *) NULL)
	    {
	    printf("Couldn't initialize measurement %" PRIi16 "!\n",iMeasIdx);
	    return EXIT_FAILURE;
	    }

	err = iInitMeasurementFromASCII(psuPCMInfo, ppsuMeasInfo[iMeasIdx], iMeasIdx, szOutPrefix, 
					    bCombineMSBLSBMeas, bDoCheckSFIDIncrement, bTStampMode);
	if ( err == EXIT_FAILURE )
	    {
	    printf("Couldn't init PCM! Exiting...\n");
	    return EXIT_FAILURE;
	    }
	/* else */
	/*     err = iInitMeasurement(psuPCMInfo, ppsuMeasInfo[iMeasIdx],iMeasIdx,szOutPrefix, */
	/* 			   bCombineMSBLSBMeas, bDoCheckSFIDIncrement, bTStampMode); */
	if (bVerbose) vPrintMeasurementInfo(ppsuMeasInfo[iMeasIdx]);
	}
    //    return EXIT_SUCCESS;


    //initialize major frame, binary array for keeping track of
    psuPCMInfo->ullBytesPerMinorFrame = psuPCMInfo->ullSampsPerMinorFrame * sizeof(uint16_t);
    psuPCMInfo->ullBytesPerMajorFrame = psuPCMInfo->ullBytesPerMinorFrame * psuPCMInfo->llMinorFramesPerMajorFrame;

    pauMinorFrame = malloc(psuPCMInfo->ullBytesPerMinorFrame);

    ppauMajorFrame = (uint16_t **) malloc(psuPCMInfo->llMinorFramesPerMajorFrame * sizeof(uint16_t *));
    pauMajFTemp = malloc(psuPCMInfo->ullBytesPerMajorFrame);

    for (llMinorFrameIdx = 0; llMinorFrameIdx < psuPCMInfo->llMinorFramesPerMajorFrame; llMinorFrameIdx++) 
	{
	ppauMajorFrame[llMinorFrameIdx] = pauMajFTemp + (llMinorFrameIdx * psuPCMInfo->ullSampsPerMinorFrame);
	}
    if (bVerbose) printf("Major Frame Size   :\t%" PRIu64 " bytes\n",psuPCMInfo->ullBytesPerMajorFrame);

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
	printf("Parsing PCM samples for TM%" PRIu16 "\n",psuPCMInfo->uTMLink);
    else  
	printf("Checking SFID increment; no output will be produced...\n");
    printf("\n");

    //Get first GPS word and MFCVal
    if ( !bFoundFirstMFCValGPSWordAndSearchWords(psuInFile,suInFileStat.st_size, psuPCMInfo, ppsuMeasInfo, pauMinorFrame, bCombineMSBLSBMeas, &llPCMWdOffset_GPS) )
	{
	printf("TStampMode: Unable to get a first major frame count! Is something wrong with your data?\n");
	printf("Exiting...");
	return EXIT_FAILURE;
	}

    //Loop over whole file
    llOldMinorFrameIdx = 0;
    llMinorFrameIdx    = 1;
    do
    	{
	//reset some counting vars
    	ullWordsWritten                   = 0;
	psuPCMInfo->ulMinorFrameSampCount = 0;
	psuPCMInfo->uAsymWRInd            = 0;
	psuPCMInfo->uAsymFRInd            = 0;

    	ullWordsRead = fread(pauMinorFrame, 2, psuPCMInfo->ullSampsPerMinorFrame, psuInFile);     	//Get samples from infile
    	if (ullWordsRead != psuPCMInfo->ullSampsPerMinorFrame)
    	    {
    	    fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullWordsRead*2,psuPCMInfo->ullBytesPerMinorFrame);
    	    break;
    	    }

	//Determine which minor frame this is
	llOldMinorFrameIdx = llMinorFrameIdx;
	llMinorFrameIdx = llGetMinorFrameIdx(psuPCMInfo, pauMinorFrame);  //The TM list counts from 1, not zero
	if ( bDebug ) printf("Minor frame: %" PRIX64 "\n",llMinorFrameIdx);

	if (bDoCheckSFIDIncrement)
	    {
	    llOldMinorFrameIdx %= psuPCMInfo->llMinorFramesPerMajorFrame;

	    if ( psuPCMInfo->ullMinorFrameCount > 0 && bBadSFIDIncrement(psuPCMInfo, llMinorFrameIdx, llOldMinorFrameIdx) )
		{
		psuPCMInfo->ullSkippedFrameCount++;
		printf("Minor frame skipped!\n");
		printf("Minor frame number     : %" PRIi64 "\n",llMinorFrameIdx);
		printf("Old minor frame number : %" PRIi64 "\n",llOldMinorFrameIdx);
		printf("Net minor frame count  : %" PRIu64 "\n",psuPCMInfo->ullMinorFrameCount);
		printf("Number of mismatches   : %" PRIu64 "\n",psuPCMInfo->ullSkippedFrameCount);
		printf("\n");
		}
	    }
	else
	    {
	    for (iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++)
		{
		    ppsuMeasInfo[iMeasIdx]->llSampIdx = 0; 		    //Reset at beginning of loop
		    ppsuMeasInfo[iMeasIdx]->llTSIdx   = 0;

		    uParseMeasurementSamples(psuPCMInfo,ppsuMeasInfo[iMeasIdx],iMeasIdx,pauMinorFrame,llMinorFrameIdx,&ullWordsWritten,bCombineMSBLSBMeas,bAssembleCounter,1);
		}
	    
	    //assemble unique counter, if requested
	    if ( bAssembleCounter )
		{
		    psuPCMInfo->ullCounterVal = ullAssembleCounterVal(psuPCMInfo,llMinorFrameIdx,&psuPCMInfo->ullCurrentMFCVal);
		    psuPCMInfo->ullCurrentMFCVal -= psuPCMInfo->ullFirstMFCVal; //Make current MF count relative to value of first recorded MF count
		    if ( bWriteCounter ) fprintf(psuUniqCounterFile,"%" PRIu64 "\n",psuPCMInfo->ullCounterVal);
		}

	    if ( bTStampMode )
		{
		    iCalcTStamps(psuPCMInfo, ppsuMeasInfo, pauMinorFrame, llMinorFrameIdx, 
				 &llPCMWdOffset_MajorFrame, &llPCMWdOffset_MinorFrame, &llPCMWdOffset_GPS, &ullWordsWritten, bCombineMSBLSBMeas);
		}

	    }

	//did we get the whole major frame?
	psuPCMInfo->ullMinorFrameCount++;
	if ( llMinorFrameIdx == psuPCMInfo->llMinorFramesPerMajorFrame ) psuPCMInfo->llMajorFrameCount++; //NOTE: This assumes that we don't skip the last frame!
	if ( bDebug ) printf("Major frame #%" PRIu64 "\n", psuPCMInfo->llMajorFrameCount);

    	if ( bDebug ) 
    	    {
    	    printf("N InSamps read     : %" PRIu64 "\n",ullNSampsRead);
    	    printf("InFile Position (bytes) : %" PRIu64 "\n",ullInFilePos);
    	    printf("Read %" PRIu64 " bytes; Wrote %" PRIu64 " bytes\n",ullWordsRead*2,ullWordsWritten);
    	    }

    	ullTotWordsRead += ullWordsRead;
    	ullTotWordsWritten += ullWordsWritten;

    	}  while( ( ullInFilePos = ftell(psuInFile) ) < suInFileStat.st_size  );
    
    //Summary
    printf("Minor frame count           : %" PRIu64 "\n", psuPCMInfo->ullMinorFrameCount);
    printf("Major frame count           : %" PRIu64 "\n", psuPCMInfo->llMajorFrameCount);
    if (bDoCheckSFIDIncrement) 
	{
	printf("Total number of frame skips : %" PRIu64 "\n",psuPCMInfo->ullSkippedFrameCount);
	if ( psuPCMInfo->ullSkippedFrameCount == 0 )
	    printf("\nShe's clean! No frame skips detected\n");
	else
	    printf("\nFrame skips exist within this file.\n");
	}
    else
	{
	printf("\n");
	printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Input File Bytes ) \n", ullTotWordsWritten*2, (suInFileStat.st_size));
	printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Input File Bytes ) \n", ullTotWordsWritten*2, (suInFileStat.st_size));
	printf("Output file size is %.2f%% of input file\n", (float)((float)( ullTotWordsWritten*2 )/(float)(suInFileStat.st_size))*100);
	}
    
    fclose(psuInFile); //close input file

    //release the mem!
    iPCMFree(psuPCMInfo);

    for ( iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++)
	{
	if ( ppsuMeasInfo[iMeasIdx] != NULL ) iMeasurementFree(ppsuMeasInfo[iMeasIdx]);
	}
    free(ppsuMeasInfo);

    //junk mem for major frame
    if ( pauMajFTemp                != NULL ) free(pauMajFTemp);
    if ( ppauMajorFrame             != NULL ) free(ppauMajorFrame);

    //junk minor frame stuff
    if ( pauMinorFrame              != NULL ) free(pauMinorFrame);
    //    if ( pauIsMinorFrameCollected   != NULL ) free(pauIsMinorFrameCollected);

    //Extras that should go away
    if ( psuUniqCounterFile         != NULL ) fclose(psuUniqCounterFile);

    return EXIT_SUCCESS;
}

int iInitPCMFromASCII(char * szPCMConfFile, struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo, 
		      uint8_t bCombineMSBLSBMeas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode, uint8_t bDebug )
{
    int iArgIdx;

    char fname[] = "TM23.tm";
    FILE * psuPCMConfFile;
    char              *  szLine;
    int                  iLineIdx;
    int                  bufsize;
    char              *  szCodeName;
    char              *  szDataItem;

    //*******************
    //Initialize vars
    psuPCMInfo->uAsymWRInd                     = 0;
    psuPCMInfo->uAsymFRInd                     = 0;
    psuPCMInfo->uSFIDIdx                       = 0;
    psuPCMInfo->uTMLink                        = 0;

    psuPCMInfo->iNMeasurements                 = 0;

    psuPCMInfo->ulMinorFrameSampCount          = 0;
    psuPCMInfo->ullMinorFrameCount             = 0;
    psuPCMInfo->ullSkippedFrameCount           = 0;
    psuPCMInfo->llMajorFrameCount              = 0;

    psuPCMInfo->pauMFCIndices                  = NULL;
    psuPCMInfo->paullMajorFrameVals            = NULL;
    psuPCMInfo->ullFirstMFCVal                 = 0;
    psuPCMInfo->ullCurrentMFCVal               = 0;
    psuPCMInfo->ullGPSMFCVal                   = 0;

    psuPCMInfo->ullCounterVal                  = 0;
    psuPCMInfo->pauGPSMeasIdx                  = NULL;
    psuPCMInfo->llFirstGPSWord                 = -1;
    psuPCMInfo->llCurrentGPSWord               = -1;
    psuPCMInfo->ullGPSWordCount                = -1;     //This will go to zero once we initialize the GPS word and MFC count
    psuPCMInfo->ullGPSWordStreakCount          = 0;

    psuPCMInfo->bDoCombineMSBAndLSB            = 0;

    psuPCMInfo->iUpper10MSBLower6LSBWord       = -1;

    bufsize=DEF_STR_SIZE;

    szLine = (char * )malloc(bufsize*sizeof(char));

    if( szLine == NULL)
    {
        perror("Unable to allocate string buffer\n");
        return EXIT_FAILURE;
    }

    psuPCMConfFile = (FILE *) fopen(szPCMConfFile,"r");
    if (psuPCMConfFile == NULL)
	{
	fprintf(stderr, "Error opening PCM configuration file! Did you provide one?\n");
	return EXIT_FAILURE;
	}

    //get line from file
    while( ( getline(&szLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 )
	{
	// If errors reading line then skip over it
	// Also skip comments, which are marked with "#"
	szCodeName = strtok(szLine, ":");
	szDataItem = strtok(NULL, "\n");

	/* if ( (szCodeName != NULL) && (szDataItem != NULL) && (szCodeName[0] != POUND_SIGN) ) //One-line value */
	if ( (szCodeName != NULL) && (szDataItem != NULL) && (szCodeName[0] != '#') ) //One-line value
	    {
	    char * szCodeNameTrimmed;
	    char * szDataItemTrimmed;

	    szCodeNameTrimmed = trimwhitespace(szCodeName);
	    szDataItemTrimmed = trimwhitespace(szDataItem);

	    //	    printf("SzCNTrimmed[%i]:szDataItemTrimmed[%i] --- %s : %s\n",(int)strlen(szCodeNameTrimmed),(int)strlen(szDataItemTrimmed),
	    //szCodeNameTrimmed,szDataItemTrimmed);
	    if(0) {}
	    READ_STR_TO_STRUCT(NAME,psuPCMInfo,szName)
	    READ_UINT_TO_STRUCT(TM_NUM,psuPCMInfo,uTMLink)
	    READ_INT_TO_STRUCT(N_MEASUREMENTS,psuPCMInfo,iNMeasurements)
	    READ_INT_TO_STRUCT(SFID_IDX,psuPCMInfo,uSFIDIdx)
	    READ_ULL_TO_STRUCT(BPS,psuPCMInfo,ullBitRate)
	    READ_ULL_TO_STRUCT(SAMPLE_BITLENGTH,psuPCMInfo,ullSampBitLength)
	    READ_ULL_TO_STRUCT(N_SAMPLES_PER_MINOR,psuPCMInfo,ullSampsPerMinorFrame)
	    READ_LL_TO_STRUCT(N_MINOR_PER_MAJOR,psuPCMInfo,llMinorFramesPerMajorFrame)
	    READ_UINT_TO_STRUCT(N_MAJFRAMECOUNTERS,psuPCMInfo,uNumMFCounters)
	    READ_UINT_TO_STRUCT(N_MINFRAME_BITPOS,psuPCMInfo,uMinorFrameBitShift)
	    READ_UINT_TO_STRUCT(N_GPS_WORDS,psuPCMInfo,uNGPSWordsInPCM)
	    READ_BOOL_TO_STRUCT(COMBINE_MSB_AND_LSB,psuPCMInfo,bDoCombineMSBAndLSB)
	    READ_INT_TO_STRUCT(UPPER6_MSB_LOWER10_LSB,psuPCMInfo,iUpper10MSBLower6LSBWord)
	    }
	else
	    continue;

	iLineIdx++;
	}

    //handle a few other vars that we can calculate now
    psuPCMInfo->dWordPeriod                      = (double) psuPCMInfo->ullSampBitLength/ (double) psuPCMInfo->ullBitRate;
    psuPCMInfo->ullBytesPerMinorFrame            = psuPCMInfo->ullSampsPerMinorFrame * sizeof(uint16_t);
    //    psuPCMInfo->paullMajorFrameVals              = (uint64_t *) malloc(psuPCMInfo->uNumMFCounters * sizeof(uint64_t));
    //    psuPCMInfo->pauMFCIndices                    = (uint16_t *) malloc(psuPCMInfo->uNumMFCounters * sizeof(uint16_t));

    //With knowledge of num measurements, initialize arrays
    if ( psuPCMInfo->iNMeasurements > 0 )
	{
	vInitPCMASCIIArrays(psuPCMInfo);
	}
    else
	{
	printf("Didn't find number of measurements in ASCII file!\n");
	printf("(Should be listed as 'N_MEASUREMENTS:  [n_meas]')\n");
	printf("Exiting...\n");
	return EXIT_FAILURE;
	}

    //Finished reading vals, now read arrays
    rewind(psuPCMConfFile);

    int nMeas;
    nMeas = psuPCMInfo->iNMeasurements;
    while( ( getline(&szLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 )
	{

	// If errors reading line then skip over it
	// Also skip comments, which are marked with "#"
	szCodeName = strtok(szLine, ":");
	szDataItem = strtok(NULL, "\n");

	if ( (szCodeName != NULL) && (szDataItem == NULL) && (szCodeName[0] != '#' ) && !isspace(szCodeName[0]) ) //One-line value
	    {
	    char * szCodeNameTrimmed;

	    szCodeNameTrimmed = trimwhitespace(szCodeName);
	    if (bDebug) printf("**%s\n",szCodeNameTrimmed);

	    if(0) {}  //For dat macro logic
	    READ_STR_ARR_TO_STRUCT(MEAS_NAME,psuPCMInfo,pszSerialMeasNames,nMeas)
	    READ_STR_ARR_TO_STRUCT(MEAS_ABBREV,psuPCMInfo,pszSerialMeasAbbrev,nMeas)
	    READ_STR_ARR_TO_STRUCT(MEAS_USER,psuPCMInfo,pszUser,nMeas)

	    READ_UINT_ARR_TO_STRUCT(MEAS_SAMPLE_LOC_IN_MINFRAME,psuPCMInfo,pauWord,nMeas)
	    READ_UINT_ARR_TO_STRUCT(MEAS_SAMPLE_INTERVAL_IN_MINFRAME,psuPCMInfo,pauWdInt,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_SAMPLE_FRAME,psuPCMInfo,pauFrame,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_FRAME_INTERVAL,psuPCMInfo,pauFrInt,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_N_ASYM_WORD_RANGES,psuPCMInfo,pauNAsymWRanges,nMeas)
            READ_TWO_ELEM_UINT_ARR_TO_STRUCT(MEAS_ASYM_WORD_RANGES,psuPCMInfo,ppauAsymWRanges,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_N_ASYM_FRAME_RANGES,psuPCMInfo,pauNAsymFRanges,nMeas)
            READ_TWO_ELEM_UINT_ARR_TO_STRUCT(MEAS_ASYM_FRAME_RANGES,psuPCMInfo,ppauAsymFRanges,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_LSB_WORD,psuPCMInfo,pauLSBWord,nMeas)

            READ_UL_ARR_TO_STRUCT(MEAS_SAMPLE_RATE,psuPCMInfo,paulSPS,nMeas)

            READ_UINT_ARR_TO_STRUCT(MEAS_TSTAMP_CALC_TYPE,psuPCMInfo,pauTSCalcType,nMeas)
            READ_STR_ARR_TO_STRUCT(MEAS_TSTAMP_SEARCH_WORD,psuPCMInfo,pszTSSearchWords,nMeas)
            READ_ULL_ARR_TO_STRUCT(MEAS_INTERN_SAMP_RATE,psuPCMInfo,pullInternWordRate,nMeas)
            READ_UINT_ARR_TO_STRUCT(MAJFRAME_INDEX,psuPCMInfo,pauMFCIndices,psuPCMInfo->uNumMFCounters)
            READ_UINT_ARR_TO_STRUCT(GPS_INDEX,psuPCMInfo,pauGPSMeasIdx,psuPCMInfo->uNGPSWordsInPCM) //This one is questionable

	    }
	}
    
    fclose(psuPCMConfFile);

    return EXIT_SUCCESS;
}

void vInitPCMASCIIArrays(struct suPCMInfo * psuPCMInfo)
{

    int nMeas;
    int iMeasIdx;
    int iArgIdx;

    nMeas = psuPCMInfo->iNMeasurements;

    psuPCMInfo->pszSerialMeasNames  = (char **     ) malloc(nMeas*sizeof(char *));
    psuPCMInfo->pszSerialMeasAbbrev = (char **     ) malloc(nMeas*sizeof(char *));
    psuPCMInfo->pszUser             = (char **     ) malloc(nMeas*sizeof(char *));

    psuPCMInfo->pauWord             = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauWdInt            = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauFrame            = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauFrInt            = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->paulSPS             = (uint32_t *  ) malloc(nMeas*sizeof(uint32_t));
    psuPCMInfo->pauNAsymWRanges     = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->ppauAsymWRanges     = (uint16_t ** ) malloc(nMeas*sizeof(uint16_t *));
    psuPCMInfo->pauNAsymFRanges     = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->ppauAsymFRanges     = (uint16_t ** ) malloc(nMeas*sizeof(uint16_t *));
    psuPCMInfo->pauLSBWord          = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauTSCalcType       = (uint16_t *  ) malloc(nMeas*sizeof(uint16_t));

    psuPCMInfo->pszTSSearchWords    = (char **     ) malloc(nMeas*sizeof(char *));

    for( iMeasIdx = 0; iMeasIdx < nMeas; iMeasIdx++ )
	{
	psuPCMInfo->pszSerialMeasNames[iMeasIdx]       = (char *     ) malloc(DEF_STR_SIZE*sizeof(char));
	psuPCMInfo->pszSerialMeasAbbrev[iMeasIdx]      = (char *     ) malloc(DEF_STR_SIZE*sizeof(char));
	psuPCMInfo->pszUser[iMeasIdx]                  = (char *     ) malloc(DEF_STR_SIZE*sizeof(char));
	psuPCMInfo->pszTSSearchWords[iMeasIdx]         = (char *     ) malloc(DEF_STR_SIZE*sizeof(char));

	psuPCMInfo->ppauAsymWRanges[iMeasIdx]          = (uint16_t * ) malloc(2*sizeof(uint16_t));
	psuPCMInfo->ppauAsymFRanges[iMeasIdx]          = (uint16_t * ) malloc(2*sizeof(uint16_t));
	}

    psuPCMInfo->pullInternWordRate  = (uint64_t *) malloc(nMeas*sizeof(uint64_t));
    
    if ( psuPCMInfo->uNumMFCounters > 0 )
	{
	psuPCMInfo->paullMajorFrameVals = (uint64_t *) malloc(psuPCMInfo->uNumMFCounters * sizeof(uint64_t));
	psuPCMInfo->pauMFCIndices       = (uint16_t *) malloc(psuPCMInfo->uNumMFCounters * sizeof(uint16_t));
	for ( iArgIdx = 0; iArgIdx < psuPCMInfo->uNumMFCounters; iArgIdx++ )
	    psuPCMInfo->paullMajorFrameVals[iArgIdx] = -1;
	}
    else
	{
	psuPCMInfo->paullMajorFrameVals = NULL;
	psuPCMInfo->pauMFCIndices       = NULL;
	}

    if ( psuPCMInfo->uNGPSWordsInPCM > 0 )
	{
	psuPCMInfo->pauGPSMeasIdx       = (uint16_t *) malloc(psuPCMInfo->uNGPSWordsInPCM * sizeof(uint16_t));
	}
    else
	psuPCMInfo->pauGPSMeasIdx       = NULL;

}

int iInitMeasurementFromASCII(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int16_t iMeasIdx,char * szOutPrefix, 
			      uint8_t bCombineMSBLSBMeas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode )
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

    psuMeasInfo->szAbbrev[0]         = '\0';
    psuMeasInfo->szOutFile[0]        = '\0';
    psuMeasInfo->szTStampFile[0]     = '\0';

    psuMeasInfo->llSampIdx           =  0;
    psuMeasInfo->llTotalSampCount    =  0;
    psuMeasInfo->llPCMWdOffset_TSSW  = -1;       //In any case, set these based on TM descriptions
    psuMeasInfo->llTSIdx             =  0;
    psuMeasInfo->uTSSWIdx            = -1;       
    psuMeasInfo->llTSSW_SampleNum    = -1;       
    psuMeasInfo->llTSSW_MFCVal       = -1;
    psuMeasInfo->uTSSWIdx            = -1;

    szTempLen = 0;
    while ( psuPCMInfo->pszSerialMeasNames[iMeasIdx][szTempLen] != '\0' )
	szTempLen++;
    strncpy(psuMeasInfo->szName, psuPCMInfo->pszSerialMeasNames[iMeasIdx], //Name of measurement, e.g., "Langmuir Probe Measurement 1 MSB"
	    szTempLen );    

    szAbbrevLen = 0;
    while ( psuPCMInfo->pszSerialMeasAbbrev[iMeasIdx][szAbbrevLen] != '\0' )
	szAbbrevLen++;
    strncpy(psuMeasInfo->szAbbrev, psuPCMInfo->pszSerialMeasAbbrev[iMeasIdx],	//Abbreviation for measurement, e.g., "LP01MSB"
	    szAbbrevLen );
    
    szTempLen = 0;
    while ( psuPCMInfo->pszUser[iMeasIdx][szTempLen] != '\0' )
	szTempLen++;
    strncpy(psuMeasInfo->szUser, psuPCMInfo->pszUser[iMeasIdx],			   //Who is the user? E.g., Dartmouth, U Iowa
	    szTempLen );
    
    psuMeasInfo->uWord               = psuPCMInfo->pauWord[iMeasIdx] - 1;  //Beginning word in the frame, -1 for counting from zero
    psuMeasInfo->uWdInt              = psuPCMInfo->pauWdInt[iMeasIdx];     //Word interval
    psuMeasInfo->uMinorFrame         = psuPCMInfo->pauFrame[iMeasIdx];     //Which minor frame is it in?
    psuMeasInfo->uMinorFrInt         = psuPCMInfo->pauFrInt[iMeasIdx];     //How often does it show up?
    psuMeasInfo->ulSPS               = psuPCMInfo->paulSPS[iMeasIdx];
    if ( psuMeasInfo->uWdInt != 0 )
	psuMeasInfo->uSampsPerMinorFrame = psuPCMInfo->ullSampsPerMinorFrame/psuMeasInfo->uWdInt; //How many of these to expect per frame?
    
    psuMeasInfo->uLSBWord            = psuPCMInfo->pauLSBWord[iMeasIdx] - 1;
    if ( ( bCombineMSBLSBMeas && ( psuMeasInfo->uLSBWord != TM_SKIP_LSB - 1 ) ) && ( psuMeasInfo->uLSBWord != TM_NO_LSB -1 ) &&
	 ( psuMeasInfo->uLSBWord != 65535 ) ) //'65535' means uLSBWord was zero for this measurement in the .pcmconf file
	psuMeasInfo->szAbbrev[szAbbrevLen-4] = '\0';
    
    uNAsymWRanges                    = psuPCMInfo->pauNAsymWRanges[iMeasIdx];
    uNAsymFRanges                    = psuPCMInfo->pauNAsymFRanges[iMeasIdx];
    
    if ( bTStampMode )
	{
	psuMeasInfo->uTSCalcType     = psuPCMInfo->pauTSCalcType[iMeasIdx];
	if ( psuPCMInfo->pszTSSearchWords[iMeasIdx] != '\0' )
	    sprintf(psuMeasInfo->szTSSearchWord,"%s",psuPCMInfo->pszTSSearchWords[iMeasIdx]); 
	else
	    psuMeasInfo->szTSSearchWord[0] = '\0';
	}
    else
	{
	    psuMeasInfo->uTSCalcType     = 0;
	}
    if ( psuPCMInfo->pullInternWordRate[iMeasIdx] > 0 )
	psuMeasInfo->dInternalWordPeriod     = (double)1/(double)psuPCMInfo->pullInternWordRate[iMeasIdx];
    else
	psuMeasInfo->dInternalWordPeriod     = 0;
    
    
    //handle asymmetric word ranges
    if (uNAsymWRanges > 0)
	{
	psuMeasInfo->uNAsymWRanges       = uNAsymWRanges;
	psuMeasInfo->ppauAsymWRanges     = (uint16_t **) malloc(uNAsymWRanges * sizeof(uint16_t *));
	if ( psuMeasInfo->ppauAsymWRanges == NULL )
	    return EXIT_FAILURE;

	ullBytesInAsymWRanges            = uNAsymWRanges * sizeof(uint16_t *) * 2;
	
	psuMeasInfo->uSampsPerMinorFrame = 0; //This has to be recalculated

	//initialize memory word ranges
	psuMeasInfo->pauWtemp            = malloc(uNAsymWRanges * 2 * sizeof(uint16_t));
	if ( psuMeasInfo->pauWtemp == NULL )
	    return EXIT_FAILURE;

	for (ullAsymRangeIdx = 0; ullAsymRangeIdx < uNAsymWRanges; ullAsymRangeIdx++) 
	    {
	    psuMeasInfo->ppauAsymWRanges[ullAsymRangeIdx] = psuMeasInfo->pauWtemp + (ullAsymRangeIdx * 2);
	    }
	
	//Now assign the values
	for (uWRangeIdx = 0; uWRangeIdx < uNAsymWRanges; uWRangeIdx++)
	    {
	    psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] = psuPCMInfo->ppauAsymWRanges[psuPCMInfo->uAsymWRInd][0];
	    psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] = psuPCMInfo->ppauAsymWRanges[psuPCMInfo->uAsymWRInd][1];

	    psuMeasInfo->uSampsPerMinorFrame += (psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1] - psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0] + 1);
	    psuPCMInfo->uAsymWRInd++;

	    //printf("%s: Asymm Range is [%u,%u]\n",psuMeasInfo->szName,psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0],psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1]);
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
	if ( psuMeasInfo->ppauAsymFRanges == NULL )
	    return EXIT_FAILURE;
	
	ullBytesInAsymFRanges        = uNAsymFRanges * sizeof(uint16_t *) * 2;
	
	//initialize memory word ranges
	psuMeasInfo->pauFtemp = malloc(uNAsymFRanges * 2 * sizeof(uint16_t));
	if ( psuMeasInfo->pauFtemp == NULL )
	    return EXIT_FAILURE;
	
	for (ullAsymRangeIdx = 0; ullAsymRangeIdx < uNAsymFRanges; ullAsymRangeIdx++) 
	    {
	    psuMeasInfo->ppauAsymFRanges[ullAsymRangeIdx] = psuMeasInfo->pauFtemp + (ullAsymRangeIdx * 2);
	    }
	
	//Now assign the values
	for (uFRangeIdx = 0; uFRangeIdx < uNAsymFRanges; uFRangeIdx++)
	    {
	    psuMeasInfo->ppauAsymFRanges[uFRangeIdx][0] = psuPCMInfo->ppauAsymFRanges[psuPCMInfo->uAsymFRInd][0];
	    psuMeasInfo->ppauAsymFRanges[uFRangeIdx][1] = psuPCMInfo->ppauAsymFRanges[psuPCMInfo->uAsymFRInd][1];

	    psuPCMInfo->uAsymFRInd++;
	    }
	}
    else
	{
	psuMeasInfo->uNAsymFRanges   = 0;
	psuMeasInfo->ppauAsymFRanges = NULL;
	psuMeasInfo->pauFtemp        = NULL;
	}
    
    //Now handle initialization of buffers
    psuMeasInfo->paullSample         = (uint64_t *) malloc(psuMeasInfo->uSampsPerMinorFrame * sizeof(uint64_t));
    if ( psuMeasInfo->paullSample == NULL )
	return EXIT_FAILURE;
	
    for ( iTmpIdx = 0; iTmpIdx < psuMeasInfo->uSampsPerMinorFrame; iTmpIdx++ )
	psuMeasInfo->paullSample[iTmpIdx] = -1;

    //Do timestamp stuff
    if ( psuMeasInfo->uTSCalcType > 0 )
	{
	char szTStampSuffix[128];
	
	if ( ( psuMeasInfo->uTSCalcType == 1 ) )
	    sprintf(szTStampSuffix,"%s","tstamps_rel_to_GPS");
	else if ( ( psuMeasInfo->uTSCalcType == 2 ) )
	    sprintf(szTStampSuffix,"%s","tstamps_rel_to_GPS--samples");
	else if ( ( psuMeasInfo->uTSCalcType == 3 ) )
	    sprintf(szTStampSuffix,"%s","tstamps_rel_to_searchword--samples");
	else if ( psuMeasInfo->uTSCalcType == 4 )
	    sprintf(szTStampSuffix,"%s","searchword_tstamps--searchword_samplenumber");


	sprintf(psuMeasInfo->szTStampFile,"%s--%s--%s.txt",szOutPrefix,psuMeasInfo->szAbbrev,szTStampSuffix);
	psuMeasInfo->psuTStampFile    = (FILE *) fopen(psuMeasInfo->szTStampFile,"w");
	psuMeasInfo->pallPCMWdOffsets  = (int64_t *) malloc(psuMeasInfo->uSampsPerMinorFrame * sizeof(int64_t));
	if ( psuMeasInfo->pallPCMWdOffsets == NULL )
	    return EXIT_FAILURE;
	
	}
    else
	{
	psuMeasInfo->szTStampFile[0]  = '\0';
	psuMeasInfo->psuTStampFile    = NULL;
	psuMeasInfo->pallPCMWdOffsets = NULL;
	}
    //	psuMeasInfo->uOffsetBufCount  = 0;

	//Output file things
	//Don't make an output file is this is an LSB channel and we're combining samples
        if ( ( ( psuPCMInfo->uTMLink > 1 ) || ( ( psuMeasInfo->uLSBWord  != TM_SKIP_LSB - 1 ) || !bCombineMSBLSBMeas ) ) && !bDoCheckSFIDIncrement 
	 && ( psuMeasInfo->uTSCalcType != 2 ) && ( psuMeasInfo->uTSCalcType != 3 ) )
	{
	sprintf(psuMeasInfo->szOutFile,"%s--%s.out",szOutPrefix,psuMeasInfo->szAbbrev);
	psuMeasInfo->psuOutFile       = (FILE *) fopen(psuMeasInfo->szOutFile,"wb");
	}
    else
	{
	psuMeasInfo->szOutFile[0]     = '\0';
	psuMeasInfo->psuOutFile       = NULL;
	}

    return EXIT_SUCCESS;
}

//Write samples from this minor frame to the appropriate measurement files
uint16_t uParseMeasurementSamples(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int iMeasIdx, 
				  uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, 
				  uint64_t * pullWordsWritten,
				  uint8_t bCombineMSBLSBMeas, uint8_t bAssembleCounter, uint8_t iWriteSamplesToFile)
{
//Modes for iWriteMode
//====================
//0: Don't write to any files or sample buffers; only find MFC words or special search words and update counters
//1: Write sample directly to the binary file corresponding to the current measurement
//2: Write samples to measurement buffers ( so that, e.g, timestamps and measurement values can be outputted as ASCII )
    int iArgIdx;
    int iWdIdx;
    int iFrIdx;

    int iWriteMode;

    if ( iWriteSamplesToFile == 1 )
	{
	if ( psuMeasInfo->uTSCalcType <= 1 )
	    iWriteMode = 1;
	else if ( ( psuMeasInfo->uTSCalcType == 2 ) || ( psuMeasInfo->uTSCalcType == 3 ) )
	    iWriteMode = 2;
	else if ( psuMeasInfo->uTSCalcType == 4 )
	    iWriteMode = 1;
	}
    else
	{
	iWriteMode = 0;
	}
	
    uint16_t uAsymIdx;
    // If there are no asymmetric word or frame ranges, just write the sample at the specified word and any others within minor frame
    if ( ( psuMeasInfo->uNAsymWRanges == 0 ) && ( psuMeasInfo->uNAsymFRanges == 0 ) )
	{
	if ( (llMinorFrameIdx % psuMeasInfo->uMinorFrInt) == ( psuMeasInfo->uMinorFrame % psuMeasInfo->uMinorFrInt ) )
	    {
	    if ( ( psuPCMInfo->bDoCombineMSBAndLSB ) && ( bCombineMSBLSBMeas ) )
		{
		int iLSBWdIdx;
		uint16_t uCombinedSample;
		uint8_t bIsCombined;
		iLSBWdIdx = psuMeasInfo->uLSBWord;
		if( iLSBWdIdx != ( TM_SKIP_LSB - 1 ) ) //Is this an LSB channel to be skipped?
		    {
		    for (iWdIdx = psuMeasInfo->uWord; iWdIdx < psuPCMInfo->ullSampsPerMinorFrame; iWdIdx += psuMeasInfo->uWdInt)
			{
			if ( iLSBWdIdx == TM_NO_LSB - 1 ) //Is there a corresponding LSB word that needs combination?
			    {
			    uCombinedSample = pauMinorFrame[iWdIdx];
			    bIsCombined = 0;
			    if ( bAssembleCounter )
				{
				for ( iArgIdx = 0; iArgIdx < psuPCMInfo->uNumMFCounters; iArgIdx++ )
				    {
				    if ( iMeasIdx == psuPCMInfo->pauMFCIndices[iArgIdx] )
					{
					psuPCMInfo->paullMajorFrameVals[iArgIdx] = uCombinedSample;
					break;
					}
				    }
				}
			    }
			/* else if ( iLSBWdIdx == TM_UPPER6_MSB_LOWER10_LSB - 1) //GPS and ACS are weird */
			else if ( iLSBWdIdx == psuPCMInfo->iUpper10MSBLower6LSBWord - 1) //GPS and ACS are weird
			    {
			    uCombinedSample = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) << 10 ) | ( pauMinorFrame[iLSBWdIdx] );
			    bIsCombined = 1;
			    }
			else
			    {
			    uCombinedSample = ( ( pauMinorFrame[iWdIdx] & 0x3FF ) <<  6 ) | ( pauMinorFrame[iLSBWdIdx] >> 4 );
			    bIsCombined = 1;
			    }

			
			if ( iWriteMode >= 1 )
			    {
			    vWriteSample((uint64_t)uCombinedSample, psuMeasInfo->llSampIdx, psuMeasInfo, iWriteMode, bIsCombined, pullWordsWritten);
			    psuMeasInfo->llSampIdx++;
			    }

			if ( psuMeasInfo->uTSCalcType >= 1 ) 
			    {
			    vSetTStampInfo(psuPCMInfo, psuMeasInfo, pauMinorFrame, llMinorFrameIdx, iWdIdx );
			    }

			psuMeasInfo->llTotalSampCount++;
			psuPCMInfo->ulMinorFrameSampCount += ( 1 + bIsCombined );
			iLSBWdIdx += psuMeasInfo->uWdInt;
			}
		    }
		}
	    else //Just good old samples, no LSB channels to worry about
		{
		for (iWdIdx = psuMeasInfo->uWord; iWdIdx < psuPCMInfo->ullSampsPerMinorFrame; iWdIdx += psuMeasInfo->uWdInt)
		    {
		    if ( iWriteMode >= 1 )
			{
			vWriteSample((uint64_t)pauMinorFrame[iWdIdx], psuMeasInfo->llSampIdx, psuMeasInfo, iWriteMode,  0, pullWordsWritten);
			psuMeasInfo->llSampIdx++;
			}

		    if ( psuMeasInfo->uTSCalcType >= 1 ) 
			{
			vSetTStampInfo(psuPCMInfo, psuMeasInfo, pauMinorFrame, llMinorFrameIdx, iWdIdx );
			}

		    if ( bAssembleCounter )
			{
			for ( iArgIdx = 0; iArgIdx < psuPCMInfo->uNumMFCounters; iArgIdx++ )
			    {
			    if ( iMeasIdx == psuPCMInfo->pauMFCIndices[iArgIdx] )
				{
				psuPCMInfo->paullMajorFrameVals[iArgIdx] = pauMinorFrame[iWdIdx];
				break;
				}
			    }
			}
		    
		    psuMeasInfo->llTotalSampCount++;
		    psuPCMInfo->ulMinorFrameSampCount++;
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
		if ( iWriteMode >= 1 )
		    {
			//printf("Samp: %" PRIu16 "\n",pauMinorFrame[iWdIdx]);
		    vWriteSample((uint64_t)pauMinorFrame[iWdIdx], psuMeasInfo->llSampIdx, psuMeasInfo, iWriteMode,  0, pullWordsWritten);
		    psuMeasInfo->llSampIdx++;
		    }

		if ( psuMeasInfo->uTSCalcType >= 1 ) 
		    {
		    vSetTStampInfo(psuPCMInfo, psuMeasInfo, pauMinorFrame, llMinorFrameIdx, iWdIdx );
		    }

		psuMeasInfo->llTotalSampCount++;
		psuPCMInfo->ulMinorFrameSampCount++;
		}
	    }
	psuPCMInfo->uAsymWRInd += psuMeasInfo->uNAsymWRanges;
	}
    // Otherwise, if we have asymmetric frame ranges but no asymmetric word ranges ...
    else if ( ( psuMeasInfo->uNAsymWRanges == 0 ) && ( psuMeasInfo->uNAsymFRanges > 0 ) )
	{
	int iLowerLim;
	int iUpperLim;
	iWdIdx = psuMeasInfo->uWord;    //Because the word location is never asymmetric when the frame location is,
                                	//we just assume it's always the same word

	for (uAsymIdx = 0; uAsymIdx < psuMeasInfo->uNAsymFRanges; uAsymIdx++)
	    {
	    iLowerLim = psuMeasInfo->ppauAsymFRanges[uAsymIdx][0]; //these ranges are inclusive
	    iUpperLim = psuMeasInfo->ppauAsymFRanges[uAsymIdx][1];
	    for ( iFrIdx = iLowerLim; iFrIdx <= iUpperLim; iFrIdx++)
		{
		if ( iFrIdx == llMinorFrameIdx )
		    {
		    if ( iWriteMode >= 1 )
			{
			vWriteSample((uint64_t)pauMinorFrame[iWdIdx], psuMeasInfo->llSampIdx, psuMeasInfo, iWriteMode,  0, pullWordsWritten);
			psuMeasInfo->llSampIdx++;
			}

		    if ( psuMeasInfo->uTSCalcType >= 1 ) 
			{
			vSetTStampInfo(psuPCMInfo, psuMeasInfo, pauMinorFrame, llMinorFrameIdx, iWdIdx );
			}

		    if ( bAssembleCounter )
			{
			for ( iArgIdx = 0; iArgIdx < psuPCMInfo->uNumMFCounters; iArgIdx++ )
			    {
			    if ( iMeasIdx == psuPCMInfo->pauMFCIndices[iArgIdx] )
				{
				psuPCMInfo->paullMajorFrameVals[iArgIdx] = pauMinorFrame[iWdIdx];
				break;
				}
			    }
			}

		    psuMeasInfo->llTotalSampCount++;
		    psuPCMInfo->ulMinorFrameSampCount++;
		    }
		}
	    }
	psuPCMInfo->uAsymFRInd += psuMeasInfo->uNAsymFRanges;
	}

    return EXIT_SUCCESS;
}

void vWriteSample(uint64_t ullSample, int64_t llSampleIdx, struct suMeasurementInfo * psuMeasInfo, int iWriteMode, uint8_t bIsCombined, uint64_t * pullWordsWritten)
{
    if ( iWriteMode == 1 )
	{
	    (*pullWordsWritten) += fwrite((int16_t * )&ullSample,2,1,psuMeasInfo->psuOutFile) * ( 1 + bIsCombined );
	}
    else if ( iWriteMode == 2 )
	{
	psuMeasInfo->paullSample[llSampleIdx] = ullSample;
	}
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

uint64_t ullAssembleCounterVal(struct suPCMInfo * psuPCMInfo, int64_t llMinorFrameIdx,uint64_t * pullMFCVal)
{
    uint64_t ullCounterVal;
    int      iMFCIdx;

    ullCounterVal = 0;
    for ( iMFCIdx = 0; iMFCIdx < psuPCMInfo->uNumMFCounters; iMFCIdx++ )
	{
	    ullCounterVal = ullCounterVal | ( psuPCMInfo->paullMajorFrameVals[iMFCIdx] << psuPCMInfo->ullSampBitLength*iMFCIdx ); 
	}
    if ( pullMFCVal != NULL ) 
	(*pullMFCVal) = ullCounterVal;

	ullCounterVal = (ullCounterVal << psuPCMInfo->uMinorFrameBitShift) | (llMinorFrameIdx - 1); //last, tack on minor frame

    return ullCounterVal;
}

uint8_t bFoundFirstMFCValGPSWordAndSearchWords(FILE * psuInFile, size_t szInFileSize, 
					       struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo,
					       uint16_t * pauMinorFrame, uint8_t bCombineMSBLSBMeas, int64_t * pllPCMWdOffset_GPS)
{
    int                  iMeasIdx;
    int                  iArgIdx;

    uint64_t             ullWordsWritten;
    uint32_t             ulMinorFrameSampCount;
    int64_t              llMinorFrameIdx;
    uint64_t             ullInFilePos;
    uint64_t             ullTemp;
    uint16_t             uMFCValCount;
    uint8_t              bAllMFCValsCollected;

    uint8_t              bGotFirstGPSWord;
    int64_t              llFirstGPSWord;

    //    uint8_t              bGotAllTSSearchWords;
    uint16_t             uNTSSearchWords;
    uint16_t             uTSSWIdx;
    int64_t           *  pallFirst_PCMWdOffset;
    int64_t           *  pallFirstTSSWSampleNum;
    uint16_t          *  pauFirst_TSSWIdx;
    int64_t           *  pallFirstTSSW_MFCVal;
    int               *  paiMeasIdx;

    //    uint8_t           *  pabGotTSSearchWord;

    ullWordsWritten        = 0;
    ulMinorFrameSampCount  = 0;
    llMinorFrameIdx        = 0;
    ullInFilePos           = 0;
    iMeasIdx               = 0;
    ullTemp                = 0;
    uMFCValCount           = 0;
    bAllMFCValsCollected   = 0;
			   
    bGotFirstGPSWord       = 0;
    llFirstGPSWord         = 0;
			   
    //    bGotAllTSSearchWords = 0;
    uNTSSearchWords        = 0;    			   
    uTSSWIdx               = 0;
    pallFirst_PCMWdOffset  = NULL;

    for (iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++)
	{
	if ( ppsuMeasInfo[iMeasIdx]->szTSSearchWord[0] != '\0' ) 
	    uNTSSearchWords++;
	}

    //    pabGotTSSearchWord   = calloc(uNTSSearchWords * sizeof(uint8_t));
    pauFirst_TSSWIdx       = calloc(uNTSSearchWords, sizeof(uint16_t));
    pallFirstTSSWSampleNum = calloc(uNTSSearchWords, sizeof(int64_t));
    pallFirstTSSW_MFCVal   = calloc(uNTSSearchWords, sizeof(int64_t));
    paiMeasIdx             = calloc(uNTSSearchWords, sizeof(int));
    pallFirst_PCMWdOffset  = calloc(uNTSSearchWords, sizeof(int64_t));

    for ( iArgIdx = 0; iArgIdx < uNTSSearchWords; iArgIdx++ )
	pallFirst_PCMWdOffset[iArgIdx] = -1;
    
    while ( ( !bAllMFCValsCollected || !bGotFirstGPSWord || ( uTSSWIdx < uNTSSearchWords ) ) && ( ( ullInFilePos = ftell(psuInFile) ) < szInFileSize ) )
	{
	psuPCMInfo->uAsymWRInd = 0;
	psuPCMInfo->uAsymFRInd = 0;

	ullTemp = fread(pauMinorFrame, 2, psuPCMInfo->ullSampsPerMinorFrame, psuInFile);
	if (ullTemp != psuPCMInfo->ullSampsPerMinorFrame)
	    {
	    fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullTemp*2,psuPCMInfo->ullSampsPerMinorFrame*2);
	    return 0;
	    }
	
	llMinorFrameIdx = llGetMinorFrameIdx(psuPCMInfo, pauMinorFrame);  //The TM list counts from 1, not zero
	
	for (iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++)
	    {
	    ppsuMeasInfo[iMeasIdx]->llSampIdx = 0; 		    //Reset at beginning of loop
	    ppsuMeasInfo[iMeasIdx]->llTSIdx   = 0;

	    uParseMeasurementSamples(psuPCMInfo, ppsuMeasInfo[iMeasIdx], iMeasIdx, pauMinorFrame, llMinorFrameIdx, &ullWordsWritten, bCombineMSBLSBMeas,1,0);

	    if ( uTSSWIdx <  uNTSSearchWords )
		{
		    if ( ( ppsuMeasInfo[iMeasIdx]->szTSSearchWord[0] != TM_NO_TS_SEARCH ) && ( ppsuMeasInfo[iMeasIdx]->llPCMWdOffset_TSSW != -1 ) )
		    {
			pallFirst_PCMWdOffset[uNTSSearchWords-1] = ppsuMeasInfo[iMeasIdx]->llPCMWdOffset_TSSW;
			pallFirstTSSWSampleNum[uNTSSearchWords-1]= ppsuMeasInfo[iMeasIdx]->llTSSW_SampleNum;
			pauFirst_TSSWIdx[uNTSSearchWords-1]      = ppsuMeasInfo[iMeasIdx]->uTSSWIdx;
			pallFirstTSSW_MFCVal[uNTSSearchWords-1]  = ppsuMeasInfo[iMeasIdx]->llTSSW_MFCVal;
			paiMeasIdx[uNTSSearchWords-1]            = iMeasIdx;

			/* printf("Search word %u\n",uTSSWIdx); */
			/* printf("%s (%s) llPCMWdOffset_TSSW: %" PRIi64 "\n", */
			/*        ppsuMeasInfo[iMeasIdx]->szName,ppsuMeasInfo[iMeasIdx]->szTSSearchWord,ppsuMeasInfo[iMeasIdx]->llPCMWdOffset_TSSW); */
			/* printf("          llTSSW_SampleNum: %11" PRIi64 "\n",ppsuMeasInfo[iMeasIdx]->llTSSW_SampleNum); */
			/* printf(" psuPCMInfo->ullCounterVal: %11" PRIi64 "\n",psuPCMInfo->ullCounterVal); */
			/* printf("             llTSSW_MFCVal: %11" PRIi64 "\n",ppsuMeasInfo[iMeasIdx]->llTSSW_MFCVal); */
			/* printf("End search word init\n\n"); */

			uTSSWIdx++;
		    }
		}
	    }
	
	if ( !bAllMFCValsCollected )
	    {
		for ( iArgIdx = 0; iArgIdx < psuPCMInfo->uNumMFCounters; iArgIdx++ )
		    {			         
		    if ( psuPCMInfo->paullMajorFrameVals[iArgIdx] != -1 )
			uMFCValCount++;
		    }		
	    }
	
	
	if ( uMFCValCount == psuPCMInfo->uNumMFCounters && ( psuPCMInfo->ullFirstMFCVal == 0 ) )
	    {
	    psuPCMInfo->ullCounterVal = ullAssembleCounterVal(psuPCMInfo,llMinorFrameIdx, &psuPCMInfo->ullFirstMFCVal);
	    bAllMFCValsCollected = 1;
	    }
	else
	    {
	    uMFCValCount = 0;
	    }

	if ( !bGotFirstGPSWord )
	    {
		if ( bGotNewGPSWord(psuPCMInfo, ppsuMeasInfo[psuPCMInfo->pauGPSMeasIdx[0]], llMinorFrameIdx, pauMinorFrame, bCombineMSBLSBMeas) )
		{
		bGotFirstGPSWord = 1;
		psuPCMInfo->ullCounterVal = ullAssembleCounterVal(psuPCMInfo, llMinorFrameIdx,&psuPCMInfo->ullGPSMFCVal);
		}
	    }
	}

    if ( bAllMFCValsCollected && bGotFirstGPSWord )
	{
	//Get the proper GPS offset
	(*pllPCMWdOffset_GPS) = psuPCMInfo->llFirstGPSWord + psuPCMInfo->llMinorFramesPerMajorFrame * (psuPCMInfo->ullGPSMFCVal - psuPCMInfo->ullFirstMFCVal) * psuPCMInfo->ullSampsPerMinorFrame - 1;

	//Get the proper TSSW offset, reset total number of samples
	for ( iArgIdx = 0; iArgIdx < uNTSSearchWords; iArgIdx++ )
	    {
	    iMeasIdx = paiMeasIdx[iArgIdx];
	    ppsuMeasInfo[iMeasIdx]->llTSSW_SampleNum = pallFirstTSSWSampleNum[iArgIdx];
	    ppsuMeasInfo[iMeasIdx]->llPCMWdOffset_TSSW = pallFirst_PCMWdOffset[iArgIdx];
	    //	    ppsuMeasInfo[iMeasIdx]->llTSSW_MFCVal = (pallFirstTSSW_MFCVal[iArgIdx] - psuPCMInfo->ullFirstMFCVal);
	    ppsuMeasInfo[iMeasIdx]->llTSSW_MFCVal = pallFirstTSSW_MFCVal[iArgIdx];
	    ppsuMeasInfo[iMeasIdx]->uTSSWIdx = pauFirst_TSSWIdx[iArgIdx];

	    /* printf("Search word %u\n",iArgIdx); */
	    /* printf("%s (%s) llPCMWdOffset_TSSW: %" PRIi64 "\n", */
	    /* 	   ppsuMeasInfo[iMeasIdx]->szName,ppsuMeasInfo[iMeasIdx]->szTSSearchWord,ppsuMeasInfo[iMeasIdx]->llPCMWdOffset_TSSW); */
	    /* printf("          llTSSW_SampleNum: %11" PRIi64 "\n",ppsuMeasInfo[iMeasIdx]->llTSSW_SampleNum); */
	    /* printf(" psuPCMInfo->ullCounterVal: %11" PRIi64 "\n",psuPCMInfo->ullCounterVal); */
	    /* printf("             llTSSW_MFCVal: %11" PRIi64 "\n",ppsuMeasInfo[iMeasIdx]->llTSSW_MFCVal); */
	    /* printf("End search word init\n\n"); */
	    ppsuMeasInfo[iMeasIdx]->llTotalSampCount = 0;
	    }

	/* printf("First GPS word       : %" PRIi64 "\n",psuPCMInfo->llFirstGPSWord); */
	/* printf("First GPS word offset: %" PRIi64 "\n",*pllPCMWdOffset_GPS); */

	//Now we go back to the beginning of the file
	rewind(psuInFile);
	ullInFilePos = 0;
	}
    else
	{
	printf("allmfc: %i, gotfirstgps: %i\n",bAllMFCValsCollected,bGotFirstGPSWord);
	return 0;
	}

    return 1;
}

int bGotNewGPSWord(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, uint16_t * pauMinorFrame,
			uint8_t bCombineMSBLSBMeas)
{
    int iWdIdx;

    uint16_t uNonZeroGPSWordCount;
    int64_t  llTestGPSWord;

    uNonZeroGPSWordCount = 0;
    llTestGPSWord        = -1;

    if ( (llMinorFrameIdx % psuMeasInfo->uMinorFrInt) == ( psuMeasInfo->uMinorFrame % psuMeasInfo->uMinorFrInt ) )
	{
	if ( ( psuPCMInfo->bDoCombineMSBAndLSB ) && ( bCombineMSBLSBMeas ) )
	    {
	    int iLSBWdIdx;
	    iWdIdx = psuMeasInfo->uWord;
	    iLSBWdIdx = psuMeasInfo->uLSBWord;

	    if( iLSBWdIdx != ( TM_SKIP_LSB - 1 ) )
		{
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
		}
	    }
	else
	    {
	    llTestGPSWord = pauMinorFrame[psuMeasInfo->uWord];
	    }
	}

    if ( llTestGPSWord > 0 )
	{
	if ( psuPCMInfo->llFirstGPSWord == -1 )
	    {
		psuPCMInfo->llFirstGPSWord = llTestGPSWord;
	    }
	else if ( llTestGPSWord != psuPCMInfo->llCurrentGPSWord )
	    {
	    psuPCMInfo->llCurrentGPSWord = llTestGPSWord;
	    //	    psuPCMInfo->ullGPSWordCount++;
	    psuPCMInfo->ullGPSWordStreakCount = 0;
	    }
	else
	    {
		//	    psuPCMInfo->ullGPSWordCount++;
	    psuPCMInfo->ullGPSWordStreakCount++;
	    }
	return 1;
	}

    return 0;
}

void vUpdateGPSWord(struct suPCMInfo * psuPCMInfo, int64_t * pllPCMWdOffset_MajorFrame, int64_t * pllPCMWdOffset_GPS)
{
    psuPCMInfo->ullGPSWordCount++;
    if ( psuPCMInfo->llCurrentGPSWord > -1 )
	*pllPCMWdOffset_GPS = (*pllPCMWdOffset_MajorFrame) + psuPCMInfo->llCurrentGPSWord;
    else
	*pllPCMWdOffset_GPS = (*pllPCMWdOffset_MajorFrame) + psuPCMInfo->llFirstGPSWord;

    /* printf("GPS Word Count     : %" PRIu64 "\n",psuPCMInfo->ullGPSWordCount); */
    /* printf("llPCMWdOffset_GPS   : %" PRIu64 "\n",*pllPCMWdOffset_GPS); */
}

void vSetTStampInfo(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, int iWdIdx )
{
    if ( ( psuMeasInfo->uTSCalcType == 1 ) || ( psuMeasInfo->uTSCalcType == 2 ) )
	{
	psuMeasInfo->pallPCMWdOffsets[psuMeasInfo->llTSIdx] = iWdIdx;
	psuMeasInfo->llTSIdx++;
	}
    else if ( psuMeasInfo->uTSCalcType == 3 )
	{
	psuMeasInfo->pallPCMWdOffsets[psuMeasInfo->llTSIdx] = psuMeasInfo->llTotalSampCount;
	psuMeasInfo->llTSIdx++;
	}

    if ( ( psuMeasInfo->uTSCalcType >= 3 ) && bIsSearchWord(psuPCMInfo, psuMeasInfo, pauMinorFrame, iWdIdx, llMinorFrameIdx) )
	{
	vUpdateSearchWord(psuPCMInfo, psuMeasInfo, llMinorFrameIdx, iWdIdx);

	if ( psuMeasInfo->uTSCalcType == 4 ) //Here, we want to output the searchword sample number with the timestamp 
	                                     //instead of the value of the searchword. We use the measurement's sample buffer to hold the value
	    {
	    psuMeasInfo->pallPCMWdOffsets[psuMeasInfo->llTSIdx] = iWdIdx;
	    vWriteSample(psuMeasInfo->llTotalSampCount, psuMeasInfo->llTSIdx, psuMeasInfo, 2, 0, NULL);
	    printf("@Index%5" PRIi64 ", llTotSampCount is %" PRIi64 "\n",psuMeasInfo->llTSIdx, psuMeasInfo->llTotalSampCount);
	    psuMeasInfo->llTSIdx++;
	    }
	}
}

int iCalcTStamps(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo, uint16_t * pauMinorFrame, int64_t llMinorFrameIdx, int64_t * pllPCMWdOffset_MajorFrame, int64_t * pllPCMWdOffset_MinorFrame, int64_t * pllPCMWdOffset_GPS, uint64_t * pullWordsWritten, uint8_t bCombineMSBLSBMeas)
{
    int iMeasIdx;

    //    printf("Bout to calc some tstamps...\n");

    *pllPCMWdOffset_MajorFrame = ( psuPCMInfo->ullCurrentMFCVal * psuPCMInfo->llMinorFramesPerMajorFrame * psuPCMInfo->ullSampsPerMinorFrame) - 1;
    *pllPCMWdOffset_MinorFrame = ( llMinorFrameIdx - 1 ) * psuPCMInfo->ullSampsPerMinorFrame;
    
    //Check for new GPS word in this minor frame, then calculate GPS word offset
    if ( bGotNewGPSWord(psuPCMInfo, ppsuMeasInfo[psuPCMInfo->pauGPSMeasIdx[0]], llMinorFrameIdx, pauMinorFrame, bCombineMSBLSBMeas) )
	{
	vUpdateGPSWord(psuPCMInfo, pllPCMWdOffset_MajorFrame, pllPCMWdOffset_GPS);
	}
    
    for (iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++)
	{
	    
	if ( ( ppsuMeasInfo[iMeasIdx]->uTSCalcType > 0 ) && ( ppsuMeasInfo[iMeasIdx]->llTSIdx > 0 ) )
	    {
	    iWriteMeasurementTStamps(psuPCMInfo,ppsuMeasInfo[iMeasIdx], llMinorFrameIdx,
				     *pllPCMWdOffset_MajorFrame, *pllPCMWdOffset_MinorFrame,
				     *pllPCMWdOffset_GPS, pullWordsWritten);
	    }
	}

    return EXIT_SUCCESS;
}

int iWriteMeasurementTStamps(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx,
			     int64_t llPCMWdOffset_MajorFrame, int64_t llPCMWdOffset_MinorFrame, int64_t llPCMWdOffset_GPS, uint64_t * pullWordsWritten)
{
    int iArgIdx;
    int64_t llBaseOffset;
    int64_t llPCMWdOffset_Measurement;
    double  dTimeOffset_Measurement;

    llBaseOffset = llPCMWdOffset_MajorFrame + llPCMWdOffset_MinorFrame;

    if ( psuMeasInfo->uTSCalcType >= 3 ) //Do special trickery for measurements that aren't actually sampled at rate given in NASA PCM measurement list
	{
	int64_t llPCMWdOffset_SW;
	double dTimeOffset_SW;

	//calculate searchword offset relative to GPS word
	llPCMWdOffset_SW = psuMeasInfo->llPCMWdOffset_TSSW - llPCMWdOffset_GPS; //This is the word offset within the PCM stream
	dTimeOffset_SW  = llPCMWdOffset_SW * psuPCMInfo->dWordPeriod + 
	    ( ( psuPCMInfo->llCurrentGPSWord != -1 ) ? psuPCMInfo->ullGPSWordCount : 0 ); //strangeness here so that we don't count first GPS word twice

	for (iArgIdx = 0; iArgIdx < psuMeasInfo->llTSIdx; iArgIdx++ )
	    {
	    //calc offset relative to number of samples between current sample and searchword sample here; 
	    //This offset is NOT calculated based on word offset in the PCM stream
	    llPCMWdOffset_Measurement = ( psuMeasInfo->pallPCMWdOffsets[iArgIdx] - psuMeasInfo->llTSSW_SampleNum ) / 2.0; //rel to current search word
	    dTimeOffset_Measurement = (double) llPCMWdOffset_Measurement * psuMeasInfo->dInternalWordPeriod + dTimeOffset_SW;
		
	    fprintf(psuMeasInfo->psuTStampFile,"%12.8f\t%10" PRIu64 "\n",dTimeOffset_Measurement,psuMeasInfo->paullSample[iArgIdx]);
	    (*pullWordsWritten)++;
	    }
	}
    else
	{
	for (iArgIdx = 0; iArgIdx < psuMeasInfo->llTSIdx; iArgIdx++ )
	    {
	    llPCMWdOffset_Measurement = (llBaseOffset + psuMeasInfo->pallPCMWdOffsets[iArgIdx]) - llPCMWdOffset_GPS; //rel to current GPS val
	    dTimeOffset_Measurement = llPCMWdOffset_Measurement * psuPCMInfo->dWordPeriod + 
		( ( psuPCMInfo->llCurrentGPSWord != -1 ) ? psuPCMInfo->ullGPSWordCount : 0 ); //strangeness here so that we don't count first GPS word twice

	    fprintf(psuMeasInfo->psuTStampFile,"%12.8f",dTimeOffset_Measurement);
	    if ( psuMeasInfo->uTSCalcType == 2 )
		{
		fprintf(psuMeasInfo->psuTStampFile,"\t%5" PRIu16 "\n", (uint16_t)psuMeasInfo->paullSample[iArgIdx]);
		(*pullWordsWritten)++;
		}
	    else
		{
		fprintf(psuMeasInfo->psuTStampFile,"\n");
		}
	    }
	}

    return EXIT_SUCCESS;
}

int64_t llGetMinorFrameIdx(struct suPCMInfo * psuPCMInfo, uint16_t * pauMinorFrame)
{
    return (pauMinorFrame[psuPCMInfo->uSFIDIdx-1] + 1) & 0b0000111111;  //The TM list counts from 1, not zero
}



uint8_t bBadSFIDIncrement(struct suPCMInfo * psuPCMInfo, int64_t llMinorFrameIdx, int64_t llOldMinorFrameIdx)
{
    return ( ( ( llMinorFrameIdx - llOldMinorFrameIdx ) != 1 ) && ( ( llMinorFrameIdx - llOldMinorFrameIdx ) != 1 - psuPCMInfo->llMinorFramesPerMajorFrame ) );
}

uint8_t bIsSearchWordInMinorFrame(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, uint16_t * pauMinorFrame, int64_t llMinorFrameIdx)
{
    uint16_t    uWdIdx;
    char        acTestWord[5];
    size_t      szKeywordLen;

    acTestWord[4] = '\0'; //Gotta terminate the test word properly so we don't make memory hate itself

    szKeywordLen = strlen(psuMeasInfo->szTSSearchWord);

    for ( uWdIdx = 0; uWdIdx < psuPCMInfo->ullSampsPerMinorFrame -1; uWdIdx++ )
	{
	acTestWord[0] = (char) ( ( pauMinorFrame[uWdIdx]        ) & 0xFF );
	acTestWord[1] = (char) ( ( pauMinorFrame[uWdIdx]   >> 8 ) & 0xFF );

	acTestWord[2] = (char) ( ( pauMinorFrame[uWdIdx+1]      ) & 0xFF );
	acTestWord[3] = (char) ( ( pauMinorFrame[uWdIdx+1] >> 8 ) & 0xFF );

	if ( strncmp(acTestWord,psuMeasInfo->szTSSearchWord,szKeywordLen) == 0 )
	    {
	    if ( psuMeasInfo->uTSCalcType >= 3 )
		{
		    psuMeasInfo->uTSSWIdx = uWdIdx; //NOTE! This value is intermediate to the calculation of the search word offset! It will change later!
		}
	    else
		{
		    //		psuMeasInfo->pallPCMWdOffsets[psuMeasInfo->uOffsetBufCount] = uWdIdx;
		    //		psuMeasInfo->uOffsetBufCount++;
		}
		//		printf("Got a search word! It's like this: %s\n",acTestWord);
	    return 1; //Drop after the first instance
	    }
	}

    return 0;
}

uint8_t bIsSearchWord(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, uint16_t * pauMinorFrame, uint16_t uWdIdx, int64_t llMinorFrameIdx)
{
    char        acTestWord[5];
    size_t      szKeywordLen;

    acTestWord[4] = '\0'; //Gotta terminate the test word properly so we don't make memory hate itself

    szKeywordLen = strlen(psuMeasInfo->szTSSearchWord);

    acTestWord[0] = (char) ( ( pauMinorFrame[uWdIdx]        ) & 0xFF );
    acTestWord[1] = (char) ( ( pauMinorFrame[uWdIdx]   >> 8 ) & 0xFF );
    
    acTestWord[2] = (char) ( ( pauMinorFrame[uWdIdx+1]      ) & 0xFF );
    acTestWord[3] = (char) ( ( pauMinorFrame[uWdIdx+1] >> 8 ) & 0xFF );

    if ( strncmp(acTestWord,psuMeasInfo->szTSSearchWord,szKeywordLen) == 0 )
	{
	//		printf("Got a search word! It's like this: %s\n",acTestWord);
	return 1; //Drop after the first instance
	}

    return 0;
}

void vUpdateSearchWord(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo * psuMeasInfo, int64_t llMinorFrameIdx, int iWdIdx)
{
    psuMeasInfo->uTSSWIdx = iWdIdx; //relative to PCM stream
    psuPCMInfo->ullCounterVal = ullAssembleCounterVal(psuPCMInfo,llMinorFrameIdx, &psuMeasInfo->llTSSW_MFCVal);
    /* psuMeasInfo->llPCMWdOffset_TSSW = psuMeasInfo->uTSSWIdx + psuPCMInfo->llMinorFramesPerMajorFrame * psuPCMInfo->ullCurrentMFCVal * psuPCMInfo->ullSampsPerMinorFrame; */
    psuMeasInfo->llPCMWdOffset_TSSW = psuMeasInfo->uTSSWIdx + psuPCMInfo->llMinorFramesPerMajorFrame * (psuMeasInfo->llTSSW_MFCVal - psuPCMInfo->ullFirstMFCVal) * psuPCMInfo->ullSampsPerMinorFrame;
    
    psuMeasInfo->llTSSW_SampleNum  = psuMeasInfo->llTotalSampCount; //internal stuff

    /* printf("%s (%s) llPCMWdOffset_TSSW: %11" PRIi64 "\n", */
    /* 	   psuMeasInfo->szName,psuMeasInfo->szTSSearchWord,psuMeasInfo->llPCMWdOffset_TSSW); */
    /* printf("          llTSSW_SampleNum: %11" PRIi64 "\n",psuMeasInfo->llTSSW_SampleNum); */
    /* printf(" psuPCMInfo->ullCounterVal: %11" PRIi64 "\n",psuPCMInfo->ullCounterVal); */
    /* printf("psuMeasInfo->llTSSW_MFCVal: %11" PRIi64 "\n",psuMeasInfo->llTSSW_MFCVal); */
    /* printf("\n"); */
}

//void vResetMeasSampleInde

int iPCMFree(struct suPCMInfo * psuPCMInfo)
{
    if ( psuPCMInfo->pauMFCIndices        != NULL ) free( psuPCMInfo->pauMFCIndices );
    if ( psuPCMInfo->paullMajorFrameVals  != NULL ) free( psuPCMInfo->paullMajorFrameVals );
    if ( psuPCMInfo->pauGPSMeasIdx        != NULL ) free( psuPCMInfo->pauGPSMeasIdx );    

    vFreePCMASCIIArrays(psuPCMInfo);

    free( psuPCMInfo );

    return (EXIT_SUCCESS);
}

void vFreePCMASCIIArrays(struct suPCMInfo * psuPCMInfo)
{

    int iMeasIdx;

    for( iMeasIdx = 0; iMeasIdx < psuPCMInfo->iNMeasurements; iMeasIdx++ )
	{
	    if ( psuPCMInfo->pszSerialMeasNames               != NULL )
		{
		if ( psuPCMInfo->pszSerialMeasNames[iMeasIdx] != NULL )
		    free( psuPCMInfo->pszSerialMeasNames[iMeasIdx] );
		}

	    if ( psuPCMInfo->pszSerialMeasAbbrev              != NULL )
		{
		if ( psuPCMInfo->pszSerialMeasAbbrev[iMeasIdx]!= NULL )
		    free( psuPCMInfo->pszSerialMeasAbbrev[iMeasIdx] );
		}

	    if ( psuPCMInfo->pszUser                          != NULL )
		{
		if ( psuPCMInfo->pszUser[iMeasIdx]            != NULL )
		    free( psuPCMInfo->pszUser[iMeasIdx] );
		}

	    if ( psuPCMInfo->pszTSSearchWords                 != NULL )
		{
		if ( psuPCMInfo->pszTSSearchWords[iMeasIdx]   != NULL )
		    free( psuPCMInfo->pszTSSearchWords[iMeasIdx] );
		}

	    if ( psuPCMInfo->ppauAsymWRanges                  != NULL )
		{
		if ( psuPCMInfo->ppauAsymWRanges[iMeasIdx]    != NULL )
		    free( psuPCMInfo->ppauAsymWRanges[iMeasIdx] );
		}

	    if ( psuPCMInfo->ppauAsymFRanges                  != NULL )
		{
		if ( psuPCMInfo->ppauAsymFRanges[iMeasIdx]    != NULL )
		    free( psuPCMInfo->ppauAsymFRanges[iMeasIdx] );
		}
	}

    if ( psuPCMInfo->pszSerialMeasNames != NULL )
	free( psuPCMInfo->pszSerialMeasNames );
    if ( psuPCMInfo->pszSerialMeasAbbrev != NULL )
	free( psuPCMInfo->pszSerialMeasAbbrev );
    if ( psuPCMInfo->pszUser != NULL )
	free( psuPCMInfo->pszUser );
    
    if ( psuPCMInfo->pauWord != NULL )
	free( psuPCMInfo->pauWord );
    if ( psuPCMInfo->pauWdInt != NULL )
	free( psuPCMInfo->pauWdInt );
    if ( psuPCMInfo->pauFrame != NULL )
	free( psuPCMInfo->pauFrame );
    if ( psuPCMInfo->pauFrInt != NULL )
	free( psuPCMInfo->pauFrInt );
    if ( psuPCMInfo->paulSPS != NULL )
	free( psuPCMInfo->paulSPS );

    if ( psuPCMInfo->pauNAsymWRanges != NULL )
	free( psuPCMInfo->pauNAsymWRanges );
    if ( psuPCMInfo->ppauAsymWRanges != NULL )
	free( psuPCMInfo->ppauAsymWRanges );

    if ( psuPCMInfo->pauNAsymFRanges != NULL )
	free( psuPCMInfo->pauNAsymFRanges );
    if ( psuPCMInfo->ppauAsymFRanges != NULL )
	free( psuPCMInfo->ppauAsymFRanges );

    if ( psuPCMInfo->pauLSBWord != NULL )
	free( psuPCMInfo->pauLSBWord );
    if ( psuPCMInfo->pauTSCalcType != NULL )
	free( psuPCMInfo->pauTSCalcType );
    
    if ( psuPCMInfo->pszTSSearchWords != NULL )
	free( psuPCMInfo->pszTSSearchWords );
    
    if ( psuPCMInfo->pullInternWordRate != NULL )
	free( psuPCMInfo->pullInternWordRate );
    
    //    if ( psuPCMInfo->pauGPSMeasIdx != NULL )
    //	free( psuPCMInfo->pauGPSMeasIdx );
}

int iMeasurementFree(struct suMeasurementInfo * psuMeasInfo)
{
    
    if ( psuMeasInfo->pauWtemp        != NULL ) free(psuMeasInfo->pauWtemp);
    if ( psuMeasInfo->ppauAsymWRanges != NULL ) free(psuMeasInfo->ppauAsymWRanges);
    
    //free asymm frame ranges
    if ( psuMeasInfo->pauFtemp         != NULL ) free(psuMeasInfo->pauFtemp);
    if ( psuMeasInfo->ppauAsymFRanges  != NULL ) free(psuMeasInfo->ppauAsymFRanges);

    if ( psuMeasInfo->paullSample      != NULL ) free(psuMeasInfo->paullSample);

    if ( psuMeasInfo->psuOutFile       != NULL ) fclose(psuMeasInfo->psuOutFile);
    if ( psuMeasInfo->psuTStampFile    != NULL ) fclose(psuMeasInfo->psuTStampFile);
    if ( psuMeasInfo->pallPCMWdOffsets != NULL ) free(psuMeasInfo->pallPCMWdOffsets);

    free(psuMeasInfo);

    return (EXIT_SUCCESS);
}

void vPrintSubFrame (uint16_t * pauMajorFrame, int64_t llMinorFrameIdx)
{ 


}

void vPrintPCMInfo (struct suPCMInfo * psuPCMInfo)
{
    printf("                                              \n");
    printf("***************************************************\n");
    printf("*****************PCM Configuration*****************\n");
    printf("Subframe ID word index               :   %10" PRIu16 "\n",psuPCMInfo->uSFIDIdx);
    printf("TM Link #                            :   %10" PRIu16 "\n",psuPCMInfo->uTMLink);
    printf("PCM bit rate                         :   %10" PRIu64 "\n",psuPCMInfo->ullBitRate);
    printf("PCM word period (s)                  :   %10.8f\n"     ,psuPCMInfo->dWordPeriod);
    printf("N measurements tracked               :   %10" PRIu16 "\n",psuPCMInfo->iNMeasurements);
    printf("                                              \n");
    printf("Sample word length in bits           :   %10" PRIu64 "\n",psuPCMInfo->ullSampBitLength);
    printf("Number of samples per minor frame    :   %10" PRIu64 "\n",psuPCMInfo->ullSampsPerMinorFrame);
    printf("N Minor frames per major frame       :   %10" PRIi64 "\n",psuPCMInfo->llMinorFramesPerMajorFrame);
    printf("                                              \n");
    printf("N bits in minor frame counter        :   %10" PRIu16 "\n",psuPCMInfo->uMinorFrameBitShift);
    printf("N major frame counters               :   %10" PRIu16 "\n",psuPCMInfo->uNumMFCounters);
    printf("                                              \n");
    if ( psuPCMInfo->bDoCombineMSBAndLSB )
	{
	    printf("Combine MSB and LSB words            :         TRUE\n");
    printf("                                              \n");		                 
	}
    if ( psuPCMInfo->iUpper10MSBLower6LSBWord > -1 )
	{
	printf("Upper10MSBLower6LSB word             :   %10" PRIi16 "\n",psuPCMInfo->iUpper10MSBLower6LSBWord);
	printf("                                              \n");
	}
}

void vPrintMeasurementInfo (struct suMeasurementInfo * psuMeasInfo)
{
    int iArgIdx;

    printf("****************************************************\n");
    printf("******%-40.40s******\n",psuMeasInfo->szName);
    //    printf("Measurement Name          :   %s\n",psuMeasInfo->szName);
    printf("Abbrev                    :   %s\n",psuMeasInfo->szAbbrev);
    printf("User                      :   %s\n",psuMeasInfo->szUser);
    printf("Samples per second        :   %" PRIu16 "\n",psuMeasInfo->ulSPS);
    printf("\n");
    printf("Word                      :   %" PRIu16 "\n",psuMeasInfo->uWord);
    printf("Word Interval             :   %" PRIu16 "\n",psuMeasInfo->uWdInt);
    //    printf("# Asym word ranges        :   %" PRIu16 "\n",psuMeasInfo->uNAsymWRanges);
    if ( psuMeasInfo->uNAsymWRanges > 0 )
	{
	for (iArgIdx = 0; iArgIdx < psuMeasInfo->uNAsymWRanges; iArgIdx++)
	    {
	    printf("Asymm word range #%3i     :   [%u,%u]\n",iArgIdx+1,psuMeasInfo->ppauAsymWRanges[iArgIdx][0],
		   psuMeasInfo->ppauAsymWRanges[iArgIdx][1]);
	    }
	}

    //    printf("N Words in Minor Frame :   %" PRIu16 "\n",psuMeasInfo->uSampsPerMinorFrame);
    printf("\n");		      
    printf("Minor Frame               :   %" PRIu16 "\n",psuMeasInfo->uMinorFrame);
    printf("Minor Frame Interval      :   %" PRIu16 "\n",psuMeasInfo->uMinorFrInt);
    printf("Samples per minor frame   :   %" PRIu16 "\n",psuMeasInfo->uSampsPerMinorFrame);
    //    printf("# Asym frame ranges       :   %" PRIu16 "\n",psuMeasInfo->uNAsymFRanges);
    if ( psuMeasInfo->uNAsymFRanges > 0 )
	{
	for (iArgIdx = 0; iArgIdx < psuMeasInfo->uNAsymFRanges; iArgIdx++)
	    {
	    printf("Asymm frame range #%3i    :   [%u,%u]\n",iArgIdx+1,psuMeasInfo->ppauAsymFRanges[iArgIdx][0],
		   psuMeasInfo->ppauAsymFRanges[iArgIdx][1]);
	    }
	}
    printf("\n");
    printf("TStamp calc enabled       :   %" PRIu8 "\n",psuMeasInfo->uTSCalcType);
    printf("\n");
    if ( psuMeasInfo->szTSSearchWord != TM_NO_TS_SEARCH )
	printf("Search word               :   %s\n",psuMeasInfo->szTSSearchWord);
    if ( psuMeasInfo->dInternalWordPeriod > 0.1 )
	printf("Internal word period (s)  :   %10.3f\n",psuMeasInfo->dInternalWordPeriod);
    printf("\n");
}

void vUsage(void)
{
    printf("\n");
    printf("parse_PCM_samples\n");
    printf("Convert a Chapter 10 PCM dump into separate measurement files!                  \n");
    printf(" Note: All of the TMs are defined in the respective header files!               \n");
    printf("                                                                                \n");
    printf("Usage: parse_CAPER_samples <filename> [parameters]                              \n");
    printf("                                                                                \n");
    printf("   <filename>   Input file name                                                 \n");
    printf("                                                                                \n");
    printf("   PARAMETERS                                                                   \n");
    printf("   -P           PCM config file                                         [none]  \n");
    printf("                 *Open PCM_conf_header.pcmconf to see the format                \n");
    printf("                                                                                \n");
    printf("   -p           Prefix for output files                                 [%s]    \n",DEF_OUTPREFIX);
    printf("                                                                                \n");
    printf("   -C           Combine MSB/LSB channels on the fly(TM1 Only!)          [%i]    \n",DEF_COMBINE_MSBLSB);
    printf("   -c           Check integrity of data by following SFID,              [%i]    \n",DEF_DO_CHECK_SFID_INCREMENT);
    printf("                    (no parsed output is produced)                              \n");
    printf("                                                                                \n");
    printf("   -A           Assemble unique counter based on major/minor            [%i]    \n",DEF_ASSEMBLE_COUNTER);
    printf("                    frames, and output to file                                  \n");
    printf("                                                                                \n");
    printf("   -T           Produce timestamps for measurements specified           [%i]    \n",DEF_CALC_TSTAMPS);
    printf("                    in PCM config file (NOT on command line!)                   \n");
    printf("                The following are valid values for the                          \n");
    printf("                  MEAS_TSTAMP_CALC_TYPE variable in PCM config files:           \n"); 
    printf("                1) TStamps calculated relative to GPS word,                     \n");
    printf("                     sample output in separate BINARY file                      \n");
    printf("                2) TStamps calculated relative to GPS word,                     \n");
    printf("                     samples and TStamps output to the same ASCII file          \n");
    printf("                3) TStamps calculated relative to measurement searchword,       \n");
    printf("                     using the corresponding measurement's (internal)           \n");
    printf("                     word period.                                               \n");
    printf("                4) TStamps corresponding to the searchword are outputted,       \n");
    printf("                     along with the TSSW sample number where the                \n");
    printf("                     searchword was found. (Useful for making sure that         \n");
    printf("                     the searchword is showing up at the right intervals.)      \n");
    printf("                                                                                \n");
    printf("   -v           Verbose                                                 [%i]    \n",DEF_VERBOSE);
    printf("                                                                                \n");
    printf("   -h           Help (this menu)                                                \n");
}
