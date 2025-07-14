#include "imports.h"
#include "ast.h"

#define COMMAND_BUFFER_SIZE 1024
#define MAX_BG_PIDS 1024
#define MAX_PIPE_STAGES 64
#define MAX_JOBS 128

char buffer[COMMAND_BUFFER_SIZE];
pid_t background_pids[MAX_BG_PIDS];
int background_pid_count = 0;

typedef struct
{
    pid_t pid;
    pid_t pgid;

} process_mapper;

typedef struct
job{
    pid_t pgid;
    int process;
    int status;

} job;

job job_table[MAX_JOBS];
static int job_count = 0;

process_mapper map[MAX_JOBS];
static int map_count = 0;
void print_mapping()
{
    for (int i = 0; i < map_count; i++)
    {
        printf("Process id: %d  Group id: %d\n", map[i].pid, map[i].pgid);
    }
}
int add_mapping(pid_t pgid, pid_t pid)
{
    if (map_count < MAX_JOBS)
    {

        for (int i = 0; i < map_count; i++)
        {
            if (map[i].pid == pid)
                return 0;
        }

        map[map_count].pgid = pgid;

        map[map_count].pid = pid;
        map_count++;
        return 1;
    }
    return 0;
}
void add_job(pid_t pgid, int process)
{
    if (job_count >= MAX_JOBS)
    {
        fprintf(stderr, "Buffer filled try later");
        return;
    }

    for (int i = 0; i < job_count; i++)
    {
        if (job_table[i].pgid == pgid)
        {
            job_table[i].process++;
            return;
        }
    }
    job_table[job_count].pgid = pgid;
    job_table[job_count].status = 1;
    job_table[job_count].process = process;
    job_count++;
}

int mark_job_done(pid_t pgid)
{
    for (int i = 0; i < job_count; i++)
    {
        if (job_table[i].pgid == pgid)
        {
            if (job_table[i].process <= 0)
            {
                job_table[i].status = 0;
                return 1;
            }

            job_table[i].process = job_table[i].process - 1;
            if (job_table[i].process <= 0)
            {
                job_table[i].status = 0;
                return 1;
            }
        }
    }
    return 0;
}

void print_jobs()
{
    for (int i = 0; i < job_count; i++)
    {
        if (job_table[i].status)

        {

            printf("[%d]  pgid:%d Running    \n", i + 1, job_table[i].pgid);
        }
        else
        {
            printf("[%d]   pgid:%d Done   \n", i + 1, job_table[i].pgid);
        }
    }
}
void flatten_pipeline(Node *node, Node **stages, int *count)
{
    if (!node)
        return;
    if (node->type == AST_pipe)
    {
        flatten_pipeline(node->left, stages, count);
        flatten_pipeline(node->right, stages, count);
    }
    else
    {
        stages[(*count)++] = node;
    }
}

void track_background_pid(pid_t pid)
{
    if (background_pid_count < MAX_BG_PIDS)
    {
        for (int i = 0; i < background_pid_count; i++)
        {
            if (background_pids[i] == pid)
                return;
        }
        background_pids[background_pid_count++] = pid;
    }
}
void print_background()
{
    for (int i = 0; i < background_pid_count; i++)
    {
        printf("[%d]:", background_pids[i]);
    }
    printf("\n");
}

int is_background_pid(pid_t pid)
{
    for (int i = 0; i < background_pid_count; i++)
    {
        if (background_pids[i] == pid)
            return 1;
    }
    return 0;
}

static void exec_leaf(Node *node, pid_t pgid, int is_background)
{
    if (!node->command || !node->command[0])
    {
        fprintf(stderr, "empty command\n");
        _exit(1);
    }

    if (node->type == AST_command)
    {
        char *path = append_str("/bin/MSH/", node->command[0]);

        execv(path, node->command);
        perror("execv");
        _exit(1);
    }
}

void handle_sigchld(int sig)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        pid_t pgid = -1;
        int poss = 0;

        for (int i = 0; i < background_pid_count; i++)
        {
            if (background_pids[i] == pid)
            {
                poss = 1;
                break;
            }
        }
        if (poss == 0)
            continue;
        for (int i = 0; i < map_count; i++)
        {
            if (map[i].pid == pid)
            {
                pgid = map[i].pgid;
                break;
            }
        }

        if (mark_job_done(pgid) == 0)
            continue;

        printf("[REAP] PID=%d PGID=%d STATUS=%d\n",
               pid, pgid, WEXITSTATUS(status));
    }

    if (pid == -1 && errno != ECHILD)
    {
        perror("waitpid");
    }
}

