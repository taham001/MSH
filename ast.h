

#include "tokeniser.h"  


typedef enum {
    AST_command,
    AST_pipe,
    AST_redirect,
    AST_and,
    AST_or,
    AST_sequence,
    AST_background,
} NodeType;


typedef struct Node {
    NodeType type;
    struct Node *left;
    struct Node *right;
    char **command;
    char *filename;
    int redir_type;
    int process;
} Node;

void free_ast(Node *node);
void print_ast(Node *node, int depth);
int count_processes(Node *node);

Node *parse_command(Token **tokens, int *pos, int count);
Node *parse_redirect(Token **tokens, int *pos, int count);
Node *parse_pipeline(Token **tokens, int *pos, int count);
Node *parse_and_or(Token **tokens, int *pos, int count);
Node *parse_background(Token **tokens, int *pos, int count);
Node *parse_sequence(Token **tokens, int *pos, int count);


