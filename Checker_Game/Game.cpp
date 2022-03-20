//
//  Game.cpp
//  Checkers
//
//  Created by rick gessner on 2/22/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include "Game.hpp"
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <math.h>
#include <map>

namespace ECE141 {
  
 // using cspace = Location (*)(const Location &aLocation, int aScale);
  
  Location noop(const Location &aLocation, int scale) {
    return aLocation;
  }
  
  Location heli(const Location &aLocation, int scale) {
    Location temp(aLocation.col, scale-1-aLocation.row);
    return temp;
  }
  
  Location martin(const Location &aLocation, int scale) {
    Location temp(scale-1-aLocation.col, aLocation.row);
    return temp;
  }
  
  Location q5av(const Location &aLocation, int scale) {
    Location temp(scale-1-aLocation.row, scale-1-aLocation.col);
//    std::cout << "q5av(" << temp.col <<"," << temp.row << ")" << std::endl;
    return temp;
  }
 
  //protected friend access to change piece...
  void Game::setPiece(Tile &aTile,  Piece* aPiece) {
    aTile.piece=aPiece;
  }

  Piece* Game::getPiece(const Tile &aTile) const {
    return aTile.piece;
  }

  //----------------------------------------------

  class RealGame : public Game {
      
    static const size_t kMaxSize = Game::kBoardDim;  //EVEN #
    static const size_t kMaxSteps = 500; //max steps before game-over
    
    std::vector<Piece*> goldPieces;
    std::vector<Piece*> bluePieces;
    std::vector<Tile>   tiles[kMaxSize]; //rows...
    std::map<const PlayerInterface*, PieceColor> colors;
    
    PieceColor      loserColor;
    Reasons         reason;
    float           priorDist;
    Piece*          priorPiece;
    const float     stepLen;
    const float     jumpLen;
    PlayerInterface *activePlayer;
    size_t          step;
    size_t          scale;
    size_t          steps;
    size_t          jumps;
    size_t          bcount;
    size_t          gcount;
    
  public:

    RealGame() : Game(), loserColor(PieceColor::none), reason(Reasons::tbd),
      priorDist(0),  priorPiece(nullptr), stepLen(sqrt(2)), jumpLen(sqrt(2)*3), step(0), scale(0), steps(0),jumps(0)  {

      static TileColor theColors[2]={TileColor::light, TileColor::dark};
      int theColor=1;
        
      std::srand((unsigned int)std::time(nullptr)); // use current time as seed for random generator
      size_t theDelta = ((kMaxSize-kBoardDim)/2)+1;
        
      scale=kBoardDim==kMaxSize ? kBoardDim : kBoardDim+((std::rand() % theDelta) *2);
        
      for(int theRow=0;theRow<(int)scale;theRow++) {
        for(int aColumn=0;aColumn<(int)scale;aColumn++) {
          Location theLocation(theRow, aColumn);
          theColor=!theColor;
          tiles[theRow].push_back( ECE141::Tile(theColors[theColor],theLocation));
        }
        theColor=!theColor;
      }
        
    }

    virtual ~RealGame() { //delete pieces from vectors...
      for(auto p : bluePieces) {delete p;}
      for(auto p : goldPieces) {delete p;}
    }
    
    //-- USE: setup the players on the board -----------------
    void initPieces() {
      //now put the pieces onto the tiles...
      int theCol=1;
      int theWidth = (int)getBoardWidth();
      for(int theRow=2;theRow>=0;theRow--) {
        for(int col=theCol;col<theWidth;col+=2) {
          Location theLoc(theRow,col);
          Piece * thePiece = new Piece(theLoc, PieceColor::gold);
          goldPieces.push_back(thePiece);
          setPiece(tiles[theRow][col], thePiece);
        }
        theCol=!theCol;
      }

      for(int theRow=(int)scale-3;theRow<(int)scale;theRow++) {
        for(int col=theCol;col<(int)scale;col+=2) {
          Location theLoc(theRow,col);
          Piece *thePiece=new Piece(theLoc,PieceColor::blue);
          bluePieces.push_back(thePiece);
          setPiece(tiles[theRow][col], thePiece);
        }
        theCol=!theCol;
      }
    }

