Name: Savannah Hamley
ID Number: 811519334
Parallel Sum Calculator
How to Run Program:
    1. Ensure the following files are located in your directory:
        - Makefile
        - proj4.c
        - proj4.h
        - main.c
        - all example files extracted from examples.tar.gz
        - README.txt
    2. Compile using the commands:
        make clean

        *clean ensures that no conflicting out files will interfere with the program. 

        make
    3. Run on a VCF node using a similar command:
        ./proj.out in1.txt out1.txt 10 1; diff out1.txt correctOut1.txt | wc -c;

        The program will compute the diagonal sums in the appropriate size grid and number 
        of threads. 

        It will display the amount of time taken to run the thread(s). This was well under 60 seconds
        for all of my test cases (excluding a couple of in depth valgrind checks).

        It lastly will write the diagonal sums to the file and display a goodbye message. A zero should 
        appear for every case to indicate it was performed correctly.

    4. Test for memory leaks with Valgrind using a similar command:
        valgrind ./proj4.out in1.txt out1.txt 10 1; diff out1.txt correctOut1.txt | wc -c;

        This report should show that all heap blocks were freed.
