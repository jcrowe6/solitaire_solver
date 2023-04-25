typedef struct t_card;
typedef struct t_deck;
typedef struct t_zones;

t_zones* init_zones();

void free_zones(t_zones* zones);

void print_zones(t_zones* zones);

void fill_tableau(t_zones* zones);


