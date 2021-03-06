#ifndef __PLAY_H__
#define __PLAY_H__

#include "Gamemode.hpp"
#include "../Pong/Object.hpp"
#include "../Pong/Controller.hpp"
#include "../Pong/Player.hpp"
#include "../Pong/User.hpp"
#include "../Pong/Ball.hpp"
#include "../Pong/Text.hpp"
#include "../Pong/GameRenderer.hpp"
#include "../NeuralNetwork/Sensor.hpp"
#include "../NeuralNetwork/AI.hpp"
#include "../NeuralNetwork/NetworkHandler.hpp"
#include "../definitions.hpp"

#include <string>
#include <cmath>
#include <windows.h>
#include <io.h>

class Play : public Gamemode {
    friend class PlayTests; // for unit testing purpose
    private:
        //Controller* left_controller = nullptr;
        Player* left_paddle = nullptr;
        //Controller* right_controller = nullptr;
        Player* right_paddle = nullptr;
        Ball* ball = nullptr;
        Text* score_l = nullptr;
        Text* score_r = nullptr;
        int score_left = 0;
        int score_right = 0;
        bool turn = 0; // turn is 1 or 0 == player 1'turn or player 2's turn

    public:
        Play(string input) : Gamemode() {
            if (TTF_Init() < 0) {
                fprintf(stderr, "Could not init TTF\n", SDL_GetError());
                throw "Could not init TTF\n";
            }

            // set up ball
            ball = new Ball();
            ball->setSpeed(BALL_SPEED * 2);

            Controller* right_controller;

            if (input == "1") {
                SPEED = 12.5;
                ball->setSpeed(9 * 2);
                right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_ral896q24j/4_3_1_5_score13_kirq024328"));
            }
            else if (input == "2") {
                SPEED = 12.5;
                ball->setSpeed(9 * 2);
                right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_w1amn7x1h9/4_3_1_5_score58_6lup69i97x"));
            }
            else if (input == "3") {
                SPEED = 15.0;
                ball->setSpeed(9 * 2);
                right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_fenqh117a3/4_3_1_5_score1598_9ns5o6310d"));
            }
            else if (input == "4") {
                SPEED = 12.5;
                ball->setSpeed(14 * 2);
                right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_92eqfsd939/3_3_1_5_score6184_a17f88g27w"));
            }
            else if (input == "5") {
                SPEED = 12.5;
                ball->setSpeed(14 * 2);
                right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_4o5hoxxzm1/3_3_1_5_score748_xt75k0v150"));
            }
            else if (input == "6") {
                SPEED = 9.0;
                ball->setSpeed(14 * 2);
                right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_4o5hoxxzm1/3_3_1_5_score748_xt75k0v150"));
            }
            else {
                string filename = "../saves/";
                filename += input;
                right_controller = new AI(new Sensor(ball), new NeuralNetwork(input));
            }

            // set up right user player
            //Controller* right_controller = new AI(new Sensor(ball), new NeuralNetwork("../saves/save_state_w1amn7x1h9/4_3_1_5_score4081_7g4ey57126"));
            right_paddle = new Player(right_controller, WIDTH-32,(HEIGHT/2)-(HEIGHT/8),(HEIGHT/HEIGHT_RATIO),12);
            right_paddle->randomize_color();

            // set up left user player
            Controller* left_controller = new User(PLAYER_UP, PLAYER_DOWN);
            left_paddle = new Player(left_controller, 32, (HEIGHT/2)-(HEIGHT/8), (HEIGHT/HEIGHT_RATIO), 12);
            left_paddle->randomize_color();

            // set up static texts
            // Text* message = new Text("Press ESCAPE to exit", 50);
            // message->create_text(renderer);
            // message->set_text_pos(300, 0); // settings related to the text's position needs to be called after create()

            // add all created game objects to gameRend for rendering
            gameRend.add(left_paddle);
            gameRend.add(right_paddle);
            gameRend.add(ball);

            // initial scores
            score_r = new Text(to_string(score_right).c_str(), 100, make_pair(920,0));
            score_r->create(renderer);
            gameRend.add(score_r);
            score_l = new Text(to_string(score_left).c_str(), 100, make_pair(280,0));
            score_l->create(renderer);
            gameRend.add(score_l);

            serve(turn);
        }

        ~Play() {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();

            cout << "destructing" << endl;

            //delete left_controller;
            delete left_paddle;
            //delete right_controller;
            delete right_paddle;
            delete ball;
            //delete score_l;
            //delete score_r;

            cout << "destructing" << endl;
        }

        // render all objects on screen and run the game
        virtual void update(bool &running) {
            lastFrame = SDL_GetTicks();
            if(lastFrame >= (lastTime + 1000)) {
                lastTime = lastFrame;
                fps = frameCount;
                frameCount = 0;
            }
            update(renderer, turn, score_left, score_right);
            input(running);
            left_paddle->get_input();
            right_paddle->get_input();

            gameRend.render_all(renderer, frameCount, timerFPS, lastFrame);

            return;
        }