    //-- USE: retrieve basic game properties...
    virtual size_t getBoardWidth() const override {return size_t(scale);}
    virtual size_t getBoardHeight() const override  {return size_t(scale);}
    virtual size_t getPiecesPerPlayer() const override  {return (scale/2)*3;}

    bool knownPiece(Piece *aPiece) {
      auto theBEnd=bluePieces.end();
      if(std::find(bluePieces.begin(), theBEnd, aPiece) != theBEnd) {
        return true;
      }
      auto theGEnd=goldPieces.end();
      if(std::find(goldPieces.begin(), theGEnd, aPiece) != theGEnd) {
        return true;
      }
      return false;
    }
   
    
    //-- USE: make sure location is legit -----------------
    bool validLocation(const Location &aLocation) const {
      if(aLocation.row>=0 && aLocation.row< (int)getBoardHeight()) {
        return (aLocation.col>=0 && aLocation.col< (int)getBoardWidth());
      }
      return false;
    }

    //-- USE: return # of pieces on board for player -----------------
    size_t countAvailablePieces(PieceColor aColor) const override {
      auto *thePieces=PieceColor::blue==aColor ? &bluePieces : &goldPieces;

      //char theColor = PieceColor::blue==aColor ? 'b' : 'g';
      size_t theCount{0};
      for(auto thePiece: *thePieces) {
        if(PieceKind::captured!=thePiece->getKind()) {
          theCount++;
        }
      }
      return theCount;
    }
        
    //-- USE: retrieve Nth piece for player (for analysis) -----------------
    PieceOpt getAvailablePiece(PieceColor aColor, size_t anIndex) const override {
      auto *thePieces=PieceColor::blue==aColor ? &bluePieces : &goldPieces;
      size_t count=0;
      for(auto thePiece: *thePieces) {
        if(PieceKind::captured!=thePiece->getKind() && anIndex==count++) {
          return *thePiece;
        }
      }
      return std::nullopt;
    }
    
    //-- USE: [internal] retrieve game tile (if valid) -----------------
    const Tile* const tileAt(const Location &aLocation) {
      return validLocation(aLocation)
        ? &tiles[aLocation.row][aLocation.col] : nullptr;
    }
    
    //-- USE: offer RO tile (usually for player analysis) -----------------
    TileOpt getTileAt(const Location &aLocation) const override {
      if(validLocation(aLocation)) {
        /*
        static std::map<Orientation, std::function<Location(const Location &, int)> > fluxcap {
          {Orientation::north, noop}, {Orientation::east,  heli},
          {Orientation::south, q5av}, {Orientation::west,  martin}};
        
        Location temp=fluxcap[orientation](aLocation, orientation);
        */
        return tiles[aLocation.row][aLocation.col];
      }
      return std::nullopt;
    }
        
    //-- USE: remove jumped piece -------------------------
    void removePiece(Piece *aPiece) {
      auto theLoc=aPiece->getLocation();
      aPiece->setKind(PieceKind::captured);
      aPiece->setLocation(Location(-1,-1)); //obviously off board
      if(auto theTile=tileAt(theLoc)) {
        setPiece(*const_cast<Tile*>(theTile), nullptr);
      }
    }

    int moreY(PieceColor aColor) {return PieceColor::blue==aColor ? -1 : 1;}
    int lessY(PieceColor aColor) {return PieceColor::blue==aColor ? 1 : -1;}
    int moreX(PieceColor aColor) {return PieceColor::blue==aColor ? 1 : -1;}
    int lessX(PieceColor aColor) {return PieceColor::blue==aColor ? -1 : 1;}
    
