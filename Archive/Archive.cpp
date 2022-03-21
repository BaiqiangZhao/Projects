//
//  Archive.cpp
//  RGAssignment3
//
//  Created by rick gessner on 1/24/21.
//

#include "Archive.hpp"
#include <vector>
#include <time.h>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <string>


# define BLOCK_HEADER 100
# define FILE_NAME_INDEX 12
# define DATA_ARRAY_SIZE 924


namespace ECE141 {

    Archive::Archive(const std::string &aFullPath, AccessMode aMode){
        block_vector.clear();
        File_Name = "";
        num_Block = 0;
    }
    Archive::~Archive(){}
    Archive* Archive::createArchive(const std::string &anArchiveName){
        
        std::fstream new_File;
        Archive* anArchive = new Archive(anArchiveName,AccessMode::AsNew);
        if (anArchiveName.find(".arc") != std::string::npos)
            anArchive->File_Name = anArchiveName;
        else
            anArchive->File_Name = anArchiveName + ".arc";
        new_File.open(anArchive->File_Name, std::ios_base::out | std::ios_base::trunc);
        if(!new_File) return nullptr;
        else return anArchive;
        
    }
    Archive* Archive::openArchive(const std::string &anArchiveName){
        if(anArchiveName.length() > (BLOCK_HEADER - FILE_NAME_INDEX)) return nullptr;
        std::string archive_file = anArchiveName + ".arc";
        std::ifstream aFile (archive_file);
        Archive *a_Archive = new Archive(anArchiveName,AccessMode::AsExisting);
        
        std::string file_name = anArchiveName;
        aFile.seekg(0, std::ios::end);
        size_t file_size = aFile.tellg();
        a_Archive->num_Block = file_size / (kBlockSize - BLOCK_HEADER) + 1;   // theCount is number of blocks needed
        uint16_t theReminder = file_size % (kBlockSize - BLOCK_HEADER);
        size_t theIndex;
        Archive::Block ablock;
        for(theIndex=0; theIndex < a_Archive->num_Block; theIndex++){//put file data into blocks
            a_Archive->block_vector.push_back(ablock);
            a_Archive->write_block(theIndex,file_name,aFile,theReminder,a_Archive->num_Block);
        }
        
        return a_Archive;
    }
    bool Archive::add(const std::string &aFilename){
        
        std::size_t found = aFilename.find_last_of('/');
        std::string file_name = aFilename.substr(found+1);//get the file name
        std::ifstream aFile;
        
        aFile.open(aFilename);
        if(!aFile){
            do_observe(ActionType::added,file_name,false);
            return false;
        }

        aFile.seekg(0, std::ios::end);
        size_t file_size = aFile.tellg();//get adding file size
        
        size_t block_number = file_size / (kBlockSize - BLOCK_HEADER);
        uint16_t aReminder = file_size % (kBlockSize - BLOCK_HEADER);
        if(aReminder > 0) block_number += 1;
        
        size_t theBlockIndex = 0;
        int adding_file_index = 0;
        Block ablock;
        while(theBlockIndex < block_vector.size()){
            if(block_vector.at(theBlockIndex).data[0] == true){
                theBlockIndex++;
                continue;// if the block is used, go nect block
            }
            if(adding_file_index == block_number) break; // adding is done
            if (!strcmp((char*)(block_vector.at(theBlockIndex).data + FILE_NAME_INDEX), file_name.c_str())) {
                do_observe(ActionType::added,file_name,false);
                return false;
            }//if a block has the same name as the adding file, reture false
            
            write_block(theBlockIndex,file_name,aFile,aReminder,block_number,adding_file_index);
            adding_file_index++;
            theBlockIndex++;
            
        }
        while(adding_file_index < block_number){//if loop of original Archive is over but adding is not finished
            num_Block++;
            block_vector.push_back(ablock);//adding one more empty block
            write_block(theBlockIndex,file_name,aFile,aReminder,block_number,adding_file_index);
            adding_file_index++;
            theBlockIndex++;
            
        }
        aFile.close();
        write_file();
        do_observe(ActionType::added,file_name,true);
        return true;
    }

