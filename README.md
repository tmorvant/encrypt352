This program will take an input txt file and will convert it to an output txt file. I have not implemented the reset_requested and reset finished functions.

The command 'make' will compile the project and create an executable encrypt352 (with no warnings!!!!!!!). The command './encrypt352 input output' will run the program (in which input and output are txt files). The program uses multithreading to iterate through the document input. It will then simultaneously count each each character in input, encrypt it using a key, count the new characters, and print them to a document in order using different threads.

I have a strange bug where output counts are shifted one letter up  for each time the key resets when running encrypt352.c with encrypt-module-reproducible.c, but it works perfectly with the regular encrypt-module.c. This means that the first reset will show the A count in the B count and so on, while the second reset output will display the A count as C count, and so on and so forth. This does not happen with the regular encrypt-module.c so I have given up.

