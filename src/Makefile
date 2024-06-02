CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11
LFLAGS = -L.
LDLIBS = -lcheck
GCOV_FLAGS = -fprofile-arcs -ftest-coverage

CHECK_LIST = sanitizer cpp_check style_check
OPEN_GCOV = open coverage_report.html

UNAME = $(shell uname -s)
ifeq ($(UNAME), Linux)
	LDLIBS += -lrt -lsubunit -lm -lpthread
	CHECK_LIST += valgrind
OPEN_GCOV =
endif

all: gcov_report clean

gcov_report: test
	@gcov -fb our_decimal.c
	@gcovr -r . --html --html-details -o coverage_report.html
	$(OPEN_GCOV)

test: decimal_test.o our_decimal.c
	@$(CC) $(LFLAGS) our_decimal.c decimal_test.o $(GCOV_FLAGS) -g -o $@ $(LDLIBS)
	@./test

our_decimal.a: our_decimal.o
	@ar -rcs $@ $<
	@ranlib $@

decimal_test.o: decimal_test.c
	@$(CC) -g -std=c11 -c $< -o $@

our_decimal.o: our_decimal.c
	@$(CC) $(CFLAGS) -c $< -o $@

# 	Tests
# <--------------------------------------------------------------->

check_all: $(CHECK_LIST)

valgrind: test clean
	@valgrind --leak-check=full --track-origins=yes --verbose --log-file=valgrind_out.txt ./test
	@cat valgrind_out.txt | grep ERROR

sanitizer: clean
	@gcc -Wall -Wextra -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all \
	-fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment \
	-g our_decimal.c decimal_test.c $(LDLIBS) -fprofile-arcs && ./a.out
	@rm -f *.gcno *.gcda a.out

# cpp_check:
# 	@cppcheck --enable=all --suppress=missingIncludeSystem *.c *.h

style_check:
	@cp ../materials/linters/CPPLINT.cfg ./
	@python3 ../materials/linters/cpplint.py --extensions=c ./*.c ./*.h
	@rm CPPLINT.cfg

#	Clean
# <--------------------------------------------------------------->

clean:
	@rm -f *.o *.a *.gcno *.gcda

clean_all: clean
	@rm -f .gc* *.txt *.html test *.css

rebuild: clean_all our_decimal.a
	@rm -f *.o
