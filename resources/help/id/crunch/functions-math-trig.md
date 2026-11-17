# Fungsi matematika dan trigonometri
<!-- keywords: fungsi, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC -->

Ini adalah fungsi matematika dan trigonometri yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, fungsinya, dan setiap argumennya. Argumen yang diapit kurung siku bersifat opsional, dan "..." berarti Anda dapat memberikan sebanyak mungkin argumen terakhir yang Anda perlukan. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di sel, sebuah tip kecil di bawah sel menampilkan argumennya, dan klik pada nama fungsi di tip tersebut akan membuka entri fungsi ini.

## ABS

`ABS(number)`

Memberikan angka tanpa tanda positif atau negatifnya.

- `number`: Angka yang akan diproses.

## ACOS

`ACOS(number)`

Memberikan sudut, dalam radian, yang memiliki kosinus tertentu.

- `number`: Angka yang akan diproses.

## ACOSH

`ACOSH(number)`

Memberikan kosinus hiperbolik invers dari sebuah angka.

- `number`: Angka yang akan diproses.

## ACOT

`ACOT(number)`

Memberikan kotangen invers dari sebuah angka, sebagai sudut dalam radian.

- `number`: Angka yang akan diproses.

## ACOTH

`ACOTH(number)`

Memberikan kotangen hiperbolik invers dari sebuah angka.

- `number`: Angka yang akan diproses.

## AGGREGATE

`AGGREGATE(function_num, options, ref1, ...)`

Melakukan perhitungan pada daftar, seperti jumlah atau median, dan dapat mengabaikan baris tersembunyi dan galat.

- `function_num`: Perhitungan yang akan dilakukan: 1 rata-rata, 2 hitung, 3 hitung yang tidak kosong, 4 maksimum, 5 minimum, 6 hasil kali, 7 dan 8 simpangan baku, 9 jumlah, 10 dan 11 varians.
- `options`: Apa yang diabaikan: 0 total bersarang, 1 juga baris tersembunyi, 2 juga galat, 3 keduanya, 4 sampai 7 untuk tetap menyertakan total bersarang.
- `ref1`: Sel pertama atau blok sel pertama.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## ARABIC

`ARABIC(text)`

Membaca angka Romawi dalam teks sebagai bilangan.

- `text`: Teks yang akan diproses, atau sel yang berisinya.

## ASIN

`ASIN(number)`

Memberikan sudut, dalam radian, yang memiliki sinus tertentu.

- `number`: Angka yang akan diproses.

## ASINH

`ASINH(number)`

Memberikan sinus hiperbolik invers dari sebuah angka.

- `number`: Angka yang akan diproses.

## ATAN

`ATAN(number)`

Memberikan sudut, dalam radian, yang memiliki tangen tertentu.

- `number`: Angka yang akan diproses.

## ATAN2

`ATAN2(x_num, y_num)`

Memberikan sudut, dalam radian, dari titik pada x dan y.

- `x_num`: Nilai x.
- `y_num`: Nilai y.

## ATANH

`ATANH(number)`

Memberikan tangen hiperbolik invers dari sebuah angka.

- `number`: Angka yang akan diproses.

## BASE

`BASE(number, radix, [min_length])`

Menuliskan angka dalam basis lain, dari 2 sampai 36, sebagai teks.

- `number`: Angka yang akan diproses.
- `radix`: Basis dari angka tersebut: dari 2 sampai 36.
- `min_length` (opsional): Jumlah karakter paling sedikit yang dimiliki hasil; hasil yang lebih pendek diisi nol di sebelah kiri.

## CEILING

`CEILING(number, [significance])`

Membulatkan angka ke atas ke kelipatan terdekat.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.

## CEILING.MATH

`CEILING.MATH(number, [significance], [mode])`

Membulatkan angka ke atas ke kelipatan terdekat, dengan pilihan untuk angka negatif.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.
- `mode` (opsional): Cara membulatkan angka negatif: 0 atau dihilangkan adalah cara biasa, dan angka lain membalikkannya.

## CEILING.PRECISE

`CEILING.PRECISE(number, [significance])`

Membulatkan angka ke atas ke kelipatan terdekat, apa pun tanda kelipatannya.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.

## COMBIN

`COMBIN(number, number_chosen)`

Menghitung cara memilih item dari sekelompok item ketika urutan tidak penting.

- `number`: Angka yang akan diproses.
- `number_chosen`: Berapa item yang akan dipilih di setiap kelompok.

## COMBINA

`COMBINA(number, number_chosen)`

Memberikan jumlah cara memilih item ketika sebuah item boleh dipilih lebih dari satu kali.

