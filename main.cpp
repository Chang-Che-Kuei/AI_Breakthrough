#include"Breakthrough.h"
#include <SFML/Graphics.hpp>
#include <time.h>
#include<stdio.h>
#include<windows.h>
#include<iostream>
#include<stdlib.h>
#include<time.h>
#define win 0
#define WHITE_TURN 0
#define BLACK_TURN 1
#define FACTOR 7    //evaluated by experiment
using namespace std;
//吃的分數不應該是1000加上位子分數  要加上被吃得旗子的威脅程度

int size = 56;
sf::Vector2f offset(28,28);

sf::Sprite f[32]; //figures
std::string position="";

int board[8][8] =//decide what kind of chess(6 implies to pawns)
{
    -6,-6,-6,-6,-6,-6,-6,-6,     // (-y)        //black
    -6,-6,-6,-6,-6,-6,-6,-6,     //  ^
    0, 0, 0, 0, 0, 0, 0, 0,     //  |
    0, 0, 0, 0, 0, 0, 0, 0,     //  |
    0, 0, 0, 0, 0, 0, 0, 0,     //  |- - >x
    0, 0, 0, 0, 0, 0, 0, 0,     //  |
    6, 6, 6, 6, 6, 6, 6, 6,     //  |
    6, 6, 6, 6, 6, 6, 6, 6,
};   // (+y)        //white

std::string toChessNote(sf::Vector2f p)
{
    std::string s = "";
    s += char(p.x/size+97);
    s += char(7-p.y/size+49);
    return s;
}

sf::Vector2f toCoord(char a,char b)
{
    int x = int(a) - 97;//convert English char to int
    int y = 7-int(b)+49;//convert number  char to int
    return sf::Vector2f(x*size,y*size);
}

void move(std::string str)
{
    sf::Vector2f oldPos = toCoord(str[0],str[1]);
    sf::Vector2f newPos = toCoord(str[2],str[3]);

    //find the eaten figure and eater
    for(int i=0; i<32; i++)
        if (f[i].getPosition()==newPos) f[i].setPosition(896+56,0);//be eaten
    for(int i=0; i<32; i++)
        if (f[i].getPosition()==oldPos) f[i].setPosition(newPos);//eater go to new position
}

void loadPosition()//init
{
    int k=0;
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
        {
            int n = board[i][j];
            if (!n) continue;
            int x = abs(n)-1;
            int y = n>0?1:0;
            f[k].setTextureRect( sf::IntRect(size*x,size*y,size,size) );//the rectangle of printing area from the source
            f[k].setPosition(size*j,size*i);//choose where to print to the screen
            k++;
        }
    for(int i=0; i<position.length(); i+=5) //the history movement
        move(position.substr(i,4));

}

