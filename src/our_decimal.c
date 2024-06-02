#include "our_decimal.h"

#define SIGN_POS 255
#define BIG_DECI_BITS 224
#define DECI_BITS 96

#define StopIf(assertion, error_action, ...) \
    if (assertion) { fprintf(stderr, __VA_ARGS__); error_action; }

#define InvertBits(bigDecimal) \
  for (int i = 0; i < 7; i++) { (bigDecimal).bits[i] = ~(bigDecimal).bits[i]; }

#define IsZero(bigDecimal) (first_true_bit(bigDecimal) == -1)

#define ZeroToZero(bigDecimal) { \
  if (IsZero(*(bigDecimal))) { ClearDecimal((bigDecimal), our_bigDecimal); } \
}

#define SwapByTemp(var1, var2, type) { \
  type temp = var1; \
  var1 = var2; \
  var2 = temp; \
}

#define ConvertToBig(value_1, value_2, result) { \
  ClearDecimal(&bigResult, our_bigDecimal); \
  convert_toBig(value_1, &bigVal_1); \
  convert_toBig(value_2, &bigVal_2); \
}

#define ClearDecimal(decimal, type) memset(decimal, 0, sizeof(type));

#define ClearMantissa(bigDecimal) for (int i = 0; i < 7; i++) { (bigDecimal).bits[i] = 0; }

/* Арифметические операторы */

int our_add(our_decimal value_1, our_decimal value_2, our_decimal *result) {
  int error_flag = 0;
  our_bigDecimal bigVal_1, bigVal_2, bigResult;
  ConvertToBig(value_1, value_2, result);
  if (get_sign(bigVal_1) == get_sign(bigVal_2)) {
    exp_alignment(&bigVal_1, &bigVal_2);
    set_exp(&bigResult, get_exp(bigVal_1));
    set_sign(&bigResult, get_sign(bigVal_1));
    sum_mantissa(bigVal_1, bigVal_2, &bigResult);
    error_flag = convert_fromBig(bigResult, result);
  } else {
    switch_sign(&bigVal_2);
    convert_fromBig(bigVal_2, &value_2);
    error_flag = our_sub(value_1, value_2, result);
  }
  return error_flag;
}

int our_sub(our_decimal value_1, our_decimal value_2, our_decimal *result) {
  int error_flag = 0;
  our_bigDecimal bigVal_1, bigVal_2, bigResult;
  ConvertToBig(value_1, value_2, result);
  if (get_sign(bigVal_1) == get_sign(bigVal_2)) {
    exp_alignment(&bigVal_1, &bigVal_2);
    set_exp(&bigResult, get_exp(bigVal_1));
    set_sign(&bigResult, get_sign(bigVal_1));
    if (is_less_bigDeci(bigVal_1, bigVal_2)) {
      SwapByTemp(bigVal_1, bigVal_2, our_bigDecimal);
      switch_sign(&bigResult);
    }
    sub_mantissa(bigVal_1, bigVal_2, &bigResult);
    error_flag = convert_fromBig(bigResult, result);
  } else {
    switch_sign(&bigVal_2);
    convert_fromBig(bigVal_2, &value_2);
    error_flag = our_add(value_1, value_2, result);
  }
  return error_flag;
}

int our_mul(our_decimal value_1, our_decimal value_2, our_decimal *result) {
  int error_flag = 0;
  our_bigDecimal bigVal_1, bigVal_2, bigResult;
  ConvertToBig(value_1, value_2, result);
  set_sign(&bigResult, (get_sign(bigVal_1) ^ get_sign(bigVal_2)));
  set_exp(&bigResult, (get_exp(bigVal_1) + get_exp(bigVal_2)));
  for (int i = 0; i < 223; i++) {
    if (get_bit(bigVal_2, i)) {
      sum_mantissa(bigResult, left_shift(bigVal_1, i), &bigResult);
    }
  }
  error_flag = convert_fromBig(bigResult, result);
  return error_flag;
}

