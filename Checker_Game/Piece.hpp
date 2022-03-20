//  Piece.hpp
//
//  Created by rick gessner on 2/22/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//
#ifndef Piece_hpp
#define Piece_hpp

#include  "stdio.h"
#include <optional>

namespace ECE141 {

  class Piece; //fwd-declare to avoid compile issue...

  enum class PieceKind   {pawn, king, captured};
  enum class PieceColor : char {gold='g', blue='b', none=' '};
  enum class TileColor   {light, dark};
  enum       Orientation {north=0, east=1, south=2, west=3};
  
  //stores col, row for tiles, peices, etc.
  struct Location {
    int  row,col;
    Location(int aRow=-1, int aCol=-1) : row(aRow), col(aCol) {}
    Location(const Location &aCopy) :  row(aCopy.row), col(aCopy.col) {}
    bool operator==(const Location &aLoc) {
      return row==aLoc.row && col==aLoc.col;
    }
  };
  
  using LocationOpt = std::optional<Location>;

  //Single tile on the game board
  //NOTE: you HAVE to use the getter (accessor) methods to get properties
  

  //--------------------------------------

  //Defines game pieces on the board; 2 colors (gold, blue)
  //You MUST use getters to get piece properties
  class Piece {
    PieceKind    kind;
    Location     location;
    PieceColor   color;
    friend class Game;

  public:
    Piece(const Location &aLocation=Location(-1,-1),
          PieceColor aColor=PieceColor::none,
          PieceKind aKind=PieceKind::pawn) :
      kind(aKind), location(aLocation), color(aColor) {
    }
        
    Piece(const Piece &aCopy) {*this=aCopy;}
    
    Piece& operator=(const Piece &aCopy) {
      kind=aCopy.kind;
      color=aCopy.color;
      location=aCopy.location;
      return *this;
    }
    
    bool operator==(const Piece &aCopy) {
      return color==aCopy.color && location==aCopy.location;
    }
    
    bool operator <(const Piece& aRHS) const {
      const size_t kExtrema{1000};
      return (location.row*kExtrema)+location.col
        < (aRHS.location.row*kExtrema)+aRHS.location.col;
    }
    
    const PieceKind   getKind() const {return kind;}
    const PieceColor  getColor() const {return color;}
    const Location&   getLocation() const {return location;}
        
    Piece& setLocation(const Location &aLocation) {
      location=aLocation;
      return *this;
    }

    Piece& setKind(const PieceKind &aKind) {
      kind=aKind;
      return *this;
    }
  };

  using PieceOpt = std::optional<Piece>;

  struct Tile {
    Tile(TileColor aTileColor, Location aLocation, Piece *aPiece=nullptr)
      : location(aLocation), piece(aPiece), color(aTileColor)   {
    }

    Tile(const Tile &aTile) : location(aTile.location), piece(aTile.piece), color(aTile.color) {}

    const Location& getLocation() const {return location;}
    const TileColor getTileColor() const {return color;}
    bool            isEmpty() const {return nullptr==piece;}

    PieceOpt        getPiece() const  {
      if(piece) return *piece;
      return std::nullopt;
    }

  protected:
    
    const Location  location;
    Piece           *piece;
    TileColor       color;
    
    friend class Game;
  };
  
  using TileOpt  = std::optional<Tile>;

}
#endif /* Piece_hpp */
