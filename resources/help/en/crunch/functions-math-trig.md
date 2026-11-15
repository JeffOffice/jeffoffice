# Math and trig functions
<!-- keywords: functions, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC -->

These are the math and trig functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## ABS

`ABS(number)`

Gives a number without its sign.

- `number`: The number to work on.

## ACOS

`ACOS(number)`

Gives the angle, in radians, that has a given cosine.

- `number`: The number to work on.

## ACOSH

`ACOSH(number)`

Gives the inverse hyperbolic cosine of a number.

- `number`: The number to work on.

## ACOT

`ACOT(number)`

Gives the inverse cotangent of a number, as an angle in radians.

- `number`: The number to work on.

## ACOTH

`ACOTH(number)`

Gives the inverse hyperbolic cotangent of a number.

- `number`: The number to work on.

## AGGREGATE

`AGGREGATE(function_num, options, ref1, ...)`

Does a calculation on a list, such as a sum or a median, and can leave out hidden rows and errors.

- `function_num`: Which calculation to do: 1 average, 2 count, 3 count nonblanks, 4 maximum, 5 minimum, 6 product, 7 and 8 standard deviation, 9 sum, 10 and 11 variance.
- `options`: What to leave out: 0 nested totals, 1 also hidden rows, 2 also errors, 3 both, 4 to 7 to keep nested totals.
- `ref1`: The first cell or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## ARABIC

`ARABIC(text)`

Reads Roman numerals in text as a number.

- `text`: The text to work on, or a cell that holds it.

## ASIN

`ASIN(number)`

Gives the angle, in radians, that has a given sine.

- `number`: The number to work on.

## ASINH

`ASINH(number)`

Gives the inverse hyperbolic sine of a number.

- `number`: The number to work on.

## ATAN

`ATAN(number)`

Gives the angle, in radians, that has a given tangent.

- `number`: The number to work on.

## ATAN2

`ATAN2(x_num, y_num)`

Gives the angle, in radians, of the point at x and y.

- `x_num`: The x value.
- `y_num`: The y value.

## ATANH

`ATANH(number)`

Gives the inverse hyperbolic tangent of a number.

- `number`: The number to work on.

## BASE

`BASE(number, radix, [min_length])`

Writes a number in another base, from 2 to 36, as text.

- `number`: The number to work on.
- `radix`: The base of the number: from 2 to 36.
- `min_length` (optional): The fewest characters the result has; shorter results are filled with zeros on the left.

## CEILING

`CEILING(number, [significance])`

Rounds a number up to the nearest multiple.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.

## CEILING.MATH

`CEILING.MATH(number, [significance], [mode])`

Rounds a number up to the nearest multiple, with a choice for negative numbers.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.
- `mode` (optional): How negative numbers are rounded: 0 or left out is the usual way, and any other number reverses it.

## CEILING.PRECISE

`CEILING.PRECISE(number, [significance])`

Rounds a number up to the nearest multiple, whatever the sign of the multiple.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.

## COMBIN

`COMBIN(number, number_chosen)`

Counts the ways to choose items from a group when the order does not matter.

- `number`: The number to work on.
- `number_chosen`: How many items to pick in each group.

## COMBINA

`COMBINA(number, number_chosen)`

Gives the number of ways to choose items when an item can be chosen more than once.

- `number`: The number to work on.
- `number_chosen`: How many items to pick in each group.

## COS

`COS(number)`

Gives the cosine of an angle in radians.

- `number`: The number to work on.

## COSH

`COSH(number)`

Gives the hyperbolic cosine of a number.

- `number`: The number to work on.

## COT

`COT(angle)`

Gives the cotangent of an angle in radians.

- `angle`: The angle, in radians.

## COTH

`COTH(number)`

Gives the hyperbolic cotangent of a number.

- `number`: The number to work on.

## CSC

`CSC(angle)`

Gives the cosecant of an angle in radians.

- `angle`: The angle, in radians.

## CSCH

`CSCH(number)`

Gives the hyperbolic cosecant of a number.

- `number`: The number to work on.

## DECIMAL

`DECIMAL(text, radix)`

Reads text written in a base from 2 to 36 as a number.

- `text`: The text to work on, or a cell that holds it.
- `radix`: The base of the number: from 2 to 36.

## DEGREES

`DEGREES(angle)`

Turns radians into degrees.

- `angle`: The angle, in radians.

## EVEN

`EVEN(number)`

Rounds a number up to the nearest even whole number.

- `number`: The number to work on.

## EXP

`EXP(number)`

