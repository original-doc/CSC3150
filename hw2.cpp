#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>
#include <iostream>



#define ROW 10
#define COLUMN 50 
#define log_river ' '
#define log_log '='
#define log_frog '0'
#define in_game 1
#define game_win 2
#define game_lose 3
#define game_quit 4


pthread_mutex_t gamesstatus;
pthread_mutex_t frogstatus;
pthread_mutex_t mapstatus;

int game_status = in_game;
const int loglen = 15;
int *log_init = new int[ROW];//define the start position of each row.
char map[ROW+10][COLUMN] ; 

struct Node{
	int x , y; 
	Node( int _x , int _y ) : x( _x ) , y( _y ) {}; 
	Node(){} ; 
} frog ; 

bool check_gamestatus();
int kbhit(void);
void *frog_ctl(void *t);
void *update_map(void *t);
void *logs_move( void *t );

int main( int argc, char *argv[] ){

	// Initialize the river map and frog's starting position
	memset( map , 0, sizeof( map ) ) ;
	int i , j ; 
	for( i = 1; i < ROW; ++i ){	
		for( j = 0; j < COLUMN - 1; ++j )	
			map[i%(COLUMN-1)][j%(COLUMN-1)] = ' ' ;  
	}	

	for( j = 0; j < COLUMN - 1; ++j )	
		map[ROW][j%(COLUMN-1)] = map[0][j%(COLUMN-1)] = '|' ;

	for( j = 0; j < COLUMN - 1; ++j )	
		map[0][j%(COLUMN-1)] = map[0][j%(COLUMN-1)] = '|' ;

	frog = Node( ROW, (COLUMN-1) / 2 ) ; 
	map[frog.x%(COLUMN-1)][frog.y%(COLUMN-1)] = '0' ; 

	//initialize the logs
	for (int i = 0; i<ROW; i++){
		if(i == 0) continue;
		srand(i);
		log_init[i] = rand()%(COLUMN-1);
		//printf("%d\n", log_init[i]);
		//printf("____________row: %d ____________", i);
		for (int j = 0; j<loglen; j++){
			int tmpj = (log_init[i]+j)%(COLUMN-1); 
			//std::cout<<tmpj<<" ";
			map[i%(COLUMN-1)][tmpj%(COLUMN-1)] = log_log;
		}
		//std::cout<<std::endl;

	}
	//Print the map into screen
	system("clear");
	for(int  i = 0; i <= ROW; ++i){

		puts(map[i%(COLUMN-1)]);
	}	


	/*  Create pthreads for wood move and frog control.  */
	pthread_mutex_init(&gamesstatus, NULL);
	pthread_mutex_init(&frogstatus, NULL);
	pthread_mutex_init(&mapstatus, NULL);

	pthread_t threads[ROW+2];//9 logs + 1 frog + 1 map + 1 dump

	long log_num;
	int logthread[ROW];
	
	for (log_num = 1; log_num < ROW; log_num++){
		if (log_num == 0){
			continue;
		}else{			
			logthread[log_num] = pthread_create(&threads[log_num], NULL, logs_move, (void *)log_num);
			if(logthread[log_num] != 0){
				printf("ERROR: return code from log pthread_create() is %d\n", logthread[log_num]);
				exit(-1);
			}

		}
	}

	int frogthread;
	frogthread = pthread_create(&threads[ROW], NULL, frog_ctl, NULL);
	if (frogthread != 0){
		printf("ERROR: return code from frog_control pthread_create() is %d\n", frogthread);
		exit(-1);
	}

	int mapthread;
	mapthread = pthread_create(&threads[ROW+1], NULL, update_map, NULL);
	if (mapthread != 0){
		printf("ERROR: return code from map pthread_create() is %d\n", mapthread);
		exit(-1);
	}
	
	int return_code;
	// pthread_join for sychronizing the program; logs, frog and status.
	for (int i = 1; i < ROW+2; i++)
	{	
		if (i == 0 ) continue;
		return_code = pthread_join(threads[i], NULL);
		if (return_code != 0)
		{
			printf("ERROR: return code from status pthread_join() is %d\n", return_code);
			exit(-1);
		}
	}
	
	/*  Display the output for user: win, lose or quit.  */
	system("clear");
	//printf("\033[H\033[2J");
	switch (game_status)
	{
	case game_win:
		printf("You win the game!\n");
		break;
	case game_lose:
		printf("You lose the game!\n");
		break;
	case game_quit:
		printf("You quit the game!\n");
		break;
	
	default:
		break;
	}
	pthread_mutex_destroy(&mapstatus);
	pthread_mutex_destroy(&frogstatus);
	pthread_mutex_destroy(&gamesstatus);

	pthread_exit(NULL);

	return 0;

}


