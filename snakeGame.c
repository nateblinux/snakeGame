/**
* @author Nathan Benham, Olivia Grocki, Rich Piske
* Snake Game for cs 355
*/
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INIT_LEN 3 //inital length of snake always 2 or greater
#define SNAKE_CHAR 'o'
#define HEAD_CHAR  'O'
#define FOOD_CHAR  'b'
#define VERT_SPEED  220000
#define HOR_SPEED   200000
#define JUMP_SPACES 3 // # of spaces to jump
#define BOOST_DIV 200 // number to divide area of screen by for max boosts
#define SPEED_SCALING 100 //scaling factor for speed increase with snake len
#define version    "version 5.0"
#define TROPHY_REQUIREMENT 10

//death animation
#define BITS_CHAR  'o'

struct bit {
    int x;
    int y;
    int dir;
};

//Global
struct bit bits1[] = { //initialize array of bit structs
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
};

struct bit bits2[] = { //initialize array of bit structs
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
            {0, 0, 0},
};

//end of death animation header

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


//==========================
//FILE HANDLING
//==========================

struct hiScore {
    char *name;
    int score;
    int length;
};

struct hiScore hiScoreArray[] = {
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0},
    {NULL, 0, 0}
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

//Death animation prototypes:
void DeathAnimation(int);

void advanceBits(struct bit *);

void paintBits(struct bit *, char ch);

void generateBits(struct bit *, int x, int y);

//collision detection 1 if collision with wall or body 2 if with food
int DetectCollision(int new_x, int new_y);

//function to place food
void placeFood();

//Option menu, with welcome option
void optionMenu(int);

//GAME-OVER MENU
void scoreMenu();

void checkGamerScore();
void writeHighScoresToFile();

//high score menu
void importHighScores();

void sortHighScores();

void resetHighScoreArray();
//end high score menu

//randomly place walls based on LEVEL
void placeWalls();

void introSplashScreen();

void clearGameBoard();


//global head and tail of snake
struct snake_char * head = NULL;
struct snake_char * tail = NULL;
struct trophy * food = NULL;
int gameSpeed = 3; //initial speed
int gamerScore = 0;
int trophyCount = 0;
int userLevel = 1; //this WAS difficulty
int endGame = 0;
int snake_len;
int boost;
int numHighScoreRecords = 0;
char highScoreData[255];
char userName[8];
int newHighScore=0;
int winCondition=0; //if 0 = dead, no high score
                    //if 1 = dead, new high score
                    //if 2 = continue to next level
int wonWholeGame=0; //if 1 = game is beat
int playAnimation=1;
char dir;

int main(){
    srand ( time(NULL) );//seed rand with current time to prevent same sequence of numbers
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
    boost = 12;//(LINES * COLS) / BOOST_DIV; //start with half boost

    //check if terminal supports COLORs
    if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support COLOR\n");
		exit(1);
	}

    //set up intials
    noecho(); //don't output input to screen
    keypad(stdscr, TRUE); //add the keypad listener
    nodelay(stdscr, TRUE);
    curs_set(0); //hide the cursor if allowed;
    start_color(); //start colors

    //setup colors
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK); 
    init_pair(3, COLOR_CYAN, COLOR_BLACK); 
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    
    //draw the main screen
    start_screen();

    //Get high scores from highscores.txt
    importHighScores();
    sortHighScores();
    if(playAnimation==1)
        introSplashScreen();
    //open menu system
    if(winCondition<2)
        optionMenu(0); //0 because game has not begun
    winCondition=0; //reset winCondition after our check
    //create window for snake to live in sitting inside border
    init_snake(&curr_x, &curr_y);
    //main game loop
    game_loop(curr_x, curr_y);
    
    //attroff(COLOR_PAIR(1)); //turn off COLORs 
    endwin(); //end curses
    return 0;
}

void start_screen(){
     //COLOR pair for border
    //init_pair(2, COLOR_CYAN, COLOR_BLACK);


    //create border (window, left side, right side, top, bottom, corner, corner, corner, corner) and apply COLOR
    attron(COLOR_PAIR(4));
    border('|', '|', '-', '-', '+', '+', '+', '+');
    for(int i=1; i<COLS-1; i++) //make another border right above the bottom border (for score and status)
        mvaddch(LINES-3, i, '-');
    attroff(COLOR_PAIR(4));
}

