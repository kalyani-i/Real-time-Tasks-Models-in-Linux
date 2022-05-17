Assignment 1
---------------
Subject - CSE 438
Assignment 1
Author - Kalyani Niwrutti Ingole
ASU Id - 1223133028
---------------

ReadMe

###
System requirement

VMware virtual Ubuntu Machine
gcc Compiler 

######

We have sceduled Real-time Tasks Models in Linux for aperiodic and Periodic tasks in parallel.

###
Build
We have made a make file that includes the compiler and other commands that generate any warning needed
Import the header file provided in the main assignment file for compilation
For keyboard event to get the input number, follow the following instructions
	xinput - to get the id of the input device
	the change the "/dev/input/eventx" according to the input id of the keyboard device
To check for errors and compilation of the code use 'make' command

####


###
Execution
To run the file use 'sudo ./Homework'
Install kernael shark on virtual machine use following instructions
	sudo apt-get update –y
	sudo apt-get install –y trace-cmd kernelshark
To create the .dat file for tracing 
	sudo trace-cmd record –e sched_switch taskset –c 3 ./assignment
	the number defines the CPU in usage
use trace.dat to trace the threads
Select the appropriate CPU and task that we have to trace

###

