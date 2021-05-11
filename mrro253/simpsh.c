#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <error.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int MAXMSGLEN = 256;
int MAXLIST = 100;
int varpos = 3;
int cnt = 0;
int infrom = 0;
int outto = 0;
typedef enum { HASH, BANG, EQUALS, CD, LV, QUIT, UNSET, INFROM, OUTTO, VARNAME, GENERAL } tokentype;

struct token {
        tokentype t;
        char * value;
    
};
struct varMemory {
    char * vname;
    char * varvalue;
};

struct varMemory *init(){
    struct varMemory* vars = (struct varMemory *)calloc(MAXMSGLEN, sizeof(struct vars*));
    if(vars == NULL){
        perror("malloc");
        exit(0);
    }
    vars[0].vname = "PS";
    vars[0].varvalue = "mrrproj4> ";
    vars[1].vname = "PATH";
    vars[1].varvalue = "/bin/:/usr/bin/";
    infrom = 0;
    outto = 0;
    return vars;
}
void sigint_handler(int sig) {
    exit(0);
}
char *scan(void) 
{
        int size = MAXLIST;
        int pos = 0;
        char *buffer = malloc(sizeof(char) *size);
        int c;

        while (1) {
            
            c = getchar();
            if(c == EOF){
                printf("Goodbye...\n");
                exit(0);
            }
           if(c == '\n') {
                buffer[pos] = ' ';
                return buffer;
            }
            else { 
                buffer[pos] = c;
            }
            pos++;
        }
}
struct token *makeTokens(char *cmdline) {
    int size = MAXMSGLEN;
    int pos = 0;
    struct token* tokens = (struct token*)calloc(size, sizeof(struct token*));
    char *token;
    char delim[] = " :\"";
    cnt = 0;
    
    token = strtok(cmdline, delim);
    //printf("Entering makeToken while loop\n");
    while(token != NULL) {
        if (strcmp(token, "!") == 0){
            tokens[pos].t = BANG;
            cnt++;
        } else if (strcmp(token, "#") == 0){
            tokens[pos].t = HASH;
            cnt++;
        } else if (strcmp(token, "cd") == 0){
            tokens[pos].t = CD;
            cnt++;
        } else if (strcmp(token, "lv") == 0){
            tokens[pos].t = LV;
            cnt++;
        } else if (strcmp(token, "quit") == 0){
            tokens[pos].t = QUIT;
            cnt++;
        } else if (strcmp(token, "=") == 0){
            tokens[pos].t = EQUALS;
            cnt++;
        } else if (strcmp(token, "unset") == 0){
            tokens[pos].t = UNSET;
            cnt++;
        } else if (strcmp(token, "infrom") == 0){
            tokens[pos].t = INFROM;
            cnt++;
        } else if (strcmp(token, "outto") == 0){
            tokens[pos].t = OUTTO;
            cnt++;
        } else {
            tokens[pos].t = GENERAL;
            cnt++;
        }
        tokens[pos].value = token;
        if (pos >= size) {
            size += MAXMSGLEN;
            tokens = realloc(tokens, size * sizeof(char*));
        }
        token = strtok(NULL, delim);
        pos++;
    }
    //printf("Exiting while loop\n");
    //tokens[pos] = NULL;
    /*for (int i = 0; i < pos; i++) {
        printf("(%d, %s)\n", tokens[i].t, tokens[i].value);
    } */
    return tokens;
}

