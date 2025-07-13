
#include "tokeniser.h"



char *operators[] = {"&&", "||", ">>", "<<", ">", "<", "|", ";", "&", NULL};

void free_tokens(Token **tokens, int count) {
    for (int i = 0; i < count; i++) {
        if(tokens[i]==NULL) continue;
        free(tokens[i]->text);
        free(tokens[i]);
    }
    free(tokens);
}


int check_if_operator(char *tok, char *out)
{

    for (int i = 0; operators[i] != NULL; i++)
    {
        int len = str_len(operators[i]);
        if (compare(operators[i], tok, len) == 1)
        {

            for (int j = 0; j < len; j++)
                out[j] = operators[i][j];
            out[len] = '\0';
            return len;
        }
    }
    return 0;
}

Token **Tokenise(char *input, int size, int *index)
{

    Token **tokens = malloc(sizeof(Token *) * 100);
    if (!tokens)
    {
        perror("malloc failed");
        return NULL;
    }

    int doubleQuotes = 0;
    int singleQuotes = 0;
    int buf_index = 0;

    char buffer[100];

    for (int i = 0; i < size; i++)
    {
        if (input[i] == '"' && !singleQuotes)
        {
            doubleQuotes = !doubleQuotes;
            continue;
        }
        if (input[i] == '\'' && !doubleQuotes)
        {
            singleQuotes = !singleQuotes;
            continue;
        }

        if (!doubleQuotes && !singleQuotes)
        {

            char op[3] = {0};
            int len = check_if_operator(input + i, op);

            if (len > 0)
            {
                if (buf_index > 0)
                {
                    buffer[buf_index] = '\0';
                    Token *word = malloc(sizeof(Token));
                    word->text = copy_str(buffer);
                    word->type = TOKEN_WORD;
                    tokens[(*index)++] = word;
                    buf_index = 0;
                }
                Token *word = malloc(sizeof(Token));
                word->text = copy_str(op);
                word->type = TOKEN_OPERATOR;
                tokens[(*index)++] = word;
                i += len - 1;
                continue;
            }
        }

        if (input[i] == ' ' && !doubleQuotes && !singleQuotes)
        {
            if (buf_index > 0)
            {
                buffer[buf_index] = '\0';
                char *t = copy_str(buffer);

                Token *token = malloc(sizeof(Token));
                token->text = t;
                token->type = TOKEN_WORD;
                tokens[(*index)++] = token;

                buf_index = 0;
            }
            continue;
        }

        buffer[buf_index++] = input[i];
        if (buf_index >= 100)
        {
            fprintf(stderr, "Token too long\n");
            for (int j = 0; j < *index; j++)
            {
                free(tokens[j]->text);
                free(tokens[j]);
            }
            free(tokens);
            return NULL;
        }
    }

    if (buf_index > 0)
    {
        buffer[buf_index] = '\0';
        char *t = copy_str(buffer);

        Token *token = malloc(sizeof(Token));
        token->text = t;
        token->type = TOKEN_WORD;
        tokens[(*index)++] = token;

        buf_index = 0;
    }

    if (doubleQuotes || singleQuotes)
    {
        fprintf(stderr, "Error: unmatched quotes\n");
      

        return NULL;
    }

    tokens[(*index)] = NULL;
    // for (int i = 0; i < *index; i++)
    // {
    //     printf("%s %d\n", tokens[i]->text, tokens[i]->type);
    // }

    return tokens;
}
