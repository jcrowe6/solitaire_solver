#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct t_card {
    int value;
    int suit;
    int color;
    int faceup; // 1 if faceup, 0 if facedown
    struct t_card* below; // pointer to card that this card is on top of, NULL if it's just sitting on the table
    struct t_card* above; // pointer to card above this card, NULL if none
} t_card;

// just pointers to the top and bottom cards
// must ensure that top->above and bottom->below is NULL, and top<->bottom is doubly linked list
typedef struct t_deck {
    t_card* top;
    t_card* bottom;
    int ncards;
} t_deck;

typedef struct t_zones {
    t_deck* draw;
    t_deck* wastes;
    t_deck* tableau_facedown[7]; // keeps track of top part of each of 7 stacks on tableau. those cards that are facedown
    t_deck* tableau_faceup[7]; // the faceup cards in each stack. the bottom card of tableau_faceup[x] would be physically on top of tableau_facedown[x]
    t_deck* foundations[4];
} t_zones;

// allocates memory to hold 52 unique cards and info.
// then allocates space for a deck and returns it pointing to our cards
t_deck* init_deck() {
    t_card* cards = malloc(52 * sizeof(t_card));
    int pos = 0;
    for (int v = 1; v < 14; v++) {
        for (int s = 0; s < 4; s++) {
            cards[pos].value = v;
            cards[pos].suit = s;
            cards[pos].color = s % 2;
            cards[pos].faceup = 0;
            cards[pos].above = cards+pos-1;
            cards[pos].below = cards+pos+1;
            if (pos == 0) {
                cards[pos].above = NULL;
            } else if (pos == 51) {
                cards[pos].below = NULL;
            }
            pos++;
        }
    }
    t_deck* deck = malloc(sizeof(t_deck));
    deck->top = cards;
    deck->bottom = cards+51;
    deck->ncards = 52;
    return deck;
}

// allocates space for and returns an empty deck
t_deck* init_empty_deck() {
    t_deck* deck = malloc(sizeof(t_deck));
    deck->top = NULL;
    deck->bottom = NULL;
    deck->ncards = 0;
    return deck;
}

// Takes two valid decks, and moves `n` cards from the top of `fromdeck`
// and places them on top of `todeck`. 
// fromdeck should have the n+1 th card as it's top which has above->NULL, or if n equals the size of fromdeck, both top and bottom should be NULL.
// 
void move_deck_part(t_deck* fromdeck, t_deck* todeck, int n) {
    t_card* move_bottom = fromdeck->top; // the bottom card of the group that is moving
    for (int i = 0; i<n-1; i++) {
        move_bottom = move_bottom->below;
    }
    // 2 cases: move_bottom->below = NULL and != NULL
    // 2 more cases, todeck->top = NULL and != NULL
    if (todeck->top == NULL) {
        if (move_bottom->below == NULL) { // from deck will be empty and todeck is empty
            // just deck changes, no card link changes.
            todeck->top = fromdeck->top;
            todeck->bottom = move_bottom;
            fromdeck->top = NULL;
            fromdeck->bottom = NULL;
        } else { // from deck will have a new top and todeck is empty
            todeck->top = fromdeck->top;
            todeck->bottom = move_bottom;
            move_bottom->below->above = NULL;
            fromdeck->top = move_bottom->below;
            move_bottom->below = NULL;
        }
    } else { // todeck->top != NULL / todeck has a top
        if (move_bottom->below == NULL) { // from deck will be empty and todeck has a top card
            todeck->top->above = move_bottom;
            move_bottom->below = todeck->top;
            todeck->top = fromdeck->top;
            fromdeck->top = NULL;
            fromdeck->bottom = NULL;
        } else { // from deck will have a new top and todeck has a top
            fromdeck->top = move_bottom->below;
            todeck->top->above = move_bottom;
            move_bottom->below = todeck->top;
            todeck->top = fromdeck->top;
        }
    }

    fromdeck->ncards -= n;
    todeck->ncards += n;
}



// ensure deck is valid (has ncards cards with doubly linked list structure)
int test_deck(t_deck* deck) {
    if (deck->ncards == 0) {
        if (deck->top == NULL && deck->bottom == NULL) {
            return 0;
        } else {
            return 1;
        }
    }
    if (deck->top->above != NULL || deck->bottom->below != NULL) {
        return 1;
    }
    t_card* iter = deck->top;
    for (int i = 0; i < deck->ncards; i++) {
        if (iter->above != NULL && iter->above->below != iter) {
            return 1;
        }
        if (iter->below != NULL && iter->below->above != iter) {
            return 1;
        }
        iter = iter->below;
    }
    if (iter != NULL) {
        return 1;
    }
    return 0;
}

