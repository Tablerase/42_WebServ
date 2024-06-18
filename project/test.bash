#!/bin/bash

# ANSI colors
GREEN='\033[0;32m'
WHITE='\033[0;37m'
BLUE='\033[0;34m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
RESET='\033[0m'
	# ANSI Bold
BOLDGREEN='\033[1;32m'
BOLDWHITE='\033[1;37m'
BOLDBLUE='\033[1;34m'
BOLDRED='\033[1;31m'
BOLDORANGE='\033[1;33m'

# ! Errors to review/fix:
# 1. Nothing append in case of empty conf file (no error, no warning)
# 2. More than 1000 servers (not working - too busy - auto stop - no leaks)
    # Error msg: Could Not Start Server because of Bad file descriptor


# Build the web server
make re

# Number of servers
N=${1:-4} # The first argument passed to the script

# Start of the file
echo "" > test.conf

# Loop to create each server block
for (( i=1; i<=N; i++ ))
do
    echo "server {" >> test.conf
    echo "    listen $((i+8000));" >> test.conf
	echo "    server_name localhost;" >> test.conf
	echo "    location / {" >> test.conf
	echo "        root ./web/pages;" >> test.conf
	echo "        index index.html;" >> test.conf
	echo "    }" >> test.conf
    echo "}" >> test.conf
    echo "" >> test.conf
done

# Start the web server in the background
./WebServ "test.conf" & # & at the end to run the process in the background
pid=$! # $! is the PID of the last background process

sleep 1
# Run the tests

# Test 1
echo -e "${BOLDBLUE} Test 1: ${RESET}"
# Iterate over the servers and test each one of them with curl
test1=1
for (( i=1; i<=N; i++ ))
do
	curl -s http://localhost:$((i+8000))/index.html | diff - web/pages/index.html
	if [ $? -eq 0 ]; then
		test1=$((test1+1))
	else
		echo -e "${RED} Test $((i+1)) failed ${RESET}"
	fi
done

if [ $test1 -eq $((N+1)) ]; then
	echo -e "${GREEN} All tests passed ${RESET}"
fi

# Sleep
# sleep 4

# Stop the web server
kill $pid

# Clean up
rm -f test.conf