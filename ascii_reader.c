#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "defaults.h"
#include "PCM_and_measurement_structs.h"
#include "ascii_reader.h"

#define READ_INT_TO_STRUCT(pattern,pStruct,ifield)			\
    else if ( strncasecmp(szCodeName, #pattern":", strlen(#pattern)+1) == 0 ) \
        {								\
	int                 iTokens;					\
	int             iTemp;						\
	iTokens = sscanf(szDataItem, "%i", &iTemp);			\
	if (iTokens == 1)						\
	    {								\
	    pStruct->ifield = atoi(szDataItem);			\
	    }								\
	else								\
	    assert(0);						\
	}

#define READ_STR_TO_STRUCT(pattern,pStruct,szField)			\
    else if (strcasecmp(szCodeName, #pattern) == 0)			\
        {								\
	strcpy(pStruct->szField, szDataItem);				\
        }

int iPCMInitNUMTWO(struct suPCMInfo * psuPCMInfo, uint8_t bCombineTM1Meas, uint8_t bDoCheckSFIDIncrement, uint8_t bTStampMode )
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
    psuPCMInfo->uTMLink                        = 0;;

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

    //    psuPCMInfo->bDoSearchFrameForWord          = 0;

    bufsize=DEF_STR_SIZE;

    szLine = (char * )malloc(bufsize*sizeof(char));

    if( szLine == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    iLineIdx = 0;

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
	//	if ((szCodeName == NULL) || (szDataItem == NULL) || (szCodeName[0] == "#") )
	//	printf("%s\n",szLine);
	szCodeName = strtok(szLine, ":");
	szDataItem = strtok(NULL, "\n");

	if ( (szCodeName != NULL) && (szDataItem != NULL) && (szCodeName != "#") ) //One-line value
	    {
	    char * szCodeNameTrimmed;
	    char * szDataItemTrimmed;

	    szCodeNameTrimmed = trimwhitespace(szCodeName);
	    szDataItemTrimmed = trimwhitespace(szDataItem);

	    printf("SzCodeName:szDataItem --- %s : %s\n",szCodeNameTrimmed,szDataItem);
	    if(0) {}
	    READ_STR_TO_STRUCT(NAME,psuPCMInfo,szName)
	    READ_INT_TO_STRUCT(N_MEASUREMENTS,psuPCMInfo,iNMeasurements)
	    READ_INT_TO_STRUCT(SFID_IDX,psuPCMInfo,uSFIDIdx)
	    /* READ_ULL_VAL(BPS,ullBitRate) */
	    /* READ_UINT_VAL(SAMPLE_BITLENGTH,uSampBitLength) */
	    /* READ_ULL_VAL(N_SAMPLES_PER_MINOR,ullSampsPerMinorFrame) */
	    /* READ_LL_VAL(N_MINOR_PER_MAJOR,llMinorFramesPerMajorFrame) */
	    /* READ_UINT_VAL(N_MAJFRAMECOUNTERS,uNumMFCounters) */
	    /* READ_UINT_VAL(N_MINFRAME_BITPOS,uMinorFrameBitShift) */
	    /* READ_UINT_VAL(N_GPS_WORDS,uNGPSWordsInPCM) */

	    }
	else if ( (szDataItem == NULL) && (szCodeName != "#") )
	    {
	    printf("SzCodeName: %s\n",szCodeName);
	    
	    //	szDataItem = strtok(NULL, "\n");
	    //	    READ_INT
	//	printf("szDataItem:%c\n",szCodeName[0]);
	    
	}
	else
	    continue;

	//	sscanf(szLine,"%s:%s",szCodeName,

	// Read things
	/* if(bFALSE) {} */

	/* READ_INT(D2, szBitsPerSec, ulBitsPerSec)                    // D2 - Bit Rate */
	/* READ_INT(F1, szCommonWordLen, ulCommonWordLen)              // F1 - Common Word Length */
	/* READ_INT(F2, szWordTransferOrder, ulWordTransferOrder)      // F2 - MSB / LSB first */
	/* READ_INT(F3, szParityType, ulParityType)                    // F3 - Even, odd, none */
	/* READ_INT(F4, szParityTransferOrder, ulParityTransferOrder)  // F4 - Leading / Trailing */
	/* else if (strcasecmp(szCodeField, "MF") == 0) */
        /* { */
        /* szCodeField = strtok(NULL, "\\"); */
        /* if (bFALSE) {}                          // Keep macro logic happy */
	/* //        READ_P(N, szNumMinorFrames)           // MF\N - Number of minor frames */
        /* READ_P(N, ulNumMinorFrames)           // MF\N - Number of minor frames */
        /* } // end if MF */

	/* READ_P(MF1, ulWordsInMinorFrame)          // MF1 - Number of word in minor frame */
	/* READ_P(MF2, ulBitsInMinorFrame)           // MF2 - Number of bits in minor frame */
        /* READ_P(MF3, ulMinorFrameSyncType)         // MF3 - Minor Frame Sync Type */
	/* READ_P(MF4, ulMinorFrameSyncPatLen)       // MF4 - Minor frame sync pattern length */
        /* READ_P(MF5, ullMinorFrameSyncPat)          // MF5 - Minor frame sync pattern */

	iLineIdx++;
    }

    return EXIT_SUCCESS;
}





// Read an R record and convert the result to a boolean
#define READ_P_BOOL(pattern, field, bfield, truechar)                         \
    else if (strcasecmp(szCodeField, #pattern) == 0)                            \
        {                                                                       \
        psuRRec->field = (char *)TmatsMalloc(strlen(szDataItem)+1);           \
        strcpy(psuRRec->field, szDataItem);                                   \
        psuRRec->bfield =                                                     \
            toupper(*szFirstNonWhitespace(szDataItem)) == truechar;             \
        } /* end if pattern found */


// Read an R Data Source record and convert to a boolean
#define READ_R_DS_BOOL(pattern, field, bfield, truechar)                      \
    else if (strncasecmp(szCodeField, #pattern"-", strlen(#pattern)+1) == 0)    \
        {                                                                       \
        int                 iTokens;                                            \
        int                 iDSIIndex;                                          \
        char                szFormat[20];                                       \
        SuRDataSource     * psuDataSource;                                      \
        sprintf(szFormat, "%%*%zuc-%%i", strlen(#pattern));                      \
        iTokens = sscanf(szCodeField, szFormat, &iDSIIndex);                    \
        if (iTokens == 1)                                                       \
            {                                                                   \
            psuDataSource = psuGetRDataSource(psuRRec, iDSIIndex, bTRUE);       \
            assert(psuDataSource != NULL);                                      \
            psuDataSource->field = (char *)TmatsMalloc(strlen(szDataItem)+1); \
            strcpy(psuDataSource->field, szDataItem);                         \
            psuDataSource->bfield =                                           \
                toupper(*szFirstNonWhitespace(szDataItem)) == truechar;         \
            } /* end if sscanf OK */                                            \
        else                                                                    \
            assert(bFALSE);                                                     \
        } /* end if pattern found */


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
