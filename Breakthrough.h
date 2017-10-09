#ifndef BREAKTHROUGH_H_INCLUDED
#define BREAKTHROUGH_H_INCLUDED

#include<vector>
#include<string>
#include<stdio.h>
#include<algorithm>
typedef unsigned long long int bit64;
#define FullBitboard 0xFFFFFFFFFFFFFFFF
#define Win  9e6

#define Rank1 0xFF00000000000000
#define Rank8 0x00000000000000FF
using namespace std;
/*
bitboard index
 7, 6, 5, 4, 3, 2, 1, 0,
15,14,13,12,11,10, 9, 8,
23,22,21,20,19,18,17,16,
31,30,29,28,27,26,25,24,
39,38,37,36,35,34,33,32,
47,46,45,44,43,42,41,40,
55,54,53,52,51,50,49,48,
63,62,61,60,59,58,57,56,

Init state//white=0,black=1,empty=-1
1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,
-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
*/

struct Record
    {
        bit64 NextWhite,NextBlack;
        string bestMove="    ";
    };
struct Transposition
    {
        bit64 white=0x0,black=0x0;
        int value=-Win;
    };
class BitBoard
{
private:
    Record record;
    bit64 white=0xFFFF000000000000;//white chess
    bit64 black=0x000000000000FFFF;//black chess
    bit64 empty=FullBitboard^(white|black);//use logical operation for getting empty bitboard

    bit64 Zobrist[64][2];//64 positions and 2 kind of chess(0:white pawn, 1:black pawn)
    bit64 zHash=0x0;
    Transposition* trans;
    bool finish=false;
    int hashIndex[64]={
        63, 0,58, 1,59,47,53, 2,
        60,39,48,27,54,33,42, 3,
        61,51,37,40,49,18,28,20,
        55,30,34,11,43,14,22, 4,
        62,57,46,52,38,26,32,41,
        50,36,17,19,29,10,13,21,
        56,45,25,31,35,16, 9,12,
        44,24,15, 8,23, 7, 6, 5};
    int evaluWhite[64]={
        999999,999999,999999,999999,999999,999999,999999,999999,
        23,24,22,21,21,22,24,23,
        19,20,18,17,17,18,20,19,
        15,16,14,13,13,14,16,15,
        11,12,10,9,9,10,12,11,
        7,8,6,5,5,6,8,7,
        3,4,2,1,1,2,4,3,
        0,0,0,0,0,0,0,0,
    };
    int evaluBlack[64]={
        0,0,0,0,0,0,0,0,
        3,4,2,1,1,2,4,3,
        7,8,6,5,5,6,8,7,
        11,12,10,9,9,10,12,11,
        15,16,14,13,13,14,16,15,
        19,20,18,17,17,18,20,19,
        23,24,22,21,21,22,24,23,
        9999999,999999,999999,999999,999999,999999,999999,999999
    };

public:
    void InitZobrist();
    void InitHash();
    void Release(){free(trans);}
    bool Finish(){return (black&Rank1||white&Rank8);}
    bit64 GetWhite(){return white;}
    bit64 GetBlack(){return black;}
    void UpdateBlackWhite(const bit64 w,const bit64 b){white=w,black=b;}
    void ShowWhiteBlack(){printf("white=%llX black=%llX\n",white,black);}
    void ShowBestMove(){printf("AI     Move=%s\n",record.bestMove.c_str());}
    void UpdateEmpty(){
        empty=FullBitboard^(white&black);
    }

    void generateWhiteMove(bit64 Move[])const;
    void generateBlackMove(bit64 Move[])const;
    bit64 LS1B(bit64 &target)const{//take out the least bit
        bit64 ls1b=target&-target;
        target^=ls1b;//reset
        return ls1b;
    }
    int CountChess(const bit64 &target);//for debug
    int PerfectHashing(const bit64 singleBit)const{//find the bit's position
        return (singleBit*0x07EDD5E59A4E28C2)>>58;
    }
    void MoveChess(bit64 &target,const int index_orig,const bit64 nextMove);
    void CopyBestMove(string &str){str=record.bestMove;}
    void RecordMove(bit64 origin,bit64 nextMove);
    int AlphaBetaWhite(const int depth,const int depthLimit,int alpha,int beta);
    int AlphaBetaBlack(const int depth,const int depthLimit,int alpha,int beta);
    void UpdateBoard(){white=record.NextWhite,black=record.NextBlack,finish=false;}

    //User Move
    void WhitePlayerMove(const bit64 origin,const bit64 nextMove);
    void BlackPlayerMove(const bit64 origin,const bit64 nextMove);

};

bit64 toBit64(char a,char b);

#endif // BREAKTHROUGH_H_INCLUDED
