# Fungsi teknik

<!-- keywords: fungsi, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX -->

Ini adalah fungsi teknik yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, fungsinya, dan masing-masing argumennya. Argumen dalam tanda kurung siku bersifat opsional, dan "..." berarti Anda dapat memberikan sebanyak yang dibutuhkan tambahan untuk argumen terakhir. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di dalam sel, sebuah tips kecil di bawah sel menampilkan argumennya, dan klik pada nama fungsi di dalam tips akan membuka entrinya di sini.

## BESSELI

`BESSELI(x, n)`

Memberikan fungsi Bessel terubah jenis pertama.

- `x`: Nilai x yang akan diproses.
- `n`: Pangkat pertama dari x, atau orde fungsi, berupa bilangan bulat 0 atau lebih.

## BESSELJ

`BESSELJ(x, n)`

Memberikan fungsi Bessel jenis pertama.

- `x`: Nilai x yang akan diproses.
- `n`: Pangkat pertama dari x, atau orde fungsi, berupa bilangan bulat 0 atau lebih.

## BESSELK

`BESSELK(x, n)`

Memberikan fungsi Bessel terubah jenis kedua.

- `x`: Nilai x yang akan diproses.
- `n`: Pangkat pertama dari x, atau orde fungsi, berupa bilangan bulat 0 atau lebih.

## BESSELY

`BESSELY(x, n)`

Memberikan fungsi Bessel jenis kedua.

- `x`: Nilai x yang akan diproses.
- `n`: Pangkat pertama dari x, atau orde fungsi, berupa bilangan bulat 0 atau lebih.

## BIN2DEC

`BIN2DEC(number)`

Mengubah bilangan biner menjadi bilangan desimal.

- `number`: Angka yang akan diproses.

## BIN2HEX

`BIN2HEX(number, [places])`

Mengubah bilangan biner menjadi bilangan heksadesimal.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## BIN2OCT

`BIN2OCT(number, [places])`

Mengubah bilangan biner menjadi bilangan oktal.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## BITAND

`BITAND(number1, number2)`

Memberikan bit yang aktif pada kedua dari dua bilangan bulat.

- `number1`: Angka pertama, atau blok sel.
- `number2`: Angka kedua.

## BITLSHIFT

`BITLSHIFT(number, shift_amount)`

Menggeser bit sebuah bilangan bulat ke kiri.

- `number`: Angka yang akan diproses.
- `shift_amount`: Berapa bit yang akan digeser: bilangan positif dan negatif bergerak ke arah yang berlawanan.

## BITOR

`BITOR(number1, number2)`

Memberikan bit yang aktif pada salah satu dari dua bilangan bulat.

- `number1`: Angka pertama, atau blok sel.
- `number2`: Angka kedua.

## BITRSHIFT

`BITRSHIFT(number, shift_amount)`

Menggeser bit sebuah bilangan bulat ke kanan.

- `number`: Angka yang akan diproses.
- `shift_amount`: Berapa bit yang akan digeser: bilangan positif dan negatif bergerak ke arah yang berlawanan.

## BITXOR

`BITXOR(number1, number2)`

Memberikan bit yang aktif pada salah satu dari dua bilangan bulat, tetapi tidak pada keduanya.

- `number1`: Angka pertama, atau blok sel.
- `number2`: Angka kedua.

## COMPLEX

`COMPLEX(real_num, i_num, [suffix])`

Membuat bilangan kompleks dari bagian riil dan imajinernya.

- `real_num`: Bagian riil dari bilangan kompleks.
- `i_num`: Bagian imajiner dari bilangan kompleks.
- `suffix` (opsional): i atau j, untuk menandai bagian imajiner (i jika dihilangkan).

## CONVERT

`CONVERT(number, from_unit, to_unit)`

Mengubah angka dari satu satuan ukur ke satuan ukur lain.

- `number`: Angka yang akan diproses.
- `from_unit`: Satuan tempat angka berada, sebagai teks, misalnya "mi" atau "lbm".
- `to_unit`: Satuan yang akan diubah, sebagai teks, misalnya "km" atau "kg".

## DEC2BIN

`DEC2BIN(number, [places])`

Mengubah bilangan desimal menjadi bilangan biner.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## DEC2HEX

`DEC2HEX(number, [places])`

Mengubah bilangan desimal menjadi bilangan heksadesimal.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## DEC2OCT

`DEC2OCT(number, [places])`

Mengubah bilangan desimal menjadi bilangan oktal.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## DELTA

`DELTA(number1, [number2])`

Menentukan apakah dua angka sama, dengan 1 untuk ya dan 0 untuk tidak.

- `number1`: Angka pertama, atau blok sel.
- `number2` (opsional): Angka kedua.

## ERF

`ERF(lower_limit, [upper_limit])`

Memberikan fungsi galat, di antara dua batas atau dari 0.

- `lower_limit`: Batas bawah rentang.
- `upper_limit` (opsional): Batas atas rentang (batas bawah saja jika dihilangkan).

## ERF.PRECISE

`ERF.PRECISE(x)`

Memberikan fungsi galat dari 0 hingga suatu batas.

