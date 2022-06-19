#include "myQueue.h"
#include <bits/stdc++.h>
#include <stdlib.h>

node_t* front = NULL ;
node_t* rear = NULL ;

// Return NULL when queue is empty.

//Adding a socket to queue.
void enqueue(int *client_socket){
    node_t *newnode = (node_t *)malloc(sizeof(int)) ;
    newnode->client_socket = client_socket ;
    newnode->next = NULL ;
    if(rear == NULL){
        front = newnode ;
    }
    else{
        rear->next = newnode ;
    }
    rear = newnode ;
}

//Removing a socket from queue and returning it.
int* dequeue(){
    if(front == NULL){
        return NULL ;
    }
    else{
        int *result = front->client_socket ;
        node_t *temp = front ;
        front = front -> next ;
        if(front == NULL){
            rear = NULL ;
        }
        free(temp) ;
        return result ;
    }
}