    private:
        void serve(bool &turn){
            // cout scores in a new serve
            // cout << "Player LEFT: " << score_left << endl;
            // cout << "Player RIGHT: " << score_right << endl << endl;

            if(turn) { // turn == 1 == left's turn to serve
                left_paddle->setY((HEIGHT/2) - (left_paddle->getH())/2); //sets the paddles in place
                right_paddle->setY(left_paddle->getY()+5); // right paddle will be a bit off
                ball->setX(left_paddle->getX() + (left_paddle->getW()*4)); //serves ball
                ball->setVelX(ball->getSpeed()/2);
            }
            else {  // turn == 0 == right's turn to serve
                right_paddle->setY((HEIGHT/2) - (right_paddle->getH())/2); //sets the paddles in place
                left_paddle->setY(right_paddle->getY()+5); // left paddle will be a bit off
                ball->setX(right_paddle->getX() - (right_paddle->getW()*4)); //serves ball
                ball->setVelX(ball->getSpeed()/-2);
            }
            ball->setVelY(0);
            ball->setY((HEIGHT/2)-8);
            turn =! turn; // change turn

            return;
        }

        void update(SDL_Renderer* renderer, bool &turn, int &score_left, int &score_right){
            SDL_Rect b1 = ball->getRect();
            SDL_Rect lp = left_paddle->getRect();
            SDL_Rect rp = right_paddle->getRect();

            if(SDL_HasIntersection(&b1, &rp)){ //checks if ball and RIGHT paddle interact
                double rel = (right_paddle->getY()+(right_paddle->getH()/2))-(ball->getY()+8);
                double norm = rel/(right_paddle->getH()/2);
                double bounce = norm * (5*PI/12);
                ball->setVelX((ball->getSpeed()*-1)*cos(bounce)); //sends ball at different angle based on where the ball has hit the paddle
                ball->setVelY((ball->getSpeed())*-sin(bounce));
            }
            if(SDL_HasIntersection(&b1, &lp)){ //checks if ball and LEFT paddle interact
                double rel = (left_paddle->getY()+(left_paddle->getH()/2))-(ball->getY()+8);
                double norm = rel/(left_paddle->getH()/2);
                double bounce = norm * (5*PI/12);
                ball->setVelX((ball->getSpeed()*1)*cos(bounce)); //sends ball at different angle based on where the ball has hit the paddle
                ball->setVelY((ball->getSpeed())*-sin(bounce));
            }

            if(ball->getY() <= 0 || ball->getY() + 16 >= HEIGHT) ball->setVelY(ball->getVelY()*-1); //check to see if ball hit top or bottom walls
            ball->setX(ball->getVelX() + ball->getX()); //ball movement
            ball->setY(ball->getVelY() + ball->getY());

            if(left_paddle->getY() < 0) left_paddle->setY(0);                                                         // adds boundries for left and right paddles
            if(left_paddle->getY() + left_paddle->getH()>HEIGHT) left_paddle->setY(HEIGHT-left_paddle->getH());
            if(right_paddle->getY() < 0) right_paddle->setY(0);
            if(right_paddle->getY() + right_paddle->getH()>HEIGHT) right_paddle->setY(HEIGHT-right_paddle->getH());

            //checks to see if ball has reacted the left or right side to score point
            if(ball->getX() <= 0) {
                // turn = 0; // change turn
                score_right++;

                // create new Text object for new score and delete old object
                gameRend.remove(score_r);
                score_r = new Text(to_string(score_right).c_str(), 100, make_pair(920,0));
                score_r->create(renderer);
                gameRend.add(score_r);

                serve(turn);
            }
            if(ball->getX() -16 >= WIDTH) {
                // turn = 1; // change turn
                score_left++;

                // create new Text object for new score and delete old object
                gameRend.remove(score_l);
                score_l = new Text(to_string(score_left).c_str(), 100, make_pair(280,0));
                score_l->create(renderer);
                gameRend.add(score_l);

                serve(turn);
            }

            return;
        }

        void input(bool &running) {
            SDL_Event e;
            const Uint8 *keystates = SDL_GetKeyboardState(NULL);
            while (SDL_PollEvent(&e)) { //allows for key inputs
                if(e.type==SDL_QUIT) running = false;
                if(keystates[SDL_SCANCODE_ESCAPE]) running = false;
            }

            return;
        }

        void SetColor(int ForgC) {
            WORD wColor;

            HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO csbi;

            if(GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
                wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
                SetConsoleTextAttribute(hStdOut, wColor);
            }
            return;
        }
};

#endif
