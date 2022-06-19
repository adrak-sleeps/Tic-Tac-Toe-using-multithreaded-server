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

>`./client 88.88.88.88 8989`

where 88.88.88.88 is the local host's IP address and 8989 is the port we've alloted to the server.

## Internal Working of the Project alongwith the Additional Task



## My learnings from the Project

This project gave me a good oppotunity to learn about client-servers and socket programming, as well as the use of threads and multithreading. Through this project, I got to learn how threads can be slow while working for a large number of clients. Multithreading is a good solution to make a server fast and responsive. A definite number of threads serve clients concurrently and after their job is done, they wait for their next clients.

Also, this project has to be run on linux dependencies, thereby providing me with a chance to learn more about WSL and how to operate it. 

## Demo of the Project

## Resources used:

During the implementation of this project, I referred to the following resources:
1. [ACM's workshop] (https://github.com/acmiitr/Multithreaded-Server)
2. [Multithreading basics] (https://totalview.io/blog/multithreading-multithreaded-applications#:~:text=Multiple%20threads%20of%20execution%20are,with%20is%20a%20word%20processor.)
3. [Multithreading coding basics on GFG] (https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/)
4. [Flylib's tutorial] (https://flylib.com/books/en/2.254.1/client_server_tic_tac_toe_using_a_multithreaded_server.html)
