//
//  FakePlayer.cpp
//  PA6
//
//  Created by rick gessner on 2/20/22.
//

#include "BaiqiangPlayer870.hpp"
#include <stdlib.h>
#include <time.h>
#include <iomanip>

namespace ECE141 {



//  ------------------------------------------
  
  BaiqiangPlayer870::BaiqiangPlayer870() : PlayerInterface() {}
  


std::map<Piece, std::vector<std::pair<Location,PieceOpt>>> get_all_available_moves(PieceColor asColor,SimBoard aBoard);
void jump_and_show(Game &aGame,SimBoard &aBoard);

bool BaiqiangPlayer870::takeTurn(Game &aGame, PieceColor asColor, std::ostream &aLog) {
   
    SimBoard theBaord(aGame);
    
    std::map<Piece, std::vector<std::pair<Location,PieceOpt>>> all_available_move;
    all_available_move = get_all_available_moves(asColor, theBaord);
    
    
//    std::string theColor = PieceColor::blue == asColor ? "Blue" : "Gold";
//    std::cout << "Player is : " << theColor << std::endl;
//    std::cout << "regular move  " << "all neighbors: "<< std::endl ;
//    for(auto neighbor : all_available_move){
//        std::cout << std::endl << neighbor.first.getLocation().row << " " << neighbor.first.getLocation().col << "\n";
//        for(auto direction : neighbor.second){
//            std::string jump = "";
//            if(direction.second) jump = " Jump";
//             std::cout << direction.first.row << " " << direction.first.col << jump << "\n";
//        }
//    }
//    std::cout << std::endl;
//
    
    size_t number_available_move = 0;
    for (auto aMove : all_available_move){
        for(auto direction : aMove.second){
            number_available_move++;
            if(direction.second){               // if a jump is available, go jump
                theBaord.CurrentPiece = aMove.first;
                theBaord.CurrentDirection = direction.first;
                theBaord.CurrentCaptured = direction.second;
                jump_and_show(aGame,theBaord);
                return true;
            }
        }
    }
    
    std::srand((unsigned)time(NULL));
    size_t move_index = 0;
    int random_move = std::rand() % number_available_move;
    
    number_available_move = 0;
    for (auto aMove : all_available_move){
        for(auto direction : aMove.second){
            if(move_index == random_move) {
                theBaord.CurrentPiece = aMove.first;
                theBaord.CurrentDirection = direction.first;
                theBaord.CurrentCaptured = direction.second;
                jump_and_show(aGame,theBaord);
                return true;
            }
            move_index++;
        }
    }
    return false;

    
}

size_t countAvailablePieces(PieceColor aColor,SimBoard aBoard){
    size_t count = 0;
    for(auto piece : aBoard.thePieces){
        if( aColor == piece.value().getColor() && PieceKind::captured != piece.value().getKind()){
            count++;
        }
    }
    return count;
    
}

PieceOpt  getAvailablePiece(PieceColor aColor,size_t anIndex,SimBoard aBoard){
    size_t index =0;
    for(auto piece : aBoard.thePieces){
        if(index == anIndex && aColor == piece.value().getColor() && PieceKind::captured != piece.value().getKind()){
            return piece.value();
        }
        if(aColor == piece.value().getColor())
            index++;
    }
    return std::nullopt;
}

bool move_in_range(Location aMoveDirection){
    return (aMoveDirection.row < 8 && aMoveDirection.row > -1 && aMoveDirection.col < 8 && aMoveDirection.col > -1);
}

std::vector<std::pair<Location,PieceOpt>> find_neighbor_one_direction(SimBoard aBoard,Location move_direction, Location jump_direction,PieceColor aColor){
    std::vector<std::pair<Location,PieceOpt>> neighbors;
    bool piece_in_move_direction = false;
    bool piece_in_jump_direction = false;
    std::pair<Location,PieceOpt> aMove;
    PieceColor opponent_color = PieceColor::blue == aColor ? PieceColor::gold : PieceColor::blue;
    if(move_in_range(move_direction)){
        for(auto piece : aBoard.thePieces){
            if(move_direction == piece.value().getLocation() && PieceKind::captured != piece.value().getKind()){   // if there is piece in location of move_direction,
                
//                std::string aKind = PieceKind::pawn == piece.value().getKind() ? "Pawn" : "King";
//                std::cout << aKind << "Located on " << piece.value().getLocation().row << " " << piece.value().getLocation().col << "\n";
//
                
                
                if(move_in_range(jump_direction)){
                    if(opponent_color == piece.value().getColor()){  // if the piece detected is opponent
                        for(auto piece : aBoard.thePieces){                // check whether there is piece in location of jump_direction
                            if(jump_direction == piece.value().getLocation() &&  PieceKind::captured != piece.value().getKind()){ // if there is piece in location of jump_direction
//                                std::cout << "Piece in jump direction" << std::endl;
                                piece_in_jump_direction = true;
                                break;
                            }
                        }
                        if(!piece_in_jump_direction){
                            aMove.first = jump_direction;
                            aMove.second = piece;               // true means this move is a JUMP
                            neighbors.push_back(aMove);
//                            std::cout << "show me there is a jump to : " << jump_direction.row << " " << jump_direction.col
//                            <<   "\n";
                        }
                    }
                }
                piece_in_move_direction = true;
                break;
            }
        }
        if (!piece_in_move_direction){
            aMove.first = move_direction;
            aMove.second = std::nullopt;
            neighbors.push_back(aMove);
        }
    }
    
    return neighbors;
}


std::vector<std::pair<Location,PieceOpt>> FindNeighbour(SimBoard aBoard,Piece aPiece){
    std::map<Piece, std::vector<std::pair<Location,PieceOpt>>> NeighborMap;
    Location PieceLocation = aPiece.getLocation();
    PieceKind theKind = aPiece.getKind();
    PieceColor theColor = aPiece.getColor();
    std::vector<std::pair<Location,PieceOpt>> neighbor_vector;
    
    Location top_left = Location(PieceLocation.row-1,PieceLocation.col-1);
    Location top_right = Location(PieceLocation.row-1, PieceLocation.col+1);
    Location bot_left = Location(PieceLocation.row+1, PieceLocation.col-1);
    Location bot_right = Location(PieceLocation.row+1,PieceLocation.col+1);
    
    Location jump_top_left  = Location(PieceLocation.row-2,PieceLocation.col-2);
    Location jump_top_right = Location(PieceLocation.row-2,PieceLocation.col+2);
    Location jump_bot_left = Location(PieceLocation.row+2, PieceLocation.col-2);
    Location jump_bot_right = Location(PieceLocation.row+2,PieceLocation.col+2);
    
    
    
    if(theKind == PieceKind::king){
        
        std::vector<std::pair<Location,PieceOpt>> vector1 = find_neighbor_one_direction(aBoard,top_left,jump_top_left,theColor);
        std::vector<std::pair<Location,PieceOpt>> vector2 = find_neighbor_one_direction(aBoard,top_right,jump_top_right,theColor);
        std::vector<std::pair<Location,PieceOpt>> vector3 = find_neighbor_one_direction(aBoard,bot_left,jump_bot_left,theColor);
        std::vector<std::pair<Location,PieceOpt>> vector4 = find_neighbor_one_direction(aBoard,bot_right,jump_bot_right,theColor);
        neighbor_vector.insert(neighbor_vector.end(), vector1.begin(),vector1.end());
        neighbor_vector.insert(neighbor_vector.end(), vector2.begin(),vector2.end());
        neighbor_vector.insert(neighbor_vector.end(), vector3.begin(),vector3.end());
        neighbor_vector.insert(neighbor_vector.end(), vector4.begin(),vector4.end());
    }
    
    else if(theColor == PieceColor::gold){
        
        std::vector<std::pair<Location,PieceOpt>> vector1 = find_neighbor_one_direction(aBoard,bot_left,jump_bot_left,theColor);
        std::vector<std::pair<Location,PieceOpt>> vector2 = find_neighbor_one_direction(aBoard,bot_right,jump_bot_right,theColor);
        neighbor_vector.insert(neighbor_vector.end(), vector1.begin(),vector1.end());
        neighbor_vector.insert(neighbor_vector.end(), vector2.begin(),vector2.end());
        
           
    }
    else if(theColor == PieceColor::blue){
        
        std::vector<std::pair<Location,PieceOpt>> vector1 = find_neighbor_one_direction(aBoard,top_left,jump_top_left,theColor);
        std::vector<std::pair<Location,PieceOpt>> vector2 = find_neighbor_one_direction(aBoard,top_right,jump_top_right,theColor);
        neighbor_vector.insert(neighbor_vector.end(), vector1.begin(),vector1.end());
        neighbor_vector.insert(neighbor_vector.end(), vector2.begin(),vector2.end());
        
    }
    
    return neighbor_vector;
}


std::map<Piece,std::vector<std::pair<Location,PieceOpt>>> get_all_available_moves(PieceColor asColor,SimBoard aBoard){
    std::map<Piece, std::vector<std::pair<Location,PieceOpt>>> all_available_move;
    size_t theCount=countAvailablePieces(asColor,aBoard);
    for(size_t i=0;i<theCount;i++) {
        auto thePiece=getAvailablePiece(asColor,i,aBoard);
        std::vector<std::pair<Location,PieceOpt>> theNeighbor = FindNeighbour(aBoard,thePiece.value());
        if(theNeighbor.size() > 0)
          all_available_move[thePiece.value()] = theNeighbor;
    } //for
    return all_available_move;
}

void keep_jump(SimBoard &aBoard,Game &aGame){
    
    std::vector<std::pair<Location,PieceOpt>> Neighbors = FindNeighbour(aBoard,aBoard.CurrentPiece);
//    std::cout << "Neighbors is : ";
//    for(auto neighbor : Neighbors){
//        std::cout << neighbor.first.row << " " << neighbor.first.col << "   ";
//    }
//    std::cout << "Piece location : ";
//    std::cout << aBoard.CurrentPiece.getLocation().row << " " << aBoard.CurrentPiece.getLocation().col << std::endl;
    
    for(auto neighbor : Neighbors){
        if(neighbor.second){  // if jump is availble
            aBoard.CurrentDirection = neighbor.first;
            aBoard.CurrentCaptured = neighbor.second;
            jump_and_show(aGame,aBoard);
            return;
        }
    }
    return;
}


void jump_and_show(Game &aGame,SimBoard &aBoard){
    
    aGame.movePieceTo(aBoard.CurrentPiece,aBoard.CurrentDirection);
    
//    std::cout << "the Piece was in: " << aBoard.CurrentPiece.getLocation().row << " " << aBoard.CurrentPiece.getLocation().col << std::endl;
    aBoard.update_board();
//    std::cout << "the Piece move to: " << aBoard.CurrentDirection.row << " " << aBoard.CurrentDirection.col << std::endl;
//    aBoard.show_board();
    
    if(aBoard.CurrentCaptured){    // if the move is a jump, it can keep jump
        keep_jump(aBoard,aGame);
    }
}

}
