# Fungsi keuangan
<!-- keywords: fungsi, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT -->

Ini adalah fungsi keuangan yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa fungsinya, dan arti setiap argumennya. Argumen yang diapit kurung siku bersifat opsional, dan "..." berarti Anda dapat menambahkan argumen terakhir sebanyak yang diperlukan. Untuk mencobanya, lihat [Pilih Fungsi](insert-function). Saat Anda mengetik fungsi di dalam sel, sebuah tip kecil di bawah sel menampilkan argumennya. Jika Anda mengklik nama fungsi di dalam tip, entri fungsi ini akan terbuka.

## ACCRINT

`ACCRINT(issue, first_interest, settlement, rate, par, frequency, [basis], [calc_method])`

Menghasilkan bunga akumulasi pada sekuritas yang membayar bunga sesuai jadwal.

- `issue`: Tanggal sekuritas diterbitkan.
- `first_interest`: Tanggal bunga pertama dibayarkan.
- `settlement`: Tanggal Anda membeli sekuritas.
- `rate`: Suku bunga untuk setiap periode.
- `par`: Nilai nominal sekuritas (1.000 jika dihilangkan).
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.
- `calc_method` (opsional): True atau dihilangkan untuk menghitung bunga sejak tanggal penerbitan; false untuk menghitungnya sejak tanggal bunga pertama jika tanggal penyelesaian lebih akhir.

## ACCRINTM

`ACCRINTM(issue, settlement, rate, [par], [basis])`

Menghasilkan bunga akumulasi pada sekuritas yang membayar bunga saat jatuh tempo.

- `issue`: Tanggal sekuritas diterbitkan.
- `settlement`: Tanggal Anda membeli sekuritas.
- `rate`: Suku bunga untuk setiap periode.
- `par` (opsional): Nilai nominal sekuritas (1.000 jika dihilangkan).
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## AMORDEGRC

`AMORDEGRC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Menghasilkan penyusutan untuk satu periode dengan metode saldo menurun Prancis.

- `cost`: Harga aset saat dibeli.
- `date_purchased`: Tanggal aset dibeli.
- `first_period`: Tanggal berakhirnya periode pertama.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `period`: Periode yang akan dihitung, dengan periode pertama dihitung sebagai 1.
- `rate`: Suku bunga untuk setiap periode.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## AMORLINC

`AMORLINC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Menghasilkan penyusutan untuk satu periode dengan metode garis lurus Prancis.

- `cost`: Harga aset saat dibeli.
- `date_purchased`: Tanggal aset dibeli.
- `first_period`: Tanggal berakhirnya periode pertama.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `period`: Periode yang akan dihitung, dengan periode pertama dihitung sebagai 1.
- `rate`: Suku bunga untuk setiap periode.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## COUPDAYBS

`COUPDAYBS(settlement, maturity, frequency, [basis])`

Menghasilkan jumlah hari dari awal periode kupon hingga tanggal penyelesaian.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## COUPDAYS

`COUPDAYS(settlement, maturity, frequency, [basis])`

Menghasilkan jumlah hari dalam periode kupon yang memuat tanggal penyelesaian.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## COUPDAYSNC

`COUPDAYSNC(settlement, maturity, frequency, [basis])`

Menghasilkan jumlah hari dari tanggal penyelesaian hingga tanggal kupon berikutnya.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## COUPNCD

`COUPNCD(settlement, maturity, frequency, [basis])`

Menghasilkan tanggal kupon berikutnya setelah tanggal penyelesaian.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## COUPNUM

`COUPNUM(settlement, maturity, frequency, [basis])`

Menghasilkan jumlah kupon yang akan dibayarkan antara tanggal penyelesaian dan tanggal jatuh tempo.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## COUPPCD

`COUPPCD(settlement, maturity, frequency, [basis])`

Menghasilkan tanggal kupon sebelum tanggal penyelesaian.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## CUMIPMT

`CUMIPMT(rate, nper, pv, start_period, end_period, type)`

Menghasilkan bunga yang dibayarkan atas pinjaman di antara dua periode.