- `number`: Angka yang akan diproses.
- `number_chosen`: Berapa item yang akan dipilih di setiap kelompok.

## COS

`COS(number)`

Memberikan kosinus dari suatu sudut dalam radian.

- `number`: Angka yang akan diproses.

## COSH

`COSH(number)`

Memberikan kosinus hiperbolik dari suatu angka.

- `number`: Angka yang akan diproses.

## COT

`COT(angle)`

Memberikan kotangen dari suatu sudut dalam radian.

- `angle`: Sudut, dalam radian.

## COTH

`COTH(number)`

Memberikan kotangen hiperbolik dari sebuah angka.

- `number`: Angka yang akan diproses.

## CSC

`CSC(angle)`

Memberikan kosekan dari suatu sudut dalam radian.

- `angle`: Sudut, dalam radian.

## CSCH

`CSCH(number)`

Menghasilkan kosekan hiperbolik dari sebuah angka.

- `number`: Angka yang akan diproses.

## DECIMAL

`DECIMAL(text, radix)`

Membaca teks yang ditulis dalam basis 2 sampai 36 sebagai angka.

- `text`: Teks yang akan diproses, atau sel yang berisinya.
- `radix`: Basis dari angka tersebut: dari 2 sampai 36.

## DEGREES

`DEGREES(angle)`

Mengubah radian menjadi derajat.

- `angle`: Sudut, dalam radian.

## EVEN

`EVEN(number)`

Membulatkan angka ke atas ke bilangan bulat genap terdekat.

- `number`: Angka yang akan diproses.

## EXP

`EXP(number)`

Memberikan e dipangkatkan dengan suatu angka.

- `number`: Angka yang akan diproses.

## FACT

`FACT(number)`

Memberikan faktorial dari suatu angka.

- `number`: Angka yang akan diproses.

## FACTDOUBLE

`FACTDOUBLE(number)`

Memberikan faktorial ganda dari sebuah angka: hasil kali setiap angka kedua hingga 1 atau 2.

- `number`: Angka yang akan diproses.

## FLOOR

`FLOOR(number, [significance])`

Membulatkan angka ke bawah ke kelipatan terdekat.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.

## FLOOR.MATH

`FLOOR.MATH(number, [significance], [mode])`

Membulatkan angka ke bawah ke kelipatan terdekat, dengan pilihan untuk angka negatif.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.
- `mode` (opsional): Cara membulatkan angka negatif: 0 atau dihilangkan adalah cara biasa, dan angka lain membalikkannya.

## FLOOR.PRECISE

`FLOOR.PRECISE(number, [significance])`

Membulatkan angka ke bawah ke kelipatan terdekat, apa pun tanda kelipatannya.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.

## GCD

`GCD(number1, ...)`

Memberikan bilangan bulat terbesar yang membagi habis semua angka.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## INT

`INT(number)`

Membulatkan angka ke bawah ke bilangan bulat di bawahnya.

- `number`: Angka yang akan diproses.

## ISO.CEILING

`ISO.CEILING(number, [significance])`

Membulatkan angka ke atas ke kelipatan terdekat, apa pun tanda kelipatannya.

- `number`: Angka yang akan diproses.
- `significance` (opsional): Angka yang kelipatannya dijadikan pembulatan.

## LCM

`LCM(number1, ...)`

Memberikan bilangan bulat terkecil yang habis dibagi oleh semua angka.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## LN

`LN(number)`

Memberikan logaritma natural dari suatu angka.

- `number`: Angka yang akan diproses.

## LOG

`LOG(number, [base])`

Memberikan logaritma suatu angka dengan basis tertentu (10 jika tidak diberikan).

- `number`: Angka yang akan diproses.
- `base` (opsional): Basis logaritma (10 jika dihilangkan).

## LOG10

`LOG10(number)`

Memberikan logaritma suatu angka dengan basis 10.

- `number`: Angka yang akan diproses.

## MAX

`MAX(number1, ...)`

Memberikan angka terbesar.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## MDETERM

`MDETERM(array)`

Memberikan determinan dari blok angka persegi.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.

## MIN

`MIN(number1, ...)`

Memberikan angka terkecil.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## MINVERSE

`MINVERSE(array)`

Memberikan invers dari blok angka persegi.

- `array`: Blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.

## MMULT

`MMULT(array1, array2)`

Memberikan hasil kali dua blok angka, sebagai matriks.

- `array1`: Blok sel atau daftar nilai pertama.
- `array2`: Blok sel atau daftar nilai kedua, dengan ukuran sama seperti yang pertama.

## MOD

