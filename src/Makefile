uploadwpa2: uploadwpa2.o
	$(CXX) $(LDFLAGS) -std=c++0x *.o -o uploadwpa2 -lcrypto -lssl
uploadwpa2.o: uploadwpa.cpp
	$(CXX) $(CXXFLAGS) -std=c++0x -c *.cpp


clean:
	rm *.o uploadwpa2
