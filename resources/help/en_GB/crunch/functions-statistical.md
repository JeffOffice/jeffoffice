# Statistical functions
<!-- keywords: functions, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST, STANDARDISE -->

These are the statistical functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## AVEDEV

`AVEDEV(number1, ...)`

Gives the average of how far numbers are from their mean.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## AVERAGE

`AVERAGE(number1, ...)`

Gives the average of numbers.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## AVERAGEA

`AVERAGEA(value1, ...)`

Gives the average of values, counting text as zero and true as one.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Averages the cells that meet a condition.

- `range`: The block of cells to test.
- `criteria`: What to look for: a number, a text, or a test such as ">5".
- `average_range` (optional): The cells to average (the cells to look in if left out).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Averages the cells that meet several conditions.

- `average_range`: The cells to average (the cells to look in if left out).
- `criteria_range1`: The first block of cells to test.
- `criteria1`: What to look for in the first block of cells: a number, a text, or a test such as ">5".
- `...`: As many more as you need, of the same kind as the one before it.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Gives the beta curve's value or cumulative share at x.

- `x`: The x value to work with.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.
- `A` (optional): The lower bound of x (0 if left out).
- `B` (optional): The upper bound of x (1 if left out).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Gives the x value for a share of a beta curve.

- `probability`: A probability, from 0 to 1.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `A` (optional): The lower bound of x (0 if left out).
- `B` (optional): The upper bound of x (1 if left out).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Gives the beta curve's cumulative share at x.

- `x`: The x value to work with.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `A` (optional): The lower bound of x (0 if left out).
- `B` (optional): The upper bound of x (1 if left out).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Gives the x value for a share of a beta curve.

- `probability`: A probability, from 0 to 1.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `A` (optional): The lower bound of x (0 if left out).
- `B` (optional): The upper bound of x (1 if left out).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Gives the chance of a number of successes in a number of trials.

- `number_s`: The number of successes.
- `trials`: The number of trials.
- `probability_s`: The chance of a success on each trial, from 0 to 1.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Gives the chance that the successes fall in a range.

- `trials`: The number of trials.
- `probability_s`: The chance of a success on each trial, from 0 to 1.
- `number_s`: The number of successes.
- `number_s2` (optional): The most successes to count (the same as the least if left out).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Gives the least number of successes whose cumulative chance reaches a value.

- `trials`: The number of trials.
- `probability_s`: The chance of a success on each trial, from 0 to 1.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Gives the chance of a number of successes in a number of trials.

- `number_s`: The number of successes.
- `trials`: The number of trials.
- `probability_s`: The chance of a success on each trial, from 0 to 1.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Gives the share of the chi-squared curve above x.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## CHIINV

`CHIINV(probability, deg_freedom)`

Gives the x value for a share of the chi-squared curve, counted from the right.

- `probability`: A probability, from 0 to 1.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Gives the chi-squared curve's value or cumulative share at x.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Gives the share of the chi-squared curve above x.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Gives the x value for a share of the chi-squared curve, counted from the left.

- `probability`: A probability, from 0 to 1.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Gives the x value for a share of the chi-squared curve, counted from the right.

- `probability`: A probability, from 0 to 1.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Gives the chance that counts differ from the expected counts only by luck.

- `actual_range`: The block of cells with the counts you got.
- `expected_range`: The block of cells with the counts you expected, the same size as the counts you got.

## CHITEST

`CHITEST(actual_range, expected_range)`

Gives the chance that counts differ from the expected counts only by luck.

- `actual_range`: The block of cells with the counts you got.
- `expected_range`: The block of cells with the counts you expected, the same size as the counts you got.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Gives the margin of a confidence interval for a mean, using the normal curve.

- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `standard_dev`: The standard deviation of the distribution.
- `size`: The size of the sample, a whole number of 1 or more.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Gives the margin of a confidence interval for a mean, using the normal curve.

- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `standard_dev`: The standard deviation of the distribution.
- `size`: The size of the sample, a whole number of 1 or more.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Gives the margin of a confidence interval for a mean, using Student's t curve.

- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `standard_dev`: The standard deviation of the distribution.
- `size`: The size of the sample, a whole number of 1 or more.

## CORREL

`CORREL(array1, array2)`

Gives how closely two sets of numbers move together, from -1 to 1.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## COUNT

`COUNT(value1, ...)`

Counts the numbers in a list or a block of cells.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## COUNTA

`COUNTA(value1, ...)`

Counts the cells that are not empty.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## COUNTBLANK

`COUNTBLANK(range)`

Counts the empty cells in a block.

- `range`: The block of cells to test.

## COUNTIF

`COUNTIF(range, criteria)`

Counts the cells that meet a condition.

- `range`: The block of cells to test.
- `criteria`: What to look for: a number, a text, or a test such as ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Counts the rows where several conditions all hold.

- `criteria_range1`: The first block of cells to test.
- `criteria1`: What to look for in the first block of cells: a number, a text, or a test such as ">5".
- `...`: As many more as you need, of the same kind as the one before it.

## COVAR

`COVAR(array1, array2)`

Gives how much two sets of numbers vary together.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Gives how much two sets of numbers vary together, for a population.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Gives how much two sets of numbers vary together, for a sample.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Gives the least number of successes whose cumulative chance reaches a value.

- `trials`: The number of trials.
- `probability_s`: The chance of a success on each trial, from 0 to 1.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.

## DEVSQ

`DEVSQ(number1, ...)`

Gives the sum of the squares of how far numbers are from their mean.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Gives the exponential curve's value or cumulative share at x.

- `x`: The x value to work with.
- `lambda`: The rate of the curve, more than 0.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Gives the exponential curve's value or cumulative share at x.

- `x`: The x value to work with.
- `lambda`: The rate of the curve, more than 0.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Gives the F curve's value or cumulative share at x.

- `x`: The x value to work with.
- `deg_freedom1`: The degrees of freedom of the numerator, a whole number of 1 or more.
- `deg_freedom2`: The degrees of freedom of the denominator, a whole number of 1 or more.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Gives the share of the F curve above x.

- `x`: The x value to work with.
- `deg_freedom1`: The degrees of freedom of the numerator, a whole number of 1 or more.
- `deg_freedom2`: The degrees of freedom of the denominator, a whole number of 1 or more.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Gives the x value for a share of the F curve, counted from the left.

- `probability`: A probability, from 0 to 1.
- `deg_freedom1`: The degrees of freedom of the numerator, a whole number of 1 or more.
- `deg_freedom2`: The degrees of freedom of the denominator, a whole number of 1 or more.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Gives the x value for a share of the F curve, counted from the right.

- `probability`: A probability, from 0 to 1.
- `deg_freedom1`: The degrees of freedom of the numerator, a whole number of 1 or more.
- `deg_freedom2`: The degrees of freedom of the denominator, a whole number of 1 or more.

## F.TEST

`F.TEST(array1, array2)`

Gives the chance that two sets of numbers have the same variance.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Gives the share of the F curve above x.

- `x`: The x value to work with.
- `deg_freedom1`: The degrees of freedom of the numerator, a whole number of 1 or more.
- `deg_freedom2`: The degrees of freedom of the denominator, a whole number of 1 or more.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Gives the x value for a share of the F curve, counted from the right.

- `probability`: A probability, from 0 to 1.
- `deg_freedom1`: The degrees of freedom of the numerator, a whole number of 1 or more.
- `deg_freedom2`: The degrees of freedom of the denominator, a whole number of 1 or more.

## FISHER

`FISHER(x)`

Gives the Fisher transformation of a number between -1 and 1.

- `x`: The x value to work with.

## FISHERINV

`FISHERINV(y)`

Gives the inverse of the Fisher transformation.

