# Fungsi statistik
<!-- keywords: fungsi, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Berikut adalah fungsi statistik yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa yang dilakukannya, dan arti setiap argumennya. Argumen yang diapit kurung siku bersifat opsional, dan "..." berarti Anda dapat menambahkan argumen terakhir sebanyak yang diperlukan. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di sel, sebuah tips kecil di bawah sel menampilkan argumennya, dan klik pada nama fungsi di tips tersebut akan membuka entri fungsinya di sini.

## AVEDEV

`AVEDEV(number1, ...)`

Memberikan rata-rata jarak angka-angka dari rata-ratanya.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## AVERAGE

`AVERAGE(number1, ...)`

Memberikan rata-rata dari angka-angka.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## AVERAGEA

`AVERAGEA(value1, ...)`

Memberikan rata-rata dari nilai, dengan menganggap teks sebagai nol dan benar sebagai satu.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Merata-ratakan sel-sel yang memenuhi suatu kondisi.

- `range`: Blok sel yang akan diuji.
- `criteria`: Yang dicari: angka, teks, atau uji seperti ">5".
- `average_range` (opsional): Sel yang akan dirata-ratakan (sel yang diuji jika dihilangkan).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Merata-ratakan sel-sel yang memenuhi beberapa kondisi.

- `average_range`: Sel yang akan dirata-ratakan (sel yang diuji jika dihilangkan).
- `criteria_range1`: Blok sel pertama yang akan diuji.
- `criteria1`: Yang dicari dalam blok sel pertama: angka, teks, atau uji seperti ">5".
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Memberikan nilai atau proporsi kumulatif kurva beta pada x.

- `x`: Nilai x yang akan diproses.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.
- `A` (opsional): Batas bawah x (0 jika dihilangkan).
- `B` (opsional): Batas atas x (1 jika dihilangkan).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Memberikan nilai x untuk suatu proporsi kurva beta.

- `probability`: Peluang, dari 0 sampai 1.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `A` (opsional): Batas bawah x (0 jika dihilangkan).
- `B` (opsional): Batas atas x (1 jika dihilangkan).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Memberikan proporsi kumulatif kurva beta pada x.

- `x`: Nilai x yang akan diproses.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `A` (opsional): Batas bawah x (0 jika dihilangkan).
- `B` (opsional): Batas atas x (1 jika dihilangkan).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Memberikan nilai x untuk suatu proporsi kurva beta.

- `probability`: Peluang, dari 0 sampai 1.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `A` (opsional): Batas bawah x (0 jika dihilangkan).
- `B` (opsional): Batas atas x (1 jika dihilangkan).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Memberikan peluang sejumlah keberhasilan dalam sejumlah percobaan.

- `number_s`: Jumlah keberhasilan.
- `trials`: Jumlah percobaan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Memberikan peluang bahwa jumlah keberhasilan berada dalam suatu rentang.

- `trials`: Jumlah percobaan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.
- `number_s`: Jumlah keberhasilan.
- `number_s2` (opsional): Jumlah keberhasilan terbanyak yang dihitung (sama dengan yang terkecil jika dihilangkan).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Memberikan jumlah keberhasilan terkecil yang jumlah kumulatif peluangnya mencapai suatu nilai.

- `trials`: Jumlah percobaan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Memberikan peluang sejumlah keberhasilan dalam sejumlah percobaan.

- `number_s`: Jumlah keberhasilan.
- `trials`: Jumlah percobaan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Memberikan proporsi kurva chi-kuadrat di atas x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## CHIINV

`CHIINV(probability, deg_freedom)`

Memberikan nilai x untuk suatu proporsi kurva chi-kuadrat, dihitung dari kanan.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva chi-kuadrat pada x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Memberikan proporsi kurva chi-kuadrat di atas x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Memberikan nilai x untuk suatu proporsi kurva chi-kuadrat, dihitung dari kiri.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Memberikan nilai x untuk suatu proporsi kurva chi-kuadrat, dihitung dari kanan.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Memberikan peluang bahwa jumlah yang diperoleh berbeda dari jumlah yang diharapkan hanya karena kebetulan.

- `actual_range`: Blok sel dengan jumlah yang Anda peroleh.
- `expected_range`: Blok sel dengan jumlah yang Anda harapkan, dengan ukuran yang sama dengan jumlah yang Anda peroleh.

## CHITEST

