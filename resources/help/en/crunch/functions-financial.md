# Financial functions
<!-- keywords: functions, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT -->

These are the financial functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## ACCRINT

`ACCRINT(issue, first_interest, settlement, rate, par, frequency, [basis], [calc_method])`

Gives the interest accrued on a security that pays interest on a schedule.

- `issue`: The date the security was issued.
- `first_interest`: The date the first interest is paid.
- `settlement`: The date you buy the security.
- `rate`: The interest rate for each period.
- `par`: The face value of the security (1,000 if left out).
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.
- `calc_method` (optional): True or left out to count the interest from the issue date; false to count it from the first interest date when the settlement is later.

## ACCRINTM

`ACCRINTM(issue, settlement, rate, [par], [basis])`

Gives the interest accrued on a security that pays interest at maturity.

- `issue`: The date the security was issued.
- `settlement`: The date you buy the security.
- `rate`: The interest rate for each period.
- `par` (optional): The face value of the security (1,000 if left out).
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## AMORDEGRC

`AMORDEGRC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Gives the depreciation of a period under the French declining-balance method.

- `cost`: What the asset cost when it was bought.
- `date_purchased`: The date the asset was bought.
- `first_period`: The date the first period ends.
- `salvage`: What the asset is worth at the end.
- `period`: Which period to work on, counting the first as 1.
- `rate`: The interest rate for each period.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## AMORLINC

`AMORLINC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Gives the depreciation of a period under the French straight-line method.

- `cost`: What the asset cost when it was bought.
- `date_purchased`: The date the asset was bought.
- `first_period`: The date the first period ends.
- `salvage`: What the asset is worth at the end.
- `period`: Which period to work on, counting the first as 1.
- `rate`: The interest rate for each period.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## COUPDAYBS

`COUPDAYBS(settlement, maturity, frequency, [basis])`

Gives the days from the start of the coupon period to the settlement date.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## COUPDAYS

`COUPDAYS(settlement, maturity, frequency, [basis])`

Gives the days in the coupon period that holds the settlement date.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## COUPDAYSNC

`COUPDAYSNC(settlement, maturity, frequency, [basis])`

Gives the days from the settlement date to the next coupon date.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## COUPNCD

`COUPNCD(settlement, maturity, frequency, [basis])`

Gives the next coupon date after the settlement date.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## COUPNUM

`COUPNUM(settlement, maturity, frequency, [basis])`

Gives the number of coupons to be paid between the settlement and maturity dates.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## COUPPCD

`COUPPCD(settlement, maturity, frequency, [basis])`

Gives the coupon date before the settlement date.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## CUMIPMT

`CUMIPMT(rate, nper, pv, start_period, end_period, type)`

Gives the interest paid on a loan between two periods.

- `rate`: The interest rate for each period.
- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `start_period`: The period the stretch starts at.
- `end_period`: The period the stretch ends at.
- `type`: When payments are made: 0 or left out at the end of each period, 1 at the start.

## CUMPRINC

`CUMPRINC(rate, nper, pv, start_period, end_period, type)`

Gives the principal paid on a loan between two periods.

- `rate`: The interest rate for each period.
- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `start_period`: The period the stretch starts at.
- `end_period`: The period the stretch ends at.
- `type`: When payments are made: 0 or left out at the end of each period, 1 at the start.

## DB

`DB(cost, salvage, life, period, [month])`

Gives the depreciation of an asset for a period, by the fixed declining balance.

- `cost`: What the asset cost when it was bought.
- `salvage`: What the asset is worth at the end.
- `life`: How many periods the asset lasts.
- `period`: Which period to work on, counting the first as 1.
- `month` (optional): The month, from 1 to 12.

## DDB

`DDB(cost, salvage, life, period, [factor])`

Gives depreciation for a period by the declining balance method.

- `cost`: What the asset cost when it was bought.
- `salvage`: What the asset is worth at the end.
- `life`: How many periods the asset lasts.
- `period`: Which period to work on, counting the first as 1.
- `factor` (optional): How fast the balance declines (2 if left out, which is double declining).

## DISC

`DISC(settlement, maturity, pr, redemption, [basis])`

Gives the discount rate of a security.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `pr`: The price of the security per 100 of face value.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## DOLLARDE

`DOLLARDE(fractional_dollar, fraction)`

Turns a price written as a fraction, such as 1.02, into a decimal number.

- `fractional_dollar`: A price written as a fraction, such as 1.02 for one and two sixteenths.
- `fraction`: The number to divide a dollar by: 16 for sixteenths, for example.

## DOLLARFR

`DOLLARFR(decimal_dollar, fraction)`

Turns a decimal price into one written as a fraction, such as 1.02.

- `decimal_dollar`: A price written as a decimal number, such as 1.125.
- `fraction`: The number to divide a dollar by: 16 for sixteenths, for example.

## DURATION

`DURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Gives the average time to get back the cost of a bond, weighted by the payments.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `coupon`: The yearly coupon rate of the bond.
- `yld`: The yearly yield of the security.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## EFFECT

`EFFECT(nominal_rate, npery)`

Gives the yearly rate that a stated rate with several periods a year comes to.

- `nominal_rate`: The stated yearly rate.
- `npery`: How many periods there are in a year.

## FV

`FV(rate, nper, pmt, [pv], [type])`

Gives what regular payments are worth at the end.

- `rate`: The interest rate for each period.
- `nper`: How many payments there are in all.
- `pmt`: The payment made each period.
- `pv` (optional): What the amounts are worth now (the whole loan or investment).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.

## FVSCHEDULE

`FVSCHEDULE(principal, schedule)`

Gives the future value of an amount after a list of interest rates.

- `principal`: The amount to start with.
- `schedule`: The block of cells with the interest rates to apply, one after another.

## INTRATE

`INTRATE(settlement, maturity, investment, redemption, [basis])`

Gives the interest rate of a fully invested security.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `investment`: The amount put into the security.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## IPMT

`IPMT(rate, per, nper, pv, [fv], [type])`

Gives the interest part of a payment.

- `rate`: The interest rate for each period.
- `per`: Which payment to work on, counting the first as 1.
- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv` (optional): What is left after the last payment (0 if left out).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.

## IRR

`IRR(values, [guess])`

Gives the rate of return of a series of amounts that come at regular times.

- `values`: The block of cells that holds the amounts.
- `guess` (optional): Your guess at the result, to start the search from (0.1 if left out).

## ISPMT

`ISPMT(rate, per, nper, pv)`

Gives the interest paid in a period of a loan that is paid back in equal parts of principal.

- `rate`: The interest rate for each period.
- `per`: Which payment to work on, counting the first as 1.
- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).

## MDURATION

`MDURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Gives the duration of a bond, adjusted for its yield.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `coupon`: The yearly coupon rate of the bond.
- `yld`: The yearly yield of the security.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## MIRR

`MIRR(values, finance_rate, reinvest_rate)`

Gives the rate of return of cash flows, with one rate to borrow and another to reinvest.

- `values`: The block of cells that holds the amounts.
- `finance_rate`: The rate you pay on the money you borrow.
- `reinvest_rate`: The rate you earn on the money you reinvest.

## NOMINAL

`NOMINAL(effect_rate, npery)`

Gives the stated rate for a yearly rate and a number of periods a year.

- `effect_rate`: The yearly rate.
- `npery`: How many periods there are in a year.

## NPER

`NPER(rate, pmt, pv, [fv], [type])`

Gives how many payments it takes to pay off a loan.

- `rate`: The interest rate for each period.
- `pmt`: The payment made each period.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv` (optional): What is left after the last payment (0 if left out).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.

## NPV

`NPV(rate, value1, ...)`

Gives the present worth of a series of future amounts.

- `rate`: The interest rate for each period.
- `value1`: The first value, or block of cells.
- `...`: As many more as you need, of the same kind as the one before it.

## ODDFPRICE

`ODDFPRICE(settlement, maturity, issue, first_coupon, rate, yld, redemption, frequency, [basis])`

Gives the price per 100 of a bond with an odd first period.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `issue`: The date the security was issued.
- `first_coupon`: The date of the first coupon.
- `rate`: The interest rate for each period.
- `yld`: The yearly yield of the security.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## ODDFYIELD

`ODDFYIELD(settlement, maturity, issue, first_coupon, rate, pr, redemption, frequency, [basis])`

Gives the yield of a bond with an odd first period.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `issue`: The date the security was issued.
- `first_coupon`: The date of the first coupon.
- `rate`: The interest rate for each period.
- `pr`: The price of the security per 100 of face value.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## ODDLPRICE

`ODDLPRICE(settlement, maturity, last_interest, rate, yld, redemption, frequency, [basis])`

Gives the price per 100 of a bond with an odd last period.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `last_interest`: The date the last interest was paid.
- `rate`: The interest rate for each period.
- `yld`: The yearly yield of the security.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## ODDLYIELD

`ODDLYIELD(settlement, maturity, last_interest, rate, pr, redemption, frequency, [basis])`

Gives the yield of a bond with an odd last period.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `last_interest`: The date the last interest was paid.
- `rate`: The interest rate for each period.
- `pr`: The price of the security per 100 of face value.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## PDURATION

`PDURATION(rate, pv, fv)`

Gives the periods it takes an investment to reach a value.

- `rate`: The interest rate for each period.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv`: What is left after the last payment (0 if left out).

## PMT

`PMT(rate, nper, pv, [fv], [type])`

Gives the regular payment on a loan.

- `rate`: The interest rate for each period.
- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv` (optional): What is left after the last payment (0 if left out).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.

## PPMT

`PPMT(rate, per, nper, pv, [fv], [type])`

Gives the part of a payment that pays off the loan itself.

