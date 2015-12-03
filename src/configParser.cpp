#include "configParser.h"


bool configParser::getCommandConfig(commandConfig* cmdConf){

  //load cmdConf with all the data and check for error
  if(!getCommandsAndBash(cmdConf))
    return false;
  
  if(!getExitCommands(cmdConf))
    return false;
  
  if(!getKeywords(cmdConf))
    return false;

  return true;

}


//private functions:

//parses the file for both the commands and the coresponding bash commands
bool configParser::getCommandsAndBash(commandConfig* cmdConf){
  //check to make sure the file is good
  if(!configFile->good()){
    return false;
  }

  
  //start at the beg of file
  configFile->seekg(0, configFile->beg);
  
  //read the file until the line is @commands
  std::string line;
  while(getline(*configFile, line)){
    if(line.find("@commands") !=  std::string::npos){
      break;
    }
  }

  //true if the current line is a voice command and false if bash command
  bool isCommand = true;
 
  //while !@exit:
  while(getline(*configFile, line)){
    //leave the loop once you reach @exit
     if(line.find("@exit") !=  std::string::npos){
      break;
     }
     else{
       //if !#:
       if (line[0] != '#' && line.size() > 0){
	 if (isCommand){
	   //add element to voiceCommand
	   //cout << "Voice Command: " << line << endl;
	   cmdConf->voiceCommands.push_back(line);
	   isCommand = false;
	 }
	 else{
	   //add element to bashCommands
	   //cout << "Bash Command: " << line << endl;
	   cmdConf->voiceCommandBash.push_back(line);
	   isCommand = true;
	 }
       }
     }
  }
  
  //check to make sure neither vectors are empty
  if(cmdConf->voiceCommands.empty() || cmdConf->voiceCommandBash.empty()){
    return false;
  }
  
  //if the the two vectors are equal in length return true
  if(cmdConf->voiceCommands.size() != cmdConf->voiceCommandBash.size()){
    return false;
  }
  
  return true;
}

//parse the file for the exit commands
bool configParser::getExitCommands(commandConfig* cmdConf){
  //check to make sure the file is good
  if(!configFile->good()){
    return false;
  }
  
  //start at the begining of the file
  configFile->seekg(0, configFile->beg);
  
  std::string line;
  //read file until @exit
  while(getline(*configFile, line)){
    if(line.find("@exit") != std::string::npos){
      break;
    }
  }
  //read all the commands until the end of the file
  while( getline(*configFile, line) ){
    if(line[0] != '#' && line.size() > 0){
      //cout << "exit command: " << line <<endl;
      cmdConf->exitCommands.push_back(line);
    }
  }
  //if everything went well return true
  bool success = true;
  
  //check to make sure its not empty
  if(cmdConf->exitCommands.empty()){
    success = false;
  }

  
  return success;
}

//parse the keywords files for the keywords
bool configParser::getKeywords(commandConfig* cmdConf){
  //make sure the file is good
  if(!keywordsFile->good()){
    return false;
  }
  
  //read each line until the end of the file
  std::string line;
  while( getline(*keywordsFile, line) ){
    //add each substring (everything before " /" in the line) to the keywords
    size_t endPos = line.find(" /");
    if(endPos != std::string::npos){ 
      cmdConf->keywords.push_back(line.substr(0,endPos));
      //cout << "Keyword Phrase: "<< line.substr(0,endPos) << endl;
    }
  }

  //check to make sure at least one keyword was found
  if(cmdConf->keywords.empty()){
    return false;
  }

  //everything looks good return true
  return true;
}