    //-- USE: see if valid jump exists at (deltaX, deltaY) for given piece...
    bool canJumpTo(const Piece &aPiece, int aDeltaX, int aDeltaY) {
      Location thePLoc = aPiece.getLocation();
      Location theStep(thePLoc.row+aDeltaY,thePLoc.col+aDeltaX);
      auto theColor=aPiece.getColor();
      if(const Tile *theTile=tileAt(theStep)) {
        if(auto thePiece=theTile->getPiece()) {
          if(thePiece && theColor!=thePiece->getColor()) {
            Location theJump(theStep.row+aDeltaY,theStep.col+aDeltaX);
            if(auto theJumpTile=tileAt(theJump)) {
              return theJumpTile->isEmpty();
            }
          }
        }
      }
      return false;
    }
    
    //-- USE: call to determine if player has any available jump...
    bool pieceHasJump(const Piece &aPiece, PieceColor aColor) {
      if(canJumpTo(aPiece, lessX(aColor), moreY(aColor))) {
        return true;
      }
      
      if(canJumpTo(aPiece, moreX(aColor), moreY(aColor))) {
        return true;
      }
      
      if (PieceKind::king==aPiece.getKind()) {
        if(canJumpTo(aPiece, lessX(aColor), lessY(aColor))) {
          return true;
        }
        
        if(canJumpTo(aPiece, moreX(aColor), lessY(aColor))) {
          return true;
        }
      }
      return false;
    }
    
    //-- USE: call to determine if player has any available jump...
    bool playerHasJumps(const PlayerInterface *aPlayer) {
      PieceColor theColor = colors[aPlayer];
      size_t theCount=countAvailablePieces(theColor);
      for(size_t i=0;i<theCount;i++) {
        if(auto thePiece=getAvailablePiece(theColor,i)) {
          if(pieceHasJump(*thePiece, theColor)) {
            return true;
          }
        }
      } //for
      return false;
    }

    //-- USE: check move distance between tiles ------------------------
    float distance(const Location &aSrc, const Location &aDest) {
      return sqrt(pow(aDest.col - aSrc.col, 2) + pow(aDest.row - aSrc.row, 2));
    }

    //-- USE: confirm that dest may be occupied -------------------------
    bool isAvailable(const Tile &aTile) {
      auto thePiece = aTile.getPiece();
      return (!thePiece && TileColor::dark==aTile.getTileColor());
    }
        
    //-- USE: confirm piece can move that direction  -------------------
    bool isValidDirection(const Piece &aPiece, const Location &aSrc, const Location &aDest) {
      if(PieceKind::pawn==aPiece.getKind()) {
        return PieceColor::gold==aPiece.getColor()
          ? aSrc.row<aDest.row : aSrc.row>aDest.row;
      }
      return true;
    }
    
    //-- USE: determine whether kingable piece is opponent first row  ----
    bool isKingable(const Piece &aPiece, const Location &aDest) {
      if(PieceKind::pawn==aPiece.getKind()) {
        return PieceColor::blue==aPiece.getColor()
          ? 0==aDest.row : (int)(scale-1)==aDest.row;
      }
      return false;
    }
    
    //-- USE: determine if piece jumped on way to aDest  ----
    Piece* jumpedOpponent(const Piece &aPiece, const Location &aDest) {
      Location thePLoc = aPiece.getLocation();
      int theRow = std::min(thePLoc.row, aDest.row)+1;
      int theCol = std::min(thePLoc.col, aDest.col)+1;
      Location theOppoLoc(theRow, theCol);
      if(const Tile *theTile=tileAt(theOppoLoc)) {
        if(auto theOPiece = getPiece(*theTile)) {
          return aPiece.getColor()==theOPiece->getColor() ? nullptr : theOPiece;
        }
      }
      return nullptr;
    }
    