- `rate`: Suku bunga untuk setiap periode.
- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `start_period`: Periode tempat rentang dimulai.
- `end_period`: Periode tempat rentang berakhir.
- `type`: Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## CUMPRINC

`CUMPRINC(rate, nper, pv, start_period, end_period, type)`

Menghasilkan pokok pinjaman yang dibayarkan di antara dua periode.

- `rate`: Suku bunga untuk setiap periode.
- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `start_period`: Periode tempat rentang dimulai.
- `end_period`: Periode tempat rentang berakhir.
- `type`: Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## DB

`DB(cost, salvage, life, period, [month])`

Menghasilkan penyusutan aset untuk satu periode, dengan metode saldo menurun tetap.

- `cost`: Harga aset saat dibeli.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `life`: Berapa periode masa pakai aset.
- `period`: Periode yang akan dihitung, dengan periode pertama dihitung sebagai 1.
- `month` (opsional): Bulan, dari 1 sampai 12.

## DDB

`DDB(cost, salvage, life, period, [factor])`

Menghasilkan penyusutan untuk satu periode dengan metode saldo menurun.

- `cost`: Harga aset saat dibeli.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `life`: Berapa periode masa pakai aset.
- `period`: Periode yang akan dihitung, dengan periode pertama dihitung sebagai 1.
- `factor` (opsional): Seberapa cepat saldo menurun (2 jika dihilangkan, yaitu saldo menurun ganda).

## DISC

`DISC(settlement, maturity, pr, redemption, [basis])`

Menghasilkan tingkat diskonto sekuritas.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `pr`: Harga sekuritas per 100 nilai nominal.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## DOLLARDE

`DOLLARDE(fractional_dollar, fraction)`

Mengubah harga yang ditulis sebagai pecahan, seperti 1.02, menjadi bilangan desimal.

- `fractional_dollar`: Harga yang ditulis sebagai pecahan, misalnya 1.02 untuk satu dan dua per enam belas.
- `fraction`: Angka untuk membagi satu dolar: 16 untuk per enam belas, misalnya.

## DOLLARFR

`DOLLARFR(decimal_dollar, fraction)`

Mengubah harga desimal menjadi harga yang ditulis sebagai pecahan, seperti 1.02.

- `decimal_dollar`: Harga yang ditulis sebagai bilangan desimal, misalnya 1,125.
- `fraction`: Angka untuk membagi satu dolar: 16 untuk per enam belas, misalnya.

## DURATION

`DURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Menghasilkan rata-rata waktu untuk mengembalikan biaya obligasi, dengan pembobotan menurut pembayaran.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `coupon`: Tingkat kupon tahunan obligasi.
- `yld`: Hasil tahunan dari sekuritas tersebut.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## EFFECT

`EFFECT(nominal_rate, npery)`

Menghasilkan tingkat suku bunga tahunan efektif dari suku bunga nominal yang memiliki beberapa periode dalam setahun.

- `nominal_rate`: Suku bunga tahunan yang dinyatakan.
- `npery`: Berapa periode dalam setahun.

## FV

`FV(rate, nper, pmt, [pv], [type])`

Menghasilkan nilai pembayaran rutin pada akhir periode.

- `rate`: Suku bunga untuk setiap periode.
- `nper`: Jumlah seluruh pembayaran.
- `pmt`: Pembayaran yang dilakukan setiap periode.
- `pv` (opsional): Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## FVSCHEDULE

`FVSCHEDULE(principal, schedule)`

Menghasilkan nilai masa depan suatu jumlah setelah serangkaian suku bunga.

- `principal`: Jumlah awal.
- `schedule`: Blok sel yang berisi suku bunga yang akan diterapkan, satu demi satu.

## INTRATE

`INTRATE(settlement, maturity, investment, redemption, [basis])`

Menghasilkan suku bunga sekuritas yang sepenuhnya dibiayai.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `investment`: Jumlah yang dimasukkan ke dalam sekuritas.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## IPMT

`IPMT(rate, per, nper, pv, [fv], [type])`

Menghasilkan bagian bunga dari sebuah pembayaran.

- `rate`: Suku bunga untuk setiap periode.
- `per`: Pembayaran yang akan dihitung, dengan pembayaran pertama dihitung sebagai 1.
- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv` (opsional): Sisa setelah pembayaran terakhir (0 jika dihilangkan).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## IRR