- `y`: The number to find the Fisher transformation's inverse of.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Predicts a value along the best-fitting straight line.

- `x`: The x value to predict a y value for.
- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's`: The block of cells that holds the x values you already have.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Predicts a value along the best-fitting straight line.

- `x`: The x value to work with.
- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's`: The block of cells that holds the x values you already have.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Counts how many values fall in each range, and gives the counts as a column.

- `data_array`: The block of cells with the values to count.
- `bins_array`: The block of cells with the limits of the ranges to count in.

## FTEST

`FTEST(array1, array2)`

Gives the chance that two sets of numbers have the same variance.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## GAMMA

`GAMMA(x)`

Gives the gamma function of a number.

- `x`: The x value to work with.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Gives the gamma curve's value or cumulative share at x.

- `x`: The x value to work with.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Gives the x value for a share of a gamma curve.

- `probability`: A probability, from 0 to 1.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Gives the gamma curve's value or cumulative share at x.

- `x`: The x value to work with.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Gives the x value for a share of a gamma curve.

- `probability`: A probability, from 0 to 1.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).

## GAMMALN

`GAMMALN(x)`

Gives the natural logarithm of the gamma function.

- `x`: The x value to work with.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Gives the natural logarithm of the gamma function.

- `x`: The x value to work with.

## GAUSS

`GAUSS(z)`

Gives the share of the standard normal curve between the mean and z.

- `z`: The z value.

## GEOMEAN

`GEOMEAN(number1, ...)`

Gives the geometric mean of positive numbers.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Gives the values along the best-fitting exponential curve for new x values.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's` (optional): The block of cells that holds the x values you already have.
- `new_x's` (optional): The x values to give the y values for (the known x values if left out).
- `const` (optional): True or left out to fit a line that does not have to pass through zero; false to make it pass through zero.

## HARMEAN

`HARMEAN(number1, ...)`

Gives the harmonic mean of positive numbers.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Gives the chance of drawing a number of successes without putting items back.

- `sample_s`: The number of successes in the sample.
- `number_sample`: The number of items drawn.
- `population_s`: The number of successes in the whole population.
- `number_pop`: The number of items in the whole population.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Gives the chance of drawing a number of successes without putting items back.

- `sample_s`: The number of successes in the sample.
- `number_sample`: The number of items drawn.
- `population_s`: The number of successes in the whole population.
- `number_pop`: The number of items in the whole population.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Gives where the best-fitting straight line crosses the vertical axis.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's`: The block of cells that holds the x values you already have.

## KURT

`KURT(number1, ...)`

Gives how peaked or flat a set of numbers is compared with a normal curve.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## LARGE

`LARGE(array, k)`

Gives the k-th largest number of a set.

- `array`: A block of cells, or a list of values written in braces.
- `k`: Which place to give, counting from the largest: 1 is the largest.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Fits a straight line or plane to the data by least squares, and gives its numbers as a row.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's` (optional): The block of cells that holds the x values you already have.
- `const` (optional): True or left out to fit a line that does not have to pass through zero; false to make it pass through zero.
- `stats` (optional): True to give more numbers about the fit as well; false or left out to give the line only.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Fits an exponential curve to the data, and gives its numbers as a row.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's` (optional): The block of cells that holds the x values you already have.
- `const` (optional): True or left out to fit a line that does not have to pass through zero; false to make it pass through zero.
- `stats` (optional): True to give more numbers about the fit as well; false or left out to give the line only.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Gives the x value for a share of a log-normal curve.

- `probability`: A probability, from 0 to 1.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Gives the log-normal curve's value or cumulative share at x.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Gives the x value for a share of a log-normal curve.

- `probability`: A probability, from 0 to 1.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Gives the log-normal curve's cumulative share at x.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.

## MAXA

`MAXA(value1, ...)`

Gives the largest value, counting text as zero and true as one.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Gives the largest number among the cells that meet several conditions.

