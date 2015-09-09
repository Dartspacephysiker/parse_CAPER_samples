#ifndef _TM1_MAJORFRAME
#define _TM1_MAJORFRAME

#define N_TM1_CHANS   59

#define SFID_CHAN_IDX 25

static char   * szStatSerialChanNames[]  =    {"Langmuir Probe Channel 1 MSB ", "Langmuir Probe Channel 1 LSB ",    //Serial #1, 0-7
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

					       "(Remote A/D # 3) VLF‐A MSB   ", "(Remote A/D # 3) VLF‐A LSB   ",    //Serial #4, 22-29
					       "(Remote A/D # 3) VLF‐B MSB   ", "(Remote A/D # 3) VLF‐B LSB   ",
					       "(Remote A/D # 3) VLF‐AGCA MSB", "(Remote A/D # 3) VLF‐AGCA LSB",
					       "(Remote A/D # 3) VLF‐AGCB MSB", "(Remote A/D # 3) VLF‐AGCB LSB",

					       "(Remote A/D # 4) VF‐ALO MSB  ", "(Remote A/D # 4) VF‐ALO LSB  ",    //Serial #5, 30-37
					       "(Remote A/D # 4) VF‐AHI MSB  ", "(Remote A/D # 4) VF‐AHI LSB  ",
					       "(Remote A/D # 4) VF‐BLO MSB  ", "(Remote A/D # 4) VF‐BLO LSB  ",
					       "(Remote A/D # 4) VF‐BHI MSB  ", "(Remote A/D # 4) VF‐BHI LSB  ",

					       "(Remote A/D # 5) ELF‐ALO MSB ", "(Remote A/D # 5) ELF‐ALO LSB ",    //Serial #6, 38-45
					       "(Remote A/D # 5) ELF‐AHI MSB ", "(Remote A/D # 5) ELF‐AHI LSB ",
					       "(Remote A/D # 5) ELF‐BLO MSB ", "(Remote A/D # 5) ELF‐BLO LSB ",
					       "(Remote A/D # 5) ELF‐BHI MSB ", "(Remote A/D # 5) ELF‐BHI LSB ",

					       "(Remote A/D # 6) SKIN‐LO MSB ", "(Remote A/D # 6) SKIN‐LO LSB ",    //Serial #7, 46-51
					       "(Remote A/D # 6) SKIN‐HI MSB ", "(Remote A/D # 6) SKIN‐HI LSB ",
					       "(Remote A/D # 6) HF‐AGC MSB  ", "(Remote A/D # 6) HF‐AGC LSB  ",
					       //"(Remote A/D # 6) NOT SAMPLED ", "(Remote A/D # 6) NOT SAMPLED ",

					       "Subframe ID                  ",
					       "Major Frame Counter 1        ", "Major Frame Counter 2        ",    //TM + SFID  52-58
					       "Major Frame Counter 3        ", "Frame Sync 1                 ",
					       "Frame Sync 2                 ", "Frame Sync 3                 "};