Gives e raised to a power.

- `number`: The number to work on.

## FACT

`FACT(number)`

Gives the factorial of a number.

- `number`: The number to work on.

## FACTDOUBLE

`FACTDOUBLE(number)`

Gives the double factorial of a number: the product of every second number down to 1 or 2.

- `number`: The number to work on.

## FLOOR

`FLOOR(number, [significance])`

Rounds a number down to the nearest multiple.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.

## FLOOR.MATH

`FLOOR.MATH(number, [significance], [mode])`

Rounds a number down to the nearest multiple, with a choice for negative numbers.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.
- `mode` (optional): How negative numbers are rounded: 0 or left out is the usual way, and any other number reverses it.

## FLOOR.PRECISE

`FLOOR.PRECISE(number, [significance])`

Rounds a number down to the nearest multiple, whatever the sign of the multiple.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.

## GCD

`GCD(number1, ...)`

Gives the largest whole number that divides all the numbers.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## INT

`INT(number)`

Rounds a number down to the whole number below it.

- `number`: The number to work on.

## ISO.CEILING

`ISO.CEILING(number, [significance])`

Rounds a number up to the nearest multiple, whatever the sign of the multiple.

- `number`: The number to work on.
- `significance` (optional): The number whose multiples to round to.

## LCM

`LCM(number1, ...)`

Gives the smallest whole number that all the numbers divide.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## LN

`LN(number)`

Gives the natural logarithm of a number.

- `number`: The number to work on.

## LOG

`LOG(number, [base])`

Gives the logarithm of a number to a base (10 if none is given).

- `number`: The number to work on.
- `base` (optional): The base of the logarithm (10 if left out).

## LOG10

`LOG10(number)`

Gives the logarithm of a number to base 10.

- `number`: The number to work on.

## MAX

`MAX(number1, ...)`

Gives the largest number.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MDETERM

`MDETERM(array)`

Gives the determinant of a square block of numbers.

- `array`: A block of cells, or a list of values written in braces.

## MIN

`MIN(number1, ...)`

Gives the smallest number.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MINVERSE

`MINVERSE(array)`

Gives the inverse of a square block of numbers.

- `array`: A block of cells, or a list of values written in braces.

## MMULT

`MMULT(array1, array2)`

Gives the product of two blocks of numbers, as matrices.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## MOD

`MOD(number, divisor)`

Gives what is left after dividing one number by another.

- `number`: The number to work on.
- `divisor`: The number to divide by.

## MROUND

`MROUND(number, multiple)`

Rounds a number to the nearest multiple.

- `number`: The number to work on.
- `multiple`: The number whose multiples to round to.

## MULTINOMIAL

`MULTINOMIAL(number1, ...)`

Gives the factorial of the sum of the numbers divided by the product of their factorials.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MUNIT

`MUNIT(dimension)`

Gives the identity matrix of a size.

- `dimension`: The size of the square block: how many rows and columns.

## ODD

`ODD(number)`

Rounds a number up to the nearest odd whole number.

- `number`: The number to work on.

## PERMUT

`PERMUT(number, number_chosen)`

Counts the ways to choose items from a group when the order matters.

- `number`: The number to work on.
- `number_chosen`: How many items to pick in each group.

## PI

`PI()`

Gives the number pi.

## POWER

`POWER(number, power)`

Raises a number to a power.

- `number`: The number to work on.
- `power`: The power to raise the number to.

## PRODUCT

`PRODUCT(number1, ...)`

Multiplies numbers together.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## QUOTIENT

`QUOTIENT(numerator, denominator)`

Gives the whole part of a division.

- `numerator`: The number to be divided.
- `denominator`: The number to divide by.

## RADIANS

`RADIANS(angle)`

Turns degrees into radians.

- `angle`: The angle, in radians.

## RAND

`RAND()`

Gives a random number from 0 up to, but not including, 1.

## RANDARRAY

`RANDARRAY([rows], [columns], [min], [max], [whole_number])`

Gives a block of random numbers.

- `rows` (optional): How many rows to move: positive down, negative up.
- `columns` (optional): How many columns wide the result is (1 if left out).
- `min` (optional): The smallest number the result may have (0 if left out).
- `max` (optional): The largest number the result may have (1 if left out).
- `whole_number` (optional): True for whole numbers; false or left out for numbers with decimals.

## RANDBETWEEN

`RANDBETWEEN(bottom, top)`

Gives a random whole number between two numbers.

- `bottom`: The smallest value the result may have.
- `top`: The largest value the result may have.

## ROMAN

