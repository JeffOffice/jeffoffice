# Funciones financieras
<!-- keywords: funciones, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT, funciones financieras, INT.ACUMULADO, INT.ACUM.V, AMORDEGRC, AMORLINC, CUPON.DIAS.L1, CUPON.DIAS, CUPON.DIAS.L2, CUPON.FECHA.L2, CUPON.NUM, CUPON.FECHA.L1, PAGO.INT.ENTRE, PAGO.PRINC.ENTRE, DB, DDB, TASA.DESC, MONEDA.DEC, MONEDA.FRAC, DURACION, INT.EFECTIVO, VF, VF.PLAN, TASA.INT, PAGOINT, TIR, INT.PAGO.DIR, DURACION.MODIFICADA, TIRM, TASA.NOMINAL, NPER, VAN, PRECIO.PER.IRREGULAR.1, RENDTO.PER.IRREGULAR.1, PRECIO.PER.IRREGULAR.2, RENDTO.PER.IRREGULAR.2, NPER.INC, PAGO, PAGOPRIN, PRECIO, PRECIO.DESCUENTO, PRECIO.VENCIMIENTO, VA, TASA, CANTIDAD.RECIBIDA, TASA.INT.EQUIV, SLN, SYD, LETRA.EQV.BONO, LETRA.PRECIO, LETRA.RENTABILIDAD, DDB.VAR, TIR.NO.PER, VAN.NO.PER, RENDTO, RENDTO.DESC, RENDTO.VENCIM -->

Estas son las funciones financieras que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y qué es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y «...» significa que puede dar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña ayuda debajo de la celda con sus argumentos; si hace clic en el nombre de la función en la ayuda, se abre su entrada aquí.

## INT.ACUMULADO

`INT.ACUMULADO(emisión, primer_interés, liquidación, tasa, valor_nominal, frecuencia, [base], [método_cálculo])`

Devuelve los intereses acumulados de un valor que paga intereses periódicamente.

- `emisión`: La fecha en que se emitió el valor.
- `primer_interés`: La fecha en que se paga el primer interés.
- `liquidación`: La fecha en que compra el valor.
- `tasa`: La tasa de interés de cada período.
- `valor_nominal` (opcional): El valor nominal del valor (1.000 si se omite).
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.
- `método_cálculo` (opcional): VERDADERO u omitido para contar los intereses desde la fecha de emisión; FALSO para contarlos desde la fecha del primer interés cuando la liquidación es posterior.

## INT.ACUM.V

`INT.ACUM.V(emisión, liquidación, tasa, [valor_nominal], [base])`

Devuelve los intereses acumulados de un valor que paga los intereses al vencimiento.

- `emisión`: La fecha en que se emitió el valor.
- `liquidación`: La fecha en que compra el valor.
- `tasa`: La tasa de interés de cada período.
- `valor_nominal` (opcional): El valor nominal del valor (1.000 si se omite).
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## AMORDEGRC

`AMORDEGRC(costo, fecha_compra, primer_período, valor_residual, período, tasa, [base])`

Devuelve la amortización de un período con el método francés de saldo decreciente.

- `costo`: Lo que costó el activo cuando se compró.
- `fecha_compra`: La fecha en que se compró el activo.
- `primer_período`: La fecha en que termina el primer período.
- `valor_residual`: Lo que vale el activo al final.
- `período`: El período que se calcula, contando el primero como 1.
- `tasa`: La tasa de interés de cada período.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## AMORLINC

`AMORLINC(costo, fecha_compra, primer_período, valor_residual, período, tasa, [base])`

Devuelve la amortización de un período con el método francés de línea recta.

- `costo`: Lo que costó el activo cuando se compró.
- `fecha_compra`: La fecha en que se compró el activo.
- `primer_período`: La fecha en que termina el primer período.
- `valor_residual`: Lo que vale el activo al final.
- `período`: El período que se calcula, contando el primero como 1.
- `tasa`: La tasa de interés de cada período.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## CUPON.DIAS.L1

`CUPON.DIAS.L1(liquidación, vencimiento, frecuencia, [base])`

Devuelve los días desde el inicio del período de cupón hasta la fecha de liquidación.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## CUPON.DIAS

`CUPON.DIAS(liquidación, vencimiento, frecuencia, [base])`

