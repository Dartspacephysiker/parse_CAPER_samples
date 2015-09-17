#ifndef _TM1_MAJORFRAME
#define _TM1_MAJORFRAME

#define N_TM1_MEASUREMENTS         66    //Total number of measurementnels in this header
			          
#define TM1_SFID_IDX               25    //Location of subframe ID within minor frame, counting from 1
			          
#define TM1_BPS               9600000    //Link BPS

#define TM1_NUM_MFCOUNTERS          3

#define TM1_WORD_BITLENGTH         10
#define TM1_MINORFRAME_BITSHIFT     5    //up to 32 minor frames

#define TM1_N_GPS_WORDS             2

//Static variables to accommodate combination of MSB/LSB channels for measurements that are separated
#define TM_SKIP_LSB               499    //Skip this channel if combination is done on the fly
#define TM_NO_LSB                 498    //Don't try to combine this channel
#define TM_UPPER6_MSB_LOWER10_LSB 109

/*Names of measurements, defined by NASA PCM doc*/
static char   * szTM1SerialMeasNames[]  =    {"Langmuir Probe Channel 1 MSB ", "Langmuir Probe Channel 1 LSB ",	    //Serial #1, 0-7
					       "Langmuir Probe Channel 2 MSB ", "Langmuir Probe Channel 2 LSB ",
					       "Langmuir Probe Channel 3 MSB ", "Langmuir Probe Channel 3 LSB ",
					       "Langmuir Probe Channel 4 MSB ", "Langmuir Probe Channel 4 LSB ",

					       "(Remote A/D # 1) VCO1 MSB    ", "(Remote A/D # 1) VCO1 LSB    ",    //Serial #2, 8-15
					       "(Remote A/D # 1) VCO2 MSB    ", "(Remote A/D # 1) VCO2 LSB    ",
					       "(Remote A/D # 1) FixedLP MSB ", "(Remote A/D # 1) FixedLP LSB ",
					       "(Remote A/D # 1) SweptLP MSB ", "(Remote A/D # 1) SweptLP LSB ",

					       "(Remote A/D # 2) MAG X MSB   ", "(Remote A/D # 2) MAG X LSB   ",    //Serial #3, 16-21
					       "(Remote A/D # 2) MAG Y MSB   ", "(Remote A/D # 2) MAG Y LSB   ",
					       "(Remote A/D # 2) MAG Z MSB   ", "(Remote A/D # 2) MAG Z LSB   ",
					       //"(Remote A/D # 2) NOT SAMPLED ", "(Remote A/D # 2) NOT SAMPLED ",

					       "(Remote A/D # 3) VLF-A MSB   ", "(Remote A/D # 3) VLF-A LSB   ",    //Serial #4, 22-29
					       "(Remote A/D # 3) VLF-B MSB   ", "(Remote A/D # 3) VLF-B LSB   ",
					       "(Remote A/D # 3) VLF-AGCA MSB", "(Remote A/D # 3) VLF-AGCA LSB",
					       "(Remote A/D # 3) VLF-AGCB MSB", "(Remote A/D # 3) VLF-AGCB LSB",

					       "(Remote A/D # 4) VF-ALO MSB  ", "(Remote A/D # 4) VF-ALO LSB  ",    //Serial #5, 30-37
					       "(Remote A/D # 4) VF-AHI MSB  ", "(Remote A/D # 4) VF-AHI LSB  ",
					       "(Remote A/D # 4) VF-BLO MSB  ", "(Remote A/D # 4) VF-BLO LSB  ",
					       "(Remote A/D # 4) VF-BHI MSB  ", "(Remote A/D # 4) VF-BHI LSB  ",

					       "(Remote A/D # 5) ELF-ALO MSB ", "(Remote A/D # 5) ELF-ALO LSB ",    //Serial #6, 38-45
					       "(Remote A/D # 5) ELF-AHI MSB ", "(Remote A/D # 5) ELF-AHI LSB ",
					       "(Remote A/D # 5) ELF-BLO MSB ", "(Remote A/D # 5) ELF-BLO LSB ",
					       "(Remote A/D # 5) ELF-BHI MSB ", "(Remote A/D # 5) ELF-BHI LSB ",

					       "(Remote A/D # 6) SKIN-LO MSB ", "(Remote A/D # 6) SKIN-LO LSB ",    //Serial #7, 46-51
					       "(Remote A/D # 6) SKIN-HI MSB ", "(Remote A/D # 6) SKIN-HI LSB ",
					       "(Remote A/D # 6) HF-AGC MSB  ", "(Remote A/D # 6) HF-AGC LSB  ",
					       //"(Remote A/D # 6) NOT SAMPLED ", "(Remote A/D # 6) NOT SAMPLED ",

					       "Subframe ID                  ",
					       "Major Frame Counter 1        ", "Major Frame Counter 2        ",    //TM + SFID  52-58
					       "Major Frame Counter 3        ", "Frame Sync 1                 ",
					       "Frame Sync 2                 ", "Frame Sync 3                 ",

					       "GPS 1PPS msb                 ", "GPS 1PPS lsb                 ", 
					       "ACS 1PPS msb                 ", "ACS 1PPS lsb                 ", 

                                               "Correlator 1 FEEA Bagel 1 N/S", "Correlator 2 FEEA Bagel 2 N/S",
                                               "EEPAA (!!!NOT SYMMETRIC!!!)  "};

