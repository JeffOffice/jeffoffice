# Fonctions financières

<!-- keywords: fonctions, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT, fonctions financières, calcul financier, amortissement, intérêts, taux, emprunt, valeur actuelle, valeur future -->

Voici les fonctions financières que JeffCrunch peut calculer. Pour chacune, vous trouverez sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires que nécessaire après le dernier. Pour essayer l'une d'elles, consultez [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments ; un clic sur le nom de la fonction dans cette info-bulle ouvre sa fiche ici.

## ACCRINT

`ACCRINT(émission, premier_intérêt, règlement, taux, valeur_nominale, fréquence, [base], [méthode_calcul])`

Renvoie les intérêts courus d'un titre qui verse des intérêts selon un échéancier.

- `émission` : La date d'émission du titre.
- `premier_intérêt` : La date de versement du premier intérêt.
- `règlement` : La date d'achat du titre.
- `taux` : Le taux d'intérêt par période.
- `valeur_nominale` : La valeur nominale du titre (1 000 si omis).
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.
- `méthode_calcul` (facultatif) : VRAI ou omis pour compter les intérêts à partir de la date d'émission ; FAUX pour les compter à partir de la date du premier intérêt lorsque le règlement est postérieur.

## ACCRINTM

`ACCRINTM(émission, règlement, taux, [valeur_nominale], [base])`

Renvoie les intérêts courus d'un titre qui verse des intérêts à l'échéance.

- `émission` : La date d'émission du titre.
- `règlement` : La date d'achat du titre.
- `taux` : Le taux d'intérêt par période.
- `valeur_nominale` (facultatif) : La valeur nominale du titre (1 000 si omis).
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## AMORDEGRC

`AMORDEGRC(coût, date_achat, première_période, valeur_résiduelle, période, taux, [base])`

Renvoie l'amortissement d'une période selon la méthode française de l'amortissement dégressif.

- `coût` : Le coût d'achat du bien.
- `date_achat` : La date d'achat du bien.
- `première_période` : La date de fin de la première période.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `période` : La période à calculer, la première étant comptée 1.
- `taux` : Le taux d'intérêt par période.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## AMORLINC

`AMORLINC(coût, date_achat, première_période, valeur_résiduelle, période, taux, [base])`

Renvoie l'amortissement d'une période selon la méthode française de l'amortissement linéaire.

- `coût` : Le coût d'achat du bien.
- `date_achat` : La date d'achat du bien.
- `première_période` : La date de fin de la première période.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `période` : La période à calculer, la première étant comptée 1.
- `taux` : Le taux d'intérêt par période.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## COUPDAYBS

`COUPDAYBS(règlement, échéance, fréquence, [base])`

Renvoie le nombre de jours entre le début de la période de coupon et la date de règlement.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## COUPDAYS

`COUPDAYS(règlement, échéance, fréquence, [base])`

Renvoie le nombre de jours de la période de coupon qui contient la date de règlement.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## COUPDAYSNC

`COUPDAYSNC(règlement, échéance, fréquence, [base])`

Renvoie le nombre de jours entre la date de règlement et le prochain coupon.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## COUPNCD

`COUPNCD(règlement, échéance, fréquence, [base])`

Renvoie la date du prochain coupon après la date de règlement.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## COUPNUM

`COUPNUM(règlement, échéance, fréquence, [base])`

Renvoie le nombre de coupons à verser entre la date de règlement et la date d'échéance.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## COUPPCD

`COUPPCD(règlement, échéance, fréquence, [base])`

Renvoie la date du coupon précédant la date de règlement.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## CUMIPMT

`CUMIPMT(taux, npm, va, période_début, période_fin, type)`

Renvoie les intérêts versés sur un emprunt entre deux périodes.

- `taux` : Le taux d'intérêt par période.
- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `période_début` : La période à laquelle commence l'intervalle.
- `période_fin` : La période à laquelle se termine l'intervalle.
- `type` : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## CUMPRINC

`CUMPRINC(taux, npm, va, période_début, période_fin, type)`

Renvoie le capital remboursé sur un emprunt entre deux périodes.

- `taux` : Le taux d'intérêt par période.
- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `période_début` : La période à laquelle commence l'intervalle.
- `période_fin` : La période à laquelle se termine l'intervalle.
- `type` : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## DB

`DB(coût, valeur_résiduelle, durée, période, [mois])`

Renvoie l'amortissement d'un bien pour une période, selon la méthode du taux fixe dégressif.

- `coût` : Le coût d'achat du bien.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `durée` : Nombre de périodes pendant lesquelles le bien est amorti.
- `période` : La période à calculer, la première étant comptée 1.
- `mois` (facultatif) : Le mois, de 1 à 12.

## DDB

`DDB(coût, valeur_résiduelle, durée, période, [facteur])`

Renvoie l'amortissement d'un bien pour une période, selon la méthode de l'amortissement dégressif.

- `coût` : Le coût d'achat du bien.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `durée` : Nombre de périodes pendant lesquelles le bien est amorti.
- `période` : La période à calculer, la première étant comptée 1.
- `facteur` (facultatif) : Vitesse de baisse de la valeur (2 si omis, ce qui correspond à un dégressif double).

## DISC

`DISC(règlement, échéance, cours, valeur_remboursement, [base])`

Renvoie le taux d'escompte d'un titre.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `cours` : Le prix du titre pour 100 de valeur nominale.
- `valeur_remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## DOLLARDE

`DOLLARDE(prix_fractionnaire, fraction)`

Convertit un prix écrit sous forme de fraction, comme 1,02, en nombre décimal.

- `prix_fractionnaire` : Un prix écrit sous forme de fraction, comme 1,02 pour un dollar et deux seizièmes.
- `fraction` : Le nombre par lequel diviser un dollar : 16 pour des seizièmes, par exemple.

## DOLLARFR

`DOLLARFR(prix_décimal, fraction)`

Convertit un prix décimal en prix écrit sous forme de fraction, comme 1,02.

- `prix_décimal` : Un prix écrit sous forme de nombre décimal, comme 1,125.
- `fraction` : Le nombre par lequel diviser un dollar : 16 pour des seizièmes, par exemple.

## DURATION

`DURATION(règlement, échéance, coupon, rendement, fréquence, [base])`

Renvoie la durée moyenne nécessaire pour récupérer le coût d'une obligation, pondérée par les versements.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `coupon` : Le taux annuel du coupon de l'obligation.
- `rendement` : Le rendement annuel du titre.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## EFFECT

`EFFECT(taux_nominal, npm_par_an)`

Renvoie le taux annuel effectif correspondant à un taux annoncé avec plusieurs périodes par an.

- `taux_nominal` : Le taux annuel annoncé.
- `npm_par_an` : Nombre de périodes dans une année.

## FV

`FV(taux, npm, pmt, [va], [type])`

Renvoie la valeur future de paiements réguliers en fin de parcours.

- `taux` : Le taux d'intérêt par période.
- `npm` : Nombre total de paiements.
- `pmt` : Le paiement effectué à chaque période.
- `va` (facultatif) : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## FVSCHEDULE

`FVSCHEDULE(principal, échéancier)`

Renvoie la valeur future d'un montant après une liste de taux d'intérêt.

- `principal` : Le montant de départ.
- `échéancier` : La plage de cellules contenant les taux d'intérêt à appliquer, l'un après l'autre.

## INTRATE

`INTRATE(règlement, échéance, investissement, remboursement, [base])`

Renvoie le taux d'intérêt d'un titre entièrement investi.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `investissement` : Le montant placé dans le titre.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## IPMT

`IPMT(taux, pér, npm, va, [vc], [type])`

Renvoie la part d'intérêts d'un remboursement.

- `taux` : Le taux d'intérêt par période.
- `pér` : Le paiement à calculer, le premier étant compté 1.
- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` (facultatif) : Ce qui reste après le dernier paiement (0 si omis).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## IRR

`IRR(valeurs, [estimation])`

Renvoie le taux de rendement d'une série de montants qui arrivent à intervalles réguliers.

- `valeurs` : La plage de cellules contenant les montants.
- `estimation` (facultatif) : Votre estimation du résultat, pour démarrer la recherche à partir de cette valeur (0,1 si omis).

## ISPMT

`ISPMT(taux, pér, npm, va)`

Renvoie les intérêts payés sur une période d'un emprunt remboursé par parts de capital égales.

- `taux` : Le taux d'intérêt par période.
- `pér` : Le paiement à calculer, le premier étant compté 1.
- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).

