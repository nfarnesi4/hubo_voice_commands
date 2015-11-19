CC = g++

#compiler flags
CFLAGS =-c -Wall

SRC_DIR = src

SOURCES=$(SRC_DIR)/main.cpp $(SRC_DIR)/configReader.cpp
OBJECTS=$(SOURCES:.cpp=.o)

#inlcude path and linker
LINKER_FLAGS = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad

#the excutible name
TARGET = heyListen

#the dir location for the share folder on the comuter
SHARE_DIR = /usr/local/share/$(TARGET)
#the dir for the install bin
BIN_DIR = /usr/local/bin
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LINKER_FLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(LINKER_FLAGS) $< -o $@  

#NOTE: must be run as super user
install: $(TARGET)
	mkdir -p $(SHARE_DIR)
	cp -rf model $(SHARE_DIR)
	cp -f config $(SHARE_DIR)/$(TARGET).config
	cp -f $(TARGET) $(BIN_DIR)

#NOTE: must be run as a super user
remove:
	rm -rf $(SHARE_DIR)
	rm $(BIN_DIR)/$(TARGET)

clean:
	$(RM) $(TARGET) *~ hubo-us/*~ $(SRC_DIR)/*~ $(SRC_DIR)/*.o 

