#define _POSIX_C_SOURCE  199309L
#define _DEFAULT_SOURCE
#include <ncurses.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h> // pause
#include <sys/time.h>
#include <time.h>

#define COLUMN_WIDTH 5
#define COLUMN_BETWEEN_WIDTH 20
#define TUNNEL_HEIGHT 10
#define BIRD_WIDTH 8


#define BIRD_COLOR 1
#define BICK_COLOR 2




int frame = 0;
int flappy = 0;
int up = 0;
int bird_x, bird_y;
int column_x;
int numCols;
jmp_buf env;

typedef struct Column {
	int x;
	int height;
	struct Column *next;
} Column;

Column *columns;
Column *first, *last, *middle;


void init_game();
void main_loop();



void game_over(){
	struct itimerval iv;
	iv.it_interval.tv_sec = 0;
	iv.it_interval.tv_usec = 0;
	iv.it_value.tv_sec = 0;
	iv.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &iv, NULL);

	mvprintw((LINES / 2) - 2, (COLS / 2 ) - 12, "--------------------------");
	mvprintw((LINES / 2) - 1, (COLS / 2 ) - 12, "|                        |");
	mvprintw((LINES / 2) - 0, (COLS / 2 ) - 12, "|                        |");
	mvprintw((LINES / 2) + 1, (COLS / 2 ) - 12, "|                        |");
	mvprintw((LINES / 2) + 2, (COLS / 2 ) - 12, "|                        |");
	mvprintw((LINES / 2) + 2, (COLS / 2 ) - 12, "--------------------------");
	mvprintw((LINES / 2) - 1, (COLS / 2) - 8, "You lose: neet");
	mvprintw((LINES / 2), (COLS / 2) - 8, "Restart: r");
	mvprintw((LINES / 2) + 1, (COLS / 2) - 8, "Quit: q");
	refresh();
	for(;;){
		chtype ch = getch();
		switch(ch){
			case 'q':
				endwin();
				exit(EXIT_SUCCESS);
				break;
			case 'r':
				longjmp(env, 3);
				break;
			default:
				continue;
		}
	}
}

int get_random_height(){
	return rand() % ((int)(LINES * 0.7) + 1 - (int)(LINES * 0.3)) + (int)(LINES * 0.3);
}

void paint_bird(){
	if(flappy > 0){
		move(bird_y, bird_x);
		addch('.' | COLOR_PAIR(BIRD_COLOR));
		addch('\\' | COLOR_PAIR(BIRD_COLOR));
		addch('\\' | COLOR_PAIR(BIRD_COLOR));
		addch('_' | COLOR_PAIR(BIRD_COLOR));
		addch('(' | COLOR_PAIR(BIRD_COLOR));
		addch('.');
		addch(')' | COLOR_PAIR(BIRD_COLOR));
		addch('>' | COLOR_PAIR(BICK_COLOR));
		
		flappy -= 1;
	}else  {
		move(bird_y, bird_x);
		addch('.' | COLOR_PAIR(BIRD_COLOR));
		addch(' ' | COLOR_PAIR(BIRD_COLOR));
		addch(' ' | COLOR_PAIR(BIRD_COLOR));
		addch('_' | COLOR_PAIR(BIRD_COLOR));
		addch('(' | COLOR_PAIR(BIRD_COLOR));
		addch('.');
		addch(')' | COLOR_PAIR(BIRD_COLOR));
		addch('>' | COLOR_PAIR(BICK_COLOR));
		move(bird_y + 1, bird_x);
		addch(' ' | COLOR_PAIR(BIRD_COLOR));
		addch('/' | COLOR_PAIR(BIRD_COLOR));
		addch('/' | COLOR_PAIR(BIRD_COLOR));

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
		for(i = 0; i < (length_u - 1); i++){
			mvaddch(i, column_x, '|');
			mvaddch(i, column_x + COLUMN_WIDTH, '|');
		}
		mvaddch(length_u - 2, column_x - 1, '_');
		mvaddch(length_u - 2, column_x + COLUMN_WIDTH + 1, '_');
		mvaddch(length_u - 1, column_x - 2, '|');
		mvaddch(length_u - 1 , column_x + COLUMN_WIDTH + 2, '|');
		for(i = 0; i <= COLUMN_WIDTH + 2; i++){
			mvaddch(length_u - 1, column_x - 1 + i, '_');
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

void collision_detection(){
	if(middle != NULL){
		if(bird_y <= (middle->height + 0) || (bird_y + 1) >= (middle->height + TUNNEL_HEIGHT )){
			fprintf(stderr, "moddle-height; %d; bird_y: %d; TUNNEL_GEIGHT %ld;\n", middle->height, bird_y, (long) TUNNEL_HEIGHT);
			game_over();	
		}
	}

}

void init_columns(){
	numCols = (COLS / (COLUMN_WIDTH + COLUMN_BETWEEN_WIDTH)) + 3;
	middle = NULL;
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
	if(bird_y > LINES || bird_y < -5){
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
	

	static int acc = 2;
	static int cnt = 0;
	
	middle = NULL;
	for(i = 0; i < numCols; i++){
		if((((columns + i)->x - 1) < (bird_x + BIRD_WIDTH)) &&
					(((columns+i)->x + 1 + COLUMN_WIDTH) > bird_x)){
			middle = (columns+i);
		}

		(columns + i)->x -= 1;
	}

	collision_detection();

	if(up > 0){
		bird_y -= 2;
		up -= 2;
		cnt = 0;
		acc = 2;
	} else if(frame % acc == 0){
		bird_y += 1;
		cnt++;
	}
	if(cnt == 3){
		acc = 1;
		cnt = 0;
	}
	
	erase();
	update_columns();
	paint_bird();
	refresh();
}

void init_ncurses(){
	// Init Ncuses
	initscr();
	//cbreak();
	raw();
	noecho();
	start_color();
	curs_set(0);

	// Init colors
	use_default_colors();
	init_pair(BIRD_COLOR,  -1, -1);
	init_pair(BICK_COLOR, -1, -1);



}

void init_game(){
	erase();
	refresh();
	srand(time(NULL));

	
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = handle_frame;
	if(sigaction(SIGALRM, &sa, NULL) == -1){
		perror("sigaction");
		endwin();
		exit(EXIT_FAILURE);
	}

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
}

void main_loop(){
	sigset_t st;
	sigemptyset(&st);
	sigprocmask(SIG_SETMASK, &st, NULL);
	for(;;){
		chtype ch = getchar();
		switch(ch){
			case 'j':
				up = 5;
				flappy += 4;
				break;

			default:
				continue;
		}
	}
}


int main(){


	init_ncurses();
	setjmp(env);
	init_game();


	main_loop();

	

	endwin();
	exit(EXIT_SUCCESS);
}