/*Abbreviation of measurements for outputting files*/
static char   * szTM1SerialMeasAbbrev[] =    { "LP01_MSB",      "LP01_LSB",					    //Serial #1, 0-7  
					       "LP02_MSB",      "LP02_LSB",					                      
					       "LP03_MSB",      "LP03_LSB",					                      
					       "LP04_MSB",      "LP04_LSB",					                      
														                      
					       "VCO1_MSB",     "VCO1_LSB",					    //Serial #2, 8-15 
					       "VCO2_MSB",     "VCO2_LSB",					                      
					       "FixedLP_MSB",  "FixedLP_LSB",					                      
					       "SweptLP_MSB",  "SweptLP_LSB",					                      
														                      
					       "MAG_X_MSB",    "MAG_X_LSB",					    //Serial #3, 16-21
					       "MAG_Y_MSB",    "MAG_Y_LSB",					                      
					       "MAG_Z_MSB",    "MAG_Z_LSB",					                      
					       //"NOT_SAMPLED",  "NOT_           SAMPLED",				              
														                      
					       "VLF-A_MSB",    "VLF-A_LSB",					    //Serial #4, 22-29
					       "VLF-B_MSB",    "VLF-B_LSB",					                      
					       "VLF-AGCA_MSB", "VLF-AGCA_LSB",					                      
					       "VLF-AGCB_MSB", "VLF-AGCB_LSB",					                      
														                      
					       "VF-ALO_MSB",   "VF-ALO_LSB",					    //Serial #5, 30-37
					       "VF-AHI_MSB",   "VF-AHI_LSB",					                      
					       "VF-BLO_MSB",   "VF-BLO_LSB",					                      
					       "VF-BHI_MSB",   "VF-BHI_LSB",					                      
														                      
					       "ELF-ALO_MSB",  "ELF-ALO_LSB",					    //Serial #6, 38-45
					       "ELF-AHI_MSB",  "ELF-AHI_LSB",					                      
					       "ELF-BLO_MSB",  "ELF-BLO_LSB",					                      
					       "ELF-BHI_MSB",  "ELF-BHI_LSB",					                      
														                      
					       "SKIN-LO_MSB",  "SKIN-LO_LSB",					    //Serial #7, 46-51
					       "SKIN-HI_MSB",  "SKIN-HI_LSB",					                      
					       "HF-AGC_MSB",   "HF-AGC_LSB",					                      
					       //"NOT_SAMPLED ", "NOT_SAMPLED "};					              
														                      
					       "SFID",								                      
					       "MF1",          "MF2",						    //TM + SFID   52-58
					       "MF3",          "FS1",
					       "FS2",          "FS3",

					       "GPS_1PPS_msb", "GPS_1PPS_lsb", 
					       "ACS_1PPS_msb", "ACS_1PPS_lsb", 

                                               "FEEA_Bagel1",  "FEEA_Bagel2",
                                               "EEPAA"};

