#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    int row, col, curr_x, curr_y;//terminal height, width, current x and y of snake head
    int ch;
    char dir = 'l'; //current direction l, r, u, or d start by going left.

    WINDOW *mywin; //game window


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

    //color pair for border
    init_pair(2, COLOR_RED, COLOR_RED);

    //get the size of vthe terminal window
    getmaxyx(stdscr,row,col);
    noecho();

    //get center of terminal window
    curr_x = row / 2;
    curr_y = col / 2;

    //create border (window, left side, right side, top, bottom, corner, corner, corner, corner) and apply color
    wattron(stdscr, COLOR_PAIR(2));
    wborder(stdscr, '|', '|', '-', '-', '+', '+', '+', '+');
    attroff(COLOR_PAIR(2));

    //create window for snake to live in sitting inside border
    mywin = newwin(row - 2, col - 2, 1,1);
    refresh(); //refresh the main screen to apply changes

    //turn on color for snake window
    wattron(mywin, COLOR_PAIR(1));

    //print the snake head 
    mvwaddch(mywin, curr_x, curr_y, ACS_BLOCK);
    wrefresh(mywin);//refresh snake window to apply changes

    //add the keypad listener
    keypad(mywin, TRUE);
    //get a keystroke to start
    wgetch(mywin);
    //dont wait for inputs ie getch returns error if no keystroke
    nodelay(mywin, TRUE);

    //main game loop
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
        }

        //break if backspace
        if(ch == KEY_BACKSPACE){
            break;
        }

        //delete snake head
        mvwdelch(mywin, curr_x, curr_y);

        //check current direction and move xy coordinates of snake head
        if(dir == 'l'){
            if(curr_y == 0);//if statement detects collision with border
            else
                curr_y--;
        }else if(dir == 'r'){
            if(curr_y == col-3);//col - 3 is location of bottom of screen
            else
                curr_y++;
        }else if(dir == 'u'){
            if(curr_x == 0);
            else
                curr_x--;
        }else{
            if(curr_x == row-3);//row - 3 is rightmost border
            else
                 curr_x++;
        }

        //redraw head in new location
        mvwaddch(mywin, curr_x, curr_y, ACS_BLOCK );

        //refresh the window to apply changes
        wrefresh(mywin);
        delay_output(250);//wait 250ms or .25 sec
        
    }
    //turn off colors and end curses
    attroff(COLOR_PAIR(1));
    endwin();
    return 0;
}