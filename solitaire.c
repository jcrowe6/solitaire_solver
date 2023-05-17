#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct t_card {
    int value;
    int suit;
    int color;
    struct t_card* below; // pointer to card that this card is on top of, NULL if it's just sitting on the table
    struct t_card* above; // pointer to card above this card, NULL if none
} t_card;

// just pointers to the top and bottom cards
// must ensure that top->above and bottom->below is NULL, and top<->bottom is doubly linked list
typedef struct t_deck {
    t_card* top;
    t_card* bottom;
    int ncards;

    t_card* card_mem; // when we malloc space for all the cards, we need to save this pointer to free later
                      // this will only be set for the draw deck, all others that start empty will just get NULL
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
    deck->card_mem = cards;
    return deck;
}

void free_deck(t_deck* deck) {
    if (deck->card_mem != NULL) {
        free(deck->card_mem);
    }
    free(deck);
}

// allocates space for and returns an empty deck
t_deck* init_empty_deck() {
    t_deck* deck = malloc(sizeof(t_deck));
    deck->top = NULL;
    deck->bottom = NULL;
    deck->ncards = 0;
    deck->card_mem = NULL;
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
            t_card* fd_top = fromdeck->top;
            fromdeck->top = move_bottom->below;
            fromdeck->top->above = NULL;
            todeck->top->above = move_bottom;
            move_bottom->below = todeck->top;
            todeck->top = fd_top;
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

void output_deck(t_deck* deck) {
    t_card* iter = deck->top;
    while (iter) {
        printf("%d:%d ", iter->value, iter->suit);
        iter = iter->below;
    }
    printf("\n");
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

void free_zones(t_zones* zones) {
    free_deck(zones->draw);
    free_deck(zones->wastes);
    for (int i = 0; i < 7; i++) {
        free_deck(zones->tableau_facedown[i]);
        free_deck(zones->tableau_faceup[i]);
    }
    for (int i = 0; i < 4; i++) {
        free_deck(zones->foundations[i]);
    }
    free(zones);
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
        if (d1bottom->value == 13) { return 1 ;}
        else { return 0; }
    } else if (d1bottom->color != d2top->color && d1bottom->value == d2top->value-1) {
        return 1;
    } else {
        return 0;
    }
}

// return 1 if card can move on top of deck2
int can_move_card(t_card* card, t_deck* deck2) {
    t_card* d1bottom = card;
    t_card* d2top = deck2->top;
    if (d2top == NULL) {
        if (d1bottom->value == 13) { return 1 ;}
        else { return 0; }
    } else if (d1bottom->color != d2top->color && d1bottom->value == d2top->value-1) {
        return 1;
    } else {
        return 0;
    }
}

int can_top_move(t_deck* deck1, t_deck* deck2) {
    t_card* d1top = deck1->top;
    t_card* d2top = deck2->top;
    if (d2top == NULL) {
        if (d1top->suit == 13) { return 1 ;}
        else { return 0; }
    } else if (d1top->color != d2top->color && d1top->value == d2top->value-1) {
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
// tab_i is the int such that faceup == zones->tableau_faceup[tab_i]
// this was added to easily find the respective facedown deck to check if it has cards, in the case of moving a King card
// (since if K could move without any facedown cards in it's column, we will repeatedly move K's back and forth to empty decks)
t_deck* find_move(t_deck* faceup, t_zones* zones, int tab_i) {
    if (faceup->ncards == 0) {
        return NULL;
    }
    t_deck* other; 
    for (int i = 0; i<7; i++) {
        other = zones->tableau_faceup[i];
        if (faceup != other && can_move(faceup, other)) {
            if (faceup->bottom->value == 13 && other->top == NULL && zones->tableau_facedown[tab_i]->ncards == 0) { 
                return NULL; 
            } // prevent pointless King moves
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
    if (zones->wastes->ncards > 0) {
        for (int i = 0; i<7; i++) { // quick little findmove for wastes
            other = zones->tableau_faceup[i];
            if (can_top_move(zones->wastes, other)) {
                // printf("move ");
                // print_card(zones->wastes->top);
                // printf(" fw to tableau ");
                // print_cardn(other->top);

                move_deck_part(zones->wastes, other, 1);
                return 1;
            }
        }
    }
    for (int i = 0; i<7; i++) {
        to_move = zones->tableau_faceup[i];
        other = find_move(to_move, zones, i);
        if (other != NULL) {
            // printf("move ");
            // print_card(to_move->bottom);
            //printf(" to tableau ");
            //print_cardn(other->top);

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
            //printf("move ");
            //print_card(zones->wastes->top);
            //printf(" to foundation ");
            //print_cardn(other->top);

            move_deck_part(zones->wastes, other, 1);
            return 1;
        }
    }
    for (int i = 0; i<7; i++) {
        to_move = zones->tableau_faceup[i];
        other = find_foundation_move(to_move, zones);
        if (other != NULL) {
            //printf("move ");
            //print_card(to_move->top);
            //printf(" to foundation ");
            //print_cardn(other->top);

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

void flip_deck(t_deck* deck) {
    t_card* currcard = deck->top;
    for (int i = 0; i < deck->ncards; i++) {
        t_card* below = currcard->below;
        currcard->below = currcard->above;
        currcard->above = below;
        currcard = currcard->above;
    }
    t_card* bot = deck->bottom;
    deck->bottom = deck->top;
    deck->top = bot;
}

// return 1 if had to flip, else 0
int drawn(t_zones* zones, int n) { 
    if (zones->draw->ncards <= 0) {return -1; }
    else { 
        for (int i = 0; i < n; i++) {
            if (zones->draw->ncards > 0) {
                // printf("moving 1 card from draw to wastes... \n");
                move_deck_part(zones->draw, zones->wastes, 1); 
            }
        }
    }
    return 0;
}

int flip(t_zones* zones) {
    if (zones->wastes->ncards <= 0 || zones->draw->ncards != 0) { return -1; }
    else {
        move_deck_part(zones->wastes, zones->draw, zones->wastes->ncards);
        flip_deck(zones->draw);
        return 0;
    }
}

int check_win(t_zones* zones) {
    for (int i = 0; i < 4; i++) {
        if (zones->foundations[i]->ncards != 13) {
            return 0;
        }
    }
    return 1;
}

void output_state(t_zones* zones) {
    output_deck(zones->draw);
    output_deck(zones->wastes);
    for (int i = 0; i<4; i++) {
        output_deck(zones->foundations[i]);
    }
    for (int i = 0; i<7; i++) {
        printf("%d \n", zones->tableau_facedown[i]->ncards);
    }
    for (int i = 0; i<7; i++) {
        output_deck(zones->tableau_faceup[i]);
    }
}

// Output list of possible actions in [ D | F | {fromdeck}:{cardsFromTop}:{todeck} ] format
// D means draw a cards and will be an option if draw pile has cards
// F means flip the wastes and will be an option if the draw pile has no cards
// Otherwise, we're moving some cards around
// fromdeck could be "W" for wastes, "TN" for Nth faceup tableau, "FN" for Nth foundation
// todeck could be "TN" or "FN"
// cardsFromTop is how many we're moving. if fromdeck is W or FN, this must be 1.
// If fromdeck is TN, cardsFromTop can be > 1 but toDeck must be TN
void output_actions(t_zones* zones) {
    if (zones->draw->ncards > 0) { // we can draw
        printf("D " );
    } else {
        printf("F ");
    }
    if (zones->wastes->ncards > 0) { // can we move wastes top anywhere?
        for (int i = 0; i < 7; i++) {
            if (can_top_move(zones->wastes, zones->tableau_faceup[i])) {
                printf("W:1:T%d ", i);
            }
        }
        for (int i = 0; i < 4; i++) {
            if (can_foundation_move(zones->wastes, zones->foundations[i])) {
                printf("W:1:F%d ", i);
            }
        }
    }
    // now check all tableaus, all positions, if we can move it
    for (int t1 = 0; t1 < 7; t1++) {
        t_card* card = zones->tableau_faceup[t1]->top;
        int n = zones->tableau_faceup[t1]->ncards;
        for (int i = 0; i < n; i++) {
            for (int t2 = 0; t2 < 7; t2++) {
                if (t2 != t1 && can_move_card(card, zones->tableau_faceup[t2])) {
                    printf("T%d:%d:T%d ", t1, i+1, t2);
                }
            }
            card = card->below;
        }
    }
    // now check all top of tableaus to move to foundations
    for (int t1 = 0; t1 < 7; t1++) {
        for (int i = 0 ; i < 4; i++) {
            if (can_foundation_move(zones->tableau_faceup[t1], zones->foundations[i])) {
                printf("T%d:1:F%d ",t1,i);
            }
            if (zones->foundations[i]->top && can_top_move(zones->foundations[i], zones->tableau_faceup[t1])) {
                printf("F%d:1:T%d ",i,t1);
            }
        }
    }
    printf("\n");
}

// Loops the game actions with a simple decision tree of:
// 1. Make all possible tableau moves
// 2. Make all possible moves to foundations
// 3. If 1. and 2. had no moves, draw cards. Go to 1.
// This is an agent. But not a good one! It's winrate is about 0.0071
int play_game(int verbose) {
    t_zones* zones = init_zones();
    fill_tableau(zones);

    int hasmove = 1;
    int tab_move = 0;
    int found_move = 0;
    int win = 0;
    int flipped = 0;
    while (win == 0) {
        tab_move = 0;
        found_move = 0;

        hasmove = 1;
        if (verbose) {
            print_zones(zones);
            printf("executing all tableau moves\n");
            output_actions(zones);
         
            getchar();
        }
        while (hasmove) {
            hasmove = make_tableau_move(zones);
            if (hasmove == 1) {
                tab_move = hasmove;
                if (flipped == 1) { // we had to flip before but we just found a move
                    flipped = 0;
                }
            }
        }

        hasmove = 1;
        if (verbose) {
            print_zones(zones);
            printf("executing all foundation moves\n");
        }
        while (hasmove) {
            hasmove = make_foundation_move(zones);
            if (hasmove == 1) {
                found_move = hasmove;
                if (flipped == 1) { // we had to flip before but we just found a move
                    flipped = 0;
                }
            }
        }

        if (zones->draw->ncards == 0 && check_win(zones)) {
            return 1;
        }

        if (verbose) {
            print_zones(zones);
            printf("Tableau moved? - %d. Foundation moved? - %d\nIf both 0, drawing 3\n", tab_move, found_move);
        }

        if (tab_move == 0 && found_move == 0) {
            int result = drawn(zones, 3);
            if (result == 1 && flipped == 0) {
                flipped = 1;
            } else if (result == 1 && flipped == 1) { // we flipped and didn't find any moves. we're stuck (?)
                printf("lose :< \n");
                output_state(zones);
                free_zones(zones);
                return 0; // no win
            }
        }

    }
}

int execute_move(char* move, t_zones* zones) {
    if (move[0] == 'D') {
        drawn(zones, 1);
        return 0;
    } else if (move[0] == 'F') {
        flip(zones);
        return 0;
    }
    // We are moving cards, either from wastes, tableau, or foundations
    // to somewher eon tableau or foundations. parse out the from and to decks

    char tok1[4]; // buffer for parsing out token 1 (the from location)
    char tok2[4]; // 
    char tok3[4];
    strcpy(tok1, strtok(move, ":")); // copy out the tokens
    strcpy(tok2, strtok(NULL, ":"));
    strcpy(tok3, strtok(NULL, ":"));

    t_deck* from_deck;
    t_deck* to_deck;
    if (tok1[0] == 'W') {
        from_deck = zones->wastes; 
    } else if (tok1[0] == 'T') {
        from_deck = zones->tableau_faceup[atoi(tok1+1)];
    } else if (tok1[0] == 'F') {
        from_deck = zones->foundations[atoi(tok1+1)];
    } else {return -1; }

    if (tok3[0] == 'T') {
        to_deck = zones->tableau_faceup[atoi(tok3+1)];
    } else if (tok3[0] == 'F') {
        to_deck = zones->foundations[atoi(tok3+1)];
    } else {return -1; }

    int n_cards = atoi(tok2);

    move_deck_part(from_deck, to_deck, n_cards); // move it
    // for now, I'm trusting that the from,to,ncards here represents a legal
    // solitaire move, since in theory, the agent should only ever
    // select actions from the list provided by output_actions, which 
    // should all be legal moves.

    // If fromdeck was on tableau, check if a facedown card needs to flip
    if (tok1[0] == 'T') {
        int i = atoi(tok1+1);
        if (zones->tableau_faceup[i]->ncards == 0 && zones->tableau_facedown[i]->ncards > 0) {
            move_deck_part(zones->tableau_facedown[i], zones->tableau_faceup[i], 1);
        }
    }
    return 0;
}

int bot_play_game() {
    t_zones* zones = init_zones();
    fill_tableau(zones);

    char move[32]; // formatted move string from input (ex T1:0:F2)
    
    while (1) {
        // 1. Output state and legal actions
        output_state(zones);
        output_actions(zones);

        // 2. Get the action from command line
        fgets(move, 32, stdin); // get move string from stdin
        
        // 3. Execute action
        execute_move(move, zones);
    }

    free_zones(zones);
}

// It is at this point I can see that the win rate of the basic strategy implemented in play_game
// is less than 1% - when 80% of games are winnable!
// So, we need to abstract out the decisionmaking process a bit. Could play around with different decision trees
// and heuristics, and numerically verify what works... but that seems a bit of a lot of effort and learning about how 
// solitaire works. but could be fun I guess..? OR I figure out how to run ~ MACHINE LEARNING ~ on this.
// It would need to take in the board state, possibly the seen cards in the draw, and output it's action.
// Need to read about how to do ML in problems like this...
int main(int argc, char **argv) {
    int verbose = 0;
    if (argc > 1 && argv[1][0] == 'v') { verbose = 1; }
    setbuf(stdout, NULL);
    srand(time(NULL));
    
    /**
    int wins = 0;
    int ngames = 10000;
    for (int i = 0; i < ngames; i++) {
        wins += play_game(verbose);
    }

    printf("Won %d games out of %d\n%.4lf winrate", wins, ngames, (double) wins / (double) ngames);
    */
    
    int ret;
    ret = bot_play_game();
    printf("game over, ret = %d\n", ret);
    
    /**
    char move[32]; // formatted move string from input (ex T1:0:F2)
    
    fgets(move, 32, stdin);
    printf("hello\n");
    */
    return 0;
}