void game_loop(int curr_x, int curr_y){
    int ch;
    int addch = 0;
    int jump = 0;
    snake_len = INIT_LEN;//reset snake length
    while(!endGame){
        
        //slow vertical speed to make vertical speed feel consistent with horizontal speed
        if(jump > 0){
            if(dir == 'u' || dir == 'd')
                usleep(((VERT_SPEED/gameSpeed) - ((snake_len / 3) - 1) * SPEED_SCALING) / 2200);
            else
               usleep(((HOR_SPEED/gameSpeed) - ((snake_len / 3) - 1) * SPEED_SCALING) / 2000);//wait 250ms or .25 sec
            jump--;
            food -> loops_alive++; //dont reduce loops alive for food
        }else{//reduce the delay for the jump to .001 sec per loop
            if(dir == 'u' || dir == 'd')
                usleep((VERT_SPEED/gameSpeed) - ((snake_len / 3) - 1) * SPEED_SCALING);
            else
               usleep((HOR_SPEED/gameSpeed) - ((snake_len / 3) - 1) * SPEED_SCALING);//wait 250ms or .25 sec
        }
        

        //check keystroke
        switch(getch()){
            case KEY_UP:
                //flush input buffer to prevent stacking keystrokes
                flushinp();
                if (dir == 'u' && boost > 0){
                    jump = JUMP_SPACES - 1;// correct for height difference
                    boost--;
                } 
                else dir='u';
                break;
            case KEY_DOWN:
                flushinp();
                if(dir == 'd' && boost > 0){
                    jump = JUMP_SPACES - 1;
                    boost--;
                }
                else dir='d';
                break;
            case KEY_LEFT:
                flushinp();
                if(dir == 'l' && boost > 0){
                    jump = JUMP_SPACES;
                    boost--;
                }
                else dir='l';
                break;
            case KEY_RIGHT:
                flushinp();
                if (dir == 'r' && boost > 0){
                    jump = JUMP_SPACES;
                    boost--;
                }
                else dir='r';
                break;
            case ' ':
                optionMenu(1); //1 because we're in game
                placeFood();//if food is behind the menu, it disappears, so redraw
                break;
            default:
                flushinp();
                break;
        }

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

        if((DetectCollision(curr_x, curr_y) == 1) || (boost==0)) {
            game_over();
            endGame = 1;
        }
        if(DetectCollision(curr_x, curr_y) == 2){
            addch = food->new_len;
            snake_len += food->new_len;
            gamerScore += food->loops_alive; //score decreases as loops_alive decreases
            food->loops_alive = 0;
            trophyCount++;
            if(boost < 24) { //cap the number of boosts able to be collected
                if(boost + 3 < 24)
                    boost+=3;
                else if(boost + 2 < 24)
                    boost+=2;
                else if(boost + 1 < 24)
                    boost+=1;
                //else maxed out, no boost!
            }
        }

        //DRAW GAME STATS
        attron(COLOR_PAIR(3));
        attron(A_BOLD);
        mvprintw(LINES-2, 2, "Trophy Value: %03d", food->loops_alive);
        mvprintw(LINES-1, 2, "Score: %05d", gamerScore);

        mvprintw(LINES-2, COLS/2-18, "Boost: [");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        mvprintw(LINES-2, COLS/2-10, "                         ");
        if(boost<=24) {
            if(boost<=5) {
                attron(COLOR_PAIR(5));
                attron(A_BLINK);
            }
            else if (boost<=10)
                attron(COLOR_PAIR(1));
            else attron(COLOR_PAIR(2));
                
            for(int i=0; i<boost; i++) {
                mvaddch(LINES-2, COLS/2+(i-10), '#');
            }
        }
        else mvprintw(LINES-2, COLS/2+1, "%d", boost);
        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));
        attroff(COLOR_PAIR(5));
        attroff(A_BLINK);
        attron(COLOR_PAIR(3));
        attron(A_BOLD);
        mvaddch(LINES-2, COLS/2+13, ']');
        mvprintw(LINES-2, COLS-24, "                        ");
        mvprintw(LINES-2, COLS-4, "/%d", TROPHY_REQUIREMENT);
        mvprintw(LINES-2, COLS-(14+TROPHY_REQUIREMENT), "Trophy's:");
        for (int i=0; i<trophyCount; i++)
            mvaddch(LINES-2, COLS-5-i, '*');
        attroff(COLOR_PAIR(3));
        

        //DRAW SNAKE
        attron(COLOR_PAIR(1)); //SNAKE COLOR ON
        //delete snake head by replaceing character with a space
        if(addch <= 0){
            mvaddch(tail->x, tail->y, ' ');
            del_tail();
        }else{
            addch--;
        }
        mvaddch(head->x, head->y, SNAKE_CHAR);
        
        //create new head
        new_head(curr_x, curr_y);
        //redraw head in new location
        mvaddch(head->x, head->y, HEAD_CHAR);

        attroff(COLOR_PAIR(1)); //SNAKE COLOR OFF

        placeFood();
        
        attroff(A_BOLD);

        placeWalls();

        refresh();
        if(trophyCount == TROPHY_REQUIREMENT){
            mvaddch(food->X, food->Y, ' '); //clear food
            win();
            endGame = 1;
        }
    }
}