- `rate`: The interest rate for each period.
- `per`: Which payment to work on, counting the first as 1.
- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv` (optional): What is left after the last payment (0 if left out).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.

## PRICE

`PRICE(settlement, maturity, rate, yld, redemption, frequency, [basis])`

Gives the price per 100 of a bond that pays interest on a schedule.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `rate`: The interest rate for each period.
- `yld`: The yearly yield of the security.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## PRICEDISC

`PRICEDISC(settlement, maturity, discount, redemption, [basis])`

Gives the price per 100 of a security bought at a discount.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `discount`: The discount rate of the security.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## PRICEMAT

`PRICEMAT(settlement, maturity, issue, rate, yld, [basis])`

Gives the price per 100 of a security that pays interest at maturity.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `issue`: The date the security was issued.
- `rate`: The interest rate for each period.
- `yld`: The yearly yield of the security.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## PV

`PV(rate, nper, pmt, [fv], [type])`

Gives what a series of future payments is worth today.

- `rate`: The interest rate for each period.
- `nper`: How many payments there are in all.
- `pmt`: The payment made each period.
- `fv` (optional): What is left after the last payment (0 if left out).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.

## RATE

`RATE(nper, pmt, pv, [fv], [type], [guess])`

Gives the interest rate for each period of a loan or an investment.

- `nper`: How many payments there are in all.
- `pmt`: The payment made each period.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv` (optional): What is left after the last payment (0 if left out).
- `type` (optional): When payments are made: 0 or left out at the end of each period, 1 at the start.
- `guess` (optional): Your guess at the result, to start the search from (0.1 if left out).

## RECEIVED

`RECEIVED(settlement, maturity, investment, discount, [basis])`

Gives the amount received at maturity for a fully invested security.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `investment`: The amount put into the security.
- `discount`: The discount rate of the security.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## RRI

`RRI(nper, pv, fv)`

Gives the interest rate that makes an investment grow to a value.

- `nper`: How many payments there are in all.
- `pv`: What the amounts are worth now (the whole loan or investment).
- `fv`: What is left after the last payment (0 if left out).

## SLN

`SLN(cost, salvage, life)`

Gives one year's straight-line depreciation.

- `cost`: What the asset cost when it was bought.
- `salvage`: What the asset is worth at the end.
- `life`: How many periods the asset lasts.

## SYD

`SYD(cost, salvage, life, per)`

Gives one year's depreciation by the sum of the years' digits.

- `cost`: What the asset cost when it was bought.
- `salvage`: What the asset is worth at the end.
- `life`: How many periods the asset lasts.
- `per`: Which payment to work on, counting the first as 1.

## TBILLEQ

`TBILLEQ(settlement, maturity, discount)`

Gives the bond-equivalent yield of a Treasury bill.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `discount`: The discount rate of the security.

## TBILLPRICE

`TBILLPRICE(settlement, maturity, discount)`

Gives the price per 100 of a Treasury bill.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `discount`: The discount rate of the security.

## TBILLYIELD

`TBILLYIELD(settlement, maturity, pr)`

Gives the yield of a Treasury bill.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `pr`: The price of the security per 100 of face value.

## VDB

`VDB(cost, salvage, life, start_period, end_period, [factor], [no_switch])`

Gives depreciation for any stretch of periods by the declining balance method.

- `cost`: What the asset cost when it was bought.
- `salvage`: What the asset is worth at the end.
- `life`: How many periods the asset lasts.
- `start_period`: The period the stretch starts at.
- `end_period`: The period the stretch ends at.
- `factor` (optional): How fast the balance declines (2 if left out, which is double declining).
- `no_switch` (optional): True to keep using the declining balance method after straight-line would give more.

## XIRR

`XIRR(values, dates, [guess])`

Gives the rate of return of amounts that come on dates you choose.

- `values`: The block of cells that holds the amounts.
- `dates`: The dates of the amounts, in the same order as the amounts.
- `guess` (optional): Your guess at the result, to start the search from (0.1 if left out).

## XNPV

`XNPV(rate, values, dates)`

Gives the present worth of amounts that come on dates you choose.

- `rate`: The interest rate for each period.
- `values`: The block of cells that holds the amounts.
- `dates`: The dates of the amounts, in the same order as the amounts.

## YIELD

`YIELD(settlement, maturity, rate, pr, redemption, frequency, [basis])`

Gives the yield of a bond that pays interest on a schedule.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `rate`: The interest rate for each period.
- `pr`: The price of the security per 100 of face value.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `frequency`: How many coupons are paid in a year: 1, 2, or 4.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## YIELDDISC

`YIELDDISC(settlement, maturity, pr, redemption, [basis])`

Gives the yearly yield of a security bought at a discount.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `pr`: The price of the security per 100 of face value.
- `redemption`: The value of the security at maturity, per 100 of face value.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.

## YIELDMAT

`YIELDMAT(settlement, maturity, issue, rate, pr, [basis])`

Gives the yearly yield of a security that pays interest at maturity.

- `settlement`: The date you buy the security.
- `maturity`: The date the security ends and is paid back.
- `issue`: The date the security was issued.
- `rate`: The interest rate for each period.
- `pr`: The price of the security per 100 of face value.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.
