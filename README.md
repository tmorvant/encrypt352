This program will take an input txt file and will convert it to an output txt file. I have not implemented the reset_requested and reset finished functions.

The command 'make' will compile the project and create an executable encrypt352 (with no warnings!!!!!!!). The command './encrypt352 input output' will run the program (in which input and output are txt files). The program uses multithreading to iterate through the document input. It will then simultaneously count each each character in input, encrypt it using a key, count the new characters, and print them to a document in order using different threads.

The command 'make clean' will remove any .o files, the encrypt352 executable, and any file named "output.txt".

