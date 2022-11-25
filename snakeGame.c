#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

#define INIT_LEN 3 //inital length of snake always 2 or greater
#define SNAKE_CHAR 'o'
#define HEAD_CHAR  'O'
#define FOOD_CHAR  'b'
#define WIN_MSG "YOU WON!"
#define LOSE_MSG "GAME OVER"

//structure to allow for a linked list implementation of a queue for the snake
struct snake_char{
    int x;
    int y;

    struct snake_char * next;
    struct snake_char * prev;
};

struct trophy {
    int X;
    int Y;
    int new_len;
    int loops_alive;
    //char symbol;

};

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

//DEATH
void DeathAnimation();

//collision detection 1 if collision with wall or body 2 if with food
int DetectCollision(int new_x, int new_y);

//function to place food
void placeFood();

//global head and tail of snake
struct snake_char * head = NULL;
struct snake_char * tail = NULL;
struct trophy * food = NULL;

int main(){
    int curr_x, curr_y;//terminal height, width, current x and y of snake head
    int ch;
    char dir = 'l'; //current direction l, r, u, or d start by going left.

    food = (struct trophy *)malloc(sizeof(struct trophy));
    food->loops_alive = -1;
    food->X = 1;
    food->Y = 1;
   
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
    int addch = 0;
    while(!end){
        //get next keystroke from input buffer
        ch = getch();

        //check keystroke
        switch(ch){
            case KEY_UP:
                //flush input buffer to prevent stacking keystrokes
                flushinp();
                //change direction dont allow reversing
                //removing these for a function would most likely not make the code any more conscise. 
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

        placeFood();
        
        //delete snake head by replaceing character with a space
        if(addch <= 0){
            mvaddch(tail->x, tail->y, ' ');
            del_tail();
        }else{
            addch--;
        }
        mvaddch(head->x, head->y, SNAKE_CHAR);

        //check current direction and move xy coordinates of snake head
        switch(dir) {
            case 'l':
                curr_y--;
                break;
            case 'r':
                curr_y++;
                break;
            case 'u':
                curr_x--;
                break;
            case 'd':
                curr_x++;
                break;
            default:
                break;  
        }

        if(DetectCollision(curr_x, curr_y) == 1){
            game_over();
            end = 1;
        }
         if(DetectCollision(curr_x, curr_y) == 2){
            addch = food->new_len;
            food->loops_alive = 0;
        }
        
        
        //create new head
        new_head(curr_x, curr_y);
        //redraw head in new location
        mvaddch(head->x, head->y, HEAD_CHAR);


        //refresh the window to apply changes
        refresh();

        //slow vertical speed to make vertical speed feel consistent with horizontal speed
        if(dir == 'u' || dir == 'd')
            usleep(220000);
        else
            usleep(200000);//wait 250ms or .25 sec
        
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
    while(next->next != NULL){
        mvaddch(next->x, next->y, SNAKE_CHAR);
        next = next->next;
    }
    mvaddch(next->x, next->y, HEAD_CHAR);
    
    refresh();//refresh snake window to apply changes

    //add the keypad listener
    keypad(stdscr, TRUE);

    //return window with initial config
    //return scrn;
}

int DetectCollision(int new_x, int new_y) {
    //Detect contact with boundaries
    if( (new_x==LINES-1||new_x==0) || (new_y==COLS-1||new_y==0) )
        return 1;
    //Detect contact with body   
    int char_at = mvinch(new_x, new_y) & A_CHARTEXT; 
    if((char)char_at == SNAKE_CHAR){
        return 1;
    }
    //detect food collision
    if((head->x == food->X) && (head->y == food->Y)){
        return 2;
    }
    return 0; //no collisions   
            
}

void placeFood(){
    if(food->loops_alive > 0){
        food->loops_alive--;
    }else{
        mvaddch(food->X, food->Y, ' ');
        int char_at = 0;
        food->new_len=(rand()%9)+1;
        do{
            food->X=rand()%LINES;
            food->Y=rand()%COLS;
            if(food->X < 1)
                food->X++;
            if(food->Y < 1)
                food->Y++;
            int char_at = mvinch(food->X, food->Y) & A_CHARTEXT;
        }while(((char)char_at == SNAKE_CHAR) || (char)char_at == HEAD_CHAR ); 
        food->loops_alive = ((rand()%5) + 10)/.1;
        mvaddch(food->X, food->Y, FOOD_CHAR);
    }

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
    DeathAnimation();
    clear();
    refresh();
}

void win(){
    clear();
    refresh();
}

//All bits bounce off of one wall.  Two walls seems even more complicated.  
//If you have a simpler way to do this though, i'm all ears. 
void DeathAnimation(){
 struct snake_char * erase = (struct snake_char *)malloc(sizeof(struct snake_char));
    erase = head;
    //erase snake from screen
    while(erase->prev != NULL) {
        mvaddch(erase->x,erase->y,' ');
        erase=erase->prev;
    }
    mvaddch(erase->x,erase->y,' ');
   
    int upCount=0, downCount=0, leftCount=0, rightCount=0;
    int upLcount=0, upRcount=0, downLcount=0, downRcount=0;    
    for(int i=0; i<100; i++) {
       
        mvaddch(head->x+downCount, head->y, 'o'); //down x+
        mvaddch(head->x-upCount, head->y, 'o');//up x-
       
        mvaddch(head->x, head->y+rightCount, 'o'); //right  y+
        mvaddch(head->x, head->y-leftCount, 'o'); //left  y-

        mvaddch(head->x+downRcount, head->y+downRcount, 'o');//down-right x+ y+
        mvaddch(head->x+downLcount, head->y-downLcount, 'o');//down-left x- y+

        mvaddch(head->x-upRcount, head->y+upRcount, 'o');//up-right x- y+
        mvaddch(head->x-upLcount, head->y-upLcount, 'o');//up-left x- y-
       
        refresh();
        usleep(100000);
       
        //down
        if((head->x+i)>=LINES-2)
            mvaddch(head->x+downCount--, head->y, ' ');//up
        else mvaddch(head->x+downCount++, head->y, ' '); //down
        //up
        if((head->x-i)<=2)
            mvaddch(head->x-upCount--, head->y, ' ');//down
        else mvaddch(head->x-upCount++, head->y, ' ');//up
       
        //right
        if((head->y+i)>=COLS-2)
            mvaddch(head->x, head->y+rightCount--, ' ');//left
        else mvaddch(head->x, head->y+rightCount++, ' ');//right    
       
        //left
        if((head->y-i)<=2)
            mvaddch(head->x, head->y-leftCount--, ' ');//right
        else mvaddch(head->x, head->y-leftCount++, ' ');//left

        //down-right x+ y+
        if( (head->x+i)>=LINES-2 || (head->y+i)>=COLS-2 )
            mvaddch(head->x+downRcount+1, head->y+downRcount--, ' ');
        else mvaddch(head->x+downRcount-1, head->y+downRcount++, ' ');
       
        //down-left x- y+
        if( (head->x+i)>=LINES-2 || (head->y-i)<=2 )
            mvaddch(head->x+downLcount+1, head->y-downLcount--, ' ');
        else mvaddch(head->x+downLcount-1, head->y-downLcount++, ' ');

        //up-right x- y+
        if( (head->x-i)<=2 || (head->y+i)>=COLS-2 )
            mvaddch(head->x-upRcount-1, head->y+upRcount--, ' ');
        else mvaddch(head->x-upRcount+1, head->y+upRcount++, ' ');
       
        //up-left x- y-
        if( (head->x-i)<=2 || (head->y-i)<=2 )
            mvaddch(head->x-upLcount-1, head->y-upLcount--, ' ');            
        else mvaddch(head->x-upLcount+1, head->y-upLcount++, ' ');
    }
}