int our_div(our_decimal value_1, our_decimal value_2, our_decimal *result) {
  int error_flag = 0;
  our_bigDecimal bigVal_1, bigVal_2, bigResult;
  ConvertToBig(value_1, value_2, result);
  if (IsZero(bigVal_2)) {
    error_flag = DIV_ZERO;
  } else if (IsZero(bigVal_1)) {
    error_flag = (get_sign(bigVal_1) ^ get_sign(bigVal_2)) ? TO_SMALL : TO_LARGE;
  } else {
    while (is_less_bigDeci(bigVal_1, bigVal_2) && get_exp(bigVal_1) < 56) {
      mul_by_10(&bigVal_1);
      set_exp(&bigVal_1, get_exp(bigVal_1) + 1);
    }
    our_bigDecimal mod = div_mantissa(bigVal_1, bigVal_2, &bigResult);
    while (!IsZero(mod) && get_exp(bigVal_1) < 28) {
      int flag = mul_by_10(&bigVal_1);
      if (flag) { break; }
      mod = div_mantissa(bigVal_1, bigVal_2, &bigResult);
      set_exp(&bigVal_1, get_exp(bigVal_1) + 1);
    }
    set_sign(&bigResult, (get_sign(bigVal_1) ^ get_sign(bigVal_2)));
    int exp = get_exp(bigVal_1) - get_exp(bigVal_2);
    if (exp < 0) { while (exp++) { mul_by_10(&bigResult); }
    } else { set_exp(&bigResult, exp); }
    error_flag = convert_fromBig(bigResult, result);
  }
  return error_flag;
}

int our_mod(our_decimal value_1, our_decimal value_2, our_decimal *result) {
  int error_flag = 0;
  our_bigDecimal bigVal_1, bigVal_2, bigResult;
  ConvertToBig(value_1, value_2, result);
  if (IsZero(bigVal_2)) {
    error_flag = DIV_ZERO;
  } else if (IsZero(bigVal_1)) {
    error_flag = (get_sign(bigVal_1) ^ get_sign(bigVal_2)) ? TO_SMALL : TO_LARGE;
  } else {
    exp_alignment(&bigVal_1, &bigVal_2);
    if (is_less_bigDeci(bigVal_1, bigVal_2)) {
      bigResult = bigVal_1;
    } else {
      bigResult = div_mantissa(bigVal_1, bigVal_2, &bigVal_1);
    }
    set_sign(&bigResult, get_sign(bigVal_1));
    set_exp(&bigResult, get_exp(bigVal_1));
    convert_fromBig(bigResult, result);
  }
  return error_flag;
}

// /* Операторы сравнения */

int our_is_less(our_decimal value_1, our_decimal value_2) {
  bool rez = 0;
  our_bigDecimal bigVal_1, bigVal_2;
  convert_toBig(value_1, &bigVal_1);
  convert_toBig(value_2, &bigVal_2);
  bool sign_1 = get_sign(bigVal_1);
  bool sign_2 = get_sign(bigVal_2);
  if (IsZero(bigVal_1) && IsZero(bigVal_2)) { rez = false;
  } else if (sign_1 && !sign_2)             { rez = true; }
  if (sign_1 == sign_2) {
    if (sign_1) { SwapByTemp(bigVal_1, bigVal_2, our_bigDecimal); }
    exp_alignment(&bigVal_1, &bigVal_2);
    rez = is_less_bigDeci(bigVal_1, bigVal_2);
  }
  return rez;
}

int our_is_less_or_equal(our_decimal value_1, our_decimal value_2) {
  return (our_is_less(value_1, value_2) || our_is_equal(value_1, value_2));
}

int our_is_greater(our_decimal value_1, our_decimal value_2) {
  return !our_is_less_or_equal(value_1, value_2);
}

int our_is_greater_or_equal(our_decimal value_1, our_decimal value_2) {
  return !our_is_less(value_1, value_2);
}

int our_is_equal(our_decimal value_1, our_decimal value_2) {
  bool rez = 0;
  our_bigDecimal bigVal_1, bigVal_2;
  convert_toBig(value_1, &bigVal_1);
  convert_toBig(value_2, &bigVal_2);
  if (IsZero(bigVal_1) && IsZero(bigVal_2)) {
    rez = 1;
  } else if (get_sign(bigVal_1) == get_sign(bigVal_2)) {
    exp_alignment(&bigVal_1, &bigVal_2);
    for (int i = 0; i < 8; i++) {
      if (bigVal_1.bits[i] != bigVal_2.bits[i]) { rez = 0; break; }
      rez = 1;
    }
  }
  return rez;
}

int our_is_not_equal(our_decimal value_1, our_decimal value_2) {
  return !our_is_equal(value_1, value_2);
}

// /* Функции преобразования */

int our_from_int_to_decimal(int src, our_decimal *dst) {
  int result = 1;
  if (dst) {
    ClearDecimal(dst, our_decimal);
    dst->bits[0] = (src < 0) ? ~(src - 1) : src;
    dst->bits[3] = (src < 0) ? 0x80000000U : 0U;
    result = 0;
  }
  return result;
}

