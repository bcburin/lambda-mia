#include "lambda_ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_lambda(struct lambda *l) {
    if (!l) {
        printf("NULL");
        return;
    }

    switch (l->type) {
        case LAMBDA_ABSTRACTION:
            printf("\\%s.", l->value.abstraction.var);
            print_lambda(l->value.abstraction.body);
            break;

        case LAMBDA_APPLICATION:
            printf("(");
            print_lambda(l->value.application.func);
            printf(" ");
            print_lambda(l->value.application.arg);
            printf(")");
            break;

        case LAMBDA_LVAR:
        case LAMBDA_VAR:
            printf("%s", l->value.var);
            break;

        case LAMBDA_LITERAL:
            printf("%d", l->value.ival);
            break;

        case LAMBDA_IF:
            printf("if ");
            print_lambda(l->value.if_expr.cond);
            printf(" then ");
            print_lambda(l->value.if_expr.then_expr);
            printf(" else ");
            print_lambda(l->value.if_expr.else_expr);
            break;

        case LAMBDA_LET:
            printf("let %s = ", l->value.let_expr.lvar);
            print_lambda(l->value.let_expr.decl);
            printf(" in ");
            print_lambda(l->value.let_expr.body);
            break;

        case LAMBDA_LIST_CONSTRUCTION:
            if (l->value.list_construction.head != NULL) {
                printf("[");
                print_lambda(l->value.list_construction.head);
                printf(" : ");
                print_lambda(l->value.list_construction.tail);
                printf("]");
            } else {
                printf("[]");
            }
            break;

        default:
            printf("Unknown lambda type: %d", l->type);
            break;
    }
}

struct {
    char *name;
    struct lambda *lambda;
} symbol_table[MAX_VARS];
int symbol_table_size = 0;

struct lambda *lookup_var(char *name) {
    for (int i = 0; i < symbol_table_size; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return symbol_table[i].lambda;
        }
    }
    printf("Error: Variable %s not found\n", name);
    return NULL;
}

void store_var(char *name, struct lambda *lambda) {
    if (symbol_table_size < MAX_VARS) {
        symbol_table[symbol_table_size].name = strdup(name);
        symbol_table[symbol_table_size].lambda = lambda;
        symbol_table_size++;
    } else {
        printf("Error: Symbol table full\n");
    }
}

struct lambda *create_lambda(char *var, struct lambda *body) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_ABSTRACTION;
    lambda->value.abstraction.var = strdup(var);
    lambda->value.abstraction.body = body;
    return lambda;
}

struct lambda *create_application(struct lambda *func, struct lambda *arg) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_APPLICATION;
    lambda->value.application.func = func;
    lambda->value.application.arg = arg;
    return lambda;
}

struct lambda *create_var(char *var) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_VAR;
    lambda->value.var = strdup(var);
    return lambda;
}

struct lambda *create_lambda_var(char *lvar) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LVAR;
    lambda->value.lvar = strdup(lvar);
    return lambda;
}

struct lambda *create_literal_int(int value) {
    struct lambda *f = create_var("f");
    struct lambda *x = create_var("x");
    for (int i = 0; i < value; i++) {
        x = create_application(f, x);
    }
    struct lambda *inner = create_lambda("x", x);
    struct lambda *lambda = create_lambda("f", inner);
    return lambda;
}

struct lambda *create_if(struct lambda *cond, struct lambda *then_expr,
                         struct lambda *else_expr) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_IF;
    lambda->value.if_expr.cond = cond;
    lambda->value.if_expr.then_expr = then_expr;
    lambda->value.if_expr.else_expr = else_expr;
    return lambda;
}

struct lambda *create_let(struct binding *binding, struct lambda *body) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LET;
    lambda->value.let_expr.lvar = binding->lvar;
    lambda->value.let_expr.decl = binding->lambda;
    lambda->value.let_expr.body = body;
    return lambda;
}

struct lambda *create_list_construction(struct lambda *head,
                                        struct lambda *tail) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LIST_CONSTRUCTION;
    lambda->value.list_construction.head = head;
    lambda->value.list_construction.tail = eval_lambda(tail);
    return lambda;
}