Devuelve los días del período de cupón que contiene la fecha de liquidación.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## CUPON.DIAS.L2

`CUPON.DIAS.L2(liquidación, vencimiento, frecuencia, [base])`

Devuelve los días desde la fecha de liquidación hasta la fecha del siguiente cupón.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## CUPON.FECHA.L2

`CUPON.FECHA.L2(liquidación, vencimiento, frecuencia, [base])`

Devuelve la fecha del siguiente cupón posterior a la fecha de liquidación.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## CUPON.NUM

`CUPON.NUM(liquidación, vencimiento, frecuencia, [base])`

Devuelve el número de cupones que se pagarán entre las fechas de liquidación y vencimiento.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## CUPON.FECHA.L1

`CUPON.FECHA.L1(liquidación, vencimiento, frecuencia, [base])`

Devuelve la fecha del cupón anterior a la fecha de liquidación.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## PAGO.INT.ENTRE

`PAGO.INT.ENTRE(tasa, nper, va, período_inicial, período_final, tipo)`

Devuelve los intereses pagados de un préstamo entre dos períodos.

- `tasa`: La tasa de interés de cada período.
- `nper`: Cuántos pagos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `período_inicial`: El período en que empieza el intervalo.
- `período_final`: El período en que termina el intervalo.
- `tipo`: Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## PAGO.PRINC.ENTRE

`PAGO.PRINC.ENTRE(tasa, nper, va, período_inicial, período_final, tipo)`

Devuelve el capital pagado de un préstamo entre dos períodos.

- `tasa`: La tasa de interés de cada período.
- `nper`: Cuántos pagos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `período_inicial`: El período en que empieza el intervalo.
- `período_final`: El período en que termina el intervalo.
- `tipo`: Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## DB

`DB(costo, valor_residual, vida, período, [mes])`

Devuelve la amortización de un activo para un período, por el método de saldo fijo decreciente.

- `costo`: Lo que costó el activo cuando se compró.
- `valor_residual`: Lo que vale el activo al final.
- `vida`: Cuántos períodos dura el activo.
- `período`: El período que se calcula, contando el primero como 1.
- `mes` (opcional): El mes, de 1 a 12.

## DDB

`DDB(costo, valor_residual, vida, período, [factor])`

Devuelve la amortización de un período por el método de saldo decreciente.

- `costo`: Lo que costó el activo cuando se compró.
- `valor_residual`: Lo que vale el activo al final.
- `vida`: Cuántos períodos dura el activo.
- `período`: El período que se calcula, contando el primero como 1.
- `factor` (opcional): La velocidad a la que decrece el saldo (2 si se omite, que es doble saldo decreciente).

## TASA.DESC

`TASA.DESC(liquidación, vencimiento, precio, reembolso, [base])`

Devuelve la tasa de descuento de un valor.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `precio`: El precio del valor por cada 100 de valor nominal.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## MONEDA.DEC

`MONEDA.DEC(moneda_fraccionaria, fracción)`

Convierte un precio escrito como fracción, como 1,02, en un número decimal.

- `moneda_fraccionaria`: Un precio escrito como fracción, como 1,02 para una unidad y dos dieciseisavos.
- `fracción`: El número por el que se divide una unidad: 16 para dieciseisavos, por ejemplo.

## MONEDA.FRAC

`MONEDA.FRAC(moneda_decimal, fracción)`

Convierte un precio decimal en otro escrito como fracción, como 1,02.

- `moneda_decimal`: Un precio escrito como número decimal, como 1,125.
- `fracción`: El número por el que se divide una unidad: 16 para dieciseisavos, por ejemplo.

## DURACION

`DURACION(liquidación, vencimiento, cupón, rendimiento, frecuencia, [base])`

Devuelve el tiempo medio que tarda en recuperarse el costo de un bono, ponderado por los pagos.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `cupón`: La tasa anual de cupón del bono.
- `rendimiento`: El rendimiento anual del valor.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## INT.EFECTIVO

`INT.EFECTIVO(tasa_nominal, núm_per_año)`

Devuelve la tasa anual a la que equivale una tasa indicada con varios períodos al año.

- `tasa_nominal`: La tasa anual indicada.
- `núm_per_año`: Cuántos períodos hay en un año.

## VF

`VF(tasa, nper, pago, [va], [tipo])`