    bool Archive::extract(const std::string &aName, const std::string &aFullOutputPath){
        std::ofstream output_file; //open a file in the path
        output_file.open (aFullOutputPath, std::ios::out | std::ios::binary);
        bool written = false;
        for(size_t theIndex = 0; theIndex < block_vector.size(); theIndex++){
            if(block_vector.at(theIndex).data[0] == false) continue;// if the block is empty, go next block
            if(!std::strcmp((char*)(block_vector[theIndex].data + FILE_NAME_INDEX), aName.c_str())){
                // read reaminer
                uint16_t length = DATA_ARRAY_SIZE;
                if (theIndex == block_vector[theIndex].data[1] - 1)
                    length = (((uint16_t)block_vector[theIndex].data[2]) << 8) | block_vector[theIndex].data[3];
                
                output_file.write((char*)(block_vector.at(theIndex).data + BLOCK_HEADER), length);
                written = true;
            }
        }
        output_file.close();
        
        if (written) {// if something is written in, return true
            do_observe(ActionType::extracted,aName,true);
            return true;
        }
        else {
            do_observe(ActionType::extracted,aName,false);
            return false;
        }
    }
    bool Archive::remove(const std::string &aFilename){
        bool found = false;
        for(size_t theIndex = 0; theIndex < block_vector.size(); theIndex++){
            if(block_vector.at(theIndex).data[0] == false) continue;// if the block is empty, go nect block
            if(!std::strcmp((char*)(block_vector[theIndex].data + FILE_NAME_INDEX), aFilename.c_str())){
                block_vector.at(theIndex).data[0] = false;
                found = true;
            }
        }
        if (found == true) {
            do_observe(ActionType::removed,aFilename,true);
            return true;
        }
        else {
            do_observe(ActionType::removed,aFilename,false);
            return false;
        }
    }
size_t Archive::list(std::ostream &aStream){
    aStream << "\n" << std::left << std::setw(5) << "###"
    << std::left << std::setw(15) << "name"
    << std::left << std::setw(10) << "size"
    << std::left << std::setw(15) << "date added"
    << std::endl;//print header
    
    
    std::vector<std::string> file_name_vector;
    size_t file_number = 0;
    for(size_t theIndex = 0; theIndex < block_vector.size(); theIndex++){
        if(block_vector.at(theIndex).data[0] == false) continue;//if the block is removed or empty
        std::string file_name = get_block_name(theIndex);
        if(std::count(file_name_vector.begin(),file_name_vector.end(),file_name)) continue;//if file name is in file_name_vector, continue
        else{
            file_number++;
            time_t now;
            std::memcpy(&now, block_vector.at(theIndex).data + 4, sizeof(now));
            auto added_time = ctime(&now);
            file_name_vector.push_back(file_name);
            aStream << std::left << std::setw(5) << file_number
            << std::left << std::setw(15) << this->get_block_name(theIndex)
            << std::left << std::setw(10) << this->get_block_size(theIndex)
            << std::left << std::setw(15) << added_time << std::endl;
            
        }
    }
    do_observe(ActionType::listed,"",true);
    return file_number;
}
    void Archive::write_block(size_t block_vector_index,std::string &input_file_name, std::ifstream &file_data,uint16_t aReminder,size_t file_block_numer, int file_block_index){
            time_t now = time(0);
            auto *ptr_time = &now;
            
            block_vector.at(block_vector_index).data[0] = true;// write whether block is used ot empty
            block_vector.at(block_vector_index).data[1] = file_block_numer;//store how many blocks to store the data
            block_vector.at(block_vector_index).data[2] = aReminder >> 8; //reminder of block size part 1
            block_vector.at(block_vector_index).data[3] = aReminder; //reminder of block size part 2
            std::memcpy(block_vector.at(block_vector_index).data + 4,ptr_time,sizeof(now)); //store current time to data index 3
            std::strcpy((char*)(block_vector.at(block_vector_index).data + FILE_NAME_INDEX), (input_file_name).c_str());//store file name to rest space of header
            if(file_block_index == -1) file_block_index = (int)block_vector_index;// if it is not adding
           
            file_data.seekg(file_block_index * DATA_ARRAY_SIZE );
            if(file_block_index < (file_block_numer - 1))
                file_data.read((char*)(block_vector.at(block_vector_index).data + BLOCK_HEADER), 924);//store data from file to archive
            else file_data.read((char*)(block_vector.at(block_vector_index).data + BLOCK_HEADER) , aReminder);//store the last block of data from file to archive
        }
    std::string Archive::get_block_name(size_t theIndex){
        return (char*)(block_vector.at(theIndex).data + FILE_NAME_INDEX);
    }
    size_t Archive::get_block_size(size_t theIndex){
        return (block_vector.at(theIndex).data[1] - 1) * 924 + ((block_vector.at(theIndex).data[2] << 8) + block_vector.at(theIndex).data[3]);
    }
    size_t Archive::compact(){
        std::vector<Block>::iterator ptr;
        size_t Count = 0;
        for (ptr = block_vector.begin(); ptr < block_vector.end(); ptr++){
            if (ptr->data[0] == false) block_vector.erase(ptr);
            else Count++;
        }
        return Count;
    }
    size_t Archive::debugDump(std::ostream &aStream){
        aStream << "\n" << std::left << std::setw(5) << "###"
        << std::left << std::setw(10) << "status"
        << std::left << std::setw(15) << "name"
        << std::endl;//print header
        bool status = false;
        std::string name;
        size_t index = 0;
        for(auto &block : this->block_vector){
            status = block.data[0];
            name = (char*)(block.data + FILE_NAME_INDEX);
            index++;
            aStream << "\n" << std::left << std::setw(5) << index
            << std::left << std::setw(10) << status
            << std::left << std::setw(15) << name
            << std::endl;//print header
        }
        do_observe(ActionType::dumped,"",true);
        return num_Block;
    }
    Archive&  Archive::addObserver(ArchiveObserver &anObserver){
        observer_vector.push_back(&anObserver);
        return *this;
    }
    void Archive::write_file(){
        
        std::ofstream anOutput;
        anOutput.open(File_Name);
        for(auto &block : this->block_vector)
            anOutput.write((char*)block.data, 1024);
        
        anOutput.close();
    }
    void Archive::do_observe(ActionType anAction, const std::string& aName, bool status){
        for (auto *obs : observer_vector)
            (*obs)(anAction,aName,status);
    }
}