`IRR(values, [guess])`

Menghasilkan tingkat pengembalian dari serangkaian jumlah yang datang pada waktu yang teratur.

- `values`: Blok sel yang berisi jumlah-jumlah tersebut.
- `guess` (opsional): Perkiraan Anda tentang hasilnya, sebagai titik awal pencarian (0,1 jika dihilangkan).

## ISPMT

`ISPMT(rate, per, nper, pv)`

Menghasilkan bunga yang dibayarkan dalam satu periode pinjaman yang dilunasi dengan bagian pokok yang sama besar.

- `rate`: Suku bunga untuk setiap periode.
- `per`: Pembayaran yang akan dihitung, dengan pembayaran pertama dihitung sebagai 1.
- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).

## MDURATION

`MDURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Menghasilkan durasi obligasi yang disesuaikan dengan hasilnya.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `coupon`: Tingkat kupon tahunan obligasi.
- `yld`: Hasil tahunan dari sekuritas tersebut.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## MIRR

`MIRR(values, finance_rate, reinvest_rate)`

Menghasilkan tingkat pengembalian dari arus kas, dengan satu tingkat untuk meminjam dan tingkat lain untuk menginvestasikan kembali.

- `values`: Blok sel yang berisi jumlah-jumlah tersebut.
- `finance_rate`: Tingkat yang Anda bayar atas uang yang Anda pinjam.
- `reinvest_rate`: Tingkat yang Anda peroleh atas uang yang Anda investasikan kembali.

## NOMINAL

`NOMINAL(effect_rate, npery)`

Menghasilkan suku bunga nominal dari suku bunga tahunan dan jumlah periode dalam setahun.

- `effect_rate`: Suku bunga tahunan.
- `npery`: Berapa periode dalam setahun.

## NPER

`NPER(rate, pmt, pv, [fv], [type])`

Menghasilkan jumlah pembayaran yang diperlukan untuk melunasi pinjaman.

- `rate`: Suku bunga untuk setiap periode.
- `pmt`: Pembayaran yang dilakukan setiap periode.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv` (opsional): Sisa setelah pembayaran terakhir (0 jika dihilangkan).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## NPV

`NPV(rate, value1, ...)`

Menghasilkan nilai sekarang dari serangkaian jumlah di masa depan.

- `rate`: Suku bunga untuk setiap periode.
- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## ODDFPRICE

`ODDFPRICE(settlement, maturity, issue, first_coupon, rate, yld, redemption, frequency, [basis])`

Menghasilkan harga per 100 untuk obligasi dengan periode pertama yang tidak penuh.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `issue`: Tanggal sekuritas diterbitkan.
- `first_coupon`: Tanggal kupon pertama.
- `rate`: Suku bunga untuk setiap periode.
- `yld`: Hasil tahunan dari sekuritas tersebut.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## ODDFYIELD

`ODDFYIELD(settlement, maturity, issue, first_coupon, rate, pr, redemption, frequency, [basis])`

Menghasilkan hasil obligasi dengan periode pertama yang tidak penuh.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `issue`: Tanggal sekuritas diterbitkan.
- `first_coupon`: Tanggal kupon pertama.
- `rate`: Suku bunga untuk setiap periode.
- `pr`: Harga sekuritas per 100 nilai nominal.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## ODDLPRICE

`ODDLPRICE(settlement, maturity, last_interest, rate, yld, redemption, frequency, [basis])`

Menghasilkan harga per 100 untuk obligasi dengan periode terakhir yang tidak penuh.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `last_interest`: Tanggal bunga terakhir dibayarkan.
- `rate`: Suku bunga untuk setiap periode.
- `yld`: Hasil tahunan dari sekuritas tersebut.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## ODDLYIELD

`ODDLYIELD(settlement, maturity, last_interest, rate, pr, redemption, frequency, [basis])`