void init_snake(int * curr_x, int * curr_y){

    //initialize the x and y coordinates of snake head
    *curr_x = LINES / 2;
    *curr_y = ((COLS / 2)-INIT_LEN) - 10; //snake head dead center shifted 10 for levels

    //turn on COLOR pair
    attron(COLOR_PAIR(1));


    //initialize head and tail of snake
    head->x = *curr_x;
    head->y = *curr_y;

    //choose random start direction and assign tail x,y
    int init_dir = rand() % 3;
    if(init_dir == 0){
        dir = 'r';
        tail->x = *curr_x;
        tail->y = *curr_y - 1;
    }
    if(init_dir == 1){
        dir = 'l';
        tail->x = *curr_x;
        tail->y = *curr_y + 1;
    }
    if(init_dir == 2){
        dir = 'u';
        tail->x = *curr_x - 1;
        tail->y = *curr_y;
    }
    if(init_dir == 3){
        dir = 'd';
        tail->x = *curr_x + 1;
        tail->y = *curr_y;
    }


    //add the rest of the initial snake body elements
    for(int i = 0; i < INIT_LEN - 2; i++){
        switch(dir) {
            case 'l':
                *curr_y = *curr_y - 1;
                break;
            case 'r':
                *curr_y = *curr_y + 1;
                break;
            case 'u':
                *curr_x = *curr_x - 1;
                break;
            case 'd':
                *curr_x = *curr_x+ 1;
                break;
            default:
                break;  
        }
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

    attroff(COLOR_PAIR(1));
    
}

int DetectCollision(int new_x, int new_y) {
    //detect food collision
    if((head->x == food->X) && (head->y == food->Y)){
        return 2;
    }
    int char_at = mvinch(new_x, new_y) & A_CHARTEXT; 
    if((char)char_at != FOOD_CHAR && (char)char_at != ' '){
        return 1;
    }
    return 0; //no collisions   
            
}

void placeFood(int collision){
    attron(COLOR_PAIR(6)); //FOOD COLOR
    if(food->loops_alive > 0){//reduce the number of loops that the food is alive
        food->loops_alive--;
    }else{
        int char_at = mvinch(food->X, food->Y) & A_CHARTEXT;
        if((char)char_at == SNAKE_CHAR || (char)char_at == HEAD_CHAR)
            mvaddch(food->X, food->Y, SNAKE_CHAR); //CHANGE COLOR HERE for consistent snake color
        else
            mvaddch(food->X, food->Y, ' ');
        food->new_len=(rand()%5)+1; //changed to length 5
        do{
            food->X=rand()%(LINES - 4);//fill in the food x and y with random places on screen
            food->Y=rand()%COLS;
            if(food->X < 1)//make sure that x and y are on the screen
                food->X++;
            if(food->Y < 1)
                food->Y++;
            char_at = mvinch(food->X, food->Y) & A_CHARTEXT;
        }while((char)char_at != ' '); //make sure food does not generate inside the snake
        food->loops_alive = ((rand()%3) + 9)/(.22 / gameSpeed);// random value from 3 to 9 seconds in loops
        mvaddch(food->X, food->Y, FOOD_CHAR);
    }
    attroff(COLOR_PAIR(6));
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
    struct snake_char * old_tail = (struct snake_char *)malloc(sizeof(struct snake_char));
    old_tail = tail;
    tail = tail->next;
    tail->prev = NULL;
    free(old_tail);
}

//==================================
//GAME EVENTS (WIN/LOSE)
//==================================

void game_over(){
    DeathAnimation(0);
    checkGamerScore(); 
    scoreMenu(); //remember we have WINCONDITION
    clear();
    refresh();
}

void win(){
    winCondition=2; //because we won
    wonWholeGame=0;
    userLevel++;
    if(userLevel>5) {
        userLevel=1;
        wonWholeGame=1;
        clearGameBoard();
        checkGamerScore();
    }
    DeathAnimation(1); //this will be WINCONDITION animation
    scoreMenu(); //using WINCONDITION
    clear();
    refresh();
}

//==================================
//CREATE LEVELS
//==================================

void placeWalls(){
    attron(A_BOLD);
    switch(userLevel) {
        case 2:
            attron(COLOR_PAIR(2));
            for(int i = (LINES / 3); i < (2*(LINES / 3)); i++){
                mvaddch(i, COLS/2, '|');
            }
            refresh();
            attroff(COLOR_PAIR(2));
            break;
        case 3:
            attron(COLOR_PAIR(3));
            for(int i = 1; i < (2*(LINES / 3)); i++){
                mvaddch(i, COLS/4, '|');
                mvaddch(LINES - (i + 3), COLS/4 * 3, '|');
            }
            attroff(COLOR_PAIR(3));
            break;
        case 4:
            attron(COLOR_PAIR(4));
            for(int i = 1; i < (2*(LINES / 3)); i++){
                mvaddch(i, COLS/4, '|');
            }
            for(int i = LINES - 4; i > LINES / 4; i--){
                mvaddch(i, COLS/4 * 3, '|');
            }

            for(int i = 8; i < (COLS / 4); i++){
                mvaddch(2 * LINES / 3, i, '-');
                mvaddch((LINES / 3) - 3, COLS - i, '-');
            }
            attroff(COLOR_PAIR(4));
            break;
        case 5:
            attron(COLOR_PAIR(5));
            for(int i = 1; i < (2*(LINES / 3)); i++){
                mvaddch(i, COLS/4, '|');
            }
            for(int i = LINES - 4; i > LINES / 4; i--){
                mvaddch(i, COLS/4 * 3, '|');
            }

            for(int i = 6; i < (COLS / 2); i++){
                mvaddch(2 * LINES / 3, i, '-');
                mvaddch( LINES / 4, COLS - i, '-');
            }

            for(int i = (2 * LINES / 5); i < (3*(LINES / 5)); i++){
                mvaddch(i, COLS/2, '|');
            }
            attroff(COLOR_PAIR(5));
            break;
    }
    attroff(A_BOLD);
    refresh();
}

//==================================
//PROCESS GAMER SCORE
//==================================

void checkGamerScore() {
    if(numHighScoreRecords>=5) {
        int lowScore = hiScoreArray[numHighScoreRecords-1].score;
        if(gamerScore>lowScore) {
            hiScoreArray[numHighScoreRecords-1].name = "";
            hiScoreArray[numHighScoreRecords-1].score = gamerScore;
            hiScoreArray[numHighScoreRecords-1].length = snake_len;
            newHighScore=1; //we have a new highscore
            winCondition=1; //dead, with new highscore
        }
    } else {
        hiScoreArray[numHighScoreRecords].name = "";
        hiScoreArray[numHighScoreRecords].score = gamerScore;
        hiScoreArray[numHighScoreRecords++].length = snake_len;
        newHighScore=1;
        winCondition=1;
    }
    sortHighScores();
}

//==================================
//FILE HANDLING / IMPORT HIGH SCORES
//==================================

void importHighScores() {

    FILE *fp;
    char ch;
    int i = 0;
    if((fp = fopen("highscores.txt", "r")) != NULL) {
        while( (ch = getc(fp)) != EOF ) {
            highScoreData[i++] = ch;
        }
    
        fclose(fp);

         //we have our string data separated by ,
        char *delim = ",";
        char *token = strtok(highScoreData, delim);
        int count = 0;
        while(token != NULL) {
            switch(count%3) {
                case 0:
                    hiScoreArray[numHighScoreRecords].name = token;
                    break;
                case 1:
                    hiScoreArray[numHighScoreRecords].score = atoi(token);
                    break;
                case 2:
                    hiScoreArray[numHighScoreRecords].length = atoi(token);
                    break;
            }
            count++;
            if(count%3==0)
                numHighScoreRecords++;
            token = strtok(NULL, delim);
        }
    }
}

void writeHighScoresToFile() {
    FILE *fp;
    char ch;
    int i = 0;
    fp = fopen("highscores.txt", "w+");
    while(i<numHighScoreRecords) {
        fprintf(fp, "%s,%d,%d", hiScoreArray[i].name, hiScoreArray[i].score, hiScoreArray[i].length);
        if(++i != numHighScoreRecords)
            fputc(',',fp);
    }
    fclose(fp);
}

void sortHighScores() {
    char *temp_name;
    int temp_score, temp_length;
    for(int i=0; i<numHighScoreRecords; i++)
        for(int j=i+1; j<numHighScoreRecords; j++)
            if(hiScoreArray[i].score<hiScoreArray[j].score) {
                //swap name
                temp_name = hiScoreArray[i].name;
                hiScoreArray[i].name = hiScoreArray[j].name;
                hiScoreArray[j].name = temp_name;
                //swap score
                temp_score = hiScoreArray[i].score;
                hiScoreArray[i].score = hiScoreArray[j].score;
                hiScoreArray[j].score = temp_score;
                //swap length
                temp_length = hiScoreArray[i].length;
                hiScoreArray[i].length = hiScoreArray[j].length;
                hiScoreArray[j].length = temp_length;
            }
}

void resetHighScoreArray() {
    for(int i=0; i<numHighScoreRecords; i++) {
        hiScoreArray[i].name = '\0';
        hiScoreArray[i].score = 0;
        hiScoreArray[i].length = 0;
    }
}

//=================================
//Clear Screen Options
//=================================

void clearMenu() { //int startX, int startY, int height, int width) {
    /*for (int i=startX; i<height; i++)
        for(int j=startY; j<width; j++)
            mvaddch(i, j, ' ');*/
    mvprintw(LINES/2-7, COLS/2-20, "                                        ");
    mvprintw(LINES/2-6, COLS/2-20, "                                        ");
    mvprintw(LINES/2-5, COLS/2-20, "                                        ");
    mvprintw(LINES/2-4, COLS/2-20, "                                        ");
    mvprintw(LINES/2-3, COLS/2-20, "                                        ");
    mvprintw(LINES/2-2, COLS/2-20, "                                        ");
    mvprintw(LINES/2-1, COLS/2-20, "                                        ");
    mvprintw(LINES/2,   COLS/2-20, "                                        ");
    mvprintw(LINES/2+1, COLS/2-20, "                                        ");
    mvprintw(LINES/2+2, COLS/2-20, "                                        ");
    mvprintw(LINES/2+3, COLS/2-20, "                                        ");
    mvprintw(LINES/2+4, COLS/2-20, "                                        ");
    mvprintw(LINES/2+5, COLS/2-20, "                                        ");
    mvprintw(LINES/2+6, COLS/2-20, "                                        ");
    mvprintw(LINES/2+7, COLS/2-20, "                                        ");
}

void clearGameBoard() {
    for (int i=1; i<LINES-3; i++)
        for (int j=1; j<COLS-1; j++)
            mvaddch(i, j, ' ');
}



//=================================
//HIGH SCORE MENU
//=================================

void printHighScoreMenu() { 
    //init_pair(3, COLOR_BLUE, COLOR_BLACK);
    attron(A_BOLD);
    attron(COLOR_PAIR(3));
    mvprintw(LINES/2-7, COLS/2-20, "========================================");
    mvprintw(LINES/2-6, COLS/2-20, "*             HIGH SCORES              *");
    mvprintw(LINES/2-5, COLS/2-20, "*                                      *");
    mvprintw(LINES/2-4, COLS/2-20, "*    Name        Score       Length    *");
    mvprintw(LINES/2-3, COLS/2-20, "* ----------   ----------  ----------- *");
    mvprintw(LINES/2-2, COLS/2-20, "*                                      *");
    mvprintw(LINES/2-1, COLS/2-20, "* 1.                                   *");
    mvprintw(LINES/2,   COLS/2-20, "* 2.                                   *");
    mvprintw(LINES/2+1, COLS/2-20, "* 3.                                   *");
    mvprintw(LINES/2+2, COLS/2-20, "* 4.                                   *");
    mvprintw(LINES/2+3, COLS/2-20, "* 5.                                   *");
    mvprintw(LINES/2+4, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+5, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+6, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+7, COLS/2-20, "========================================");
    attroff(COLOR_PAIR(3));
    attroff(A_BOLD);
}

void printHighScoreOptions(int position) {
    //init_pair(4, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    char message1[] = "(Press enter key)";
    char message2[] = "(Press UP to enter name)";
    char message[13];
    int offset;
    if (newHighScore==1)
        strcpy(message, message2);
    else strcpy(message, message1);
    switch(position) {
        case 0:
            attron(A_BOLD); //this will be a loop to loop through records
            offset = -1;
            for(int i=0; i<numHighScoreRecords; i++) {
                    mvprintw(LINES/2+offset, COLS/2-15, "%s", hiScoreArray[i].name); //FILE->NAME1
                    mvprintw(LINES/2+offset, COLS/2-3, "%d", hiScoreArray[i].score); //FILE->SCORE1
                    mvprintw(LINES/2+offset++, COLS/2+11, "%d", hiScoreArray[i].length); //FILE->LENGTH1   
                }
            attroff(A_BOLD);
            attron(A_STANDOUT);
            mvprintw(LINES/2+5, COLS/2-(strlen(message)/2), "%s", message);
            attroff(A_STANDOUT);
            break;
        case 1:
            attron(A_BOLD); //this will be a loop to loop through records
            int enterNamePosition = -1;
            offset = -1;
            for(int i=0; i<numHighScoreRecords; i++) {
                    if(hiScoreArray[i].name == "")
                        enterNamePosition = i;
                    mvprintw(LINES/2+offset, COLS/2-15, "%s", hiScoreArray[i].name); //FILE->NAME1
                    mvprintw(LINES/2+offset, COLS/2-3, "%d", hiScoreArray[i].score); //FILE->SCORE1
                    mvprintw(LINES/2+offset++, COLS/2+11, "%d", hiScoreArray[i].length); //FILE->LENGTH1   
            }
            offset = -1; //reset offset
            attroff(A_BOLD);
            
            attron(A_STANDOUT);
            mvprintw(LINES/2+5, COLS/2-(strlen(message)/2), "%s", message);
            attroff(A_STANDOUT);         
            echo();
            nodelay(stdscr, FALSE);
            curs_set(1);
            move(LINES/2+(offset+enterNamePosition), COLS/2-15);
            getnstr(userName, 8);
            hiScoreArray[enterNamePosition].name = userName;
            newHighScore=0; //turn this off, because already entered.
            if(!wonWholeGame)
                winCondition=0; //they no longer have a high score to enter
            curs_set(0);
            break;
    }
    attroff(COLOR_PAIR(1));

}

void highScoreMenu() { //if 1, newHighScore allows entry
    int position=0, resume=0;
    int ch;
    while(!resume) {
        printHighScoreMenu();
        printHighScoreOptions(position);
        ch = getch();
        switch(ch) {
            case '\n':
                if(position==0)
                    resume=1;
                break;
            case KEY_UP:
                if((newHighScore == 1) && (position<1))
                    position++;
                break;
            case KEY_DOWN:
                if(position>0)
                    position--;
                break;
        }
        refresh();
    }
    clearMenu();
}


//=================================
//GAME OVER MENU
//=================================
void printScoreMenu() {
    //init_pair(3, COLOR_BLUE, COLOR_BLACK);
    char message1[] = "GAME OVER!";
    char message2[] = "NEXT LEVEL!";
    char message[11];
    if(winCondition == 2)
        strcpy(message, message2);
    else strcpy(message, message1);
    attron(A_BOLD);
    attron(COLOR_PAIR(3));
    mvprintw(LINES/2-6, COLS/2-15, "*============================*");
    mvprintw(LINES/2-5, COLS/2-15, "*                            *");
    mvprintw(LINES/2-4, COLS/2-15, "*                            *");
    mvprintw(LINES/2-3, COLS/2-15, "*                            *");
    mvprintw(LINES/2-2, COLS/2-15, "*                            *");
    mvprintw(LINES/2-1, COLS/2-15, "*  Your score:               *");
    mvprintw(LINES/2,   COLS/2-15, "*                            *");
    mvprintw(LINES/2+1, COLS/2-15, "*                            *");
    mvprintw(LINES/2+2, COLS/2-15, "*                            *");
    mvprintw(LINES/2+3, COLS/2-15, "*                            *");
    mvprintw(LINES/2+4, COLS/2-15, "*                            *");
    mvprintw(LINES/2+5, COLS/2-15, "*============================*");

    mvprintw(LINES/2-5, COLS/2-(strlen(message)/2), "%s", message);
    attroff(COLOR_PAIR(3));
    attroff(A_BOLD);
}


void printScoreOptions(int position) { //we need 
    //init_pair(4, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    char optionMsg0[] = "View High Scores";
    char optionMsg1[] = "Save Score!";
    char optionMsg2[] = "Next Level!";
    char optionMsg[16];

    char highScoreMsg0[] = "NEW HIGH SCORE!!";
    char highScoreMsg1[] = "Awesome! You placed 2nd!";
    char highScoreMsg2[] = "3rd Place Finish!";
    char highScoreMsg3[] = "4th Place Finish!";
    char highScoreMsg4[] = "Top 5 Finish!";
    char highScoreMsg5[] = "You did not make Top 5 :(";
    char highScoreMsg[25];

    char levelMessage0[] = "You made it to Level 2!";
    char levelMessage1[] = "You made it to Level 3!";
    char levelMessage2[] = "You made it to Level 4!";
    char levelMessage3[] = "You made it to Level 5!";
    char levelMessage4[] = "YOU WON!!! Congratulations!";
    char levelMessage[23];
    
    int scorePosition;
    int lowscore;
    switch(winCondition) {
        case 0: //DEATH WITH NO HIGH SCORE
            //the following check is so, after entering score, they don't get Msg5 notice
            lowscore = hiScoreArray[numHighScoreRecords-1].score;
            if(lowscore>gamerScore)
                strcpy(highScoreMsg, highScoreMsg5);
            else strcpy(highScoreMsg, ""); 
            strcpy(optionMsg, optionMsg0);
            break;
        case 1: //DEATH WITH NEW HIGH SCORE
            
            for(int i=0; i<numHighScoreRecords; i++)
                if(hiScoreArray[i].name == "")
                    scorePosition = i;
            switch(scorePosition) {
                case 0:
                    strcpy(highScoreMsg, highScoreMsg0);
                    break;
                case 1:
                    strcpy(highScoreMsg, highScoreMsg1);
                    break;
                case 2:
                    strcpy(highScoreMsg, highScoreMsg2);
                    break;
                case 3:
                    strcpy(highScoreMsg, highScoreMsg3);
                    break;
                case 4:
                    strcpy(highScoreMsg, highScoreMsg4);
                    break;
            }
            strcpy(optionMsg, optionMsg1);
            break;
        case 2: //NEXT LEVEL
            switch(userLevel) {
                case 2:
                    strcpy(levelMessage, levelMessage0);
                    break;
                case 3:
                    strcpy(levelMessage, levelMessage1);
                    break;
                case 4:
                    strcpy(levelMessage, levelMessage2);
                    break;
                case 5:
                    strcpy(levelMessage, levelMessage3);
                    break;
                case 6:
                    strcpy(levelMessage, levelMessage4);  
            }
            if (wonWholeGame)
                strcpy(optionMsg, optionMsg0); //Save Score!
            else strcpy(optionMsg, optionMsg2); //NEXT LEVEL
            break;
    }
    

    switch(position) {
        case 0:
            attron(A_BLINK);
            if (winCondition==2) {
                mvprintw(LINES/2-3, COLS/2-(strlen(levelMessage)/2), "%s", levelMessage);  
            } else mvprintw(LINES/2-3, COLS/2-(strlen(highScoreMsg)/2), "%s", highScoreMsg);
            attroff(A_BLINK);
            mvprintw(LINES/2-1, COLS/2, "%d", gamerScore);
            attron(A_STANDOUT);
            mvprintw(LINES/2+1, COLS/2-(strlen(optionMsg)/2), "%s", optionMsg);
            attroff(A_STANDOUT);
            mvprintw(LINES/2+2, COLS/2-2, "Retry");
            mvprintw(LINES/2+3, COLS/2-2, "Exit");
            break;
        case 1:
            attron(A_BLINK);
            if (winCondition==2) {
                mvprintw(LINES/2-3, COLS/2-(strlen(levelMessage)/2), "%s", levelMessage);  
            } else mvprintw(LINES/2-3, COLS/2-(strlen(highScoreMsg)/2), "%s", highScoreMsg);
            attroff(A_BLINK);
            mvprintw(LINES/2-1, COLS/2, "%d", gamerScore);
            mvprintw(LINES/2+1, COLS/2-(strlen(optionMsg)/2), "%s", optionMsg);
            attron(A_STANDOUT);
            mvprintw(LINES/2+2, COLS/2-2, "Retry");
            attroff(A_STANDOUT);
            mvprintw(LINES/2+3, COLS/2-2, "Exit");
            break;
        case 2:
            attron(A_BLINK);
            if (winCondition==2) {
                mvprintw(LINES/2-3, COLS/2-(strlen(levelMessage)/2), "%s", levelMessage);  
            } else mvprintw(LINES/2-3, COLS/2-(strlen(highScoreMsg)/2), "%s", highScoreMsg);
            attroff(A_BLINK);
            mvprintw(LINES/2-1, COLS/2, "%d", gamerScore);
            mvprintw(LINES/2+1, COLS/2-(strlen(optionMsg)/2), "%s", optionMsg);
            mvprintw(LINES/2+2, COLS/2-2, "Retry");
            attron(A_STANDOUT);
            mvprintw(LINES/2+3, COLS/2-2, "Exit");
            attroff(A_STANDOUT);
            break;
        case 3: //SPECIAL CASE, animation is playing, and we just want flashing msg & score
            attron(A_BLINK);
            if (winCondition==2) {
                mvprintw(LINES/2-3, COLS/2-(strlen(levelMessage)/2), "%s", levelMessage);  
            } else mvprintw(LINES/2-3, COLS/2-(strlen(highScoreMsg)/2), "%s", highScoreMsg);
            attroff(A_BLINK);
            attron(A_BOLD);
            mvprintw(LINES/2-1, COLS/2, "%d", gamerScore);
            attroff(A_BOLD);
            mvprintw(LINES/2+2, COLS/2-12, "(Press Space to Continue)");
            break;
    }
    attroff(COLOR_PAIR(1));
}

void scoreMenu() { //REMEMBER WE HAVE WINCONDITION

    int position=0, alive=1;
    int ch;
    while(alive) {
        printScoreMenu(); //
        printScoreOptions(position);
        ch = getch();
        switch(ch) {
            case '\n':
                if(position==0) {
                    if (winCondition==2) {//FIX THIS SHIT
                        if (wonWholeGame==1) {
                            userLevel=1; 
                            wonWholeGame=0;
                            gamerScore=0;
                            highScoreMenu();
                            winCondition=0;
                            
                        }
                        trophyCount=0;
                        if (userName[0] != '\0') //don't save the score if no name
                            writeHighScoresToFile();
                        resetHighScoreArray();
                        memset(userName, 0, sizeof(userName));
                        numHighScoreRecords = 0;
                        clearGameBoard();
                        main();
                        alive=0;
                    }
                    else highScoreMenu(); //think about this ..it works when not next level conditions
                } 
                else if(position==1) { //RETRY
                    clear(); //clear the screen
                    //reset all Globals
                        gamerScore=0; //reset progress
                        trophyCount=0;
                        //userLevel=1;
                        if (userName[0] != '\0') //don't save the score if no name
                            writeHighScoresToFile();
                        resetHighScoreArray();
                        memset(userName, 0, sizeof(userName));
                        numHighScoreRecords = 0;
                    main(); //start at the top
                    alive=0; //pointless?
                }
                else {
                    if (userName[0] != '\0') //don't save the score if no name
                            writeHighScoresToFile();
                    endGame=1;
                    alive=0;
                }
            case KEY_UP:
                if(position>0)
                    position--;
                break;
            case KEY_DOWN:
                if(position<2)
                    position++;
        }
        refresh();
    }
    //clearMenu();
}

//=================================
//OPTION MENU
//=================================

void printMenu() {
    //init_pair(3, COLOR_BLUE, COLOR_BLACK);
    attron(A_BOLD);
    attron(COLOR_PAIR(4));
    mvprintw(LINES/2-5, COLS/2-20, "========================================");
    mvprintw(LINES/2-4, COLS/2-20, "*           Welcome to Snake           *");
    mvprintw(LINES/2-3, COLS/2-20, "*                                      *");
    mvprintw(LINES/2-2, COLS/2-20, "* Options:                             *");
    mvprintw(LINES/2-1, COLS/2-20, "*     -Speed:                          *");
    mvprintw(LINES/2,   COLS/2-20, "*     -Level:                          *");
    mvprintw(LINES/2+1, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+2, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+3, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+4, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+5, COLS/2-20, "*                                      *");
    mvprintw(LINES/2+6, COLS/2-20, "========================================");
    attroff(COLOR_PAIR(4));
    attroff(A_BOLD);
}

void printOptions(int position, int inGame) {
    //init_pair(4, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    //attron(A_BOLD);
    char message1[] = "Start Game!";
    char message2[] = "Resume Game!";
    char message[13];
    if (inGame==1)
        strcpy(message, message2);
    else strcpy(message, message1);
    switch(position) {
        case 0:
            mvprintw(LINES/2-1,   COLS/2, "[%d]", gameSpeed);
            mvprintw(LINES/2, COLS/2, "[%d]", userLevel);
            mvprintw(LINES/2+2, COLS/2-(strlen(message)/2), "%s", message);
            mvprintw(LINES/2+3, COLS/2-8, "View High Scores");
            attron(A_STANDOUT);
            mvprintw(LINES/2+4, COLS/2-2, "Exit");
            attroff(A_STANDOUT);
            break;
        case 1:
            mvprintw(LINES/2-1,   COLS/2, "[%d]", gameSpeed);
            mvprintw(LINES/2, COLS/2, "[%d]", userLevel);
            mvprintw(LINES/2+2, COLS/2-(strlen(message)/2), "%s", message);
            attron(A_STANDOUT);
            mvprintw(LINES/2+3, COLS/2-8, "View High Scores");
            attroff(A_STANDOUT);
            mvprintw(LINES/2+4, COLS/2-2, "Exit");
            break;
        case 2:
            mvprintw(LINES/2-1, COLS/2, "[%d]", gameSpeed);
            mvprintw(LINES/2,   COLS/2, "[%d]", userLevel);
            attron(A_STANDOUT);
            mvprintw(LINES/2+2, COLS/2-(strlen(message)/2), "%s", message);
            attroff(A_STANDOUT);
            mvprintw(LINES/2+3, COLS/2-8, "View High Scores");
            mvprintw(LINES/2+4, COLS/2-2, "Exit");
            break;
        case 3:
            mvprintw(LINES/2-1, COLS/2, "[%d]", gameSpeed);
            attron(A_STANDOUT);
            mvprintw(LINES/2,   COLS/2, "<%d>", userLevel);
            attroff(A_STANDOUT);
            mvprintw(LINES/2+2, COLS/2-(strlen(message)/2), "%s", message);
            mvprintw(LINES/2+3, COLS/2-8, "View High Scores");
            mvprintw(LINES/2+4, COLS/2-2, "Exit");
            break;
        case 4:
            attron(A_STANDOUT);
            mvprintw(LINES/2-1, COLS/2, "<%d>", gameSpeed);
            attroff(A_STANDOUT);
            mvprintw(LINES/2,   COLS/2, "[%d]", userLevel);
            mvprintw(LINES/2+2, COLS/2-(strlen(message)/2), "%s", message);
            mvprintw(LINES/2+3, COLS/2-8, "View High Scores");
            mvprintw(LINES/2+4, COLS/2-2, "Exit");
            break;
    }
    attroff(A_BOLD);
    attroff(COLOR_PAIR(1));

}

void optionMenu(int inGame) { //inGame lets us know if the game is in progress
    
    int position=2, gameStart=0;
    int ch;
    while (!gameStart) {
        printMenu();
        printOptions(position, inGame);
        ch = getch();
        switch(ch) {
            case '\n':
                switch(position) {
                    case 0:
                        endGame=1;
                        gameStart=1;
                        break;
                    case 1:
                        highScoreMenu();
                        break;
                    case 2:
                        gameStart=1;
                        break;
                }
                break;
            case KEY_UP:
                if(position<4)
                    position++;
                break;    
            case KEY_DOWN:
                if(position>0)
                    position--;    
                break;
            case KEY_RIGHT:
                if(position==4 && gameSpeed<5)
                    gameSpeed++;
                else if(position==3 && userLevel<5)
                    userLevel++;
                break;
            case KEY_LEFT:
                if(position==4 && gameSpeed>1)
                    gameSpeed--;
                else if(position==3 && userLevel>1)
                    userLevel--;
                break;
        }
        refresh();
    }
    clearMenu();
    
}

//=================================
//DEATH ANIMATION / EXPLODING BITS
//=================================

void generateBits(struct bit *bits, int x, int y){
    for(int i=0; i<8; i++) {
        bits[i].x=x;
        bits[i].y=y;
        bits[i].dir=i;
    }
}

void paintBits(struct bit *bits, char ch){
    //For next level, we're gonna do color changing balls
    attron(COLOR_PAIR(1));
    for(int i=0; i<8; i++) {
        if(winCondition == 2 || wonWholeGame)
            attron(COLOR_PAIR(rand()%6+1)); //CELEBRATION BITS
        mvaddch(bits[i].x, bits[i].y, ch);
    }
    attroff(COLOR_PAIR(1));
}

void advanceBits(struct bit *bits) {
    int char_at;
    for(int i=0; i<8; i++) {
        switch(bits[i].dir) {
            case 0: //up
                char_at = mvinch(bits[i].x-1, bits[i].y) & A_CHARTEXT;
                if (char_at == '-' || char_at == '=' || char_at == '*') {
                    bits[i].dir = 4;    
                    bits[i].x += 1;
                } else bits[i].x -= 1;
                break;
            case 1: //up-right
                char_at = mvinch(bits[i].x-1, bits[i].y+1) & A_CHARTEXT;
                if (char_at == '|' || char_at == '*') {
                    bits[i].dir = 7;    
                    bits[i].x -= 1;
                    bits[i].y -= 1;
                } 
                if (char_at == '-' || char_at == '=') {
                    bits[i].dir = 3;
                    bits[i].x += 1;
                    bits[i].y += 1;
                } else {
                    bits[i].x -= 1;
                    bits[i].y += 1;
                }
                break;
            case 2: //right
                char_at = mvinch(bits[i].x, bits[i].y+1) & A_CHARTEXT;
                if (char_at == '|' || char_at == '*' || char_at == '=') {
                    bits[i].dir = 6;
                    bits[i].y -= 1;
                } else bits[i].y += 1;
                break;
            case 3: //down-right
                char_at = mvinch(bits[i].x+1, bits[i].y+1) & A_CHARTEXT;
                if (char_at == '|' || char_at == '*') {
                    bits[i].dir = 5;
                    bits[i].x += 1;
                    bits[i].y -= 1;
                } 
                if (char_at == '-' || char_at == '=') {
                    bits[i].dir = 1;
                    bits[i].x -= 1;
                    bits[i].y += 1;
                } else {
                    bits[i].x += 1;
                    bits[i].y += 1;
                }
                break;
            case 4: //down
                char_at = mvinch(bits[i].x+1, bits[i].y) & A_CHARTEXT;
                if (char_at == '-' || char_at == '=' || char_at == '*') {
                    bits[i].dir = 0;
                    bits[i].x -= 1;
                } else bits[i].x += 1;
                break;
            case 5: //down-left
                char_at = mvinch(bits[i].x+1, bits[i].y-1) & A_CHARTEXT;
                if (char_at == '|' || char_at == '*') {
                    bits[i].dir = 3;
                    bits[i].x += 1;
                    bits[i].y += 1;
                } 
                if (char_at == '-' || char_at == '=') {
                    bits[i].dir = 7;
                    bits[i].x -= 1;
                    bits[i].y -= 1;
                } else {
                    bits[i].x += 1;
                    bits[i].y -= 1;
                }
                break;
            case 6: //left
                char_at = mvinch(bits[i].x, bits[i].y-1) & A_CHARTEXT;
                if (char_at == '|' || char_at == '*' || char_at == '=') {
                    bits[i].dir = 2;
                    bits[i].y += 1;
                } else bits[i].y -= 1;
                break;
            case 7: //up-left
                char_at = mvinch(bits[i].x-1, bits[i].y-1) & A_CHARTEXT;
                if (char_at == '|' || char_at == '*') {
                    bits[i].dir = 1;
                    bits[i].x -= 1;
                    bits[i].y += 1;
                } 
                if (char_at == '-' || char_at == '=') {
                    bits[i].dir = 5;
                    bits[i].x += 1;
                    bits[i].y -= 1;
                } else {
                    bits[i].x -= 1;
                    bits[i].y -= 1;
                }
                break;
        }
    }
}

void DeathAnimation(int win){ //0 for DEATH, 1 for WIN
    struct snake_char * erase = (struct snake_char *)malloc(sizeof(struct snake_char));
    
    if(!win) { //ONLY IF DEATH
        //RED AND YELLOW FLASHING BEFORE DEATH
        for(int i=0; i<2; i++) {
            
            erase = head;
            attron(COLOR_PAIR(5));
            mvaddch(erase->x, erase->y, HEAD_CHAR);
            erase=erase->prev;
            while(erase->prev != NULL) {
                mvaddch(erase->x, erase->y, SNAKE_CHAR);
                erase=erase->prev;
            }
            mvaddch(erase->x,erase->y,SNAKE_CHAR);
            refresh();
            usleep(140000);

            erase = head;
            attron(COLOR_PAIR(1));
            mvaddch(erase->x, erase->y, HEAD_CHAR);
            erase=erase->prev;
            while(erase->prev != NULL) {
                mvaddch(erase->x, erase->y, SNAKE_CHAR);
                erase=erase->prev;
            }
            mvaddch(erase->x,erase->y, SNAKE_CHAR);
            refresh();
            usleep(140000);
        }
        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(5));
    }
    
    //erase snake from screen
    erase = head;
    while(erase->prev != NULL) {
        mvaddch(erase->x,erase->y,' ');
        erase=erase->prev;
    }
    mvaddch(erase->x,erase->y,' ');

    printScoreMenu();
    printScoreOptions(3);

    int bits1RandomX = 15; //we assume initially they're "celebration" bits
    int bits1RandomY = 15;
    int bits2RandomX = 15;
    int bits2RandomY = COLS - 15;

    if (!win) { //if Death, we want to explode on snake so change coordinates
        bits1RandomX = head->x;
        if(bits1RandomX>=LINES-4)
            bits1RandomX -= 2;
        else if (bits1RandomX<=1)
            bits1RandomX += 2;

        bits1RandomY = head->y;
        if(bits1RandomY>=COLS-2)
            bits1RandomY -= 2;
        else if (bits1RandomY<=1)
            bits1RandomY += 2;

        bits2RandomX = tail->x;
        if(bits2RandomX>=LINES-4)
            bits2RandomX -= 2;
        else if (bits2RandomX<=1)
            bits2RandomX += 2;

        bits2RandomY = tail->y;
        if(bits2RandomY>=COLS-2)
            bits2RandomY -= 2;
        else if (bits2RandomY<=1)
            bits2RandomY += 2;

        if((head->x>=LINES/2-6 && head->x<=LINES/2+5) && (head->y>=COLS/2-15 && head->y<=COLS/2+15))
            do {
                bits1RandomX = rand()%LINES - 5;
                bits1RandomY = rand()%COLS - 1;
                if(bits1RandomX < 2)
                    bits1RandomX = 2;
                if(bits1RandomY < 2)
                    bits1RandomY = 2;
            } while ((bits1RandomX>=LINES/2-6 && bits1RandomX<=LINES/2+5) && (bits1RandomY>=COLS/2-15 && bits1RandomY<=COLS/2+15));
        
        if((tail->x>=LINES/2-6 && tail->x<=LINES/2+5) && (tail->y>=COLS/2-15 && tail->y<=COLS/2+15))
            do {
                bits2RandomX = rand()%LINES - 5;
                bits2RandomY = rand()%COLS - 1;
                if(bits2RandomX < 2)
                    bits2RandomX = 2;
                if(bits2RandomY < 2)
                    bits2RandomY = 2;
            } while ((bits2RandomX>=LINES/2-6 && bits2RandomX<=LINES/2+5) && (bits2RandomY>=COLS/2-15 && bits2RandomY<=COLS/2+15));
    }

    generateBits(bits1, bits1RandomX, bits1RandomY); //start at head of snake
    generateBits(bits2, bits2RandomX, bits2RandomY); //start at tail of snake
    
    do {
        //ch = getch();
        paintBits(bits1, BITS_CHAR); //paint bits with BITS_CHAR
        paintBits(bits2, BITS_CHAR);
        refresh();
        usleep(50000);
        if(!wonWholeGame) { //when they win the game, as celebration we want bits to stay and paint screen
            paintBits(bits1, ' '); //clear bits with ' '
            paintBits(bits2, ' ');
        }
        advanceBits(bits1);
        advanceBits(bits2);
    } while(getch() != ' ');
    clearMenu();
}

