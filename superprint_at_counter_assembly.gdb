break parse_CAPER_samples.c:912
  commands
    printf "0x%llX 0x%llX 0x%llX 0x%llX: 0x%llX\n%llu %llu %llu %llu: %llu\n\n",paullMajorFrameVals[0],paullMajorFrameVals[1],paullMajorFrameVals[2],llMinorFrameIdx,ullCounterVal,paullMajorFrameVals[0],paullMajorFrameVals[1],paullMajorFrameVals[2],llMinorFrameIdx,ullCounterVal
    printf "Shift 5 then 6...\n"
    p/t (paullMajorFrameVals[2] << (20+uMinorFrameBitShift)) | (paullMajorFrameVals[1] << (10+uMinorFrameBitShift)) | (paullMajorFrameVals[0] << uMinorFrameBitShift) | llMinorFrameIdx & 0b1111111111
    p/t (paullMajorFrameVals[2] << (21+uMinorFrameBitShift)) | (paullMajorFrameVals[1] << (11+uMinorFrameBitShift)) | (paullMajorFrameVals[0] << 1+uMinorFrameBitShift) | llMinorFrameIdx & 0b1111111111
    printf "\n"
    p/d (paullMajorFrameVals[2] << (20+uMinorFrameBitShift)) | (paullMajorFrameVals[1] << (10+uMinorFrameBitShift)) | (paullMajorFrameVals[0] << uMinorFrameBitShift) | llMinorFrameIdx & 0b1111111111
    p/d (paullMajorFrameVals[2] << (21+uMinorFrameBitShift)) | (paullMajorFrameVals[1] << (11+uMinorFrameBitShift)) | (paullMajorFrameVals[0] << 1+uMinorFrameBitShift) | llMinorFrameIdx & 0b1111111111
  end
break parse_CAPER_samples.c:441
