#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <mutex>
#include <cmath>
#include <string>
 
class PONG {
private:
    static const int SIZE = 40;
    static const char BALL = 'O';
    static const char PADDLE = '#';
    static const int PADDLESIZE = 5;
    static const int RUNNING = 1;
    static const int STOPPED = 0;
    static const int ENDSCORE = 5;
 
    int paddle1 = 10; // Uppermost "Pixel" of the Paddle
    int paddle2 = 10; 
    std::vector<int> ball = {20, 20};
    std::vector<double> speed = {0, 1}; //Initial Speed 
    std::vector<int> score = {0, 0};
    std::vector<double> bounce = {-1, -0.5, 0, 0.5, 1};
    double vspeed_leftover = 0;
    int framedivisor = 1;
    int status = RUNNING; // status of the game
    int winner = 0; // winner condition 
    std::mutex mtx; // mutex for thread-safe access
    bool simple;
 
public:
    bool replay = false;

    void clearScreen() {
        std::cout << "\033[2J\033[H"; // Clear Screen and position the Cursor
        std::cout << "\033[?25l"; // Hide the Cursor
    }
 
    void drawWalls() {
        std::cout << "\x1b[0m"; // set draw color to default
        for (int i = 1; i <= SIZE; ++i) {
            std::cout << "\033[1;" << i << "H─"; // Wall
            std::cout << "\033[" << SIZE << ";" << i << "H─"; // Wall
        }
    }
 
    void drawPaddles() {
        std::cout << "\x1b[31m"; // set draw color to red
        for (int i = 0; i < PADDLESIZE; i++) {
            std::cout << "\033[" << paddle1 + i << ";" << 1 << "H"; // Paddle1 is alwys in COL 1
            std::cout << PADDLE;
            std::cout << "\033[" << paddle2 + i << ";" << SIZE << "H"; // Paddle1 is alwys in COL SIZE
            std::cout << PADDLE;
        }

        std::cout << "\x1b[0m"; // reset color
    }

    void drawBall() {
        std::cout << "\x1b[31m"; // set draw color to red
        std::cout << "\033[" << ball[0] << ";" << ball[1] << "H"; // set cursor to ball position
        std::cout << BALL;
        std::cout << "\x1b[0m"; // reset color
    }

    void drawScore() {
        /* draw the Score under the Playfield */
        std::cout << "\x1b[31m"; // set draw color to red
        std::cout << "\033[" << SIZE+2 << ";" << 15 << "H" << score[0]; // Score Player 1
        std::cout << "\033[" << SIZE+2 << ";" << 20 << "H" << ":"; // ":"
        std::cout << "\033[" << SIZE+2 << ";" << 25 << "H" << score[1]; // Score Player 2
        std::cout << "\x1b[0m"; // reset color
    }

    void drawWinner() {
        /* draw the Winner under the Score */
        std::cout << "\x1b[31m"; // set draw color to red
        std::cout << "\033[" << SIZE+3<< ";" << 13 << "H";
        std::cout << "WINNER PLAYER: " << winner << std::endl;
        std::cout << "\x1b[0m"; // reset color
    }

    void moveBall() {
        /* move the Ball to his new Position */
        // use this to handle 0.5-Values
        double vspeed = speed[0];
        if(speed[0] == 0.5) {
            vspeed = std::floor(speed[0] + vspeed_leftover);
            vspeed_leftover = speed[0] + vspeed_leftover - vspeed;
        }
        else if (speed[0] == -0.5)  {
            vspeed = std::ceil(speed[0] + vspeed_leftover);
            vspeed_leftover = speed[0] + vspeed_leftover - vspeed;
        }

        // actually move the ball
        ball[0] += vspeed;
        ball[1] += speed[1];
    }

    void startNewRound() {
        ball = {20, 20};
        setStartDirection();
        framedivisor = 1;
        vspeed_leftover = 0;
    }

