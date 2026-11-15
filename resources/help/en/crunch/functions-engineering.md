# Engineering functions
<!-- keywords: functions, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX -->

These are the engineering functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## BESSELI

`BESSELI(x, n)`

Gives the modified Bessel function of the first kind.

- `x`: The x value to work with.
- `n`: The first power of x, or the order of the function, a whole number of 0 or more.

## BESSELJ

`BESSELJ(x, n)`

Gives the Bessel function of the first kind.

- `x`: The x value to work with.
- `n`: The first power of x, or the order of the function, a whole number of 0 or more.

## BESSELK

`BESSELK(x, n)`

Gives the modified Bessel function of the second kind.

- `x`: The x value to work with.
- `n`: The first power of x, or the order of the function, a whole number of 0 or more.

## BESSELY

`BESSELY(x, n)`

Gives the Bessel function of the second kind.

- `x`: The x value to work with.
- `n`: The first power of x, or the order of the function, a whole number of 0 or more.

## BIN2DEC

`BIN2DEC(number)`

Turns a binary number into a decimal number.

- `number`: The number to work on.

## BIN2HEX

`BIN2HEX(number, [places])`

Turns a binary number into a hexadecimal number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## BIN2OCT

`BIN2OCT(number, [places])`

Turns a binary number into a octal number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## BITAND

`BITAND(number1, number2)`

Gives the bits that are set in both of two whole numbers.

- `number1`: The first number, or block of cells.
- `number2`: The second number.

## BITLSHIFT

`BITLSHIFT(number, shift_amount)`

Moves the bits of a whole number to the left.

- `number`: The number to work on.
- `shift_amount`: How many bits to move: positive and negative numbers go in opposite ways.

## BITOR

`BITOR(number1, number2)`

Gives the bits that are set in either of two whole numbers.

- `number1`: The first number, or block of cells.
- `number2`: The second number.

## BITRSHIFT

`BITRSHIFT(number, shift_amount)`

Moves the bits of a whole number to the right.

- `number`: The number to work on.
- `shift_amount`: How many bits to move: positive and negative numbers go in opposite ways.

## BITXOR

`BITXOR(number1, number2)`

Gives the bits that are set in one of two whole numbers but not both.

- `number1`: The first number, or block of cells.
- `number2`: The second number.

## COMPLEX

`COMPLEX(real_num, i_num, [suffix])`

Makes a complex number from its real and imaginary parts.

- `real_num`: The real part of the complex number.
- `i_num`: The imaginary part of the complex number.
- `suffix` (optional): i or j, to mark the imaginary part (i if left out).

## CONVERT

`CONVERT(number, from_unit, to_unit)`

Changes a number from one unit of measure to another.

- `number`: The number to work on.
- `from_unit`: The unit the number is in, as text, such as "mi" or "lbm".
- `to_unit`: The unit to change it to, as text, such as "km" or "kg".

## DEC2BIN

`DEC2BIN(number, [places])`

Turns a decimal number into a binary number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## DEC2HEX

`DEC2HEX(number, [places])`

Turns a decimal number into a hexadecimal number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## DEC2OCT

`DEC2OCT(number, [places])`

Turns a decimal number into a octal number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## DELTA

`DELTA(number1, [number2])`

Tells whether two numbers are equal, with 1 for yes and 0 for no.

- `number1`: The first number, or block of cells.
- `number2` (optional): The second number.

## ERF

`ERF(lower_limit, [upper_limit])`

Gives the error function, between two limits or from 0.

- `lower_limit`: The lower limit of the range.
- `upper_limit` (optional): The upper limit of the range (the lower limit alone if left out).

## ERF.PRECISE

`ERF.PRECISE(x)`

Gives the error function from 0 to a limit.

- `x`: The x value to work with.

## ERFC

`ERFC(x)`

Gives the complementary error function from a limit to infinity.