`MOD(number, divisor)`

Memberikan sisa dari pembagian satu angka dengan angka lain.

- `number`: Angka yang akan diproses.
- `divisor`: Angka untuk membagi.

## MROUND

`MROUND(number, multiple)`

Membulatkan angka ke kelipatan terdekat.

- `number`: Angka yang akan diproses.
- `multiple`: Angka yang kelipatannya dijadikan pembulatan.

## MULTINOMIAL

`MULTINOMIAL(number1, ...)`

Memberikan faktorial dari jumlah angka dibagi dengan hasil kali faktorial masing-masing angka.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## MUNIT

`MUNIT(dimension)`

Memberikan matriks identitas dengan ukuran tertentu.

- `dimension`: Ukuran blok persegi: berapa baris dan kolom.

## ODD

`ODD(number)`

Membulatkan angka ke atas ke bilangan bulat ganjil terdekat.

- `number`: Angka yang akan diproses.

## PERMUT

`PERMUT(number, number_chosen)`

Menghitung cara memilih item dari sekelompok item ketika urutan penting.

- `number`: Angka yang akan diproses.
- `number_chosen`: Berapa item yang akan dipilih di setiap kelompok.

## PI

`PI()`

Memberikan nilai pi.

## POWER

`POWER(number, power)`

Memangkatkan suatu angka.

- `number`: Angka yang akan diproses.
- `power`: Pangkat yang digunakan untuk memangkatkan angka.

## PRODUCT

`PRODUCT(number1, ...)`

Mengalikan angka-angka.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## QUOTIENT

`QUOTIENT(numerator, denominator)`

Memberikan bagian bulat dari suatu pembagian.

- `numerator`: Angka yang akan dibagi.
- `denominator`: Angka untuk membagi.

## RADIANS

`RADIANS(angle)`

Mengubah derajat menjadi radian.

- `angle`: Sudut, dalam derajat.

## RAND

`RAND()`

Memberikan angka acak dari 0 hingga, tetapi tidak termasuk, 1.

## RANDARRAY

`RANDARRAY([rows], [columns], [min], [max], [whole_number])`

Menghasilkan blok angka acak.

- `rows` (opsional): Berapa baris yang akan digeser: positif ke bawah, negatif ke atas.
- `columns` (opsional): Lebar hasil dalam jumlah kolom (1 jika dihilangkan).
- `min` (opsional): Angka terkecil yang boleh dimiliki hasil (0 jika dihilangkan).
- `max` (opsional): Angka terbesar yang boleh dimiliki hasil (1 jika dihilangkan).
- `whole_number` (opsional): Benar untuk bilangan bulat; salah atau dihilangkan untuk angka desimal.

## RANDBETWEEN

`RANDBETWEEN(bottom, top)`

Memberikan bilangan bulat acak di antara dua angka.

- `bottom`: Nilai terkecil yang boleh dimiliki hasil.
- `top`: Nilai terbesar yang boleh dimiliki hasil.

## ROMAN

`ROMAN(number, [form])`

Menuliskan bilangan bulat dari 0 sampai 3999 sebagai angka Romawi dalam teks.

- `number`: Angka yang akan diproses.
- `form` (opsional): Seberapa pendek angka Romawi tersebut: 0 atau dihilangkan untuk bentuk klasik, hingga 4 untuk bentuk terpendek.

## ROUND

`ROUND(number, num_digits)`

Membulatkan angka sampai sejumlah digit.

- `number`: Angka yang akan diproses.
- `num_digits`: Berapa digit yang dibulatkan: positif untuk sisi kanan tanda desimal, 0 untuk bilangan bulat, negatif untuk sisi kiri.

## ROUNDDOWN

`ROUNDDOWN(number, num_digits)`

Membulatkan angka ke bawah, menuju nol, sampai sejumlah digit.

- `number`: Angka yang akan diproses.
- `num_digits`: Berapa digit yang dibulatkan: positif untuk sisi kanan tanda desimal, 0 untuk bilangan bulat, negatif untuk sisi kiri.

## ROUNDUP

`ROUNDUP(number, num_digits)`

Membulatkan angka ke atas, menjauhi nol, sampai sejumlah digit.

- `number`: Angka yang akan diproses.
- `num_digits`: Berapa digit yang dibulatkan: positif untuk sisi kanan tanda desimal, 0 untuk bilangan bulat, negatif untuk sisi kiri.

## SEC

`SEC(angle)`

Memberikan sekan dari suatu sudut dalam radian.

- `angle`: Sudut, dalam radian.

## SECH

`SECH(number)`