    //-- USE: ensure given piece can move to given tile... -----------
    bool isValidMove(const Piece &aPiece, const Tile &aSrc, const Tile &aDest) {
      float theDist = distance(aSrc.getLocation(), aDest.getLocation());
      if(theDist<=jumpLen && isAvailable(aDest)) {
        if(isValidDirection(aPiece, aSrc.getLocation(), aDest.getLocation())) {
          if(priorDist==stepLen) return false; //CANT take 2 steps with one piece...
          if(theDist>stepLen) {
            return jumpedOpponent(aPiece, aDest.getLocation())!=nullptr;
          }
          return true; //slide...
        }
      }
      return false;
    }
    
    Piece* getActualPiece(const Piece &aPiece) {
      auto theColor=aPiece.getColor();
      auto *thePieces=PieceColor::blue==theColor ? &bluePieces : &goldPieces;
      auto theLocation=aPiece.getLocation();
      for(auto thePiece : *thePieces) {
        if(theLocation==thePiece->getLocation()) {
          return thePiece;
        }
      }
      return nullptr;
    }
    
    //-- USE: Ensure user doesn't change pieces during multi-moves --------
    Reasons validatePiece(const Piece &aPiece) {
      Piece *thePiecePtr=(Piece*)&aPiece;
          
      if(priorPiece && (priorPiece!=thePiecePtr)
          && (colors[activePlayer]==aPiece.getColor())) {
        return Reasons::moved2;
      }
      priorPiece=thePiecePtr;
      return Reasons::tbd;
    }
        
    //-- USE: accept player move, update game state ------------------
    bool movePieceTo(const Piece &aPiece, const Location &aDestination) override {
      if(auto theActualPiece=getActualPiece(aPiece)) {
      
        reason=validatePiece(*theActualPiece);
        if(Reasons::tbd==reason) {
          
         // float theDist = distance(aPiece.location, aDest.location);
          Tile* theSrcTile=(Tile*)tileAt(aPiece.getLocation());
          Tile* theDestTile=(Tile*)tileAt(aDestination);
          
          if(isValidMove(aPiece, *theSrcTile, *theDestTile)) {
            setPiece(*theSrcTile, nullptr);
            setPiece(*theDestTile, theActualPiece);
            
            priorDist=stepLen;
            if(auto theOpponent = jumpedOpponent(*theActualPiece, aDestination)) {
              removePiece(theOpponent);
              priorDist=jumpLen;
              jumps++;
            }
            else {
              if(pieceHasJump(*theActualPiece, colors[activePlayer])) {
                reason=Reasons::missedJump;
                return false;
              }
              steps++;
            }
            
            theActualPiece->setLocation(aDestination);
            if(isKingable(aPiece, aDestination)) {
              theActualPiece->setKind(PieceKind::king);
            }
            
            return true;
          }
          else reason=Reasons::badmove;
        }
        loserColor = colors[activePlayer];
      }
      
      return false;
    }
    
    //-- USE: identify terminal condition for game -------------------------
    bool gameCanContinue(PlayerInterface &aPlayer) {
      
      if(Reasons::tbd!=reason) return false; //someone already lost...
      
      if(++step<kMaxSteps) {
        size_t blue_count = countAvailablePieces(PieceColor::blue);
        size_t gold_count = countAvailablePieces(PieceColor::gold);
        
        auto theColor = colors[&aPlayer];

        if(theColor==PieceColor::blue) {
          if(!blue_count) {
            loserColor = PieceColor::blue;
            reason=Reasons::eliminated;
          }
        }
        else {
          if(!gold_count) {
            loserColor = PieceColor::gold;
            reason=Reasons::eliminated;
          }
        }
        return blue_count*gold_count>0;
      }
      reason=Reasons::clockExpired;
      return false;
    }
    
