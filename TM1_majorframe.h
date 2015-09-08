#ifndef _TM1_MAJORFRAME
#define _TM1_MAJORFRAME

#define N_TM1_CHANS 28

static char   * szStatSerialChanNames[]  =    {"Langmuir Probe Channel 1 MSB ", "Langmuir Probe Channel 1 LSB ",
					       "Langmuir Probe Channel 2 MSB ", "Langmuir Probe Channel 2 LSB ",
					       "Langmuir Probe Channel 3 MSB ", "Langmuir Probe Channel 3 LSB ",
					       "Langmuir Probe Channel 4 MSB ", "Langmuir Probe Channel 4 LSB ",

					       "(Remote A/D # 1) VCO1 MSB    ", "(Remote A/D # 1) VCO1 LSB    ",
					       "(Remote A/D # 1) VCO2 MSB    ", "(Remote A/D # 1) VCO2 LSB    ",
					       "(Remote A/D # 1) FixedLP MSB ", "(Remote A/D # 1) FixedLP LSB ",
					       "(Remote A/D # 1) SweptLP MSB ", "(Remote A/D # 1) SweptLP LSB ",

					       "(Remote A/D # 2) MAG X MSB   ", "(Remote A/D # 2) MAG X LSB   ",
					       "(Remote A/D # 2) MAG Y MSB   ", "(Remote A/D # 2) MAG Y LSB   ",
					       "(Remote A/D # 2) MAG Z MSB   ", "(Remote A/D # 2) MAG Z LSB   ",
					       "(Remote A/D # 2) NOT SAMPLED ", "(Remote A/D # 2) NOT SAMPLED ",

					       "(Remote A/D # 3) VLF‐A MSB   ", "(Remote A/D # 3) VLF‐A LSB   ",
					       "(Remote A/D # 3) VLF‐B MSB   ", "(Remote A/D # 3) VLF‐B LSB   ",
					       "(Remote A/D # 3) VLF‐AGCA MSB", "(Remote A/D # 3) VLF‐AGCA LSB",
					       "(Remote A/D # 3) VLF‐AGCB MSB", "(Remote A/D # 3) VLF‐AGCB LSB",

					       "(Remote A/D # 4) VF‐ALO MSB  ", "(Remote A/D # 4) VF‐ALO LSB  ",
					       "(Remote A/D # 4) VF‐AHI MSB  ", "(Remote A/D # 4) VF‐AHI LSB  ",
					       "(Remote A/D # 4) VF‐BLO MSB  ", "(Remote A/D # 4) VF‐BLO LSB  ",
					       "(Remote A/D # 4) VF‐BHI MSB  ", "(Remote A/D # 4) VF‐BHI LSB  ",

					       "(Remote A/D # 5) ELF‐ALO MSB ", "(Remote A/D # 5) ELF‐ALO LSB ",
					       "(Remote A/D # 5) ELF‐AHI MSB ", "(Remote A/D # 5) ELF‐AHI LSB ",
					       "(Remote A/D # 5) ELF‐BLO MSB ", "(Remote A/D # 5) ELF‐BLO LSB ",
					       "(Remote A/D # 5) ELF‐BHI MSB ", "(Remote A/D # 5) ELF‐BHI LSB ",

					       "(Remote A/D # 6) SKIN‐LO MSB ", "(Remote A/D # 6) SKIN‐LO LSB ",
					       "(Remote A/D # 6) SKIN‐HI MSB ", "(Remote A/D # 6) SKIN‐HI LSB ",
					       "(Remote A/D # 6) HF‐AGC MSB  ", "(Remote A/D # 6) HF‐AGC LSB  ",
					       "(Remote A/D # 6) NOT SAMPLED ", "(Remote A/D # 6) NOT SAMPLED "};


static char   * szStatSerialChanAbbrev[] =    {"LP01MSB",      "LP01LSB",
					       "LP02MSB",      "LP02LSB",
					       "LP03MSB",      "LP03LSB",
					       "LP04MSB",      "LP04LSB",

					       "VCO1_MSB",     "VCO1_LSB",
					       "VCO2_MSB",     "VCO2_LSB",
					       "FixedLP_MSB",  "FixedLP_LSB",
					       "SweptLP_MSB",  "SweptLP_LSB",

					       "MAG_X_MSB",    "MAG_X_LSB",
					       "MAG_Y_MSB",    "MAG_Y_LSB",
					       "MAG_Z_MSB",    "MAG_Z_LSB",
					       "NOT_SAMPLED",  "NOT_SAMPLED",

					       "VLF‐A_MSB",    "VLF‐A_LSB",
					       "VLF‐B_MSB",    "VLF‐B_LSB",
					       "VLF‐AGCA_MSB", "VLF‐AGCA_LSB",
					       "VLF‐AGCB_MSB", "VLF‐AGCB_LSB",

					       "VF‐ALO_MSB",   "VF‐ALO_LSB",
					       "VF‐AHI_MSB",   "VF‐AHI_LSB",
					       "VF‐BLO_MSB",   "VF‐BLO_LSB",
					       "VF‐BHI_MSB",   "VF‐BHI_LSB",

					       "ELF‐ALO_MSB",  "ELF‐ALO_LSB",
					       "ELF‐AHI_MSB",  "ELF‐AHI_LSB",
					       "ELF‐BLO_MSB",  "ELF‐BLO_LSB",
					       "ELF‐BHI_MSB",  "ELF‐BHI_LSB",

					       "SKIN‐LO_MSB",  "SKIN‐LO_LSB",
					       "SKIN‐HI_MSB",  "SKIN‐HI_LSB",
					       "HF‐AGC_MSB",   "HF‐AGC_LSB",
					       "NOT_SAMPLED ", "NOT_SAMPLED "};