int our_from_float_to_decimal(float src, our_decimal *dst) {
  int error_flag = 0;
  if ((dst == NULL) || (src != src) || (src == INFINITY) || (src == -INFINITY)) {
    error_flag = 1;
  } else if (fabs(src) < 1e-28) {
    ClearDecimal(dst, our_decimal);
      if (src != 0) error_flag = 1;
  } else {
    ClearDecimal(dst, our_decimal);
    int sign = 0, expo2 = 0;
    unsigned int src_int, mant = 0;
    our_decimal twodec = {}, powdec = {}, mantdec = {};
    twodec.bits[0] = 2;
    powdec.bits[0] = 1;
    mantdec.bits[0] = 1;
    dst->bits[0] = 1;
    dst->bits[3] |= 0x80000000;
    if (src < 0) {
      sign = 1;
      src = -src;
    }
    src_int = *(unsigned int*)&src;
    expo2 = (src_int >> 23) - 127;
    mant = (src_int << 9) >> 9;
    for (int i = 0; i < 23; i++) {
      our_div(powdec, twodec, &powdec);
      if ((mant << (9 + i)) >> 31) { our_add(mantdec, powdec, &mantdec); }
    }
    if (sign == 1) { our_mul(mantdec, *dst, &mantdec); }
    if (expo2 > 0) {
      for (int i = 0; i < expo2; i++) {
        error_flag = our_mul(mantdec, twodec, &mantdec);
        if (error_flag) { break; }
      }
    } else if (expo2 < 0) {
      expo2 = -expo2;
      for (int i = 0; i < expo2; i++) {
        error_flag = our_div(mantdec, twodec, &mantdec);
      }
    }
    our_bigDecimal bigDec = {};
    convert_toBig(mantdec, &bigDec);
    for (int n = first_true_bit(bigDec); n > 24; n = first_true_bit(bigDec)) {
      round_by_10(&bigDec, 1, BANKING);
    }
    convert_fromBig(bigDec, dst);
  }
  if (error_flag != 0) error_flag = 1;
  return error_flag;
}

int our_from_decimal_to_int(our_decimal src, int *dst) {
  int res = 0;
  if (dst == NULL) { res = 1;
  } else {
    our_bigDecimal bigSrc = {};
    convert_toBig(src, &bigSrc);
    if (IsZero(bigSrc)) {
      *dst = 0;
    } else {
      round_by_10(&bigSrc, get_exp(bigSrc), CUT);
      if (bigSrc.bits[1] || bigSrc.bits[2] || get_bit(bigSrc, 31)) {
        res = (get_sign(bigSrc)) ? TO_SMALL : TO_LARGE;
      } else {
        if (get_sign(bigSrc)) {
          InvertBits(bigSrc);
          increment(&bigSrc);
          set_bit(&bigSrc, 31, true);
        }
        *dst = bigSrc.bits[0];
      }
    }
  }
  return res;
}

int our_from_decimal_to_float(our_decimal src, float *dst) {
  int ret = 0;
  if (dst == NULL) {
    ret = 1;
  } else {
    long double tmp = 0;
    *dst = 0;
    our_bigDecimal bigSrc;
    convert_toBig(src, &bigSrc);
    for (int i = 0; i < DECI_BITS; i++) {
      tmp = tmp + get_bit(bigSrc, i) * pow(2, i);
    }
    *dst = (float)tmp;
    for (int i = 0; i < get_exp(bigSrc); i++) {
      *dst = (*dst)/10;
    }
    if (src.bits[3] >> 31) *dst = -1 * (*dst);
  }
  return ret;
}

// /* Прочие функции */

int our_floor(our_decimal value, our_decimal *result) {
int ret = 1;
if (result) {
  our_bigDecimal bigVal = {};
  convert_toBig(value, &bigVal);
  round_by_10(&bigVal, get_exp(bigVal), TO_FLOOR);
  convert_fromBig(bigVal, result);
  ret = 0;
}
return ret;
}

int our_round(our_decimal value, our_decimal *result) {
int ret = 1;
if (result) {
  our_bigDecimal bigVal = {};
  convert_toBig(value, &bigVal);
  round_by_10(&bigVal, get_exp(bigVal), NORMAL);
  convert_fromBig(bigVal, result);
  ret = 0;
}
return ret;
}

int our_truncate(our_decimal value, our_decimal *result) {
int ret = 1;
if (result) {
  our_bigDecimal bigVal = {};
  convert_toBig(value, &bigVal);
  round_by_10(&bigVal, get_exp(bigVal), CUT);
  while (!first_digit(bigVal) && !IsZero(bigVal)) { div_by_10(&bigVal); }
  convert_fromBig(bigVal, result);
  ret = 0;
}
return ret;
}