## MDURATION

`MDURATION(règlement, échéance, coupon, rendement, fréquence, [base])`

Renvoie la duration d'une obligation, ajustée en fonction de son rendement.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `coupon` : Le taux annuel du coupon de l'obligation.
- `rendement` : Le rendement annuel du titre.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## MIRR

`MIRR(valeurs, taux_financement, taux_réinvestissement)`

Renvoie le taux de rendement de flux de trésorerie, avec un taux pour emprunter et un autre pour réinvestir.

- `valeurs` : La plage de cellules contenant les montants.
- `taux_financement` : Le taux que vous payez sur l'argent emprunté.
- `taux_réinvestissement` : Le taux que vous gagnez sur l'argent réinvesti.

## NOMINAL

`NOMINAL(taux_effectif, npm_par_an)`

Renvoie le taux annoncé correspondant à un taux annuel effectif et à un nombre de périodes par an.

- `taux_effectif` : Le taux annuel.
- `npm_par_an` : Nombre de périodes dans une année.

## NPER

`NPER(taux, pmt, va, [vc], [type])`

Renvoie le nombre de paiements nécessaires pour rembourser un emprunt.

- `taux` : Le taux d'intérêt par période.
- `pmt` : Le paiement effectué à chaque période.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` (facultatif) : Ce qui reste après le dernier paiement (0 si omis).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## NPV

`NPV(taux, valeur1, ...)`

Renvoie la valeur actuelle d'une série de montants futurs.

- `taux` : Le taux d'intérêt par période.
- `valeur1` : La première valeur, ou plage de cellules.
- `...` : Autant d'autres que nécessaire, du même type que le précédent.

## ODDFPRICE

`ODDFPRICE(règlement, échéance, émission, premier_coupon, taux, rendement, remboursement, fréquence, [base])`

Renvoie le prix pour 100 d'une obligation dont la première période est irrégulière.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `émission` : La date d'émission du titre.
- `premier_coupon` : La date du premier coupon.
- `taux` : Le taux d'intérêt par période.
- `rendement` : Le rendement annuel du titre.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## ODDFYIELD

`ODDFYIELD(règlement, échéance, émission, premier_coupon, taux, cours, remboursement, fréquence, [base])`

Renvoie le rendement d'une obligation dont la première période est irrégulière.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `émission` : La date d'émission du titre.
- `premier_coupon` : La date du premier coupon.
- `taux` : Le taux d'intérêt par période.
- `cours` : Le prix du titre pour 100 de valeur nominale.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## ODDLPRICE

`ODDLPRICE(règlement, échéance, dernier_intérêt, taux, rendement, remboursement, fréquence, [base])`

Renvoie le prix pour 100 d'une obligation dont la dernière période est irrégulière.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `dernier_intérêt` : La date de versement du dernier intérêt.
- `taux` : Le taux d'intérêt par période.
- `rendement` : Le rendement annuel du titre.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## ODDLYIELD

`ODDLYIELD(règlement, échéance, dernier_intérêt, taux, cours, remboursement, fréquence, [base])`

Renvoie le rendement d'une obligation dont la dernière période est irrégulière.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `dernier_intérêt` : La date de versement du dernier intérêt.
- `taux` : Le taux d'intérêt par période.
- `cours` : Le prix du titre pour 100 de valeur nominale.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## PDURATION

`PDURATION(taux, va, vc)`

Renvoie le nombre de périodes nécessaires à un placement pour atteindre une valeur.

- `taux` : Le taux d'intérêt par période.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` : Ce qui reste après le dernier paiement (0 si omis).

