Written by: 
Ali Tolga Dinçer
150150730


Summary:

I read the all of the file at the beginning of the program and put it into large char array
named input[], in order to avoid unexpected behaviours while forking.
I read the water capacity as an integer using fscanf, since it can be greater then 9.

I then created a child process which will wait in a while loop, until an empty signal comes
or the program ends. When an empty signal comes the process fills the machine (changes the shared memory),
sends a full signal and continues to sleep.

The main program reads until the end of the input. For each input (1 or 2) it creates a child process
and waits until the child is finished. The child checks if it can decrement the shared memory variable,
if it can it simply decrements the variable. If the shared memory variable will become less then 1, it calls
the filler process, waits until the water is filled (full signal) and then decrements it. After finishing it's job the
child sends a continue signal to main.

In the end; all processes are finished, the 2 semaphores and the shared memory area are returned to the system.



Variables:

I used 2 semaphores (empty and full). The filler process waits for the empty signal to come. While the filler
process is running, the other process waits for the full signal to come.
I allocated 1 integer sized shared memory for the water level.



Compilation:

You can use "gcc MyHomework.c -o hw3.out" in terminal to compile.

Running:

You can use "./hw3.out input.txt" to run the precompiled program.



