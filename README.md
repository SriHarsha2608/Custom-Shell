# Custom Shell

A custom Linux shell implemented in C. This project simulates a standard UNIX shell environment with custom implementations of various built-in commands alongside executing standard system binaries. It is designed to be a lightweight, robust, and extensible shell environment.

## Features

### Built-in Commands
- **Custom Prompt**: Displays system information, current user, and dynamically updates the current working directory, relative to the initial home directory where the shell was launched.
- **`hop` (Change Directory)**: A custom implementation of `cd`. Allows navigating the file system with support for absolute and relative paths, as well as `~` for the home directory and `-` for the previous directory.
- **`reveal` (List Contents)**: A custom implementation of `ls`. Lists files and directories with support for flags like `-l` (detailed info) and `-a` (hidden files).
- **`log` (Command History)**: Maintains a persistent history of commands executed across sessions. You can view the history and execute previous commands easily.
- **`jobs` (Job Control)**: Lists currently running or stopped background jobs controlled by the shell natively.
- **`ping` (Signal Management)**: Sends standard UNIX signals to running processes or jobs (similar to the `kill` command).

### Process & Execution Management
- **Foreground and Background Execution**: Run commands seamlessly in the foreground or execute them asynchronously in the background by appending `&` to your commands.
- **Process Tracking**: Automatically tracks background processes and prints status notifications when they finish or are terminated.
- **System Commands**: Transparently falls back to `execvp` to run standard executables available in the system `$PATH`.

### Signal Handling
- **`SIGINT` (Ctrl+C)**: Interrupts the foreground process but keeps the shell running.
- **`SIGTSTP` (Ctrl+Z)**: Stops the foreground process and sends it to the background, allowing the shell to prompt for new commands.
- **`EOF` (Ctrl+D)**: Gracefully exits the shell, ensuring background processes are properly dealt with.

## Project Structure & Architecture

The project is modularly structured, separating concerns into dedicated files:
- `src/`: Contains all the C source files orchestrating the shell's logic:
  - `input.c` / `prompt.c`: Deals with fetching user input and displaying the contextual prompt.
  - `tokenizer.c` / `parser.c`: Handles lexical analysis and parses complex command lines into executable arguments.
  - `executor.c`: The core engine that forks processes, manages built-ins, and handles system process execution.
  - `jobs.c` / `signals.c`: Manages background processes and handles system signals gracefully.
  - `hop.c`, `reveal.c`, `log.c`, `ping.c`: Individual implementations of the built-in commands.
- `include/`: Contains the header files (`.h`) exposing the interfaces for the different modules.
- `Makefile`: Build instructions for easy compilation using `gcc`.

## Prerequisites
- A UNIX-like operating system (Linux, macOS, WSL)
- `gcc` (GNU Compiler Collection)
- `make`

## Build and Run

To compile the shell, make sure you are in the project root directory and run:

1. **Compile**:
   ```bash
   make
   ```

2. **Run**:
   ```bash
   ./shell.out
   ```

3. **Clean build files**:
   ```bash
   make clean
   ```

## Usage Examples

Once inside the shell, you can use the custom built-ins alongside standard Unix tools:

```bash
# Navigate directories
<user@system:~> hop /tmp
<user@system:/tmp> hop -

# List files with details (including hidden files)
<user@system:~> reveal -a -l

# Run a process in the background
<user@system:~> sleep 10 &
[1] 12345

# Check background jobs
<user@system:~> jobs
[1] Running sleep 10 [12345]

# Send a signal to a process
<user@system:~> ping 9 12345

# Review history and execute a previous command
<user@system:~> log
<user@system:~> log execute 1
```