# Tic-Tac-Toe-using-Multithreaded-Server

## Project Description

This project shows an implementation of a multithreaded client-server model, customised as a two-player Tic-Tac-Toe game. The server creates the game in Local Area Network. Clients in the Local Area Network can join in simultaneously by coonecting to the IP address of the server and have two-player matches. 

I have tried to implement the concept of multithreading by assigning two clients to every thread of a thread pool (which has a definite number of threads), since a single thread will allow only one match to take place at a time. Multiple thread will allow concurrent matches to occur, thereby making our server responsive and fast.

## How to run the Project?

This project has implementation of threads, implying it should be run on a linux (or any of its dependency's) terminal. The system shoud also have GCC so that C++ files can be compiled and run. 

The file "server.cpp" should be compiled and run on terminal as:

>`g++ -o server server.cpp myQueue.cpp -lpthread`

>`./server`

Open two new terminals, compile and run "client.cpp" on both of them as:

>`g++ -o client client.cpp`

>`./client`

## Internal Working of the Project alongwith the Additional Task



## My learnings from the Project



## Demo of the Project

## Resources used:
