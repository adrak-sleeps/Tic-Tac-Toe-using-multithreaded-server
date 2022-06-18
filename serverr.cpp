#include<bits/stdc++.h>
#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<limits.h>
#include<pthread.h>
#include "myQueue.h"
using namespace std;

#define SERVERPORT 8989
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 20

int players=0;

pthread_t thread_pool[THREAD_POOL_SIZE];

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER ;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER ;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

int server_socket, client_socket, addr_size;
SA_IN server_addr, client_addr;

int check(int exp, const char *msg)
{
    if (exp == SOCKETERROR)
    {
        perror(msg);
        exit(1);
    }
    return exp;
}

void msg_to_client(int client, const char* msg){
    check(write(client,msg,strlen(msg)<0),"Error, message not passed to client!");
}

void msg_to_clients(int *client, const char* msg){
    msg_to_client(client[0], msg);
    msg_to_client(client[1], msg);
}

void print_grid(char grid [][3]){

    printf(" %c | %c | %c \n", grid[0][0], grid[0][1], grid[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[1][0], grid[1][1], grid[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[2][0], grid[2][1], grid[2][2]);
}

int get_move(int client){
    msg_to_client(client,"TURN:");

    int msg=0;
    check(msg=read(client,&msg,sizeof(int))<0, "Invalid!\n");
    return msg;
}

int check_valid_move(char grid[][3], int move, int client){
    if(grid[move/3][move%3]==' ' || move==9){
        return 1;
    }
    msg_to_client(client, "INV");
    return -1;
}

void update_grid(char grid[][3], int move, int client){
    grid[move/3][move%3]=(client?'X':'O');
}


int check_grid(char grid[][3], int move){

    int row=move/3;
    int col=move%3;

    if(grid[row][0]==grid[row][1] && grid[row][1]==grid[row][2]){
        printf("Win by row %d\n",row);
        return 1;
    }
    else if(grid[0][col]==grid[1][col] && grid[1][col]==grid[2][col]){
        printf("Win by column %d\n", col);
        return 1;
    }
    else if(move%2!=0){
        if((move==0)||(move==4)||(move==8) && grid[0][0]==grid[1][1] && grid[1][1]==grid[2][2]){
            printf("Win by backslash diagonal\n");
            return 1;
        }

        else if((move==2)||(move==4)||(move==8) && grid[0][2]==grid[1][1] && grid[1][1]==grid[2][0]){
            printf("Win by leading diagonal\n");
            return 1;
        }
    }

    return 0;
}


void* game(void* thread_data){

    int *client=(int*)thread_data;

    char grid[3][3]={{' ',' ',' '},
                    {' ',' ',' '},
                    {' ',' ',' '}};

    printf("Game started!!\n");

    msg_to_clients(client,"START");

    print_grid(grid);

    int prev_player=1, curr_player=0;
    bool game_over=false;
    int no_of_turns=0;

    while(!game_over){

        if(prev_player!=curr_player){
            msg_to_client(client[curr_player+1]%2,"WAIT");
        }

        int valid=0;
        int move=0;

        while(!valid){
            move=get_move(client[curr_player]);
            if(move==-1){
                break;
            }
            printf("Player %d played position %d\n",curr_player,move);

            check(valid=check_valid_move(grid, move, curr_player),"Invalid move!");
        }

        if(move<0){
            printf("Player not available!!");
            break;
        }

        else{
            update_grid(grid, move, curr_player);
            print_grid(grid);

            game_over=check_grid(grid, move);

            if(game_over){
                msg_to_client(client[curr_player],"WIN");
                msg_to_client(client[(curr_player+1)%2],"LOSE");
                printf("Player %d won \n", curr_player);
            }
            else if(no_of_turns==8){
                printf("Draw!!");
                msg_to_clients(client,"DRAW");
                game_over=1;
            }

            prev_player=curr_player;
            curr_player=(curr_player+1)%2;
            no_of_turns++;
        }
    }

    printf("Game over!");

    close(client[0]);
    close(client[1]);

    pthread_mutex_lock(&lock1);
    players-=2;
    printf("Number of players is now: %d", players);
    pthread_mutex_unlock(&lock1);

    free(client);
    pthread_exit(NULL);
}

void get_clients(int server, int* client){

    socklen_t client_len;
    
    int num_conn=0;
    while(num_conn<2){
        listen(server, 20-players);
        memset(&client_addr,0,sizeof(client_addr));

        client_len=sizeof(client_addr);

        check(client[num_conn]=accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),"Error connecting to client!");
        write(client[num_conn],&num_conn,sizeof(int));

        pthread_mutex_lock (&lock1);        
        players++;
        printf("Number of players now: %d\n",players);
        pthread_mutex_unlock(&lock1);

        if(num_conn==0){
            msg_to_client(client[0],"HLD");
        }
        num_conn++;
    }
}

void* thread_function(void* args){
     while(true){
        int *pclient ;
        pthread_mutex_lock(&lock1) ;
        if((pclient = dequeue()) == NULL){
            pclient = dequeue() ;
        }
        pthread_mutex_unlock(&lock1) ;
        if(pclient != NULL){
            game(pclient) ;
        }
    }
}

int main(int argc, char **argv){

    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    //SETTING UP THE SOCKET
    check(server_socket=socket(AF_INET, SOCK_STREAM, 0),"Setting server failed!\n");

    //BINDING THE SERVER
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(SERVERPORT);
    server_addr.sin_addr.s_addr=INADDR_ANY;

    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)),"Binding failed!\n");
    
    //LISTENING

    check(listen(server_socket, SERVER_BACKLOG),"Listening failed!\n");


    printf("Server set up succesfully!\n Waiting for connections!\n");

    while(true){

        if(players<=20){
            int *client=(int*)malloc(2*sizeof(int));
            memset(client,0,2*sizeof(int));

            get_clients(server_socket, client);
            printf("Both players ready!\n");

            pthread_mutex_lock(&lock1);
            enqueue(client);
            pthread_mutex_unlock(&lock1);

            // pthread_t thr;
            // pthread_create(&thr,NULL,game,(void*)client);

        }
    }

    close(server_socket);

    return 0;
}