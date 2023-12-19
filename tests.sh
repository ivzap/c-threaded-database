#!/bin/bash
# clean up any dir before testing and make dir
make all

green="\e[32m"
red="\e[31m"
yellow="\e[33m"
reset="\e[0m"

# Prep
rm -rf received
rm -rf DATABASE

mkdir DATABASE
mkdir received

# TEST 1: (Download small)
echo -e "${yellow}Running Test 1:${reset}"

cd DATABASE
truncate -s 100000 smallDownload.txt

cd ../

./Server -m 1024 -r 8000 2>/dev/null &
sleep 1
cmd="python test.py -r 8000 -a '127.0.0.1' -d 1 -f 'smallDownload.txt'"
echo "${cmd}"
python test.py -r 8000 -a "127.0.0.1" -d 1 -f "smallDownload.txt"
sleep 1
if cmp -s "./received/smallDownload.txt" "./DATABASE/smallDownload.txt"; then
    echo -e "${green}TEST 1 PASSED :D${reset}"
else
    echo -e "${red}TEST 1 FAILED :(${reset}"
fi

kill $! 2>/dev/null
wait $! 2>/dev/null

echo ""

# TEST 2: (Download large)
echo -e "${yellow}Running Test 2:${reset}"

cd DATABASE
truncate -s 50000000 largeDownload.txt

cd ../

./Server -m 1024 -r 8000 2>/dev/null &
sleep 1
cmd="python test.py -r 8000 -a '127.0.0.1' -d 1 -f 'largeDownload.txt'"
echo "${cmd}"
python test.py -r 8000 -a "127.0.0.1" -d 1 -f "largeDownload.txt"
sleep 1
if cmp -s "./received/largeDownload.txt" "./DATABASE/largeDownload.txt"; then
    echo -e "${green}TEST 2 PASSED :D${reset}"
else
    echo -e "${red}TEST 2 FAILED :(${reset}"
fi

kill $! 2>/dev/null
wait $! 2>/dev/null

# Cleanup
rm -rf received
rm -rf DATABASE

exit 0




