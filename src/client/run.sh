#!/bin/bash

# Example run - checkout ./load -h and ./sevault -h to configure benchmarks
./load -i 127.0.0.1 -p 11211 -k 16 -v 16 -t 1
sudo ./sevault -i 127.0.0.1 -p 11211 -k 16 -v 16 -t 1 -n 100 -r 100

#./load -i 127.0.0.1 -p 12345 -k 16 -v 16 -t 4
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 16 -t 4 -n 100000 -r 100
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 16 -t 4 -n 100000 -r 90
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 16 -t 4 -n 100000 -r 50
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 16 -t 4 -n 100000 -r 10

#./load -i 127.0.0.1 -p 12345 -k 16 -v 128 -t 4
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 128 -t 4 -n 100000 -r 100
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 128 -t 4 -n 100000 -r 90
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 128 -t 4 -n 100000 -r 50
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 128 -t 4 -n 100000 -r 10

#./load -i 127.0.0.1 -p 12345 -k 16 -v 512 -t 4
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 512 -t 4 -n 100000 -r 100
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 512 -t 4 -n 100000 -r 90
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 512 -t 4 -n 100000 -r 90
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 512 -t 4 -n 100000 -r 10

#./load -i 127.0.0.1 -p 12345 -k 16 -v 1024 -t 4
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 1024 -t 4 -n 100000 -r 100
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 1024 -t 4 -n 100000 -r 90
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 1024 -t 4 -n 100000 -r 50
#sudo ./sevault -i 127.0.0.1 -p 12345 -k 16 -v 1024 -t 4 -n 100000 -r 10

#./load -i 127.0.0.1 -p 12345 -k 16 -v 2048 -t 4
#sudo ./base -i 127.0.0.1 -p 12345 -k 16 -v 2048 -t 4 -n 100000 -r 90
#./load -i 127.0.0.1 -p 12345 -k 16 -v 4096 -t 4
#sudo ./base -i 127.0.0.1 -p 12345 -k 16 -v 4096 -t 4 -n 100000 -r 90

