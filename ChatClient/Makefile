SOURCE = main.cpp Application.cpp Chat.cpp NewMessages.cpp
TARGET = chat.out
UTILS_LIB = Utils
SHA_LIB = SHA1
PREFIX = /usr/local/bin

out: $(SOURCE) lib_utils lib_sha 
	g++ -o $(TARGET) $(SOURCE) -L. -l$(UTILS_LIB) -l$(SHA_LIB)

lib_utils: $(UTILS_LIB).cpp $(UTILS_LIB).h
	g++ -o $(UTILS_LIB).o $(UTILS_LIB).cpp -c
	ar rc lib$(UTILS_LIB).a $(UTILS_LIB).o

lib_sha: $(SHA_LIB).cpp $(SHA_LIB).h
	g++ -o $(SHA_LIB).o $(SHA_LIB).cpp -c
	ar rc lib$(SHA_LIB).a $(SHA_LIB).o 

clean: 
	rm -rf *.o *.a
install:
	install $(TARGET) $(PREFIX)
uninstall:
	rm -rf $(PREFIX)/$(TARGET)
	rm $(PREFIX)/User.txt
	rm $(PREFIX)/Chat.txt
	rm $(PREFIX)/UserHash.txt
	rm $(PREFIX)/NewMessages.txt
