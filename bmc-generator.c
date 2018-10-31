#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

/*
 * Generate 2 files.
 * A reducer program,
 * and a bmc formula pre-form
 */

/* 
 * Structs 
 */
typedef struct Array {
    int size;
    char** elements;
    // For Opers
    int* is_binary;
} Array;


/* 
 * Static Variables 
 */
static Array* Vars = NULL;
static Array* Opers = NULL;
static Array* Cmps = NULL;
static int Java_fd;
static int Bmc_fd;
static int* Line_types; 
// Line_types:
//  0 -> normal
//  1 -> "if (--) {"
//  2 -> "} else {"
//  3 -> "}"

static int VAR_NUM = 7;
static int IF_NUM = 10;
static int BASELINE = 200;
static int LINE = 0;        


/* 
 * Functions 
 */
int writeline(char *str) {
    int size = 0;
    int length = strlen(str);
    char *ptr = str;
    while (size < length) {
        int n = write(Java_fd, ptr, length - size);
        ptr += n;
        size += n;
    }
    return size;
}

int write_pre(char *str) {
    int size = 0;
    int length = strlen(str);
    char *ptr = str;
    while (size < length) {
        int n = write(Bmc_fd, ptr, length - size);
        ptr += n;
        size += n;
    }
    return size;
}

int get_random(int max) {
    if (max == 0) return 0;
    int random = rand();
    return random % max;
}

int open_filefd(char* filename) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) {
        fprintf(stderr, "file fd failed\n");
        exit(1);
    }
    return fd;
}

Array* init_Array() {
    Array* array = malloc(sizeof(Array));
    array->size = 0;
    array->elements = NULL;
    array->is_binary = NULL;
    return array;
}

void push_element(Array* array, char* element) {
    if (element == NULL) return;

    array->size++;
    array->elements = realloc(array->elements, sizeof(char*) * array->size);
    array->elements[array->size - 1] = strdup(element);
}

void push_oper(Array* array, char* operation, int is_binary) {
    if (operation == NULL) return;

    array->size++;
    array->elements = realloc(array->elements, sizeof(char*) * array->size);
    array->elements[array->size - 1] = strdup(operation);

    array->is_binary = realloc(array->is_binary, sizeof(int*) * array->size);
    array->is_binary[array->size - 1] = is_binary;
}

void init_vars() {
    char c = 'a';
    for (int i = 0; i < VAR_NUM; i++) {
        char* str = malloc(6);
        str[0] = c;
        str[1] = i / 100 + '0';
        str[2] = (i % 100) / 10 + '0';
        str[3] = (i % 10) + '0';
        str[4] = '_';
        str[5] = '\0';
        push_element(Vars, str);
        free(str);
    }
}

void init_opers() {
    push_oper(Opers, "+", 1);
    push_oper(Opers, "-", 1);
    push_oper(Opers, "*", 1);
    //push_oper(Opers, "/", 1);
    //push_oper(Opers, "%", 1);
    //push_oper(Opers, "+=", 0);
    //push_oper(Opers, "-=", 0);
    //push_oper(Opers, "*=", 0);
    //push_oper(Opers, "/=", 0);
}

void init_cmps() {
    push_element(Cmps, "==");
    push_element(Cmps, "!=");
    push_element(Cmps, ">=");
    push_element(Cmps, "<=");
    push_element(Cmps, ">");
    push_element(Cmps, "<");
}

