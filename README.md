---------
About
--------
The program is a mini-shell that runs executables, with an option to give arguments to the executables and save the output to a file.

---------------------------------
Shell Capabilities
---------------------------------
* Running executables that are in the path, including bash commands.
* Sending arguments to executable files

---------------------------------
Command Line Prompt
---------------------------------

Command line format: [return code] [user]@[host]$

Command Line Prompt Example: 0 shai@shai-laptop$


---------------------------------
Command Line Arguments
---------------------------------
When you'll run the mini-shell a prompt line will appear.
Every time it appears you can enter a new command.
- The shell will exit without an error when the word "exit" is typed.
- If a wrong command will be enter, the user will get a match error massage
Command Line Arguments Example: ls > exampleFile.txt


---------------
Compilation
---------------
Compelation line: make

run line: ./ex1
