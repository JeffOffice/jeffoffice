# Fungsi logika
<!-- keywords: functions, AND, CHOOSE, FALSE, IF, IFERROR, IFNA, IFS, LET, NOT, OR, SWITCH, TRUE, XOR -->

Berikut adalah fungsi logika yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa yang dilakukannya, dan apa arti setiap argumennya. Argumen dalam kurung siku bersifat opsional, dan "..." berarti Anda dapat menambahkan sebanyak yang diperlukan dari argumen terakhir. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di dalam sel, sebuah tip kecil di bawah sel menampilkan argumennya, dan jika Anda mengklik nama fungsi pada tip tersebut, entri fungsi ini akan terbuka.

## AND

`AND(logical1, ...)`

Bernilai benar jika setiap kondisi bernilai benar.

- `logical1`: Nilai atau uji pertama, yang bernilai benar atau salah.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## CHOOSE

`CHOOSE(index, value1, ...)`

Memilih nilai dalam daftar yang berada di posisi yang ditunjukkan oleh indeks.

- `index`: Nilai mana yang akan diberikan, dengan menghitung yang pertama sebagai 1.
- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## FALSE

`FALSE()`

Nilai logika salah.

## IF

`IF(condition, value_if_true, [value_if_false])`

Memberikan satu nilai jika suatu kondisi terpenuhi dan nilai lain jika tidak.

- `condition`: Nilai atau uji yang bernilai benar atau salah.
- `value_if_true`: Nilai yang diberikan jika kondisi bernilai benar.
- `value_if_false` (opsional): Nilai yang diberikan jika kondisi bernilai salah (FALSE jika dihilangkan).

## IFERROR

`IFERROR(value, value_if_error)`

Memberikan suatu nilai, atau pengganti jika nilai tersebut berupa galat.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.
- `value_if_error`: Nilai yang diberikan jika nilai tersebut berupa galat.

## IFNA

`IFNA(value, value_if_na)`

Memberikan suatu nilai, atau pengganti jika nilai tersebut berupa galat tidak tersedia.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.
- `value_if_na`: Nilai yang diberikan jika nilai tersebut berupa galat tidak tersedia.

## IFS

`IFS(condition1, value1, ...)`

Menguji kondisi secara berurutan dan memberikan nilai yang sesuai dengan kondisi pertama yang terpenuhi.

- `condition1`: Uji pertama, yang bernilai benar atau salah.
- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## LET

`LET(name1, value1, ...)`

Memberi nama pada nilai di dalam satu rumus, lalu menghitung perhitungan yang menggunakan nama tersebut.

- `name1`: Nama yang diberikan kepada nilai yang mengikutinya. Nama ini dapat dipakai di nilai-nilai setelahnya dan di dalam perhitungan.
- `value1`: Nilai pertama, atau blok sel.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## NOT

`NOT(logical)`

Mengubah benar menjadi salah dan salah menjadi benar.

- `logical`: Nilai atau uji yang bernilai benar atau salah.

## OR

`OR(logical1, ...)`

Bernilai benar jika sedikitnya satu kondisi bernilai benar.

- `logical1`: Nilai atau uji pertama, yang bernilai benar atau salah.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## SWITCH

`SWITCH(expression, value1, result1, ...)`

Membandingkan suatu nilai dengan daftar nilai dan memberikan hasil yang sesuai dengan kecocokan pertama.

- `expression`: Nilai yang akan dibandingkan dengan nilai-nilai setelahnya.
- `value1`: Nilai pertama, atau blok sel.
- `result1`: Hasil yang diberikan ketika nilai sebelumnya cocok.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.

## TRUE

`TRUE()`

Nilai logika benar.

## XOR

`XOR(logical1, ...)`

Bernilai benar jika jumlah kondisi yang bernilai benar ganjil.

- `logical1`: Nilai atau uji pertama, yang bernilai benar atau salah.
- `...`: Sebanyak yang diperlukan lagi, dengan jenis yang sama seperti argumen sebelumnya.
