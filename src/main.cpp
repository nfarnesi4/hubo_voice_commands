#include <iostream>
#include <pocketsphinx/pocketsphinx.h>
#include <stdio.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <unistd.h>
#include <ps_search.h>

#include "configParser.h"

//#define MODELDIRECTORY "model/hubo-us/"
#define SHAREDIR "/usr/local/share/heyListen/"
#define MODELDIRECTORY SHAREDIR "model/hubo-us/"

//consts:

//the dirs for all the resources
const char* grammarLocation = MODELDIRECTORY "hubo.jsgf";
const char* keywordsLocation = MODELDIRECTORY "keywords.txt";
const char* lmLocation = MODELDIRECTORY "en-us.lm.bin";
const char* dictLocation = MODELDIRECTORY "cmudict-en-us.dict";
const char* hmmLocation = MODELDIRECTORY "en-us";
const char* configLocation = SHAREDIR "heyListen.config";

//the different search modes
const char* keywordsSearch = "keyword";
const char* grammarSearch = "commands";
const char* lmSearch = "commandslm";

const unsigned int kSleepTime = 100000; //sleep time is us

//const int32 kSampleRate = 16000; //samples per second

// the min score that will be accepted for the grammar search
const int32 kScoreThres = -5000;

// the max number of times grammar search will be attempted before
// the program switches back to keyword search
const unsigned int kMaxGrammarSearchTries = 4;

const unsigned int kBufferSize = 2048; //audio buffer size

//function protos:

//test if the input is one of the exit commands
bool isExitCmd(const char*);

//test if the input string is one of the keywords
bool isKeyword(const char*);

//run the command based on the input
int runCommand(const char*);

//init all the sphix stuff
bool init();

//free up the memory
void close();

//global objects:

//command config
commandConfig cmdConf;
//audio decoder
ps_decoder_t *ps;
//audio device (mic)
ad_rec_t *ad;
//holds the configuration
cmd_ln_t *config;


int main(int argc, char *argv[]){
  //hold the guess for what the person said
  char const *hyp;
  //the score for the guess of what the person said
  int32 score;
  //booleans to hold the differnt states
  uint8 uttStarted, inSpeech, commandMode;
  //true when the user exits the program
  bool endRec = false;
  //buffer to hold the recorded audio
  int16 buff[kBufferSize];
  //holds the value of what was read in from the audio device
  int32 adReadValue;
  //stores the number of grammar search tries
  unsigned int grammarSearchTries = 0;

  if(init()){
  
  //start recording
  if(ad_start_rec(ad) < 0){
    std::cout << "Could not start recording\n" << std::endl;	    
  }

  //set the keywordSearch
  if(ps_set_search(ps, keywordsSearch) < 0){
    std::cout << "failed to set the search :(" << std::endl;
  }

  //start the utterance
  if(ps_start_utt(ps) < 0){
    std::cout << "Could not start utterance\n" << std::endl;
  }

  uttStarted = FALSE;
  
  //commandMode will be true once the key word is detected
  commandMode = FALSE;
  

  std::cout << "Ready" << std::endl;
  //start the processing
  while(!endRec) {
  
    //check to make sure the read goes ok
    if( (adReadValue = ad_read(ad, buff, kBufferSize)) < 0){
      std::cout << "!!!Failed to read the audio!!!" << std::endl;
    }

    //process the raw audio from the mic
    ps_process_raw(ps, buff, adReadValue, FALSE, FALSE);

    //test to see if someone is currently talking
    inSpeech = ps_get_in_speech(ps);

    //if the utterance is not started and someone is talking
    //start listeing
    if(inSpeech && !uttStarted) {
      uttStarted = TRUE;
      std::cout << "Listening: " << std::endl;
    }

    //the person stopped speaking now or the buffer is full
    //process the data and start a new utterance
    if(!inSpeech && uttStarted){
      
      //end the untterance
      ps_end_utt(ps);

      //get the result and store it in hyp
      hyp = ps_get_hyp(ps, &score);

      //if were not looking for a command check for a keyword
      if(!commandMode){
	//check for keyword
	if(hyp != NULL){
	  std::cout << "\nI heard: A keyword: " << hyp << std::endl;
	  //keyword found start looking for a command
	  commandMode = TRUE;
	  ps_set_search(ps, grammarSearch);
	  std::cout << "Search switched to grammar" << std::endl;
	}
      }
      else{
	if(grammarSearchTries < kMaxGrammarSearchTries){
	  if( hyp != NULL && (strcmp(hyp, "") != 0) ){
	    std::cout << "\nI heard: " << hyp << " Score: " << score << std::endl;
	    //check to make sure the score is good
	    if(score > kScoreThres){
	      //check to make sure its not a keyword again
	      if( !isKeyword(hyp) ){
		//check to see if its and exit command
		if( isExitCmd(hyp) ){
		  endRec =  true;
		}
		else{
		  runCommand(hyp);
		}
		//everything went well go back to keyword search
		commandMode = FALSE;
		ps_set_search(ps, keywordsSearch);
		grammarSearchTries = 0;
		std::cout << "Looking for a keyword now" << std::endl;
	      }
	      else{
		++grammarSearchTries;
		std::cout << "The command is a keyword. Try: " << grammarSearchTries << std::endl;
	      }
	    }
	    else{
	      ++grammarSearchTries;
	      std::cout << "Not above threshold. Try: " << grammarSearchTries << std::endl; 
	    }  
	  }
	  else{
	    //grammar could not be matched to anything
	    ++grammarSearchTries;
	    std::cout << "Could not find a command that matched the gram.";
	    std::cout << " Try: " << grammarSearchTries << std::endl;
	  }
	}
	else{
	  //exceeded the number of tries
	  //switch back to keyword search
	  commandMode = FALSE;
	  ps_set_search(ps, keywordsSearch);
	  grammarSearchTries = 0;
	  std::cout << "Max tries for grammar search reached. Switchig back to keyword" << std::endl;
	}
      }

      //start a new utterance
      if(ps_start_utt(ps) < 0){
	std::cout << "Failed to start utterance" << std::endl;
      }
      uttStarted = FALSE;
      std::cout << "Ready" << std::endl;
    }
    //sleep for 100 ms
    usleep(kSleepTime);
  }
}
  //All done now free all the memory
  close();
  
  std::cout << "Bye" << std::endl;
  return 0;
}

