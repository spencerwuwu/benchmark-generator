#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "z3-bmc-generator.h"

/* 
 * Convert .pre to bounded model checking 
 * formula in z3
 */


/* 
 * Function implementations in z3-bmc-generator.h
 */

// Pair
Pair* new_pair(char* name, int index) {
    Pair* new = malloc(sizeof(Pair));
    new->name = name;
    new->index = index;
    return new;
}

Pair* clone_pair(Pair* pair) {
    Pair* new = malloc(sizeof(Pair));
    new->name = pair->name;
    new->index = pair->index;
    return new;
}

// Map
void destroy_map(Map** target) {
    Map* map = *target;
    for (int i = 0; i < map->size; i++) {
        free(map->pairs[i]);
        map->pairs[i] = NULL;
    }
    free(map->pairs);
    free(map);
    *target = NULL;
}

Map* clone_map(Map* map) {
    Map* new = malloc(sizeof(Map));
    new->pairs = malloc(sizeof(Pair *) * map->size);
    for (int i = 0; i < map->size; i++) {
        new->pairs[i] = clone_pair(map->pairs[i]);
    }
    new->size = map->size;
    return new;
}

Map* merge_map(Map* ma, Map* mb) {
    Map* new = clone_map(ma);
    for (int i = 0; i < new->size; i++) {
        int a = ma->pairs[i]->index;
        int b = mb->pairs[i]->index;
        new->pairs[i]->index = a > b ? a : b;
    }
    return new;
}

void add_pair(Map* map, Pair* pair) {
    map->size++;
    map->pairs = realloc(map->pairs, sizeof(Pair *) * map->size);
    map->pairs[map->size - 1] = pair;
}

// Stage
Stage* new_stage() {
    Stage* new = malloc(sizeof(Stage));
    new->formulas = NULL;
    new->formula_size = 0;
    new->map = NULL;
    return new;
}

Stage* clone_stage(Stage* stage) {
    Stage* new = malloc(sizeof(Stage));
    int size = stage->formula_size;
    new->formulas = malloc(sizeof(char *) * size);
    new->formula_size = size;
    for (int i = 0; i < size; i++)
        new->formulas[i] = strdup(stage->formulas[i]);
    new->map = clone_map(stage->map);
    return new;
}

void destroy_stage(Stage** stage) {
    Stage* target = *stage;
    destroy_map(&target->map);
    for (int i = 0; i < target->formula_size; i++) {
        free(target->formulas[i]);
    }
    free(target->formulas);
    free(target);
    *stage = NULL;
}

int size_of_map(Stage* stage) {
    return stage->map->size;
}

Pair* get_pair_by_index(Stage* stage, int index) {
    return stage->map->pairs[index];
}

Pair* get_pair_by_name(Stage* stage, char* name) {
    Pair** pairs = stage->map->pairs;
    for (int i = 0; i < size_of_map(stage); i++) {
        if (hasString(name, pairs[i]->name))
            return pairs[i];
    }
    return NULL;
}

void add_formula(char *formula, Stage* stage) {
    int size = stage->formula_size;
    size++;
    stage->formulas = realloc(stage->formulas, sizeof(char *) * size);
    stage->formulas[size - 1] = strdup(formula);
    stage->formula_size = size;
}

char* generate_formula(Stage* stage) {
    char* formula = NULL;
    for (int i = 0; i < stage->formula_size; i++) {
        if (formula == NULL) {
            formula = strdup(stage->formulas[i]);
        } else {
            char *tmp = strdup("(and ");
            char *n_formula = stage->formulas[i];
            size_t size = strlen(tmp) + strlen(formula) + strlen(n_formula) + 4;
            tmp = realloc(tmp, size);
            strcat(tmp, formula);
            strcat(tmp, " ");
            strcat(tmp, n_formula);
            strcat(tmp, " )");

            free(formula);
            formula = NULL;
            formula = tmp;
        }
    }
    return formula;
}

