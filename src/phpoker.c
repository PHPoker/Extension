#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "phpoker.h"
#include "arrays.h"
#include "zend_exceptions.h"  /* Added for exception support */

ZEND_BEGIN_ARG_INFO(arginfo_poker_evaluate_hand, 0)
    ZEND_ARG_INFO(0, hand)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_poker_calculate_equity, 0)
    ZEND_ARG_INFO(0, hole_cards)
    ZEND_ARG_INFO(0, board_cards)
    ZEND_ARG_INFO(0, iterations)
    ZEND_ARG_INFO(0, dead_cards)
ZEND_END_ARG_INFO()

const zend_function_entry phpoker_functions[] = {
    PHP_FE(poker_evaluate_hand, arginfo_poker_evaluate_hand)
    PHP_FE(poker_calculate_equity, arginfo_poker_calculate_equity)
    PHP_FE_END
};

zend_module_entry phpoker_module_entry = {
    STANDARD_MODULE_HEADER,
    "phpoker",
    phpoker_functions,
    PHP_MINIT(phpoker),
    PHP_MSHUTDOWN(phpoker),
    NULL,
    NULL,
    PHP_MINFO(phpoker),
    PHP_PHPOKER_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHPOKER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(phpoker)
#endif

/* Card representation:
 *   +--------+--------+--------+--------+
 *   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
 *   +--------+--------+--------+--------+
 *
 * p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
 * r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
 * cdhs = suit of card
 * b = bit turned on depending on rank of card
 */

#define CLUB    0x8000
#define DIAMOND 0x4000
#define HEART   0x2000
#define SPADE   0x1000
#define RANK(x) ((x >> 8) & 0xF)

/* Each of the thirteen card ranks has its own prime number
 * Note: primes array is already defined in arrays.h, so we don't redefine it here
 */
/* extern int primes[]; */ /* The array is already accessible from arrays.h */

/* Map card characters to their numeric values */
static int char_to_rank(char c) {
    switch (c) {
        case '2': return 0;
        case '3': return 1;
        case '4': return 2;
        case '5': return 3;
        case '6': return 4;
        case '7': return 5;
        case '8': return 6;
        case '9': return 7;
        case 'T': case 't': return 8;
        case 'J': case 'j': return 9;
        case 'Q': case 'q': return 10;
        case 'K': case 'k': return 11;
        case 'A': case 'a': return 12;
        default: return -1;
    }
}

/* Map suit characters to their bit values */
static int char_to_suit(char c) {
    switch (c) {
        case 'C': case 'c': return CLUB;
        case 'D': case 'd': return DIAMOND;
        case 'H': case 'h': return HEART;
        case 'S': case 's': return SPADE;
        default: return 0;
    }
}

/* Parse a hand string into an array of card values */
static int parse_hand(const char *hand_str, int *cards, int max_cards) {
    int i, count = 0;
    size_t len = strlen(hand_str);

    for (i = 0; i < len && count < max_cards; i++) {
        char rank_c = hand_str[i];

        /* Skip spaces */
        if (rank_c == ' ' || rank_c == '\t') {
            continue;
        }

        /* Need two characters for a card */
        if (i + 1 >= len) {
            break;
        }

        char suit_c = hand_str[i + 1];

        int rank = char_to_rank(rank_c);
        int suit = char_to_suit(suit_c);

        if (rank < 0 || suit == 0) {
            /* Invalid card */
            return -1;
        }

        /* Create card value in the format expected by the evaluator */
        cards[count++] = (1 << (16 + rank)) | (rank << 8) | suit | primes[rank];

        /* Skip the suit character we just processed */
        i++;
    }

    return count;
}

/* Check if a card exists in a deck */
static int card_exists(int card, int *deck, int num_cards) {
    int i;
    for (i = 0; i < num_cards; i++) {
        if (deck[i] == card) {
            return 1;
        }
    }
    return 0;
}

/* Perform a perfect hash lookup (courtesy of Paul Senzee) */
static unsigned find_fast(unsigned u) {
    unsigned a, b, r;

    u += 0xe91aaa35;
    u ^= u >> 16;
    u += u << 8;
    u ^= u >> 4;
    b = (u >> 8) & 0x1ff;
    a = (u + (u << 2)) >> 19;
    r = a ^ hash_adjust[b];
    return r;
}

/* Evaluate 5 cards */
static unsigned short eval_5cards(int c1, int c2, int c3, int c4, int c5) {
    int q = (c1 | c2 | c3 | c4 | c5) >> 16;
    short s;

    /* Check for flushes and straight flushes */
    if (c1 & c2 & c3 & c4 & c5 & 0xf000)
        return flushes[q];

    /* Check for straights and high card hands */
    if ((s = unique5[q]))
        return s;

    /* Perform a perfect-hash lookup for remaining hands */
    q = (c1 & 0xff) * (c2 & 0xff) * (c3 & 0xff) * (c4 & 0xff) * (c5 & 0xff);
    return hash_values[find_fast(q)];
}

/* Evaluate a hand of 5 cards */
static unsigned short eval_5hand(int *hand) {
    return eval_5cards(hand[0], hand[1], hand[2], hand[3], hand[4]);
}

/* Evaluate a hand of 7 cards - finds the best 5-card hand */
static unsigned short eval_7hand(int *hand) {
    int i, j, subhand[5];
    unsigned short q, best = 9999;

    for (i = 0; i < 21; i++) {
        for (j = 0; j < 5; j++)
            subhand[j] = hand[perm7[i][j]];
        q = eval_5hand(subhand);
        if (q < best)
            best = q;
    }
    return best;
}

/* Return the hand rank based on the evaluation value */
static int hand_rank(unsigned short val) {
    if (val > 6185) return 9; /* High card */
    if (val > 3325) return 8; /* One pair */
    if (val > 2467) return 7; /* Two pair */
    if (val > 1609) return 6; /* Three of a kind */
    if (val > 1599) return 5; /* Straight */
    if (val > 322)  return 4; /* Flush */
    if (val > 166)  return 3; /* Full house */
    if (val > 10)   return 2; /* Four of a kind */
    return 1;                 /* Straight flush */
}

/* Get string representation of hand rank */
static const char* get_hand_name(int rank) {
    switch(rank) {
        case 1: return "Straight Flush";
        case 2: return "Four of a Kind";
        case 3: return "Full House";
        case 4: return "Flush";
        case 5: return "Straight";
        case 6: return "Three of a Kind";
        case 7: return "Two Pair";
        case 8: return "One Pair";
        case 9: return "High Card";
        default: return "Unknown";
    }
}

/* Initialize a deck with all 52 cards */
static void init_deck(int *deck) {
    int i, j, n = 0, suit = 0x8000;

    for (i = 0; i < 4; i++, suit >>= 1)
        for (j = 0; j < 13; j++, n++) {
            deck[n] = primes[j] | (j << 8) | suit | (1 << (16+j));
        }
}

/* Initialize random seed, if necessary */
static void init_random_seed() {
    static int seeded = 0;

    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
}

/* Random number between 0 and limit-1 */
static int rand_int(int limit) {
    return (int)(((double)rand() / RAND_MAX) * limit);
}

/* Shuffle a deck of cards */
static void shuffle_deck(int *deck, int num_cards) {
    int i, j, temp;

    for (i = num_cards - 1; i > 0; i--) {
        j = rand_int(i + 1);
        temp = deck[j];
        deck[j] = deck[i];
        deck[i] = temp;
    }
}

/* Remove cards from the deck that are already in play */
static int prepare_deck(int *deck, int *used_cards, int num_used_cards) {
    int full_deck[52];
    int i, count = 0;

    /* Initialize full deck */
    init_deck(full_deck);

    /* Copy cards to deck that are not used */
    for (i = 0; i < 52; i++) {
        if (!card_exists(full_deck[i], used_cards, num_used_cards)) {
            /* Ensure we don't go out of bounds */
            if (count < 52) {
                deck[count++] = full_deck[i];
            }
        }
    }

    return count;
}

/* PHP Module initialization */
PHP_MINIT_FUNCTION(phpoker)
{
    return SUCCESS;
}

/* PHP Module shutdown */
PHP_MSHUTDOWN_FUNCTION(phpoker)
{
    return SUCCESS;
}

/* PHP Module info */
PHP_MINFO_FUNCTION(phpoker)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "phpoker support", "enabled");
    php_info_print_table_row(2, "Version", PHP_PHPOKER_VERSION);
    php_info_print_table_row(2, "Author", "Nick Poulos");
    php_info_print_table_end();
}