`CHITEST(actual_range, expected_range)`

Memberikan peluang bahwa jumlah yang diperoleh berbeda dari jumlah yang diharapkan hanya karena kebetulan.

- `actual_range`: Blok sel dengan jumlah yang Anda peroleh.
- `expected_range`: Blok sel dengan jumlah yang Anda harapkan, dengan ukuran yang sama dengan jumlah yang Anda peroleh.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Memberikan batas kesalahan interval kepercayaan untuk rata-rata, dengan memakai kurva normal.

- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `standard_dev`: Simpangan baku distribusi.
- `size`: Ukuran sampel, bilangan bulat 1 atau lebih.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Memberikan batas kesalahan interval kepercayaan untuk rata-rata, dengan memakai kurva normal.

- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `standard_dev`: Simpangan baku distribusi.
- `size`: Ukuran sampel, bilangan bulat 1 atau lebih.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Memberikan batas kesalahan interval kepercayaan untuk rata-rata, dengan memakai kurva t Student.

- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `standard_dev`: Simpangan baku distribusi.
- `size`: Ukuran sampel, bilangan bulat 1 atau lebih.

## CORREL

`CORREL(array1, array2)`

Memberikan seberapa erat dua kumpulan angka bergerak bersama, dari -1 sampai 1.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## COUNT

`COUNT(value1, ...)`

Menghitung angka dalam daftar atau blok sel.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## COUNTA

`COUNTA(value1, ...)`

Menghitung sel yang tidak kosong.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## COUNTBLANK

`COUNTBLANK(range)`

Menghitung sel kosong dalam sebuah blok.

- `range`: Blok sel yang akan diuji.

## COUNTIF

`COUNTIF(range, criteria)`

Menghitung sel yang memenuhi suatu kondisi.

- `range`: Blok sel yang akan diuji.
- `criteria`: Yang dicari: angka, teks, atau uji seperti ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Menghitung baris yang memenuhi beberapa kondisi sekaligus.

- `criteria_range1`: Blok sel pertama yang akan diuji.
- `criteria1`: Yang dicari dalam blok sel pertama: angka, teks, atau uji seperti ">5".
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## COVAR

`COVAR(array1, array2)`

Memberikan seberapa besar dua kumpulan angka bervariasi bersama.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Memberikan seberapa besar dua kumpulan angka bervariasi bersama, untuk populasi.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Memberikan seberapa besar dua kumpulan angka bervariasi bersama, untuk sampel.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Memberikan jumlah keberhasilan terkecil yang jumlah kumulatif peluangnya mencapai suatu nilai.

- `trials`: Jumlah percobaan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.

## DEVSQ

`DEVSQ(number1, ...)`

Memberikan jumlah kuadrat jarak angka-angka dari rata-ratanya.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva eksponensial pada x.

- `x`: Nilai x yang akan diproses.
- `lambda`: Laju kurva, lebih dari 0.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva eksponensial pada x.

- `x`: Nilai x yang akan diproses.
- `lambda`: Laju kurva, lebih dari 0.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva F pada x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom1`: Derajat kebebasan pembilang, bilangan bulat 1 atau lebih.
- `deg_freedom2`: Derajat kebebasan penyebut, bilangan bulat 1 atau lebih.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Memberikan proporsi kurva F di atas x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom1`: Derajat kebebasan pembilang, bilangan bulat 1 atau lebih.
- `deg_freedom2`: Derajat kebebasan penyebut, bilangan bulat 1 atau lebih.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Memberikan nilai x untuk suatu proporsi kurva F, dihitung dari kiri.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom1`: Derajat kebebasan pembilang, bilangan bulat 1 atau lebih.
- `deg_freedom2`: Derajat kebebasan penyebut, bilangan bulat 1 atau lebih.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Memberikan nilai x untuk suatu proporsi kurva F, dihitung dari kanan.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom1`: Derajat kebebasan pembilang, bilangan bulat 1 atau lebih.
- `deg_freedom2`: Derajat kebebasan penyebut, bilangan bulat 1 atau lebih.

## F.TEST

`F.TEST(array1, array2)`