int our_negate(our_decimal value, our_decimal *result) {
int ret = 1;
if (result) {
  our_bigDecimal bigVal = {};
  convert_toBig(value, &bigVal);
  if (!IsZero(bigVal)) { switch_sign(&bigVal); }
  convert_fromBig(bigVal, result);
  ret = 0;
}
return ret;
}

/*Служебные функции*/
/*Битовые операции*/

bool get_bit(our_bigDecimal bigDecimal, int position) {
  unsigned int mask = 1u << (position % 32);
  return !!(bigDecimal.bits[position / 32] & mask);
}

void set_bit(our_bigDecimal *bigDecimal, int position, bool value) {
  unsigned int mask = 1u << (position % 32);
  if (value) { (*bigDecimal).bits[position / 32] |= mask;
  } else     { (*bigDecimal).bits[position / 32] &= ~mask; }
  return;
}

bool get_sign(our_bigDecimal bigDecimal) {
  return get_bit(bigDecimal, SIGN_POS);
}

void set_sign(our_bigDecimal *bigDecimal, bool value) {
  set_bit(bigDecimal, SIGN_POS, value);
  return;
}

void switch_sign(our_bigDecimal *bigDecimal) {
  set_sign(bigDecimal, !(get_sign(*bigDecimal)));
  return;
}

int get_exp(our_bigDecimal bigDecimal) {
  int exp = bigDecimal.bits[7] << 1;
  return exp >>= 17;
}

void set_exp(our_bigDecimal *bigDecimal, int exp) {
  bool sign = get_sign(*bigDecimal);
  exp <<= 16;
  (*bigDecimal).bits[7] = exp;
  set_sign(bigDecimal, sign);
  return;
}

our_bigDecimal left_shift(our_bigDecimal bigDecimal, int shift) {
  while (shift--) {
    bigDecimal.bits[6] <<= 1;
    for (int i = 5; i >= 0; i--) {
      if ((1u << 31) & bigDecimal.bits[i]) { bigDecimal.bits[i + 1]++; }
      bigDecimal.bits[i] <<= 1;
    }
  }
  return bigDecimal;
}

our_bigDecimal right_shift(our_bigDecimal bigDecimal, int shift) {
  while (shift--) {
    bigDecimal.bits[0] >>= 1;
    for (int i = 1; i < 7; i++) {
      if (1u & bigDecimal.bits[i]) { bigDecimal.bits[i - 1] |= (1u << 31); }
      bigDecimal.bits[i] >>= 1;
    }
  }
  return bigDecimal;
}

int first_true_bit(our_bigDecimal bigDecimal) {
  int i = 223;
  for (; i >= 0 && !get_bit(bigDecimal, i); i--) {}
  return i;
}

/* Операции над мантиссой */

int sum_mantissa(our_bigDecimal value_1, our_bigDecimal value_2, our_bigDecimal *result) {
  bool carry = 0;
  our_bigDecimal temp = *result;
  for (int i = 0; i < BIG_DECI_BITS; i++) {
    bool bit_1 = get_bit(value_1, i);
    bool bit_2 = get_bit(value_2, i);
    set_bit(result, i, (bit_1 ^ bit_2 ^ carry));
    carry = (bit_1 & carry) | (bit_2 & carry) | (bit_1 & bit_2);
  }
  if (carry) { *result = temp; }
  return (carry) ? TO_LARGE : OK;
}

void sub_mantissa(our_bigDecimal value_1, our_bigDecimal value_2, our_bigDecimal* result) {
  InvertBits(value_2);
  bool carry = 1;
  for (int i = 0; i < BIG_DECI_BITS; i++) {
    bool bit_1 = get_bit(value_1, i);
    bool bit_2 = get_bit(value_2, i);
    set_bit(result, i, (bit_1 ^ bit_2 ^ carry));
    carry = (bit_1 & carry) | (bit_2 & carry) | (bit_1 & bit_2);
  }
}

our_bigDecimal div_mantissa(our_bigDecimal devidend, our_bigDecimal devider, our_bigDecimal *result) {
  StopIf((is_less_bigDeci(devidend, devider)), exit(EXIT_FAILURE), "div_mantissa error: A les then B!\n");
  ClearMantissa(*result);
  int n = normalize(devidend, &devider);
  for (int i = 0; i < n + 1; i++) {
    if (!is_less_bigDeci(devidend, devider)) {
      *result = left_shift(*result, 1);
      increment(result);
      sub_mantissa(devidend, devider, &devidend);
      devider = right_shift(devider, 1);
    } else {
      *result = left_shift(*result, 1);
      devider = right_shift(devider, 1);
    }
  }
  return devidend;
}