int execute_command(Node *node, pid_t pgid, int is_background)
{
    if (!node || !node->command || !node->command[0])
        return 1;

    if (compare(node->command[0], "exit", 4) == 1)
        exit(0);

    if (compare(node->command[0], "cd", 2) == 1)
    {
        if (node->command[1] == NULL || chdir(node->command[1]) != 0)
        {
            perror("cd failed");
            return 1;
        }
        return 0;
    }
    if (compare(node->command[0], "mkdir", 5) == 1)
    {
        if (mkdir(node->command[1], 0755) == -1)
        {
            perror("mkdir failed");
            return 1;
        }
        else
        {
            write(1, "Directory created.\n", 20);
        }
        return 0;
    }
    if (compare(node->command[0], "jobs", 4) == 1)
    {
        print_jobs();
        return 0;
    }
    if (compare(node->command[0], "true", 4) == 1)
    {
        return 0;
    }
    if (compare(node->command[0], "false", 5) == 1)
    {
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        signal(SIGTSTP, SIG_DFL);  
signal(SIGINT,  SIG_DFL); 
signal(SIGTTIN, SIG_DFL);  
signal(SIGTTOU, SIG_DFL);
        if (pgid == 0)
            pgid = getpid();
        setpgid(0, pgid);
        if (!is_background) {
    tcsetpgrp(STDIN_FILENO, pgid); 
}
        char *path = append_str("/bin/MSH/", node->command[0]);
        execv(path, node->command);
        perror("exec failed");
        exit(1);
    }
    if (pgid == 0)
        pgid = pid;
    setpgid(pid, pgid);
    if (is_background)
    {
        track_background_pid(pid);
        add_mapping(pgid, pid);
        add_job(pgid, 1);
    }
    int status;
    if (!is_background)
    {
        tcsetpgrp(STDIN_FILENO, pgid);            
    waitpid(pid, &status, WUNTRACED);        
    tcsetpgrp(STDIN_FILENO, getpgrp());
 if (WIFSTOPPED(status)) {
    
    track_background_pid(pid);
    add_mapping(pgid, pid);
    add_job(pgid, 1);
    printf("\n[Stopped] pgid=%d moved to background\n", pgid);
    return 0;
}
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
    return 0;
}

int execute_ast(Node *node, pid_t pgid, int is_background)
{
    if (!node)
        return 1;

    switch (node->type)
    {
    case AST_command:

        return execute_command(node, pgid, is_background);

    case AST_redirect:
    {
        int fd;
        char b[1024];
        int i = 0;
        if (node->redir_type == 1)
        {
            fd = open(node->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        else if (node->redir_type == 2)
        {
            fd = open(node->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else if (node->redir_type == 3)
        {
            fd = open(node->filename, O_RDONLY);
        }
        else if (node->redir_type == 4)
        {
            char *delim = node->filename;
          
            int len = str_len(delim);
        
            while (1) {
    write(1, "> ", 2);

    int j = 0;
    char line[256];
    char ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        line[j++] = ch;
    }
    line[j] = '\0';

    if (compare(line, delim, len) == 1) {
        break;  
    }

    for (int k = 0; k < j; ++k) {
        b[i++] = line[k];
    }
    b[i++] = '\n';

    if (i >= 1020) break;
}
            b[i] = '\0';
        }
        else
        {
            fprintf(stderr, "Unknown redirection type\n");
            return 1;
        }

        if (fd < 0 && node->redir_type!=4)
        {
            perror("open failed");
            return 1;
        }




        pid_t pid = fork();
        if (pid == 0)
        {
            if (pgid == 0)
            {
                pgid = getpid();
            }
            setpgid(0, pgid);
            if (!is_background) {
    tcsetpgrp(STDIN_FILENO, pgid);
}

signal(SIGTSTP, SIG_DFL);  
signal(SIGINT,  SIG_DFL);  
signal(SIGTTIN, SIG_DFL); 
signal(SIGTTOU, SIG_DFL);
            if (node->redir_type == 3)
                dup2(fd, STDIN_FILENO);
            else if (node->redir_type == 4)
            {
                int piper[2];
                pipe(piper);
                
          
             
                write(piper[1], b, str_len(b));
                close(piper[1]);
                dup2(piper[0], STDIN_FILENO);
                close(piper[0]);
            }
            else
                dup2(fd, STDOUT_FILENO);
            close(fd);
            if (node->left->type == AST_command)
            {
                exec_leaf(node->left, pgid, is_background);
            }
            else
                exit(execute_ast(node->left, pgid, is_background));
        }
        else
        {
            if (pgid == 0)
            {
                pgid = pid;
            }
            setpgid(pid, pgid);
            if (is_background)
            {
                track_background_pid(pid);

                int try = add_mapping(pgid, pid);
                if (try)
                    add_job(pgid, 1);
            }
            int status;
            if (!is_background)
            {
               tcsetpgrp(STDIN_FILENO, pgid);
waitpid(pid,&status, WUNTRACED);
tcsetpgrp(STDIN_FILENO, getpgrp());
if (WIFSTOPPED(status)) {
    
    track_background_pid(pid);
     int try = add_mapping(pgid, pid);
                if (try)
                    add_job(pgid, 1);
    printf("\n[Stopped] pgid=%d moved to background\n", pgid);
    return 0;
}
                close(fd);
                return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
            }
            return 0;
        }
    }

    case AST_pipe:
    {
        Node *stages[MAX_PIPE_STAGES];
        int count = 0;
        flatten_pipeline(node, stages, &count);

        int pipefds[2 * (count - 1)];
        for (int i = 0; i < count - 1; i++)
        {
            if (pipe(pipefds + 2 * i) < 0)
            {
                perror("pipe");
                return 1;
            }
        }

        pid_t pids[MAX_PIPE_STAGES];
        pgid = 0;
        for (int i = 0; i < count; i++)
        {
            pid_t pid = fork();

            if (pid == 0)
            {
                if (pgid == 0)
                    pgid = getpid();
                setpgid(0, pgid);
                if (!is_background) {
    tcsetpgrp(STDIN_FILENO, getpid());
}

signal(SIGTSTP, SIG_DFL);
signal(SIGINT,  SIG_DFL);
signal(SIGTTIN, SIG_DFL);
signal(SIGTTOU, SIG_DFL);

                if (i > 0)
                {
                    dup2(pipefds[2 * (i - 1)], STDIN_FILENO);
                }

                if (i < count - 1)
                {
                    dup2(pipefds[2 * i + 1], STDOUT_FILENO);
                }

                for (int j = 0; j < 2 * (count - 1); j++)
                    close(pipefds[j]);

                if (stages[i]->type == AST_command)
                {
                    exec_leaf(stages[i], pgid, is_background);
                }
                else
                {
                    exit(execute_ast(stages[i], pgid, is_background));
                }
            }

            if (pgid == 0)
                pgid = pid;
            setpgid(pid, pgid);
            pids[i] = pid;
            if (is_background)
            {

                track_background_pid(pid);

                int try = add_mapping(pgid, pid);
                if (try)
                    add_job(pgid, 1);
            }
        }

        for (int j = 0; j < 2 * (count - 1); j++)
            close(pipefds[j]);

        if (is_background)
        {

            return 0;
        }
        else
        {
            int status;
            for (int i = 0; i < count; i++)
            {
                tcsetpgrp(STDIN_FILENO, pgid);
waitpid(pids[i],&status, WUNTRACED);
tcsetpgrp(STDIN_FILENO, getpgrp());
if (WIFSTOPPED(status)) {
  
    track_background_pid(pids[i]);
    int try = add_mapping(pgid, pids[i]);
                if (try)
                    add_job(pgid, 1);
    printf("\n[Stopped] pgid=%d moved to background\n", pgid);
    return 0;
}
               
            }
            return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }
    }

    case AST_and:
    {
        int status = execute_ast(node->left, pgid, is_background);
        if (status == 0)
            return execute_ast(node->right, pgid, is_background);
        return status;
    }

    case AST_or:
    {
        int status = execute_ast(node->left, pgid, is_background);
        if (status == 1)
            return execute_ast(node->right, pgid, is_background);
        return 0;
    }

    case AST_sequence:
    {
        execute_ast(node->left, pgid, is_background);
        return execute_ast(node->right, pgid, is_background);
    }

    case AST_background:
    {
        if (!node->left)
        {
            fprintf(stderr, "Syntax error: nothing before '&'\n");
            return 1;
        }

        if (node->left->type == AST_background)
        {
            execute_ast(node->left, 0, 0);
        }
        else
        {
            execute_ast(node->left, 0, 1);
        }
        return execute_ast(node->right, 0, 0);
    }
    }

    return 1;
}

void prompt()
{
    printf("Welcome to MSH shell,This shell follows posix standard\n");
}

int main()
{pid_t shell_pgid = getpid();
setpgid(shell_pgid, shell_pgid);



tcsetpgrp(STDIN_FILENO, shell_pgid); 

signal(SIGTTOU, SIG_IGN);  
signal(SIGTTIN, SIG_IGN);  
signal(SIGTSTP, SIG_IGN);  
signal(SIGINT,  SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    prompt();

    char *user = getenv("USER");
    char dir[100];

    if (!user)
        user = "unknown";
    while (1)
    {
       
        getcwd(dir, 100);
        printf("MSH-%s@ %s $: ", user, dir);

        char c;
        int index = 0;
        char prev = '[';

        while ((result=scanf("%c", &c))!=EOF && ((index > 0 ? buffer[index - 1] == '\\' : 0) || c != '\n'))
        {
        
            if (index >= 1023)
                break;
            if (index == 0 && c == ' ')
                continue;
            if (c == ' ' || c == '\n')
            {
                if (c == '\n')
                {
                    index = index - 1;
                    continue;
                }
                prev = ' ';
                continue;
            }
            if (prev == ' ')
            {
                buffer[index++] = prev;
                prev = '[';
            }
            buffer[index++] = c;
        }
         if(result==EOF){
            exit(0);
        }

        buffer[index] = '\0';
        if (index == 0)
            continue;

        int token_count = 0;
        Token **tokens = Tokenise(buffer, index, &token_count);
        if (tokens == NULL)
        {

            free_tokens(tokens, token_count);
            continue;
        }

        int pos = 0;

        Node *ast = parse_sequence(tokens, &pos, token_count);
        if (pos != token_count)
        {

            free_tokens(tokens, token_count);
            fprintf(stderr, "Syntax error: unexpected extra tokens after position %d\n", pos);
            continue;
        }

  
        execute_ast(ast, 0, 0);

        free_ast(ast);
        free_tokens(tokens, token_count);
   

    }
    return 0;
}