CC = g++
OUT_FILE_NAME = libMyMalloc.a

CFLAGS= -fPIC -O0 -g -Wall -c -fpermissive

INC = -I../Import

OBJ_DIR=./obj

OUT_DIR=./lib

$(OUT_FILE_NAME): $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(wildcard *.cpp))
	ar -r -o $(OUT_DIR)/$@ $^



#Compiling every *.cpp to *.o
$(OBJ_DIR)/%.o: %.cpp dirmake
	$(CC) -c $(INC) $(CFLAGS) -o $@  $<
	
dirmake:
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJ_DIR)
	
clean:
	rm -f $(OBJ_DIR)/*.o $(OUT_DIR)/$(OUT_FILE_NAME) Makefile.bak

rebuild: clean build
