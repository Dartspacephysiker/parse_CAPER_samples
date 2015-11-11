#!/bin/bash
#2015/11/11
#Here are the commands I used for doing the FFTs of the dumps:

#Vars and defaults
SAMP_RATE=${2:-1000}
N_PER_FFT=${3:-1024}
N_FFTS_TO_AVG=${4:-1}

if [ $1 ]; then 
    if [ `expr match ${1} "-h"` -eq 2 -o `expr match ${1} "--help"` -eq 6 ]; then
	echo "$0 <File_to_FFT.out> <SAMPLE_RATE IN HZ [${SAMP_RATE}]> <N_SAMPLES_PER_FFT [${N_PER_FFT}]> <N_FFTS_TO_AVG [${N_FFTS_TO_AVG}]> <PDF BASENAME [basename of File_to_FFT.out]>"
	exit
    fi
    PARSED_FILE=${1} 
    PARSED_PREF=${5:-`basename -s .out ${PARSED_FILE}`}
    PARSED_DIR=`dirname ${PARSED_FILE}`
else 
    echo "$0 <File_to_FFT.out> <SAMPLE_RATE IN HZ [${SAMP_RATE}]> <N_SAMPLES_PER_FFT [${N_PER_FFT}]> <N_FFTS_TO_AVG [${N_FFTS_TO_AVG}]>"
    exit
fi

#to the directory
OLDDIR=`pwd`
cd /usr/src/Core-FFT/

echo "Creating ${PARSED_PREF}.pdf..." 
echo "./core-fft.py --ngdef -F ${SAMP_RATE} -N ${N_PER_FFT} -a ${N_FFTS_TO_AVG} ${PARSED_FILE} ${PARSED_PREF}"
echo ""
./core-fft.py --ngdef -F ${SAMP_RATE} -N ${N_PER_FFT} -a ${N_FFTS_TO_AVG} ${OLDDIR}/${PARSED_FILE} ${PARSED_PREF}
gray -d ${PARSED_PREF}.ngdef ${PARSED_PREF}
ps2pdf gray.ps
mv gray.pdf ${OLDDIR}/${PARSED_PREF}.pdf
rm ${PARSED_PREF}.ngdef ${PARSED_PREF} gray.ps

