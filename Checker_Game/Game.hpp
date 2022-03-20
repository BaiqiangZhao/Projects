//
//  Game.hpp
//  Checkers
//
//  Created by rick gessner on 2/22/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Game_hpp
#define Game_hpp

#include <iostream>
#include "stdio.h"
#include "Piece.hpp"


namespace ECE141 {
  
  enum class Reasons {tbd, forfeit, badmove, eliminated,
                      missedJump, moved2, clockExpired};

  class Game;

  struct PlayerInterface {
    virtual bool takeTurn(Game &aGame, PieceColor asColor,
                          std::ostream &aLog)=0;
  };

  class Game {
  public:
           
    virtual size_t getBoardWidth() const {return kBoardDim;}
    virtual size_t getBoardHeight() const  {return kBoardDim;}
    virtual size_t getPiecesPerPlayer() const {return (kBoardDim/2)*3;}

    static  Reasons   run(PlayerInterface *aP1,
                          PlayerInterface *aP2, std::ostream &anOutput);
    
      //These methods are used by player to interact with game...

    virtual size_t    countAvailablePieces(PieceColor aColor) const=0;
    virtual PieceOpt  getAvailablePiece(PieceColor aColor,size_t anIndex) const =0;
    virtual TileOpt   getTileAt(const Location &aSpot) const=0;
    virtual bool      movePieceTo(const Piece &aPiece,const Location &aSpot)=0;
        
  protected:
    Piece*            getPiece(const Tile &aTile) const;
    void              setPiece(Tile &aTile,  Piece* aPiece);
    
    static const size_t kBoardDim = 8; //8x8!
  };


}

#endif /* Game_hpp */