Devuelve el valor final de unos pagos periódicos.

- `tasa`: La tasa de interés de cada período.
- `nper`: Cuántos pagos hay en total.
- `pago`: El pago que se hace en cada período.
- `va` (opcional): Lo que valen hoy los importes (el préstamo o la inversión completa).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## VF.PLAN

`VF.PLAN(capital, plan)`

Devuelve el valor futuro de un importe tras aplicar una lista de tasas de interés.

- `capital`: El importe inicial.
- `plan`: El rango de celdas con las tasas de interés que se aplican, una tras otra.

## TASA.INT

`TASA.INT(liquidación, vencimiento, inversión, reembolso, [base])`

Devuelve la tasa de interés de un valor totalmente invertido.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `inversión`: El importe invertido en el valor.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## PAGOINT

`PAGOINT(tasa, período, nper, va, [vf], [tipo])`

Devuelve la parte de intereses de un pago.

- `tasa`: La tasa de interés de cada período.
- `período`: El pago que se calcula, contando el primero como 1.
- `nper`: Cuántos pagos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf` (opcional): Lo que queda después del último pago (0 si se omite).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## TIR

`TIR(valores, [estimación])`

Devuelve la tasa interna de retorno de una serie de importes que llegan en intervalos regulares.

- `valores`: El rango de celdas que contiene los importes.
- `estimación` (opcional): Su estimación del resultado, desde la que empieza la búsqueda (0,1 si se omite).

## INT.PAGO.DIR

`INT.PAGO.DIR(tasa, período, nper, va)`

Devuelve los intereses pagados en un período de un préstamo que se devuelve en partes de capital iguales.

- `tasa`: La tasa de interés de cada período.
- `período`: El pago que se calcula, contando el primero como 1.
- `nper`: Cuántos pagos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).

## DURACION.MODIFICADA

`DURACION.MODIFICADA(liquidación, vencimiento, cupón, rendimiento, frecuencia, [base])`

Devuelve la duración de un bono, ajustada a su rendimiento.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `cupón`: La tasa anual de cupón del bono.
- `rendimiento`: El rendimiento anual del valor.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## TIRM

`TIRM(valores, tasa_financiación, tasa_reinversión)`

Devuelve la tasa de retorno de los flujos de caja, con una tasa para pedir prestado y otra para reinvertir.

- `valores`: El rango de celdas que contiene los importes.
- `tasa_financiación`: La tasa que paga por el dinero que pide prestado.
- `tasa_reinversión`: La tasa que gana por el dinero que reinvierte.

## TASA.NOMINAL

`TASA.NOMINAL(tasa_efectiva, núm_per_año)`

Devuelve la tasa indicada a partir de una tasa anual y un número de períodos al año.

- `tasa_efectiva`: La tasa anual.
- `núm_per_año`: Cuántos períodos hay en un año.

## NPER

`NPER(tasa, pago, va, [vf], [tipo])`

Devuelve cuántos pagos hacen falta para saldar un préstamo.

- `tasa`: La tasa de interés de cada período.
- `pago`: El pago que se hace en cada período.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf` (opcional): Lo que queda después del último pago (0 si se omite).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## VAN

`VAN(tasa, valor1, ...)`

Devuelve el valor actual de una serie de importes futuros.

- `tasa`: La tasa de interés de cada período.
- `valor1`: El primer valor, o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## PRECIO.PER.IRREGULAR.1

`PRECIO.PER.IRREGULAR.1(liquidación, vencimiento, emisión, primer_cupón, tasa, rendimiento, reembolso, frecuencia, [base])`

Devuelve el precio por cada 100 de un bono con un primer período irregular.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `emisión`: La fecha en que se emitió el valor.
- `primer_cupón`: La fecha del primer cupón.
- `tasa`: La tasa de interés de cada período.
- `rendimiento`: El rendimiento anual del valor.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## RENDTO.PER.IRREGULAR.1

`RENDTO.PER.IRREGULAR.1(liquidación, vencimiento, emisión, primer_cupón, tasa, precio, reembolso, frecuencia, [base])`

Devuelve el rendimiento de un bono con un primer período irregular.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `emisión`: La fecha en que se emitió el valor.
- `primer_cupón`: La fecha del primer cupón.
- `tasa`: La tasa de interés de cada período.
- `precio`: El precio del valor por cada 100 de valor nominal.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## PRECIO.PER.IRREGULAR.2