## PMT

`PMT(taux, npm, va, [vc], [type])`

Renvoie le paiement périodique d'un emprunt.

- `taux` : Le taux d'intérêt par période.
- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` (facultatif) : Ce qui reste après le dernier paiement (0 si omis).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## PPMT

`PPMT(taux, pér, npm, va, [vc], [type])`

Renvoie la part d'un remboursement qui rembourse le capital de l'emprunt lui-même.

- `taux` : Le taux d'intérêt par période.
- `pér` : Le paiement à calculer, le premier étant compté 1.
- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` (facultatif) : Ce qui reste après le dernier paiement (0 si omis).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## PRICE

`PRICE(règlement, échéance, taux, rendement, remboursement, fréquence, [base])`

Renvoie le prix pour 100 d'une obligation qui verse des intérêts selon un échéancier.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `taux` : Le taux d'intérêt par période.
- `rendement` : Le rendement annuel du titre.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## PRICEDISC

`PRICEDISC(règlement, échéance, escompte, remboursement, [base])`

Renvoie le prix pour 100 d'un titre acheté avec une décote.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `escompte` : Le taux d'escompte du titre.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## PRICEMAT

`PRICEMAT(règlement, échéance, émission, taux, rendement, [base])`

Renvoie le prix pour 100 d'un titre qui verse des intérêts à l'échéance.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `émission` : La date d'émission du titre.
- `taux` : Le taux d'intérêt par période.
- `rendement` : Le rendement annuel du titre.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## PV

