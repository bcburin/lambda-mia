#ifndef LAMBDA_AST_H
#define LAMBDA_AST_H

#include <stdio.h>

#define MAX_VARS 1024
#define MAX_STATEMENTS 1024

typedef enum {
    LAMBDA_ABSTRACTION,
    LAMBDA_APPLICATION,
    LAMBDA_VAR,
    LAMBDA_LVAR,
    LAMBDA_LITERAL,
    LAMBDA_IF,
    LAMBDA_LET,
    LAMBDA_LIST_CONSTRUCTION,
    LAMBDA_LIST_OP,
    LAMBDA_COMP_OP,
} lambda_type;

typedef enum { LIST_HEAD_OP, LIST_TAIL_OP, LIST_EMPTY_OP } list_op_type;

// Lambda AST node.
struct lambda {
    lambda_type type;
    union {
        struct {
            char *var;
            struct lambda *body;
        } abstraction;

        struct {
            struct lambda *func;
            struct lambda *arg;
        } application;

        char *var;
        char *lvar;

        int ival;

        struct {
            struct lambda *cond;
            struct lambda *then_expr;
            struct lambda *else_expr;
        } if_expr;

        struct {
            char *lvar;
            struct lambda *decl;
            struct lambda *body;
        } let_expr;

        struct {
            struct lambda *head;
            struct lambda *tail;
        } list_construction;

        struct {
            list_op_type type;
            struct lambda *list;
        } list_op;
    } value;
};

struct binding {
    char *lvar;
    struct lambda *lambda;
};

struct statement {
    struct lambda *lambda;
    struct binding *binding;
};

struct program {
    struct statement *statements[MAX_STATEMENTS];
    int statement_count;
};

// Function declarations for creating lambda AST nodes.
struct lambda *create_lambda(char *var, struct lambda *body);
struct lambda *create_application(struct lambda *func, struct lambda *arg);
struct lambda *create_var(char *var);
struct lambda *create_lambda_var(char *lvar);
struct lambda *create_literal_int(int value);
struct lambda *create_if(struct lambda *cond, struct lambda *then_expr,
                         struct lambda *else_expr);
struct lambda *create_let(struct binding *binding, struct lambda *body);
struct lambda *create_list_construction(struct lambda *head,
                                        struct lambda *tail);
struct lambda *create_empty_list();
struct lambda *create_list_head_op(struct lambda *l);
struct lambda *create_list_tail_op(struct lambda *l);
struct lambda *create_list_empty_op(struct lambda *l);
struct lambda *create_true();
struct lambda *create_false();
int check_lambda_is_boolean_true(struct lambda *lambda);
int check_lambda_is_boolean_false(struct lambda *lambda);

struct binding *create_binding(char *lvar, struct lambda *lambda);

struct statement *create_lambda_statement(struct lambda *lambda);
struct statement *create_binding_statement(struct binding *binding);

struct lambda *eval_lambda(struct lambda *lambda);

struct lambda *substitute(struct lambda *body, char *var, struct lambda *arg);
struct lambda *apply_lambda(struct lambda *lambda, struct lambda *arg);

struct lambda *lookup_var(char *name);
void store_var(char *name, struct lambda *lambda);

struct program *create_program();
void add_statement_to_program(struct program *prog, struct statement *stm);

void generate_assembly_node(FILE *out, struct lambda *lambda);
void generate_assembly(struct program *program, const char *output_filename);

#endif