`PRECIO.PER.IRREGULAR.2(liquidación, vencimiento, último_interés, tasa, rendimiento, reembolso, frecuencia, [base])`

Devuelve el precio por cada 100 de un bono con un último período irregular.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `último_interés`: La fecha en que se pagó el último interés.
- `tasa`: La tasa de interés de cada período.
- `rendimiento`: El rendimiento anual del valor.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## RENDTO.PER.IRREGULAR.2

`RENDTO.PER.IRREGULAR.2(liquidación, vencimiento, último_interés, tasa, precio, reembolso, frecuencia, [base])`

Devuelve el rendimiento de un bono con un último período irregular.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `último_interés`: La fecha en que se pagó el último interés.
- `tasa`: La tasa de interés de cada período.
- `precio`: El precio del valor por cada 100 de valor nominal.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## NPER.INC

`NPER.INC(tasa, va, vf)`

Devuelve los períodos que tarda una inversión en alcanzar un valor.

- `tasa`: La tasa de interés de cada período.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf`: Lo que queda después del último pago (0 si se omite).

## PAGO

`PAGO(tasa, nper, va, [vf], [tipo])`

Devuelve el pago periódico de un préstamo.

- `tasa`: La tasa de interés de cada período.
- `nper`: Cuántos pagos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf` (opcional): Lo que queda después del último pago (0 si se omite).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## PAGOPRIN

`PAGOPRIN(tasa, período, nper, va, [vf], [tipo])`

Devuelve la parte de un pago que amortiza el propio préstamo.

- `tasa`: La tasa de interés de cada período.
- `período`: El pago que se calcula, contando el primero como 1.
- `nper`: Cuántos pagos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf` (opcional): Lo que queda después del último pago (0 si se omite).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## PRECIO

`PRECIO(liquidación, vencimiento, tasa, rendimiento, reembolso, frecuencia, [base])`

Devuelve el precio por cada 100 de un bono que paga intereses periódicamente.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `tasa`: La tasa de interés de cada período.
- `rendimiento`: El rendimiento anual del valor.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## PRECIO.DESCUENTO

`PRECIO.DESCUENTO(liquidación, vencimiento, descuento, reembolso, [base])`

Devuelve el precio por cada 100 de un valor comprado con descuento.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `descuento`: La tasa de descuento del valor.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## PRECIO.VENCIMIENTO

`PRECIO.VENCIMIENTO(liquidación, vencimiento, emisión, tasa, rendimiento, [base])`

Devuelve el precio por cada 100 de un valor que paga los intereses al vencimiento.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `emisión`: La fecha en que se emitió el valor.
- `tasa`: La tasa de interés de cada período.
- `rendimiento`: El rendimiento anual del valor.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## VA

`VA(tasa, nper, pago, [vf], [tipo])`

Devuelve lo que vale hoy una serie de pagos futuros.

- `tasa`: La tasa de interés de cada período.
- `nper`: Cuántos pagos hay en total.
- `pago`: El pago que se hace en cada período.
- `vf` (opcional): Lo que queda después del último pago (0 si se omite).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.

## TASA

`TASA(nper, pago, va, [vf], [tipo], [estimación])`

Devuelve la tasa de interés de cada período de un préstamo o una inversión.

- `nper`: Cuántos pagos hay en total.
- `pago`: El pago que se hace en cada período.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf` (opcional): Lo que queda después del último pago (0 si se omite).
- `tipo` (opcional): Cuándo se hacen los pagos: 0 u omitido al final de cada período; 1 al principio.
- `estimación` (opcional): Su estimación del resultado, desde la que empieza la búsqueda (0,1 si se omite).

## CANTIDAD.RECIBIDA

`CANTIDAD.RECIBIDA(liquidación, vencimiento, inversión, descuento, [base])`

Devuelve el importe recibido al vencimiento por un valor totalmente invertido.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `inversión`: El importe invertido en el valor.
- `descuento`: La tasa de descuento del valor.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## TASA.INT.EQUIV

`TASA.INT.EQUIV(núm_per, va, vf)`

Devuelve la tasa de interés que hace crecer una inversión hasta un valor.

