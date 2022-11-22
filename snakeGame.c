#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

#define INIT_LEN 5 //inital length of snake always 2 or greater
#define SNAKE_CHAR 'O'
#define WIN_MSG "YOU WON!"
#define LOSE_MSG "GAME OVER"

//structure to allow for a linked list implementation of a queue for the snake
struct snake_char{
    int x;
    int y;

    struct snake_char * next;
    struct snake_char * prev;
};

struct food {
    int X;
    int Y;
    char symbol;

}food;

//initializes the main screen with border
void start_screen();

//main game loop/
void game_loop(int start_x, int start_y);

//creates and returns the window for the snakepit
void init_snake(int * curr_x, int * curr_y);

//adds a new element to the queue at x, y
void new_head(int x, int y);

//deletes the tail of the snake from the queue
void del_tail();

//draw game over screen
void game_over();

//draw the win screen
void win();

//global head and tail of snake
struct snake_char * head = NULL;
struct snake_char * tail = NULL;

int main(){
    int curr_x, curr_y;//terminal height, width, current x and y of snake head
    int ch;
    char dir = 'l'; //current direction l, r, u, or d start by going left.
   
    //initalize the linked list for the snake body
    head = (struct snake_char *)malloc(sizeof(struct snake_char));
    tail = (struct snake_char *)malloc(sizeof(struct snake_char));
    head->prev = tail;
    head->next = NULL;
    tail->next = head;
    tail->prev = NULL;

    initscr(); //start curses screen

    //check if terminal supports COLORs
    if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support COLOR\n");
		exit(1);
	}

    //create a COLOR pair for snake head
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_GREEN);

    //draw the main screen
    start_screen(stdscr, &LINES, &COLS);
    curs_set(0); //hide the cursor if allowed;

    //create window for snake to live in sitting inside border
    init_snake(&curr_x, &curr_y);
    //get a keystroke to start
    wgetch(stdscr);
    //dont wait for inputs ie getch returns error if no keystroke
    nodelay(stdscr, TRUE);


    //main game loop
    game_loop(curr_x, curr_y);
    //turn off COLORs and end curses
    attroff(COLOR_PAIR(1));
    endwin();
    return 0;
}

void start_screen(){
     //COLOR pair for border
    init_pair(2, COLOR_RED, COLOR_RED);

    //get the size of vthe terminal window
    noecho();


    //create border (window, left side, right side, top, bottom, corner, corner, corner, corner) and apply COLOR
    attron(COLOR_PAIR(2));
    border('|', '|', '-', '-', '+', '+', '+', '+');
    attroff(COLOR_PAIR(2));
}

void game_loop(int curr_x, int curr_y){
    char dir = 'r';
    int ch;
    int end = 0;
    while(!end){
        //get next keystroke from input buffer
        ch = getch();

        //check keystroke
        switch(ch){
            case KEY_UP:
                //flush input buffer to prevent stacking keystrokes
                flushinp();
                //change direction dont allow reversing
                if(dir == 'd'){
                    game_over();
                    end = 1;
                }               
                else
                    dir='u';
                break;
            case KEY_DOWN:
                flushinp();
                if(dir == 'u'){
                    game_over();
                    end = 1;
                }
                else
                    dir='d';
                break;
            case KEY_LEFT:
                flushinp();
                if(dir == 'r'){
                    game_over();
                    end = 1;
                }
                else
                    dir='l';
                break;
            case KEY_RIGHT:
                flushinp();
                if(dir == 'l'){
                    game_over();
                    end = 1;
                }
                else
                    dir='r';
                break;
            default:
                flushinp();
                break;
        }

        //break if spacebar
        if(ch == ' '){
            break;
        }
        
        //delete snake head by replaceing character with a space
        mvaddch(tail->x, tail->y, ' ');
        del_tail();

        //check current direction and move xy coordinates of snake head
        if(dir == 'l'){
            if(curr_y == 1){//if statement detects COLSlision with border
                game_over();
                end = 1;
            }
            else
                curr_y--;
        }else if(dir == 'r'){
            if(curr_y == COLS-2){//COLS - 2 is location of right of screen
                game_over();
                end = 1;
            }
            else
                curr_y++;
        }else if(dir == 'u'){
            if(curr_x == 1){
                game_over();
                end = 1;
            }
            else
                curr_x--;
        }else{
            if(curr_x == LINES-2){
                game_over();
                end = 1;
            }
            else
                 curr_x++;
        }

        //create new head
        new_head(curr_x, curr_y);
        //redraw head in new location
        mvaddch(head->x, head->y, SNAKE_CHAR);

        //refresh the window to apply changes
        refresh();

        //slow vertical speed to make vertical speed feel consistent with horizontal speed
        if(dir == 'u' || dir == 'd')
            usleep(120000);
        else
            usleep(100000);//wait 250ms or .25 sec
        
    }
}

void init_snake(int * curr_x, int * curr_y){

    //create the snakepit window
    //WINDOW * scrn = newwin(LINES - 2, COLS - 2, 1,1);
    refresh();

    //create snake COLOR pair
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    //initialize the x and y coordinates of snake head
    *curr_x = LINES / 2;
    *curr_y = (COLS / 2)-5;

    //turn on COLOR pair
    attron(COLOR_PAIR(1));


    //initialize head and tail of snake
    head->x = *curr_x;
    head->y = *curr_y;
    tail->x = *curr_x;
    tail->y = *curr_y - 1;


    //add the rest of the initial snake body elements
    for(int i = 0; i < INIT_LEN - 2; i++){
        *curr_y += 1;
        new_head(*curr_x, *curr_y);
    }

    

    //print the snake head by looping through the linked list
    struct snake_char * next = (struct snake_char *)malloc(sizeof(struct snake_char));
    next = tail;
    while(next != NULL){
        mvaddch(next->x, next->y, SNAKE_CHAR);
        next = next->next;
    }
    
    refresh();//refresh snake window to apply changes

    //add the keypad listener
    keypad(stdscr, TRUE);

    //return window with initial config
    //return scrn;
}

//add a new head to the linked list
void new_head(int x, int y){
    struct snake_char * new = (struct snake_char *)malloc(sizeof(struct snake_char));
    new->x = x;
    new->y = y;
    new->prev = head;
    new->next = NULL;
    head->next = new;
    head = new;
}

//delete the tail of the linked list
void del_tail(){
    tail = tail->next;
    tail->prev = NULL;
}

void game_over(){
    clear();
    refresh();
}

void win(){
    clear();
    refresh();
}
