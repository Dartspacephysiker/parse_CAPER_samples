#DESCRIPTION OF TM link to be read by parse_PCM_samples v0.3
#
#NOTE the following:
#  -->A line beginning with '#' is treated as a comment
#  -->A line beginning with ';' marks the end of an array
#
#  -->A line beginning with any of the following is read into the program:
#
#	Values
#	======
#	NAME
#	N_MEASUREMENTS
#	SFID_IDX
#	BPS
#
#	SAMPLE_BITLENGTH
#	N_SAMPLES_PER_MINOR
#	N_MINOR_PER_MAJOR
#	N_MAJFRAMECOUNTERS
#	N_MINFRAME_BITPOS
#	N_GPS_WORDS
#
#	Arrays
#	======
#	MEAS_NAME
#	MEAS_ABBREV
#	MEAS_USER
#	MEAS_SAMPLE_LOC_IN_MINFRAME
#	MEAS_SAMPLE_INTERVAL_IN_MINFRAME
#	MEAS_SAMPLE_FRAME
#	MEAS_FRAME_INTERVAL
#	MEAS_SAMPLE_RATE
#	MEAS_N_ASYM_WORD_RANGES
#	MEAS_ASYM_WORD_RANGES
#	MEAS_N_ASYM_FRAME_RANGES
#	MEAS_ASYM_FRAME_RANGES
#	MEAS_LSB_WORD
#	MEAS_TSTAMP_CALC_TYPE
#	MEAS_TSTAMP_SEARCH_WORD
#	MEAS_INTERN_SAMP_RATE
#	MAJFRAME_INDEX
#	GPS_INDEX

# Last of all, note that an array word (e.g., MEAS_USER) that is immediately followed by ';' on the next line
#   denotes that all values of the array should be zero.

#Description of TM link
NAME			TM23
N_MEASUREMENTS		7		//Number of measurements in this link
SFID_IDX		1		//Location of subframe ID within minor frame, counting from 1
BPS			9600000		//Transfer rate of this link, in bits per second


#Minor/major frame description
SAMPLE_BITLENGTH	16		//Length of each sample in bits (remember, 8 bits per byte)
N_SAMPLES_PER_MINOR	400		//Number of samples in each minor frame
N_MINOR_PER_MAJOR	4		//Number of minor frames per major frame
N_MAJFRAMECOUNTERS	2		//Number of major frame counters in this link
N_MINFRAME_BITPOS	2		//Number of bits required for minor frame counter (e.g., up to 4 minor frames requires
					//  two bit positions to count 0, 1, 2, 3). This is used to form a unique counter from
					//  the major frame counter(s) and the minor frame counter in the stream
#Extra
N_GPS_WORDS		1		//Number of GPS words defined in the PCM stream. (Max is two!)

#//Static variable to indicate that there is no timestamp_searchword
#//#define TM23_NO_TS_SEARCH    '\0'
#define TM23_NO_TS_SEARCH      0x0000

#Names of measurements, defined by NASA PCM doc
MEAS_NAME
Subframe ID
GPS 1PPS
Major Frame Counter 1
Major Frame Counter 2 (!!!NOT SYMMETRIC!!!)
Frame Sync 1
Frame Sync 2
RX‐DSP (!!!NOT SYMMETRIC!!!);

#Abbreviation of measurements (used for outputting files)
MEAS_ABBREV
SFID
GPS
MF1
MF2
FS1
FS2
Rx-DSP;
					   
#User of measurement, as defined by NASA PCM doc
MEAS_USER
TM
TM
TM
TM
TM
TM
Dartmouth;
					   
#Sample number within minor frame, counting from 1
#If the number is 0, it means there is a range of words in the minor frame that contain the relevant measurement.
# These are usually marked "(!!!NOT SYMMETRIC!!!)" in NASA PCM docs
MEAS_SAMPLE_LOC_IN_MINFRAME
1
201
200
201
399
400
0;
					   
#Word interval within a minor frame
#When the interval is the size of the minor frame, the word only occurs once within the minor frame containing it.
MEAS_SAMPLE_INTERVAL_IN_MINFRAME
400
400
400
400
400
400;

#First frame containing each word
MEAS_SAMPLE_FRAME
1
4
1
1
1
1
1;

#*Interval of frames containing each word
MEAS_FRAME_INTERVAL
1
4
1
1
1
1
1;

#Sampling rate of each measurement
MEAS_SAMPLE_RATE
1500
375
1500
1125
1500
1500
592500;

#Number of asym word ranges in minor frame for each measurement
#A range of zero means the measurement corresponds to a single word
MEAS_N_ASYM_WORD_RANGES
0
0
0
0
0
0
2;

#Specification of the asym word ranges w/in a minor frame, inclusive
MEAS_ASYM_WORD_RANGES
2, 199		#Just for RxDSP words, inclusive of words
202, 398;

#Number of asym frame ranges in minor frame for this measurement
MEAS_N_ASYM_FRAME_RANGES
0
0
0
1
0
0
0;

#Specification of the frame ranges w/in a minor frame, inclusive
MEAS_ASYM_FRAME_RANGES
1, 3;   #Just for MF2 words, inclusive of frame 2

#If any channel has an LSB word, mark it here
MEAS_LSB_WORD
;

#Type of timestamp calculation for this measurement
MEAS_TSTAMP_CALC_TYPE
0
0
0
0
0
0
3;

#Search words for calculation of timestamp
MEAS_TSTAMP_SEARCH_WORD
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
aDtr;

#Sampling rate of the measurement (not the rate at which it is sampled within the PCM range, but
#  the sampling rate of instrument making the measurement).
#This is used for calculating timestamps relative to the instrument sampling rate
MEAS_INTERN_SAMP_RATE
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
NO_TS_SEARCH
333333;

#Major frame sample index (or indices), indexing from zero in the arrays above
MAJFRAME_INDEX
2
3;    

#GPS 1pps measurement index (or indices), indexing from zero in the arrays above
GPS_INDEX
1;