simpsh.c

Trey Ballengee and Marshall Royce

Our project takes a line of arguments and create tokens.  We used strtok to create tokens 
that were separated by a space.  We then used a while loop to save the tokens that we created
 into our token struct, which consisted of a name and value character pointer.  We created
  a checkSyntax function that, given the tokens we created and a similarly structured memory
   structure, would check the syntax of the user’s command and make sure it fits all necessary
    requirements for each function. Then, we created an execFunction that would do a switch 
    statement on the first or second token and carry out the appropriate command. On errors,
     they are reported, and on success, the user is taken back to PS command line. If desired,
      our shell can redirect stdin and stdout for the bang(!) function.

Our limitations for our project were:
1.	In order to substitute a token for a variable name using the $ operator, the variable name
    must be the whole token. Our program cannot support a substitution such as this: foo = $bar/p4,
    however, normal substitutions such as foo = $bar are supported.
2.	When using the bang(!) command, the argument cmd cannot be relaitve or local (begin with ./
    or /) because in order to pass the correct path to execve, we could only do so by changing
    variable PATH to “/bin/:/usr/bin/”
3.	Lv does not support arguments (no output redirection)