void print_deck(t_deck* deck) {
    t_card* iter = deck->top;
    while (iter) {
        printf("%d %d\n", iter->value, iter->suit);
        iter = iter->below;
    }
    if (test_deck(deck)) {
        printf("invalid!\n");
    } else {
        printf("valid\n");
    }
}

void shuffle_deck(t_deck* deck) {
    for (int i = 0; i < 1000; i++) {
        // cut deck
        t_card* top = deck->top;
        t_card* mid = deck->top;
        for (int j = 0; j < 26; j++) {
            mid = mid->below;
        }
        mid->above->below = NULL;
        mid->above = NULL;
        t_card* new_top = NULL;
        t_card* new_bottom = NULL;
        while (top != NULL && mid != NULL) {
            if (rand()%2) { // take from top
                t_card* top_below = top->below;
                if (!new_top) {
                    new_top = top;
                    new_bottom = top;
                    top->below = NULL;
                    top = top_below;
                    top->above = NULL;
                } else {
                    top->above = new_bottom;
                    new_bottom->below = top;
                    if (top->below != NULL) {
                        top->below->above = NULL;
                    }
                    top->below = NULL;
                    new_bottom = top;
                    top = top_below;
                }
            } else {
                t_card* mid_below = mid->below;
                if (!new_top) {
                    new_top = mid;
                    new_bottom = mid;
                    mid->below = NULL;
                    mid = mid_below;
                    mid->above = NULL;
                } else {
                    mid->above = new_bottom;
                    new_bottom->below = mid;
                    if (mid->below != NULL) {
                        mid->below->above = NULL;
                    }
                    mid->below = NULL;
                    new_bottom = mid;
                    mid = mid_below;
                }
            }
        }
        while (top != NULL) {
            t_card* top_below = top->below;
            top->above = new_bottom;
            new_bottom->below = top;
            if (top->below != NULL) {
                top->below->above = NULL;
            }
            top->below = NULL;
            new_bottom = top;
            top = top_below;
        }
        while (mid != NULL) {
            t_card* mid_below = mid->below;
            mid->above = new_bottom;
            new_bottom->below = mid;
            if (mid->below != NULL) {
                mid->below->above = NULL;
            }
            mid->below = NULL;
            new_bottom = mid;
            mid = mid_below;
        }
        deck->top = new_top;
        deck->bottom = new_bottom;
    }
}


t_zones* init_zones() {
    t_zones* zone = malloc(sizeof(t_zones));
    t_deck* deck = init_deck();
    shuffle_deck(deck);
    zone->draw = deck;
    zone->wastes = init_empty_deck();
    for (int i = 0; i<7; i++) { 
        zone->tableau_facedown[i] = init_empty_deck();
        zone->tableau_faceup[i] = init_empty_deck();
    }
    for (int i = 0; i<4; i++) {
         zone->foundations[i] = init_empty_deck();
    }
    return zone;
}

char suit(t_card* card) {
    if (card == NULL) {
        return 'X';
    }
    int suit = card->suit;
    if (suit == 0) {
        return 'D';
    } else if (suit == 1) {
        return 'C';
    } else if (suit == 2) {
        return 'H';
    } else {
        return 'S';
    }
}

char value(t_card* card) {
    if (card == NULL) {
        return 'X';
    }
    int val = card->value;
    if (val == 1) {
        return 'A';
    } else if (val < 10) {
        return val+'0';
    } else if (val == 10) {
        return 'T';
    } else if (val == 11) {
        return 'J';
    } else if (val == 12) {
        return 'Q';
    } else if (val == 13) {
        return 'K';
    }
}

void print_card(t_card* card) {
    printf("%c,%c  ", value(card),suit(card));
}

void print_cardn(t_card* card) {
    printf("%c,%c  \n", value(card),suit(card));
}

void print_zones(t_zones* zones) {
    printf("draw, wastes size: %d, %d\n", zones->draw->ncards, zones->wastes->ncards);
    printf("wastes top: ");
    print_cardn(zones->wastes->top);
    printf("foundations: \n");
    print_card(zones->foundations[0]->top);
    print_card(zones->foundations[1]->top);
    print_card(zones->foundations[2]->top);
    print_cardn(zones->foundations[3]->top);
    printf("tableau, facedown counts\n");
    for (int i = 0; i < 6; i++) {
        printf("%d    ", zones->tableau_facedown[i]->ncards);
    }
    printf("%d\n", zones->tableau_facedown[6]->ncards);
    printf("---------------tableau---------------\n");
    for (int i = 0; i < 6; i++) {
        print_card(zones->tableau_faceup[i]->bottom);
    }
    print_cardn(zones->tableau_faceup[6]->bottom);
    printf("                ...\n");
    for (int i = 0; i < 6; i++) {
        print_card(zones->tableau_faceup[i]->top);
    }
    print_cardn(zones->tableau_faceup[6]->top);
}