//Functions:

bool isExitCmd(const char* hyp){
  //test for the exit commands
  //this will be done from a file in the future
  if (strncmp(hyp, "stop ", 5) == 0){
    return true;
  }
  else if (strcmp(hyp, "close hubo") == 0){
    return true;
  }
  else{
    return false;
  }
  
}

bool isKeyword(const char* hyp){
  //test for all the keywords
  //this will be done from the keywords file in the future

  if (strcmp(hyp, "ok hubo") == 0){
    return true;
  }
  else if (strcmp(hyp, "listen up hubo") == 0){
    return true;
  }
  else if (strcmp(hyp, "what's up hubo") == 0){
    return true;
  }
  else{
    return false;
  }
  
}

int runCommand(const char* command){
  int success = 0;

  //check for the different commands
  //this will be done by reading from a file in the future
  
  if( strncmp(command, "wave to", 7) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "run dancing demo", 16) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "run drumming demo", 17) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "run fabric demo", 15) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "do the robot", 12) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "do the disco", 12) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "do the egyptian", 15) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "do the chicken dance", 20) == 0 )
    success = system("python wave.py");
  else
    std::cout << "Command: " << command <<" not recognized" << std::endl;

  
  return success;

}


bool init(){
  //read all the config files
  configParser confPar(configLocation, keywordsLocation);

  if(!confPar.getCommandConfig(&cmdConf)){
    std::cout << "Failed to read a config files" << std::endl;
    return false;
  }

  
config = cmd_ln_init(NULL, ps_args(), TRUE,
		     "-hmm", hmmLocation,
		     "-lm", lmLocation,
		     "-dict", dictLocation,
		       NULL);

 if (config == NULL){
    std::cout << "Failed to create config object, see log for details" << std::endl;
    return false;
  }

  ps = ps_init(config);

  if(ps == NULL){
    std::cout << "Failed to create recongnizer, see log for details" << std::endl;
    return false;

  }

  //add the keywords search ps from the file
  if(ps_set_kws(ps, keywordsSearch, keywordsLocation) < 0){
    std::cout << "failed to add the keywords search" << std::endl;
    return false;
  }

  //add the gramar search to the ps
  if(ps_set_jsgf_file(ps, grammarSearch, grammarLocation) < 0){
    std::cout << "failed to add the grammer" << std::endl;
    return false;
  }
  
  //add the commands language model
  if(ps_set_lm_file(ps, lmSearch, lmLocation) < 0){
    std::cout << "failed to add the language model" << std::endl;
    return false;
  }

  //open the defualt audio device at the default sample rate
  if( (ad = ad_open()) == NULL ){
    std::cout << "\n!!!Device creation didn't work :( \n" << std::endl;
    //std::cout << "Failed to open audio device\n" << std::endl;
    return false;
  }
  
  //everything set up correctly 
  return true;
}


void close(){
  //clean up
  ad_close(ad);
  ps_free(ps);
  cmd_ln_free_r(config);
 
}

