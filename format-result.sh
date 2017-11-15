#!/bin/bash

set -e

OUTPUT_DIR=output/$(basename $1 .txt)

[ -d "$OUTPUT_DIR" ] || mkdir -p "$OUTPUT_DIR"

FILE=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f $FILE" EXIT

cat $1 > $FILE

output_file_name()
{
    printf "%s.data\n" "$*"
}

get_unique_names()
{
    cut -d'#' -f2 $1 \
        | sort \
        | uniq 
}

split_file()
{
    get_unique_names $1 \
        | while read NAME
    do
        grep "# $NAME"'$' $FILE \
             > "$OUTPUT_DIR/$(output_file_name $NAME)"
    done
}

print_plot()
{
    local FILE=$1
    local OUTPUT=$2
    local TITLE="$3"
    local COLUMN=$4
    
    printf 'set term png linewidth 2 size 2048,1536 font Arial 20
set output "%s.png"
set xlabel "Maximum word length"
set ylabel "relative speed"
set title "%s"
set xrange [ 2 : 11 ]
set key outside center bottom horizontal Left reverse\n' \
           "$OUTPUT" "$TITLE"
    
    LINE_BEGIN="plot "

    get_unique_names $FILE \
        | while read NAME
    do
        printf '%s "%s" using 1:%s title "%s" with linespoints ps 2' \
               "$LINE_BEGIN" "$(output_file_name $NAME)" "$COLUMN" "$NAME"

        LINE_BEGIN=','
    done

    printf "\n"
}

split_file $1

(
    print_plot $1 "benchmark-forward" \
               "Valid string lookup (1000 runs, baseline=std::unorderd_set<std::string>)" \
               2
    print_plot $1 "benchmark-reverse" \
               "Invalid string lookup (1000 runs, baseline=std::unorderd_set<std::string>)" \
               3
) > $OUTPUT_DIR/bench.plot
