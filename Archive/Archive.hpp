//
//  Archive.hpp
//  RGAssignment4
//
//  Created by rick gessner on 1/24/21.
//

#ifndef Archive_hpp
#define Archive_hpp

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>


namespace ECE141 {
  
  enum class ActionType {added, extracted, removed, listed, dumped};
  enum class AccessMode {AsNew, AsExisting}; //you can change values (but not names) of this enum

  struct ArchiveObserver {
    virtual void operator()(ActionType anAction,
        const std::string &aName, bool status) = 0;
  };
  
  //---------------------------------------------------

  class Archive {
  protected:
              Archive(const std::string &aFullPath, AccessMode aMode);  //protected on purpose

  public:

              ~Archive();  //

    static    Archive* createArchive(const std::string &anArchiveName);
    static    Archive* openArchive(const std::string &anArchiveName);

    Archive&  addObserver(ArchiveObserver &anObserver);
    
    bool      add(const std::string &aFilename);
    bool      extract(const std::string &aName, const std::string &aFullOutputPath);
    bool      remove(const std::string &aFilename);
    
    size_t    list(std::ostream &aStream);
    size_t    debugDump(std::ostream &aStream);

    size_t    compact();
    std::string getFullPath() const; //get archive path (including .arc extension)
      
    const static size_t kBlockSize{1024};
    struct Block {uint8_t data[kBlockSize];};
    std::vector<Block> block_vector;// create a vector which contains blocks
    std::vector<ArchiveObserver*> observer_vector;
    std::string File_Name;
    size_t num_Block;
    
      
    
    std::string get_block_name(size_t theIndex);
    size_t get_block_size(size_t theIndex);
    std::string extract_file_name(const std::string &aFullPath);
      
    void write_file();
    
    void write_block(size_t block_vector_index,std::string &input_file_name, std::ifstream &file_data, uint16_t aReminder,size_t file_block_numer, int file_block_index = -1);
    void do_observe(ActionType anAction, const std::string& aName, bool status);
  };

}


#endif /* Archive_hpp */