/* The poker_evaluate_hand() function */
PHP_FUNCTION(poker_evaluate_hand)
{
    char *hand_str;
    size_t hand_len;
    int cards[7]; /* Increased max size to 7 */
    int num_cards;
    unsigned short eval_result;
    int hand_type;

    /* Parse function arguments */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &hand_str, &hand_len) == FAILURE) {
        RETURN_NULL();
    }

    /* Parse the hand string into cards */
    num_cards = parse_hand(hand_str, cards, 7); /* Increased max size to 7 */

    /* Check for invalid cards */
    if (num_cards == -1) {
        zend_throw_exception(zend_ce_exception, "Invalid card format in hand", 0);
        RETURN_NULL();
    }

    /* Check for correct number of cards */
    if (num_cards != 5 && num_cards != 7) {
        zend_throw_exception(zend_ce_exception, "Invalid number of cards (need exactly 5 or 7)", 0);
        RETURN_NULL();
    }

    /* Evaluate the hand */
    if (num_cards == 5) {
        eval_result = eval_5hand(cards);
    } else { /* Must be 7 */
        eval_result = eval_7hand(cards);
    }

    /* Get the hand type */
    hand_type = hand_rank(eval_result);

    /* Create return array with hand info */
    array_init(return_value);
    add_assoc_long(return_value, "value", eval_result);
    add_assoc_long(return_value, "rank", hand_type);
    add_assoc_string(return_value, "name", (char*)get_hand_name(hand_type));
    add_assoc_long(return_value, "cards", num_cards); /* Add card count to output */
}

