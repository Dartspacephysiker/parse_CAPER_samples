#!/usr/bin/awk -f

#2015/09/24
# stats_tstamp_searchwordfile.awk --- Check the time and sample differences between lines, do basic statistics

# Records are separated by newline characters
BEGIN { 
    oldT = $1 
    oldS = $2
    maxNTimesToTrack = 20
    totalNTimes = 0
}

{
    currentT = $1
    currentS = $2
    currentDiff = currentT - oldT
    currentSDiff = currentS - oldS
#    print "Current T   : ", currentT
#    print "Current diff: ", currentDiff
    oldT = currentT
    oldS = currentS

    #timestamps
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

    #sample numbers
    if (( currentSDiff in sDiffCount ) && totalNTimes > 0 )
    {	
	sDiffCount[currentSDiff] = sDiffCount[currentSDiff] + 1
	if ( sDiffCount[currentSDiff] <= maxNTimesToTrack )
	{
	    sDiffTime[currentSDiff,sDiffCount[currentSDiff]] = currentS
	    sDiffNum[currentSDiff,sDiffCount[currentSDiff]] = totalNTimes
	}
    }
    else if ( totalNTimes > 0 )
    {
	sDiffCount[currentSDiff] = 1
	sDiffTime[currentSDiff,sDiffCount[currentSDiff]] = currentS
	sDiffNum[currentSDiff,sDiffCount[currentSDiff]] = totalNTimes
    }    

    totalNTimes++
}
END {
    printf "-->\n"
    printf "-->Statistics for searchword timestamp file %s\n", ARGV[1]
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

    printf "\n\n"
    printf "Samp diff  | Count     |Sample num of occurrence  | Occurrence number\n"
    printf "=====================================================================\n\n"
    for (idx in sDiffCount)
    {
	    printf "%9d  : %i", idx, sDiffCount[idx]
	    if ( sDiffCount[idx] < maxNTimesToTrack )
	    {
		maxIdx2 = sDiffCount[idx]
		printf "\n"
	    }
	    else
	    {
		maxIdx2 = maxNTimesToTrack
		printf "        (First %i)\n", maxNTimesToTrack
	    }
	    
	    for (idx2 = 1; idx2 <= maxIdx2; idx2++)
		printf "                       %10d                   %4i\n", sDiffTime[idx,idx2],sDiffNum[idx,idx2]
	    printf "\n"
	}

	    printf "Total N samp differences: %i\n", totalNTimes-1
	    printf "\n"

}