int increment(our_bigDecimal *bigDecimal) {
  bool carry = 1;
  for (int i = 0; carry && (i < BIG_DECI_BITS); i++) {
    bool bit = get_bit(*bigDecimal, i);
    set_bit(bigDecimal, i, bit ^ carry);
    carry &= bit;
  }
  return carry;
}

int normalize(our_bigDecimal devidend, our_bigDecimal *devider) {
  int n = first_true_bit(devidend) - first_true_bit(*devider);
  *devider = left_shift(*devider, n);
  return n;
}

bool is_less_bigDeci(our_bigDecimal bigVal_1, our_bigDecimal bigVal_2) {
  bool rez = false;
  for (int i = 223; i >= 0; i--) {
    if        (get_bit(bigVal_2, i) && !get_bit(bigVal_1, i)) { rez = true; break;
    } else if (get_bit(bigVal_1, i) && !get_bit(bigVal_2, i)) {             break; }
  }
  return rez;
}

/* Операции со степенью */

void exp_alignment(our_bigDecimal *value_1, our_bigDecimal *value_2) {
  if (get_exp(*value_2) < get_exp(*value_1)) {
    SwapByTemp(value_1, value_2, our_bigDecimal*);
  }
  int diff = get_exp(*value_2) - get_exp(*value_1);
  while (diff--) {
    mul_by_10(value_1);
    set_exp(value_1, get_exp(*value_1) + 1);
  }
}

int first_digit(our_bigDecimal bigDecimal) {
  return div_by_10(&bigDecimal);
}

int mul_by_10(our_bigDecimal *bigDecimal) {
  return sum_mantissa(left_shift(*bigDecimal, 3), left_shift(*bigDecimal, 1), bigDecimal);
}

int div_by_10(our_bigDecimal *decimal) {
  our_bigDecimal ten_devider = {}, temp = {};
  ten_devider.bits[0] = 10;
  if (is_less_bigDeci(*decimal, ten_devider)) {
      temp.bits[0] = decimal->bits[0];
      ClearMantissa(*decimal);
  } else {
      temp = div_mantissa(*decimal, ten_devider, decimal);
  }
  return temp.bits[0];  // перевод децимал в инт!!!!
}

void round_by_10(our_bigDecimal *bigDecimal, int n, int round_type) {
  int mod = 0, flag = false;
  for (int i = 0; i < n; i++) {
    mod = div_by_10(bigDecimal);
    if (i < n - 1 && mod) { flag = true; }
  }
  set_exp(bigDecimal, get_exp(*bigDecimal) - n);
  switch (round_type) {
    case NORMAL:
      if (mod >= 5) { increment(bigDecimal); }
      break;
    case BANKING:
      if (first_digit(*bigDecimal) % 2 != 0 || flag) { mod++; }
      if (mod > 5) { increment(bigDecimal); }
      break;
    case TO_FLOOR:
      if (get_sign(*bigDecimal) && (mod || flag)) { increment(bigDecimal); }
      break;
    case CUT:
      break;
  }
}

/* Конвертеры */

void convert_toBig(our_decimal decimal, our_bigDecimal *bigDecimal) {
  ClearDecimal(bigDecimal, our_bigDecimal);
  for (int i = 0; i < 3; i++) {
    bigDecimal->bits[i] = decimal.bits[i];
  }
  bigDecimal->bits[7] = decimal.bits[3];
  ZeroToZero(bigDecimal);
}

int convert_fromBig(our_bigDecimal bigDecimal, our_decimal *decimal) {
  int error_code = 0;
  ClearDecimal(decimal, our_decimal);
  ZeroToZero(&bigDecimal);
  while (first_true_bit(bigDecimal) > 95 && get_exp(bigDecimal) > 0) {
    round_by_10(&bigDecimal, 1, BANKING);
  }
  if (get_exp(bigDecimal) > 28) {
    round_by_10(&bigDecimal, (get_exp(bigDecimal) - 28), BANKING);
  }
  while (first_digit(bigDecimal) == 0 && get_exp(bigDecimal) > 0) {  // Проверь тест кейсы
    div_by_10(&bigDecimal);
    set_exp(&bigDecimal, get_exp(bigDecimal) - 1);
  }
  if (first_true_bit(bigDecimal) < DECI_BITS) {
    for (int i = 0; i < 3; i++) { decimal->bits[i] = bigDecimal.bits[i]; }
    decimal->bits[3] = bigDecimal.bits[7];
  } else { error_code = get_sign(bigDecimal) ? TO_SMALL : TO_LARGE; }
  return error_code;
}