`PV(taux, npm, pmt, [vc], [type])`

Renvoie la valeur actuelle d'une série de paiements futurs.

- `taux` : Le taux d'intérêt par période.
- `npm` : Nombre total de paiements.
- `pmt` : Le paiement effectué à chaque période.
- `vc` (facultatif) : Ce qui reste après le dernier paiement (0 si omis).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.

## RATE

`RATE(npm, pmt, va, [vc], [type], [estimation])`

Renvoie le taux d'intérêt par période d'un emprunt ou d'un placement.

- `npm` : Nombre total de paiements.
- `pmt` : Le paiement effectué à chaque période.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` (facultatif) : Ce qui reste après le dernier paiement (0 si omis).
- `type` (facultatif) : Moment des paiements : 0 ou omis en fin de période, 1 en début de période.
- `estimation` (facultatif) : Votre estimation du résultat, pour démarrer la recherche à partir de cette valeur (0,1 si omis).

## RECEIVED

`RECEIVED(règlement, échéance, investissement, escompte, [base])`

Renvoie le montant perçu à l'échéance pour un titre entièrement investi.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `investissement` : Le montant placé dans le titre.
- `escompte` : Le taux d'escompte du titre.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## RRI

`RRI(npm, va, vc)`

Renvoie le taux d'intérêt qui fait croître un placement jusqu'à une valeur.

- `npm` : Nombre total de paiements.
- `va` : La valeur actuelle (la totalité de l'emprunt ou du placement).
- `vc` : Ce qui reste après le dernier paiement (0 si omis).

## SLN

`SLN(coût, valeur_résiduelle, durée)`

Renvoie l'amortissement linéaire d'une année.

- `coût` : Le coût d'achat du bien.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `durée` : Nombre de périodes pendant lesquelles le bien est amorti.

## SYD

`SYD(coût, valeur_résiduelle, durée, période)`

Renvoie l'amortissement d'une année selon la méthode de la somme des annuités dégressives.

- `coût` : Le coût d'achat du bien.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `durée` : Nombre de périodes pendant lesquelles le bien est amorti.
- `période` : Le paiement à calculer, le premier étant compté 1.

## TBILLEQ

`TBILLEQ(règlement, échéance, escompte)`

Renvoie le rendement équivalent obligataire d'un bon du Trésor.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `escompte` : Le taux d'escompte du titre.

## TBILLPRICE

`TBILLPRICE(règlement, échéance, escompte)`

Renvoie le prix pour 100 d'un bon du Trésor.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `escompte` : Le taux d'escompte du titre.

## TBILLYIELD

`TBILLYIELD(règlement, échéance, cours)`

Renvoie le rendement d'un bon du Trésor.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `cours` : Le prix du titre pour 100 de valeur nominale.

## VDB

`VDB(coût, valeur_résiduelle, durée, période_début, période_fin, [facteur], [sans_bascule])`

Renvoie l'amortissement sur un intervalle quelconque de périodes, selon la méthode du dégressif à taux fixe.

- `coût` : Le coût d'achat du bien.
- `valeur_résiduelle` : La valeur du bien en fin de parcours.
- `durée` : Nombre de périodes pendant lesquelles le bien est amorti.
- `période_début` : La période à laquelle commence l'intervalle.
- `période_fin` : La période à laquelle se termine l'intervalle.
- `facteur` (facultatif) : Vitesse de baisse de la valeur (2 si omis, ce qui correspond à un dégressif double).
- `sans_bascule` (facultatif) : VRAI pour continuer le dégressif à taux fixe lorsque l'amortissement linéaire donnerait un montant plus élevé.

## XIRR

`XIRR(valeurs, dates, [estimation])`

Renvoie le taux de rendement de montants qui arrivent à des dates que vous choisissez.

- `valeurs` : La plage de cellules contenant les montants.
- `dates` : Les dates des montants, dans le même ordre que les montants.
- `estimation` (facultatif) : Votre estimation du résultat, pour démarrer la recherche à partir de cette valeur (0,1 si omis).

## XNPV

`XNPV(taux, valeurs, dates)`

Renvoie la valeur actuelle de montants qui arrivent à des dates que vous choisissez.

- `taux` : Le taux d'intérêt par période.
- `valeurs` : La plage de cellules contenant les montants.
- `dates` : Les dates des montants, dans le même ordre que les montants.

## YIELD

`YIELD(règlement, échéance, taux, cours, remboursement, fréquence, [base])`

Renvoie le rendement d'une obligation qui verse des intérêts selon un échéancier.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `taux` : Le taux d'intérêt par période.
- `cours` : Le prix du titre pour 100 de valeur nominale.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `fréquence` : Nombre de coupons versés par an : 1, 2 ou 4.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## YIELDDISC

`YIELDDISC(règlement, échéance, cours, remboursement, [base])`

Renvoie le rendement annuel d'un titre acheté avec une décote.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `cours` : Le prix du titre pour 100 de valeur nominale.
- `remboursement` : La valeur du titre à l'échéance, pour 100 de valeur nominale.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.

## YIELDMAT

`YIELDMAT(règlement, échéance, émission, taux, cours, [base])`

Renvoie le rendement annuel d'un titre qui verse des intérêts à l'échéance.

- `règlement` : La date d'achat du titre.
- `échéance` : La date de fin du titre, à laquelle il est remboursé.
- `émission` : La date d'émission du titre.
- `taux` : Le taux d'intérêt par période.
- `cours` : Le prix du titre pour 100 de valeur nominale.
- `base` (facultatif) : Mode de décompte des jours de l'année : 0 ou omis pour 30/360, 1 jours réels, 2 jours réels sur une année de 360 jours, 3 jours réels sur une année de 365 jours, 4 30/360 européen.