- `max_range`: The cells to take the largest value from.
- `criteria_range1`: The first block of cells to test.
- `criteria1`: What to look for in the first block of cells: a number, a text, or a test such as ">5".
- `...`: As many more as you need, of the same kind as the one before it.

## MEDIAN

`MEDIAN(number1, ...)`

Gives the middle number of a set.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MINA

`MINA(value1, ...)`

Gives the smallest value, counting text as zero and true as one.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Gives the smallest number among the cells that meet several conditions.

- `min_range`: The cells to take the smallest value from.
- `criteria_range1`: The first block of cells to test.
- `criteria1`: What to look for in the first block of cells: a number, a text, or a test such as ">5".
- `...`: As many more as you need, of the same kind as the one before it.

## MODE

`MODE(number1, ...)`

Gives the number that occurs most often.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MODE.MULT

`MODE.MULT(number1, ...)`

Gives a column of the numbers that come most often in a set.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Gives the number that comes most often in a set.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Gives the chance of a number of failures before a number of successes.

- `number_f`: The number of failures.
- `number_s`: The number of successes.
- `probability_s`: The chance of a success on each trial, from 0 to 1.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Gives the chance of a number of failures before a number of successes.

- `number_f`: The number of failures.
- `number_s`: The number of successes.
- `probability_s`: The chance of a success on each trial, from 0 to 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Gives the normal curve's value or cumulative share at x.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Gives the x value for a share of a normal curve.

- `probability`: A probability, from 0 to 1.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Gives the standard normal curve's value or cumulative share at z.

- `z`: The z value.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## NORM.S.INV

`NORM.S.INV(probability)`

Gives the z value for a share of the standard normal curve.

- `probability`: A probability, from 0 to 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Gives the normal curve's value or cumulative share at x.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Gives the x value for a share of a normal curve.

- `probability`: A probability, from 0 to 1.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.

## NORMSDIST

`NORMSDIST(z)`

Gives the standard normal curve's cumulative share up to z.

- `z`: The z value.

## NORMSINV

`NORMSINV(probability)`

Gives the z value for a share of the standard normal curve.

- `probability`: A probability, from 0 to 1.

## PEARSON

`PEARSON(array1, array2)`

Gives how closely two sets of numbers move together, from -1 to 1.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.

## PERCENTILE

`PERCENTILE(array, k)`

Gives the value below which a share k of the numbers fall.

- `array`: A block of cells, or a list of values written in braces.
- `k`: The share of the values, from 0 to 1, to give the value at.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Gives the value below which a share k of the numbers fall, leaving out 0 and 1.

- `array`: A block of cells, or a list of values written in braces.
- `k`: The place in the order, counting from 1.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Gives the value below which a share k of the numbers fall, from 0 to 1.

- `array`: A block of cells, or a list of values written in braces.
- `k`: The place in the order, counting from 1.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Gives the rank of a value in a set of numbers as a share from 0 to 1.

- `array`: A block of cells, or a list of values written in braces.
- `x`: The x value to work with.
- `significance` (optional): The number whose multiples to round to.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Gives the rank of a value in a set of numbers as a share, leaving out 0 and 1.

- `array`: A block of cells, or a list of values written in braces.
- `x`: The x value to work with.
- `significance` (optional): The number whose multiples to round to.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Gives the rank of a value in a set of numbers as a share from 0 to 1.

- `array`: A block of cells, or a list of values written in braces.
- `x`: The x value to work with.
- `significance` (optional): The number whose multiples to round to.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Gives the number of ordered choices when an item can be chosen more than once.

- `number`: The number to work on.
- `number_chosen`: How many items to pick in each group.

## PHI

`PHI(x)`

Gives the height of the standard normal curve at x.

- `x`: The x value to work with.

## POISSON

`POISSON(x, mean, cumulative)`