    //-- USE: Print conclusing message to console... -------------------------
    PieceColor showGameResults(std::ostream &anOutput) {

      const char *theLoser=PieceColor::blue==loserColor ? "Blue" : "Gold";
      const char *theWinner=PieceColor::blue==loserColor ? "Gold" : "Blue";

      switch(reason) {
         case Reasons::eliminated:
          anOutput << theWinner << " has eliminated " << theLoser << " from the board.\n";
           break;
         case Reasons::badmove:
           anOutput << theLoser << " is disqualified for choosing an illegal move.\n";
           break;
         case Reasons::forfeit:
           anOutput << theLoser << " has forfeited the game.\n";
           break;
         case Reasons::missedJump:
          anOutput << theLoser << " has forfeited by missing a jump.\n";;
           break;
         case Reasons::moved2:
          anOutput << theLoser << " is disqualified for moving two pieces.\n";;
           break;
         case Reasons::clockExpired:
          anOutput << "Game terminated because the clock expired.\n";;
           break;
         case Reasons::tbd:
           anOutput << "Game terminated for an unknown reason.\n";
           break;
       }
      anOutput << steps << " steps, " << jumps << " jumps\n";
       return loserColor==PieceColor::blue ? PieceColor::gold : PieceColor::blue;
    }
    
    void showSeperator(std::ostream &anOutput, const char *aPrefix="") {
      anOutput << aPrefix;
      for(size_t i=0;i<scale;i++) anOutput << "---|";
      anOutput << std::endl;
    }
    
    //-- USE: dumps state of board to terminal... -------------------------
    void visualizeBoard(std::ostream &anOutput, PieceColor aColor=PieceColor::none) {
      
      anOutput << std::endl << "  Step " << step;
      if(PieceColor::none!=aColor) {
        anOutput << " (" << (char)aColor << ")";
      }
      anOutput << "\n";
      
      showSeperator(anOutput, "   |");
      
      for(int row=0;row<(int)scale;row++) {
        anOutput << std::setfill (' ') << std::setw(2) << row << " |";
        for(int col=0;col<(int)scale;col++) {
          Location theLocation(row,col);
          auto &tile = tiles[row][col];
          auto thePiece = tile.getPiece();
          if(!thePiece) {
            anOutput << " . |";
          }
          else {
            char theColor=' ';
            if(thePiece) {
              auto temp=thePiece.value();
              theColor = PieceColor::blue==temp.getColor() ? 'b' : 'g';
              if(PieceKind::king==temp.getKind()) theColor=toupper(theColor);
            }
            anOutput << " " << theColor << " |";
          }
        }
        anOutput << std::endl;
        if(row<(int)scale) showSeperator(anOutput, "   |");
      }
      
      anOutput  << "  ";
      for(int col=0;col<(int)scale;col++) {
        anOutput << std::setfill (' ') << std::setw(4) << col;
      }
      anOutput << std::endl;
    }
        
    
    //-- USE: iterates player turns until end state  -------------------------
    Reasons run(PlayerInterface *aP1, PlayerInterface *aP2, std::ostream &anOutput) {
    
      initPieces(); //once at the start of each game...
      
      activePlayer=aP1;
      colors[aP1]=PieceColor::gold;
      colors[aP2]=PieceColor::blue;
      step=0;
      visualizeBoard(anOutput);

      while(Reasons::tbd==reason && gameCanContinue(*activePlayer)) {
        auto theColor=colors[activePlayer];
        
        priorPiece=nullptr; //tracking multi-moves...
        
        //debugDump();

        if(!activePlayer->takeTurn(*this, theColor, anOutput)) {
          loserColor = theColor;
          reason=Reasons::forfeit;
        }
        else if(priorDist==jumpLen && pieceHasJump(*priorPiece,theColor)) {
          loserColor = theColor;
          reason=Reasons::missedJump;
        }
        
        visualizeBoard(anOutput, theColor);

        activePlayer = (activePlayer==aP1) ? aP2 : aP1;
        priorDist=0;
      }
      showGameResults(anOutput);
      return reason;
    }
    
  };

  //-- USE: game factory  -------------------------
  Reasons Game::run(PlayerInterface *aP1, PlayerInterface *aP2,
                    std::ostream &anOut) {
    RealGame theGame;
    return theGame.run(aP1, aP2, anOut);
  }
  
}
