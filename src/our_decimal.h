#ifndef SRC_our_DECIMAL_H_
#define SRC_our_DECIMAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct {
    unsigned int bits[4];
} our_decimal;

typedef struct {
    unsigned int bits[8];
} our_bigDecimal;

enum deci_errors{
    OK,
    TO_LARGE,
    TO_SMALL,
    DIV_ZERO
};

enum round_type{
    NORMAL,
    BANKING,
    TO_FLOOR,
    CUT
};

int our_add(our_decimal value_1, our_decimal value_2, our_decimal *result);
int our_sub(our_decimal value_1, our_decimal value_2, our_decimal *result);
int our_mul(our_decimal value_1, our_decimal value_2, our_decimal *result);
int our_div(our_decimal value_1, our_decimal value_2, our_decimal *result);
int our_mod(our_decimal value_1, our_decimal value_2, our_decimal *result);

int our_is_less(our_decimal value_1, our_decimal value_2);
int our_is_less_or_equal(our_decimal value_1, our_decimal value_2);
int our_is_greater(our_decimal value_1, our_decimal value_2);
int our_is_greater_or_equal(our_decimal value_1, our_decimal value_2);
int our_is_equal(our_decimal value_1, our_decimal value_2);
int our_is_not_equal(our_decimal value_1, our_decimal value_2);

int our_from_int_to_decimal(int src, our_decimal *dst);
int our_from_float_to_decimal(float src, our_decimal *dst);
int our_from_decimal_to_int(our_decimal src, int *dst);
int our_from_decimal_to_float(our_decimal src, float *dst);

int our_floor(our_decimal value, our_decimal *result);
int our_round(our_decimal value, our_decimal *result);
int our_truncate(our_decimal value, our_decimal *result);
int our_negate(our_decimal value, our_decimal *result);

/* Служебные функции */
/* Битовые операции  */
bool get_bit(our_bigDecimal bigDecimal, int position);
void set_bit(our_bigDecimal *bigDecimal, int position, bool value);
bool get_sign(our_bigDecimal bigDecimal);
void set_sign(our_bigDecimal *bigDecimal, bool value);
void switch_sign(our_bigDecimal *bigDecimal);
int get_exp(our_bigDecimal bigDecimal);
void set_exp(our_bigDecimal *bigDecimal, int exp);
our_bigDecimal left_shift(our_bigDecimal bigDecimal, int shift);
our_bigDecimal right_shift(our_bigDecimal bigDecimal, int shift);
int first_true_bit(our_bigDecimal bigDecimal);

/* Операции над мантиссой */
int sum_mantissa(our_bigDecimal value_1, our_bigDecimal value_2, our_bigDecimal *result);
void sub_mantissa(our_bigDecimal value_1, our_bigDecimal value_2, our_bigDecimal* result);
our_bigDecimal div_mantissa(our_bigDecimal devidend, our_bigDecimal devider, our_bigDecimal *result);
int increment(our_bigDecimal *bigDecimal);
int normalize(our_bigDecimal devidend, our_bigDecimal *devider);
bool is_less_bigDeci(our_bigDecimal bigVal_1, our_bigDecimal bigVal_2);

/* Операции со степенью */
void exp_alignment(our_bigDecimal *value_1, our_bigDecimal *value_2);
int mul_by_10(our_bigDecimal *bigDecimal);
int div_by_10(our_bigDecimal *decimal);
void round_by_10(our_bigDecimal *bigDecimal, int n, int round_type);
int first_digit(our_bigDecimal bigDecimal);

/* Конвертеры */
void convert_toBig(our_decimal decimal, our_bigDecimal *bigDecimal);
int convert_fromBig(our_bigDecimal bigDecimal, our_decimal *decimal);

#endif  // SRC_our_DECIMAL_H_
