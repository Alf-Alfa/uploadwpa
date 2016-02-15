# build uploadwpa executable when user executes "make" 

uploadwpa.o: uploadwpa.cpp
	$(CXX) $(CXXFLAGS) -std=c++11 -c HTTPClient.cpp uploadwpa.cpp
	$(CXX) $(LDFLAGS) -std=c++11 HTTPClient.o uploadwpa.o -o uploadwpa

# remove object files and executable when user executes "make clean"
clean:
	rm *.o uploadwpa
