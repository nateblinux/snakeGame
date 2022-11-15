#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

//structure to allow for a linked list implementation of a queue for the snake
struct snake_char{
    int x;
    int y;

    struct snake_char * next;
    struct snake_char * prev;
};

//initializes the main screen with border
void start_screen(WINDOW *scrn, int * row, int * col);

//main game loop/
void game_loop(WINDOW *scrn, int start_x, int start_y);

//creates and returns the window for the snakepit
WINDOW * init_snake(int * curr_x, int * curr_y);

//adds a new element to the queue at x, y
void new_head(int x, int y);

//deletes the tail of the snake from the queue
void del_tail();

//global for height and width of terminal window 

int row, col;

//global head and tail of snake
struct snake_char * head = NULL;
struct snake_char * tail = NULL;

int main(){
    int curr_x, curr_y;//terminal height, width, current x and y of snake head
    int ch;
    char dir = 'l'; //current direction l, r, u, or d start by going left.
    WINDOW *mywin; //game window

    //initalize the linked list for the snake body
    head = (struct snake_char *)malloc(sizeof(struct snake_char));
    tail = (struct snake_char *)malloc(sizeof(struct snake_char));
    head->prev = tail;
    head->next = NULL;
    tail->next = head;
    tail->prev = NULL;

    initscr(); //start curses screen

    //check if terminal supports colors
    if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

    //create a color pair for snake head
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    //draw the main screen
    start_screen(stdscr, &row, &col);
    curs_set(0); //hide the cursor if allowed;

    //create window for snake to live in sitting inside border
    mywin = init_snake(&curr_x, &curr_y);
    //get a keystroke to start
    wgetch(mywin);
    //dont wait for inputs ie getch returns error if no keystroke
    nodelay(mywin, TRUE);


    //main game loop
    game_loop(mywin, curr_x, curr_y);
    //turn off colors and end curses
    attroff(COLOR_PAIR(1));
    endwin();
    return 0;
}

void start_screen(WINDOW *scrn, int * row, int * col){
     //color pair for border
    init_pair(2, COLOR_RED, COLOR_RED);

    //get the size of vthe terminal window
    getmaxyx(scrn,*row,*col);
    noecho();


    //create border (window, left side, right side, top, bottom, corner, corner, corner, corner) and apply color
    wattron(scrn, COLOR_PAIR(2));
    wborder(scrn, '|', '|', '-', '-', '+', '+', '+', '+');
    attroff(COLOR_PAIR(2));
}

void game_loop(WINDOW *mywin, int curr_x, int curr_y){
    char dir = 'l';
    int ch;
    while(1){
        //get next keystroke from input buffer
        ch = wgetch(mywin);

        //check keystroke
        switch(ch){
            case KEY_UP:
                //flush input buffer to prevent stacking keystrokes
                flushinp();
                //change direction
                dir='u';
                break;
            case KEY_DOWN:
                flushinp();
                dir='d';
                break;
            case KEY_LEFT:
                flushinp();
                dir='l';
                break;
            case KEY_RIGHT:
                flushinp();
                dir='r';
                break;
            default:
                flushinp();
                break;
        }

        //break if backspace
        if(ch == KEY_BACKSPACE){
            break;
        }
        
        //delete snake head by replaceing character with a space
        mvwaddch(mywin, tail->x, tail->y, ' ');
        del_tail();

        //check current direction and move xy coordinates of snake head
        if(dir == 'l'){
            if(curr_y == 0);//if statement detects collision with border
            else
                curr_y--;
        }else if(dir == 'r'){
            if(curr_y == col-3);//col - 3 is location of right of screen
            else
                curr_y++;
        }else if(dir == 'u'){
            if(curr_x == 0);
            else
                curr_x--;
        }else{
            if(curr_x == row-3);
            else
                 curr_x++;
        }

        //create new head
        new_head(curr_x, curr_y);
        //redraw head in new location
        mvwaddch(mywin, head->x, head->y, ACS_BLOCK );

        //refresh the window to apply changes
        wrefresh(mywin);
        delay_output(250);//wait 250ms or .25 sec
        
    }
}

WINDOW * init_snake(int * curr_x, int * curr_y){

    WINDOW * mywin = newwin(row - 2, col - 2, 1,1);
    refresh();

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    *curr_x = row / 2;
    *curr_y = col / 2;

    wattron(mywin, COLOR_PAIR(1));

    head->x = *curr_x;
    head->y = *curr_y;
    tail->x = *curr_x - 1;
    tail->y = *curr_y;

    //print the snake head 
    mvwaddch(mywin, head->x, head->y, ACS_BLOCK);
    mvwaddch(mywin, tail->x, tail->y, ACS_BLOCK);
    wrefresh(mywin);//refresh snake window to apply changes

    //add the keypad listener
    keypad(mywin, TRUE);

    return mywin;
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
