#include"Breakthrough.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define Loss -999999

#define White 0
#define Black 1

#define KindofMove 3
#define LeftUP     0
#define UP         1
#define RightUP    2
int whiteShift[3]= {7,8,9}; //correspond to the shift of LeftUP, UP and RightUP
#define LeftDown  0
#define Down      1
#define RightDown 2
int blackShift[3]= {9,8,7};

#define Rank1 0xFF00000000000000
#define Rank8 0x00000000000000FF
#define FileA 0x8080808080808080
#define FileH 0x0101010101010101
bit64 ONE=0x1;
void BitBoard::InitZobrist()
{
    srand(time(0));
    for(int i=0; i<64; ++i) //init zobrist
    {
        Zobrist[i][0]=((bit64)rand() << 15) ^ ((bit64)rand() << 30) ^ ((bit64)rand() << 45) ^ ((bit64)rand() << 60);
        Zobrist[i][1]=((bit64)rand() << 15) ^ ((bit64)rand() << 30) ^ ((bit64)rand() << 45) ^ ((bit64)rand() << 60);
        //printf("%llX %llX\n",Zobrist[i][0],Zobrist[i][1]);
    }
}
void BitBoard::InitHash()
{
    trans = (Transposition*) malloc (16777216*sizeof(Transposition));//2^24 entries*20bytes/per entry= about 1GB
    if(trans==NULL){printf("no enough memory\n");return;}
    //init zHash
    bit64 tempBlack=black;
    bit64 tempWhite=white;
    bit64 temp;
    while(tempWhite)
    {
            temp=LS1B(tempWhite);
            int index=hashIndex[PerfectHashing(temp)];
            zHash^=Zobrist[index][0];
    }
    while(tempBlack)
    {
            temp=LS1B(tempBlack);
            int index=hashIndex[PerfectHashing(temp)];
            zHash^=Zobrist[index][1];
    }
}
int BitBoard::CountChess(const bit64 &target)
{
    int temp=0;
    bit64 t=target;
    while(t)
    {
        LS1B(t);
        temp++;
    }
    return temp;
}
void BitBoard::generateWhiteMove(bit64 Move[])const
{
    Move[LeftUP] =(white&~FileA)>>7&(FullBitboard&~white);//move to up-left
    Move[UP]     = white>>8&(FullBitboard&~black&~white);//up
    Move[RightUP]=(white&~FileH)>>9&(FullBitboard&~white);//right-up
}
void BitBoard::generateBlackMove(bit64 Move[])const
{
    Move[LeftDown] =(black&~FileA)<<9&(FullBitboard&~black);//move to down-left
    Move[Down]     = black<<8&(FullBitboard&~black&~white);//up
    Move[RightDown]=(black&~FileH)<<7&(FullBitboard&~black);//right-down
}
void BitBoard::MoveChess(bit64 &target,const int index_orig,const bit64 nextMove)
{
    target&=~(ONE << index_orig);//clear the original bit
    target|=nextMove;//then, add the new move bit
}
void BitBoard::RecordMove(bit64 origin,bit64 nextMove)
{
    //transfer the chess position to the output format according to (0,0) on the left-up corner
    int x=7,y=7;//x stands for 1~8; ystands for a~h
    while(origin>=0x100)origin>>=8,--x;
    while(origin>1)origin>>=1,--y;
    record.bestMove[0]=('a'+y),record.bestMove[1]=('1'+x);

    x=7,y=7;
    while(nextMove>=0x100)nextMove>>=8,--x;
    while(nextMove>1)nextMove>>=1,--y;
    record.bestMove[2]=('a'+y),record.bestMove[3]=('1'+x);

    record.NextWhite=white,record.NextBlack=black;
}
int BitBoard::AlphaBetaWhite(const int depth,const int depthLimit,int alpha,int beta)
{
    if(black&Rank1)return Loss-(depth<<14);   //loss more late is better
    if(white&Rank8)return -Loss+(depth<<14);   //win earlier is better
    if(depth<=0)return 0;

    int value;
    bit64 saveW=white,saveB=black,saveE=empty,saveHash=zHash;
    bit64 Move[KindofMove];
    generateWhiteMove(Move);
    for(int i=0; i<KindofMove; ++i)
    {
        while(Move[i])
        {
            bit64 nextMove=LS1B(Move[i]);//extract the least bit and remove from Move[i]
            bit64 origin=nextMove<<whiteShift[i];//original position before move
            int index_next=hashIndex[PerfectHashing(nextMove)];
            int index_orig=hashIndex[PerfectHashing(origin)];
            MoveChess(white,index_orig,nextMove);//update the bitboard according to nextMove
            //position score
            int moveScore=evaluWhite[ index_next ];

            if(nextMove&black )  //there is a black chess can be eaten
            {
                //white pawn move to black pawn and eat, look up transposition table


                //the potential danger of this black chess
                int danger=max(evaluBlack[index_next+7],evaluWhite[index_next+8]);
                danger=max(danger,evaluBlack[index_next+9]);
                //value=eat point +depth*5+moveScore+potential danger
                value=        1000+depth*5+moveScore+danger;
                black&=~(ONE << index_next);//take out the eaten black chess
                UpdateEmpty();

                if(depth==1)value-=AlphaBetaBlack(1,depthLimit,-beta+value,-alpha+value);//Quiescence
                else        value-=AlphaBetaBlack(depth-1,depthLimit,-beta+value,-alpha+value);
            }
            else  //no blocking
            {
                //white pawn move to blank place, look up transposition table

                UpdateEmpty();
                value=moveScore-AlphaBetaBlack(depth-1,depthLimit,-beta+moveScore,-alpha+moveScore);
            }

            if(value>alpha)
            {
                alpha=value;
                if(depth==depthLimit)RecordMove(origin,nextMove);//record move
            }
            //unmake move
            white=saveW;
            black=saveB;
            empty=saveE;
            zHash=saveHash;

            if(value>=beta)return value;
        }

    }

    return alpha;
}
int BitBoard::AlphaBetaBlack(const int depth,const int depthLimit,int alpha,int beta)
{
    if(white&Rank8)return Loss-(depth<<14);   //loss more late is better
    if(black&Rank1)return -Loss+(depth<<14);   //win earlier is better
    if(depth<=0)return 0;

    int value;
    bit64 saveW=white,saveB=black,saveE=empty,saveHash=zHash;
    bit64 Move[KindofMove];
    generateBlackMove(Move);
    for(int i=0; i<KindofMove; ++i)
    {
        while(Move[i])
        {
            bit64 nextMove=LS1B(Move[i]);//extract the least bit and remove from Move[i]
            bit64 origin=nextMove>>blackShift[i];//original position before move
            int index_next=hashIndex[PerfectHashing(nextMove)];
            int index_orig=hashIndex[PerfectHashing(origin)];
            MoveChess(black,index_orig,nextMove);//update the bitboard according to nextMove
            //position score
            int moveScore=evaluBlack[ index_next ];
            if(nextMove&white )  //there is a white chess can be eaten
            {
                //black pawn move to white pawn and eat, look up transposition table

                //the potential danger of this white chess
                int danger=max(evaluWhite[index_next-7],evaluWhite[index_next-8]);
                danger=max(danger,evaluWhite[index_next-9]);
                //value=eat point +depth*5+moveScore+potential danger
                value=       1000+ depth+moveScore+danger;

                white&=~(ONE << index_next);//take out the eaten white chess
                UpdateEmpty();
                if(depth==1)value-=AlphaBetaWhite(1,depthLimit,-beta+value,-alpha+value);//Quiescence
                else        value-=AlphaBetaWhite(depth-1,depthLimit,-beta+value,-alpha+value);
            }
            else  //no blocking
            {
                //black pawn move to blank place, look up transposition table

                UpdateEmpty();
                value=moveScore-AlphaBetaWhite(depth-1,depthLimit,-beta+moveScore,-alpha+moveScore);
            }

            if(value>alpha)
            {
                alpha=value;
                if(depth==depthLimit)RecordMove(origin,nextMove);//record move
            }
            //unmake move
            white=saveW;
            black=saveB;
            empty=saveE;
            zHash=saveHash;

            if(value>=beta)return value;
        }

    }

    return alpha;
}
void BitBoard::WhitePlayerMove(const bit64 origin,const bit64 nextMove)
{
    int index_next=hashIndex[PerfectHashing(nextMove)];
    int index_orig=hashIndex[PerfectHashing(origin)];
    MoveChess(white,index_orig,nextMove);
    if(nextMove&black ) //there is a black chess can be eaten
        black&=~(ONE << index_next);//take out the eaten black chess
    UpdateEmpty();
}
void BitBoard::BlackPlayerMove(const bit64 origin,const bit64 nextMove)
{
    int index_next=hashIndex[PerfectHashing(nextMove)];
    int index_orig=hashIndex[PerfectHashing(origin)];
    MoveChess(black,index_orig,nextMove);
    if(nextMove&white ) //there is a white chess can be eaten
        white&=~(ONE << index_next);//take out the eaten white chess
    UpdateEmpty();
}


bit64 toBit64(char a,char b)
{
    bit64 target=0x1;
    int x=7-int(a-'a'),y=7-int(b-'1');
    while(y)target<<=8,--y;
    while(x)target<<=1,--x;
    return target;
}