Memberikan peluang bahwa dua kumpulan angka memiliki varians yang sama.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Memberikan proporsi kurva F di atas x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom1`: Derajat kebebasan pembilang, bilangan bulat 1 atau lebih.
- `deg_freedom2`: Derajat kebebasan penyebut, bilangan bulat 1 atau lebih.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Memberikan nilai x untuk suatu proporsi kurva F, dihitung dari kanan.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom1`: Derajat kebebasan pembilang, bilangan bulat 1 atau lebih.
- `deg_freedom2`: Derajat kebebasan penyebut, bilangan bulat 1 atau lebih.

## FISHER

`FISHER(x)`

Memberikan transformasi Fisher dari angka antara -1 dan 1.

- `x`: Nilai x yang akan diproses.

## FISHERINV

`FISHERINV(y)`

Memberikan invers dari transformasi Fisher.

- `y`: Angka yang akan dicari invers transformasi Fisher-nya.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Memprediksi nilai pada garis lurus yang paling sesuai.

- `x`: Nilai x yang akan diprediksi nilai y-nya.
- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's`: Blok sel yang berisi nilai x yang sudah Anda miliki.

## FORECAST.ETS

`FORECAST.ETS(target_date, values, timeline, [seasonality], [data_completion], [aggregation])`

Memprediksi nilai masa depan dari deret yang memiliki tren dan musim, dengan pemulusan eksponensial.

- `target_date`: Waktu yang akan diprediksi nilainya.
- `values`: Nilai yang sudah Anda miliki.
- `timeline`: Tanggal atau angka dari nilai-nilai tersebut, dengan jarak yang sama.
- `seasonality`: 1 untuk mencari musim dari nilai, 0 jika tidak ada, atau panjang musim.
- `data_completion`: 1 untuk mengisi titik yang hilang dengan garis di antara tetangganya, 0 untuk mengisinya dengan nol.
- `aggregation`: Cara nilai pada waktu yang sama digabungkan: 1 rata-rata, 2 hitung, 3 counta, 4 maks, 5 median, 6 min, 7 jumlah.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(target_date, values, timeline, [confidence_level], [seasonality], [data_completion], [aggregation])`

Memberikan seberapa jauh di atas dan di bawah prakiraan pemulusan nilai tersebut kemungkinan berada.

- `confidence_level`: Angka di atas 0 dan di bawah 1, 0,95 jika dihilangkan.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(values, timeline, [data_completion], [aggregation])`

Memberikan panjang musim yang berulang dalam sebuah deret, atau 1 jika tidak ada.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(values, timeline, statistic_type, [seasonality], [data_completion], [aggregation])`

Memberikan salah satu konstanta pemulusan atau ukuran kesesuaian dari prakiraan pemulusan.

- `statistic_type`: 1 Alfa, 2 Beta, 3 Gamma, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 langkah dari garis waktu.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Memprediksi nilai pada garis lurus yang paling sesuai.

- `x`: Nilai x yang akan diproses.
- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's`: Blok sel yang berisi nilai x yang sudah Anda miliki.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Menghitung berapa banyak nilai yang berada di setiap rentang, dan memberikan hitungannya sebagai kolom.

- `data_array`: Blok sel dengan nilai yang akan dihitung.
- `bins_array`: Blok sel dengan batas-batas rentang tempat menghitung.

## FTEST

`FTEST(array1, array2)`

Memberikan peluang bahwa dua kumpulan angka memiliki varians yang sama.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## GAMMA

`GAMMA(x)`

Memberikan fungsi gamma dari suatu angka.

- `x`: Nilai x yang akan diproses.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva gamma pada x.

- `x`: Nilai x yang akan diproses.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Memberikan nilai x untuk suatu proporsi kurva gamma.

- `probability`: Peluang, dari 0 sampai 1.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva gamma pada x.

- `x`: Nilai x yang akan diproses.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Memberikan nilai x untuk suatu proporsi kurva gamma.

- `probability`: Peluang, dari 0 sampai 1.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).

## GAMMALN

`GAMMALN(x)`

Memberikan logaritma natural dari fungsi gamma.

- `x`: Nilai x yang akan diproses.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Memberikan logaritma natural dari fungsi gamma.

- `x`: Nilai x yang akan diproses.

## GAUSS

`GAUSS(z)`

Memberikan proporsi kurva normal baku antara rata-rata dan z.

- `z`: Nilai z.

## GEOMEAN

`GEOMEAN(number1, ...)`

