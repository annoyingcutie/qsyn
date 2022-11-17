#!/bin/bash
source benchmark/qsyn_testing_code/BASH_UTIL.sh

RETURN_CODE=0

function USAGE () {
    printf "%s\t%-18s%-11s%-18s\n" "Usage:" "./RUN_ALL_TEST.sh" "[-Verbose]" "[-NOColor]"
    printf "\t%-9s%-2s%-12s\n" "-Verbose" ":" "Print diff details"
    printf "\t%-9s%-2s%-12s\n" "-NOColor" ":" "Turn off colored output"
    printf "\t%-9s%-2s%-12s\n" "-Help" ":" "Print this help message"
    exit 1
} >&2

if (( $# > 3 )); then 
    USAGE
fi

for TOKEN in "$@"; do
    if [[ $(MATCH_STR "$TOKEN" "-help" 2) == 1 ]]; then 
        USAGE
    fi
done

DOCOLOR=1
DOVERBOSE=0

for TOKEN in "$@"; do
    if [[ $(MATCH_STR "$TOKEN" '-nocolor' 4) == 1 ]]; then 
        if [[ $DOCOLOR == 0 ]]; then
            printf "Error: Extra Option (%s)!!\n" $TOKEN
            exit 1
        fi 
        DOCOLOR=0
    elif [[ $(MATCH_STR "$TOKEN" '-verbose' 2) == 1 ]]; then 
        if [[ $DOVERBOSE == 1 ]]; then
            printf "Error: Extra Option (%s)!!\n" $TOKEN
            exit 1
        fi 
        DOVERBOSE=1
    else 
        printf "Error: illegal option (%s)!!\n" $TOKEN
        exit 1
    fi
done

TAGS="-d"
if [[ $DOVERBOSE == 0 ]]; then TAGS+=" -q"; fi
if [[ $DOCOLOR == 0 ]]; then TAGS+=" -noc"; fi

echo "> Testing commands..."
ALL_TEST_INTERNAL $TAGS
STATUS=$?
PASS_SUMMARY="Passed all ${N_FILES} dofiles."
FAIL_SUMMARY="${STATUS} out of ${N_FILES} dofiles failed."
if [ $STATUS -ne 0 ]; then 
    RETURN_CODE=1;
fi
if [[ $DOCOLOR == 1 ]]; then 
    PASS_SUMMARY="${BOLD}${GREEN}${PASS_SUMMARY}${WHITE}${NORMAL}"
    FAIL_SUMMARY="${BOLD}${RED}${FAIL_SUMMARY}${WHITE}${NORMAL}"
fi
if [[ $STATUS == 0 ]]; then
    echo -e ${PASS_SUMMARY}
else 
    echo -e ${FAIL_SUMMARY}
fi

# echo $RESULT_TXT

printf "\n> Testing functions...\n"
if [[ $DOCOLOR == 1 ]]; then
    ./tests/bin/tests -r compact
else 
    ./tests/bin/tests -r compact | cat
fi

if [ ${PIPESTATUS[0]} -ne 0 ]; then 
    RETURN_CODE=1;
fi

exit ${RETURN_CODE}