#include "core/functioninfo.h"

#include "calc/functions.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::crunch {

namespace {

using Cat = FunctionCategory;

// Every function the engine has (a test checks the list against the engine's): its name, kind, arguments (an optional one in brackets, "..." for as many more as there are room for), and what it does.
const FunctionDoc kDocs[] = {
    {"IF", Cat::Logical, "condition, value_if_true, [value_if_false]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives one value when a condition holds and another when it does not.")},
    {"IFERROR", Cat::Logical, "value, value_if_error", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a value, or a replacement when that value is an error.")},
    {"IFNA", Cat::Logical, "value, value_if_na", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a value, or a replacement when that value is the not-available error.")},
    {"CHOOSE", Cat::Logical, "index, value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Picks the value in a list that sits at the place the index names.")},
    {"IFS", Cat::Logical, "condition1, value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Tests conditions in turn and gives the value that goes with the first one that holds.")},
    {"SWITCH", Cat::Logical, "expression, value1, result1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Compares a value with a list of values and gives the result that goes with the first match.")},
    {"AND", Cat::Logical, "logical1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when every one of the conditions is true.")},
    {"OR", Cat::Logical, "logical1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when at least one of the conditions is true.")},
    {"XOR", Cat::Logical, "logical1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when an odd number of the conditions are true.")},
    {"NOT", Cat::Logical, "logical", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns true into false and false into true.")},
    {"TRUE", Cat::Logical, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "The logical value true.")},
    {"FALSE", Cat::Logical, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "The logical value false.")},
    {"ISNUMBER", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is a number.")},
    {"ISTEXT", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is text.")},
    {"ISNONTEXT", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is anything but text.")},
    {"ISLOGICAL", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is true or false.")},
    {"ISBLANK", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the cell is empty.")},
    {"ISERROR", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is any error.")},
    {"ISERR", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is an error other than not-available.")},
    {"ISNA", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is the not-available error.")},
    {"ISREF", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when the value is a reference to cells.")},
    {"ISEVEN", Cat::Information, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when a whole number is even.")},
    {"ISODD", Cat::Information, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when a whole number is odd.")},
    {"NA", Cat::Information, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the not-available error, to mark a value that is missing.")},
    {"N", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a value into a number: dates and true values become numbers, text becomes zero.")},
    {"TYPE", Cat::Information, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a number that says what kind of value it is: number, text, logical, error, or array.")},
    {"ERROR.TYPE", Cat::Information, "error_value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the number that stands for a kind of error.")},
    {"CELL", Cat::Information, "info_type, [reference]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Tells something about a cell, such as its address or its format.")},
    {"VLOOKUP", Cat::LookupReference, "lookup_value, table_array, col_index_num, [range_lookup]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Looks for a value in the first column of a table and gives a value from the same row.")},
    {"HLOOKUP", Cat::LookupReference, "lookup_value, table_array, row_index_num, [range_lookup]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Looks for a value in the first row of a table and gives a value from the same column.")},
    {"MATCH", Cat::LookupReference, "lookup_value, lookup_array, [match_type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the place of a value in a list.")},
    {"LOOKUP", Cat::LookupReference, "lookup_value, lookup_vector, [result_vector]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Looks for a value in one row or column and gives the value at the same place in another.")},
    {"INDEX", Cat::LookupReference, "array, row_num, [column_num], [area_num]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the value at a row and column of a block of cells.")},
    {"OFFSET", Cat::LookupReference, "reference, rows, cols, [height], [width]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the cells a given number of rows and columns away from a starting cell.")},
    {"INDIRECT", Cat::LookupReference, "ref_text, [a1]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the cell that an address written as text points to.")},
    {"ROW", Cat::LookupReference, "[reference]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the row number of a cell.")},
    {"COLUMN", Cat::LookupReference, "[reference]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the column number of a cell.")},
    {"ROWS", Cat::LookupReference, "array", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the rows in a block of cells.")},
    {"COLUMNS", Cat::LookupReference, "array", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the columns in a block of cells.")},
    {"ADDRESS", Cat::LookupReference, "row_num, column_num, [abs_num], [a1], [sheet_text]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Writes the address of a cell as text, from its row and column numbers.")},
    {"TRANSPOSE", Cat::LookupReference, "array", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns rows into columns and columns into rows.")},
    {"HYPERLINK", Cat::LookupReference, "link_location, [friendly_name]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Makes a cell that opens a link when it is clicked.")},
    {"SUM", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Adds numbers and the numbers in blocks of cells.")},
    {"PRODUCT", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Multiplies numbers together.")},
    {"SUMSQ", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Adds the squares of numbers.")},
    {"MAX", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the largest number.")},
    {"MIN", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the smallest number.")},
    {"ABS", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a number without its sign.")},
    {"SQRT", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the square root of a number.")},
    {"EXP", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives e raised to a power.")},
    {"LN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the natural logarithm of a number.")},
    {"LOG", Cat::MathTrig, "number, [base]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the logarithm of a number to a base (10 if none is given).")},
    {"LOG10", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the logarithm of a number to base 10.")},
    {"SIGN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives 1 for a positive number, 0 for zero, and -1 for a negative one.")},
    {"INT", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number down to the whole number below it.")},
    {"PI", Cat::MathTrig, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the number pi.")},
    {"POWER", Cat::MathTrig, "number, power", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Raises a number to a power.")},
    {"MOD", Cat::MathTrig, "number, divisor", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives what is left after dividing one number by another.")},
    {"QUOTIENT", Cat::MathTrig, "numerator, denominator", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the whole part of a division.")},
    {"ROUND", Cat::MathTrig, "number, num_digits", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number to a number of digits.")},
    {"ROUNDUP", Cat::MathTrig, "number, num_digits", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number up, away from zero, to a number of digits.")},
    {"ROUNDDOWN", Cat::MathTrig, "number, num_digits", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number down, toward zero, to a number of digits.")},
    {"TRUNC", Cat::MathTrig, "number, [num_digits]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Cuts a number off after a number of digits, without rounding.")},
    {"CEILING", Cat::MathTrig, "number, [significance]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number up to the nearest multiple.")},
    {"FLOOR", Cat::MathTrig, "number, [significance]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number down to the nearest multiple.")},
    {"CEILING.MATH", Cat::MathTrig, "number, [significance], [mode]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number up to the nearest multiple, with a choice for negative numbers.")},
    {"FLOOR.MATH", Cat::MathTrig, "number, [significance], [mode]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number down to the nearest multiple, with a choice for negative numbers.")},
    {"MROUND", Cat::MathTrig, "number, multiple", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number to the nearest multiple.")},
    {"EVEN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number up to the nearest even whole number.")},
    {"ODD", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Rounds a number up to the nearest odd whole number.")},
    {"FACT", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the factorial of a number.")},
    {"RAND", Cat::MathTrig, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a random number from 0 up to, but not including, 1.")},
    {"RANDBETWEEN", Cat::MathTrig, "bottom, top", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a random whole number between two numbers.")},
    {"GCD", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the largest whole number that divides all the numbers.")},
    {"LCM", Cat::MathTrig, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the smallest whole number that all the numbers divide.")},
    {"COMBIN", Cat::MathTrig, "number, number_chosen", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the ways to choose items from a group when the order does not matter.")},
    {"PERMUT", Cat::MathTrig, "number, number_chosen", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the ways to choose items from a group when the order matters.")},
    {"SIN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the sine of an angle in radians.")},
    {"COS", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the cosine of an angle in radians.")},
    {"TAN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the tangent of an angle in radians.")},
    {"ASIN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the angle, in radians, that has a given sine.")},
    {"ACOS", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the angle, in radians, that has a given cosine.")},
    {"ATAN", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the angle, in radians, that has a given tangent.")},
    {"ATAN2", Cat::MathTrig, "x_num, y_num", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the angle, in radians, of the point at x and y.")},
    {"SINH", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the hyperbolic sine of a number.")},
    {"COSH", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the hyperbolic cosine of a number.")},
    {"TANH", Cat::MathTrig, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the hyperbolic tangent of a number.")},
    {"RADIANS", Cat::MathTrig, "angle", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns degrees into radians.")},
    {"DEGREES", Cat::MathTrig, "angle", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns radians into degrees.")},
    {"SUMPRODUCT", Cat::MathTrig, "array1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Multiplies the numbers in blocks of the same size, place by place, and adds the products.")},
    {"SUBTOTAL", Cat::MathTrig, "function_num, ref1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives a total, average, count, or other figure for a block of cells.")},
    {"AVERAGE", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the average of numbers.")},
    {"AVERAGEA", Cat::Statistical, "value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the average of values, counting text as zero and true as one.")},
    {"COUNT", Cat::Statistical, "value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the numbers in a list or a block of cells.")},
    {"COUNTA", Cat::Statistical, "value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the cells that are not empty.")},
    {"COUNTBLANK", Cat::Statistical, "range", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the empty cells in a block.")},
    {"MAXA", Cat::Statistical, "value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the largest value, counting text as zero and true as one.")},
    {"MINA", Cat::Statistical, "value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the smallest value, counting text as zero and true as one.")},
    {"MEDIAN", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the middle number of a set.")},
    {"MODE", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the number that occurs most often.")},
    {"STDEV", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Estimates the spread of a whole group from a sample of it.")},
    {"STDEV.S", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Estimates the spread of a whole group from a sample of it.")},
    {"STDEVP", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the spread of a whole group.")},
    {"STDEV.P", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the spread of a whole group.")},
    {"VAR", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Estimates the variance of a whole group from a sample of it.")},
    {"VAR.S", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Estimates the variance of a whole group from a sample of it.")},
    {"VARP", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the variance of a whole group.")},
    {"VAR.P", Cat::Statistical, "number1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the variance of a whole group.")},
    {"LARGE", Cat::Statistical, "array, k", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the k-th largest number of a set.")},
    {"SMALL", Cat::Statistical, "array, k", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the k-th smallest number of a set.")},
    {"RANK", Cat::Statistical, "number, ref, [order]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the place of a number in a list when the list is put in order.")},
    {"SUMIF", Cat::Statistical, "range, criteria, [sum_range]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Adds the cells that meet a condition.")},
    {"COUNTIF", Cat::Statistical, "range, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the cells that meet a condition.")},
    {"SUMIFS", Cat::Statistical, "sum_range, criteria_range1, criteria1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Adds the cells that meet several conditions.")},
    {"COUNTIFS", Cat::Statistical, "criteria_range1, criteria1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the rows where several conditions all hold.")},
    {"AVERAGEIF", Cat::Statistical, "range, criteria, [average_range]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Averages the cells that meet a condition.")},
    {"AVERAGEIFS", Cat::Statistical, "average_range, criteria_range1, criteria1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Averages the cells that meet several conditions.")},
    {"MAXIFS", Cat::Statistical, "max_range, criteria_range1, criteria1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the largest number among the cells that meet several conditions.")},
    {"MINIFS", Cat::Statistical, "min_range, criteria_range1, criteria1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the smallest number among the cells that meet several conditions.")},
    {"CORREL", Cat::Statistical, "array1, array2", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives how closely two sets of numbers move together, from -1 to 1.")},
    {"PEARSON", Cat::Statistical, "array1, array2", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives how closely two sets of numbers move together, from -1 to 1.")},
    {"RSQ", Cat::Statistical, "known_y's, known_x's", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives how well a straight line fits two sets of numbers.")},
    {"SLOPE", Cat::Statistical, "known_y's, known_x's", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the slope of the straight line that best fits two sets of numbers.")},
    {"INTERCEPT", Cat::Statistical, "known_y's, known_x's", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives where the best-fitting straight line crosses the vertical axis.")},
    {"FORECAST", Cat::Statistical, "x, known_y's, known_x's", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Predicts a value along the best-fitting straight line.")},
    {"COVAR", Cat::Statistical, "array1, array2", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives how much two sets of numbers vary together.")},
    {"PERCENTILE", Cat::Statistical, "array, k", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the value below which a share k of the numbers fall.")},
    {"QUARTILE", Cat::Statistical, "array, quart", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the quarter points of a set of numbers.")},
    {"NORMSDIST", Cat::Statistical, "z", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the standard normal curve's cumulative share up to z.")},
    {"NORMSINV", Cat::Statistical, "probability", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the z value for a share of the standard normal curve.")},
    {"NORMDIST", Cat::Statistical, "x, mean, standard_dev, cumulative", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the normal curve's value or cumulative share at x.")},
    {"NORMINV", Cat::Statistical, "probability, mean, standard_dev", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the x value for a share of a normal curve.")},
    {"DSUM", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Adds the numbers in a column of a list for the rows that meet the criteria.")},
    {"DAVERAGE", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Averages the numbers in a column of a list for the rows that meet the criteria.")},
    {"DCOUNT", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the numbers in a column of a list for the rows that meet the criteria.")},
    {"DCOUNTA", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the cells that are not empty in a column of a list for the rows that meet the criteria.")},
    {"DMAX", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the largest number in a column of a list for the rows that meet the criteria.")},
    {"DMIN", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the smallest number in a column of a list for the rows that meet the criteria.")},
    {"DGET", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the one value in a column of a list for the single row that meets the criteria.")},
    {"DPRODUCT", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Multiplies the numbers in a column of a list for the rows that meet the criteria.")},
    {"DSTDEV", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Estimates the spread of the numbers in a column of a list for the rows that meet the criteria.")},
    {"DSTDEVP", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the spread of the numbers in a column of a list for the rows that meet the criteria.")},
    {"DVAR", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Estimates the variance of the numbers in a column of a list for the rows that meet the criteria.")},
    {"DVARP", Cat::Database, "database, field, criteria", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the variance of the numbers in a column of a list for the rows that meet the criteria.")},
    {"LEN", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the characters in a text.")},
    {"LEFT", Cat::Text, "text, [num_chars]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the first characters of a text.")},
    {"RIGHT", Cat::Text, "text, [num_chars]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the last characters of a text.")},
    {"MID", Cat::Text, "text, start_num, num_chars", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives characters from the middle of a text.")},
    {"REPLACE", Cat::Text, "old_text, start_num, num_chars, new_text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Puts new text in place of some characters of a text.")},
    {"TRIM", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Takes extra spaces out of a text, leaving single spaces between words.")},
    {"CLEAN", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Takes characters that do not print out of a text.")},
    {"REPT", Cat::Text, "text, number_times", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Repeats a text a number of times.")},
    {"SUBSTITUTE", Cat::Text, "text, old_text, new_text, [instance_num]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Swaps one piece of text for another wherever it occurs.")},
    {"FIND", Cat::Text, "find_text, within_text, [start_num]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives where a text starts inside another, telling capitals from small letters.")},
    {"SEARCH", Cat::Text, "find_text, within_text, [start_num]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives where a text starts inside another, ignoring capitals, and allowing ? and * in what is sought.")},
    {"EXACT", Cat::Text, "text1, text2", QT_TRANSLATE_NOOP("jp::crunch::Functions", "True when two texts are exactly the same, capitals included.")},
    {"CONCATENATE", Cat::Text, "text1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Joins texts into one.")},
    {"CONCAT", Cat::Text, "text1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Joins texts and the texts in blocks of cells into one.")},
    {"TEXTJOIN", Cat::Text, "delimiter, ignore_empty, text1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Joins texts with a separator between them.")},
    {"UPPER", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a text into capital letters.")},
    {"LOWER", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a text into small letters.")},
    {"PROPER", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives each word of a text a capital first letter.")},
    {"T", Cat::Text, "value", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the text itself, and nothing for anything that is not text.")},
    {"CHAR", Cat::Text, "number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the character that has a code number.")},
    {"CODE", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the code number of the first character of a text.")},
    {"VALUE", Cat::Text, "text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a text that looks like a number into a number.")},
    {"TEXT", Cat::Text, "value, format_text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a number into text in a format you choose.")},
    {"FIXED", Cat::Text, "number, [decimals], [no_commas]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a number into text with a set number of decimals.")},
    {"DOLLAR", Cat::Text, "number, [decimals]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a number into text as an amount of money.")},
    {"DATE", Cat::DateTime, "year, month, day", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Makes a date from a year, a month, and a day.")},
    {"YEAR", Cat::DateTime, "serial_number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the year of a date.")},
    {"MONTH", Cat::DateTime, "serial_number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the month of a date, from 1 to 12.")},
    {"DAY", Cat::DateTime, "serial_number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the day of the month of a date.")},
    {"WEEKDAY", Cat::DateTime, "serial_number, [return_type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the day of the week of a date as a number.")},
    {"WEEKNUM", Cat::DateTime, "serial_number, [return_type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the number of the week of the year that a date falls in.")},
    {"ISOWEEKNUM", Cat::DateTime, "date", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the international week number of a date.")},
    {"HOUR", Cat::DateTime, "serial_number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the hour of a time.")},
    {"MINUTE", Cat::DateTime, "serial_number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the minutes of a time.")},
    {"SECOND", Cat::DateTime, "serial_number", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the seconds of a time.")},
    {"TIME", Cat::DateTime, "hour, minute, second", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Makes a time from an hour, minutes, and seconds.")},
    {"TIMEVALUE", Cat::DateTime, "time_text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a time written as text into a time.")},
    {"DATEVALUE", Cat::DateTime, "date_text", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Turns a date written as text into a date.")},
    {"TODAY", Cat::DateTime, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives today's date.")},
    {"NOW", Cat::DateTime, "", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the date and time right now.")},
    {"EDATE", Cat::DateTime, "start_date, months", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the date a number of months before or after a date.")},
    {"EOMONTH", Cat::DateTime, "start_date, months", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the last day of the month a number of months before or after a date.")},
    {"WORKDAY", Cat::DateTime, "start_date, days, [holidays]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the date a number of working days before or after a date.")},
    {"NETWORKDAYS", Cat::DateTime, "start_date, end_date, [holidays]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the working days between two dates.")},
    {"DAYS", Cat::DateTime, "end_date, start_date", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the days between two dates.")},
    {"DATEDIF", Cat::DateTime, "start_date, end_date, unit", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the years, months, or days between two dates.")},
    {"DAYS360", Cat::DateTime, "start_date, end_date, [method]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Counts the days between two dates in a year of twelve 30-day months.")},
    {"YEARFRAC", Cat::DateTime, "start_date, end_date, [basis]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the part of a year between two dates.")},
    {"PMT", Cat::Financial, "rate, nper, pv, [fv], [type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the regular payment on a loan.")},
    {"FV", Cat::Financial, "rate, nper, pmt, [pv], [type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives what regular payments are worth at the end.")},
    {"PV", Cat::Financial, "rate, nper, pmt, [fv], [type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives what a series of future payments is worth today.")},
    {"NPER", Cat::Financial, "rate, pmt, pv, [fv], [type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives how many payments it takes to pay off a loan.")},
    {"IPMT", Cat::Financial, "rate, per, nper, pv, [fv], [type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the interest part of a payment.")},
    {"PPMT", Cat::Financial, "rate, per, nper, pv, [fv], [type]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the part of a payment that pays off the loan itself.")},
    {"NPV", Cat::Financial, "rate, value1, ...", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the present worth of a series of future amounts.")},
    {"SLN", Cat::Financial, "cost, salvage, life", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives one year's straight-line depreciation.")},
    {"SYD", Cat::Financial, "cost, salvage, life, per", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives one year's depreciation by the sum of the years' digits.")},
    {"EFFECT", Cat::Financial, "nominal_rate, npery", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the yearly rate that a stated rate with several periods a year comes to.")},
    {"NOMINAL", Cat::Financial, "effect_rate, npery", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the stated rate for a yearly rate and a number of periods a year.")},
    {"DDB", Cat::Financial, "cost, salvage, life, period, [factor]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives depreciation for a period by the declining balance method.")},
    {"VDB", Cat::Financial, "cost, salvage, life, start_period, end_period, [factor], [no_switch]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives depreciation for any stretch of periods by the declining balance method.")},
    {"IRR", Cat::Financial, "values, [guess]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the rate of return of a series of amounts that come at regular times.")},
    {"XNPV", Cat::Financial, "rate, values, dates", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the present worth of amounts that come on dates you choose.")},
    {"XIRR", Cat::Financial, "values, dates, [guess]", QT_TRANSLATE_NOOP("jp::crunch::Functions", "Gives the rate of return of amounts that come on dates you choose.")},
};

} // namespace

const FunctionDoc *functionDocs(int *count)
{
    *count = int(sizeof(kDocs) / sizeof(kDocs[0]));
    return kDocs;
}

const FunctionDoc *docOf(const QString &name)
{
    const QString upper = name.toUpper();
    for (const FunctionDoc &d : kDocs)
        if (upper == QLatin1String(d.name)) return &d;
    return nullptr;
}

QString categoryName(FunctionCategory category)
{
    switch (category) {
    case Cat::Financial: return QCoreApplication::translate("jp::crunch::Functions", "Financial");
    case Cat::DateTime: return QCoreApplication::translate("jp::crunch::Functions", "Date & Time");
    case Cat::MathTrig: return QCoreApplication::translate("jp::crunch::Functions", "Math & Trig");
    case Cat::Statistical: return QCoreApplication::translate("jp::crunch::Functions", "Statistical");
    case Cat::LookupReference: return QCoreApplication::translate("jp::crunch::Functions", "Lookup & Reference");
    case Cat::Text: return QCoreApplication::translate("jp::crunch::Functions", "Text");
    case Cat::Logical: return QCoreApplication::translate("jp::crunch::Functions", "Logical");
    case Cat::Information: return QCoreApplication::translate("jp::crunch::Functions", "Information");
    case Cat::Database: return QCoreApplication::translate("jp::crunch::Functions", "Database");
    }
    return QString();
}

QString summaryOf(const FunctionDoc &doc) { return QCoreApplication::translate("jp::crunch::Functions", doc.summary); }

QStringList argumentsOf(const FunctionDoc &doc)
{
    QStringList out;
    for (const QString &part : QString::fromLatin1(doc.args).split(QLatin1Char(','), Qt::SkipEmptyParts)) out << part.trimmed();
    return out;
}

QString signatureOf(const FunctionDoc &doc)
{
    return QLatin1String(doc.name) + QLatin1Char('(') + argumentsOf(doc).join(QStringLiteral(", ")) + QLatin1Char(')');
}

} // namespace jp::crunch