Memberikan rata-rata geometrik dari angka-angka positif.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Memberikan nilai pada kurva eksponensial yang paling sesuai untuk nilai x baru.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's` (opsional): Blok sel yang berisi nilai x yang sudah Anda miliki.
- `new_x's` (opsional): Nilai x yang akan dicari nilai y-nya (nilai x yang sudah ada jika dihilangkan).
- `const` (opsional): Benar atau dihilangkan untuk mencocokkan garis yang tidak harus melalui nol; salah untuk membuatnya melalui nol.

## HARMEAN

`HARMEAN(number1, ...)`

Memberikan rata-rata harmonik dari angka-angka positif.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Memberikan peluang memperoleh sejumlah keberhasilan saat mengambil item tanpa dikembalikan.

- `sample_s`: Jumlah keberhasilan dalam sampel.
- `number_sample`: Jumlah item yang diambil.
- `population_s`: Jumlah keberhasilan dalam seluruh populasi.
- `number_pop`: Jumlah item dalam seluruh populasi.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Memberikan peluang memperoleh sejumlah keberhasilan saat mengambil item tanpa dikembalikan.

- `sample_s`: Jumlah keberhasilan dalam sampel.
- `number_sample`: Jumlah item yang diambil.
- `population_s`: Jumlah keberhasilan dalam seluruh populasi.
- `number_pop`: Jumlah item dalam seluruh populasi.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Memberikan titik tempat garis lurus yang paling sesuai memotong sumbu vertikal.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's`: Blok sel yang berisi nilai x yang sudah Anda miliki.

## KURT

`KURT(number1, ...)`

Memberikan seberapa runcing atau datar kumpulan angka dibandingkan dengan kurva normal.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## LARGE

`LARGE(array, k)`

Memberikan angka terbesar ke-k dari suatu kumpulan.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `k`: Urutan yang akan diberikan, dihitung dari yang terbesar: 1 adalah yang terbesar.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Mencocokkan garis lurus atau bidang dengan data menggunakan kuadrat terkecil, dan memberikan angkanya sebagai baris.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's` (opsional): Blok sel yang berisi nilai x yang sudah Anda miliki.
- `const` (opsional): Benar atau dihilangkan untuk mencocokkan garis yang tidak harus melalui nol; salah untuk membuatnya melalui nol.
- `stats` (opsional): Benar untuk memberikan juga angka tambahan tentang kecocokan; salah atau dihilangkan untuk memberikan garisnya saja.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Mencocokkan kurva eksponensial dengan data, dan memberikan angkanya sebagai baris.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's` (opsional): Blok sel yang berisi nilai x yang sudah Anda miliki.
- `const` (opsional): Benar atau dihilangkan untuk mencocokkan garis yang tidak harus melalui nol; salah untuk membuatnya melalui nol.
- `stats` (opsional): Benar untuk memberikan juga angka tambahan tentang kecocokan; salah atau dihilangkan untuk memberikan garisnya saja.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Memberikan nilai x untuk suatu proporsi kurva log-normal.

- `probability`: Peluang, dari 0 sampai 1.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva log-normal pada x.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Memberikan nilai x untuk suatu proporsi kurva log-normal.

- `probability`: Peluang, dari 0 sampai 1.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Memberikan proporsi kumulatif kurva log-normal pada x.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.

## MAXA

`MAXA(value1, ...)`

Memberikan nilai terbesar, dengan menganggap teks sebagai nol dan benar sebagai satu.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Memberikan angka terbesar di antara sel-sel yang memenuhi beberapa kondisi.

- `max_range`: Sel tempat mengambil nilai terbesar.
- `criteria_range1`: Blok sel pertama yang akan diuji.
- `criteria1`: Yang dicari dalam blok sel pertama: angka, teks, atau uji seperti ">5".
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MEDIAN

`MEDIAN(number1, ...)`

Memberikan angka tengah dari suatu kumpulan.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MINA

`MINA(value1, ...)`

Memberikan nilai terkecil, dengan menganggap teks sebagai nol dan benar sebagai satu.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Memberikan angka terkecil di antara sel-sel yang memenuhi beberapa kondisi.

- `min_range`: Sel tempat mengambil nilai terkecil.
- `criteria_range1`: Blok sel pertama yang akan diuji.
- `criteria1`: Yang dicari dalam blok sel pertama: angka, teks, atau uji seperti ">5".
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MODE

`MODE(number1, ...)`

Memberikan angka yang paling sering muncul.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MODE.MULT

`MODE.MULT(number1, ...)`

