# Fungsi tanggal dan waktu
<!-- keywords: fungsi, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC, rumus, tanggal, waktu, hari, bulan, tahun, jam, menit, detik, hari kerja, fungsi tanggal, fungsi waktu -->

Ini adalah fungsi tanggal dan waktu yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa fungsinya, dan apa arti setiap argumennya. Argumen dalam tanda kurung siku bersifat opsional, dan "..." berarti Anda dapat memberikan sebanyak mungkin argumen terakhir yang Anda perlukan. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di dalam sel, sebuah tips kecil di bawah sel menampilkan argumennya, dan klik pada nama fungsi di tips tersebut akan membuka entri fungsi ini.

## DATE

`DATE(tahun, bulan, hari)`

Membuat tanggal dari tahun, bulan, dan hari.

- `tahun`: Tahunnya, misalnya 2027.
- `bulan`: Bulannya, dari 1 sampai 12.
- `hari`: Hari dalam bulan, dari 1 sampai 31.

## DATEDIF

`DATEDIF(tanggal_mulai, tanggal_akhir, satuan)`

Memberikan jumlah tahun, bulan, atau hari di antara dua tanggal.

- `tanggal_mulai`: Tanggal periode dimulai.
- `tanggal_akhir`: Tanggal periode berakhir.
- `satuan`: Apa yang dihitung: "Y" tahun, "M" bulan, "D" hari, "MD" hari dengan mengabaikan bulan dan tahun, "YM" bulan dengan mengabaikan tahun, "YD" hari dengan mengabaikan tahun.

## DATEVALUE

`DATEVALUE(teks_tanggal)`

Mengubah tanggal yang ditulis sebagai teks menjadi tanggal.

- `teks_tanggal`: Tanggal yang ditulis sebagai teks, misalnya "3/15/2027".

## DAY

`DAY(nomor_seri)`

Memberikan hari dalam bulan dari sebuah tanggal.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.

## DAYS

`DAYS(tanggal_akhir, tanggal_mulai)`

Menghitung jumlah hari di antara dua tanggal.

- `tanggal_akhir`: Tanggal periode berakhir.
- `tanggal_mulai`: Tanggal periode dimulai.

## DAYS360

`DAYS360(tanggal_mulai, tanggal_akhir, [metode])`

Menghitung jumlah hari di antara dua tanggal dalam setahun yang terdiri dari dua belas bulan berisi 30 hari.

- `tanggal_mulai`: Tanggal periode dimulai.
- `tanggal_akhir`: Tanggal periode berakhir.
- `metode` (opsional): Cara menghitung bulan: salah atau dihilangkan untuk cara AS, benar untuk cara Eropa.

## EDATE

`EDATE(tanggal_mulai, bulan)`

Memberikan tanggal sejumlah bulan sebelum atau sesudah suatu tanggal.

- `tanggal_mulai`: Tanggal periode dimulai.
- `bulan`: Berapa bulan yang akan ditambahkan (negatif untuk mundur).

## EOMONTH

`EOMONTH(tanggal_mulai, bulan)`

Memberikan hari terakhir dari bulan sejumlah bulan sebelum atau sesudah suatu tanggal.

- `tanggal_mulai`: Tanggal periode dimulai.
- `bulan`: Berapa bulan yang akan ditambahkan (negatif untuk mundur).

## HOUR

`HOUR(nomor_seri)`

Memberikan jam dari sebuah waktu.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.

## ISOWEEKNUM

`ISOWEEKNUM(tanggal)`

Memberikan nomor minggu internasional dari sebuah tanggal.

- `tanggal`: Tanggal, atau sel yang berisi tanggal.

## MINUTE

`MINUTE(nomor_seri)`

Memberikan menit dari sebuah waktu.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.

## MONTH

`MONTH(nomor_seri)`

Memberikan bulan dari sebuah tanggal, dari 1 sampai 12.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.

## NETWORKDAYS

`NETWORKDAYS(tanggal_mulai, tanggal_akhir, [hari_libur])`

Menghitung jumlah hari kerja di antara dua tanggal.

