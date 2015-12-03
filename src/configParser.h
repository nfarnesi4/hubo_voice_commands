#include <vector>
#include <string>
#include <fstream>

#pragma once

//the data type that contains all of the config data read from the files
struct commandConfig{
  //A vector of all the possible commands
  std::vector<std::string> voiceCommands;
  //A vector of all the bash commands to the coresponding voiceCommands
  std::vector<std::string> voiceCommandBash;
  //holds a list of all the exit commands
  std::vector<std::string> exitCommands;
  //holds the list ok keyword phrases
  std::vector<std::string> keywords;
};
  


class configParser{
 private:
  
  std::ifstream* configFile;
  std::ifstream* keywordsFile;

 //private functions:
  
  bool getCommandsAndBash(commandConfig*);
  bool getExitCommands(commandConfig*);
  bool getKeywords(commandConfig*);

 public:
  
  //constructor
  configParser(const char* configFileLoc, const char* keywordsFileLoc){
    configFile = new std::ifstream(configFileLoc);
    keywordsFile = new std::ifstream(keywordsFileLoc);
  }

  //destructor
  ~configParser(){

    //close and delete the in file streams
    
    configFile->close();
    keywordsFile->close();

    delete configFile;
    delete keywordsFile;
  }


  //reads the all the config files loads the the data into the input commandConfig
  bool getCommandConfig(commandConfig*);

  
};