/*User of measurement, as defined by NASA PCM doc*/
static char   * szTM1User[]             =    {"UiO",       "UiO",					    //Serial #1, 0-7  
					      "UiO",       "UiO",						                      
					      "UiO",       "UiO",						                      
					      "UiO",       "UiO",						                      
													                      
					      "U_Iowa",    "U_Iowa",					    //Serial #2, 8-15 
					      "U_Iowa",    "U_Iowa",						                      
					      "U_Iowa",    "U_Iowa",						                      
					      "U_Iowa",    "U_Iowa",						                      
													                      
					      "U_Iowa",    "U_Iowa",					    //Serial #3, 16-21
					      "U_Iowa",    "U_Iowa",						                      
					      "U_Iowa",    "U_Iowa",						                      
					      //"N/A",       "N/A",             					              
														                      
					      "Dartmouth", "Dartmouth",					    //Serial #4, 22-29
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
														                      
					      "Dartmouth", "Dartmouth",					    //Serial #5, 30-37
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
														                      
					      "Dartmouth", "Dartmouth",					    //Serial #6, 38-45
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
														                      
					      "Dartmouth", "Dartmouth",					    //Serial #7, 46-51
					      "Dartmouth", "Dartmouth",					                      
					      "Dartmouth", "Dartmouth",					                      
					      //"N/A",       "N/A",             					              
														                      
					      "TM",//SFID							                      
					      "TM",        "TM",					    //TM + SFID   52-58
					      "TM",        "TM",
					      "TM",        "TM",

					      "TM",        "TM", 
					      "ACS",       "ACS", 

                                              "U_Iowa",    "U_Iowa",
                                              "U_Iowa"};

/*Location of first (and possibly only) word corresponding to measurement*/
static uint16_t          uTM1Word[]     =    { 94,  95,                                                             //Serial #1, 0-7  
					       96,  97,								                      
					       98, 101,								                      
					      102, 114,								                      
														                      
					       29,  30,								    //Serial #2, 8-15 
					       29,  30,								                      
					       29,  30,								                      
					       29,  30,								                      
														                      
					       33,  34,								    //Serial #3, 16-21
					       35,  36,								                      
					       37,  38,								                      
					       //-1,  -1,		                 			    		        
														                      
					        3,   4,								    //Serial #4, 22-29
					        7,   8,								                      
					        5,   6,								                      
					        9,  10,								                      
														                      
					       13,  14,								    //Serial #5, 30-37
					       17,  18,								                      
					       21,  22,								                      
					       23,  24,								                      
														                      
					       29,  30,								    //Serial #6, 38-45
					       29,  30,								                      
					       33,  34,								                      
					       33,  34,								                      
														                      
					       33,  34,								    //Serial #7, 46-51
					       33,  34,								                      
					        1,   2,								                      
					       //-1, -1};		                 			    		        
														                      
					       25,//SFID							                      
					       65,  66,                 //TM					    //TM + SFID   52-58
					      113, 118,
					      119, 120,

					      110, 109,
					      110, 109,

					       45,  57,
					       73};

/*Word interval within a minor frame*/
/*When the interval is the size of the minor frame, the word only occurs once within the minor frame containing it*/
/*When the interval is 0, it shows up in strange places (i.e., there are a range of words in the PCM minor frame containing it*/

static uint16_t         uTM1WdInt[]     =      {120, 120,							    //Serial #1, 0-7  
						120, 120,								                      
						120, 120,								                      
						120, 120,								                      
														                      
						120, 120,							    //Serial #2, 8-15 
						120, 120,								                      
						120, 120,								                      
						120, 120,								                      
														                      
						120, 120,							    //Serial #3, 16-21
						120, 120,								                      
						120, 120,								                      
														                      
						 12,  12,							    //Serial #4, 22-29
						 12,  12,								                      
					        120, 120,								                      
					        120, 120,								                      
														                      
						120, 120,							    //Serial #5, 30-37
						120, 120,								                      
						120, 120,								                      
						120, 120,								                      
														                      
						120, 120,							    //Serial #6, 38-45
						120, 120,								                      
						120, 120,								                      
						120, 120,								                      
														                      
						120, 120,							    //Serial #7, 46-51
						120, 120,								                      
						 40,  40,								                      
						
						120,
						120, 120,                             				    //TM + SFID   52-58
						120, 120,
						120, 120,

						120, 120,
						120, 120,

						120, 120,
						120};
					    
/*Interval of frames containing each word*/
static uint16_t        uTM1Frame[]      =    {1, 1,                                                                   //Serial #1, 0-7  
					      1, 1,								                      
					      1, 1,								                      
					      1, 1,								                      
														                      
					      1, 1,								    //Serial #2, 8-15 
					      2, 2,								                      
					      3, 3,								                      
					      4, 4,								                      
					      
					      1, 1,								    //Serial #3, 16-21
					      1, 1,								                      
					      1, 1,								                      
					      //0, 0,			                 			    		        
					      
					      1, 1,								    //Serial #4, 22-29
					      1, 1,								                      
					      1, 1,								                      
					      1, 1,								                      
					      
					      1, 1,								    //Serial #5, 30-37
					      1, 1,								                      
					      1, 1,								                      
					      1, 1,								                      
					      
					      7, 7,								    //Serial #6, 38-45
					      8, 8,								                      
					      2, 2,								                      
					      3, 3,								                      
					      
					      4, 4,								    //Serial #7, 46-51
					      5, 5,								                      
					      1, 1,								                      
					      //0, 0};			                 			    		        
					      
					      1,   //SFID								                      
					      1, 1,								    //TM + SFID   52-58
					      1, 1,
					      1, 1,
					      
					     29,29,
					     30,30,
					      
					      1, 1,
					      1};