Gives the chance of a number of events, from how many to expect.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Gives the chance of a number of events, from how many to expect.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Gives the chance that values fall between two limits.

- `x_range`: The block of cells with the x values.
- `prob_range`: The chances that go with each x value, each from 0 to 1, adding up to 1.
- `lower_limit`: The lower limit of the range.
- `upper_limit` (optional): The upper limit of the range (the lower limit alone if left out).

## QUARTILE

`QUARTILE(array, quart)`

Gives the quarter points of a set of numbers.

- `array`: A block of cells, or a list of values written in braces.
- `quart`: Which quartile: 0 smallest, 1 first quarter, 2 median, 3 third quarter, 4 largest.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Gives the quarter points of a set of numbers, leaving out the ends.

- `array`: A block of cells, or a list of values written in braces.
- `quart`: Which quartile: 0 smallest, 1 first quarter, 2 median, 3 third quarter, 4 largest.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Gives the quarter points of a set of numbers, from the lowest to the highest.

- `array`: A block of cells, or a list of values written in braces.
- `quart`: Which quartile: 0 smallest, 1 first quarter, 2 median, 3 third quarter, 4 largest.

## RANK

`RANK(number, ref, [order])`

Gives the place of a number in a list when the list is put in order.

- `number`: The number to work on.
- `ref`: A cell or block of cells.
- `order` (optional): 0 or left out puts the largest first; any other number puts the smallest first.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Gives the rank of a number in a list, and the average rank when some tie.

- `number`: The number to work on.
- `ref`: A cell or block of cells.
- `order` (optional): 0 or left out puts the largest first; any other number puts the smallest first.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Gives the rank of a number in a list, and the top rank when some tie.

- `number`: The number to work on.
- `ref`: A cell or block of cells.
- `order` (optional): 0 or left out puts the largest first; any other number puts the smallest first.

## RSQ

`RSQ(known_y's, known_x's)`

Gives how well a straight line fits two sets of numbers.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's`: The block of cells that holds the x values you already have.

## SKEW

`SKEW(number1, ...)`

Gives how lopsided a set of numbers is around its mean, for a sample.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## SKEW.P

`SKEW.P(number1, ...)`

Gives how lopsided a set of numbers is around its mean, for a whole population.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## SLOPE

`SLOPE(known_y's, known_x's)`

Gives the slope of the straight line that best fits two sets of numbers.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's`: The block of cells that holds the x values you already have.

## SMALL

`SMALL(array, k)`

Gives the k-th smallest number of a set.

- `array`: A block of cells, or a list of values written in braces.
- `k`: Which place to give, counting from the smallest: 1 is the smallest.

## STANDARDISE

`STANDARDIZE(x, mean, standard_dev)`

Gives how many standard deviations a value is from the mean.

- `x`: The x value to work with.
- `mean`: The average of the distribution.
- `standard_dev`: The standard deviation of the distribution.

## STDEV

`STDEV(number1, ...)`

Estimates the spread of a whole group from a sample of it.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## STDEV.P

`STDEV.P(number1, ...)`

Gives the spread of a whole group.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## STDEV.S

`STDEV.S(number1, ...)`

Estimates the spread of a whole group from a sample of it.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## STDEVA

`STDEVA(value1, ...)`

Gives the standard deviation of a sample, counting text as 0 and true as 1.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## STDEVP

`STDEVP(number1, ...)`

Gives the spread of a whole group.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## STDEVPA

`STDEVPA(value1, ...)`

Gives the standard deviation of a population, counting text as 0 and true as 1.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## STEYX

`STEYX(known_y's, known_x's)`

Gives the standard error of the predicted y values of a fitted line.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's`: The block of cells that holds the x values you already have.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Adds the cells that meet a condition.

- `range`: The block of cells to test.
- `criteria`: What to look for: a number, a text, or a test such as ">5".
- `sum_range` (optional): The cells to add up (the cells that are tested if left out).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Adds the cells that meet several conditions.

