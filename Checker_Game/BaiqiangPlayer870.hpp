//
//  FakePlayer.hpp
//  PA6
//
//  Created by rick gessner on 2/20/22.
//

#ifndef FakePlayer_hpp
#define FakePlayer_hpp

#include <stdio.h>
#include "Game.hpp"
#include <vector>
#include <map>


namespace ECE141 {
  
class BaiqiangPlayer870 : public PlayerInterface {
public:
  BaiqiangPlayer870();
  
  bool takeTurn(Game &aGame,
                        PieceColor asColor,
                        std::ostream &aLog) override;
};

class SimBoard : public BaiqiangPlayer870{
public:
    std::vector<PieceOpt> thePieces;
//    std::pair<Piece,std::pair<Location,PieceOpt>> theCurrentMove;
    Piece CurrentPiece;
    Location CurrentDirection;
    PieceOpt CurrentCaptured;
    
    SimBoard(Game &aGame){
        for(int i=0;i<64;i++){
            int row = i/8;
            int col = i%8;
            Location theLocation(row,col);
            if(aGame.getTileAt(theLocation)->getPiece())
                thePieces.push_back(aGame.getTileAt(theLocation)->getPiece().value());
        }
    }
    void update_board(){
        
        for (auto &piece : thePieces){
            if (piece.value() == CurrentPiece){
                piece->setLocation(CurrentDirection);
                CurrentPiece.setLocation(CurrentDirection);
                if (0 == CurrentDirection.row && PieceColor::blue == CurrentPiece.getColor()){
                    piece.value().setKind(PieceKind::king);
                    CurrentPiece.setKind(PieceKind::king);
                }
                if (7 == CurrentDirection.row && PieceColor::gold == CurrentPiece.getColor()){
                    piece.value().setKind(PieceKind::king);
                    CurrentPiece.setKind(PieceKind::king);
                }
                if (CurrentCaptured){  // if the move is a jump
                    int index = 0;
                    for(auto& piece : thePieces){
                        if(CurrentCaptured.value() == piece.value()) {
//                            piece.value().setKind(PieceKind::captured);  // set the captured piece as captured
                            thePieces.erase(index + thePieces.begin());
                            CurrentCaptured.value().setKind(PieceKind::captured);
                            break;
                        }
                        index++;
                    }
                }
                return;
            }
        }
        return;
    }
    
    void show_board(){
        
        std::vector<char> line(8,'.');
        std::vector<std::vector<char>> theBoard(8,line);
        
        for(auto piece : thePieces){
            int row = piece.value().getLocation().row;
            int col = piece.value().getLocation().col;
            char aColor;
            if(PieceKind::captured == piece.value().getKind()) aColor = 'C';
            else aColor = PieceColor::blue == piece.value().getColor() ? 'b' : 'g';
            if(PieceKind::king == piece.value().getKind()) aColor = toupper(aColor);
            
            theBoard[row][col] = aColor;
        }
        int index = 0;
        for(auto line : theBoard){
            std::cout << index << " ";
            for(auto tile : line){
                std::cout << tile << " " ;
            }
            std::cout << std::endl;
            index++;
        }
        std::cout << "  0 1 2 3 4 5 6 7" << std::endl;
        
    }
    
    
};




}

#endif /* FakePlayer_hpp */