static char   * szStatUser[]             =    {"UiO",       "UiO",
					       "UiO",       "UiO",
					       "UiO",       "UiO",
					       "UiO",       "UiO",

					       "U_Iowa",    "U_Iowa",
					       "U_Iowa",    "U_Iowa",
					       "U_Iowa",    "U_Iowa",
					       "U_Iowa",    "U_Iowa",

					       "U_Iowa",    "U_Iowa",
					       "U_Iowa",    "U_Iowa",
					       "U_Iowa",    "U_Iowa",
					       "N/A",       "N/A",

					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",

					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",

					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",

					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "Dartmouth", "Dartmouth",
					       "N/A",       "N/A"};

static uint16_t          uStatWord[]     =    {94,  95,
					       96,  97,
					       98, 101,
					      102, 114,

					       29,  30,
					       29,  30,
					       29,  30,
					       29,  30,

					       33,  34,
					       35,  36,
					       37,  38,
					       -1,  -1,

					       3,   4,
					       7,   8,
					       5,   6,
					       9,  10,

					      13,  14,
					      17,  18,
					      21,  22,
					      23,  24,

					      29,  30,
					      29,  30,
					      33,  34,
					      33,  34,

					      33,  34,
					      33,  34,
					       1,   2,
					      -1, -1};

static uint16_t         uStatWdInt[]     = {120, 120,
					    120, 120,
					    120, 120,
					    120, 120,

					    120, 120,
					    120, 120,
					    120, 120,
					    120, 120,

					    120, 120,
					    120, 120,
					    120, 120,
					     -1,  -1,

					     12,  12,
					     12,  12,
					    120, 120,
					    120, 120,

					    120, 120,
					    120, 120,
					    120, 120,
					    120, 120,

					    120, 120,
					    120, 120,
					    120, 120,
					    120, 120,

					    120, 120,
					    120, 120,
					     40,  40,
					     -1, -1};

static uint16_t        uStatFrame[]      = {1, 1,
					    1, 1,
					    1, 1,
					    1, 1,

					    1, 1,
					    2, 2,
					    3, 3,
					    4, 4,

					    1, 1,
					    1, 1,
					    1, 1,
					    0, 0,

					    1, 1,
					    1, 1,
					    1, 1,
					    1, 1,

					    1, 1,
					    1, 1,
					    1, 1,
					    1, 1,

					    7, 7,
					    8, 8,
					    2, 2,
					    3, 3,

					    4, 4,
					    5, 5,
					    1, 1,
					    0, 0};

static uint16_t        uStatFrInt[]      = {1, 1,
					    1, 1,
					    1, 1,
					    1, 1,

					    4, 4,
					    4, 4,
					    8, 8,
					    8, 8,

					    8, 8,
					    8, 8,
					    8, 8,
					    0, 0,

					    1, 1,
					    1, 1,
					    1, 1,
					    1, 1,

					    1, 1,
					    1, 1,
					    1, 1,
					    1, 1,

					    8, 8,
					    8, 8,
					    8, 8,
					    8, 8,

					    8, 8,
					    8, 8,
					    1, 1,
					    0, 0};

static uint32_t           ulStatSPS[]    = { 8000,  8000,
					     8000,  8000,
					     8000,  8000,
					     8000,  8000,
						    
					     2000,  2000,
					     2000,  2000,
					     1000,  1000,
					     1000,  1000,
						    
					     1000,  1000,
					     1000,  1000,
					     1000,  1000,
						0,     0,

					    80000, 80000,
					    80000, 80000,
					     8000,  8000,
					     8000,  8000,

					     8000,  8000,
					     8000,  8000,
					     8000,  8000,
					     8000,  8000,

					     1000,  1000,
					     1000,  1000,
					     1000,  1000,
					     1000,  1000,

					     1000,  1000,
					     1000,  1000,
					    24000, 24000};

#endif // #ifndef _TM1_MAJORFRAME

