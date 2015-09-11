#ifndef _TM23_MAJORFRAME
#define _TM23_MAJORFRAME

//Total number of channels in this header
#define N_TM23_CHANS   7

//Location of subframe ID within minor frame, counting from 1
#define SFID_CHAN_IDX 1

/*Names of measurements, defined by NASA PCM doc*/
static char   * szStatSerialChanNames[]    =    {"Subframe ID                 ", "GPS 1PPS                                   ",
					         "Major Frame Counter 1       ", "Major Frame Counter 2 (!!!NOT SYMMETRIC!!!)",
					         "Frame Sync 1                ", "Frame Sync 2                               ",
					         "RX‐DSP (!!!NOT SYMMETRIC!!!)"};
					   
/*Abbreviation of measurements for outputting files*/
static char   * szStatSerialChanAbbrev[]   =    {"SFID",         "GPS",
					         "MF1",          "MF2",
					         "FS1",	         "FS2",
					         "Rx-DSP"};
					   
/*User of measurement, as defined by NASA PCM doc*/		   
static char   * szStatUser[]               =    {"TM",        "TM",
					         "TM",        "TM",
					         "TM",        "TM",
                                                 "Dartmouth"};
					   
/*Word number within minor frame, counting from 1.*/
/* If the word is 0, it means there is a range of words in the minor frame that contain the relevant measurement*/
static uint16_t         uStatWord[]        =    {  1, 201,
					         200, 201,
						 399, 400,
                                                   0};
					   
/*Word interval within a minor frame*/
/*When the interval is the size of the minor frame, the word only occurs once within the minor frame containing it*/
static uint16_t         uStatWdInt[]       =    {400, 400,
					         400, 400,
					         400, 400};
/*First frame containing each word*/
static uint16_t        uStatFrame[]        =    {1, 4,  
					         1, 1,
					         1, 1,
                                                 1};
/*Interval of frames containing each word*/
static uint16_t        uStatFrInt[]        =    {1, 4,  
					         1, 1,
					         1, 1,
                                                 1};

/*Sampling rate of each measurement*/
static uint32_t         ulStatSPS[]        =    {1500,   375,								                      
					         1500,  1125,
					         1500,  1500,
                                               592500};

/*Number of asym word ranges in minor frame for each measurement*/
/*A range of zero means the measurement corresponds to a single word*/
static uint16_t         uNAsymWRanges[][]  =    {0, 0,
						 0, 0,
						 0, 0,
                                                 2};
/*Specification of the asym word ranges w/in a minor frame, inclusive*/
static uint16_t         uAsymWRanges[][]   =    {{  2, 199},   //Just for RxDSP words, inclusive of words
					        {202, 398}};

/*Number of asym frame ranges in minor frame for this measurement*/
static uint16_t         uNAsymFRanges[][]  =    {0, 0,
						 0, 1,
						 0, 0,
                                                 0};

/*Specification of the frame ranges w/in a minor frame, inclusive*/
static uint16_t         uAsymFRanges[][]   =    {{  2, 199}};   //Just for MF2 words, inclusive of words

#endif // #ifndef _TM23_MAJORFRAME

