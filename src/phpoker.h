#ifndef PHPOKER_H
#define PHPOKER_H

extern zend_module_entry phpoker_module_entry;
#define phpext_phpoker_ptr &phpoker_module_entry

#define PHP_PHPOKER_VERSION "1.0.5"

#if defined(ZTS) && defined(COMPILE_DL_PHPOKER)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

PHP_FUNCTION(poker_evaluate_hand);
PHP_FUNCTION(poker_calculate_equity);

PHP_MINIT_FUNCTION(phpoker);
PHP_MSHUTDOWN_FUNCTION(phpoker);
PHP_MINFO_FUNCTION(phpoker);

#endif /* PHPOKER_H */