Stage* merge_stages(char* condition, Stage* sa, Stage* sb) {
    Stage* stage = new_stage();
    stage->map = merge_map(sa->map, sb->map);

    for (int i = 0; i < size_of_map(stage); i++) {
        Pair* cur = get_pair_by_index(stage, i);
        Pair* pa = get_pair_by_index(sa, i);
        Pair* pb = get_pair_by_index(sb, i);
        if (cur->index > pa->index) {
            char* f = NULL;
            asprintf(&f, "(= %si%d %si%d )"
                    , cur->name, cur->index, pa->name, pa->index);
            add_formula(f, sa);
            free(f);
        }
        if (cur->index > pb->index) {
            char* f = NULL;
            asprintf(&f, "(= %si%d %si%d )"
                    , cur->name, cur->index, pb->name, pb->index);
            add_formula(f, sb);
            free(f);
        }
    }

    char* fa = generate_formula(sa);
    char* fb = generate_formula(sb);
    char* formula = NULL;
    asprintf(&formula, "(ite %s %s %s )", condition, fa, fb);
    add_formula(formula, stage);
    free(fa);
    free(fb);
    free(formula);

    return stage;
}

char* g_assignment_condition(char* assignment, Stage* stage, int flag) {
    // flag 0 -> assignment
    // flag 1 -> condition
    char* ptr = assignment;
    char* formula = NULL;
    int length = 0;
    int pos = 0;
    Pair* lhs = NULL;
    int is_lhs = 1;
    while (pos < (int)strlen(assignment)) {
        length = 0;
        while (*(ptr + length) != ' ' && pos + length < (int)strlen(assignment)) {
            length += 1;
        }
        char* seg = malloc(length + 1);
        seg[0] = '\0';
        strncat(seg, ptr, length);
        seg[length] = '\0';

        char *new_seg = NULL;
        if (hasString(seg, "_")) {
            if (flag == 0 && is_lhs == 1) {
                lhs = get_pair_by_name(stage, seg);
                asprintf(&new_seg, "%si%d", seg, lhs->index + 1);
                is_lhs = 0;
            } else {
                asprintf(&new_seg, "%si%d", seg, get_pair_by_name(stage, seg)->index);
            }
        } else {
            new_seg = strdup(seg);
        }

        if (formula == NULL) {
            formula = strdup(new_seg);
        } else {
            size_t len = strlen(formula) + strlen(new_seg) + 2;
            formula = realloc(formula, len);
            strcat(formula, " ");
            strcat(formula, new_seg);
        }


        free(seg);
        free(new_seg);
        ptr += length + 1;
        pos += length + 1;
    }
    if (lhs != NULL) lhs->index++;

    return formula;
}

// Stack of Stage
Stack_s* init_stack_s() {
    Stack_s* stack = malloc(sizeof(Stack_s));
    stack->elements = malloc(sizeof(Stack_s *) * 2);
    stack->stack_ptr = 0;
    stack->stack_space = 2;
    return stack;
}

Stage* pop_stack_s(Stack_s* stack) {
    if (stack->stack_ptr == 0) return NULL;
    return stack->elements[--stack->stack_ptr];
}

void push_stack_s(Stack_s* stack, Stage* stage) {
    stack->elements[stack->stack_ptr++] = stage; 
    if (stack->stack_ptr >= stack->stack_space) {
        int space = stack->stack_space * 2;
        stack->elements = realloc(stack->elements, sizeof(Stage *) * space);
        stack->stack_space = space;
    }
}

void destroy_stack_s(Stack_s** stack) {
    Stack_s* ptr = *stack;
    free(ptr->elements);
    free(ptr);
    *stack = NULL;
}

// Stack of condition
Stack_c* init_stack_c() {
    Stack_c* stack = malloc(sizeof(Stack_c));
    stack->elements = malloc(sizeof(Stack_c *) * 2);
    stack->stack_ptr = 0;
    stack->stack_space = 2;
    return stack;
}

char* pop_stack_c(Stack_c* stack) {
    if (stack->stack_ptr == 0) return NULL;
    return stack->elements[--stack->stack_ptr];
}

