<h2>MiniShell</h2>

MiniShell is a lightweight Unix-like shell written in C. It supports built-in commands, external command execution, job control, signal handling, and multi-pipe operations using Linux system calls, pipes, and signals.

<h3>Features</h3>

Built-in commands: cd, pwd, echo, exit, jobs, fg, bg

Execute external programs using fork() and execvp()

Foreground & background job management

Signal handling for Ctrl+C and Ctrl+Z

Multi-pipe support (cmd1 | cmd2 | cmd3)

Prevents zombie/orphan processes

<h3>Build & Run</h3>
gcc *.c -o minishell.out<br>
./minishell.out

<h3>Example Usage</h3>
minishell> ls -l | grep txt<br>
minishell> sleep 5 &<br>
minishell> jobs<br>
minishell> fg %1<br>

<h3>Technologies Used</h3>
C Programming<br>
Linux System Calls<br>
Pipes & Signals<br>
Command Line Interface<br>