Memberikan sebuah kolom berisi angka-angka yang paling sering muncul dalam suatu kumpulan.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Memberikan angka yang paling sering muncul dalam suatu kumpulan.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Memberikan peluang sejumlah kegagalan sebelum sejumlah keberhasilan.

- `number_f`: Jumlah kegagalan.
- `number_s`: Jumlah keberhasilan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Memberikan peluang sejumlah kegagalan sebelum sejumlah keberhasilan.

- `number_f`: Jumlah kegagalan.
- `number_s`: Jumlah keberhasilan.
- `probability_s`: Peluang keberhasilan pada setiap percobaan, dari 0 sampai 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva normal pada x.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Memberikan nilai x untuk suatu proporsi kurva normal.

- `probability`: Peluang, dari 0 sampai 1.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva normal baku pada z.

- `z`: Nilai z.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## NORM.S.INV

`NORM.S.INV(probability)`

Memberikan nilai z untuk suatu proporsi kurva normal baku.

- `probability`: Peluang, dari 0 sampai 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva normal pada x.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Memberikan nilai x untuk suatu proporsi kurva normal.

- `probability`: Peluang, dari 0 sampai 1.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.

## NORMSDIST

`NORMSDIST(z)`

Memberikan proporsi kumulatif kurva normal baku sampai z.

- `z`: Nilai z.

## NORMSINV

`NORMSINV(probability)`

Memberikan nilai z untuk suatu proporsi kurva normal baku.

- `probability`: Peluang, dari 0 sampai 1.

## PEARSON

`PEARSON(array1, array2)`

Memberikan seberapa erat dua kumpulan angka bergerak bersama, dari -1 sampai 1.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.

## PERCENTILE

`PERCENTILE(array, k)`

Memberikan nilai yang berada di bawah proporsi k dari angka-angka.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `k`: Proporsi nilai, dari 0 sampai 1, untuk memberikan nilai tersebut.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Memberikan nilai yang berada di bawah proporsi k dari angka-angka, tanpa menyertakan 0 dan 1.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `k`: Urutan dalam deret, dihitung dari 1.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Memberikan nilai yang berada di bawah proporsi k dari angka-angka, dari 0 sampai 1.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `k`: Urutan dalam deret, dihitung dari 1.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Memberikan peringkat suatu nilai dalam kumpulan angka sebagai proporsi dari 0 sampai 1.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `x`: Nilai x yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya akan dijadikan pembulatan.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Memberikan peringkat suatu nilai dalam kumpulan angka sebagai proporsi, tanpa menyertakan 0 dan 1.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `x`: Nilai x yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya akan dijadikan pembulatan.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Memberikan peringkat suatu nilai dalam kumpulan angka sebagai proporsi dari 0 sampai 1.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `x`: Nilai x yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya akan dijadikan pembulatan.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Memberikan jumlah pilihan berurutan saat sebuah item boleh dipilih lebih dari sekali.

- `number`: Angka yang akan diproses.
- `number_chosen`: Berapa item yang akan dipilih di setiap kelompok.

## PHI

`PHI(x)`

Memberikan tinggi kurva normal baku pada x.

- `x`: Nilai x yang akan diproses.

## POISSON

`POISSON(x, mean, cumulative)`

Memberikan peluang sejumlah kejadian, dari berapa banyak yang diharapkan.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Memberikan peluang sejumlah kejadian, dari berapa banyak yang diharapkan.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Memberikan peluang bahwa nilai berada di antara dua batas.

- `x_range`: Blok sel dengan nilai x.
- `prob_range`: Peluang untuk setiap nilai x, masing-masing dari 0 sampai 1, dengan jumlah total 1.
- `lower_limit`: Batas bawah rentang.
- `upper_limit` (opsional): Batas atas rentang (hanya batas bawah jika dihilangkan).

## QUARTILE

`QUARTILE(array, quart)`

Memberikan titik kuartil dari suatu kumpulan angka.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `quart`: Kuartil yang mana: 0 terkecil, 1 kuartil pertama, 2 median, 3 kuartil ketiga, 4 terbesar.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Memberikan titik kuartil dari suatu kumpulan angka, tanpa menyertakan ujung-ujungnya.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `quart`: Kuartil yang mana: 0 terkecil, 1 kuartil pertama, 2 median, 3 kuartil ketiga, 4 terbesar.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Memberikan titik kuartil dari suatu kumpulan angka, dari yang terendah sampai yang tertinggi.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `quart`: Kuartil yang mana: 0 terkecil, 1 kuartil pertama, 2 median, 3 kuartil ketiga, 4 terbesar.

