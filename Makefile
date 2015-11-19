CC = g++

#compiler flags
CFLAGS = -Wall

#inlcude path and linker
LINKER_FLAGS = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad

#the excutible name
TARGET = heyListen

#the dir location for the share folder on the comuter
SHARE_DIR = /usr/local/share/$(TARGET)
#the dir for the install bin
BIN_DIR = /usr/local/bin

all: $(TARGET)

$(TARGET): src/main.cpp
	$(CC) $(CFLAGS) $(LINKER_FLAGS) -o $(TARGET) src/main.cpp

#NOTE: must be run as super user
install: $(TARGET)
	mkdir -p $(SHARE_DIR)
	cp -rf model $(SHARE_DIR)
	cp -f config $(SHARE_DIR)/$(TARGET).config
	cp -f $(TARGET) $(BIN_DIR)

#NOTE: must be run as a super user
remove: $(TARGET)
	rm -rf $(SHARE_DIR)
	rm $(BIN_DIR)/$(TARGET)

clean:
	$(RM) $(TARGET) ~* hubo-us/~* src/~* 

