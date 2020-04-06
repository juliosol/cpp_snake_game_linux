/*************************************************************************************************
 * Code written by Julio Soldevilla making variations to the code written by ThePoorEngineer
*************************************************************************************************/

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stropts.h>
#include <stdio.h>
#include <curses.h>
#include <pthread.h>

using namespace std;

/// Defining Global Variables

#define MAX 50
#define WIDTH 40
#define HEIGHT 40
#define FOOD 1
#define SNAKE_LENGTH 4

// We will more global variables to let the whole environment know we are referring 

#define SCORE -6
#define GAMEOVER -5
#define HEAD -3
#define BODY -4
#define SNAKE -1
#define WALL -2
#define NOTHING 0
#define EXIT -1

// We will also use global variables for the movement of the snake
#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3
static int dx[5] = {1, 0, -1, 0};
static int dy[5] = {0, -1, 0, 1};

int item = NOTHING; // Current item the snake has.
int input = RIGHT;
int bytesWaiting, i;
const char* clearcommand = "clear";
unsigned int microseconds;

void gotoxy(int x,int y)
{
    printf("%c[%d;%df",0x1B,y,x);
}

// Function to check if a key was hit or not.
// Taken from http://www.flipcode.com/archives/_kbhit_for_Linux.shtml 
// As an option for getch()
int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    //int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

void clear_background(void){
    system(clearcommand);
}

int oppositeDirection(int input1, int input2)
{
    if (input1 == RIGHT && input2 == LEFT)
        return 1;
    if (input1 == LEFT && input2 == RIGHT)
        return 1;
    if (input1 == UP && input2 == DOWN)
        return 1;
    if (input1 == DOWN && input2 == UP)
        return 1;
    return 0;
}

struct Coordinate
{
    int x;
    int y;
};

class snake
{

private:
    int length;
    int foodCounter;
    int direction;
    int ground[MAX][MAX];
    Coordinate body[HEIGHT * WIDTH];
public:
    void initSnake();
    void initGround();
    void updateFood();
    void updateSnake(int delay);
    void inputReading(const char k);
    //void updateBackground();
    int getFoodCounter();
    void drawBckgrd();
    void drawStarterMenu(int menuDelay);
    void drawResultWindow();
};

void snake::initSnake()
{
    length = SNAKE_LENGTH;
    body[0].x = WIDTH / 2;
    body[0].y = HEIGHT / 2;
    foodCounter = 0;
    direction = input;

    for (int i = 1; i < length; i++)
    {  
        body[i].x = body[i-1].x - dx[direction];
        body[i].y = body[i-1].y - dy[direction];
    }

    for (int i = 0; i < length; i++)
    {
        ground[body[i].y][body[i].x] = SNAKE;
    }

};

void snake::initGround()
{
    int i,j;
    for (i=0; i<MAX; i++)
        for (j=0; j<MAX; j++)
            ground[i][j] = 0;

    for (i = 0; i <= WIDTH + 1; i++)
    {
        ground[0][i] = WALL;
        ground[HEIGHT + 1][i] = WALL;
    }
    
    for (i = 0; i <= HEIGHT + 1; i++)
    {
        ground[i][0] = WALL;
        ground[i][WIDTH+1] = WALL;
    }
};

int snake::getFoodCounter()
{
    return foodCounter;
};

void snake::updateFood()
{
    int x,y;
    do
    {
        x = rand() % WIDTH + 1;
        y = rand() % HEIGHT + 1;
    } while (ground[y][x] != NOTHING);

    ground[y][x] = FOOD;
    foodCounter++;
};

void snake::updateSnake(int delay)
{
    int i;
    Coordinate prev[HEIGHT*WIDTH];
    for (i = 0; i < length; i++)
    {
        prev[i].x = body[i].x;
        prev[i].y = body[i].y;
    }

    if (input != EXIT || !oppositeDirection(direction, input))
        direction = input;

    // Update head
    body[0].x = prev[0].x + dx[direction];
    body[0].y = prev[0].y + dy[direction];
    
    // Check if head position is not colliding with anything
    if (ground[body[0].y][body[0].x] < NOTHING )
    {
        item = -1;
        ground[HEIGHT+2][WIDTH/2] = GAMEOVER;
        return;
    }

    // If head finds a piece of food, should
    if (ground[body[0].y][body[0].x] == FOOD)
    {
        length++;
        item = FOOD;
    }
    else
    {
        ground[body[length -1].y][body[length-1].x] = NOTHING;
        item = NOTHING;
    }

    ground[body[0].y][body[0].x] = SNAKE;

    // Move the rest of the body
    for (i=1; i < length; i++)
    {
        body[i].x = prev[i-1].x;
        body[i].y = prev[i-1].y;
        ground[body[i].y][body[i].x] = SNAKE;
    }
    
    ground[HEIGHT+3][WIDTH/2] = SCORE;

    usleep(delay);

    return;
};


void snake::drawBckgrd()
{
    clear_background();
    int i, j;
    for (i = 0; i <= HEIGHT+4; i++)
    {
        for (j = 0; j <= WIDTH+4; j++)
        {
            switch (ground[i][j])
            {
                case NOTHING:
                    cout << " "; break;
                case WALL:
                    if ((i == 0 && j == 0)
                        || (i == 0 && j == WIDTH + 1) 
                        || (i == HEIGHT + 1 && j == 0) 
                        || (i == HEIGHT + 1 && j == WIDTH + 1))
                        cout << "+";
                    else
                        if (j == 0 || j == HEIGHT + 1)
                            cout << "|";
                        else
                            cout << "-"; 
                    break;
                case SNAKE:
                    if (i == body[0].y && j == body[0].x)
                        cout << "#";
                    else
                        cout << "+";
                    break;
                case FOOD:
                    cout << "%";
                    break;
                case GAMEOVER:
                    cout << "***** GAME OVER! *****";
                    break;
                case SCORE:
                    cout << "The score is " << (foodCounter - 1);
                    break;
            }
        }
        cout << endl;
    }
};

void snake::inputReading(const char k)
{
    if (k == 'd'||k == '6')
    {
        input = RIGHT;
    }
    if (k == 'w' || k == 8)
    {
        input = UP;
    }
    if (k == 'a' || k == 4)
    {
        input = LEFT;
    }
    if (k == 's' || k == 2)
    {
        input = DOWN;
    }
};

int main()
{
    pthread_t ptid;
    int delay =1;
    char k;
    microseconds = 100000;
	srand(time(NULL));
	snake serpe;
	serpe.initGround();
	serpe.initSnake();
    serpe.updateFood();
    serpe.drawBckgrd();
    
    do
    {
        if (_kbhit())
        {
            cin >> k;
            serpe.inputReading(k);
        }
        serpe.updateSnake(microseconds);
        serpe.drawBckgrd();
        if (item == FOOD)
        {
            serpe.updateFood();
        }
    } while (item >= 0 && input != EXIT);

    serpe.drawBckgrd();

    return 0;
}
