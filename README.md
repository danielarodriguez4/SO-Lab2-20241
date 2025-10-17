# Laboratory: Processes and Unix Shell

> A comprehensive report documenting the development of a Unix shell implementation in C, focusing on process management and systems programming concepts.

**Team Members:**
- Estiven Ospina González
- María Daniela Rodríguez Chacón

---

## Introduction

This laboratory reinforces theoretical and practical concepts related to process management in Unix/Linux operating systems using C programming language. The main deliverable is a functional Unix shell that executes commands and manages processes similarly to conventional Unix shells like bash.

---

## Objectives

### General Objective
Develop a functional Unix shell in C that implements execution of internal and external commands, demonstrating mastery of the process API in Unix/Linux systems.

### Specific Objectives
- Understand and correctly apply the process API (fork, exec, wait)
- Implement command-line argument handling
- Develop file handling skills in C
- Create a functional and user-friendly command-line interface
- Complete exercise 7 from the programming problems section

---

## Concepts Studied

### Command-Line Argument Handling
Processing arguments passed through `argc` and `argv`, enabling dynamic command and parameter acceptance.

### File Handling in C
Implementation using standard C functions: `fopen()`, `fclose()`, `fread()`, `fwrite()`, and others.

### Linux Console Philosophy
Understanding how traditional shells (bash, sh) function to replicate their core functionality in a reduced form.

---

## Project Components

### Unix Shell Implementation

**Main Features:**
- Read commands from standard input
- Parse commands and arguments
- Execute internal and external commands
- Manage child processes
- Basic error handling

## Development & Testing

### Development Workflow

1. **Repository Setup**
   ```bash
   git clone https://github.com/danielarodriguez4/SO-Lab2-20241.git
   ```

2. **Collaboration**
   - Forked the base repository
   - Added team members as collaborators
   - Created local copies for collaborative development
   - Made regular commits throughout development

3. **Testing Protocol**
   - Ran provided test suite continuously
   - Used test results as progress indicators
   - Validated against expected behavior

### Usage Example  

```bash
cd SO-Lab2-20241
clang -Wall -Wextra -std=c11 -o wish wish.c
cd enunciado && cp ../wish . && ./test-wish.sh #to run tests
./wish #to start the new terminal
```
---

## Learnings

### Acquired Knowledge
- Deep understanding of Unix/Linux process API
- Practical skills for OS-level programming
- Advanced C programming techniques
- System process management

### Developed Competencies
- Process management and multitasking
- Low-level C programming
- Debugging and troubleshooting
- Collaborative development with Git
---
