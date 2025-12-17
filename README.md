# Nut-Shell

A **shell** is a command-line interface (CLI) that allows users to interact with an operating system by:

- **Accepting commands** from users
- **Interpreting/parsing** those commands
- **Executing** the requested operations
- **Returning results** to the user
## Shell Components

1. **Command Line Interpreter (CLI)**
    - Parser (front-end): Reads and parses user commands
    - Executor (back-end): Executes parsed commands
2. **Abstract Syntax Tree (AST)**
    - High-level representation of commands
    - Parser creates AST → Executor reads AST
3. **REPL Loop**
    - Read-Eval-Print-Loop
    - Interactive mode shell operation
4. **Symbol Table** (Part II)
    - Stores variables, values, attributes
5. **History Facility**
    - Command history, editing, re-execution
6. **Builtin Utilities**
    - Commands like `cd`, `fg`, `bg` implemented within shell

**Tokenizer** - Break commands into arguments

**Parser** - Create AST from tokens

**Executor** - Execute parsed commands using `fork()` + `exec()`

**Simple command execution** (not just echoing)
### SHELL PROMPT TYPES

**PS1** (`$`): Primary prompt, shows when ready for command
**PS2** (`>`): Secondary prompt, shows during multi-line input

```bash
PS1: "$ "       (Primary, command ready)
PS2: "> "       (Continuation, multi-line)
PS3: "#? "      (Used with 'select' command)
PS4: "+ "       (Debug mode prefix)
```

Till here progress:

```bash
$ ./shell
$ hi
hi

$ hi \
> hello
hi hello
```
## Implementing Lexical Scanner

To be able to execute simple command shell needs to perform following steps:

- Scan input, one character at time, to find next token this process is called **lexical scanning** and part of shell that performs this task is called **lexical scanner**.
- Extract input token which is called **tokenizing input**.
- Parse the tokens and create and Abstract Syntax Tree (AST). The part of shell responsible for it is known as **parser**.
- Execute the AST which is job of the **executor**.
### Scanning Input

A **delimiter character** is one that marks the end of a token (and possibly the beginning of another token). Typically, delimiters are whitespace characters (space, tab, newline), but can also include other characters, such as `;` and `&`.

- Retrieve the next character from input.
- Return the last character we've read back to input.
- Lookahead (or peek) to check the next character, without actually retrieving it.
- Skip over whitespace characters.

```bash
Input:  "ls -l /home"
Scanner reads: l → s → space → - → l → space → / → h → o → m → e
Groups into tokens: ["ls", "-l", "/home"]
```

`source.h`

```c
#ifndef SOURCE_H
#define SOURCE_H

/*
 * SOURCE.H - Input Scanner Abstraction Layer
 * 
 * This header defines the interface for scanning input character-by-character.
 * It provides a clean abstraction over raw string input, enabling features
 * like lookahead, backtracking, and position tracking.
 */

/* 
 * EOF - End Of File marker
 * Value: -1 (negative one)
 * Why -1: ASCII characters range from 0-255, so -1 cannot be confused
 *         with any valid character. Standard C library also uses -1 for EOF.
 */
#define EOF             (-1)

/* 
 * ERRCHAR - Error Character indicator  
 * Value: 0 (null character)
 * Purpose: Return value when an error occurs during character reading
 * Why 0: In C strings, 0 marks the end, making it a safe error indicator
 */
#define ERRCHAR         ( 0)

/*
 * INIT_SRC_POS - Initial source position
 * Value: -2 (negative two)
 * Why -2: Creates a clear state machine:
 *   -2 = Before reading starts (initial state)
 *   -1 = EOF/End of input reached
 *    0 = At first character (index 0 in buffer)
 *    n = At nth character
 */
#define INIT_SRC_POS    (-2)

/*
 * struct source_s - Input source abstraction structure
 * 
 * This structure encapsulates all information needed to scan input:
 * - The actual input text
 * - Its total size  
 * - Current reading position
 * 
 * Benefits:
 * 1. Single parameter passing instead of (string, position, length)
 * 2. Easy position saving/restoration
 * 3. Clean separation between data and operations
 */
struct source_s
{   
    char *buffer;       /* Pointer to input text (malloc'd string from read_cmd()) */
    long bufsize;       /* Total size of input text (including null terminator) */
    long  curpos;       /* Current absolute position in source (not array index!) */
};

/* 
 * next_char() - Get the next character from input
 * @src: Pointer to source structure
 * Returns: Next character, or EOF if at end, or ERRCHAR on error
 * Side effect: Advances curpos by one position
 * 
 * Usage example:
 *   char c = next_char(&src);  // Get 'l' from "ls -l"
 */
char next_char(struct source_s *src);

/* 
 * unget_char() - Put back the last read character (backtracking)
 * @src: Pointer to source structure
 * Returns: void
 * Side effect: Decrements curpos by one
 * 
 * Why needed:
 * - Sometimes we read too far (e.g., reading "10+" we consume '0' before seeing '+')
 * - Error recovery: Try parsing, backtrack if fails
 * 
 * Example:
 *   char c = next_char(src);  // Gets '='
 *   if(peek_char(src) != '=') {
 *       unget_char(src);      // Put '=' back, it's not "=="
 *   }
 */
void unget_char(struct source_s *src);

/* 
 * peek_char() - Look at next character without consuming it
 * @src: Pointer to source structure
 * Returns: Next character (or EOF/ERRCHAR), DOES NOT advance curpos
 * 
 * Critical for parsing decisions:
 * - "==" vs "=" (need to see second '=')
 * - "&&" vs "&" (need to see second '&')
 * - Quoted strings (need to find matching quote)
 * 
 * Example:
 *   if(peek_char(src) == '"') {
 *       // Start reading quoted string
 *   }
 */
char peek_char(struct source_s *src);

/* 
 * skip_white_spaces() - Skip over whitespace characters
 * @src: Pointer to source structure
 * Returns: void
 * Side effect: Advances curpos past consecutive whitespace
 * 
 * What counts as whitespace:
 * - Space (' ')
 * - Tab ('\t')
 * - Newline ('\n')
 * - Carriage return ('\r')
 * 
 * Example:
 *   Input: "   ls -l"
 *   After skip_white_spaces(): curpos points to 'l'
 *   Whitespace is skipped, next token starts at 'l'
 */
void skip_white_spaces(struct source_s *src);

#endif /* SOURCE_H */
```

After creating these functions in `source.c` we can have our scanner's functions in place to extract input tokens in but first define a new structure which we will use to represent our tokens `scanner.h`.

```c
#ifndef SCANNER_H
#define SCANNER_H

struct token_s
{
    struct source_s *src;       /* source of input */
    int    text_len;            /* length of token text */
    char   *text;               /* token text */
};

/* the special EOF token, which indicates the end of input */
extern struct token_s eof_token;

struct token_s *tokenize(struct source_s *src);
void free_token(struct token_s *tok);

#endif
```

`extern` in C means this variable/function exists **SOMEWHERE ELSE**, not in this file."

```c
/* Just DECLARES: "This exists somewhere" */
extern int global_counter;  /* NO memory allocated! Just a promise */

void print_counter(void) {
    printf("Counter: %d\n", global_counter);  /* Uses shared variable */
}
```

---



