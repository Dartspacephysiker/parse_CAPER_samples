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

#define DEF_N_SAMPBITS                   16 //in bits
#define MAX_N_MINORFRAMES               256
#define DEF_SAMPSPERMINORFRAME          120
#define DEF_MINOR_PER_MAJOR              32

#define DEF_VERBOSE                       0 //please tell me

#define DEF_OUTPREFIX  "parsed_TM1-samples"
#define DEF_STR_SIZE                   1024

//struct declarations
struct suChanInfo
{
    char        szName[1024];          //Name of the channel, e.g., "Langmuir Probe Channel 1 MSB"
    char        szAbbrev[1024];        //Abbreviation for channel, e.g., "LP01MSB"
    char        szUser[1024];	      //Who is the user? E.g., Dartmouth, U Iowa
    uint16_t    uWord;		      //Beginning word in the frame
    uint16_t    uWdInt;		      //Word interval
    uint16_t    uSampsPerMinorFrame;     //How many of these to expect per frame? Most are just one.
    uint16_t    uMinorFrame;          //Which minor frame is it in?
    uint16_t    uMinorFrInt;          //How often does it show up?
    uint32_t    ulSPS;

    uint16_t    uSample;
    uint64_t    ullSampCount;

    char        szOutFileName[1024];
    FILE *      psuOutFile;
};

//function declarations
int iChanInit(struct suChanInfo * psuChInfo, int16_t iChIdx,char * szOutPrefix, uint64_t ullSampsPerMinorFrame);

void vUsage(void);
void vPrintChanInfo (struct suChanInfo * psuChInfo);
void vPrintSubFrame (uint16_t * pauMajorFrame, int16_t iMinorFrameIdx);

//struct suChanInfo * psuChanInit(int16_t iChIdx);

