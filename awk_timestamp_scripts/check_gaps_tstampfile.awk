#!/usr/bin/awk -f

#2015/09/25
# check_gaps_tstampfile.awk --- Are these time differences acceptable?

# Records are separated by newline characters
BEGIN { 
    oldT             = $1      #Start time
    oldL             = $0      #Start line

    maxTDiff         = 0.005   #Max time between samples

    totalNTimes      = 0
    maxNTimesToTrack = 50

    lastGapT         = -1
    lastGapNR        = -1
}

{
    #Handle times
    currentT = $1
    currentDiff = currentT - oldT

    #Handle lines
    currentL = $0

    if ( currentDiff > maxTDiff )
    {
	# printf "****************\n"
	# printf "%8d: %s\n", NR-1, oldL
	# printf "%8d: %s\n", NR  , currentL
	# printf "Diff: %10.8f\n", currentDiff
	# if ( lastGapT != -1 )
	# {
	#     printf "gapTDiff, gapLDiff: %10.8f, %8d\n", (currentT - lastGapT), (NR - lastGapNR)
	# }

	#do some stats on this
	if (( currentDiff in tDiffCount ) && totalNTimes > 0 )
	{	
	    tDiffCount[currentDiff] = tDiffCount[currentDiff] + 1
	    if ( tDiffCount[currentDiff] <= maxNTimesToTrack )
	    {
		tDiffTime[currentDiff,tDiffCount[currentDiff]] = currentT
		tDiffNum[currentDiff,tDiffCount[currentDiff]]  = totalNTimes
		gDiffNum[currentDiff,tDiffCount[currentDiff]]  = (NR - lastGapNR)
	    }
	}
	else if ( totalNTimes > 0 )
	{
	    tDiffCount[currentDiff] = 1
	    tDiffTime[currentDiff,tDiffCount[currentDiff]] = currentT
	    tDiffNum[currentDiff,tDiffCount[currentDiff]]  = totalNTimes
	    gDiffNum[currentDiff,tDiffCount[currentDiff]]  = (NR - lastGapNR)
	}    
	
	#update gap values
	lastGapT = currentT
	lastGapNR = NR

	totalNTimes++
    }


    #Set up the old guys
    oldT = currentT
    oldL = currentL

}

END {
    printf "-->\n"
    printf "-->Statistics of large time gaps for timestamp file %s\n", ARGV[1]
    printf "-->Max time difference: %12.8f\n", maxTDiff
    printf "-->\n"
    printf "gapT diff  | Count     |Occurrence time  | Gap between this and last | Occurrence num\n"
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
		printf "                       %10.6f      %10d            %4i\n", tDiffTime[idx,idx2], gDiffNum[idx,idx2], tDiffNum[idx,idx2]
	    printf "\n"
	}
	    printf "Total N large time gaps: %i\n", totalNTimes-1
}