/* The poker_calculate_equity() function */
PHP_FUNCTION(poker_calculate_equity)
{
    zval *hole_cards_array, *board_cards_array = NULL, *dead_cards_array = NULL;
    HashTable *hole_cards_hash, *board_cards_hash = NULL, *dead_cards_hash = NULL;
    zval *hole_cards_item;
    zval *board_cards_item;
    zval *dead_cards_item;
    zend_long iterations = 10000;

    int num_players, i, j, h, b, d;
    int board_count = 0;
    int dead_count = 0;
    int used_cards_count = 0;
    int remaining_board_count = 0;
    int remaining_deck_count = 0;
    int player_hands[10][7]; /* Max 10 players, 7 cards per hand */
    int board_cards[5];      /* Max 5 board cards */
    int dead_cards[52];      /* Max 52 dead cards (though this is overkill) */
    int used_cards[52];      /* Track all cards in use */
    int deck[52];            /* Remaining cards in the deck */
    int wins[10] = {0};      /* Win counter for each player */
    int ties[10] = {0};      /* Tie counter for each player */
    unsigned short scores[10]; /* Score for each player in a given hand */
    double equity[10] = {0.0}; /* Equity percentage for each player */

    /* Initialize the random seed for shuffling */
    init_random_seed();

    /* Parse function arguments */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a|a!l!a!", &hole_cards_array, &board_cards_array,
                              &iterations, &dead_cards_array) == FAILURE) {
        RETURN_NULL();
    }

    /* Get hash tables from zvals */
    hole_cards_hash = Z_ARRVAL_P(hole_cards_array);
    num_players = zend_hash_num_elements(hole_cards_hash);

    /* Check if we have at least 2 players */
    if (num_players < 2) {
        zend_throw_exception(zend_ce_exception, "At least 2 players needed for equity calculation", 0);
        RETURN_NULL();
    }

    /* Check for reasonable iteration count */
    if (iterations <= 0) {
        iterations = 10000; /* Default to 10,000 if invalid */
    } else if (iterations > 1000000) {
        iterations = 1000000; /* Cap at 1,000,000 for performance */
    }

    /* Get board cards if provided */
    if (board_cards_array != NULL && Z_TYPE_P(board_cards_array) == IS_ARRAY) {
        board_cards_hash = Z_ARRVAL_P(board_cards_array);
        board_count = zend_hash_num_elements(board_cards_hash);

        /* Ensure we don't have too many board cards */
        if (board_count > 5) {
            zend_throw_exception(zend_ce_exception, "Board cannot have more than 5 cards", 0);
            RETURN_NULL();
        }
    }

    /* Get dead cards if provided */
    if (dead_cards_array != NULL && Z_TYPE_P(dead_cards_array) == IS_ARRAY) {
        dead_cards_hash = Z_ARRVAL_P(dead_cards_array);
        dead_count = zend_hash_num_elements(dead_cards_hash);
    }

    /* Parse the hole cards for each player */
    i = 0;
    ZEND_HASH_FOREACH_VAL(hole_cards_hash, hole_cards_item) {
        if (i >= 10) break; /* Limit to 10 players for sanity */

        if (Z_TYPE_P(hole_cards_item) != IS_STRING) {
            zend_throw_exception(zend_ce_exception, "Hole cards must be strings", 0);
            RETURN_NULL();
        }

        int num_hole_cards = parse_hand(Z_STRVAL_P(hole_cards_item), player_hands[i], 2);

        if (num_hole_cards != 2) {
            zend_throw_exception(zend_ce_exception, "Each player must have exactly 2 hole cards", 0);
            RETURN_NULL();
        }

        /* Add player hole cards to used cards array */
        used_cards[used_cards_count++] = player_hands[i][0];
        used_cards[used_cards_count++] = player_hands[i][1];

        i++;
    } ZEND_HASH_FOREACH_END();

    num_players = i; /* In case we hit the 10 player limit */

    /* Parse the board cards */
    if (board_count > 0) {
        i = 0;
        ZEND_HASH_FOREACH_VAL(board_cards_hash, board_cards_item) {
            if (i >= 5) break; /* Limit to 5 board cards */

            if (Z_TYPE_P(board_cards_item) != IS_STRING) {
                zend_throw_exception(zend_ce_exception, "Board cards must be strings", 0);
                RETURN_NULL();
            }

            int num_parsed = parse_hand(Z_STRVAL_P(board_cards_item), &board_cards[i], 1);

            if (num_parsed != 1) {
                zend_throw_exception(zend_ce_exception, "Invalid board card format", 0);
                RETURN_NULL();
            }

            /* Check for duplicates */
            if (card_exists(board_cards[i], used_cards, used_cards_count)) {
                zend_throw_exception(zend_ce_exception, "Duplicate card found on board", 0);
                RETURN_NULL();
            }

            /* Add to used cards */
            used_cards[used_cards_count++] = board_cards[i];

            i++;
        } ZEND_HASH_FOREACH_END();

        board_count = i;
    }

    /* Parse the dead cards */
    if (dead_count > 0) {
        i = 0;
        ZEND_HASH_FOREACH_VAL(dead_cards_hash, dead_cards_item) {
            if (i >= 52) break; /* Safety limit */

            if (Z_TYPE_P(dead_cards_item) != IS_STRING) {
                zend_throw_exception(zend_ce_exception, "Dead cards must be strings", 0);
                RETURN_NULL();
            }

            /* Parse single card from the string */
            int temp_card[1];
            int num_parsed = parse_hand(Z_STRVAL_P(dead_cards_item), temp_card, 1);

            if (num_parsed != 1) {
                zend_throw_exception(zend_ce_exception, "Invalid dead card format", 0);
                RETURN_NULL();
            }

            /* Store the parsed card */
            dead_cards[i] = temp_card[0];

            /* Check for duplicates with used cards */
            if (card_exists(dead_cards[i], used_cards, used_cards_count)) {
                zend_throw_exception(zend_ce_exception, "Dead card already in use by player or on board", 0);
                RETURN_NULL();
            }

            /* Add to used cards */
            used_cards[used_cards_count++] = dead_cards[i];

            i++;
        } ZEND_HASH_FOREACH_END();

        dead_count = i;
    }

    /* Calculate how many more board cards we need to deal */
    remaining_board_count = 5 - board_count;

    /* Prepare the deck (remove all used cards) */
    remaining_deck_count = prepare_deck(deck, used_cards, used_cards_count);

    /* Check if we have enough cards left */
    if (remaining_deck_count < remaining_board_count) {
        zend_throw_exception(zend_ce_exception, "Not enough cards left in deck after removing used/dead cards", 0);
        RETURN_NULL();
    }

    /* Main simulation loop */
    for (i = 0; i < iterations; i++) {
        /* Shuffle the deck for this iteration */
        shuffle_deck(deck, remaining_deck_count);

        /* Deal remaining board cards */
        for (j = 0; j < remaining_board_count; j++) {
            board_cards[board_count + j] = deck[j];
        }

        /* Evaluate each player's hand */
        for (j = 0; j < num_players; j++) {
            /* Combine hole cards with board cards to form a 7-card hand */
            int full_hand[7];
            full_hand[0] = player_hands[j][0];
            full_hand[1] = player_hands[j][1];

            /* Add the board cards */
            for (h = 0; h < 5; h++) {
                full_hand[2 + h] = board_cards[h];
            }

            /* Evaluate the hand */
            scores[j] = eval_7hand(full_hand);
        }

        /* Find the best score (lowest value is best) */
        unsigned short best_score = 9999;
        int winners = 0;
        int winner_indices[10] = {0};

        for (j = 0; j < num_players; j++) {
            if (scores[j] < best_score) {
                best_score = scores[j];
                winners = 1;
                winner_indices[0] = j;
            } else if (scores[j] == best_score) {
                winner_indices[winners++] = j;
            }
        }

        /* Update win/tie counters */
        if (winners == 1) {
            wins[winner_indices[0]]++;
        } else {
            /* It's a tie between multiple players */
            for (j = 0; j < winners; j++) {
                ties[winner_indices[j]]++;
            }
        }
    }

    /* Calculate equity percentages */
    for (i = 0; i < num_players; i++) {
        equity[i] = (double)(wins[i] + (ties[i] / (double)2)) / iterations * 100.0;
    }

    /* Return the results */
    array_init(return_value);

    for (i = 0; i < num_players; i++) {
        zval player_result;
        array_init(&player_result);

        add_assoc_double(&player_result, "equity", equity[i]);
        add_assoc_long(&player_result, "wins", wins[i]);
        add_assoc_long(&player_result, "ties", ties[i]);

        add_next_index_zval(return_value, &player_result);
    }
}