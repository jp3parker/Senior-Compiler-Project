
Note: This was my senior project during my last semester of college. It is a compiler for a C-like
language. It would take code from this language and then output an assembly language file which could
be ran using SPIM (which can be ran directly from the command line).

---------------------------------------------

1. Your name, your partner’s name, course id. Also, please clearly specify which member is taking
which course ID (5331 or 4318). Grading scheme for each will be different.

Jacob Parker, course ID 4318
James Hang, course ID 4318

2. How often do you plan to work on the project? What technology will you use for the virtual
meetings?

We plan to set a few hours a week on the side to work on the project. We will use slack for the main means of commuication. 

3. Explain which parts of the assignment have been implemented by which team member. The
instructor reserves the right to weight grade based on the fraction of the work performed.

Jacob and James both worked on the gencode. Jacob did error handling while James did the testing.

4. Provide two examples of how you tested the correctness of your code. These examples should be
different from the ones provided with this assignment.

Multiple Declared Functions
int foo(){}
int foo(){}

Multiple Declared Variables 
int main(){
   int i;
   int i;
}

Function calls
int foo(){
}

int main(){
   foo();
}

5. Provide details about known bugs in your program and how you debugged your code. Also, provide
the name of the tool you used for debugging your code.

We were getting bugs with the gencode, emit problems and stack frame. We debugged with spim, zeus server, and the vscode debugger.


6. Describe the calling convention of functions in detail.

$a0 is used to store a function parameter (if there is one)

At start of function we store old $fp at 0($sp) and we store $ra in -4($sp)
We then set the new frame pointer to the old $sp and then we move the $sp down
by (the number of local variables plus 3) * 4 (adding 3 because we will have a return value,
a return address and optionally a parameter for the function.)

$v0 is used to store the return result (if there is one)

As the end of a function we restore the stack pointer and frame pointer to what they
were before the function. We load the return address and return.

Register Name Number Usage
$zero 	0 	Constant 0
$at 		1 	reserved for assembler
$v0-$v1 	2-3 	expression evaluation/results
$a0-$a4 	4-7	arguments
$t0-$t7 	8-15 	temporary registers
$s0-$s7 	16-23 saved registers
$t8-$t9 	24-25 temporary registers
$k0-$k1 	26-27 reserved for kernel
$gp 		28 	global pointer
$sp 		29 	stack pointer
$fp 		30 	frame pointer
$ra 		31	return address

7. Provide instructions for running your code. Clearly specify all the steps from make, to running
the generated code on SPIM.

Commands:

make
./obj/mcc -o output.asm <fileName.mC>     <--- dont include "<" and ">"
spim                                      <--- this should load spim up
load "output.asm"
run