Memberikan sekan hiperbolik dari sebuah angka.

- `number`: Angka yang akan diproses.

## SEQUENCE

`SEQUENCE(rows, [columns], [start], [step])`

Memberikan blok angka yang bertambah mulai dari angka awal dengan langkah tertentu.

- `rows`: Berapa baris yang akan digeser: positif ke bawah, negatif ke atas.
- `columns` (opsional): Lebar hasil dalam jumlah kolom (1 jika dihilangkan).
- `start` (opsional): Angka pertama (1 jika dihilangkan).
- `step` (opsional): Jumlah yang ditambahkan untuk setiap angka (1 jika dihilangkan; untuk GESTEP, angka pembanding, 0 jika dihilangkan).

## SERIESSUM

`SERIESSUM(x, n, m, coefficients)`

Memberikan jumlah dari deret pangkat.

- `x`: Nilai x yang akan diproses.
- `n`: Pangkat pertama dari x, atau urutan fungsi, berupa bilangan bulat 0 atau lebih.
- `m`: Langkah yang ditambahkan pada pangkat untuk setiap koefisien.
- `coefficients`: Angka yang akan dikalikan dengan setiap pangkat x.

## SIGN

`SIGN(number)`

Memberikan 1 untuk angka positif, 0 untuk nol, dan -1 untuk angka negatif.

- `number`: Angka yang akan diproses.

## SIN

`SIN(number)`

Memberikan sinus dari suatu sudut dalam radian.

- `number`: Angka yang akan diproses.

## SINH

`SINH(number)`

Memberikan sinus hiperbolik dari suatu angka.

- `number`: Angka yang akan diproses.

## SQRT

`SQRT(number)`

Memberikan akar kuadrat dari suatu angka.

- `number`: Angka yang akan diproses.

## SQRTPI

`SQRTPI(number)`

Memberikan akar kuadrat dari suatu angka dikali pi.

- `number`: Angka yang akan diproses.

## SUBTOTAL

`SUBTOTAL(function_num, ref1, ...)`

Memberikan total, rata-rata, jumlah hitungan, atau angka lain untuk blok sel.

- `function_num`: Perhitungan yang akan dilakukan: 1 rata-rata, 2 hitung, 3 hitung yang tidak kosong, 4 maksimum, 5 minimum, 6 hasil kali, 7 dan 8 simpangan baku, 9 jumlah, 10 dan 11 varians.
- `ref1`: Sel pertama atau blok sel pertama.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## SUM

`SUM(number1, ...)`

Menjumlahkan angka dan angka dalam blok sel.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## SUMPRODUCT

`SUMPRODUCT(array1, ...)`

Mengalikan angka dalam blok yang berukuran sama, posisi demi posisi, lalu menjumlahkan hasil kalinya.

- `array1`: Blok sel atau daftar nilai pertama.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## SUMSQ

`SUMSQ(number1, ...)`

Menjumlahkan kuadrat angka.

- `number1`: Angka pertama, atau blok sel.
- `...`: Sebanyak yang Anda perlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## SUMX2MY2

`SUMX2MY2(array_x, array_y)`

Menjumlahkan selisih kuadrat dari dua kumpulan angka.

- `array_x`: Blok sel atau daftar nilai pertama.
- `array_y`: Blok sel atau daftar nilai kedua, dengan ukuran sama seperti yang pertama.

## SUMX2PY2

`SUMX2PY2(array_x, array_y)`

Menjumlahkan hasil penjumlahan kuadrat dari dua kumpulan angka.

- `array_x`: Blok sel atau daftar nilai pertama.
- `array_y`: Blok sel atau daftar nilai kedua, dengan ukuran sama seperti yang pertama.

## SUMXMY2

`SUMXMY2(array_x, array_y)`

Menjumlahkan kuadrat selisih dari dua kumpulan angka.

- `array_x`: Blok sel atau daftar nilai pertama.
- `array_y`: Blok sel atau daftar nilai kedua, dengan ukuran sama seperti yang pertama.

## TAN

`TAN(number)`

Memberikan tangen dari suatu sudut dalam radian.

- `number`: Angka yang akan diproses.

## TANH

`TANH(number)`

Memberikan tangen hiperbolik dari suatu angka.

- `number`: Angka yang akan diproses.

## TRUNC

`TRUNC(number, [num_digits])`

Memotong angka setelah sejumlah digit, tanpa pembulatan.

- `number`: Angka yang akan diproses.
- `num_digits` (opsional): Berapa digit yang dibulatkan: positif untuk sisi kanan tanda desimal, 0 untuk bilangan bulat, negatif untuk sisi kiri.