struct lambda *create_empty_list() {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LIST_CONSTRUCTION;
    lambda->value.list_construction.head = NULL;
    lambda->value.list_construction.tail = NULL;
    return lambda;
}

struct lambda *create_list_head_op(struct lambda *l) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LIST_OP;
    lambda->value.list_op.type = LIST_HEAD_OP;
    lambda->value.list_op.list = l;
    return lambda;
}

struct lambda *create_list_tail_op(struct lambda *l) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LIST_OP;
    lambda->value.list_op.type = LIST_TAIL_OP;
    lambda->value.list_op.list = l;
    return lambda;
}

struct lambda *create_list_empty_op(struct lambda *l) {
    struct lambda *lambda = malloc(sizeof(struct lambda));
    lambda->type = LAMBDA_LIST_OP;
    lambda->value.list_op.type = LIST_EMPTY_OP;
    lambda->value.list_op.list = l;
    return lambda;
}

struct lambda *create_true() {
    return create_lambda("x", create_lambda("y", create_var("x")));
}

struct lambda *create_false() {
    return create_lambda("x", create_lambda("y", create_var("y")));
}

int check_lambda_is_boolean_true(struct lambda *lambda) {
    struct lambda *var0 = create_literal_int(0);
    struct lambda *var1 = create_literal_int(1);
    struct lambda *rslt = apply_lambda(apply_lambda(lambda, var0), var1);
    return rslt != NULL &&
           rslt->value.abstraction.body->value.abstraction.body->type ==
               LAMBDA_VAR;
}

int check_lambda_is_boolean_false(struct lambda *lambda) {
    struct lambda *var0 = create_literal_int(0);
    struct lambda *var1 = create_literal_int(1);
    struct lambda *rslt = apply_lambda(apply_lambda(lambda, var0), var1);
    return rslt != NULL &&
           rslt->value.abstraction.body->value.abstraction.body->type ==
               LAMBDA_APPLICATION;
}

struct binding *create_binding(char *lvar, struct lambda *lambda) {
    struct binding *binding = malloc(sizeof(struct binding));
    binding->lvar = lvar;
    binding->lambda = lambda;
    return binding;
}

struct statement *create_lambda_statement(struct lambda *lambda) {
    struct statement *stm = malloc(sizeof(struct statement));
    stm->lambda = lambda;
    stm->binding = NULL;
    struct lambda *result = eval_lambda(lambda);
    // printf("Result: ");
    // print_lambda(result);
    // printf("\n");
    return stm;
}

struct statement *create_binding_statement(struct binding *binding) {
    struct statement *stm = malloc(sizeof(struct statement));
    stm->lambda = NULL;
    stm->binding = binding;
    store_var(binding->lvar, binding->lambda);
    return stm;
}

struct lambda *eval_lambda(struct lambda *lambda) {
    if (!lambda) return NULL;

    switch (lambda->type) {
        case LAMBDA_ABSTRACTION: {
            return lambda;  // Return the lambda expression itself
        }
        case LAMBDA_APPLICATION: {
            struct lambda *func = eval_lambda(lambda->value.application.func);
            struct lambda *arg = eval_lambda(lambda->value.application.arg);
            return apply_lambda(func, arg);
        }
        case LAMBDA_VAR:
            return lambda;
        case LAMBDA_LVAR:
            return lookup_var(lambda->value.lvar);
        case LAMBDA_LITERAL:
            return lambda;
        case LAMBDA_IF: {
            struct lambda *cond = eval_lambda(lambda->value.if_expr.cond);
            if (check_lambda_is_boolean_true(cond))
                return eval_lambda(lambda->value.if_expr.then_expr);
            if (check_lambda_is_boolean_false(cond))
                return eval_lambda(lambda->value.if_expr.else_expr);
            printf("Condition is not a boolean value");
        }
        case LAMBDA_LET: {
            struct lambda *result = substitute(lambda->value.let_expr.body,
                                               lambda->value.let_expr.lvar,
                                               lambda->value.let_expr.decl);
            return eval_lambda(result);
        }
        case LAMBDA_LIST_CONSTRUCTION:
            if (lambda->value.list_construction.head == NULL) return lambda;
            lambda->value.list_construction.head =
                eval_lambda(lambda->value.list_construction.head);
            lambda->value.list_construction.tail =
                eval_lambda(lambda->value.list_construction.tail);

            return lambda;

        case LAMBDA_LIST_OP:
            struct lambda *l = eval_lambda(lambda->value.list_op.list);
            switch (lambda->value.list_op.type) {
                case LIST_HEAD_OP:
                    return l->value.list_construction.head;

                case LIST_TAIL_OP:
                    return l->value.list_construction.tail;

                case LIST_EMPTY_OP:
                    if (l->value.list_construction.head != NULL)
                        return create_false();
                    else
                        return create_true();

                default:
                    printf("Unknown list operation type\n");
                    break;
            }

        default:
            printf("Unknown lambda type\n");
            return NULL;
    }
}

