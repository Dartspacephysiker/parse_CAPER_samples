//2015/09/21
//Defaults for parse_PCM_samples.c
#ifndef _PARSE_PCM_DEFS
#define _PARSE_PCM_DEFS

#define DEF_TM_LINK                       1

#define DEF_PCMCONF_FILE          "TM23.tm"
#define DEF_OUTPREFIX  "parsed_TM1-samples"
#define DEF_COMBINE_TM1                   0    //combine TM1 channels on the fly
#define DEF_DO_CHECK_SFID_INCREMENT       0    //Check that SFID increments uniformly
#define DEF_ASSEMBLE_COUNTER              0    //Create and output unique counter produced by major and minor frame counters
#define DEF_CALC_TSTAMPS                  0    //Create and output timestamps based on GPS 1 pps
#define DEF_VERBOSE                       0    //please tell me

#define DEF_STR_SIZE                   1024

#define TM_SKIP_LSB                     499    //Skip this channel if combination is done on the fly
#define TM_NO_LSB                       498    //Don't try to combine this channel

//#define TM_NO_TS_SEARCH                '\0'
#define TM_NO_TS_SEARCH              0x0000

//#define MAX_N_MINORFRAMES               256
//#define MAX_GPS_WORDS                     2


#endif //end #ifndef _PARSE_PCM_DEFS