## RANK

`RANK(number, ref, [order])`

Memberikan urutan suatu angka dalam daftar saat daftar tersebut diurutkan.

- `number`: Angka yang akan diproses.
- `ref`: Sel atau blok sel.
- `order` (opsional): 0 atau dihilangkan menempatkan yang terbesar di awal; angka lain menempatkan yang terkecil di awal.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Memberikan peringkat suatu angka dalam daftar, dan peringkat rata-rata jika ada yang sama.

- `number`: Angka yang akan diproses.
- `ref`: Sel atau blok sel.
- `order` (opsional): 0 atau dihilangkan menempatkan yang terbesar di awal; angka lain menempatkan yang terkecil di awal.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Memberikan peringkat suatu angka dalam daftar, dan peringkat tertinggi jika ada yang sama.

- `number`: Angka yang akan diproses.
- `ref`: Sel atau blok sel.
- `order` (opsional): 0 atau dihilangkan menempatkan yang terbesar di awal; angka lain menempatkan yang terkecil di awal.

## RSQ

`RSQ(known_y's, known_x's)`

Memberikan seberapa baik garis lurus cocok dengan dua kumpulan angka.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's`: Blok sel yang berisi nilai x yang sudah Anda miliki.

## SKEW

`SKEW(number1, ...)`

Memberikan seberapa miring kumpulan angka di sekitar rata-ratanya, untuk sampel.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## SKEW.P

`SKEW.P(number1, ...)`

Memberikan seberapa miring kumpulan angka di sekitar rata-ratanya, untuk seluruh populasi.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## SLOPE

`SLOPE(known_y's, known_x's)`

Memberikan kemiringan garis lurus yang paling sesuai dengan dua kumpulan angka.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's`: Blok sel yang berisi nilai x yang sudah Anda miliki.

## SMALL

`SMALL(array, k)`

Memberikan angka terkecil ke-k dari suatu kumpulan.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `k`: Urutan yang akan diberikan, dihitung dari yang terkecil: 1 adalah yang terkecil.

## STANDARDIZE

`STANDARDIZE(x, mean, standard_dev)`

Memberikan berapa simpangan baku nilai tersebut dari rata-rata.

- `x`: Nilai x yang akan diproses.
- `mean`: Rata-rata distribusi.
- `standard_dev`: Simpangan baku distribusi.

## STDEV

`STDEV(number1, ...)`

Memperkirakan sebaran seluruh kelompok dari sebuah sampel.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## STDEV.P

`STDEV.P(number1, ...)`

Memberikan sebaran seluruh kelompok.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## STDEV.S

`STDEV.S(number1, ...)`

Memperkirakan sebaran seluruh kelompok dari sebuah sampel.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## STDEVA

`STDEVA(value1, ...)`

Memberikan simpangan baku sebuah sampel, dengan menganggap teks sebagai 0 dan benar sebagai 1.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## STDEVP

`STDEVP(number1, ...)`

Memberikan sebaran seluruh kelompok.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## STDEVPA

`STDEVPA(value1, ...)`

Memberikan simpangan baku sebuah populasi, dengan menganggap teks sebagai 0 dan benar sebagai 1.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## STEYX

`STEYX(known_y's, known_x's)`

Memberikan galat baku dari nilai y yang diprediksi pada garis yang dicocokkan.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's`: Blok sel yang berisi nilai x yang sudah Anda miliki.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Menjumlahkan sel-sel yang memenuhi suatu kondisi.

- `range`: Blok sel yang akan diuji.
- `criteria`: Yang dicari: angka, teks, atau uji seperti ">5".
- `sum_range` (opsional): Sel yang akan dijumlahkan (sel yang diuji jika dihilangkan).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Menjumlahkan sel-sel yang memenuhi beberapa kondisi.

- `sum_range`: Sel yang akan dijumlahkan (sel yang diuji jika dihilangkan).
- `criteria_range1`: Blok sel pertama yang akan diuji.
- `criteria1`: Yang dicari dalam blok sel pertama: angka, teks, atau uji seperti ">5".
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva t Student pada x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Memberikan proporsi kurva t Student di luar -x dan x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Memberikan proporsi kurva t Student di atas x.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## T.INV