- `x`: The x value to work with.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Gives the complementary error function from a limit to infinity.

- `x`: The x value to work with.

## GESTEP

`GESTEP(number, [step])`

Tells whether a number is at least a step, with 1 for yes and 0 for no.

- `number`: The number to work on.
- `step` (optional): How much to add for each number (1 if left out; for GESTEP, the number to compare with, 0 if left out).

## HEX2BIN

`HEX2BIN(number, [places])`

Turns a hexadecimal number into a binary number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## HEX2DEC

`HEX2DEC(number)`

Turns a hexadecimal number into a decimal number.

- `number`: The number to work on.

## HEX2OCT

`HEX2OCT(number, [places])`

Turns a hexadecimal number into a octal number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## IMABS

`IMABS(inumber)`

Gives the size of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMAGINARY

`IMAGINARY(inumber)`

Gives the imaginary part of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMARGUMENT

`IMARGUMENT(inumber)`

Gives the angle of a complex number, in radians.

- `inumber`: A complex number written as text, such as "3+4i".

## IMCONJUGATE

`IMCONJUGATE(inumber)`

Gives the conjugate of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMCOS

`IMCOS(inumber)`

Gives the cosine of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMCOSH

`IMCOSH(inumber)`

Gives the hyperbolic cosine of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMCOT

`IMCOT(inumber)`

Gives the cotangent of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMCSC

`IMCSC(inumber)`

Gives the cosecant of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMCSCH

`IMCSCH(inumber)`

Gives the hyperbolic cosecant of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMDIV

`IMDIV(inumber1, inumber2)`

Gives the quotient of two complex numbers.

- `inumber1`: The first complex number, written as text, such as "3+4i".
- `inumber2`: The second complex number, written as text.

## IMEXP

`IMEXP(inumber)`

Gives e raised to the power of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMLN

`IMLN(inumber)`

Gives the natural logarithm of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMLOG10

`IMLOG10(inumber)`

Gives the base-10 logarithm of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMLOG2

`IMLOG2(inumber)`

Gives the base-2 logarithm of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMPOWER

`IMPOWER(inumber, number)`

Gives a complex number raised to a power.

- `inumber`: A complex number written as text, such as "3+4i".
- `number`: The number to work on.

## IMPRODUCT

`IMPRODUCT(inumber1, ...)`

Gives the product of complex numbers.

- `inumber1`: The first complex number, written as text, such as "3+4i".
- `...`: As many more as you need, of the same kind as the one before it.

## IMREAL

`IMREAL(inumber)`

Gives the real part of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMSEC

`IMSEC(inumber)`

Gives the secant of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMSECH

`IMSECH(inumber)`

Gives the hyperbolic secant of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMSIN

`IMSIN(inumber)`

Gives the sine of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMSINH

`IMSINH(inumber)`

Gives the hyperbolic sine of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMSQRT

`IMSQRT(inumber)`

Gives the square root of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## IMSUB

`IMSUB(inumber1, inumber2)`

Gives the difference of two complex numbers.

- `inumber1`: The first complex number, written as text, such as "3+4i".
- `inumber2`: The second complex number, written as text.

## IMSUM

`IMSUM(inumber1, ...)`

Gives the sum of complex numbers.

- `inumber1`: The first complex number, written as text, such as "3+4i".
- `...`: As many more as you need, of the same kind as the one before it.

## IMTAN

`IMTAN(inumber)`

Gives the tangent of a complex number.

- `inumber`: A complex number written as text, such as "3+4i".

## OCT2BIN

`OCT2BIN(number, [places])`

Turns a octal number into a binary number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.

## OCT2DEC

`OCT2DEC(number)`

Turns a octal number into a decimal number.

- `number`: The number to work on.

## OCT2HEX

`OCT2HEX(number, [places])`

Turns a octal number into a hexadecimal number.

- `number`: The number to work on.
- `places` (optional): How many characters to fill the result out to with zeros on the left.