void doLV(struct varMemory *vars){
    for (int i = 0; i < varpos; i++){
        if (vars[i].vname == NULL){
            break;
        }else
            printf("%s = %s\n", vars[i].vname, vars[i].varvalue);
    }
}
void doCD(struct token *args) {
    if(args[1].value == NULL) {
        printf("Error, expected argument to cd\n");

    } else {
        if (chdir(args[1].value) != 0) {
            perror("CHDIR ERROR");
        }
    }
}
int launch(struct token *tok, struct varMemory *vars, char * path){
    const char *spath = path;
    char **args = malloc(sizeof(char) *MAXLIST);
    char *envp[] = { NULL };
    args[0] = (char *)strdup(tok[1].value);
    int infilepos = 0;
    int outfilepos = 0;
    if(infrom == 1){
        
        for (int i = 1; i < cnt; i++){
            if (tok[i].t == 7){
                infilepos = i+1;
                break;
            }
        }
    }
    if (outto == 1){
        for (int i = 1; i < cnt; i++){
            if(tok[i].t == 8){
                outfilepos = i+1;
            }
        }
    }
    int incnt = 1;
    int outcnt = 1;
    if(infrom == 0 && outto == 0){
        for(int i = 2; i < cnt; i++){
            if(args[i-1] == NULL){
                args = realloc(args, MAXLIST * sizeof(char*));
            }
            args[i-1] = strdup(tok[i].value);
        }
        pid_t pid = fork();
        if(pid == -1)
            perror("Fork");
        if (pid == 0){ //Child function
            execve(spath, args, envp);
        }
        int status;
        pid_t w = waitpid(pid, &status, 0);
        if(w == -1){
            perror("waitpid");
        }
        else{
            free(args);
            return 1;
        }
    }
    else if (infrom == 1 && outto == 0){
        int fdin = open(tok[infilepos].value, O_RDWR);
        if(!fdin)
            perror("Open2");
        for(int i = 2; i < infilepos; i++){
            if(args[i-1] == NULL){
                args = realloc(args, MAXLIST * sizeof(char*));
            }
            args[i-1] = strdup(tok[i+1].value);
            incnt++;
        }
        args[infilepos-1] = NULL;
        incnt++;
       
        pid_t pid = fork();
        if(pid == -1)
            perror("Fork");
        if (pid == 0){ //Child function
            
            close(0);
            dup2(fdin, 0);
            execve(spath, args, envp);
        }
        int status;
        pid_t w = waitpid(pid, &status, 0);
        if(w == -1){
            perror("waitpid");
        }
        else{
            free(args);
            close(fdin);
            return 1;
        }
        
        //for(int j = infilepos; j < cnt; j++){
            //args[j-1] = strdup(tok[j-1].value);
        //}
    } else if (outto == 1 && infrom == 0){
        int fd;
         FILE *fdout;
        fdout = fopen(tok[outfilepos].value, "a+");
        if(!fdout)
            perror("Open");
        for(int i = 1; i < outfilepos-2; i++){
            if(args[i+1] == NULL){
                args = realloc(args, MAXLIST * sizeof(char*));
            }
            args[i] = strdup(tok[i+1].value);
            outcnt++;
        }
        args[outfilepos-2] = NULL;
        outcnt++;
        pid_t pid = fork();
        if(pid == -1)
            perror("Fork");
        if (pid == 0){ //Child function
            fd = fileno(fdout);
            close(1);
            dup2(fd, 1);
            execve(spath, args, envp);
        }
        int status;
        pid_t w = waitpid(pid, &status, 0);
        if(w == -1){
            perror("waitpid");
        }
    } else if (infrom == 1 && outto == 1){
            int fd;
            FILE *fdout;
            fdout = fopen(tok[outfilepos].value, "a+");
            if(!fdout)
                perror("Open");
            int fdin = open(tok[infilepos].value, O_RDWR);
            if(!fdin)
                perror("Open2");
            for(int i = 2; i < cnt; i++){
                if(args[i-1] == NULL){
                    args = realloc(args, MAXLIST * sizeof(char*));
                }
                args[i-1] = strdup(tok[i].value);
            }
            pid_t pid = fork();
            if(pid == -1)
                perror("Fork");
            if (pid == 0){ //Child function
                fd = fileno(fdout);
                close(1);
                dup2(fd, 1);
                close(0);
                dup2(fdin, 0);
                execve(spath, args, envp);
            }
            int status;
            pid_t w = waitpid(pid, &status, 0);
            if(w == -1){
                perror("waitpid");
            }
            else{
                free(args);
                close(fdin);
                fclose(fdout);
                return 1;
            }
        }
    
    
    
    return 1;
}
void unset(struct token *tok, struct varMemory *vars){
        char * search = tok[1].value;
        for(int j = 0; j < varpos; j++) {
            if(strcmp(vars[j].vname, search) == 0){
                vars[j].vname = 0;
                vars[j].varvalue = 0;
                varpos--;
                if(j < varpos){
                    int diff = varpos-j;
                    for(int i = 1; i <= diff; i++){
                        vars[j] = vars[j+i];
                    }
                }
                return;
            }
        }
}
int execFunction(struct token *tok, struct varMemory *vars){
    int size = MAXMSGLEN;
    int didExist = 0;
    switch(tok[0].t) {
        case 0: //HASH
            break;
        case 1: //Bang
            break;
        case 3: //CD
            doCD(tok);
            break;
        case 4: //LV
            doLV(vars);
            break;
        case 5: //QUIT
            return -1;
            break;
        case 6: //UNSET
            unset(tok, vars);
            break;
        case 7: //INFROM
            infrom = 1;
            break;
        case 8: //OUTTO
            outto = 1;
            break;
        case 10: //GENERAL
            
            if(tok[1].t == 2){  //second token is equals
                for(int i = 0; i < varpos; i++){
                    if (strcmp(vars[i].vname, tok[0].value) == 0){ //If the variable already exists
                        didExist = 1;
                        vars[i].varvalue = tok[2].value;    //Store it there, dont add a new one
                        break;
                    }
                }
                if(didExist)
                    break;
                if(varpos >= size){
                    vars = realloc(vars, size * sizeof(struct vars*));
                    if(vars == NULL){
                        perror("Realloc");
                    }
                }
                vars[varpos].vname = tok[0].value;
                vars[varpos].varvalue = tok[2].value;
                ++varpos;
                break;
            } 
            break;
        default:
            printf("Default\n");
            break;
    }
    return 0;
}
int checkSyntax(struct token *tok, struct varMemory *vars){
    int isValid = 1;
    DIR *dir;
    struct dirent *ent;
    char *ret;
    const char ch = '$';
    for(int i = 0; i < cnt; i++){ //Check For $
        ret = strchr(tok[i].value, ch);
        if(ret != NULL){
            char * searchMsg = ret + 1;
            tok[i].value = searchMsg;
            for(int j = 0; j < varpos; j++) {
                if(strcmp(vars[j].vname, searchMsg) == 0){
                    tok[i].value = vars[j].varvalue;
                    break;
                }
            }
        }
    }
    for(int i = 0; i < cnt; i++){
        if(tok[i].t == 7){
            infrom = 1;
        }
        if(tok[i].t == 8){
            outto = 1;
        }
    }
    if(tok[0].t == 1){ //FIRST TOKEN IS BANG
        int pathcnt = 0;
        char * temp = strdup(vars[1].varvalue);
        char *path = strtok(temp, " :");
        pathcnt++;
        if (strcmp(&tok[1].value[0],"/") == 0 || (strcmp(&tok[1].value[0],".") == 0 && (strcmp(&tok[1].value[1],"/") == 0))){
            return 1;
        }
        while (path != NULL){
            if (!(dir = opendir(path)))
                perror("Opendir");
            while ((ent = readdir(dir)) != NULL) {
                //printf("%s\n", ent->d_name);
                char * tempName = strdup(ent->d_name);
                //tempName[0] = '/';
                //tempName[1] = strdup(ent->d_name); */
                if(strcmp(tempName,tok[1].value) == 0){
                    //printf("Found %s.\n", tempName);
                    char * final = strcat(path, tempName);
                    if(launch(tok, vars, final)){
                        return 1;
                    }
                }
            }
                closedir(dir);
                path = strtok(NULL, ":");
        }
        printf("ERROR, file %s not found.\n", tok[1].value);
        
        return 0;
    }
    if (tok[0].t == 0){ //First token is #
        for(int i = 1; i < cnt; i++){
            for(int j = 0; j < sizeof(tok->t); j++){
                if(tok[i].value[j] == '#'){
                    isValid = 0;
                    printf("ERROR: # may only be the first token.\n");
                    return isValid;
                    break;
                }
                else 
                    isValid = 1;
            }
        }
        return isValid;
    }
    if(tok[0].t == 6){
        if (strcmp(tok[1].value,"PATH")==0 || strcmp(tok[1].value,"PS")==0 || strcmp(tok[1].value, "CWD")==0){
            printf("ERROR: Cannot unset %s.\n", tok[1].value);
            return 0;
        } else {
            return 1;
        }
    }
    if(tok[1].t == 2){ //second token is =
        if (tok[0].t == 10){    //first token is word
            if(cnt != 3){
                printf("ERROR: format is name = value\n");
                return 0;
            }
            if(isalpha(tok[0].value[0]) == 0){  //if first char not a letter
                printf("ERROR: name must start with letter\n");
                return 0;
            }
        }
    } else if (tok[0].t == 10){
        printf("Error: please enter a valid command.\n");
    }
    if (tok[0].t == 4){
        if (cnt > 1){
            printf("Error, lv doesn't take args.\n");
            return 0;
        } 
    }
    return 1;

}
int main()
{
    char *cmdline;
    struct token *tokens;
    int exec = 0;
    struct varMemory *vars;
    int syntaxGood = 0;
    vars = init();
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("signal handler");
    }
    /* Use realloc() if more space is needed */
    for(;;) { /*loop forever */
        infrom = 0;
        outto = 0;
        vars[2].vname = "CWD";
        vars[2].varvalue = getcwd(NULL, 0);
        printf("%s", vars[0].varvalue);
        cmdline = scan();
        if(cmdline == NULL || *cmdline == EOF){
            break;
        }

        tokens = makeTokens(cmdline);
        syntaxGood = checkSyntax(tokens, vars);
        if(syntaxGood == 1){
            exec = execFunction(tokens, vars);
            if (exec == -1){
                printf("Goodbye.\n");
                break;
            }
        } else if (syntaxGood == -1) {
            printf("Goodbye...\n");
            exit(0);
        } 
        /* Check syntax */
        //if (syntaxGood){
            /* Carry out operation */
        //} else {
            //printf("Error");
        //}
       // if (exec == 1){
       //     execArgs(parsedArgs);
        //}
    }
    fflush(stdout);
    free(tokens);
    free(vars);
    return 0;
    //free(value);
}