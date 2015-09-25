#!/usr/bin/awk -f

#2015/09/21
# stats_tstampfile.awk --- Check the time differences between lines, do basic statistics

# Records are separated by newline characters
BEGIN { 
    oldT = $1 
    maxNTimesToTrack = 20
    totalNTimes = 0
}

{
    currentT = $1
    currentDiff = currentT - oldT
#    print "Current T   : ", currentT
#    print "Current diff: ", currentDiff
    oldT = currentT
    if (( currentDiff in tDiffCount ) && totalNTimes > 0 )
    {	
	tDiffCount[currentDiff] = tDiffCount[currentDiff] + 1
	if ( tDiffCount[currentDiff] <= maxNTimesToTrack )
	{
	    tDiffTime[currentDiff,tDiffCount[currentDiff]] = currentT
	    tDiffNum[currentDiff,tDiffCount[currentDiff]] = totalNTimes
	}
    }
    else if ( totalNTimes > 0 )
    {
	tDiffCount[currentDiff] = 1
	tDiffTime[currentDiff,tDiffCount[currentDiff]] = currentT
	tDiffNum[currentDiff,tDiffCount[currentDiff]] = totalNTimes
    }    
    totalNTimes++
}
END {
    printf "-->\n"
    printf "-->Statistics for timestamp file %s\n", ARGV[1]
    printf "-->\n"
    printf "Time diff  | Count     |Time(s) of occurrence     | Occurrence number\n"
    printf "=====================================================================\n\n"
    for (idx in tDiffCount)
    {
	    printf "%9.6f  : %i", idx, tDiffCount[idx]
	    if ( tDiffCount[idx] < maxNTimesToTrack )
	    {
		maxIdx2 = tDiffCount[idx]
		printf "\n"
	    }
	    else
	    {
		maxIdx2 = maxNTimesToTrack
		printf "        (First %i)\n", maxNTimesToTrack
	    }
	    
	    for (idx2 = 1; idx2 <= maxIdx2; idx2++)
		printf "                       %10.6f                   %4i\n", tDiffTime[idx,idx2],tDiffNum[idx,idx2]
	    printf "\n"
	}

	    printf "Total N time differences: %i\n", totalNTimes-1
}
