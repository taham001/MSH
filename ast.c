
#include "ast.h"


void free_ast(Node *node) {
    if (!node) return;

    free_ast(node->left);
    free_ast(node->right);

    if (node->command) {
        
        free(node->command);
    }
    free(node);
}
void print_ast(Node *node, int depth)
{
    if (!node)
        return;
   
    
    for (int i = 0; i < depth; i++)
        printf("  ");

    switch (node->type)
    {
    case AST_command:
        printf("COMMAND: ");
        for (int i = 0; node->command[i]; i++)
        {
            printf("%s ", node->command[i]);
        }
        printf("\n");
        break;

    case AST_pipe:
        printf("PIPE\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
        break;

    case AST_redirect:
        printf("REDIRECT (type %d): %s\n", node->redir_type, node->filename);
        print_ast(node->left, depth + 1);
        break;

    case AST_and:
        printf("AND (&&)\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
        break;

    case AST_or:
        printf("OR (||)\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
        break;

    case AST_sequence:
        printf("SEQUENCE (;)\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
        break;
    case AST_background:
        printf("BACKGROUND (&) %d\n", node->process);
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
        break;
    }
}

int count_processes(Node *node)
{
    if (node == NULL)
        return 0;
    if (node->type == AST_command)
        return 1;
    return count_processes(node->left) + count_processes(node->right);
}

Node *parse_command(Token **tokens, int *pos, int count)
{
    if (*pos >= count || tokens[*pos] == NULL)
    {
        
        return NULL;
    }

    if (tokens[*pos]->type == TOKEN_OPERATOR)
    {
        fprintf(stderr, "Syntax error: unexpected operator '%s' at position %d (expected command)\n",
                tokens[*pos]->text, *pos);
        return NULL; 
    }
    int start = *pos;
    while (*pos < count && tokens[*pos]->type == TOKEN_WORD)
    {
        (*pos)++;
    }

    int n = *pos - start;

    if (n == 0)
        return NULL;

    Node *node = malloc(sizeof(Node));
    node->type = AST_command;
    node->left = node->right = NULL;
    node->filename = NULL;
    node->redir_type = 0;
    node->process = 0;

    node->command = malloc(sizeof(char *) * (n + 1));
    for (int i = 0; i < n; i++)
    {
        node->command[i] = tokens[start + i]->text;
    }
    node->command[n] = NULL;
    return node;
}

Node *parse_redirect(Token **tokens, int *pos, int count)
{
    Node *node = parse_command(tokens, pos, count);
    if (!node){
       
        return NULL;
    }

    while (*pos < count &&
           tokens[*pos]->type == TOKEN_OPERATOR &&
           (compare(tokens[*pos]->text, ">",1) == 1 ||
            compare(tokens[*pos]->text, ">>",2) == 1 ||
            compare(tokens[*pos]->text, "<",1) == 1 || compare(tokens[*pos]->text, "<<",2) == 1))
    {

        int type = 0;
      
         if (compare(tokens[*pos]->text, ">>",2))
            type = 2;
        else  if (compare(tokens[*pos]->text, ">",1))
            type = 1;
         else if (compare(tokens[*pos]->text, "<<",2))
            type = 4;
        else if (compare(tokens[*pos]->text, "<",1))
            type = 3;
       

        (*pos)++; 

        if (*pos >= count || tokens[*pos]->type != TOKEN_WORD)
        {
            free_ast(node);
            fprintf(stderr, "Syntax error: expected filename after redirection\n");
            return NULL;
        }

        char *filename = tokens[*pos]->text;
        (*pos)++;

        Node *redir = malloc(sizeof(Node));
        redir->type = AST_redirect;
        redir->redir_type = type;
        redir->filename = filename;
        redir->left = node;
        redir->right = NULL;
        redir->command = NULL;
        node->process = 0;

        node = redir;
    }

    return node;
}

Node *parse_pipeline(Token **tokens, int *pos, int count)
{
    Node *left = parse_redirect(tokens, pos, count);
    if (!left){
        return NULL;
    }

    while (*pos < count &&
           tokens[*pos]->type == TOKEN_OPERATOR &&
           compare(tokens[*pos]->text, "|", 1) == 1 && compare(tokens[*pos]->text, "||", 2) != 1)
    {
        (*pos)++;

        Node *right = parse_redirect(tokens, pos, count);
        if (!right)
        {
            free_ast(left);
            fprintf(stderr, "Syntax error: expected command after '|'\n");
            return NULL;
        }

        Node *pipe_node = malloc(sizeof(Node));
        pipe_node->type = AST_pipe;
        pipe_node->left = left;
        pipe_node->right = right;
        pipe_node->command = NULL;
        pipe_node->filename = NULL;
        pipe_node->redir_type = 0;
        pipe_node->process = 0;

        left = pipe_node;
    }

    return left;
}
Node *parse_and_or(Token **tokens, int *pos, int count)
{
    Node *left = parse_pipeline(tokens, pos, count);
    if (!left)
        return NULL;

    while (*pos < count &&
           tokens[*pos]->type == TOKEN_OPERATOR &&
           (compare(tokens[*pos]->text, "&&", 2) == 1 || compare(tokens[*pos]->text, "||", 2) == 1))
    {

        NodeType op_type = (compare(tokens[*pos]->text, "&&", 2) == 1) ? AST_and : AST_or;
        (*pos)++;

        Node *right = parse_pipeline(tokens, pos, count);
        if (!right)
        {
            free_ast(left);
            fprintf(stderr, "Syntax error: expected command after '&&' or '||'\n");
            return NULL;
        }

        Node *and_or = malloc(sizeof(Node));
        and_or->type = op_type;
        and_or->left = left;
        and_or->right = right;
        and_or->command = NULL;
        and_or->filename = NULL;
        and_or->redir_type = 0;
        and_or->process = 0;

        left = and_or;
    }

    return left;
}

Node *parse_background(Token **tokens, int *pos, int count)
{

    Node *left = parse_and_or(tokens, pos, count);
    if (!left)
        return NULL;
    while (*pos < count && tokens[*pos] && compare(tokens[*pos]->text, "&", 1) == 1)
    {

        (*pos)++; 
        Node *right = parse_background(tokens, pos, count);
        Node *bg = malloc(sizeof(Node));
        int total_processes = count_processes(left);
        bg->type = AST_background;
        bg->left = left;
        bg->right = right;
        bg->command = NULL;
        bg->filename = NULL;
        bg->redir_type = 0;
        bg->process = total_processes;

        left = bg;
    }

    return left;
}
Node *parse_sequence(Token **tokens, int *pos, int count)
{
    Node *left = parse_background(tokens, pos, count);
    if (!left)
        return NULL;
    while (*pos < count && tokens[*pos]->type == TOKEN_OPERATOR && compare(tokens[*pos]->text, ";", 1) == 1)
    {
        (*pos)++;
        Node *right = parse_background(tokens, pos, count);

        if (!right)
        {
            free_ast(left);
            printf("Syntax error: Expected another sequence of commands");
            return NULL;
        }
        Node *seq = malloc(sizeof(Node));
        seq->type = AST_sequence;
        seq->left = left;
        seq->right = right;
        seq->command = NULL;
        seq->filename = NULL;
        seq->redir_type = 0;

        left = seq;
    }

    return left;
}