static char   * szStatSerialChanAbbrev[] =    {"LP01MSB",      "LP01LSB",                                           //Serial #1, 0-7  
					       "LP02MSB",      "LP02LSB",					                      
					       "LP03MSB",      "LP03LSB",					                      
					       "LP04MSB",      "LP04LSB",					                      
														                      
					       "VCO1_MSB",     "VCO1_LSB",					    //Serial #2, 8-15 
					       "VCO2_MSB",     "VCO2_LSB",					                      
					       "FixedLP_MSB",  "FixedLP_LSB",					                      
					       "SweptLP_MSB",  "SweptLP_LSB",					                      
														                      
					       "MAG_X_MSB",    "MAG_X_LSB",					    //Serial #3, 16-21
					       "MAG_Y_MSB",    "MAG_Y_LSB",					                      
					       "MAG_Z_MSB",    "MAG_Z_LSB",					                      
					       //"NOT_SAMPLED",  "NOT_           SAMPLED",				              
														                      
					       "VLF‐A_MSB",    "VLF‐A_LSB",					    //Serial #4, 22-29
					       "VLF‐B_MSB",    "VLF‐B_LSB",					                      
					       "VLF‐AGCA_MSB", "VLF‐AGCA_LSB",					                      
					       "VLF‐AGCB_MSB", "VLF‐AGCB_LSB",					                      
														                      
					       "VF‐ALO_MSB",   "VF‐ALO_LSB",					    //Serial #5, 30-37
					       "VF‐AHI_MSB",   "VF‐AHI_LSB",					                      
					       "VF‐BLO_MSB",   "VF‐BLO_LSB",					                      
					       "VF‐BHI_MSB",   "VF‐BHI_LSB",					                      
														                      
					       "ELF‐ALO_MSB",  "ELF‐ALO_LSB",					    //Serial #6, 38-45
					       "ELF‐AHI_MSB",  "ELF‐AHI_LSB",					                      
					       "ELF‐BLO_MSB",  "ELF‐BLO_LSB",					                      
					       "ELF‐BHI_MSB",  "ELF‐BHI_LSB",					                      
														                      
					       "SKIN‐LO_MSB",  "SKIN‐LO_LSB",					    //Serial #7, 46-51
					       "SKIN‐HI_MSB",  "SKIN‐HI_LSB",					                      
					       "HF‐AGC_MSB",   "HF‐AGC_LSB",					                      
					       //"NOT_SAMPLED ", "NOT_SAMPLED "};					              
														                      
					       "SFID",								                      
					       "MF1",          "MF2",						    //TM + SFID   52-58
					       "MF3",          "FS1",
					       "FS2",          "FS3"};


static char   * szStatUser[]             =    {"UiO",       "UiO",                                                  //Serial #1, 0-7  
					       "UiO",       "UiO",						                      
					       "UiO",       "UiO",						                      
					       "UiO",       "UiO",						                      
														                      
					       "U_Iowa",    "U_Iowa",						    //Serial #2, 8-15 
					       "U_Iowa",    "U_Iowa",						                      
					       "U_Iowa",    "U_Iowa",						                      
					       "U_Iowa",    "U_Iowa",						                      
														                      
					       "U_Iowa",    "U_Iowa",						    //Serial #3, 16-21
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
					       "TM",        "TM",						    //TM + SFID   52-58
					       "TM",        "TM",
					       "TM",        "TM"};

static uint16_t          uStatWord[]     =    {94,  95,                                                             //Serial #1, 0-7  
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
					      119, 120};

static uint16_t         uStatWdInt[]     = {120, 120,                                                               //Serial #1, 0-7  
					    120, 120,								                      
					    120, 120,								                      
					    120, 120,								                      
														                      
					    120, 120,								    //Serial #2, 8-15 
					    120, 120,								                      
					    120, 120,								                      
					    120, 120,								                      
														                      
					    120, 120,								    //Serial #3, 16-21
					    120, 120,								                      
					    120, 120,								                      
					    //-1,  -1,			                 			    		        
														                      
					     12,  12,								    //Serial #4, 22-29
					     12,  12,								                      
					    120, 120,								                      
					    120, 120,								                      
														                      
					    120, 120,								    //Serial #5, 30-37
					    120, 120,								                      
					    120, 120,								                      
					    120, 120,								                      
														                      
					    120, 120,								    //Serial #6, 38-45
					    120, 120,								                      
					    120, 120,								                      
					    120, 120,								                      
														                      
					    120, 120,								    //Serial #7, 46-51
					    120, 120,								                      
					     40,  40,								                      
					    //-1, -1};			                 			    		        
														                      
					    120,//SFID								                      
					    120, 120,                             				    //TM + SFID   52-58
					    120, 120,
					    120, 120};
					    

static uint16_t        uStatFrame[]      = {1, 1,                                                                   //Serial #1, 0-7  
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
					    1, 1};

static uint16_t        uStatFrInt[]      = {1, 1,                                                                   //Serial #1, 0-7  
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
					    1, 1};

static uint32_t           ulStatSPS[]    = { 8000,  8000,                                                           //Serial #1, 0-7  
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
					     8000,  8000};

#endif // #ifndef _TM1_MAJORFRAME