int main( int argc, char * argv[] )
    {
    //************
    //Declare vars

    char         szInFile[512];		       //input/output file stuff
    char         szOutFile[512];

    FILE       * psuInFile;
    FILE       * psuOutFile;
    //    char         szOutPrefix[] = DEF_OUTPREFIX;
    char         szOutPrefix[1024];

    struct stat  suInFileStat;		       //input stuff
    uint64_t     ulSampBitLength;	       //Sample size (in bytes)
    uint64_t     ullSampsPerMinorFrame;	       //Number of samples to grab at each periodic interval
    uint64_t     ullNBytesPerMinorFrame;	       //Number of bits in MinorFrame

    uint16_t   * pauMinorFrame;

    //    struct suChanInfo * psuChInfo[N_TM1_CHANS];
    struct suChanInfo ** ppsuChInfo;
    int16_t      iChIdx;
    int16_t      iNChans;

    uint64_t     ullMinorFrameIdx;
    int64_t      llMinorFramesPerMajorFrame;
    uint64_t     ullBytesInMajorFrame;
    int64_t      llMajorFrameCount;
    uint16_t  *  pauIsMinorFrameCollected;
    uint16_t **  ppauMajorFrame;
    uint16_t  *  temp;

    uint64_t     ullInFilePos;
    uint64_t     ullBytesRead;
    uint64_t     ullBytesWritten;
    uint64_t     ullNSampsRead;
    uint64_t     ullTotBytesRead;
    uint64_t     ullTotBytesWritten;

    uint32_t     ulMinorFrameSampCount;
    uint64_t     ullMinorFrameCount;
    int32_t      lSaveData;

    int          iArgIdx;

    uint8_t      bVerbose;

    //*******************
    //Initialize vars
    ulMinorFrameSampCount  = 0;
    ullMinorFrameCount     = 0;
    lSaveData              = 1;

    iArgIdx                = 0;

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
    strncpy(szOutPrefix,DEF_OUTPREFIX,1024);

    ulSampBitLength         = DEF_N_SAMPBITS;
    ullSampsPerMinorFrame   = DEF_SAMPSPERMINORFRAME;
    ullNBytesPerMinorFrame  = ulSampBitLength * ullSampsPerMinorFrame;

    ppsuChInfo = (struct suChanInfo **) !NULL;
    iChIdx                  = 0;
    iNChans                 = N_TM1_CHANS;
    ullMinorFrameIdx = 0;
    llMinorFramesPerMajorFrame = DEF_MINOR_PER_MAJOR;

    ullInFilePos            = 0;
    ullBytesRead            = 0;
    ullBytesWritten         = 0;
    ullNSampsRead           = 0;
    ullTotBytesRead         = 0;
    ullTotBytesWritten      = 0;

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
			sscanf(argv[iArgIdx],"%" PRIi64 ,&ulSampBitLength);
			break;

		    case 'n' :                   /* # Samples */
			iArgIdx++;
			if(iArgIdx >= argc)
			    {
			    vUsage();
			    return EXIT_FAILURE;
			    }
			sscanf(argv[iArgIdx],"%" PRIi64 ,&ullSampsPerMinorFrame);
			break;

		    case 'P' :                  /* Verbosities */
			iTmp = 0;
			iArgIdx++;
			//			while( (iArgIdx) != '\0')
			//			    {
			//			    szOutPrefix[iTmp++] = *argv[iArgIdx];
			strncpy(szOutPrefix, argv[iArgIdx],DEF_STR_SIZE);
			    //			    printf("%s\n",argv[iArgIdx]);
			//			    }
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

    if ( ulSampBitLength < 1 || ulSampBitLength > 16 )
	{
	fprintf(stderr,"Invalid sample size provided! Exiting...\n");
	return EXIT_FAILURE;
	}
    printf(" \n");

    //init chans
    //    psuChInfo = (struct suChanInfo *) malloc(sizeof(struct suChanInfo) * iNChans);
    ppsuChInfo = malloc(N_TM1_CHANS * sizeof(struct suChanInfo *));
    for ( iChIdx = 0; iChIdx < iNChans; iChIdx++)
	{
        int err;
	//	    psuChInfo[iChIdx] = psuChanInit(iChIdx);
	ppsuChInfo[iChIdx] = (struct suChanInfo *) malloc(sizeof(struct suChanInfo));
	//	if (psuChInfo == (struct suChanInfo *) !NULL)
	if (ppsuChInfo[iChIdx] == (struct suChanInfo *) !NULL)
	    {
	    printf("Couldn't initialize channel %" PRIi16 "!\n",iChIdx);
	    return -1;
	    }

	    err = iChanInit(ppsuChInfo[iChIdx],iChIdx,szOutPrefix,ullSampsPerMinorFrame);
	    if (bVerbose) vPrintChanInfo(ppsuChInfo[iChIdx]);
	}

    ullBytesInMajorFrame = llMinorFramesPerMajorFrame * sizeof(uint16_t *);

    //initialize major frame, binary array for keeping track of
    pauMinorFrame = malloc(ullSampsPerMinorFrame * ulSampBitLength);

    pauIsMinorFrameCollected = (uint16_t *) calloc(llMinorFramesPerMajorFrame,2);
    ppauMajorFrame = (uint16_t **) malloc(ullBytesInMajorFrame);
    temp = malloc(llMinorFramesPerMajorFrame * ullSampsPerMinorFrame * sizeof(uint16_t));

    for (ullMinorFrameIdx = 0; ullMinorFrameIdx < llMinorFramesPerMajorFrame; ullMinorFrameIdx++) 
	{
	ppauMajorFrame[ullMinorFrameIdx] = temp + (ullMinorFrameIdx * ullSampsPerMinorFrame);
	}
    printf("Major Frame Size   :\t%" PRIu64 " bytes\n",ullBytesInMajorFrame);


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
    printf("\nConverting %" PRIu64 "-bit to 16-bit samples...\n", ulSampBitLength);

    //Loop over whole file
    do
    	{
    	ullBytesWritten = 0;
    	//Get samples from infile
    	ullBytesRead = fread(pauMinorFrame, 1, ullNBytesPerMinorFrame, psuInFile);
    	if (ullBytesRead != ullNBytesPerMinorFrame)
    	    {
    	    fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullBytesRead,ullNBytesPerMinorFrame);
    	    break;
    	    }

	//set all samples to -1, reset counters
	for (iChIdx = 0; iChIdx < iNChans; iChIdx++)
	    {
	    ppsuChInfo[iChIdx]->uSample = -1;
	    }
	
	ulMinorFrameSampCount = 0;

	//Determine which minor frame this is
	ullMinorFrameIdx = pauMinorFrame[SFID_CHAN_IDX] & 0b0000011111;  //The TM list counts from 1, not zero
	printf("Minor frame: %" PRIX64 "\n",ullMinorFrameIdx);

	//prepare all the samples, man
	    //	while(ulMinorFrameSampCount < ullSampsPerMinorFrame)
	    //	    {

	    //check the sync words

	    //write this minor frame to the appropriate channel finle
	for (iChIdx = 0; iChIdx < iNChans; iChIdx++)
	    {
	    if ( (ullMinorFrameIdx % ppsuChInfo[iChIdx]->uMinorFrInt) == ( ppsuChInfo[iChIdx]->uMinorFrame % ppsuChInfo[iChIdx]->uMinorFrInt ) )
		{
		int iTmpIdx;
		for (iTmpIdx = ppsuChInfo[iChIdx]->uWord; iTmpIdx < ullSampsPerMinorFrame; iTmpIdx += ppsuChInfo[iChIdx]->uWdInt)
		    {
		    fwrite(&pauMinorFrame[iTmpIdx],2,1,ppsuChInfo[iChIdx]->psuOutFile);
		    ppsuChInfo[iChIdx]->ullSampCount++;
		    }
		}
	    }
	//	    ulMinorFrameSampCount++;


	/* int i = 0; */
	/* for (i = 0; i < ullSampsPerMinorFrame; i++) */
	/*     { */
	/* 	printf("Samp #%i:   0x%" PRIx16 "\n",i,pauMinorFrame[i]); */
	/*     } */

	//did we get the whole major frame?
	if (bVerbose) printf("Major frame #%" PRIu64 "\n", llMajorFrameCount);

	for (ullMinorFrameIdx = 0; ullMinorFrameIdx < llMinorFramesPerMajorFrame; ullMinorFrameIdx++)
	    {
		//		ullBytesWritten += fwrite(&pauMinorFrame[Count],
		//					  1,2,psuOutFile); 
		
		if (bVerbose) 
		    {
		    if (pauIsMinorFrameCollected[ullMinorFrameIdx] == 0)
			printf("Did not collect subframe %" PRIi64 "!",ullMinorFrameIdx);
		    }
	    }

	//write out major frame


    	if (bVerbose) 
    	    {// printf("Read %" PRIu64 " bytes (iteration #%" PRIu64 ")\n",ullInFilePos,ullNSampsRead);
    	    printf("N InSamps read     : %" PRIu64 "\n",ullNSampsRead);
    	    printf("InFile Position (bytes) : %" PRIu64 "\n",ullInFilePos);
    	    printf("Read %" PRIu64 " bytes; Wrote %" PRIu64 " bytes\n",ullBytesRead,ullBytesWritten);
    	    }

	//	vRenewCounters(&llMajorFrameCount,&ulMinorFrameSampCount); 
                
    	ullTotBytesRead += ullBytesRead;
    	ullTotBytesWritten += ullBytesWritten;
	ullMinorFrameCount++;

	//    	fseek(psuInFile, ullMinorFrameCount , SEEK_CUR );
	
    	}  while( ( ullInFilePos = ftell(psuInFile) ) < suInFileStat.st_size  );
    
    //Summary
    printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Total Bytes) \n", ullTotBytesWritten, (suInFileStat.st_size));
    printf("Wrote %" PRIu64 "./%" PRIu64 ". (Bytes / Total Bytes) \n", ullTotBytesWritten, (suInFileStat.st_size));
    // printf("Wrote %" PRIu64 " bytes out of %" PRIu64 " bytes total\n", ullTotBytesRead, (suInFileStat.st_size - llHeaderSkipBytes));
    printf("Output file size is %.2f%% of input file\n", (float)((float)( ullTotBytesWritten )/(float)(suInFileStat.st_size))*100);

    //close files
    fclose(psuInFile);
    fclose(psuOutFile);


    //release the mem!
    for ( iChIdx = 0; iChIdx < iNChans; iChIdx++)
	{
	    if ( ppsuChInfo[iChIdx] != NULL )
		{
		if ( ppsuChInfo[iChIdx]->psuOutFile != NULL )
		    fclose(ppsuChInfo[iChIdx]->psuOutFile);

		free(ppsuChInfo[iChIdx]);
		}
	}
    free(ppsuChInfo);

    return EXIT_SUCCESS;
    }