//===========================
//Snake Intro Splash Screen
//===========================
void animateSplashScreen() { //DOUBT WE'RE GONNA DO THIS
    int border=0;
    char ch;
    for (int i=0; i<31; i++) {
            if (ch == ' ')
                break;
            if(i<=20) {
                attron(COLOR_PAIR(1));
                mvaddch(LINES/2-4, (COLS/2+2)-border, '*');
                mvaddch(LINES/2+5, (COLS/2+2)+border++, '*');
            }
            attroff(COLOR_PAIR(1));    
            usleep(50000);
            refresh();
        }
    
}

void introSplashScreen() {
    char letterS[] = 
    "  .d8888b.   d88P  Y88b  Y88b.        \"Y888b.        \"Y88b.        \"888  Y88b  d88P   \"Y8888P\"  ";
    char letterN[] =
    "888b    888 8888b   888 88888b  888 888Y88b 888 888 Y88b888 888  Y88888 888   Y8888 888    Y888 ";
    char letterA[] =
    "       d8888      d88888     d88P888    d88P 888   d88P  888  d88P   888 d8888888888d88P     888";
    char letterK[] =
    "888    d8P  888   d8P   888  d8P    888d88K     8888888b    888  Y88b   888   Y88b  888    Y88b ";
    char letterE[] =
    "8888888888  888         888         8888888     888         888         888         8888888888  ";
    int current_line = LINES/2-4;
    int current_col = COLS/2-30;
    int border=0, sideBorder=0, printVersion=0;
    attron(COLOR_PAIR(5));
    attron(A_BOLD);
    for (int i=0; i<strlen(letterS); i++) {
        if(i%12==0) {
            current_line++;
            current_col = COLS/2-30;
        }
        mvaddch(current_line, current_col, letterS[i]);
        mvaddch(current_line, current_col+13, letterN[i]);
        mvaddch(current_line, current_col+26, letterA[i]);
        mvaddch(current_line, current_col+39, letterK[i]);
        mvaddch(current_line, current_col+52, letterE[i]);
        current_col++;

        if ((i>=strlen(letterS)/2) && border<=33) {
            mvaddch(LINES/2-4, (COLS/2+2)-border,'*');
            mvaddch(LINES/2-4, COLS/2+border,'*');
            mvaddch(LINES/2+5, (COLS/2+2)-border,'*');
            mvaddch(LINES/2+5, COLS/2+border++,'*');
        }
        if (i>=strlen(letterS)-5) {
            mvaddch(LINES/2-sideBorder, COLS/2-31,'*');
            mvaddch(LINES/2+sideBorder, COLS/2-31,'*');
            mvaddch(LINES/2-sideBorder, COLS/2+33,'*');
            mvaddch(LINES/2+sideBorder++, COLS/2+33,'*');
        }
        if (i>=strlen(letterS)-11)
            mvaddch(LINES/2+6, COLS/2+23+printVersion, version[printVersion++]);
        refresh();
        usleep(20000);
    }
    attroff(A_BOLD);
    attron(A_BLINK);
    mvprintw(LINES/2+8, COLS/2-11, "(Press Space to Continue)");
    playAnimation=0;
    attroff(A_BLINK);
     
    char ch;
    int exitSplash=0;
    border=0;
    sideBorder=0;
    while(!exitSplash) {
        //animateSplashScreen();
        ch = getch();
        if (ch == ' ')
            exitSplash=1;
    }
    attroff(COLOR_PAIR(5));
    clearGameBoard();
}
