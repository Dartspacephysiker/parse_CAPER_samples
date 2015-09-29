#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "defaults.h"
#include "PCM_and_measurement_structs.h"
#include "ascii_reader.h"

#define  DEBUG            1

#define READ_INT_TO_STRUCT(pattern,pStruct,iField)			\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
        {								\
	int             iTokens;					\
	int             iTemp;						\
									\
	iTokens = sscanf(szDataItem, "%i", &iTemp);			\
	if (iTokens == 1)						\
	    {								\
	    pStruct->iField = atoi(szDataItemTrimmed);			        \
	    if (DEBUG) printf("%s: %i\n",#pattern,pStruct->iField);	\
	    }								\
	else								\
	    assert(0);							\
	}

#define READ_ULL_TO_STRUCT(pattern,pStruct,ullField)			\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
        {								\
	int             iTokens;					\
	uint64_t        ullTemp;					\
									\
	iTokens = sscanf(szDataItem, "%" PRIu64, &ullTemp);		\
	if (iTokens == 1)						\
	    {								\
	    pStruct->ullField = strtoull(szDataItemTrimmed,NULL,10);    \
	    if (DEBUG) printf("%s: %" PRIu64"\n",#pattern,pStruct->ullField); \
	    }								\
	else								\
	    assert(0);							\
	}

#define READ_LL_TO_STRUCT(pattern,pStruct,llField)			\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
        {								\
	int             iTokens;					\
	int64_t         llTemp;						\
									\
	iTokens = sscanf(szDataItem, "%" PRIi64, &llTemp);		\
	if (iTokens == 1)						\
	    {								\
	    pStruct->llField = atoll(szDataItemTrimmed);		\
	    if (DEBUG) printf("%s: %" PRIi64"\n",#pattern,pStruct->llField); \
	    }								\
	else								\
	    assert(0);						\
	}

#define READ_UINT_TO_STRUCT(pattern,pStruct,uField)			\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
        {								\
	int             iTokens;					\
	uint16_t        uTemp;						\
									\
	iTokens = sscanf(szDataItemTrimmed, "%" PRIu16, &uTemp);		\
	if (iTokens == 1)						\
	    {								\
	    pStruct->uField = strtoul(szDataItemTrimmed,NULL,10);	\
	    if (DEBUG) printf("%s: %" PRIu16"\n",#pattern,pStruct->uField); \
	    }								\
	else								\
	    assert(0);							\
	}

#define READ_STR_TO_STRUCT(pattern,pStruct,szField)			\
    else if (strcasecmp(szCodeNameTrimmed, #pattern) == 0)		\
        {								\
	strcpy(pStruct->szField, szDataItemTrimmed);			\
	if (DEBUG) printf("%s: %s\n",#pattern,pStruct->szField);	\
        }


#define READ_UINT_ARR_TO_STRUCT(pattern,pStruct,pauField,nArrElem)	\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
	{								\
	char     *  szSubLine;						\
	char     *  szTrimmed;						\
	int         iCnt;						\
									\
	szSubLine = malloc(bufsize*sizeof(char));			\
	iCnt = 0;							\
									\
	while ( ( getline(&szSubLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 ) \
	    {								\
		if ( isspace(szSubLine[0]) || ( szSubLine[0] == '#' ) )	\
		    continue;						\
		else if ( szSubLine[0] == ';' )				\
		    {							\
		    if ( iCnt == 0 ) /*init to zero*/			\
			{						\
			int iTmp;					\
			for ( iCnt = 0; iCnt < nArrElem; iCnt++ )	\
			    {						\
				pStruct->pauField[iCnt] = 0;		\
			    }						\
			printf("Initialized %s->%s to zeros...\n",#pStruct,#pauField); \
			}						\
		    if (DEBUG) printf("\nLeaving UINT loop w/ count %i...\n\n",iCnt); \
		    break;						\
		    }							\
		else							\
		    {							\
		    szTrimmed = trimwhitespace(szSubLine);		\
		    if (DEBUG) printf("%s, ",szTrimmed);		\
		    pStruct->pauField[iCnt++] = (uint16_t) strtoul(szTrimmed,NULL,10); \
		    }							\
	    }								\
	}

#define READ_ULL_ARR_TO_STRUCT(pattern,pStruct,paullField,nArrElem)	\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
	{								\
	char     *  szSubLine;						\
	char     *  szTrimmed;						\
	int         iCnt;						\
									\
	szSubLine = malloc(bufsize*sizeof(char));			\
	iCnt = 0;							\
									\
	while ( ( getline(&szSubLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 ) \
	    {								\
		if ( isspace(szSubLine[0]) || ( szSubLine[0] == '#' ) )	\
		    continue;						\
		else if ( szSubLine[0] == ';' )				\
		    {							\
		    if (DEBUG) printf("\nLeaving ULL loop w/ count %i...\n\n",iCnt); \
		    break;						\
		    }							\
		else							\
		    {							\
		    szTrimmed = trimwhitespace(szSubLine);		\
		    if (DEBUG) printf("%s, ",szTrimmed);		\
		    pStruct->paullField[iCnt++] = strtoull(szTrimmed,NULL,10); \
		    }							\
	    }								\
	}


#define READ_UL_ARR_TO_STRUCT(pattern,pStruct,paulField,nArrElem)	\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
	{								\
	char     *  szSubLine;						\
	char     *  szTrimmed;						\
	int         iCnt;						\
									\
	szSubLine = malloc(bufsize*sizeof(char));			\
	iCnt = 0;							\
									\
	while ( ( getline(&szSubLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 ) \
	    {								\
		if ( isspace(szSubLine[0]) || ( szSubLine[0] == '#' ) )	\
		    continue;						\
		else if ( szSubLine[0] == ';' )				\
		    {							\
		    if (DEBUG) printf("\nLeaving UL loop w/ count %i...\n\n",iCnt); \
		    break;						\
		    }							\
		else							\
		    {							\
		    szTrimmed = trimwhitespace(szSubLine);		\
		    if (DEBUG) printf("%s, ",szTrimmed);		\
		    pStruct->paulField[iCnt++] = strtoul(szTrimmed,NULL,10); \
		    }							\
	    }								\
	}

#define READ_STR_ARR_TO_STRUCT(pattern,pStruct,pszField,nArrElem)	\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
	{								\
	char     *  szSubLine;						\
	char     *  szTrimmed;						\
	int         iCnt;						\
									\
	szSubLine = malloc(bufsize*sizeof(char));			\
	iCnt = 0;							\
									\
	while ( ( getline(&szSubLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 ) \
	    {								\
		if ( isspace(szSubLine[0]) || ( szSubLine[0] == '#' ) )	\
		    continue;						\
		else if ( szSubLine[0] == ';' )				\
		    {							\
		    if (DEBUG) printf("\nLeaving STR loop w/ count %i...\n\n",iCnt); \
		    break;						\
		    }							\
		else							\
		    {							\
		    szTrimmed = trimwhitespace(szSubLine);		\
		    if (DEBUG) printf("%s, ",szTrimmed);		\
		    strncpy(pStruct->pszField[iCnt++],szTrimmed,strlen(szTrimmed)); \
		    }							\
	    }								\
	}

#define READ_TWO_ELEM_UINT_ARR_TO_STRUCT(pattern,pStruct,pau2Field,nArrElem)	\
    else if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
	{								\
	char     *  szSubLine;						\
	char     *  szTrimmed;						\
	int         iCnt;						\
									\
	char     *  szItem1;						\
	char     *  szItem2;						\
	uint16_t    uItem1;						\
	uint16_t    uItem2;						\
									\
	szSubLine = malloc(bufsize*sizeof(char));			\
	iCnt   = 0;							\
	uItem1 = 0;							\
	uItem2 = 0;							\
									\
	while ( ( getline(&szSubLine,(size_t *)&bufsize,psuPCMConfFile) ) != -1 ) \
	    {								\
	    if ( isspace(szSubLine[0]) || ( szSubLine[0] == '#' ) )	\
		continue;						\
	    else if ( szSubLine[0] == ';' )				\
		{							\
		    if ( iCnt == 0 ) /*init to zero*/			\
			{						\
			int iCnt;					\
			for ( iCnt = 0; iCnt < nArrElem; iCnt++ )	\
			    {						\
				pStruct->pau2Field[iCnt][0] = 0;	\
				pStruct->pau2Field[iCnt][1] = 0;	\
			    }						\
			printf("Initialized %s->%s to zeros...\n",#pStruct,#pau2Field); \
			}						\
		if (DEBUG) printf("\nLeaving UINT loop w/ count %i...\n\n",iCnt); \
		break;							\
		}							\
	    else							\
		{							\
		szItem1 = strtok(szSubLine, ",");			\
		szItem2 = strtok(NULL, "\n");				\
									\
		if ( ( szItem1 != NULL ) && ( szItem2 != NULL ) )	\
		    {							\
		    pStruct->pau2Field[iCnt][0] = strtoul(szItem1,NULL,10); \
		    pStruct->pau2Field[iCnt][1] = strtoul(szItem2,NULL,10); \
		    iCnt++;						\
		    if (DEBUG) printf("szItem1: %s; szItem2: %s\t",szItem1,szItem2); \
		    }							\
		}							\
	    }								\
	}

int iInitPCMFromASCII(char * szPCMConfFile, struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo, 
		   uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode )
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
	    if (DEBUG) printf("**%s\n",szCodeNameTrimmed);

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
    if ( ( bCombineTM1Meas && ( psuMeasInfo->uLSBWord != TM_SKIP_LSB - 1 ) ) && ( psuMeasInfo->uLSBWord != TM_NO_LSB -1 ) )
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

	    printf("%s: Asymm Range is [%u,%u]\n",psuMeasInfo->szName,psuMeasInfo->ppauAsymWRanges[uWRangeIdx][0],psuMeasInfo->ppauAsymWRanges[uWRangeIdx][1]);
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
	sprintf(psuMeasInfo->szTStampFile,"%s--%s--tstamps.txt",szOutPrefix,psuMeasInfo->szAbbrev);
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
    if ( ( ( psuPCMInfo->uTMLink > 1 ) || ( ( psuMeasInfo->uLSBWord  != TM_SKIP_LSB - 1 ) || !bCombineTM1Meas ) ) && !bDoCheckSFIDIncrement )
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
    
    //    if ( psuPCMInfo->pauMFCMeasIdx != NULL )
    //	free( psuPCMInfo->pauMFCMeasIdx );
    
    //    if ( psuPCMInfo->pauGPSMeasIdx != NULL )
    //	free( psuPCMInfo->pauGPSMeasIdx );
}

char * trimwhitespace(char * str)
{
    char *end;
  
    // Trim leading space
    while(isspace(*str)) str++;
    
    if(*str == 0)  // All spaces?
	return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;
    
    // Write new null terminator
    *(end+1) = 0;
    
    return str;
}
