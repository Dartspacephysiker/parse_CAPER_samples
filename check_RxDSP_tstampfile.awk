#!/usr/bin/awk -f

#2015/09/21
# check_RxDSP_tstampfile.awk --- Check the time differences between lines, do basic statistics

# Records are separated by newline characters
BEGIN { 
    oldT = $1 
    maxNTimesToTrack = 20
}

{
    currentT = $1
    currentDiff = currentT - oldT
#    print "Current T   : ", currentT
#    print "Current diff: ", currentDiff
    oldT = currentT
    if (currentDiff in tDiffCount)
    {	
	tDiffCount[currentDiff] = tDiffCount[currentDiff] + 1
#	tDiffCount[currentDiff] = tDiffCount[currentDiff]
	if ( tDiffCount[currentDiff] <= maxNTimesToTrack )
	    tDiffTime[currentDiff,tDiffCount[currentDiff]] = currentT
    }
    else
    {
	tDiffCount[currentDiff] = 1
	tDiffTime[currentDiff,tDiffCount[currentDiff]] = currentT
    }    
}
END {
    totalTimes = 0

    printf "Stats for Rx-DSP file %s\n\n", ARGV[1]
    printf "Time diff  | Count     |Time(s) of occurrence\n"
    printf "=============================================\n\n"
    for (idx in tDiffCount)
    {
	totalTimes += tDiffCount[idx]
	printf "%9.6f  : %i", idx, tDiffCount[idx]
	if ( tDiffCount[idx] < maxNTimesToTrack )
	{
	    maxIdx2 = tDiffCount[idx]
	    printf "\n"
	}
	else
	{
	    maxIdx2 = maxNTimesToTrack
	    printf "         (First %i)\n", maxNTimesToTrack
	}

	for (idx2 = 1; idx2 <= maxIdx2; idx2++)
	    printf "                        %10.6f\n", tDiffTime[idx,idx2]
	printf "\n"
    }
    printf "Total times: %i\n", totalTimes
}
