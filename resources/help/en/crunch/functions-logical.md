# Logical functions
<!-- keywords: functions, AND, CHOOSE, FALSE, IF, IFERROR, IFNA, IFS, LET, NOT, OR, SWITCH, TRUE, XOR -->

These are the logical functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## AND

`AND(logical1, ...)`

True when every one of the conditions is true.

- `logical1`: The first value or test, that is true or false.
- `...`: As many more as you need, of the same kind as the one before it.

## CHOOSE

`CHOOSE(index, value1, ...)`

Picks the value in a list that sits at the place the index names.

- `index`: Which value to give, counting the first as 1.
- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## FALSE

`FALSE()`

The logical value false.

## IF

`IF(condition, value_if_true, [value_if_false])`

Gives one value when a condition holds and another when it does not.

- `condition`: A value or test that is true or false.
- `value_if_true`: What to give if the condition is true.
- `value_if_false` (optional): What to give if the condition is false (FALSE if left out).

## IFERROR

`IFERROR(value, value_if_error)`

Gives a value, or a replacement when that value is an error.

- `value`: The value to work on, or a cell that holds it.
- `value_if_error`: What to give if the value is an error.

## IFNA

`IFNA(value, value_if_na)`

Gives a value, or a replacement when that value is the not-available error.

- `value`: The value to work on, or a cell that holds it.
- `value_if_na`: What to give if the value is the not-available error.

## IFS

`IFS(condition1, value1, ...)`

Tests conditions in turn and gives the value that goes with the first one that holds.

- `condition1`: The first test, that is true or false.
- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## LET

`LET(name1, value1, ...)`

Gives names to values inside one formula, then works out a calculation that uses them.

- `name1`: A name to give to the value that follows. It can be used in the values after it and in the calculation.
- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## NOT

`NOT(logical)`

Turns true into false and false into true.

- `logical`: A value or test that is true or false.

## OR

`OR(logical1, ...)`

True when at least one of the conditions is true.

- `logical1`: The first value or test, that is true or false.
- `...`: As many more as you need, of the same kind as the one before it.

## SWITCH

`SWITCH(expression, value1, result1, ...)`

Compares a value with a list of values and gives the result that goes with the first match.

- `expression`: The value to compare with the values that follow.
- `value1`: The first value, or block of cells.
- `result1`: The result to give when the value before it matches.
- `...`: As many more as you need, of the same kind as the one before it.

## TRUE

`TRUE()`

The logical value true.

## XOR

`XOR(logical1, ...)`

True when an odd number of the conditions are true.

- `logical1`: The first value or test, that is true or false.
- `...`: As many more as you need, of the same kind as the one before it.