    void checkBallCollision() {
        /* check if the Ball hits something */
        // First: handle Cornercases
        if(ball[0] <= 1 && ball[1] <= 1 && paddle1 == 2) {
            ball[1] = 2;
            framedivisor++;
            speed[0] = bounce[0];
            speed[1] *= -1;
        } else if ((ball[0] <= 1 && ball[1] <= SIZE && paddle1 == SIZE - PADDLESIZE)) {
            ball[1] = 2;
            framedivisor++;
            speed[0] = bounce[4];
            speed[1] *= -1;
        } else if ((ball[0] <= SIZE && ball[1] <= 1 && paddle2 == 2)) {
            ball[1] = SIZE - 1;
            framedivisor++;
            speed[0] = bounce[0];
            speed[1] *= -1;
        } else if ((ball[0] <= SIZE && ball[1] <= SIZE && paddle2 == SIZE - PADDLESIZE)) {
            ball[1] = SIZE - 1;
            framedivisor++;
            speed[0] = bounce[4];
            speed[1] *= -1;
        }
        // Second: Check if a Wall is hit
        else if(ball[0] <= 1 || ball[0] >= SIZE) {
            speed[0] *= -1;
            framedivisor++;
            vspeed_leftover = 0; //to avoid always 0 when -0.5 + 0.5
            ball[0] = (ball[0] < 2) ? 2 : SIZE-1;
        } else if(ball[1] <= 1) { // 3rd: Check if left Paddle is hit
            for (int i = 0; i < PADDLESIZE; i++) {
                if(ball[0] == paddle1 + i) {
                    ball[1] = 2;
                    speed[0] = bounce[i]; // bounce, depending where the ball hit the paddle
                    speed[1] *= -1;
                    framedivisor++;
                    break;
                } else if(i == PADDLESIZE - 1) {
                    score[1] += 1;
                    if(score[1] == ENDSCORE) {
                        winner = 2;
                        status = STOPPED;
                    } else {
                        startNewRound();
                    }
                }
            }
        } else if (ball[1] >= SIZE) { // 4th: Check if right Paddle is hit
            for (int i = 0; i < PADDLESIZE; i++) {
                if(ball[0] == paddle2 + i) {
                    ball[1] = SIZE-2;
                    speed[0] = bounce[i]; // bounce, depending where the ball hit the paddle
                    speed[1] *= -1;
                    framedivisor++;
                    break;
                } else if(i == PADDLESIZE - 1) {
                    score[0] += 1;
                    if(score[0] == ENDSCORE) {
                        winner = 1;
                        status = STOPPED;
                    } else {
                        startNewRound();
                    }
                }
            }
        }
    }

    void movePaddels() {
        initscr();              // start ncurses-Mode
        timeout(50);            // set Timeout for dejittering
        keypad(stdscr, TRUE);   // activate arrow-keys
        cbreak();               // activate raw-mode for ncurses
        noecho();               // disable output from keyboard entry
 
        while (status == RUNNING) {
            int input = getch(); // non-blocking variant
 
            // Use a mutex to ensure thread-safe access to direction
            std::lock_guard<std::mutex> lock(mtx);
            switch (input) {
                case 'w':
                    paddle1 -= 1;
                    paddle1 = (paddle1 < 2) ? 2 : paddle1;  
                    break;
                case 's':
                    paddle1 += 1;
                    paddle1 = (paddle1 >35) ? 35 : paddle1;
                    break;
                case KEY_UP:
                    paddle2 -= 1;
                    paddle2 = (paddle2 < 2) ? 2 : paddle2;  
                    break;
                case KEY_DOWN:
                    paddle2 += 1;
                    paddle2 = (paddle2 >35) ? 35 : paddle2;
                    break;
            }
        }
 
        endwin(); // Close ncurses mode
    }
 
    void timeControl() {
        /* set gamespeed */
        int sleep = (400 + framedivisor +1 ) / framedivisor;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }

    void initGame() {
        /* init the variables needed */
        setStartDirection();
    }

    void playGame() {
        /* loopfunction for the gameplay */
        // TODO: use 4 Threads (!)
        std::thread steering_thread([this]() {
            movePaddels();
        });
 
        while (status == RUNNING) {
            timeControl();
            clearScreen();
            drawPaddles();
            drawBall();
            drawWalls();
            drawScore();
            moveBall();
            checkBallCollision();
            std::cout << std::flush; // Needed for a stable drawing
        }
 
        // Stop steering
        steering_thread.join(); // Wait for the steering thread to finish
    }
 
    void setStartDirection() {
        /* set start direction of the ball */
        std::mt19937 generator(static_cast<unsigned int>(std::time(0)));
        std::uniform_int_distribution<int> distribution(0, 1);

        double direction = (distribution(generator)) ? -1 : 1; 
        speed = {0 , direction};
    }

    void drawGameEnd() {
        /* show score */
        clearScreen();
        drawPaddles();
        drawWalls();
        drawScore();
        drawWinner();
    }

    void askReplay() {
        /* ask for replay */
        initscr();              // start ncurses-Mode
        cbreak();               // activate raw-mode for ncurses 

        char user_in;
        std::cout << "\033[" << SIZE+4<< ";" << 0 << "H";
        std::cout << "Neues Spiel? (j): ";
        std::cin >> user_in;
        replay = (user_in == 'j' || user_in == 'J');

        endwin();
    }

    void endGame(PONG &pong) {
        drawGameEnd();
        askReplay();
    }
};

int main(int argc, char* argv[]) {
    bool replay = false;

    do {
        PONG pong;
        pong.initGame();
        pong.playGame();
        pong.endGame(pong);
        replay = pong.replay;
    } while (replay);
}