`T.INV(probability, deg_freedom)`

Memberikan nilai x untuk suatu proporsi kurva t Student, dihitung dari kiri.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Memberikan nilai x untuk suatu proporsi kurva t Student, dibagi di kedua ekor.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Memberikan peluang bahwa rata-rata dua kumpulan angka sama.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.
- `tails`: 1 untuk satu ekor kurva, 2 untuk kedua ekor.
- `type`: Kapan pembayaran dilakukan: 0 atau dihilangkan di akhir setiap periode, 1 di awal.

## TDIST

`TDIST(x, deg_freedom, tails)`

Memberikan proporsi kurva t Student di satu ekor atau di kedua ekor.

- `x`: Nilai x yang akan diproses.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.
- `tails`: 1 untuk satu ekor kurva, 2 untuk kedua ekor.

## TINV

`TINV(probability, deg_freedom)`

Memberikan nilai x untuk suatu proporsi kurva t Student, dibagi di kedua ekor.

- `probability`: Peluang, dari 0 sampai 1.
- `deg_freedom`: Derajat kebebasan, bilangan bulat 1 atau lebih.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Memberikan nilai pada garis lurus yang paling sesuai untuk nilai x baru.

- `known_y's`: Blok sel yang berisi nilai y yang sudah Anda miliki.
- `known_x's` (opsional): Blok sel yang berisi nilai x yang sudah Anda miliki.
- `new_x's` (opsional): Nilai x yang akan dicari nilai y-nya (nilai x yang sudah ada jika dihilangkan).
- `const` (opsional): Benar atau dihilangkan untuk mencocokkan garis yang tidak harus melalui nol; salah untuk membuatnya melalui nol.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Memberikan rata-rata dari sekumpulan angka setelah memotong proporsi nilai tertinggi dan terendah.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `percent`: Proporsi nilai, dari 0 hingga kurang dari 1, yang dihilangkan di kedua ujung.

## TTEST

`TTEST(array1, array2, tails, type)`

Memberikan peluang bahwa rata-rata dua kumpulan angka sama.

- `array1`: Blok sel pertama atau daftar nilai.
- `array2`: Blok sel kedua atau daftar nilai, dengan ukuran yang sama dengan yang pertama.
- `tails`: 1 untuk satu ekor kurva, 2 untuk kedua ekor.
- `type`: Kapan pembayaran dilakukan: 0 atau dihilangkan di akhir setiap periode, 1 di awal.

## VAR

`VAR(number1, ...)`

Memperkirakan varians seluruh kelompok dari sebuah sampel.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## VAR.P

`VAR.P(number1, ...)`

Memberikan varians seluruh kelompok.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## VAR.S

`VAR.S(number1, ...)`

Memperkirakan varians seluruh kelompok dari sebuah sampel.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## VARA

`VARA(value1, ...)`

Memberikan varians sebuah sampel, dengan menganggap teks sebagai 0 dan benar sebagai 1.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## VARP

`VARP(number1, ...)`

Memberikan varians seluruh kelompok.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## VARPA

`VARPA(value1, ...)`

Memberikan varians sebuah populasi, dengan menganggap teks sebagai 0 dan benar sebagai 1.

- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva Weibull pada x.

- `x`: Nilai x yang akan diproses.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Memberikan nilai atau proporsi kumulatif kurva Weibull pada x.

- `x`: Nilai x yang akan diproses.
- `alpha`: Peluang salah yang Anda terima, misalnya 0,05 untuk tingkat kepercayaan 95 persen.
- `beta`: Angka yang membentuk kurva (skala kurva untuk kurva gamma dan Weibull).
- `cumulative`: Benar untuk memberikan jumlah kumulatif sampai titik ini; salah untuk memberikan nilai pada titik ini saja.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Memberikan peluang satu sisi bahwa rata-rata sampel berada di atas suatu nilai.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `x`: Nilai x yang akan diproses.
- `sigma` (opsional): Simpangan baku seluruh populasi (simpangan baku sampel jika dihilangkan).

## ZTEST

`ZTEST(array, x, [sigma])`

Memberikan peluang satu sisi bahwa rata-rata sampel berada di atas suatu nilai.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `x`: Nilai x yang akan diproses.
- `sigma` (opsional): Simpangan baku seluruh populasi (simpangan baku sampel jika dihilangkan).
