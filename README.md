# MSH - A POSIX-Compatible Shell in C

**MSH** is a custom shell implementation written in C, providing a basic POSIX-compatible command-line environment with:

- Job control  
- Redirections  
- Pipelines  
- Background execution  
- Self-implemented versions of common commands like `ls`, `pwd`, `mv`, `cp`, `rm`, `echo`, `touch`  

---

## ✅ Features

### POSIX-Compatible Parsing
- Implements a recursive-descent parser respecting operator precedence.

### Command Execution via AST
- Parses input into an Abstract Syntax Tree (AST) and executes it accordingly.

### Built-in Commands
- `cd`
- `exit`
- `mkdir`
- `jobs`
- `true`
- `false`

### Redirection Support
- `>` – Output redirection  
- `>>` – Append output  
- `<` – Input redirection  
- `<<` – Heredoc

### Pipelines
- Supports Unix-style pipelines like:
  ```sh
  ls | grep txt | wc -l
### Sequence
- Running multiple commands in one line via ;
 ```sh
echo 1 ; echo2 ; pwd
```

✅ **Background Execution**  
Supports `&` to run commands in the background.

✅ **Job Control**
- Tracks background jobs by PGID (process group ID)
- Shows active/done jobs via `jobs` command
- Handles `SIGCHLD` and reaps background processes

---

## 🔧 Building the Shell

Ensure `gcc` is installed.

### 1. Place source files in a folder named `MSH`.

### 2. Move the folder to `/bin`:
```sh
sudo mv /path/to/MSH /bin
```
### 3. Navigate to MSH
```sh
cd /bin/MSH
```
### 4.Compile using make 
```sh
make
```
### 5. Run the shell
```sh
./msh


```
# Requirements
- POSIX-compliant system (Linux, WSL)

- GCC (for compilation)



# Notes
- Shell commands are resolved from /bin/MSH/ via execv, not PATH.

- Heredoc (<<) reads until the delimiter is matched exactly.

- Invalid or malformed ASTs are rejected during parsing with helpful error messages.

  




