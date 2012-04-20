#!/bin/bash

(( nd = 5 ))
while getopts :d:n OPT; do
    case $OPT in
        d)
            (( nd = $OPTARG ))
            ;;
        n)
            (( want_numawrap = 1 ))
            ;;
        *)
            echo "usage: ${0##*/} [-d #] [-n] [--] <hosts-file> <fcl-file>"
            exit 2
    esac
done
shift $(( OPTIND - 1 ))
OPTIND=1

hosts="$1"
fcl="$2"

(( np = $(cat "$hosts" | wc -l) ))
(( nb = np - 2 * nd ))
declare -a numacmd=("$ARTDAQ_BUILD/bin/numawrap" "-v" "-H" "$hosts" "--ranks=0-$((2 * nd - 1))" "--")

set -x
mpirun_rsh --rsh -np $np -hostfile "$hosts" \
"PRINT_HOST=1" \
"ART_OPENMP_DIR=$ART_OPENMP_DIR" \
"FHICL_FILE_PATH=\"$FHICL_FILE_PATH\"" \
"LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH\"" \
"${numacmd[@]}" \
"$ARTDAQ_BUILD/bin/builder" \
$nd $nb 1 1 -- -c "$fcl"
(( status = $? ))
set +x

exit $status
