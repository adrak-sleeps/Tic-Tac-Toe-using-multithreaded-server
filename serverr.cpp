#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "myQueue.h"

#define THREAD_POOL_SIZE 20
#define SOCKETERROR (-1)

//Defining the server port as 8989 by default
#define SERVERPORT 8989

pthread_t thread_pool[THREAD_POOL_SIZE];

int players = 0;
pthread_mutex_t lockx;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

//Function to check an expression and display a message if error value is SOCKETERROR.
int check(int exp, const char *msg)
{
    if (exp == SOCKETERROR)
    {
        perror(msg);
        exit(1);
    }
    return exp;
}

//Function to display message if error occurs, error value can be anything.
void error(const char *msg)
{
    perror(msg);
    pthread_exit(NULL);
}

//Function to send integer message like current_move, or number_of_players to players.
void int_msg_to_client(int client, int msg)
{
    int n = write(client, &msg, sizeof(int));
    check(n,"Error writing int to client");
}

//Function to send integer message to both players
void int_msg_to_clients(int * client, int msg)
{
    int_msg_to_client(client[0], msg);
    int_msg_to_client(client[1], msg);
}


//Sending string message of length 3 to client.
/*Abbreviations used: 
START-"SRT" to denote beginning of game 
HOLD-"HLD" to ask player 1 to wait for player 2 to arrive
TURN-"TRN" to denote turn of a player
INVALID_MOVE-"INV" to denote invalid move (value not inside tic-tac-toe)
COUNT-"CNT" to denote count of active players
UPDATE-"UPD" to update both players about the stats
WAIT-"WAT" to ask player to wait while the other player plays his turn
WIN-"WIN" sent to the winning player
LOOSE-"LSE" sent to the player who lost
DRAW-"DRW" sent if match ends in a draw
*/

void msg_to_client(int client, const char * msg)
{
    int n = write(client, msg, strlen(msg));
    check(n,"Error writing message to client");
}


//Sending string message of length 3 to both the clients.
void msg_to_clients(int * client, const char * msg)
{
    msg_to_client(client[0], msg);
    msg_to_client(client[1], msg);
}


//Function to receive message from player, ie, the move played by him.
//Returns -1 for invalid move
int recv_int(int client)
{
    int msg = 0;
    int n = read(client, &msg, sizeof(int));
    
    if (n < 0 || n != sizeof(int)){
        return -1;
    }
    
    return msg;
}

//Function to get two clients and fix them in an array, so that they both can be assigned to one thread.
void get_clients(int server, int * client)
{
    socklen_t client_len;
    SA_IN server_addr, client_addr;

    int num_conn = 0;

    //Listening until we get 2 clients
    while(num_conn < 2)
    {
        //Listening by the server, and having a backlog of 50-current_players. Other incoming players will be refused.
	    listen(server, 50 - players);
        
        //Alloting space for client_address
        memset(&client_addr, 0, sizeof(client_addr));

        client_len = sizeof(client_addr);

        //Alloting the incoming client a space in the client array.
        client[num_conn] = accept(server, (SA*) &client_addr, &client_len);
    
        check(client[num_conn],"Error connecting to client\n");
        
        //Client 0 is marked as 0, client 1 is marked as 1.
        write(client[num_conn], &num_conn, sizeof(int));
        
        //This section of code is locked to prevent haphazard changes in value of players.
        pthread_mutex_lock(&lockx);

        //Number of players is updated after client is successfully alloted space in the client array.
        players++;
        printf("Number of players is now %d.\n", players);

        pthread_mutex_unlock(&lockx);

        //If only one connection has been made yet, we ask the player to wait for another player to join.
        if (num_conn == 0) {
            msg_to_client(client[0],"HLD");
        }

        num_conn++;
    }
}

//Function to alert a player about his turn, and to receive his move.
int get_move(int client)
{
    msg_to_client(client, "TRN");
    return recv_int(client);
}

//Function to check if the move is valid or not.
//If move is 9, we have to display the number of active players.
//If the requested move if empty on the grid, it is valid.
//In all other cases, move is invalid.

int check_valid_move(char board[][3], int move, int player_id)
{
    if ((move == 9) || (board[move/3][move%3] == ' ')) { 
        return 1;
   }
   return 0;
}

//Updating the grid with the required move.
//Player 0 is alloted O and player 1 is alloted X by default.

void update_grid(char grid[][3], int move, int player_id)
{
    grid[move/3][move%3] = player_id ? 'X' : 'O';
    
}

