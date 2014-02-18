cd target
valgrind --track-origins=yes --leak-check=full --suppressions=./../tools/valgrind.supp ./ia