void push_stack_c(Stack_c* stack, char* str) {
    stack->elements[stack->stack_ptr++] = str;
    if (stack->stack_ptr >= stack->stack_space) {
        int space = stack->stack_space * 2;
        stack->elements = realloc(stack->elements, sizeof(char *) * space);
        stack->stack_space = space;
    }
}

void destroy_stack_c(Stack_c** stack) {
    Stack_c* ptr = *stack;
    free(ptr->elements);
    free(ptr);
    *stack = NULL;
}

/* 
 * Functions 
 */
int hasString(char* src, char* target) {
    if (strstr(src, target) != NULL)
        return 1;
    else 
        return 0;
}

int read_prefile(char* file, char*** lines) {
    char** pre_lines = NULL;
    FILE *pre_file = fopen(file, "r");
    char *buff = NULL;
    size_t size;
    int pre_line_num = 0;
    char *free_buff = NULL;
    while (getline(&buff, &size, pre_file) > 0) {
        pre_line_num++;
        pre_lines = realloc(pre_lines, sizeof(char *) * pre_line_num);
        pre_lines[pre_line_num - 1] = strdup(buff);
        int n = strlen(buff);
        if (buff[n - 1] == '\n')
            pre_lines[pre_line_num - 1][n - 1] = '\0';
        free_buff = buff;
    }
    free(free_buff);
    fclose(pre_file);
    *lines = pre_lines;
    return pre_line_num;
}


void bmc_generator(int pre_line_num, char** pre_lines) {
    int index = 0;
    char** var_names = NULL;
    int var_name_num = 0;
    Map* init_map = malloc(sizeof(Map));

    init_map->size =  0;
    init_map->pairs = NULL;
    while (index < pre_line_num && hasString(pre_lines[index], "VAR:")) {
        var_name_num++;
        var_names = realloc(var_names, sizeof(char *) * var_name_num);
        char* name = strdup(pre_lines[index] + 5);
        var_names[var_name_num - 1] = name;
        add_pair(init_map, new_pair(name, 0));
        index++;
    }

    Stack_s* stage_stack = init_stack_s();
    Stack_c* condition_stack = init_stack_c();

    Stage* stage_c = new_stage();
    stage_c->map = clone_map(init_map);
    while (index < pre_line_num) {
        if (hasString(pre_lines[index], "IF:")) {
            char* c = g_assignment_condition(pre_lines[index] + 4, stage_c, 1);
            push_stack_c(condition_stack, c);
            push_stack_s(stage_stack, clone_stage(stage_c));
        } else if (hasString(pre_lines[index], "ELSE:")) {
            Stage* tmp = stage_c;
            stage_c = pop_stack_s(stage_stack);
            push_stack_s(stage_stack, tmp);
        } else if (hasString(pre_lines[index], "END")) {
            Stage* else_stage = stage_c;
            Stage* if_stage = pop_stack_s(stage_stack);
            char* condition = pop_stack_c(condition_stack);
            stage_c = merge_stages(condition, if_stage, else_stage);
            destroy_stage(&if_stage);
            destroy_stage(&else_stage);
            free(condition);
        } else {
            char* f = g_assignment_condition(pre_lines[index], stage_c, 0);
            add_formula(f, stage_c);
            free(f);
        }

        index++;
    }

    char *final = generate_formula(stage_c);
    printf("%s\n", final);
    free(final);

    destroy_stack_s(&stage_stack);
    destroy_stack_c(&condition_stack);
    destroy_stage(&stage_c);
    destroy_map(&init_map);
    for (int i = 0; i < var_name_num; i++) {
        free(var_names[i]);
        var_names[i] = NULL;
    }
    free(var_names);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "./z3-bmc-generator filename\n");
        exit(1);
    }
    char** pre_lines = NULL;
    int pre_line_num = 0;

    pre_line_num = read_prefile(argv[1], &pre_lines);

    bmc_generator(pre_line_num, pre_lines);

    for (int i = 0; i < pre_line_num; i++) {
        free(pre_lines[i]);
    }
    free(pre_lines);

    return 0;
}