void init_line_types() {
    Line_types = malloc(sizeof(int) * LINE);
    for (int i = 0; i < LINE; i++) Line_types[i] = 0;
    if (IF_NUM <= 0) return;

    int if_else_end[3] = {IF_NUM, 0, 0};
    int hierachy_stack[IF_NUM];
    int hierachy = 0;
    int total = IF_NUM * 3;
    int done = 0;
    int current_line = 0;
    int target_line = 0;
    int avaliable = 0;
    int avaliable_list[3] = {-1, -1, -1};
    int range_max = LINE / IF_NUM / 3;

    for (int i = 0; i < IF_NUM; i++) hierachy_stack[i] = 0;

    for (int i = 0; i < total; i++) {
        int range = (LINE - (total - done) - current_line);
        range = range > range_max ? (range_max * 2) : range;
        target_line = get_random(range) + current_line;
        avaliable = 0;

        if (hierachy_stack[hierachy] == 0) {
            if (if_else_end[0] > 0) {
                avaliable_list[avaliable] = 0; 
                avaliable++;
            }
        } else if (hierachy_stack[hierachy] == 1) {
            for (int j = 0; j < 3; j++) {
                if (j != 2 && if_else_end[j] > 0) {
                    avaliable_list[avaliable] = j; 
                    avaliable++;
                }
            }
        } else {
            for (int j = 0; j < 3; j++) {
                if (j != 1 && if_else_end[j] > 0) {
                    avaliable_list[avaliable] = j; 
                    avaliable++;
                }
            }
        }

        int target = get_random(avaliable);
        if (avaliable_list[target] == 0) {
            Line_types[target_line] = 1;
            if_else_end[0]--;
            if_else_end[1]++;
            if (hierachy_stack[hierachy] == 0) {
                hierachy_stack[hierachy] = 1;
            } else {
                hierachy_stack[++hierachy] = 1;
            }
        } else if (avaliable_list[target] == 1) {
            Line_types[target_line] = 2;
            if_else_end[1]--;
            if_else_end[2]++;
            hierachy_stack[hierachy] = 2;
        } else {
            Line_types[target_line] = 3;
            if_else_end[2]--;
            hierachy_stack[hierachy] = 0;
            if (hierachy != 0) hierachy--;
        }

        done++;
        current_line = target_line + 1;
        assert(current_line <= LINE);
    }
    assert(hierachy == 0);
}

void destroy_Array(Array* array) {
    if (array->elements != NULL) {
        for (int i = 0; i < array->size; i++) {
            free(array->elements[i]);
            array->elements[i] = NULL;
        }
        free(array->elements);
        array->elements = NULL;
    }
    if (array->is_binary != NULL) {
        free(array->is_binary);
    }
    free(array);
}

void write_init_vars() {
    for(int i = 0; i < Vars->size; i++) {
        writeline("int ");
        writeline(Vars->elements[i]);
        writeline(" = 0;\n");

        write_pre("VAR: ");
        write_pre(Vars->elements[i]);
        write_pre("\n");
    }
    writeline("\n");
}

void sum_up_vars() {
    for(int i = 0; i < Vars->size; i++) {
        writeline("sum += ");
        writeline(Vars->elements[i]);
        writeline(";\n");
    }
    writeline("\n");
}

char* generate_normal_line() {
    // 1% have *
    if (get_random(100) == 1) {
        int lhs_i = get_random(Vars->size);
        char** rhs = malloc(sizeof(char*) * 2);
        int rhs_i = get_random(Vars->size);
        rhs[0] = strdup(Vars->elements[rhs_i]);
        asprintf(&rhs[1], "%d", get_random(10) - 5);
        char *result = NULL;
        asprintf(&result, "%s = %s * %s;\n", 
                Vars->elements[lhs_i], rhs[0], rhs[1]);

        char *bmc = NULL;
        asprintf(&bmc, "(= %s  (* %s  %s ))\n", 
                Vars->elements[lhs_i], rhs[0], rhs[1]);
        write_pre(bmc);

        free(rhs[0]);
        free(rhs[1]);
        free(rhs);
        free(bmc);
        return result;
    }

    int lhs_i = get_random(Vars->size);
    int op_i = get_random(Opers->size);
    //if (Opers->is_binary[op_i] == 1) {
    char** rhs = malloc(sizeof(char*) * 2);
    for (int i = 0; i < 2; i++) {
        int rhs_i = get_random(Vars->size + 1);
        if (rhs_i == Vars->size) asprintf(&rhs[i], "%d", get_random(10) - 5);
        else rhs[i] = strdup(Vars->elements[rhs_i]);
    }
    char *result = NULL;
    asprintf(&result, "%s = %s %s %s;\n", 
            Vars->elements[lhs_i], rhs[0], Opers->elements[op_i], rhs[1]);

    char *bmc = NULL;
    asprintf(&bmc, "(= %s (%s %s %s ))\n", 
            Vars->elements[lhs_i], Opers->elements[op_i], rhs[0], rhs[1]);
    write_pre(bmc);

    free(rhs[0]);
    free(rhs[1]);
    free(rhs);
    free(bmc);
    return result;
    /*
    } else {
        int rhs_i = get_random(Vars->size + 1);
        char *rhs = NULL;
        if (rhs_i == Vars->size) asprintf(&rhs, "%d", get_random(10) - 5);
        else rhs = strdup(Vars->elements[rhs_i]);
        char *result = NULL; 
        asprintf(&result, "%s %s %s;\n", 
                Vars->elements[lhs_i], Opers->elements[op_i], rhs);
        free(rhs);
        return result;
    }
    */
}