`ROMAN(number, [form])`

Writes a whole number from 0 to 3999 as Roman numerals in text.

- `number`: The number to work on.
- `form` (optional): How short the Roman numerals are: 0 or left out for the classic form, up to 4 for the shortest.

## ROUND

`ROUND(number, num_digits)`

Rounds a number to a number of digits.

- `number`: The number to work on.
- `num_digits`: How many digits to round to: positive for the right of the decimal point, 0 for a whole number, negative for the left.

## ROUNDDOWN

`ROUNDDOWN(number, num_digits)`

Rounds a number down, toward zero, to a number of digits.

- `number`: The number to work on.
- `num_digits`: How many digits to round to: positive for the right of the decimal point, 0 for a whole number, negative for the left.

## ROUNDUP

`ROUNDUP(number, num_digits)`

Rounds a number up, away from zero, to a number of digits.

- `number`: The number to work on.
- `num_digits`: How many digits to round to: positive for the right of the decimal point, 0 for a whole number, negative for the left.

## SEC

`SEC(angle)`

Gives the secant of an angle in radians.

- `angle`: The angle, in radians.

## SECH

`SECH(number)`

Gives the hyperbolic secant of a number.

- `number`: The number to work on.

## SEQUENCE

`SEQUENCE(rows, [columns], [start], [step])`

Gives a block of numbers that count up from a start by a step.

- `rows`: How many rows to move: positive down, negative up.
- `columns` (optional): How many columns wide the result is (1 if left out).
- `start` (optional): The first number (1 if left out).
- `step` (optional): How much to add for each number (1 if left out; for GESTEP, the number to compare with, 0 if left out).

## SERIESSUM

`SERIESSUM(x, n, m, coefficients)`

Gives the sum of a power series.

- `x`: The x value to work with.
- `n`: The first power of x, or the order of the function, a whole number of 0 or more.
- `m`: The step to add to the power for each coefficient.
- `coefficients`: The numbers to multiply each power of x by.

## SIGN

`SIGN(number)`

Gives 1 for a positive number, 0 for zero, and -1 for a negative one.

- `number`: The number to work on.

## SIN

`SIN(number)`

Gives the sine of an angle in radians.

- `number`: The number to work on.

## SINH

`SINH(number)`

Gives the hyperbolic sine of a number.

- `number`: The number to work on.

## SQRT

`SQRT(number)`

Gives the square root of a number.

- `number`: The number to work on.

## SQRTPI

`SQRTPI(number)`

Gives the square root of a number times pi.

- `number`: The number to work on.

## SUBTOTAL

`SUBTOTAL(function_num, ref1, ...)`

Gives a total, average, count, or other figure for a block of cells.

- `function_num`: Which calculation to do: 1 average, 2 count, 3 count nonblanks, 4 maximum, 5 minimum, 6 product, 7 and 8 standard deviation, 9 sum, 10 and 11 variance.
- `ref1`: The first cell or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## SUM

`SUM(number1, ...)`

Adds numbers and the numbers in blocks of cells.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## SUMPRODUCT

`SUMPRODUCT(array1, ...)`

Multiplies the numbers in blocks of the same size, place by place, and adds the products.

- `array1`: The first block of cells or list of values.
- `...`: As many more as you need, of the same kind as the one before it.

## SUMSQ

`SUMSQ(number1, ...)`

Adds the squares of numbers.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## SUMX2MY2

`SUMX2MY2(array_x, array_y)`

Adds the differences of the squares of two sets of numbers.

- `array_x`: The first block of cells or list of values.
- `array_y`: The second block of cells or list of values, the same size as the first.

## SUMX2PY2

`SUMX2PY2(array_x, array_y)`

Adds the sums of the squares of two sets of numbers.

- `array_x`: The first block of cells or list of values.
- `array_y`: The second block of cells or list of values, the same size as the first.

## SUMXMY2

`SUMXMY2(array_x, array_y)`

Adds the squares of the differences of two sets of numbers.

- `array_x`: The first block of cells or list of values.
- `array_y`: The second block of cells or list of values, the same size as the first.

## TAN

`TAN(number)`

Gives the tangent of an angle in radians.

- `number`: The number to work on.

## TANH

`TANH(number)`

Gives the hyperbolic tangent of a number.

- `number`: The number to work on.

## TRUNC

`TRUNC(number, [num_digits])`

Cuts a number off after a number of digits, without rounding.

- `number`: The number to work on.
- `num_digits` (optional): How many digits to round to: positive for the right of the decimal point, 0 for a whole number, negative for the left.