Menghasilkan hasil obligasi dengan periode terakhir yang tidak penuh.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `last_interest`: Tanggal bunga terakhir dibayarkan.
- `rate`: Suku bunga untuk setiap periode.
- `pr`: Harga sekuritas per 100 nilai nominal.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## PDURATION

`PDURATION(rate, pv, fv)`

Menghasilkan jumlah periode yang diperlukan investasi untuk mencapai suatu nilai.

- `rate`: Suku bunga untuk setiap periode.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv`: Sisa setelah pembayaran terakhir (0 jika dihilangkan).

## PMT

`PMT(rate, nper, pv, [fv], [type])`

Menghasilkan pembayaran rutin untuk sebuah pinjaman.

- `rate`: Suku bunga untuk setiap periode.
- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv` (opsional): Sisa setelah pembayaran terakhir (0 jika dihilangkan).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## PPMT

`PPMT(rate, per, nper, pv, [fv], [type])`

Menghasilkan bagian dari sebuah pembayaran yang digunakan untuk melunasi pinjaman itu sendiri.

- `rate`: Suku bunga untuk setiap periode.
- `per`: Pembayaran yang akan dihitung, dengan pembayaran pertama dihitung sebagai 1.
- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv` (opsional): Sisa setelah pembayaran terakhir (0 jika dihilangkan).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## PRICE

`PRICE(settlement, maturity, rate, yld, redemption, frequency, [basis])`

Menghasilkan harga per 100 untuk obligasi yang membayar bunga sesuai jadwal.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `rate`: Suku bunga untuk setiap periode.
- `yld`: Hasil tahunan dari sekuritas tersebut.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## PRICEDISC

`PRICEDISC(settlement, maturity, discount, redemption, [basis])`

Menghasilkan harga per 100 untuk sekuritas yang dibeli dengan diskon.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `discount`: Tingkat diskonto sekuritas.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## PRICEMAT

`PRICEMAT(settlement, maturity, issue, rate, yld, [basis])`

Menghasilkan harga per 100 untuk sekuritas yang membayar bunga saat jatuh tempo.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `issue`: Tanggal sekuritas diterbitkan.
- `rate`: Suku bunga untuk setiap periode.
- `yld`: Hasil tahunan dari sekuritas tersebut.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## PV

`PV(rate, nper, pmt, [fv], [type])`

Menghasilkan nilai sekarang dari serangkaian pembayaran di masa depan.

- `rate`: Suku bunga untuk setiap periode.
- `nper`: Jumlah seluruh pembayaran.
- `pmt`: Pembayaran yang dilakukan setiap periode.
- `fv` (opsional): Sisa setelah pembayaran terakhir (0 jika dihilangkan).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.

## RATE

`RATE(nper, pmt, pv, [fv], [type], [guess])`

Menghasilkan suku bunga untuk setiap periode pinjaman atau investasi.

- `nper`: Jumlah seluruh pembayaran.
- `pmt`: Pembayaran yang dilakukan setiap periode.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv` (opsional): Sisa setelah pembayaran terakhir (0 jika dihilangkan).
- `type` (opsional): Waktu pembayaran: 0 atau dihilangkan pada akhir setiap periode, 1 pada awal periode.
- `guess` (opsional): Perkiraan Anda tentang hasilnya, sebagai titik awal pencarian (0,1 jika dihilangkan).

## RECEIVED

`RECEIVED(settlement, maturity, investment, discount, [basis])`

Menghasilkan jumlah yang diterima saat jatuh tempo untuk sekuritas yang sepenuhnya dibiayai.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `investment`: Jumlah yang dimasukkan ke dalam sekuritas.
- `discount`: Tingkat diskonto sekuritas.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## RRI

`RRI(nper, pv, fv)`

Menghasilkan suku bunga yang membuat suatu investasi tumbuh hingga mencapai nilai tertentu.

- `nper`: Jumlah seluruh pembayaran.
- `pv`: Nilai sekarang dari jumlah tersebut (seluruh pinjaman atau investasi).
- `fv`: Sisa setelah pembayaran terakhir (0 jika dihilangkan).

## SLN

`SLN(cost, salvage, life)`

Menghasilkan penyusutan garis lurus untuk satu tahun.

