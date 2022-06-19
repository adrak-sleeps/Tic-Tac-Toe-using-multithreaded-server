#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

//Function to display message if error occurs.
void error(const char *msg)
{
    perror(msg);
    printf("Server not available!\n");
    exit(0);
}

//Function to pass a message, for example, desired move, to server
void msg_to_server(int socket, int msg)
{ 
    if (write(socket, &msg, sizeof(int)) < 0){
        error("Error writing in to server");
    }
}

//Function to receive message from server, regarding moves or result of the game.
void receive_msg(int socket, char * msg)
{
    memset(msg, 0, 4);
    int n = read(socket, msg, 3);
    
    if (n < 0 || n != 3){
        error("Error receiving message from server!\n");
    }
}

//Function to receive integer like player id or number of players from the server.
int receive_int(int socket)
{
    int msg = 0;
    int n = read(socket, &msg, sizeof(int));
    
    if (n < 0 || n != sizeof(int)) {
        error("Error receiving int from server");
    }
    return msg;
}


//Function to connect client to server.
int connect_to_server(char * hostname, int portno)
{
    SA_IN serv_addr;

    //Here, server is the host.
    struct hostent *server;
    
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0){
        printf("Error getting socket\n");
    }
	
    //Getting details about the host
    server = gethostbyname(hostname);
	
	memset(&serv_addr, 0, sizeof(serv_addr));

    //Setting address details of the server
    serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno); 

    //Connecting sock to the server
    if(connect(sock, (SA*) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR connecting to server");
    }

    //Sock is now connected to the host (server)
    return sock;
}

void print_grid(char grid [][3]){

    printf(" %c | %c | %c \n", grid[0][0], grid[0][1], grid[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[1][0], grid[1][1], grid[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[2][0], grid[2][1], grid[2][2]);
}

void make_move(int sock)
{
    //Char array to store moves
    char buffer[10];
    
    while (true) { 
        printf("Enter 0-8 to make a move, or 9 for number of active players: ");

        //Input to be given by the player
	    fgets(buffer, 10, stdin);
	    int move = buffer[0] - '0';
        if (move <= 9 && move >= 0){
            printf("\n");

            //Sending the move to the server, if valid.
            msg_to_server(sock, move);   
            break;
        } 
        else{
            printf("\nInvalid input. Try again.\n");
        }
    }
}

void get_update(int sock, char grid[][3])
{
    //Receiving update about which player played his move right now, and what was the move.
    //Accordingly, the grid is updated.
    int player_id = receive_int(sock);
    int move = receive_int(sock);

    grid[move/3][move%3] = (player_id ? 'X' : 'O');    
}

void game(int sock, int id)
{
    //Reserving space for any message to be received from the server.
    char msg[4];

    //The initial grid.
    char grid[3][3] = { {' ', ' ', ' '}, 
                         {' ', ' ', ' '}, 
                         {' ', ' ', ' '} };

    printf("Tic-Tac-Toe\n------------\n");

    do {

        receive_msg(sock, msg);

        //Waiting until another client joins in for the game
        if (!strcmp(msg, "HLD")){
            printf("Waiting for a second player...\n");
        }
            
    } while (strcmp(msg,"SRT"));

   
    printf("Game begins!\n");

    //The first player plays 'O' and second player plays 'X' by default
    printf("You have to play %c's\n", id ? 'X' : 'O');

    print_grid(grid);

    //Running the for loop infinitely to check for messages at all times
    while(true) {

        receive_msg(sock, msg);

        if (!strcmp(msg, "TRN")) { 
	        printf("Your turn:\n");
	        make_move(sock);
        }
        else if (!strcmp(msg, "INV")) { 
            printf("That position is already filled. Try again.\n"); 
        }
        else if (!strcmp(msg, "CNT")) { 
            int num_players = receive_int(sock);
            printf("%d players are active.\n", num_players); 
        }
        else if (!strcmp(msg, "UPD")) { 
            get_update(sock, grid);
            print_grid(grid);
        }
        else if (!strcmp(msg, "WAT")) { 
            printf("Waiting for other player's move.\n");
        }
        else if (!strcmp(msg, "WIN")) { 
            printf("You win!\n");
            break;
        }
        else if (!strcmp(msg, "LSE")) { 
            printf("You lost :(\n");
            break;
        }
        else if (!strcmp(msg, "DRW")) { 
            printf("Draw.\n");
            break;
        }
        else {
            error("Unknown message!!\n");
        }
    }
}

int main(int argc, char *argv[])
{
      if(argc<3){
       error("Enter valid host!");
       exit(0);
    }

    int sock = connect_to_server(argv[1], atoi(argv[2]));
    int id = receive_int(sock); 

    game(sock,id);
    
    printf("Game over.\n");
    close(sock);
    return 0;
}