- `x`: Nilai x yang akan diproses.

## ERFC

`ERFC(x)`

Memberikan fungsi galat komplementer dari suatu batas hingga tak hingga.

- `x`: Nilai x yang akan diproses.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Memberikan fungsi galat komplementer dari suatu batas hingga tak hingga.

- `x`: Nilai x yang akan diproses.

## GESTEP

`GESTEP(number, [step])`

Menentukan apakah sebuah angka setidaknya sebesar suatu langkah, dengan 1 untuk ya dan 0 untuk tidak.

- `number`: Angka yang akan diproses.
- `step` (opsional): Berapa yang ditambahkan untuk setiap angka (1 jika dihilangkan; untuk GESTEP, angka pembanding, 0 jika dihilangkan).

## HEX2BIN

`HEX2BIN(number, [places])`

Mengubah bilangan heksadesimal menjadi bilangan biner.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## HEX2DEC

`HEX2DEC(number)`

Mengubah bilangan heksadesimal menjadi bilangan desimal.

- `number`: Angka yang akan diproses.

## HEX2OCT

`HEX2OCT(number, [places])`

Mengubah bilangan heksadesimal menjadi bilangan oktal.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## IMABS

`IMABS(inumber)`

Memberikan besar dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMAGINARY

`IMAGINARY(inumber)`

Memberikan bagian imajiner dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMARGUMENT

`IMARGUMENT(inumber)`

Memberikan sudut dari bilangan kompleks, dalam radian.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMCONJUGATE

`IMCONJUGATE(inumber)`

Memberikan konjugat dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMCOS

`IMCOS(inumber)`

Memberikan kosinus dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMCOSH

`IMCOSH(inumber)`

Memberikan kosinus hiperbolik dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMCOT

`IMCOT(inumber)`

Memberikan kotangen dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMCSC

`IMCSC(inumber)`

Memberikan kosekan dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMCSCH

`IMCSCH(inumber)`

Memberikan kosekan hiperbolik dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMDIV

`IMDIV(inumber1, inumber2)`

Memberikan hasil bagi dua bilangan kompleks.

- `inumber1`: Bilangan kompleks pertama, ditulis sebagai teks, misalnya "3+4i".
- `inumber2`: Bilangan kompleks kedua, ditulis sebagai teks.

## IMEXP

`IMEXP(inumber)`

Memberikan e dipangkatkan dengan bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMLN

`IMLN(inumber)`

Memberikan logaritma natural dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMLOG10

`IMLOG10(inumber)`

Memberikan logaritma basis-10 dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMLOG2

`IMLOG2(inumber)`

Memberikan logaritma basis-2 dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMPOWER

`IMPOWER(inumber, number)`

Memberikan bilangan kompleks yang dipangkatkan.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".
- `number`: Angka yang akan diproses.

## IMPRODUCT

`IMPRODUCT(inumber1, ...)`

Memberikan hasil kali dari bilangan kompleks.

- `inumber1`: Bilangan kompleks pertama, ditulis sebagai teks, misalnya "3+4i".
- `...`: Sebanyak tambahan yang Anda butuhkan, dengan jenis yang sama seperti argumen sebelumnya.

## IMREAL

`IMREAL(inumber)`

Memberikan bagian riil dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMSEC

`IMSEC(inumber)`

Memberikan sekan dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMSECH

`IMSECH(inumber)`

Memberikan sekan hiperbolik dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMSIN

`IMSIN(inumber)`

Memberikan sinus dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMSINH

`IMSINH(inumber)`

Memberikan sinus hiperbolik dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMSQRT

`IMSQRT(inumber)`

Memberikan akar kuadrat dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## IMSUB

`IMSUB(inumber1, inumber2)`

Memberikan selisih dua bilangan kompleks.

- `inumber1`: Bilangan kompleks pertama, ditulis sebagai teks, misalnya "3+4i".
- `inumber2`: Bilangan kompleks kedua, ditulis sebagai teks.

## IMSUM

`IMSUM(inumber1, ...)`

Memberikan jumlah dari bilangan kompleks.

- `inumber1`: Bilangan kompleks pertama, ditulis sebagai teks, misalnya "3+4i".
- `...`: Sebanyak tambahan yang Anda butuhkan, dengan jenis yang sama seperti argumen sebelumnya.

## IMTAN

`IMTAN(inumber)`

Memberikan tangen dari bilangan kompleks.

- `inumber`: Bilangan kompleks yang ditulis sebagai teks, misalnya "3+4i".

## OCT2BIN

`OCT2BIN(number, [places])`

Mengubah bilangan oktal menjadi bilangan biner.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.

## OCT2DEC

`OCT2DEC(number)`

Mengubah bilangan oktal menjadi bilangan desimal.

- `number`: Angka yang akan diproses.

## OCT2HEX

`OCT2HEX(number, [places])`

Mengubah bilangan oktal menjadi bilangan heksadesimal.

- `number`: Angka yang akan diproses.
- `places` (opsional): Berapa karakter hasil diisi hingga panjangnya dengan angka nol di sebelah kiri.