//Displaying the grid after every move.
void print_grid(char grid [][3]){

    printf(" %c | %c | %c \n", grid[0][0], grid[0][1], grid[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[1][0], grid[1][1], grid[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[2][0], grid[2][1], grid[2][2]);
}

//Function to alerts players about the current player and the move played by that player.
void send_update(int * client, int move, int player_id)
{
    
    msg_to_clients(client, "UPD");
    int_msg_to_clients(client, player_id);
    int_msg_to_clients(client, move);
    
}

//Function to alert player about the number of active players(<=50).
void send_player_count(int client)
{
    msg_to_client(client, "CNT");
    int_msg_to_client(client, players);
}

//Checking the grid after a player plays his move, to check if he wins.
//Returns 1 if he wins; 0 if games ends in draw, or continues.
int check_grid(char grid[][3], int move){

    int row=move/3;
    int col=move%3;

    //If all elements of a row are same, results in a win.
    if(grid[row][0]==grid[row][1] && grid[row][1]==grid[row][2]){
        printf("Win by row %d\n",row);
        return 1;
    }

    //If all elements of a column are same, results in a win.
    else if(grid[0][col]==grid[1][col] && grid[1][col]==grid[2][col]){
        printf("Win by column %d\n", col);
        return 1;
    }

    else if(!(move%2)){
        if(((move==0)||(move==4)||(move==8)) && (grid[0][0]==grid[1][1] && grid[1][1]==grid[2][2])){
            printf("Win by backslash diagonal\n");
            return 1;
        }

        else if(((move==2)||(move==4)||(move==8)) && (grid[0][2]==grid[1][1] && grid[1][1]==grid[2][0])){
            printf("Win by leading diagonal\n");
            return 1;
        }
    }

    return 0;
}

void* game(void* client_data){

    int *client=(int*)client_data;

    //Empty grid of tic-tac-toe in the beginning.
    char grid[3][3]={{' ',' ',' '},
                    {' ',' ',' '},
                    {' ',' ',' '}};

    printf("Game started!!\n");

    //Both players are alerted about the game start.
    msg_to_clients(client,"SRT");

    print_grid(grid);

    int prev_player=1, curr_player=0;
    int game_over=0;
    int no_of_turns=0;

    while(!game_over){

        if(prev_player!=curr_player){
            msg_to_client(client[(curr_player+1)%2],"WAT");
        }

        //To check validity of move.
        int valid=0;
        //To receive the move from the client.
        int move=0;

        while(!valid){
            move=get_move(client[curr_player]);
            if(move==-1){
                break;
            }
            printf("\nPlayer %d played position %d\n",curr_player,move);

            //Checking if move is valid, ie, if the specified space in the grid is already filled or not.
            //If already filled, the player is asked to play his move again.

            valid = check_valid_move(grid, move, curr_player);
            if (!valid) { 
                printf("Invalid move! Try again!\n");
                msg_to_client(client[curr_player], "INV");
            }
        }

        if(move<0){
            printf("Player not available!!\n");
            break;
        }

        else if (move == 9) {
            prev_player=curr_player;
            send_player_count(client[curr_player]);
        }

        else{
            //If move is valid, the grid is updated and printed.
            update_grid(grid, move, curr_player);
            send_update( client, move, curr_player );
            print_grid(grid);

            //To check if the grid has a column or row or diagonal of 'X's or 'O's.
            //If yes, the winner and loser players are informed.
            game_over=check_grid(grid, move);

            if(game_over==1){
                msg_to_client(client[curr_player],"WIN");
                msg_to_client(client[(curr_player+1)%2],"LSE");
                printf("Player %d won \n", curr_player);
            }
            //If number of turns are exhausted, the match ends in a draw.
            else if(no_of_turns==8){
                printf("Draw!!\n");
                msg_to_clients(client,"DRW");
                game_over=1;
            }

            //The other player becomes the current player and the number of turns are incremented.
            prev_player=curr_player;
            curr_player=(curr_player+1)%2;
            no_of_turns++;
        }
    }

    printf("Game over!\n");

    //Both the client sockets are closed.
    close(client[0]);
    close(client[1]);

    //This section of code is locked so that the number of players are changed simultaneously but multiple threads.
    pthread_mutex_lock(&lockx);
    players-=2;
    printf("Number of players is now: %d\n", players);
    pthread_mutex_unlock(&lockx);

    //Space allocated to client array is freed.
    free(client);
    pthread_exit(NULL);
}


void* thread_function(void* args){

    //Running a for loop to check for incoming clients in the queue at all times
     while(true){
        int *pclient ;

        //Locking this part of code so that multiple clients do not get dequeued at once.
        pthread_mutex_lock(&lockx) ;

        if((pclient = dequeue()) == NULL){
            pclient = dequeue() ;
        }
        pthread_mutex_unlock(&lockx) ;

        //If client is not NULL, the game begins.
        if(pclient != NULL){
            game(pclient) ;
        }
    }
}


int main(int argc, char *argv[])
{   
    int server_socket;
    SA_IN server_addr;

    //SETTING UP THE SERVER SOCKET
    check(server_socket=socket(AF_INET, SOCK_STREAM, 0),"Setting server failed!\n");

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(SERVERPORT);
    server_addr.sin_addr.s_addr=INADDR_ANY;

    //BINDING THE SERVER
    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)),"Binding failed!\n");

    printf("Server active now!\n");

    //Initializing all the threads in the thread pool and sending them to thread_function to wait for clients.
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    //Initialising the lock as NULL.
    pthread_mutex_init(&lockx, NULL);

    //Running a while loop to check for incoming clients at all times
    while (true) {

        //Setting an uper limit for number of players, as 50.
        if (players <= 50) {   

            //Alloting space for 2 clients
            int *client_socket = (int*)malloc(2*sizeof(int)); 
            memset(client_socket, 0, 2*sizeof(int));
            
            //Listening for clients and alloting them space in client_socket
            get_clients(server_socket, client_socket);

            //After we get 2 clients, we tie them together, and enqueue them, so that they can be dequeued and alloted to a thread.
            //Locking this section to prevent multiple threads from accessing it at once.
            pthread_mutex_lock(&lockx);
            enqueue(client_socket);
            pthread_mutex_unlock(&lockx);
        
            printf("New game thread started.\n");
            }
    }

    //Closing the server after limit of 50 players is crossed.
    close(server_socket);

    //Destroying the lock
    pthread_mutex_destroy(&lockx);
    pthread_exit(NULL);
}