char* generate_condition() {
    int cmp_i = get_random(Cmps->size);
    int lhs_i = get_random(Vars->size);
    int rhs_i = get_random(Vars->size + 1);
    char *rhs = NULL;
    if (rhs_i == Vars->size) asprintf(&rhs, "%d", get_random(10) - 5);
    else rhs = strdup(Vars->elements[rhs_i]);
    char *result = NULL;
    asprintf(&result, "%s %s %s", 
            Vars->elements[lhs_i], Cmps->elements[cmp_i], rhs);

    char *bmc = NULL;
    char *cmp = Cmps->elements[cmp_i];
    if (strlen(cmp) > 1) {
        if (cmp[0] == '=' && cmp[1] == '=') {
            asprintf(&bmc, "(= %s %s )\n", 
                    Vars->elements[lhs_i], rhs);
        } else if (cmp[0] == '!' && cmp[1] == '=') {
            asprintf(&bmc, "(not (= %s %s ))\n", 
                    Vars->elements[lhs_i], rhs);
        } else {
            asprintf(&bmc, "(%s %s %s )\n", 
                    cmp, Vars->elements[lhs_i], rhs);
        }
    } else {
        asprintf(&bmc, "(%s %s %s )\n", 
                cmp, Vars->elements[lhs_i], rhs);
    }
    write_pre(bmc);

    free(rhs);
    free(bmc);

    return result;
}

void write_body_lines() {
    for (int i = 0; i < LINE; i++) {
        if (Line_types[i] == 0) {
            char *str = generate_normal_line();
            writeline(str);
            free(str);
        } else if (Line_types[i] == 1) {
            writeline("if (");
            write_pre("IF: ");
            char *str = generate_condition();
            writeline(str);
            writeline(") {\n");
            free(str);
        } else if (Line_types[i] == 2) {
            write_pre("ELSE:\n");
            writeline("} else {\n");
        } else if (Line_types[i] == 3) {
            write_pre("END\n");
            writeline("}\n");
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2 && argc != 5) {
        fprintf(stderr, "./generator filename <Variable Baseline If-else>\n");
        fprintf(stderr, "2 files filename.java and filename.pre will be generated\n");
        exit(1);
    }
    if (argc == 5) {
        VAR_NUM = atoi(argv[2]);
        BASELINE = atoi(argv[3]);
        IF_NUM = atoi(argv[4]);
    } else {
        VAR_NUM = 7;
        IF_NUM = 10;
        BASELINE = 200;
    }

    assert(IF_NUM >= 0);
    assert(VAR_NUM >= 0);
    assert(BASELINE > 0);

    LINE = (BASELINE + IF_NUM * 3);

    char *target_java = NULL, *target_bmc = NULL;
    asprintf(&target_java, "%s.java", argv[1]);
    asprintf(&target_bmc, "%s.pre", argv[1]);
    Java_fd = open_filefd(target_java);
    Bmc_fd = open_filefd(target_bmc);

    srand(time(NULL));
    Vars = init_Array();
    Opers = init_Array();
    Cmps = init_Array();

    writeline("// Note: only +, - operations\n");
    writeline("// Parameters:\n");
    char *str = NULL;
    asprintf(&str, "//   Variables:   %d\n", VAR_NUM);
    writeline(str);
    free(str);
    str = NULL;
    asprintf(&str, "//   Baselines:   %d\n", BASELINE);
    writeline(str);
    free(str);
    str = NULL;
    asprintf(&str, "//   If-Branches: %d\n\n", IF_NUM);
    writeline(str);
    free(str);
    str = NULL;

    init_vars();
    init_opers();
    init_cmps();
    init_line_types();


    push_element(Vars, "cur_");

    writeline("public void reduce(Text prefix, Iterator<IntWritable> iter,\n \
        OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {\n");

    write_init_vars();

    writeline("while (iter.hasNext()) {\n");
    writeline("cur_ = iter.next().get();\n");

    write_body_lines();

    writeline("}\n");


    writeline("output.collect(prefix, new IntWritable(");
    writeline(Vars->elements[get_random(Vars->size)]);
    writeline("));\n}\n");

    destroy_Array(Vars);
    destroy_Array(Opers);
    destroy_Array(Cmps);
    free(Line_types);
    Line_types = NULL;

    free(target_java);
    free(target_bmc);
    close(Java_fd);
    close(Bmc_fd);

    printf("Successly saved to %s\n", argv[1]);
    return 0;
}