struct lambda *substitute(struct lambda *body, char *var, struct lambda *arg) {
    if (!body) return NULL;

    switch (body->type) {
        case LAMBDA_VAR:
        case LAMBDA_LVAR:
            if (strcmp(body->value.var, var) == 0) {
                // Replace variable with argument
                return arg;
            } else {
                return body;
            }
        case LAMBDA_ABSTRACTION:
            if (strcmp(body->value.abstraction.var, var) == 0) {
                // If abstraction variable is the same, don't substitute
                return body;
            } else {
                // Substitute in the body of the abstraction
                struct lambda *new_body =
                    substitute(body->value.abstraction.body, var, arg);
                return create_lambda(body->value.abstraction.var, new_body);
            }
        case LAMBDA_APPLICATION: {
            // Substitute in both function and argument
            struct lambda *new_func =
                substitute(body->value.application.func, var, arg);
            struct lambda *new_arg =
                substitute(body->value.application.arg, var, arg);
            if (new_func->type == LAMBDA_LVAR) new_func = eval_lambda(new_func);
            if (new_func->type == LAMBDA_ABSTRACTION)
                return apply_lambda(new_func, new_arg);
            else
                return create_application(new_func, new_arg);
        }
        case LAMBDA_IF: {
            struct lambda *new_cond =
                substitute(body->value.if_expr.cond, var, arg);
            struct lambda *new_then =
                substitute(body->value.if_expr.then_expr, var, arg);
            struct lambda *new_else =
                substitute(body->value.if_expr.else_expr, var, arg);
            return create_if(new_cond, new_then, new_else);
        }
        case LAMBDA_LET: {
            break;
        }
        case LAMBDA_LITERAL:
            return body;  // Literals don't need substitution
        case LAMBDA_LIST_CONSTRUCTION:
            return body;  // No substitution for list construction
        default:
            printf("Error: Unknown lambda type in substitution\n");
            return NULL;
    }
}

struct lambda *apply_lambda(struct lambda *lambda, struct lambda *arg) {
    if (lambda->type == LAMBDA_ABSTRACTION) {
        // Perform substitution
        struct lambda *body = lambda->value.abstraction.body;
        char *var = lambda->value.abstraction.var;
        struct lambda *result = substitute(body, var, arg);
        return result;
    }
    printf("Error: Attempting to apply non-lambda\n");
    return NULL;
}

struct program *create_program() {
    struct program *prog = malloc(sizeof(struct program));
    prog->statement_count = 0;
    return prog;
}

void add_statement_to_program(struct program *prog, struct statement *stm) {
    if (prog->statement_count < MAX_STATEMENTS) {
        prog->statements[prog->statement_count++] = stm;
    } else {
        fprintf(stderr, "Error: Too many statements in program.\n");
        exit(1);
    }
}

