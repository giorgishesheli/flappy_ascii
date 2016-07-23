#define _POSIX_C_SOURCE  199309L
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // pause
#include <sys/time.h>
#include <time.h>

#define FLAP 1
#define NOFLAP 0

#define COLUMN_WIDTH 5
#define COLUMN_BETWEEN_WIDTH 20
#define TUNNEL_HEIGHT 10



int frame = 0;
int flappy = 0;
int up = 0;
int bird_x, bird_y;
int column_x;
int numCols;

typedef struct Column {
	int x;
	int height;
	struct Column *next;
} Column;

Column *columns;
Column *first, *last;




void game_over(){
	struct itimerval iv;
	iv.it_interval.tv_sec = 0;
	iv.it_interval.tv_usec = 0;
	iv.it_value.tv_sec = 0;
	iv.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &iv, NULL);
	erase();
	mvprintw(LINES / 2, (COLS / 2) - 8, "Game Over: Try again");
	refresh();
	pause();
}

int get_random_height(){
	return rand() % ((int)(LINES * 0.7) + 1 - (int)(LINES * 0.3)) + (int)(LINES * 0.3);
}

void paint_bird(){
	erase();
	if(flappy > 0){
		mvprintw(bird_y, bird_x, "\\.\\\\_(.)>");
		mvprintw(bird_y + 1, bird_x, "/");
		flappy -= 1;
	}else  {
		mvprintw(bird_y, bird_x, "\\.  _(.)>");
		mvprintw(bird_y + 1, bird_x, "/ //");
	}
}

void update_columns(){
	int i;
	int length_u = 15;
	//First Column
	Column *col = first;
	while(col->next != NULL){
		column_x = col->x;
		length_u = col->height;
		for(i = 0; i <= (length_u - 1); i++){
			mvaddch(i, column_x, '|');
			mvaddch(i, column_x + COLUMN_WIDTH, '|');
		}
		mvaddch(length_u - 1, column_x - 1, '_');
		mvaddch(length_u - 1, column_x + COLUMN_WIDTH + 1, '_');
		mvaddch(length_u, column_x - 2, '|');
		mvaddch(length_u, column_x + COLUMN_WIDTH + 2, '|');
		for(i = 0; i <= COLUMN_WIDTH + 2; i++){
			mvaddch(length_u, column_x - 1 + i, '_');
		}



		//Second Column
		for(i = 0; i <= COLUMN_WIDTH + 2; i++){
			mvaddch(length_u + TUNNEL_HEIGHT, column_x - 1 + i, '_');
		}
		mvaddch(length_u +TUNNEL_HEIGHT + 1, column_x - 1, '_');
		mvaddch(length_u  +TUNNEL_HEIGHT+ 1, column_x + COLUMN_WIDTH + 1, '_');
		mvaddch(length_u +TUNNEL_HEIGHT + 1, column_x - 2, '|');
		mvaddch(length_u +TUNNEL_HEIGHT + 1, column_x + COLUMN_WIDTH + 2, '|');
		for(i = length_u + TUNNEL_HEIGHT + 2; i <= LINES; i++){
			mvaddch(i, column_x, '|');
			mvaddch(i, column_x + COLUMN_WIDTH, '|');
		}

		col = col->next;
	}
	

}

void init_columns(){
	numCols = (COLS / (COLUMN_WIDTH + COLUMN_BETWEEN_WIDTH)) + 2;
	columns = calloc(numCols,  sizeof(Column)); // TODO - Error Cheking
	int i;
	columns->x = COLS;
	columns->height = get_random_height();
	columns->next = (columns+1);
	first = columns;
	for(i = 1; i < numCols; i++){
		(columns+i)->x = (columns+i-1)->x + COLUMN_WIDTH + COLUMN_BETWEEN_WIDTH;
		(columns+i)->height = get_random_height();
		if(i != (numCols - 1)){
			(columns+i)->next = (columns+i+1);
		} else {
			(columns+i)->next = NULL;
			last = (columns + i);
		}
	}
}


void handle_frame(int sig){
	//fprintf(stderr, "%d\n",  rand() % ((int)(LINES * 0.7) + 1 - (int)(LINES * 0.3)) + (int)(LINES * 0.3));
	if(bird_y == LINES || bird_y == -5){
		game_over();
	}
	frame++;

	int i;
	if((first->x + COLUMN_WIDTH + 2) < 0){
		Column *cur = first;
		cur->x = last->x + COLUMN_WIDTH + COLUMN_BETWEEN_WIDTH;
		
		last->next = cur;
		first = cur->next;
		last = cur;
		cur->height = get_random_height();
		cur->next = NULL;
	}
	for(i = 0; i < numCols; i++){
		(columns + i)->x -= 1;
	}

	if(up > 0){
		bird_y -= 2;
		up -= 2;
	} else if(frame % 2 == 0){
		bird_y += 1;
	}
	paint_bird();
	update_columns();
	refresh();
}


int main(){
	// Init Ncuses
	initscr();
	cbreak();
	noecho();
	curs_set(0);


	srand(time(NULL));

	
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handle_frame;
	if(sigaction(SIGALRM, &sa, NULL) == -1){
		perror("sigaction");
		endwin();
		exit(EXIT_FAILURE);
	}
	refresh();

	struct itimerval iv;
	iv.it_interval.tv_sec = 0;
	iv.it_interval.tv_usec = 50000;
	iv.it_value.tv_sec = 0;
	iv.it_value.tv_usec = 50000;

	setitimer(ITIMER_REAL, &iv, NULL);

	bird_x = (COLS / 2) - 4;
	bird_y = (LINES / 2) + 1;
	column_x = COLS + 1;



	init_columns();

	for(;;){
		chtype ch = getchar();
		if(ch == 'j'){
			up = 5;
			flappy += 4;
		}
	}

	

	endwin();
	exit(EXIT_SUCCESS);
}
