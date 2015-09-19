#ifndef _TM23_MAJORFRAME
#define _TM23_MAJORFRAME

#define N_TM23_MEASUREMENTS         7     //Total number of measurementnels in this header

#define TM23_SFID_IDX               1     //Location of subframe ID within minor frame, counting from 1

#define TM23_BPS              9600000     //Link BPS
#define TM23_SAMPSPERMINORFRAME   120
#define TM23_MINOR_PER_MAJOR        4
#define TM23_NUM_MFCOUNTERS         2

#define TM23_WORD_BITLENGTH        16
#define TM23_MINORFRAME_BITSHIFT    2    //up to 4 minor frames

#define TM23_N_GPS_WORDS            1

//Static variable to indicate that there is no timestamp_searchword
#define TM23_NO_TS_SEARCH    '\0'

/*Names of measurements, defined by NASA PCM doc*/
static char   * szTM23SerialMeasNames[]    =    {"Subframe ID                 ", "GPS 1PPS                                   ",
					         "Major Frame Counter 1       ", "Major Frame Counter 2 (!!!NOT SYMMETRIC!!!)",
					         "Frame Sync 1                ", "Frame Sync 2                               ",
					         "RX‐DSP (!!!NOT SYMMETRIC!!!)"};
					   
/*Abbreviation of measurements for outputting files*/
static char   * szTM23SerialMeasAbbrev[]   =    {"SFID",         "GPS",
					         "MF1",          "MF2",
					         "FS1",	         "FS2",
					         "Rx-DSP"};
					   
/*User of measurement, as defined by NASA PCM doc*/		   
static char   * szTM23User[]               =    {"TM",        "TM",
					         "TM",        "TM",
					         "TM",        "TM",
                                                 "Dartmouth"};
					   
/*Word number within minor frame, counting from 1.*/
/* If the word is 0, it means there is a range of words in the minor frame that contain the relevant measurement*/
static uint16_t         uTM23Word[]        =    {  1, 201,
					         200, 201,
						 399, 400,
                                                   0};
					   
/*Word interval within a minor frame*/
/*When the interval is the size of the minor frame, the word only occurs once within the minor frame containing it*/
static uint16_t         uTM23WdInt[]       =    {400, 400,
					         400, 400,
					         400, 400};
/*First frame containing each word*/
static uint16_t        uTM23Frame[]        =    {1, 4,  
					         1, 1,
					         1, 1,
                                                 1};
/*Interval of frames containing each word*/
static uint16_t        uTM23FrInt[]        =    {1, 4,  
					         1, 1,
					         1, 1,
                                                 1};

/*Sampling rate of each measurement*/
static uint32_t         ulTM23SPS[]        =    {1500,   375,								                      
					         1500,  1125,
					         1500,  1500,
                                               592500};

/*Number of asym word ranges in minor frame for each measurement*/
/*A range of zero means the measurement corresponds to a single word*/
static uint16_t     uTM23NAsymWRanges[]    =    {0, 0,
						 0, 0,
						 0, 0,
                                                 2};
/*Specification of the asym word ranges w/in a minor frame, inclusive*/
static uint16_t     uTM23AsymWRanges[][2]  =    {{  2, 199},   //Just for RxDSP words, inclusive of words
					        {202, 398}};

/*Number of asym frame ranges in minor frame for this measurement*/
static uint16_t     uTM23NAsymFRanges[]    = {0, 0,
					      0, 1,
					      0, 0,
					      0};

/*Specification of the frame ranges w/in a minor frame, inclusive*/
static uint16_t     uTM23AsymFRanges[][2]  = {{  1, 3}};   //Just for MF2 words, inclusive of words

static uint16_t        uTM23LSBWord[]   =    {  0, 0,
					        0, 0,
					        0, 0,
						0};

static uint8_t abTM23TSCalcEnabled[]    =    {  0, 0,
					        0, 0,
					        0, 0,
						0};

static char   * szTM23TSSearchWords[]   =    { TM23_NO_TS_SEARCH, TM23_NO_TS_SEARCH,
					       TM23_NO_TS_SEARCH, TM23_NO_TS_SEARCH, 
					       TM23_NO_TS_SEARCH, TM23_NO_TS_SEARCH, 
                                               "aDtr"};

static uint16_t    auTM23MFCMeasIdx[]   =    {  2, 3};    ///Major frame measurement indices (indexing from zero in the arrays above)

static uint16_t    auTM23GPSMeasIdx[]   =    {  1}; //GPS 1pps measurement indices (indexing from zero in the arrays above)

#endif // #ifndef _TM23_MAJORFRAME

