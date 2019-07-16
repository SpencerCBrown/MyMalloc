all:
	g++ -std=c++11 -o a.lib *.cpp 

clean:
	rm -f $(OBJS) $(OUT)