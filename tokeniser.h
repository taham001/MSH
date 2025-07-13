


#include "imports.h"

typedef enum {
    TOKEN_WORD,
    TOKEN_OPERATOR,
} TokenType;


typedef struct {
    TokenType type;
    char *text;
} Token;


extern char *operators[];


void free_tokens(Token **tokens, int count);
int check_if_operator(char *tok, char *out);
Token **Tokenise(char *input, int size, int *index);


