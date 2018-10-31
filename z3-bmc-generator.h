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
void destroy_stage(Stage** stage);
int size_of_map(Stage* stage);
Pair* get_pair_byindex(Stage* stage, int index);
Pair* get_pair_byname(Stage* stage, char* name);
void add_formula(char *formula, Stage* stage);
char* generate_formula(Stage* stage);
Stage* merge_stages(char* condition, Stage* sa, Stage* sb);


int hasString(char* src, char* target);
int read_prefile(char* file, char*** pre_lines);
void bmc_generator();
