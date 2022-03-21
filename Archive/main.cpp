//
//  main.cpp
//  Assignment4
//
//  Created by rick gessner on 1/24/21.
//

#include <iostream>
#include <cstdlib>
#include <sstream>
#include "Testing.hpp"

std::string getLocalFolder() {
    return std::string("/tmp"); //PATH TO LOCAL ARCHIVE FOLDER
}

int main(int argc, const char * argv[]) {
  
  static const char* kMsgs[]={"FAIL","PASS"};
  if(argc>1) {
    std::string temp(argv[1]);
    std::stringstream theOutput;
    
    if("compile"==temp) {
        std::cout << temp << " test " << kMsgs[true] << "\n";
    }
    else {
      std::string theFolder(getLocalFolder());
      if(3==argc) theFolder=argv[2];
      ECE141::Testing aTest(theFolder);
      if("create"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doCreateTests(theOutput)] << "\n";
      }
      else if("open"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doOpenTests(theOutput)] << "\n";
      }
      else if("add"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doAddTests(theOutput)] << "\n";
      }
      else if("extract"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doExtractTests(theOutput)] << "\n";
      }
      else if("remove"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doRemoveTests(theOutput)] << "\n";
      }
      else if("list"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doListTests(theOutput)] << "\n";
      }
      else if("dump"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doDumpTests(theOutput)] << "\n";
      }
      else if("stress"==temp) {
        std::cout << temp << " test "
          << kMsgs[aTest.doStressTests(theOutput)] << "\n";
      }
    }
    std::cout << theOutput.str() << "\n";
  }
  
  return 0;
}
