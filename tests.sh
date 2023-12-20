#!/bin/bash
# clean up any dir before testing and make dir


if [ "$EUID" -ne 0 ]; then
    echo "Please run this script with sudo."
    exit 1
fi

make all

cwd=$(pwd)
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

sudo ./Server -m 1024 -r 8000 2>/dev/null &
sleep 1
cmd="python3 test.py -r 8000 -a '127.0.0.1' -d 1 -f 'smallDownload.txt'"
echo "${cmd}"
python3 test.py -r 8000 -a "127.0.0.1" -d 1 -f "smallDownload.txt"
sleep 1
if cmp -s "./received/smallDownload.txt" "./DATABASE/smallDownload.txt"; then
    echo -e "${green}TEST 1 PASSED :D${reset}"
else
    echo -e "${red}TEST 1 FAILED :(${reset}"
fi

echo ""

# TEST 2: (Download large)
echo -e "${yellow}Running Test 2:${reset}"

cd DATABASE
truncate -s 50000000 largeDownload.txt

cd ../

sudo ./Server -m 1024 -r 8000 2>/dev/null &
sleep 1
cmd="python3 test.py -r 8000 -a '127.0.0.1' -d 1 -f 'largeDownload.txt'"
echo "${cmd}"
python3 test.py -r 8000 -a "127.0.0.1" -d 1 -f "largeDownload.txt"
sleep 1
if cmp -s "./received/largeDownload.txt" "./DATABASE/largeDownload.txt"; then
    echo -e "${green}TEST 2 PASSED :D${reset}"
else
    echo -e "${red}TEST 2 FAILED :(${reset}"
fi

echo ""

# TEST 3: (small text file upload)
echo -e "${yellow}Running Test 3:${reset}"
sudo ./Server -m 1024 -r 8000 2>/dev/null &
sleep 1 
cmd="python3 test.py -r 8000 -a '127.0.0.1' -u 1 -p '$cwd/testing/uploadSmall.txt' -f 'uploadSmall.txt'"
echo "${cmd}"
python3 test.py -r 8000 -a "127.0.0.1" -u 1 -p "$cwd/testing/uploadSmall.txt" -f "uploadSmall.txt"
sleep 1
if cmp -s "./testing/uploadSmall.txt" "./DATABASE/uploadSmall.txt"; then
    echo -e "${green}TEST 3 PASSED :D${reset}"
else
    echo -e "${red}TEST 3 FAILED :(${reset}"
fi

echo ""

# TEST 4: (jpg cat upload)
echo -e "${yellow}Running Test 4:${reset}"

sudo ./Server -m 1024 -r 8000 2>/dev/null &
sleep 1
cmd="python3 test.py -r 8000 -a '127.0.0.1' -u 1 -p '$cwd/testing/cat.jpg' -f 'cat.jpg'"
echo "${cmd}"
sudo python3 test.py -r 8000 -a "127.0.0.1" -u 1 -p "$cwd/testing/cat.jpg" -f "cat.jpg"
sleep 1
if cmp -s "./testing/cat.jpg" "./DATABASE/cat.jpg"; then
    echo -e "${green}TEST 4 PASSED :D${reset}"
else
    echo -e "${red}TEST 4 FAILED :(${reset}"
fi

echo ""

# TEST 5: (jpg apple upload)
echo -e "${yellow}Running Test 5:${reset}"

sudo ./Server -m 1024 -r 8000 2>/dev/null &
sleep 1
cmd="python3 test.py -r 8000 -a '127.0.0.1' -u 1 -p '$cwd/testing/apple.jpg' -f 'apple.jpg'"
echo "${cmd}"
python3 test.py -r 8000 -a "127.0.0.1" -u 1 -p "$cwd/testing/apple.jpg" -f "apple.jpg"
sleep 1
if cmp -s "./testing/apple.jpg" "./DATABASE/apple.jpg"; then
    echo -e "${green}TEST 5 PASSED :D${reset}"
else
    echo -e "${red}TEST 5 FAILED :(${reset}"
fi


# Cleanup
rm -rf received
rm -rf DATABASE

exit 0