/*Interval of frames containing each word*/
static uint16_t        uTM1FrInt[]      =    {1, 1,                                                                   //Serial #1, 0-7  
					      1, 1,								                      
					      1, 1,								                      
					      1, 1,								                      
					      
					      4, 4,								    //Serial #2, 8-15 
					      4, 4,								                      
					      8, 8,								                      
					      8, 8,								                      
					      
					      8, 8,								    //Serial #3, 16-21
					      8, 8,								                      
					      8, 8,								                      
					      //0, 0,			                 			    		        
					      
					      1, 1,								    //Serial #4, 22-29
					      1, 1,								                      
					      1, 1,								                      
					      1, 1,								                      
					      
					      1, 1,								    //Serial #5, 30-37
					      1, 1,								                      
					      1, 1,								                      
					      1, 1,								                      
					      
					      8, 8,								    //Serial #6, 38-45
					      8, 8,								                      
					      8, 8,								                      
					      8, 8,								                      
					      
					      8, 8,								    //Serial #7, 46-51
					      8, 8,								                      
					      1, 1,								                      
					      //0, 0};			                 			    		        
					      
					      1,   //SFID				                 		    		        
					      1, 1,								    //TM + SFID   52-58
					      1, 1,
					      1, 1,
					      
					     32,32,
					     32,32,
					      
					      1, 1,
					      1};

/*Sampling rate of each measurement*/
static uint32_t           ulTM1SPS[]    =    { 8000,  8000,                                                           //Serial #1, 0-7  
					       8000,  8000,							                      
					       8000,  8000,							                      
					       8000,  8000,							                      
					       
					       2000,  2000,							    //Serial #2, 8-15 
					       2000,  2000,							                      
					       1000,  1000,							                      
					       1000,  1000,							                      
					       
					       1000,  1000,							    //Serial #3, 16-21
					       1000,  1000,							                      
					       1000,  1000,							                      
					       //0,     0,							                      
					       
					      80000, 80000,							    //Serial #4, 22-29
					      80000, 80000,							                      
					       8000,  8000,							                      
					       8000,  8000,							                      
					       
					       8000,  8000,							    //Serial #5, 30-37
					       8000,  8000,							                      
					       8000,  8000,							                      
					       8000,  8000,							                      
					       
					       1000,  1000,							    //Serial #6, 38-45
					       1000,  1000,							                      
					       1000,  1000,							                      
					       1000,  1000,							                      
					       
					       1000,  1000,							    //Serial #7, 46-51
					       1000,  1000,							                      
					      24000, 24000,							                      
					       //0,     0,							                      
														                      
					       8000,								                      
					       8000,  8000,							    //TM + SFID   52-58
					       8000,  8000,
					       8000,  8000,
					       
					        250,   250,
					        250,   250,
					       
					      64000, 64000,
					      80000};

/*Number of asym word ranges in minor frame for each measurement*/
/*A range of zero means the measurement corresponds to a single word*/
static uint16_t      uTM1NAsymWRanges[] =      {0, 0,                                                              //Serial #1, 0-7  
						0, 0,								                      
						0, 0,								                      
						0, 0,								                      
					        
						0, 0,								    //Serial #2, 8-15 
						0, 0,								                      
						0, 0,								                      
						0, 0,								                      
					        
						0, 0,								    //Serial #3, 16-21
						0, 0,								                      
						0, 0,								                      
						//0, 0,			                 			    		        
					        
						0, 0,								    //Serial #4, 22-29
						0, 0,								                      
						0, 0,								                      
						0, 0,								                      
					        
						0, 0,								    //Serial #5, 30-37
						0, 0,								                      
						0, 0,								                      
						0, 0,								                      
					        
						0, 0,								    //Serial #6, 38-45
						0, 0,								                      
						0, 0,								                      
						0, 0,								                      
					        
						0, 0,								    //Serial #7, 46-51
						0, 0,								                      
						0, 0,								                      
						//0, 0};			                 			    		        
					        
						0,   //SFID				                 		    		        
						0, 0,								    //TM + SFID   52-58
						0, 0,
						0, 0,
						
						0, 0,
						0, 0,
						
						2, 2,
						5};

