# 19fa-project-0-koktengri
README.md file for penn-shredder by Kaan Erdogmus
pennkey/eniac username: kaanberk
src files:
penn-shredder.c (main)
ioutil.h (I/O utility function)
ioutil.c
strutil.h (String utility functions, Extra Credit)
strutil.c

To compile, run "make all" in top level directory.

Program can correctly receive and parse input, can fork and execute processes with optional parameters, correctly responds to ^C but not to ^D. penn-shredder.c contains main that runs the loop, it calls functions from ioutil.h to get and parse input and then forks and runs processes. ioutil.c calls functions from strutil for string manipulation (atoi).


# 19fa-project-1-group-65
