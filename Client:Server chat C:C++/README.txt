To compile:

	g++ chatserve.cpp -o test1
	gcc clientside.c -o test2	
to run:
	open up 2 terminal windows
	./test1 30020
	./test2 127.0.0.1 30020
	 

	The client must start typing first. Then the server can type back. To quit the client can type ctrl-c in the terminal. or type /quit. If the client disconnect a new client can reconnect.

There are lot of know issues. I didn't realize server had to be written in Cpp untill a lot later. So because of that there are lots of known bugs such as: 
	- the names displaying incorrectly and double messages being sent.
	- the chat on the client starts in a new line.
