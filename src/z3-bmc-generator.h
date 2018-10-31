/* 
 * Pair
 */
typedef struct Pair {
    int index;
    // name is called by reference
    char* name;
} Pair;

Pair* new_pair(char* name, int index);
Pair* clone_pair(Pair* pair);

/*
 * Map
 */
typedef struct Map {
    Pair** pairs;
    int size;
} Map;

void destroy_map(Map** map);
Map* clone_map(Map* map);
Map* merge_map(Map* ma, Map* mb);
void add_pair(Map* map, Pair* pair);

/*
 * Stage
 */
typedef struct Stage {
    char** formulas;
    int formula_size;
    Map* map;
} Stage;

Stage* new_stage();
Stage* clone_stage(Stage* stage);
void destroy_stage(Stage** stage);
int size_of_map(Stage* stage);
Pair* get_pair_by_index(Stage* stage, int index);
Pair* get_pair_by_name(Stage* stage, char* name);
void add_formula(char *formula, Stage* stage);
char* generate_formula(Stage* stage);
Stage* merge_stages(char* condition, Stage* sa, Stage* sb);
char* g_assignment_condition(char* assignment, Stage* stage, int flag);

/*
 * Stack of Stage
 */
typedef struct Stack_s {
    Stage** elements;
    int stack_ptr;
    int stack_space;
} Stack_s;

Stack_s* init_stack_s();
Stage* pop_stack_s(Stack_s* stack);
void push_stack_s(Stack_s* stack, Stage* stage);
void destroy_stack_s(Stack_s** stack);

/*
 * Stack of condition (char *)
 */
typedef struct Stack_c {
    char** elements;
    int stack_ptr;
    int stack_space;
} Stack_c;

Stack_c* init_stack_c();
char* pop_stack_c(Stack_c* stack);
void push_stack_c(Stack_c* stack, char* str);
void destroy_stack_c(Stack_c** stack);


int hasString(char* src, char* target);
int read_prefile(char* file, char*** pre_lines);
void declare_variable(Stage* stage);
void bmc_generator();
