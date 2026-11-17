# Fungsi informasi
<!-- keywords: fungsi, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE -->

Berikut adalah fungsi informasi yang dapat dihitung JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa yang dilakukannya, dan fungsi setiap argumennya. Argumen dalam kurung siku bersifat opsional, dan "..." berarti Anda dapat memberikan sebanyak yang Anda perlukan dari argumen terakhir. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di dalam sel, sebuah tips kecil di bawah sel menampilkan argumennya, dan jika Anda mengklik nama fungsi di dalam tips tersebut, entri fungsi ini akan terbuka.

## CELL

`CELL(info_type, [reference])`

Memberikan informasi tentang sebuah sel, seperti alamat atau formatnya.

- `info_type`: Informasi yang ingin ditampilkan tentang sel, sebagai teks seperti "address", "row", "col", atau "format".
- `reference` (opsional): Sel yang ingin ditampilkan informasinya (sel dengan kursor jika argumen ini dihilangkan).

## ERROR.TYPE

`ERROR.TYPE(error_value)`

Memberikan angka yang mewakili jenis galat.

- `error_value`: Galat yang akan dicari, atau sel yang berisi galat tersebut.

## INFO

`INFO(info_type)`

Memberikan informasi tentang program atau buku kerja, seperti foldernya atau cara penghitungannya.

- `info_type`: Informasi yang ingin ditampilkan tentang sel, sebagai teks seperti "address", "row", "col", atau "format".

## ISBLANK

`ISBLANK(value)`

Benar jika sel kosong.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISERR

`ISERR(value)`

Benar jika nilai berupa galat selain tidak tersedia.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISERROR

`ISERROR(value)`

Benar jika nilai berupa galat apa pun.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISEVEN

`ISEVEN(number)`

Benar jika bilangan bulat genap.

- `number`: Angka yang akan diproses.

## ISFORMULA

`ISFORMULA(reference)`

Benar jika sel berisi rumus.

- `reference`: Sebuah sel atau blok sel.

## ISLOGICAL

`ISLOGICAL(value)`

Benar jika nilai berupa benar atau salah.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISNA

`ISNA(value)`

Benar jika nilai berupa galat tidak tersedia.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISNONTEXT

`ISNONTEXT(value)`

Benar jika nilai bukan teks.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISNUMBER

`ISNUMBER(value)`

Benar jika nilai berupa angka.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISODD

`ISODD(number)`

Benar jika bilangan bulat ganjil.

- `number`: Angka yang akan diproses.

## ISREF

`ISREF(value)`

Benar jika nilai berupa referensi ke sel.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## ISTEXT

`ISTEXT(value)`

Benar jika nilai berupa teks.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## N

`N(value)`

Mengubah nilai menjadi angka: tanggal dan nilai benar menjadi angka, sedangkan teks menjadi nol.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## NA

`NA()`

Memberikan galat tidak tersedia, untuk menandai nilai yang hilang.

## SHEET

`SHEET([value])`

Memberikan nomor sebuah lembar, dengan menghitung lembar pertama sebagai 1.

- `value` (opsional): Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## SHEETS

`SHEETS([reference])`

Memberikan jumlah lembar dalam sebuah referensi, atau dalam buku kerja.

- `reference` (opsional): Sebuah sel atau blok sel.

## TYPE

`TYPE(value)`

Memberikan angka yang menunjukkan jenis nilai: angka, teks, logika, galat, atau larik.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.