- `cost`: Harga aset saat dibeli.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `life`: Berapa periode masa pakai aset.

## SYD

`SYD(cost, salvage, life, per)`

Menghasilkan penyusutan satu tahun dengan metode jumlah digit tahun.

- `cost`: Harga aset saat dibeli.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `life`: Berapa periode masa pakai aset.
- `per`: Pembayaran yang akan dihitung, dengan pembayaran pertama dihitung sebagai 1.

## TBILLEQ

`TBILLEQ(settlement, maturity, discount)`

Menghasilkan hasil setara obligasi dari surat perbendaharaan negara.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `discount`: Tingkat diskonto sekuritas.

## TBILLPRICE

`TBILLPRICE(settlement, maturity, discount)`

Menghasilkan harga per 100 untuk surat perbendaharaan negara.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `discount`: Tingkat diskonto sekuritas.

## TBILLYIELD

`TBILLYIELD(settlement, maturity, pr)`

Menghasilkan hasil dari surat perbendaharaan negara.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `pr`: Harga sekuritas per 100 nilai nominal.

## VDB

`VDB(cost, salvage, life, start_period, end_period, [factor], [no_switch])`

Menghasilkan penyusutan untuk rentang periode apa pun dengan metode saldo menurun.

- `cost`: Harga aset saat dibeli.
- `salvage`: Nilai aset pada akhir masa pakainya.
- `life`: Berapa periode masa pakai aset.
- `start_period`: Periode tempat rentang dimulai.
- `end_period`: Periode tempat rentang berakhir.
- `factor` (opsional): Seberapa cepat saldo menurun (2 jika dihilangkan, yaitu saldo menurun ganda).
- `no_switch` (opsional): True untuk tetap menggunakan metode saldo menurun meskipun garis lurus menghasilkan nilai yang lebih besar.

## XIRR

`XIRR(values, dates, [guess])`

Menghasilkan tingkat pengembalian dari jumlah yang datang pada tanggal yang Anda pilih.

- `values`: Blok sel yang berisi jumlah-jumlah tersebut.
- `dates`: Tanggal dari jumlah-jumlah tersebut, sesuai urutan jumlahnya.
- `guess` (opsional): Perkiraan Anda tentang hasilnya, sebagai titik awal pencarian (0,1 jika dihilangkan).

## XNPV

`XNPV(rate, values, dates)`

Menghasilkan nilai sekarang dari jumlah-jumlah yang datang pada tanggal yang Anda pilih.

- `rate`: Suku bunga untuk setiap periode.
- `values`: Blok sel yang berisi jumlah-jumlah tersebut.
- `dates`: Tanggal dari jumlah-jumlah tersebut, sesuai urutan jumlahnya.

## YIELD

`YIELD(settlement, maturity, rate, pr, redemption, frequency, [basis])`

Menghasilkan hasil obligasi yang membayar bunga sesuai jadwal.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `rate`: Suku bunga untuk setiap periode.
- `pr`: Harga sekuritas per 100 nilai nominal.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `frequency`: Berapa kali kupon dibayarkan dalam setahun: 1, 2, atau 4.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## YIELDDISC

`YIELDDISC(settlement, maturity, pr, redemption, [basis])`

Menghasilkan hasil tahunan dari sekuritas yang dibeli dengan diskon.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `pr`: Harga sekuritas per 100 nilai nominal.
- `redemption`: Nilai sekuritas saat jatuh tempo, per 100 nilai nominal.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.

## YIELDMAT

`YIELDMAT(settlement, maturity, issue, rate, pr, [basis])`

Menghasilkan hasil tahunan dari sekuritas yang membayar bunga saat jatuh tempo.

- `settlement`: Tanggal Anda membeli sekuritas.
- `maturity`: Tanggal sekuritas berakhir dan dibayarkan kembali.
- `issue`: Tanggal sekuritas diterbitkan.
- `rate`: Suku bunga untuk setiap periode.
- `pr`: Harga sekuritas per 100 nilai nominal.
- `basis` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 hari sebenarnya, 2 hari sebenarnya dalam setahun 360 hari, 3 hari sebenarnya dalam setahun 365 hari, 4 30/360 Eropa.
