# Pong for 2Players
Let play us pong, player vs Player

## Compile
I use simple g++ for compiling and add the ncurses-Header
> g++ -std=c++17 -o Pong Pong.cpp -lncurses

## Start the game
Use this in the folder containing the Pong-Binary (Terminal should have at least 42x42 [columnsxrows])
> ./Pong 

## Steps
(/) Draw Walls
(/) Paddles
  (/) Draw Paddles
  (/) Move Paddles
( ) Add Ball
  (/) Draw Ball
  (/) Let the Ball Move L/R
  (/) Add Collision Detection with Speed up
  (/) Add Collision Detection with bounce off
(/) add Score
  (/) Score when Ball hits the edge
  (/) Show Score
(/) draw GameEnd
( ) adjust TimeControll
  * Framerate has to be independt from ballspeed, so that Paddlemovement is feeling naturally
  * Need 3 seperate Threads
    * ControllThread --> takes Key-Inputs
      * we need a seperate Thread for both Players!
    * PhysicsThread --> calculate GameMachanics (here: Movement of the ball and colission)
    * RenderThreads --> Draw the game, limited to ~60 FPS (1000 ms / 60 ~ 17 ms) --> MainThread
( ) improve PaddleColission
  * that is two time the same Code