// Determine a keyboard is hit or not. If yes, return 1. If not, return 0. 
int kbhit(void){
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}


bool check_gamestatus(){
	pthread_mutex_lock(&gamesstatus);
	bool tmp = (game_status == in_game);
	pthread_mutex_unlock(&gamesstatus);
	if (tmp) return true;
	else{
		return false;
	}

}

void *update_map(void *t){
	while(check_gamestatus()){
		//printf("this is map thread!\n");
		pthread_mutex_lock(&frogstatus);
		int x = frog.x;
		int y = frog.y;
		pthread_mutex_unlock(&frogstatus);


		pthread_mutex_lock(&mapstatus);
		for (int j = 0; j < COLUMN - 1; ++j)
			map[ROW%(COLUMN-1)][j%(COLUMN-1)] = map[0][j%(COLUMN-1)] = '|';
		for (int j = 0; j < COLUMN - 1; ++j)
			map[0][j%(COLUMN-1)] = map[0][j%(COLUMN-1)] = '|';

		map[x%(COLUMN-1)][y%(COLUMN-1)] = log_frog;
		system("clear");
		//printf("\033[H\033[2J");
		for( int i = 0; i <= ROW; ++i){
			//output the updated map to terminal
			puts(map[i%(COLUMN-1)]);
		}
			

		pthread_mutex_unlock(&mapstatus);

		usleep(100000);

		if (game_status != in_game){
			break;
		}
	}
	pthread_exit(NULL);
}


void *frog_ctl(void *t){
	while(check_gamestatus()){
		//printf("this is frog thread!\n");
		if(kbhit()){
			char sig;
			std::cin>> sig;

			switch (sig)
			{
			case 'w'://move up
				pthread_mutex_lock(&frogstatus);
				frog.x -= 1;
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'W':
				pthread_mutex_lock(&frogstatus);
				frog.x -= 1;
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'a'://move left
				pthread_mutex_lock(&frogstatus);
				if (frog.x == ROW && frog.y%(COLUMN -1) == 0 ){
					
				}else{
					frog.y -= 1;
				}
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'A':
				pthread_mutex_lock(&frogstatus);
				if (frog.x == ROW && frog.y%(COLUMN -1) == 0 ){
					
				}else{
					frog.y -= 1;
				}
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'd'://move right
				pthread_mutex_lock(&frogstatus);
				if (frog.x == ROW && frog.y%(COLUMN -1) == (COLUMN -1) ){
					
				}else{
					frog.y += 1;
				}
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'D':
				pthread_mutex_lock(&frogstatus);
				if (frog.x == ROW && frog.y%(COLUMN -1) == (COLUMN -1) ){
					
				}else{
					frog.y += 1;
				}
				pthread_mutex_unlock(&frogstatus);
				break;
			case 's'://move down
				pthread_mutex_lock(&frogstatus);
				frog.x += 1;
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'S':
				pthread_mutex_lock(&frogstatus);
				frog.x += 1;
				pthread_mutex_unlock(&frogstatus);
				break;
			case 'q':
				pthread_mutex_lock(&gamesstatus);
				game_status = game_quit;
				pthread_mutex_unlock(&gamesstatus);
				break;
			case 'Q':
				pthread_mutex_lock(&gamesstatus);
				game_status = game_quit;
				pthread_mutex_unlock(&gamesstatus);
				break;
			default:
				break;
			}
		}
		pthread_mutex_lock(&frogstatus);
		int x = frog.x;
		int y = frog.y;
		pthread_mutex_unlock(&frogstatus);

		pthread_mutex_lock(&mapstatus);
		bool in_water = (map[x%(COLUMN-1)][y%(COLUMN-1)] == log_river); 
		pthread_mutex_unlock(&mapstatus);

		if (x < 0 || x > ROW || (x != ROW && y >= (COLUMN -1)) || (x != ROW && y <= 0) || in_water){
			pthread_mutex_lock(&gamesstatus);
			game_status = game_lose;
			pthread_mutex_unlock(&gamesstatus);
		}else if (x == 0){
			pthread_mutex_lock(&gamesstatus);
			game_status = game_win;
			pthread_mutex_unlock(&gamesstatus);
		}
		pthread_mutex_lock(&gamesstatus);
		int st = game_status;
		pthread_mutex_unlock(&gamesstatus);
		if (st != in_game){
			break;
		}

	}
	
	pthread_exit(NULL);
}



