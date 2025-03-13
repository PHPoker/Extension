dnl config.m4 for extension phpoker

PHP_ARG_ENABLE(phpoker, whether to enable phpoker support,
[  --enable-phpoker        Enable phpoker support])

if test "$PHP_PHPOKER" != "no"; then
  PHP_NEW_EXTENSION(phpoker, phpoker.c, $ext_shared)
fi