- `núm_per`: Cuántos períodos hay en total.
- `va`: Lo que valen hoy los importes (el préstamo o la inversión completa).
- `vf`: Lo que queda después del último pago (0 si se omite).

## SLN

`SLN(costo, valor_residual, vida)`

Devuelve la amortización lineal de un año.

- `costo`: Lo que costó el activo cuando se compró.
- `valor_residual`: Lo que vale el activo al final.
- `vida`: Cuántos períodos dura el activo.

## SYD

`SYD(costo, valor_residual, vida, período)`

Devuelve la amortización de un año por el método de la suma de los dígitos de los años.

- `costo`: Lo que costó el activo cuando se compró.
- `valor_residual`: Lo que vale el activo al final.
- `vida`: Cuántos períodos dura el activo.
- `período`: El pago que se calcula, contando el primero como 1.

## LETRA.EQV.BONO

`LETRA.EQV.BONO(liquidación, vencimiento, descuento)`

Devuelve el rendimiento equivalente de bono de una letra del Tesoro.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `descuento`: La tasa de descuento del valor.

## LETRA.PRECIO

`LETRA.PRECIO(liquidación, vencimiento, descuento)`

Devuelve el precio por cada 100 de una letra del Tesoro.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `descuento`: La tasa de descuento del valor.

## LETRA.RENTABILIDAD

`LETRA.RENTABILIDAD(liquidación, vencimiento, precio)`

Devuelve el rendimiento de una letra del Tesoro.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `precio`: El precio del valor por cada 100 de valor nominal.

## DDB.VAR

`DDB.VAR(costo, valor_residual, vida, período_inicial, período_final, [factor], [sin_cambio])`

Devuelve la amortización de cualquier intervalo de períodos por el método de saldo decreciente.

- `costo`: Lo que costó el activo cuando se compró.
- `valor_residual`: Lo que vale el activo al final.
- `vida`: Cuántos períodos dura el activo.
- `período_inicial`: El período en que empieza el intervalo.
- `período_final`: El período en que termina el intervalo.
- `factor` (opcional): La velocidad a la que decrece el saldo (2 si se omite, que es doble saldo decreciente).
- `sin_cambio` (opcional): VERDADERO para seguir usando el método de saldo decreciente aunque la línea recta dé una amortización mayor.

## TIR.NO.PER

`TIR.NO.PER(valores, fechas, [estimación])`

Devuelve la tasa de retorno de unos importes que llegan en fechas que usted elige.

- `valores`: El rango de celdas que contiene los importes.
- `fechas`: Las fechas de los importes, en el mismo orden que los importes.
- `estimación` (opcional): Su estimación del resultado, desde la que empieza la búsqueda (0,1 si se omite).

## VAN.NO.PER

`VAN.NO.PER(tasa, valores, fechas)`

Devuelve el valor actual de unos importes que llegan en fechas que usted elige.

- `tasa`: La tasa de interés de cada período.
- `valores`: El rango de celdas que contiene los importes.
- `fechas`: Las fechas de los importes, en el mismo orden que los importes.

## RENDTO

`RENDTO(liquidación, vencimiento, tasa, precio, reembolso, frecuencia, [base])`

Devuelve el rendimiento de un bono que paga intereses periódicamente.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `tasa`: La tasa de interés de cada período.
- `precio`: El precio del valor por cada 100 de valor nominal.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `frecuencia`: Cuántos cupones se pagan al año: 1, 2 o 4.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## RENDTO.DESC

`RENDTO.DESC(liquidación, vencimiento, precio, reembolso, [base])`

Devuelve el rendimiento anual de un valor comprado con descuento.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `precio`: El precio del valor por cada 100 de valor nominal.
- `reembolso`: El valor del título al vencimiento, por cada 100 de valor nominal.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.

## RENDTO.VENCIM

`RENDTO.VENCIM(liquidación, vencimiento, emisión, tasa, precio, [base])`

Devuelve el rendimiento anual de un valor que paga los intereses al vencimiento.

- `liquidación`: La fecha en que compra el valor.
- `vencimiento`: La fecha en que termina el valor y se reembolsa.
- `emisión`: La fecha en que se emitió el valor.
- `tasa`: La tasa de interés de cada período.
- `precio`: El precio del valor por cada 100 de valor nominal.
- `base` (opcional): Cómo se cuentan los días del año: 0 u omitido para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.
