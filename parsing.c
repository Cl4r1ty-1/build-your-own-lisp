#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct lval {
    int type;
    long num;
    char* err;
    char* sym;
    int count;
    struct lval** cell;
} lval;

lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval* lvel_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) {
    switch (v->type) {
        case LVAL_NUM: break;

        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;

        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
        break;
    }

    free(v);
}

void lval_print(lval v) {
    switch (v.type) {
        case LVAL_NUM: printf("%li", v.num); break;

        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by Zero!");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number!");
            }
            if (v.err == LERR_BAD_OP) {
                printf("Error: Invalid Operator!");
            }
        break;
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y) {
    
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    if ((strcmp(op, "+") == 0) || (strcmp(op, "add") == 0)) { return lval_num(x.num + y.num); }
    if ((strcmp(op, "-") == 0) || (strcmp(op, "sub") == 0)) { return lval_num(x.num - y.num); }
    if ((strcmp(op, "*") == 0) || (strcmp(op, "mul") == 0)) { return lval_num(x.num * y.num); }
    if ((strcmp(op, "/") == 0) || (strcmp(op, "div") == 0)) { 
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);
    }
    if ((strcmp(op, "%") == 0) || (strcmp(op, "mod") == 0)) { return lval_num(x.num % y.num); }
    if ((strcmp(op, "^") == 0) || (strcmp(op, "pow") == 0)) { return lval_num(x.num ^ y.num); }
    if (strcmp(op, "min") == 0) { return (x.num < y.num) ? lval_num(x.num) : lval_num(y.num); }
    if (strcmp(op, "max") == 0) { return (x.num > y.num) ? lval_num(x.num) : lval_num(y.num); }
    
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {

    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    char* op = t->children[1]->contents;
    lval x = eval(t->children[2]);

    /* check if - has a one argument, if so negate it */
    if ((strcmp(op, "-") == 0) || (strcmp(op, "sub") == 0)) {
        int expr_children = 0;
        for (int i = 2; i < t->children_num; i++) {
            if (strstr(t->children[i]->tag, "expr")) { expr_children++; }
        }
        if (expr_children == 1){
            x.num = -x.num;
            return x;
        }
    }

    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int main(int argc, char** argv) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT, 
        "                                                                                                                                   \
            number   : /-?([0-9]+[.])?[0-9]+/ ;                                                                                             \
            operator : '+' | '-' | '*' | '/' | '%' | '^' | \"add\" | \"sub\" | \"mul\" | \"div\" | \"mod\" | \"pow\" | \"min\" | \"max\";   \
            sexpr    : '(' <expr>* ')' ;                                                                                                    \
            expr     : <number> | '(' <operator> <expr>+ ')' ;                                                                              \
            lispy    : /^/ <operator> <expr>+ /$/ ;                                                                                         \
            ", 
            Number, Symbol, Sexpr, Expr, Lispy); 



    puts("Lispy Version 0.0.0.0.4");
    puts("Press Ctrl+c to Exit\n");
    puts("Interpreter Console");

    while (1) {

        char* input = readline("aura> ");

        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        };

        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
    return 0;
}