/*Specification of the asym word ranges w/in a minor frame, inclusive*/
static uint16_t      uTM1AsymWRanges[][2] =  {{45, 50},								    //Bagel 1
					      {53, 54},

					      {57, 62},								    //Bagel 2
					      {69, 70},								    /*  */

					      {73, 74},								    //EEPAA
					      {77, 78},								    /*  */
					      {83, 85},
					      {89, 90},
					      {93, 93}};

/*Number of asym frame ranges in minor frame for this measurement*/
static uint16_t      uTM1NAsymFRanges[] =    {0, 0,                                                              //Serial #1, 0-7  
					      0, 0,								                      
					      0, 0,								                      
					      0, 0,								                      
					      
					      0, 0,								    //Serial #2, 8-15 
					      0, 0,								                      
					      0, 0,								                      
					      0, 0,								                      
					      
					      0, 0,								    //Serial #3, 16-21
					      0, 0,								                      
					      0, 0,								                      
					      //0, 0,			                 			    		        
					      
					      0, 0,								    //Serial #4, 22-29
					      0, 0,								                      
					      0, 0,								                      
					      0, 0,								                      
					      
					      0, 0,								    //Serial #5, 30-37
					      0, 0,								                      
					      0, 0,								                      
					      0, 0,								                      
					      
					      0, 0,								    //Serial #6, 38-45
					      0, 0,								                      
					      0, 0,								                      
					      0, 0,								                      
					      
					      0, 0,								    //Serial #7, 46-51
					      0, 0,								                      
					      0, 0,								                      
					      //0, 0};			                 			    		        
					      
					      0,   //SFID				                 		    		        
					      0, 0,								    //TM + SFID   52-58
					      0, 0,
					      0, 0,
					      
					      0, 0,
					      0, 0,
					      
					      0, 0,
					      0};

/*Specification of the frame ranges w/in a minor frame, inclusive*/
static uint16_t      uTM1AsymFRanges[][2]  = {{0,0}};

static uint16_t         uTM1LSBWord[]   =    { 95, TM_SKIP_LSB,						    //Serial #1, 0-7  
					       97, TM_SKIP_LSB,								                      
					      101, TM_SKIP_LSB,								                      
					      114, TM_SKIP_LSB,								                      
														                      
					       30, TM_SKIP_LSB,						    //Serial #2, 8-15 
					       30, TM_SKIP_LSB,								                      
					       30, TM_SKIP_LSB,								                      
					       30, TM_SKIP_LSB,								                      
														                      
					       34, TM_SKIP_LSB,						    //Serial #3, 16-21
					       36, TM_SKIP_LSB,								                      
					       38, TM_SKIP_LSB,								                      
														                      
					        4, TM_SKIP_LSB,						    //Serial #4, 22-29
					        8, TM_SKIP_LSB,								                      
					        6, TM_SKIP_LSB,								                      
					       10, TM_SKIP_LSB,								                      
														                      
					       14, TM_SKIP_LSB,						    //Serial #5, 30-37
					       18, TM_SKIP_LSB,								                      
					       22, TM_SKIP_LSB,								                      
					       24, TM_SKIP_LSB,								                      
					       
					       30, TM_SKIP_LSB,						    //Serial #6, 38-45
					       30, TM_SKIP_LSB,								                      
					       34, TM_SKIP_LSB,								                      
					       34, TM_SKIP_LSB,								                      
														                      
					       34, TM_SKIP_LSB,						    //Serial #7, 46-51
					       34, TM_SKIP_LSB,								                      
					        2, TM_SKIP_LSB,								                      
					       
					TM_NO_LSB,
				        TM_NO_LSB,   TM_NO_LSB,						    //TM + SFID   52-58
			                TM_NO_LSB,   TM_NO_LSB,
			     	        TM_NO_LSB,   TM_NO_LSB,

					      109, TM_SKIP_LSB,
					      109, TM_SKIP_LSB,

				        TM_NO_LSB, TM_NO_LSB,
					TM_NO_LSB};



static uint16_t    auTM1MFCMeasIdx[]    =    {  53, 54,  //Major frame measurement indices (indexing from zero in the arrays above)
					        55};

static uint16_t    auTM1GPSMeasIdx[]    =    {  59, 60}; //GPS 1pps measurement indices (indexing from zero in the arrays above)

#endif // #ifndef _TM1_MAJORFRAME

