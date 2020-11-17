
ipv6_demo


Writing a ipv6 program is becoming more and more important. 
This program shows how to program for ipv6 use c++ in linux and windows. 
It helps you to know how to process ipv6 in c++.

# how to comile 

for linux, cd into linux
```
g++  server.cpp -o server 
g++  client.cpp -o client 
```
for windows, cd windows  open solution in vs2015 
click build in vs2015 



# how to run program 
```
./server  ::0      9527     #listen ipv6 and ipv4 
./server  0.0.0.0  9527     #listen only on ipv4

./client  ::0        9527   
./client  127.0.0.1  9527
```

if you listen on ::0, ip address shown in server will be 
```
::ffff:127.0.0.1   
```
then ipv6 network connection is OK!