void fill_tableau(t_zones* zones) {
    // facedown cards
    for (int i = 1; i < 7; i++) {
        move_deck_part(zones->draw, zones->tableau_facedown[i], i);
    }
    for (int i = 0; i < 7; i++) {
        move_deck_part(zones->draw, zones->tableau_faceup[i], 1);
    }
}

// Solving time...
// Possible moves:
// Move faceup stack where bottom card has another place on top of a different stack to go. may flip a facedown
// Move single card from top of faceup stack to foundation. (could be the last card of the stack -> may flip a facedown)
// Move a card from top of wastes to tableau
// Move a card from top of wastes to foundation

// quick and EZ: for each faceup stack bottom card, look through each faceup stack top card and find a card with val+1 and opposite suit
// then move and restart until no moves

// this will not be optimal but fast enough and easiest to program. similar idea for looking for foundation/wastes moves. then can run many simulations and play with the priority 
// of moves to see how it affects winrate

// return 1 if deck1 can be moved onto deck 2 by following the rulesof solitaire
// Don't call with empty deck1. But deck2 can be empty (K can be moved there)
int can_move(t_deck* deck1, t_deck* deck2) {
    t_card* d1bottom = deck1->bottom;
    t_card* d2top = deck2->top;
    if (d2top == NULL) {
        if (d1bottom->suit == 13) { return 1 ;}
        else { return 0; }
    } else if (d1bottom->color != d2top->color && d1bottom->value == d2top->value-1) {
        return 1;
    } else {
        return 0;
    }
}

// return 1 if deck1 top card can move on top of foundation deck
int can_foundation_move(t_deck* deck1, t_deck* foundation) {
    t_card* d1top = deck1->top;
    t_card* foundtop = foundation->top;
    if (foundtop == NULL) {
        if (d1top->value == 1) { return 1 ;}
        else { return 0; }
    } else if (d1top->suit == foundtop->suit && d1top->value == foundtop->value+1) {
        return 1;
    } else {
        return 0;
    }
}

// Given a deck on the faceup part of the tableau, find and return other faceup deck on tableau that
// it can be moved on top of i.e. faceup->bottom can be placed on other->top
t_deck* find_move(t_deck* faceup, t_zones* zones) {
    if (faceup->ncards == 0) {
        return NULL;
    }
    t_deck* other; 
    for (int i = 0; i<7; i++) {
        other = zones->tableau_faceup[i];
        if (faceup != other && can_move(faceup, other)) {
            return other;
        }
    }
    return NULL;
}

// Given a deck on the faceup part of the zones, find and return foundation that the top card can be moved to
t_deck* find_foundation_move(t_deck* faceup, t_zones* zones) {
    if (faceup->ncards == 0) {
        return NULL;
    }
    t_deck* other; 
    for (int i = 0; i<4; i++) {
        other = zones->foundations[i];
        if (faceup != other && can_foundation_move(faceup, other)) {
            return other;
        }
    }
    return NULL;
}

// finds and executes a tableau move if possible and returns 0 if none exist
int make_tableau_move(t_zones* zones) {
    t_deck* to_move; 
    t_deck* other;
    for (int i = 0; i<7; i++) {
        to_move = zones->tableau_faceup[i];
        other = find_move(to_move, zones);
        if (other != NULL) {
            move_deck_part(to_move, other, to_move->ncards);
            // flip facedown card if possible
            if (zones->tableau_faceup[i]->ncards == 0 && zones->tableau_facedown[i]->ncards > 0) {
                move_deck_part(zones->tableau_facedown[i], zones->tableau_faceup[i], 1);
            }
            return 1;
        }
    }
    return 0;
}

int make_foundation_move(t_zones* zones) {
    t_deck* to_move = NULL;
    t_deck* other = NULL;
    if (zones->wastes->ncards > 0) {
        other = find_foundation_move(zones->wastes, zones);
        if (other != NULL) {
            move_deck_part(zones->wastes, other, 1);
            return 1;
        }
    }
    for (int i = 0; i<7; i++) {
        to_move = zones->tableau_faceup[i];
        other = find_foundation_move(to_move, zones);
        if (other != NULL) {
            move_deck_part(to_move, other, 1);
            // flip facedown card if possible
            if (zones->tableau_faceup[i]->ncards == 0 && zones->tableau_facedown[i]->ncards > 0) {
                move_deck_part(zones->tableau_facedown[i], zones->tableau_faceup[i], 1);
            }
            return 1;
        }
    }
    return 0;
}


int main(int argc, char **argv) {
    srand(time(NULL));
    //deck* deck = init_deck();
    //shuffle_deck(deck);
    //print_deck(deck);
    t_zones* zones = init_zones();
    fill_tableau(zones);

    while (1) {
        system("cls");
        print_zones(zones);
        getchar();
        make_tableau_move(zones);
        system("cls");
        print_zones(zones);
        getchar();
        make_foundation_move(zones);
    }
    
    

    
    return 0;
}