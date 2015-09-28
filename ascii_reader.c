#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "defaults.h"
#include "PCM_and_measurement_structs.h"
#include "ascii_reader.h"

#define  POUND_SIGN       0x23
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

#DEFINE READ_ULL_TO_STRUCT(PATTERN,PSTRUCT,ULLFIELD)			\
    ELSe if ( strncasecmp(szCodeNameTrimmed, #pattern, strlen(#pattern)) == 0 ) \
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


#define READ_UINT_ARR_TO_STRUCT(pattern,pStruct,pauField,nArrElem)

#define READ_ULL_ARR_TO_STRUCT(pattern,pStruct,paullField,nArrElem)

#define READ_UL_ARR_TO_STRUCT(pattern,pStruct,paulField,nArrElem)

#define READ_STR_ARR_TO_STRUCT(pattern,pStruct,pszField,nArrElem)



int iPCMInitNUMTWO(struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo, 
		   uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode )
{
    int iArgIdx;

    char fname[] = "TM23.tm";
    FILE * psuFile;
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
        perror("Unable to allocate buffer");
        exit(1);
    }

    psuFile = (FILE *) fopen(fname,"r");
    if (psuFile == NULL)
	{
	fprintf(stderr, "Error opening input file\n");
	return EXIT_FAILURE;
	}

    //get line from file
    while( ( getline(&szLine,(size_t *)&bufsize,psuFile) ) != -1 )
    {
	// If errors reading line then skip over it
	// Also skip comments, which are marked with "#"
	szCodeName = strtok(szLine, ":");
	szDataItem = strtok(NULL, "\n");

	if ( (szCodeName != NULL) && (szDataItem != NULL) && (szCodeName[0] != POUND_SIGN) ) //One-line value
	    {
	    char * szCodeNameTrimmed;
	    char * szDataItemTrimmed;

	    szCodeNameTrimmed = trimwhitespace(szCodeName);
	    szDataItemTrimmed = trimwhitespace(szDataItem);

	    //	    printf("SzCNTrimmed[%i]:szDataItemTrimmed[%i] --- %s : %s\n",(int)strlen(szCodeNameTrimmed),(int)strlen(szDataItemTrimmed),
	    //szCodeNameTrimmed,szDataItemTrimmed);
	    if(0) {}
	    READ_STR_TO_STRUCT(NAME,psuPCMInfo,szName)
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
	else if ( (szDataItem == NULL) && (szCodeName[0] != POUND_SIGN) && !isspace(szCodeName[0]) )
	    {
	    char * szCodeNameTrimmed;
	    int  iMatchIdx;
	    int  iMatch;

	    iMatchIdx = 0;
	    iMatch = -1;
	    
	    szCodeNameTrimmed = trimwhitespace(szCodeName);		

	    for (iMatchIdx = 0; iMatchIdx < N_PCM_MEAS_FIELDS; iMatchIdx++)
		{
		if ( strncasecmp(szCodeNameTrimmed,szPCMMeasConfNames[iMatchIdx],strlen(szPCMMeasConfNames[iMatchIdx])) == 0 )
		    {
		    iMatch = iMatchIdx;
		    if (DEBUG) printf("Matched %s...\n",szPCMMeasConfNames[iMatch]);

		    break;
		    }
		}

	    //	    if(0) {}
	    //	    READ_STR_ARR_TO_PPSTRUCT(ppsuPCMInfo,iNMeasurements



	    //	szDataItem = strtok(NULL, "\n");
	    //	    READ_INT
	//	printf("szDataItem:%c\n",szCodeName[0]);
	    
	}
	else
	    continue;

	iLineIdx++;
    }


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
    rewind(psuFile);
    while( ( getline(&szLine,(size_t *)&bufsize,psuFile) ) != -1 )
    {

	// If errors reading line then skip over it
	// Also skip comments, which are marked with "#"
	szCodeName = strtok(szLine, ":");
	szDataItem = strtok(NULL, "\n");

	if ( (szCodeName != NULL) && (szDataItem == NULL) && (szCodeName[0] != POUND_SIGN) && !isspace(szCodeName[0]) ) //One-line value
	    {
	    char * szCodeNameTrimmed;
	    /* int  iMatchIdx; */
	    /* int  iMatch; */

	    szCodeNameTrimmed = trimwhitespace(szCodeName);

	    /* iMatchIdx = 0; */
	    /* iMatch = -1; */

	    /* for (iMatchIdx = 0; iMatchIdx < N_PCM_MEAS_FIELDS; iMatchIdx++) */
	    /* 	{ */
	    /* 	if ( strncasecmp(szCodeNameTrimmed,szPCMMeasConfNames[iMatchIdx],strlen(szPCMMeasConfNames[iMatchIdx])) == 0 ) */
	    /* 	    { */
	    /* 	    iMatch = iMatchIdx; */
	    /* 	    if (DEBUG) printf("Matched %s...\n",szPCMMeasConfNames[iMatch]); */

	    /* 	    break; */
	    /* 	    } */
	    /* 	} */

	    if(0) {}
	    READ_STR_ARR_TO_STRUCT(MEAS_NAME,psuPCMInfo,pszSerialMeasNames,nMeas)
	    READ_STR_ARR_TO_STRUCT(MEAS_ABBREV,psuPCMInfo,pszSerialMeasAbbrev,nMeas)
	    READ_STR_ARR_TO_STRUCT(MEAS_USER,psuPCMInfo,pszUser,nMeas)

	    READ_UINT_ARR_TO_STRUCT(MEAS_SAMPLE_LOC_IN_MINFRAME,psuPCMInfo,pauWord,nMeas)
	    READ_UINT_ARR_TO_STRUCT(MEAS_SAMPLE_INTERVAL_IN_MINFRAME,psuPCMInfo,pauWdInt,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_SAMPLE_FRAME,psuPCMInfo,pauFrame,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_FRAME_INTERVAL,psuPCMInfo,pauFrInt,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_N_ASYM_WORD_RANGES,psuPCMInfo,pauNAsymWRanges,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_ASYM_WORD_RANGES,psuPCMInfo,pauAsymWRanges,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_N_ASYM_FRAME_RANGES,psuPCMInfo,pauNAsymFRanges,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_ASYM_FRAME_RANGES,psuPCMInfo,pauAsymFRanges,nMeas)
            READ_UINT_ARR_TO_STRUCT(MEAS_LSB_WORD,psuPCMInfo,pauLSBWord,nMeas)

            READ_UL_ARR_TO_STRUCT(MEAS_SAMPLE_RATE,psuPCMInfo,paulSPS,nMeas)

            READ_UINT_ARR_TO_STRUCT(MEAS_TSTAMP_CALC_TYPE,psuPCMInfo,pauTSCalcType,nMeas)
            READ_STR_ARR_TO_STRUCT(MEAS_TSTAMP_SEARCH_WORD,psuPCMInfo,pszTSSearchWords,nMeas)
            READ_ULL_ARR_TO_STRUCT(MEAS_INTERN_SAMP_RATE,psuPCMInfo,pullInternWordRate,nMeas)
            READ_UINT_ARR_TO_STRUCT(MAJFRAME_INDEX,psuPCMInfo,pauMFCMeasIdx,nMeas)
            READ_UINT_ARR_TO_STRUCT(GPS_INDEX,psuPCMInfo,pauGPSMeasIdx,nMeas) //This one is questionable

	    }
	}
	    
    }

    return EXIT_SUCCESS;
}

void vInitPCMASCIIArrays(struct suPCMInfo * psuPCMInfo)
{

    int nMeas;
    int iMeasIdx;

    nMeas = psuPCMInfo->iNMeasurements;

    psuPCMInfo->pszSerialMeasNames  = (char **)    malloc(nMeas*sizeof(char *));
    psuPCMInfo->pszSerialMeasAbbrev = (char **)    malloc(nMeas*sizeof(char *));
    psuPCMInfo->pszUser             = (char **)    malloc(nMeas*sizeof(char *));

    psuPCMInfo->pauWord             = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauWdInt            = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauFrame            = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauFrInt            = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->paulSPS             = (uint32_t *) malloc(nMeas*sizeof(uint32_t));
    psuPCMInfo->pauNAsymWRanges     = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauAsymWRanges      = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauNAsymFRanges     = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauAsymFRanges      = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauLSBWord          = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    psuPCMInfo->pauTSCalcType       = (uint16_t *) malloc(nMeas*sizeof(uint16_t));

    psuPCMInfo->pszTSSearchWords    = (char **)    malloc(nMeas*sizeof(char *));

    for( iMeasIdx = 0; iMeasIdx < nMeas; iMeasIdx++ )
	{
	psuPCMInfo->pszSerialMeasNames[iMeasIdx]       = (char *) malloc(DEF_STR_SIZE*sizeof(char));
	psuPCMInfo->pszSerialMeasAbbrev[iMeasIdx]      = (char *) malloc(DEF_STR_SIZE*sizeof(char));
	psuPCMInfo->pszSerialMeasNames[iMeasIdx]       = (char *) malloc(DEF_STR_SIZE*sizeof(char));
	psuPCMInfo->pszTSSearchWords[iMeasIdx]         = (char *) malloc(DEF_STR_SIZE*sizeof(char));
	}

    psuPCMInfo->pullInternWordRate  = (uint64_t *) malloc(nMeas*sizeof(uint64_t));
    
    if ( psuPCMInfo->uNumMFCounters > 0 )
	psuPCMInfo->pauMFCMeasIdx       = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    else
	psuPCMInfo->pauMFCMeasIdx       = NULL;

    if ( psuPCMInfo->uNGPSWordsInPCM > 0 )
	psuPCMInfo->pauGPSMeasIdx       = (uint16_t *) malloc(nMeas*sizeof(uint16_t));
    else
	psuPCMInfo->pauGPSMeasIdx       = NULL;
}

void vFreePCMASCIIArrays(struct suPCMInfo * psuPCMInfo)
{
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
    if ( psuPCMInfo->pauAsymWRanges != NULL )
	free( psuPCMInfo->pauAsymWRanges );
    if ( psuPCMInfo->pauNAsymFRanges != NULL )
	free( psuPCMInfo->pauNAsymFRanges );
    if ( psuPCMInfo->pauAsymFRanges != NULL )
	free( psuPCMInfo->pauAsymFRanges );
    if ( psuPCMInfo->pauLSBWord != NULL )
	free( psuPCMInfo->pauLSBWord );
    if ( psuPCMInfo->pauTSCalcType != NULL )
	free( psuPCMInfo->pauTSCalcType );
    
    if ( psuPCMInfo->pszTSSearchWords != NULL )
	free( psuPCMInfo->pszTSSearchWords );
    
    if ( psuPCMInfo->pullInternWordRate != NULL )
	free( psuPCMInfo->pullInternWordRate );
    
    if ( psuPCMInfo->pauMFCMeasIdx != NULL )
	free( psuPCMInfo->pauMFCMeasIdx );
    
    if ( psuPCMInfo->pauGPSMeasIdx != NULL )
	free( psuPCMInfo->pauGPSMeasIdx );
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


//int main( struct suPCMInfo * psuPCMInfo, struct suMeasurementInfo ** ppsuMeasInfo )
/* int main( int argc, char * argv[] ) */
/* { */

/*     char fname[] = "TM23.tm"; */
/*     FILE * psuFile; */
/*     char              *  szLine; */
/*     int                  iLineIdx; */
/*     int                  bufsize; */
/*     char              *  szCodeName; */
/*     char              *  szDataItem; */

/*     bufsize=512; */

/*     szLine = (char * )malloc(bufsize*sizeof(char)); */

/*     if( szLine == NULL) */
/*     { */
/*         perror("Unable to allocate buffer"); */
/*         exit(1); */
/*     } */
/*     iLineIdx = 0; */

/*     psuFile = (FILE *) fopen(fname,"r"); */
/*     if (psuFile == NULL) */
/* 	{ */
/* 	fprintf(stderr, "Error opening input file\n"); */
/* 	return EXIT_FAILURE; */
/* 	} */

/*     //get line from file */
/*     while( ( getline(&szLine,(size_t *)&bufsize,psuFile) ) != -1 ) */
/*     { */
/* 	// If errors reading line then skip over it */
/* 	// Also skip comments, which are marked with "#" */
/* 	//	if ((szCodeName == NULL) || (szDataItem == NULL) || (szCodeName[0] == "#") ) */
/* 	//	printf("%s\n",szLine); */
/* 	szCodeName = strtok(szLine, ":"); */
/* 	szDataItem = strtok(NULL, "\n"); */

/* 	//	if ( (szLine[0] == NULL) || (isspace(szLine[0])) || (szLine[0] == "#") ) */
/* 	if ( (szCodeName == NULL) || (szDataItem == NULL) || (szCodeName == "#") ) */
/* 	    continue; */
/* 	else { */
/* 	//	vParseLine(szLine */
/* 	    //	szDataItem = strtok(NULL, "\n"); */
/* 	    printf("SzCodeName:szDataItem --- %s : %s\n",szCodeName,szDataItem); */

/* 	    if(0) {} */
/* 	    READ_INT */
/* 	//	printf("szDataItem:%c\n",szCodeName[0]); */
/* 	} */
/* 	//	sscanf(szLine,"%s:%s",szCodeName, */

/* 	// Read things */
/* 	/\* if(bFALSE) {} *\/ */

/* 	/\* READ_INT(D2, szBitsPerSec, ulBitsPerSec)                    // D2 - Bit Rate *\/ */
/* 	/\* READ_INT(F1, szCommonWordLen, ulCommonWordLen)              // F1 - Common Word Length *\/ */
/* 	/\* READ_INT(F2, szWordTransferOrder, ulWordTransferOrder)      // F2 - MSB / LSB first *\/ */
/* 	/\* READ_INT(F3, szParityType, ulParityType)                    // F3 - Even, odd, none *\/ */
/* 	/\* READ_INT(F4, szParityTransferOrder, ulParityTransferOrder)  // F4 - Leading / Trailing *\/ */
/* 	/\* else if (strcasecmp(szCodeField, "MF") == 0) *\/ */
/*         /\* { *\/ */
/*         /\* szCodeField = strtok(NULL, "\\"); *\/ */
/*         /\* if (bFALSE) {}                          // Keep macro logic happy *\/ */
/* 	/\* //        READ_P(N, szNumMinorFrames)           // MF\N - Number of minor frames *\/ */
/*         /\* READ_P(N, ulNumMinorFrames)           // MF\N - Number of minor frames *\/ */
/*         /\* } // end if MF *\/ */

/* 	/\* READ_P(MF1, ulWordsInMinorFrame)          // MF1 - Number of word in minor frame *\/ */
/* 	/\* READ_P(MF2, ulBitsInMinorFrame)           // MF2 - Number of bits in minor frame *\/ */
/*         /\* READ_P(MF3, ulMinorFrameSyncType)         // MF3 - Minor Frame Sync Type *\/ */
/* 	/\* READ_P(MF4, ulMinorFrameSyncPatLen)       // MF4 - Minor frame sync pattern length *\/ */
/*         /\* READ_P(MF5, ullMinorFrameSyncPat)          // MF5 - Minor frame sync pattern *\/ */

/* 	iLineIdx++; */
/*     } */

/*     return EXIT_SUCCESS; */
/* } */
