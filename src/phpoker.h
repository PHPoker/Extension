#ifndef PHPOKER_H
#define PHPOKER_H

extern zend_module_entry phpoker_module_entry;
#define phpext_phpoker_ptr &phpoker_module_entry

#define PHP_PHPOKER_VERSION "1.0.5"

#if defined(ZTS) && defined(COMPILE_DL_PHPOKER)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

/* Declare user functions */
PHP_FUNCTION(poker_evaluate_hand);
PHP_FUNCTION(poker_calculate_equity);

/* Module functions */
PHP_MINIT_FUNCTION(phpoker);
PHP_MSHUTDOWN_FUNCTION(phpoker);
PHP_MINFO_FUNCTION(phpoker);

/* Internal function declarations */
static int php_poker_char_to_rank(char c);
static int php_poker_char_to_suit(char c);
static int php_poker_parse_hand(const char *hand_str, int *cards, int max_cards);
static int php_poker_card_exists(int card, int *deck, int num_cards);
static unsigned php_poker_find_fast(unsigned u);
static unsigned short php_poker_eval_5cards(int c1, int c2, int c3, int c4, int c5);
static unsigned short php_poker_eval_5hand(int *hand);
static unsigned short php_poker_eval_7hand(int *hand);
static int php_poker_hand_rank(unsigned short val);
static const char* php_poker_get_hand_name(int rank);
static void php_poker_init_deck(int *deck);
static void php_poker_init_random_seed();
static int php_poker_rand_int(int limit);
static void php_poker_shuffle_deck(int *deck, int num_cards);
static int php_poker_prepare_deck(int *deck, int *used_cards, int num_used_cards);

#endif /* PHPOKER_H */