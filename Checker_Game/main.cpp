//
//  main.cpp
//  Checkers
//
//  Created by rick gessner on 2/22/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <iostream>
#include <sstream>
#include "Game.hpp"
#include "BaiqiangPlayer870.hpp"

//STUDENT: include your own playere file here
//         (named after you, like "RenjiePlayer")...
//         Use your player below, instead of the default Player class


size_t runTest(ECE141::Reasons aReason, std::ostream &anOutput) {
  size_t theCount=5;
  for(size_t theIndex=0;theIndex<5;theIndex++) {
    
    ECE141::BaiqiangPlayer870 *player1=new ECE141::BaiqiangPlayer870;
    ECE141::BaiqiangPlayer870 *player2=new ECE141::BaiqiangPlayer870;

    anOutput << "Run: " << theIndex << "\n";
    auto theResult=ECE141::Game::run(player1, player2, anOutput);
    if(aReason==theResult) theCount--;
  }
  return theCount;
}

int main(int argc, const char * argv[]) {
  
  const char* theStatus[]={"FAIL","PASS"};
        
  // insert code here...
  if(argc>1) {
        
    srand(static_cast<uint32_t>(time(0)));
    std::string temp(argv[1]);
    
    std::stringstream theStream;

    if("compile"==temp) {
      std::cout << "Compile test PASS\n";
    }
    else if("win"==temp) {
      size_t theCount=runTest(ECE141::Reasons::eliminated, theStream);
      std::cout << "Win test " << theStatus[theCount!=5] << "\n";
      std::cout << "\n" << theStream.str() << "\n";
    }
    else if ("move"==temp) {
      size_t theCount=runTest(ECE141::Reasons::badmove, theStream);
      std::cout << "Move test " << theStatus[theCount==5] << "\n";
      std::cout << "\n" << theStream.str() << "\n";
    }
    else if ("piece"==temp) {
      size_t theCount=runTest(ECE141::Reasons::moved2, theStream);
      std::cout << "Piece test " << theStatus[theCount==5] << "\n";
      std::cout << "\n" << theStream.str() << "\n";
    }
    else if ("choice"==temp) {
      size_t theCount=runTest(ECE141::Reasons::missedJump, theStream);
      std::cout << "Choice test " << theStatus[theCount==5] << "\n";
      std::cout << "\n" << theStream.str() << "\n";
    }
  }
  
  return 0;
}
