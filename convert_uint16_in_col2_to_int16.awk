#!/usr/bin/awk -f

#2015/09/21
# convert_uint16_in_col2_to_int16.awk --- Check the time differences between lines, do basic statistics

# Records are separated by newline characters
BEGIN { 
    max=2 ** 15
    range = 2 ** 16
}

{
    col1 = $1
    col2 = $2

    if ( col2 > max )
	col2 = col2 - max
    else if ( col2 < max )
	col2 = col2 + max

    printf "%12.8f\t%5d\n",col1,col2
}
END {

}
