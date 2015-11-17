#include <iostream>
#include <pocketsphinx/pocketsphinx.h>
#include <stdio.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>
#include <unistd.h>
#include <ps_search.h>

#define MODELDIRECTORY "hubo-us/"
//#define MODELDIRECTORY "/usr/local/share/pocketsphinx/model/hubo-us/"

//consts:
const char* grammarLocation = MODELDIRECTORY "hubo.jsgf";
const char* keywordsLocation = MODELDIRECTORY "keywords.txt";
const char* lmLocation = MODELDIRECTORY "en-us.lm.bin";
const char* dictLocation = MODELDIRECTORY "cmudict-en-us.dict";


const char* keywordsSearch = "keyword";
const char* grammarSearch = "commands";
const char* lmSearch = "commandslm";
 
const unsigned int kSleepTime = 100000; //sleep time is us

//const int32 kSampleRate = 16000; //samples per second 

const unsigned int kBufferSize = 2048; //audio buffer size

//function protos:

int runCommand(const char*);

bool init();

void close();

//global objeccts:
//audio decoder
ps_decoder_t *ps;
//audio device (mic)
ad_rec_t *ad;
//holds the configuration
cmd_ln_t *config;


int main(int argc, char *argv[]){
  

  char const *hyp;
  
  int32 score;

  uint8 uttStarted, inSpeech, commandMode;

  bool endRec = false;

  int16 buff[kBufferSize];

  int32 adReadValue;


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

      if(!commandMode){
	if(hyp != NULL){
	  std::cout << "\nI heard: OK HUBO! Score: " << score <<  std::endl;
	  commandMode = TRUE;
	  ps_set_search(ps, lmSearch);
	  std::cout << "Search switched to lm Search" << std::endl;
	}
      }
      else{
	if( hyp != NULL && (strcmp(hyp, "") != 0) ){
	  std::cout << "\nI heard: " << hyp << " Score: " << score << std::endl;
	  if( (strcmp(hyp, "OK HUBO")) != 0 ){
            if( (strcmp(hyp, "STOP")) == 0 ){
              endRec =  true;
            }
	    else{
	      runCommand(hyp);
	    }
	    commandMode = FALSE;
	    ps_set_search(ps, keywordsSearch);
	    std::cout << "Looking for a keyword now" << std::endl;
	  }       
	}
      }

      //start a new utterance
      if(ps_start_utt(ps) < 0){
	std::cout << "Failed to start utterance" << std::endl;
	return -1;
      }
      uttStarted = FALSE;
      std::cout << "Ready" << std::endl;
    }
    //sleep for 100 ms
    usleep(kSleepTime);
    
  }
}

    
  close();
  
  std::cout << "Bye" << std::endl;
  return 0;
}


int runCommand(const char* command){
  int success = 0;

  //check for the different commands
  if( strncmp(command, "WAVE", 4) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "RUN DANCING DEMO", 16) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "RUN DRUMBING DEMO", 17) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "RUN FABRIC DEMO", 15) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "DO THE ROBOT", 12) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "DO THE DISCO", 12) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "DO THE EGYPTIAN", 15) == 0 )
    success = system("python wave.py");
  else if( strncmp(command, "DO THE CHICKEN DANCE", 20) == 0 )
    success = system("python wave.py");
  else
    std::cout << "Command: " << command <<" not recognized" << std::endl;

  
  return success;

}


bool init(){

config = cmd_ln_init(NULL, ps_args(), TRUE,
		     "-hmm", MODELDIRECTORY "en-us",
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