- `tanggal_mulai`: Tanggal periode dimulai.
- `tanggal_akhir`: Tanggal periode berakhir.
- `hari_libur` (opsional): Blok sel berisi tanggal yang bukan hari kerja.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(tanggal_mulai, tanggal_akhir, [akhir_pekan], [hari_libur])`

Menghitung jumlah hari kerja di antara dua tanggal, dengan hari akhir pekan yang Anda pilih.

- `tanggal_mulai`: Tanggal periode dimulai.
- `tanggal_akhir`: Tanggal periode berakhir.
- `akhir_pekan` (opsional): Hari mana yang menjadi akhir pekan: kode dari 1 sampai 7 atau 11 sampai 17, atau tujuh karakter berisi 0 dan 1 mulai dari Senin, dengan 1 untuk hari libur.
- `hari_libur` (opsional): Blok sel berisi tanggal yang bukan hari kerja.

## NOW

`NOW()`

Memberikan tanggal dan waktu saat ini.

## SECOND

`SECOND(nomor_seri)`

Memberikan detik dari sebuah waktu.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.

## TIME

`TIME(jam, menit, detik)`

Membuat waktu dari jam, menit, dan detik.

- `jam`: Jamnya, dari 0 sampai 23.
- `menit`: Menitnya, dari 0 sampai 59.
- `detik`: Detiknya, dari 0 sampai 59.

## TIMEVALUE

`TIMEVALUE(teks_waktu)`

Mengubah waktu yang ditulis sebagai teks menjadi waktu.

- `teks_waktu`: Waktu yang ditulis sebagai teks, misalnya "2:30 PM".

## TODAY

`TODAY()`

Memberikan tanggal hari ini.

## WEEKDAY

`WEEKDAY(nomor_seri, [jenis_hasil])`

Memberikan hari dalam minggu dari sebuah tanggal sebagai angka.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.
- `jenis_hasil` (opsional): Hari apa minggu dimulai: 1 atau dihilangkan untuk Minggu, 2 untuk Senin.

## WEEKNUM

`WEEKNUM(nomor_seri, [jenis_hasil])`

Memberikan nomor minggu dalam tahun tempat sebuah tanggal jatuh.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.
- `jenis_hasil` (opsional): Hari apa minggu dimulai: 1 atau dihilangkan untuk Minggu, 2 untuk Senin.

## WORKDAY

`WORKDAY(tanggal_mulai, hari, [hari_libur])`

Memberikan tanggal sejumlah hari kerja sebelum atau sesudah suatu tanggal.

- `tanggal_mulai`: Tanggal periode dimulai.
- `hari`: Jumlah hari yang akan ditambahkan (negatif untuk mundur).
- `hari_libur` (opsional): Blok sel berisi tanggal yang bukan hari kerja.

## WORKDAY.INTL

`WORKDAY.INTL(tanggal_mulai, hari, [akhir_pekan], [hari_libur])`

Memberikan tanggal sejumlah hari kerja dari tanggal mulai, dengan hari akhir pekan yang Anda pilih.

- `tanggal_mulai`: Tanggal periode dimulai.
- `hari`: Jumlah hari yang akan ditambahkan (negatif untuk mundur).
- `akhir_pekan` (opsional): Hari mana yang menjadi akhir pekan: kode dari 1 sampai 7 atau 11 sampai 17, atau tujuh karakter berisi 0 dan 1 mulai dari Senin, dengan 1 untuk hari libur.
- `hari_libur` (opsional): Blok sel berisi tanggal yang bukan hari kerja.

## YEAR

`YEAR(nomor_seri)`

Memberikan tahun dari sebuah tanggal.

- `nomor_seri`: Tanggal atau waktu, atau sel yang berisi tanggal atau waktu.

## YEARFRAC

`YEARFRAC(tanggal_mulai, tanggal_akhir, [dasar])`

Memberikan bagian tahun di antara dua tanggal.

- `tanggal_mulai`: Tanggal periode dimulai.
- `tanggal_akhir`: Tanggal periode berakhir.
- `dasar` (opsional): Cara menghitung hari dalam setahun: 0 atau dihilangkan untuk 30/360, 1 jumlah hari sebenarnya, 2 jumlah hari sebenarnya dalam tahun 360 hari, 3 jumlah hari sebenarnya dalam tahun 365 hari, 4 30/360 Eropa.