void vUsage(void)
    {
    printf("\n");
    printf("parse_CAPER_samples\n");
    printf("Convert a filed outputted by bust_nBit_into_16bit_file into separate channel files!\n");
    printf("\n");
    printf("Usage: parse_CAPER_samples <input file> <output file> [flags]   \n");
    printf("                                                                      \n");
    printf("   <filename>   Input/output file names                               \n");
    printf("                                                                      \n");
    printf("   INPUT FILE PARAMETERS                                              \n");
    printf("   -s SIZE      Size of samples                    (in bits)  [%i]    \n",DEF_N_SAMPBITS);
    printf("   -n           Number of samples per minor frame  (optional) [%i]    \n",DEF_SAMPSPERMINORFRAME);
    printf("                                                                      \n");
    printf("                                                                      \n");
    printf("   OPTIONAL PARAMETERS                                                \n");
    printf("   -P           Output file prefix                            [%s]    \n",DEF_OUTPREFIX);
    printf("   -v           Verbose                                       [%i]    \n",DEF_VERBOSE);
    printf("                                                                      \n");
    }

void vPrintSubFrame (uint16_t * pauMajorFrame, int16_t ullMinorFrameIdx)
{ 
    //    for

}

//struct suChanInfo * psuChanInit(int16_t iChIdx)
int iChanInit(struct suChanInfo * psuChInfo, int16_t iChIdx,char * szOutPrefix, uint64_t ullSampsPerMinorFrame)
{
    //    struct suChanInfo * psuChInfo;

    strncpy(   psuChInfo->szName, szStatSerialChanNames[iChIdx] ,DEF_STR_SIZE);    //Name of the channel, e.g., "Langmuir Probe Channel 1 MSB"
    strncpy( psuChInfo->szAbbrev, szStatSerialChanAbbrev[iChIdx],DEF_STR_SIZE);    //Abbreviation for channel, e.g., "LP01MSB"
    strncpy(   psuChInfo->szUser, szStatUser[iChIdx],            DEF_STR_SIZE);    //Who is the user? E.g., Dartmouth, U Iowa
    psuChInfo->uWord            = uStatWord[iChIdx] - 1;			   //Beginning word in the frame, -1 for counting from zero
    psuChInfo->uWdInt           = uStatWdInt[iChIdx];				   //Word interval
    psuChInfo->uSampsPerMinorFrame = ullSampsPerMinorFrame/psuChInfo->uWdInt;      //How many of these to expect per frame? Most are just one.
    psuChInfo->uMinorFrame      = uStatFrame[iChIdx];				   //Which minor frame is it in?
    psuChInfo->uMinorFrInt      = uStatFrInt[iChIdx];				   //How often does it show up?
    psuChInfo->ulSPS            = ulStatSPS[iChIdx];

    psuChInfo->uSample          = -1;
    psuChInfo->ullSampCount     = 0;

    sprintf(psuChInfo->szOutFileName,"%s--%s.out",szOutPrefix,psuChInfo->szAbbrev);
    psuChInfo->psuOutFile       = fopen(psuChInfo->szOutFileName,"wb");

    return 0;
}

void vPrintChanInfo (struct suChanInfo * psuChInfo)
{
    printf("Channel Name           :   %s\n",psuChInfo->szName);
    printf("Abbrev                 :   %s\n",psuChInfo->szAbbrev);
    printf("User                   :   %s\n",psuChInfo->szUser);
    printf("Word                   :   %" PRIu16 "\n",psuChInfo->uWord);
    printf("Word Interval          :   %" PRIu16 "\n",psuChInfo->uWdInt);
    //    printf("N Words in Minor Frame :   %" PRIu16 "\n",psuChInfo->uSampsPerMinorFrame);
    printf("Minor Frame            :   %" PRIu16 "\n",psuChInfo->uMinorFrame);
    printf("Minor Frame Interval   :   %" PRIu16 "\n",psuChInfo->uMinorFrInt);
    printf("Samples per second     :   %" PRIu16 "\n",psuChInfo->ulSPS);

}