void *logs_move( void *t ){
	long lognum = (long)t;
	int log_num = lognum;
	//printf("this is logthrad %d\n", log_num);
	int startpos = log_init[log_num];

	/*  Check game's status  */
	while(check_gamestatus()){
		//printf("this is logthrad %d\n", log_num);
		/*  Move the logs  */
		if (log_num%2 == 0){//log not singular moves left
			startpos = (startpos -1 + COLUMN-1)%(COLUMN-1);//add COLUMN-1 to avoid negative number
			/* system("clear");
			printf("startpos:_________ %d\n", startpos);
			sleep(5); */
		}else{//log singular moves right
			startpos = (startpos +1 + COLUMN-1)%(COLUMN-1);
			/* system("clear");
			printf("startpos:_________ %d\n", startpos);
			sleep(5); */
		}

		//update the frog
		pthread_mutex_lock(&frogstatus);
		int x = frog.x;
		int y = frog.y;
		pthread_mutex_unlock(&frogstatus);

		//update the frog with log
		int endpos = (startpos+loglen)%(COLUMN-1);
		
		pthread_mutex_lock(&mapstatus);
		if (x == log_num){
			if (startpos <= endpos){
				if (y >= startpos && y < endpos){
					if (x%2 == 0){
						pthread_mutex_lock(&frogstatus);
						frog.y -= 1;
						pthread_mutex_unlock(&frogstatus);
					}else{
						pthread_mutex_lock(&frogstatus);
						frog.y += 1;
						pthread_mutex_unlock(&frogstatus);
					}
				}

			}else{

				if (!(y >= endpos && y < startpos)){
					if (x%2 == 0){
						pthread_mutex_lock(&frogstatus);
						frog.y -= 1;
						pthread_mutex_unlock(&frogstatus);
					}else{
						pthread_mutex_lock(&frogstatus);
						frog.y += 1;
						pthread_mutex_unlock(&frogstatus);
					}
				}
			}
		}

		//update the logs
		if (startpos <= endpos){
			for(int i=0; i<startpos; i++){
				map[log_num%(COLUMN-1)][i%(COLUMN-1)] = log_river;
			}
			for(int j=endpos; j<COLUMN;j++){
				map[log_num%(COLUMN-1)][j%(COLUMN-1)] = log_river;
			}
			for(int k=startpos; k<endpos; k++){
				map[log_num%(COLUMN-1)][k%(COLUMN-1)] = log_log;
			}
		}else{
			for(int i=0; i<endpos; i++){
				map[log_num%(COLUMN-1)][i%(COLUMN-1)] = log_log;
			}
			for(int j=endpos; j<startpos; j++){
				map[log_num%(COLUMN-1)][j%(COLUMN-1)] = log_river;
			}
			for(int k=startpos; k<COLUMN; k++){
				map[log_num%(COLUMN-1)][k%(COLUMN-1)] = log_log;
			}
		}

		pthread_mutex_unlock(&mapstatus);
		//system("clear");
		usleep(100000);
		if (game_status != in_game){
			break;
		}
	}

	pthread_exit(NULL);


	/*  Check keyboard hits, to change frog's position or quit the game. */

	/*  Print the map on the screen  */

	
}