// Function to generate assembly for each node in the lambda AST
void generate_assembly_node(FILE *out, struct lambda *lambda) {
    if (!lambda) return;

    switch (lambda->type) {
        case LAMBDA_ABSTRACTION:
            fprintf(out, ";; lambda abstraction %s\n",
                    lambda->value.abstraction.var);
            fprintf(out, "push rax  ;; save current environment\n");
            generate_assembly_node(out, lambda->value.abstraction.body);
            fprintf(out, "pop rax   ;; restore environment\n");
            break;

        case LAMBDA_APPLICATION:
            generate_assembly_node(out, lambda->value.application.func);
            fprintf(out, "push rax  ;; apply function\n");
            generate_assembly_node(out, lambda->value.application.arg);
            fprintf(out, "pop rbx\n");
            fprintf(out, "call rbx  ;; call function with argument\n");
            break;

        case LAMBDA_VAR:
            fprintf(out, "mov rax, [%s]  ;; load variable %s\n",
                    lambda->value.var, lambda->value.var);
            break;

        case LAMBDA_LITERAL:
            fprintf(out, "mov rax, %d  ;; load literal %d\n",
                    lambda->value.ival, lambda->value.ival);
            break;

        case LAMBDA_IF:
            generate_assembly_node(out, lambda->value.if_expr.cond);
            fprintf(out, "cmp rax, 0  ;; evaluate condition\n");
            fprintf(out, "je else_branch\n");
            generate_assembly_node(out, lambda->value.if_expr.then_expr);
            fprintf(out, "jmp end_if\n");
            fprintf(out, "else_branch:\n");
            generate_assembly_node(out, lambda->value.if_expr.else_expr);
            fprintf(out, "end_if:\n");
            break;

        case LAMBDA_LET:
            fprintf(out, ";; let expression\n");
            generate_assembly_node(out, lambda->value.let_expr.decl);
            fprintf(out, "mov [%s], rax  ;; bind %s\n",
                    lambda->value.let_expr.lvar, lambda->value.let_expr.lvar);
            generate_assembly_node(out, lambda->value.let_expr.body);
            break;

        case LAMBDA_LIST_CONSTRUCTION:
            fprintf(out, ";; list construction\n");
            generate_assembly_node(out, lambda->value.list_construction.head);
            fprintf(out, "push rax\n");
            generate_assembly_node(out, lambda->value.list_construction.tail);
            fprintf(out, "pop rbx\n");
            fprintf(out, "mov [rbx], rax  ;; construct list\n");
            break;

        case LAMBDA_LIST_OP:
            switch (lambda->value.list_op.type) {
                case LIST_HEAD_OP:
                    fprintf(out, "mov rax, [rax]  ;; list head\n");
                    break;
                case LIST_TAIL_OP:
                    fprintf(out, "mov rax, [rax + 8]  ;; list tail\n");
                    break;
                case LIST_EMPTY_OP:
                    fprintf(out, "cmp rax, 0  ;; check if list is empty\n");
                    fprintf(out, "mov rax, 1\n");
                    fprintf(out, "mov rax, 0\n");
                    break;
            }
            break;

        default:
            fprintf(out, ";; unknown type\n");
            break;
    }
}

// Wrapper function to generate assembly for the entire program
void generate_assembly(struct program *program, const char *output_filename) {
    FILE *out = fopen(output_filename, "w");
    if (!out) {
        fprintf(stderr, "Error: Could not open file %s for writing.\n",
                output_filename);
        return;
    }

    for (int i = 0; i < program->statement_count; ++i) {
        struct statement *stmt = program->statements[i];

        if (stmt->lambda) {
            // Generate assembly for evaluating the lambda expression
            fprintf(out, ";; Evaluating lambda expression\n");
            generate_assembly_node(out, stmt->lambda);
            fprintf(out, "\n");
            // Optionally, add code to output the result if necessary.
            fprintf(out, "PRINT_RESULT\n\n");
        } else if (stmt->binding) {
            // Generate assembly for defining a function with label as var_name
            fprintf(out, "%s:\n",
                    stmt->binding->lvar);  // Create label for the function
            fprintf(out, ";; Function body for %s\n", stmt->binding->lvar);
            generate_assembly_node(out, stmt->binding->lambda);
            fprintf(out, "RET\n\n");  // End of function definition
        } else {
            fprintf(
                stderr,
                "Warning: Statement %d is neither a lambda nor a binding.\n",
                i);
        }
    }

    fclose(out);
    printf("Assembly code written to %s.\n", output_filename);
}