#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "phpoker.h"
#include "arrays.h"

ZEND_BEGIN_ARG_INFO(arginfo_poker_evaluate_hand, 0)
    ZEND_ARG_INFO(0, hand)
ZEND_END_ARG_INFO()

const zend_function_entry phpoker_functions[] = {
    PHP_FE(poker_evaluate_hand, arginfo_poker_evaluate_hand)
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
    int cards[5];
    int num_cards;
    unsigned short eval_result;
    int hand_type;

    /* Parse function arguments */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &hand_str, &hand_len) == FAILURE) {
        RETURN_NULL();
    }

    /* Parse the hand string into cards */
    num_cards = parse_hand(hand_str, cards, 5);

    if (num_cards != 5) {
        php_error_docref(NULL, E_WARNING, "Invalid hand format or incorrect number of cards (need exactly 5)");
        RETURN_NULL();
    }

    /* Evaluate the hand */
    eval_result = eval_5hand(cards);

    /* Get the hand type */
    hand_type = hand_rank(eval_result);

    /* Create return array with hand info */
    array_init(return_value);
    add_assoc_long(return_value, "value", eval_result);
    add_assoc_long(return_value, "rank", hand_type);
    add_assoc_string(return_value, "name", (char*)get_hand_name(hand_type));
}