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

#define DEF_N_SAMPBITS           10 //in bits
#define MAX_N_MINORFRAMES       256
#define DEF_SAMPSPERMINORFRAME       120
#define DEF_VERBOSE               0 //please tell me
#define DEF_SWAPBYTEORDER     1

//struct declarations
struct suChanInfo
{
    char        szName[256];          //Name of the channel, e.g., "Langmuir Probe Channel 1 MSB"
    char        szAbbrev[256];        //Abbreviation for channel, e.g., "LP01MSB"
    char        szUser[16];	      //Who is the user? E.g., Dartmouth, U Iowa
    uint16_t    uWord;		      //Beginning word in the frame
    uint16_t    uWdInt;		      //Word interval
    uint16_t    uNumWordsInFrame;     //How many of these to expect per frame? Most are just one.
    uint16_t    uMinorFrame;          //Which minor frame is it in?
    uint16_t    uMinorFrInt;          //How often does it show up?
    uint32_t    ulSPS;
};

//function declarations
void vUsage(void);
void vPrintChanInfo (struct suChanInfo * psuChInfo);
void vPrintSubFrame (uint16_t * pauMajorFrame, int16_t iMinorFrameIdx);

struct suChanInfo * psuChanInit(int16_t iChIdx);

int main( int argc, char * argv[] )
    {
    //************
    //Declare vars

    char         szInFile[512];		       //input/output file stuff
    char         szOutFile[512];

    FILE       * psuInFile;
    FILE       * psuOutFile;

    struct stat  suInFileStat;		       //input stuff
    uint64_t     ulSampBitLength;	       //Sample size (in bytes)
    uint64_t     ullSampsPerMinorFrame;	       //Number of samples to grab at each periodic interval
    uint64_t     ullNBytesPerMinorFrame;	       //Number of bits in MinorFrame

    uint64_t     ullNBytesPerOutBuf;
    uint16_t   * pauMinorFrame;

    struct suChanInfo ** ppsuChInfo;
    int16_t      iChIdx;
    int16_t      iNChans;

    int16_t      iMinorFrameIdx;
    int16_t      iMinorFramesPerMajorFrame;
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

    uint64_t     ullSampMask;
    uint32_t     ulSampBitCount;

    uint32_t     ulMinorFrameBitCount;
    uint32_t     ulMinorFrameSampCount;
    uint64_t     ullMinorFrameCount;
    int32_t      lSaveData;

    int          iArgIdx;

    uint8_t      bVerbose;

    //*******************
    //Initialize vars
    ulSampBitLength        = DEF_N_SAMPBITS;
    ullSampsPerMinorFrame  = DEF_SAMPSPERMINORFRAME;

    ullInFilePos           = 0;
    ullBytesRead            = 0;
    ullBytesWritten         = 0;
    ullNSampsRead          = 0;
    ullTotBytesRead         = 0;
    ullTotBytesWritten      = 0;

    iChIdx                  = 0;
    iNChans                 = N_TM1_CHANS;

    ullSampMask            = 0;
    ulSampBitCount         = 0;

    ulMinorFrameBitCount    = 0;
    ulMinorFrameSampCount   = 0;
    ullMinorFrameCount      = 0;
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

    for (iArgIdx=1; iArgIdx<argc; iArgIdx++)   //start with 1 to skip input file
	{

	switch (argv[iArgIdx][0])
	    {
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

    if ( ulSampBitLength < 1 || ulSampBitLength > 15 )
	{
	fprintf(stderr,"Invalid sample size provided! Exiting...\n");
	return EXIT_FAILURE;
	}
    printf(" \n");

    //init chans
    ppsuChInfo = (struct suChanInfo**) malloc(sizeof(struct suChanInfo *) * iNChans);
    for ( iChIdx = 0; iChIdx < iNChans; iChIdx++)
	{
	    ppsuChInfo[iChIdx] = psuChanInit(iChIdx);
	    if (bVerbose) vPrintChanInfo(ppsuChInfo[iChIdx]);
	}

    ullBytesInMajorFrame = iMinorFramesPerMajorFrame* sizeof(uint16_t *);

    //initialize major frame, binary array for keeping track of
    pauIsMinorFrameCollected = calloc(iMinorFramesPerMajorFrame,2);
    ppauMajorFrame = malloc(ullBytesInMajorFrame);
    temp = malloc(iMinorFramesPerMajorFrame * ullSampsPerMinorFrame * sizeof(uint16_t));
    iMinorFrameIdx;
    for (iMinorFrameIdx = 0; iMinorFrameIdx < iMinorFramesPerMajorFrame; iMinorFrameIdx++) 
	{
	ppauMajorFrame[iMinorFrameIdx] = temp + (iMinorFrameIdx * ullSampsPerMinorFrame);
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

	//prepare all the samples, man
	while(ulMinorFrameBitCount < ullNBytesPerMinorFrame)
	    {

	    // set all samples to -1 so we know whether we got data

	    //locate which minor frame this is

	    //check the sync words

	    //write this minor frame to the appropriate subframe
	    // Collect all minor frames in this major frame
	    if(ulMinorFrameSampCount == ullSampsPerMinorFrame)
		{
		if(ulMinorFrameBitCount != ullNBytesPerMinorFrame) //should have all data here
		    {
		    fprintf(stderr,"Something's wrong. How did you get a good bit count and a bad samp count?\n");
		    fprintf(stderr,"MinorFrame Bit  Count: %" PRIu32 "\n",ulMinorFrameBitCount);
		    fprintf(stderr,"MinorFrame Samp Count: %" PRIu32 "\n",ulMinorFrameSampCount);
		    //		    return;
		    }
		int Count;
		uint16_t tempSamp;
		for (Count = 0; Count < ullSampsPerMinorFrame; Count++)
		    {
			tempSamp = (uint16_t)(pauMinorFrame[Count]);
		    ullBytesWritten += fwrite(&pauMinorFrame[Count],
					     1,2,psuOutFile); 
		    }
		}
	    
	    }

	//did we get the whole major frame?
	if (bVerbose) printf("Major frame #%" PRIu64 "\n", llMajorFrameCount);
	for (iMinorFrameIdx = 0; iMinorFrameIdx < iMinorFramesPerMajorFrame; iMinorFrameIdx++)
	    {
		ullBytesWritten += fwrite(&pauMinorFrame[Count],
					  1,2,psuOutFile); 

		
		if (bVerbose) 
		    {
		    if (pauIsMinorFrameCollected[iMinorFrameIdx] == 0)
			printf("Did not collect subframe %" PRIu16 "!",iMinorFrameIdx);
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

    return EXIT_SUCCESS;
    }

void vUsage(void)
    {
    printf("\nbust_nBit_into_16bit_binary_samples\n");
    printf("Convert all that nasty N-bit data into nice, 16-bit samples\n");
    printf("\n");
    printf("Usage: bust_nBit_into_16bit_binary_samples <input file> <output file> [flags]   \n");
    printf("                                                                      \n");
    printf("   <filename>   Input/output file names                               \n");
    printf("                                                                      \n");
    printf("   INPUT FILE PARAMETERS                                              \n");
    printf("   -s SIZE      Size of samples                    (in bits)  [%i]    \n",DEF_N_SAMPBITS);
    printf("   -n           Number of samples per minor frame  (optional) [%i]    \n",DEF_SAMPSPERMINORFRAME);
    printf("   -S           Turn off byte swapping                                \n");
    printf("                                                                      \n");
    printf("                                                                      \n");
    printf("   OPTIONAL PARAMETERS                                                \n");
    printf("   -v           Verbose                                       [%i]    \n",DEF_VERBOSE);
    printf("                                                                      \n");
    }

void vPrintSubFrame (uint16_t * pauMajorFrame, int16_t iMinorFrameIdx)
{ 
    //    for

}

struct suChanInfo * psuChanInit(int16_t iChIdx)
{
    struct suChanInfo * psuChInfo;

    psuChInfo = malloc(sizeof(struct suChanInfo));

    strncpy(   psuChInfo->szName, szStatSerialChanNames[iChIdx] ,256);    //Name of the channel, e.g., "Langmuir Probe Channel 1 MSB"
    strncpy( psuChInfo->szAbbrev, szStatSerialChanAbbrev[iChIdx],256);    //Abbreviation for channel, e.g., "LP01MSB"
    strncpy(   psuChInfo->szUser, szStatUser[iChIdx],            256);    //Who is the user? E.g., Dartmouth, U Iowa
    psuChInfo->uWord            = uStatWord[iChIdx];			    //Beginning word in the frame
    psuChInfo->uWdInt           = uStatWdInt[iChIdx];			    //Word interval
    psuChInfo->uNumWordsInFrame = 0;					    //How many of these to expect per frame? Most are just one.
    psuChInfo->uMinorFrame      = uStatFrame[iChIdx];			    //Which minor frame is it in?
    psuChInfo->uMinorFrInt      = uStatFrInt[iChIdx];			    //How often does it show up?
    psuChInfo->ulSPS            = ulStatSPS[iChIdx];

    return psuChInfo;
}

void vPrintChanInfo (struct suChanInfo * psuChInfo)
{
    printf("Channel Name           :   %s\n",psuChInfo->szName);
    printf("Abbrev                 :   %s\n",psuChInfo->szAbbrev);
    printf("User                   :   %s\n",psuChInfo->szUser);
    printf("Word                   :   %" PRIu16 "\n",psuChInfo->uWord);
    printf("Word Interval          :   %" PRIu16 "\n",psuChInfo->uWdInt);
    //    printf("N Words in Minor Frame :   %" PRIu16 "\n",psuChInfo->uNumWordsInFrame);
    printf("Minor Frame            :   %" PRIu16 "\n",psuChInfo->uMinorFrame);
    printf("Minor Frame Interval   :   %" PRIu16 "\n",psuChInfo->uMinorFrInt);
    printf("Samples per second     :   %" PRIu16 "\n",psuChInfo->ulSPS);

}