- `sum_range`: The cells to add up (the cells that are tested if left out).
- `criteria_range1`: The first block of cells to test.
- `criteria1`: What to look for in the first block of cells: a number, a text, or a test such as ">5".
- `...`: As many more as you need, of the same kind as the one before it.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Gives Student's t curve's value or cumulative share at x.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Gives the share of Student's t curve outside minus x and x.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Gives the share of Student's t curve above x.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## T.INV

`T.INV(probability, deg_freedom)`

Gives the x value for a share of Student's t curve, counted from the left.

- `probability`: A probability, from 0 to 1.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Gives the x value for a share of Student's t curve, split between both tails.

- `probability`: A probability, from 0 to 1.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Gives the chance that the means of two sets of numbers are the same.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.
- `tails`: 1 for one tail of the curve, 2 for both.
- `type`: When payments are made: 0 or left out at the end of each period, 1 at the start.

## TDIST

`TDIST(x, deg_freedom, tails)`

Gives the share of Student's t curve in one tail or in both.

- `x`: The x value to work with.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.
- `tails`: 1 for one tail of the curve, 2 for both.

## TINV

`TINV(probability, deg_freedom)`

Gives the x value for a share of Student's t curve, split between both tails.

- `probability`: A probability, from 0 to 1.
- `deg_freedom`: The degrees of freedom, a whole number of 1 or more.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Gives the values along the best-fitting straight line for new x values.

- `known_y's`: The block of cells that holds the y values you already have.
- `known_x's` (optional): The block of cells that holds the x values you already have.
- `new_x's` (optional): The x values to give the y values for (the known x values if left out).
- `const` (optional): True or left out to fit a line that does not have to pass through zero; false to make it pass through zero.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Gives the mean of a set of numbers after cutting a share of the highest and lowest.

- `array`: A block of cells, or a list of values written in braces.
- `percent`: The share of the values, from 0 up to but not including 1, to leave out at the two ends.

## TTEST

`TTEST(array1, array2, tails, type)`

Gives the chance that the means of two sets of numbers are the same.

- `array1`: The first block of cells or list of values.
- `array2`: The second block of cells or list of values, the same size as the first.
- `tails`: 1 for one tail of the curve, 2 for both.
- `type`: When payments are made: 0 or left out at the end of each period, 1 at the start.

## VAR

`VAR(number1, ...)`

Estimates the variance of a whole group from a sample of it.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## VAR.P

`VAR.P(number1, ...)`

Gives the variance of a whole group.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## VAR.S

`VAR.S(number1, ...)`

Estimates the variance of a whole group from a sample of it.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## VARA

`VARA(value1, ...)`

Gives the variance of a sample, counting text as 0 and true as 1.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## VARP

`VARP(number1, ...)`

Gives the variance of a whole group.

- `number1`: The first number, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## VARPA

`VARPA(value1, ...)`

Gives the variance of a population, counting text as 0 and true as 1.

- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Gives the Weibull curve's value or cumulative share at x.

- `x`: The x value to work with.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Gives the Weibull curve's value or cumulative share at x.

- `x`: The x value to work with.
- `alpha`: The chance of being wrong that you accept, such as 0.05 for a 95 percent confidence level.
- `beta`: A number that shapes the curve (the scale of the curve for the gamma and Weibull curves).
- `cumulative`: True to give the running total up to this point; false to give the value at this point alone.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Gives the one-sided chance that the mean of a sample is above a value.

- `array`: A block of cells, or a list of values written in braces.
- `x`: The x value to work with.
- `sigma` (optional): The standard deviation of the whole population (that of the sample if left out).

## ZTEST

`ZTEST(array, x, [sigma])`

Gives the one-sided chance that the mean of a sample is above a value.

- `array`: A block of cells, or a list of values written in braces.
- `x`: The x value to work with.
- `sigma` (optional): The standard deviation of the whole population (that of the sample if left out).