struct Undo
{
    bit64 w,b;
    bool isAITurn;
};
Undo undo[200];//record at most 200 times
int times=0;
int main()
{
    printf("Select which color belongs to AI(white goes first).\n");
    printf("(1)I am white\n");
    printf("(2)I am Black\n");
    printf("Choice...");
    int choice;
    while(cin>>choice&&choice!=1&&choice!=2)printf("Choice...");

    printf("Please input how many seconds can the AI thinking per 'turn' ?(sec)");
    int TimeLimitPerTurn;
    cin>>TimeLimitPerTurn;

    bool playerTurn,AIColor;
    if(choice==1)playerTurn=true,AIColor=WHITE_TURN;
    else playerTurn=false,AIColor=BLACK_TURN;

    BitBoard board;
    board.InitHash();

    sf::RenderWindow window(sf::VideoMode(504, 504), "The Chess! (press SPACE)");
    sf::Texture t1,t2;
    t1.loadFromFile("images/figures.png");
    t2.loadFromFile("images/board.png");

    for(int i=0; i<32; i++) f[i].setTexture(t1);
    sf::Sprite sBoard(t2);

    loadPosition();

    bool isMove=false;
    float dx=0, dy=0;
    sf::Vector2f oldPos,newPos;
    std::string str;
    int n=0;
    while (window.isOpen())
    {
        sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);

        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::KeyPressed)
                if (e.key.code == sf::Keyboard::BackSpace&&times>1)
                {
                    if (position.length()>6) position.erase(position.length()-6,5);

                    playerTurn=!playerTurn;
                    board.UpdateBlackWhite(undo[times-2].w,undo[times-2].b);
                    --times;
                    loadPosition();
                }
            /////User drag and drop///////
            if(playerTurn==false)
            {
                if (e.type == sf::Event::MouseButtonPressed)
                    if (e.key.code == sf::Mouse::Left)
                        for(int i=0; i<32; i++) //find which chess is pressed
                            if (f[i].getGlobalBounds().contains(pos.x,pos.y))
                            {
                                isMove=true;
                                n=i;
                                dx=pos.x - f[i].getPosition().x;
                                dy=pos.y - f[i].getPosition().y;
                                oldPos  =  f[i].getPosition();
                            }

                if (isMove&&e.type == sf::Event::MouseButtonReleased)
                    if (e.key.code == sf::Mouse::Left)
                    {
                        isMove=false;
                        sf::Vector2f p = f[n].getPosition() + sf::Vector2f(size/2,size/2);//put figure into the board block
                        newPos = sf::Vector2f( size*int(p.x/size), size*int(p.y/size) );
                        str = toChessNote(oldPos)+toChessNote(newPos);
                        move(str);
                        if (oldPos!=newPos) {
                            position+=str+" ";
                            playerTurn=true;
                        }
                        f[n].setPosition(newPos);//put the chess into the block

                        bit64 origin   =toBit64(str[0],str[1]);
                        bit64 nextMove =toBit64(str[2],str[3]);
                        //printf("%llX %llX\n",origin,nextMove);
                        if (oldPos!=newPos)
                        {
                            if   (AIColor==BLACK_TURN)board.WhitePlayerMove(origin,nextMove);//i am black, so the opponent is the white
                            else                       board.BlackPlayerMove(origin,nextMove);
                            undo[times].w=board.GetWhite();
                            undo[times].b=board.GetBlack();
                            ++times;
                            printf("PLayer Move=%s\n",str.c_str());
                        }
                    }
            }
        }

        //AI
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)&&playerTurn==true)
        {
            if(AIColor==BLACK_TURN){//IDS alpha-beta pruning
                float timeAlreadyUsed=0;
                for(int depth=1;;depth++){
                    float depthStart = clock();
                    board.InitHash();
                    board.AlphaBetaBlack(depth,depth,-9999999,9999999);
                    board.Release();
                    float depthEnd = clock();
                    float depthCost=(depthEnd-depthStart)/1000.0;
                    printf("Depth %d costs %1.6f\n",
                           depth,depthCost);
                    timeAlreadyUsed+=depthCost;
                    //this dpeth cost too much time. Don't run the next depth.
                    if( (TimeLimitPerTurn-timeAlreadyUsed)<depthCost*FACTOR )
                        break;
                }
            }
            else{
                float timeAlreadyUsed=0;
                for(int depth=1;;depth++){
                    float depthStart = clock();
                    board.InitHash();
                    board.AlphaBetaWhite(depth,depth,-99999999,99999999);
                    board.Release();
                    float depthEnd = clock();
                    float depthCost=(depthEnd-depthStart)/1000.0;
                    printf("Depth %d costs %1.6f\n",
                           depth,depthCost);
                    timeAlreadyUsed+=depthCost;
                    //this dpeth cost too much time. Don't run the next depth.
                    if( (TimeLimitPerTurn-timeAlreadyUsed)<depthCost*FACTOR )
                        break;
                }
            }
            board.UpdateBoard();
            board.ShowBestMove();
            undo[times].w=board.GetWhite();
            undo[times].b=board.GetBlack();
            ++times;

            playerTurn=false;//next turn is human player
            board.CopyBestMove(str);
            oldPos = toCoord(str[0],str[1]);
            newPos = toCoord(str[2],str[3]);

            for(int i=0; i<32; i++) //find the move of source
                if (f[i].getPosition()==oldPos) n=i;
            //board.ShowWhiteBlack();
            /////animation///////
            int timeForMove=150;//ms
            for(int k=0; k<timeForMove; k++)
            {
                sf::Vector2f p = newPos - oldPos;
                f[n].move(p.x/timeForMove, p.y/timeForMove);
                window.draw(sBoard);
                for(int i=0; i<32; i++) f[i].move(offset);
                for(int i=0; i<32; i++) window.draw(f[i]);
                window.draw(f[n]);
                for(int i=0; i<32; i++) f[i].move(-offset);
                window.display();
            }

            move(str);        //update the chess position of UI
            position+=str+" ";//for future version use
            f[n].setPosition(newPos);
        }

        if (isMove) f[n].setPosition(pos.x-dx,pos.y-dy);//move by user

        ////// draw  ///////
        window.clear();
        window.draw(sBoard);
        for(int i=0; i<32; i++) f[i].move(offset);
        for(int i=0; i<32; i++) window.draw(f[i]);
        window.draw(f[n]);
        for(int i=0; i<32; i++) f[i].move(-offset);
        window.display();

        if(board.Finish())
        {
            printf("\n/////////////\n");
            printf("/Game Over!!/\n");
            printf("/////////////\n");
            printf("Exit after 5 seconds\n");
            Sleep(5000);//stay 5 seconds before exit
            break;
        }
    }
}
