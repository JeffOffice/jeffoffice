# Fungsi teks
<!-- keywords: fungsi, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE -->

Ini adalah fungsi teks yang dapat dihitung JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa fungsinya, dan arti setiap argumennya. Argumen dalam kurung siku bersifat opsional, dan "..." berarti Anda dapat memberikan sebanyak yang diperlukan untuk argumen terakhir. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di dalam sel, sebuah tip kecil di bawah sel menampilkan argumennya, dan klik pada nama fungsi di dalam tip akan membuka entrinya di sini.

## ASC

`ASC(text)`

Mengubah huruf dan angka lebar penuh dalam teks menjadi lebar setengah.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## BAHTTEXT

`BAHTTEXT(number)`

Menuliskan angka sebagai kata-kata Thai dengan kata untuk baht.

- `number`: Angka yang akan diproses.

## CHAR

`CHAR(number)`

Memberikan karakter yang memiliki nomor kode tertentu.

- `number`: Angka yang akan diproses.

## CLEAN

`CLEAN(text)`

Menghapus karakter yang tidak tercetak dari sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## CODE

`CODE(text)`

Memberikan nomor kode dari karakter pertama sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## CONCAT

`CONCAT(text1, ...)`

Menggabungkan teks dan teks dalam blok sel menjadi satu.

- `text1`: Teks pertama, atau blok sel berisi teks.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## CONCATENATE

`CONCATENATE(text1, ...)`

Menggabungkan teks menjadi satu.

- `text1`: Teks pertama, atau blok sel berisi teks.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## DBCS

`DBCS(text)`

Mengubah huruf dan angka lebar setengah dalam teks menjadi lebar penuh.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## DOLLAR

`DOLLAR(number, [decimals])`

Mengubah angka menjadi teks sebagai jumlah uang.

- `number`: Angka yang akan diproses.
- `decimals` (opsional): Berapa digit yang disimpan setelah tanda desimal.

## ENCODEURL

`ENCODEURL(text)`

Mengubah teks agar dapat digunakan dalam alamat web.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## EXACT

`EXACT(text1, text2)`

Bernilai benar jika dua teks persis sama, termasuk huruf kapitalnya.

- `text1`: Teks pertama, atau blok sel berisi teks.
- `text2`: Teks kedua.

## FILTERXML

`FILTERXML(xml, xpath)`

Mengambil nilai dari teks yang berisi XML, menggunakan ekspresi XPath.

- `xml`: Teks yang berisi XML.
- `xpath`: XPath yang menentukan bagian mana yang diambil, misalnya //item/price.

## FIND

`FIND(find_text, within_text, [start_num])`

Memberikan posisi awal sebuah teks di dalam teks lain, dengan membedakan huruf kapital dan huruf kecil.

- `find_text`: Teks yang akan dicari.
- `within_text`: Teks tempat pencarian dilakukan.
- `start_num` (opsional): Posisi dalam teks tempat pencarian dimulai, dengan karakter pertama dihitung sebagai 1.

## FINDB

`FINDB(find_text, within_text, [start_num])`

Memberikan posisi pertama kemunculan teks di dalam teks lain, dengan membedakan huruf kapital.

- `find_text`: Teks yang akan dicari.
- `within_text`: Teks tempat pencarian dilakukan.
- `start_num` (opsional): Posisi dalam teks tempat pencarian dimulai, dengan karakter pertama dihitung sebagai 1.

## FIXED

`FIXED(number, [decimals], [no_commas])`

Mengubah angka menjadi teks dengan jumlah desimal yang ditentukan.

- `number`: Angka yang akan diproses.
- `decimals` (opsional): Berapa digit yang disimpan setelah tanda desimal.
- `no_commas` (opsional): Benar untuk menghilangkan tanda koma yang memisahkan ribuan.

## LEFT

`LEFT(text, [num_chars])`

Memberikan karakter pertama dari sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `num_chars` (opsional): Berapa karakter yang diambil (1 jika dihilangkan).

## LEFTB

`LEFTB(text, [num_chars])`

Memberikan karakter pertama dari teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `num_chars` (opsional): Berapa karakter yang diambil (1 jika dihilangkan).

## LEN

`LEN(text)`

Menghitung karakter dalam sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## LENB

`LENB(text)`

Memberikan jumlah karakter dalam teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## LOWER

`LOWER(text)`

Mengubah teks menjadi huruf kecil.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## MID

`MID(text, start_num, num_chars)`

Memberikan karakter dari bagian tengah sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `start_num`: Posisi dalam teks tempat pengambilan dimulai, dengan karakter pertama dihitung sebagai 1.
- `num_chars`: Berapa karakter yang diambil (1 jika dihilangkan).

## MIDB

`MIDB(text, start_num, num_chars)`

Memberikan karakter dari bagian tengah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `start_num`: Posisi dalam teks tempat pengambilan dimulai, dengan karakter pertama dihitung sebagai 1.
- `num_chars`: Berapa karakter yang diambil (1 jika dihilangkan).

## NUMBERVALUE

`NUMBERVALUE(text, [decimal_separator], [group_separator])`

Membaca teks sebagai angka, dengan pemisah yang Anda tentukan.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `decimal_separator` (opsional): Karakter yang menandai desimal dalam teks (tanda titik jika dihilangkan).
- `group_separator` (opsional): Karakter yang mengelompokkan digit dalam teks (tanda koma jika dihilangkan).

## PHONETIC

`PHONETIC(reference)`

Memberikan teks dari sebuah sel, untuk pembacaan teks Jepang.

- `reference`: Sebuah sel atau blok sel.

## PROPER

`PROPER(text)`

Memberikan setiap kata dalam sebuah teks huruf awal kapital.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## REPLACE

`REPLACE(old_text, start_num, num_chars, new_text)`

Menempatkan teks baru menggantikan sebagian karakter dalam sebuah teks.

- `old_text`: Teks yang akan diubah.
- `start_num`: Posisi dalam teks tempat penggantian dimulai, dengan karakter pertama dihitung sebagai 1.
- `num_chars`: Berapa karakter yang diambil (1 jika dihilangkan).
- `new_text`: Teks yang akan dimasukkan.

## REPLACEB

`REPLACEB(old_text, start_num, num_chars, new_text)`

Mengganti sebagian teks dengan teks lain.

- `old_text`: Teks yang akan diubah.
- `start_num`: Posisi dalam teks tempat penggantian dimulai, dengan karakter pertama dihitung sebagai 1.
- `num_chars`: Berapa karakter yang diambil (1 jika dihilangkan).
- `new_text`: Teks yang akan dimasukkan.

## REPT

`REPT(text, number_times)`

Mengulang sebuah teks sejumlah kali tertentu.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `number_times`: Berapa kali teks akan diulang.

## RIGHT

`RIGHT(text, [num_chars])`

Memberikan karakter terakhir dari sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `num_chars` (opsional): Berapa karakter yang diambil (1 jika dihilangkan).

## RIGHTB

`RIGHTB(text, [num_chars])`

Memberikan karakter terakhir dari teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `num_chars` (opsional): Berapa karakter yang diambil (1 jika dihilangkan).

## SEARCH

`SEARCH(find_text, within_text, [start_num])`

Memberikan posisi awal sebuah teks di dalam teks lain, tanpa membedakan huruf kapital, dan mengizinkan ? dan * dalam teks yang dicari.

- `find_text`: Teks yang akan dicari.
- `within_text`: Teks tempat pencarian dilakukan.
- `start_num` (opsional): Posisi dalam teks tempat pencarian dimulai, dengan karakter pertama dihitung sebagai 1.

## SEARCHB

`SEARCHB(find_text, within_text, [start_num])`

Memberikan posisi pertama kemunculan teks di dalam teks lain, tanpa membedakan huruf kapital.

- `find_text`: Teks yang akan dicari.
- `within_text`: Teks tempat pencarian dilakukan.
- `start_num` (opsional): Posisi dalam teks tempat pencarian dimulai, dengan karakter pertama dihitung sebagai 1.

## SUBSTITUTE

`SUBSTITUTE(text, old_text, new_text, [instance_num])`

Menukar satu bagian teks dengan teks lain di mana pun teks itu muncul.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
- `old_text`: Teks yang akan dicari.
- `new_text`: Teks yang akan dimasukkan.
- `instance_num` (opsional): Kemunculan yang mana yang akan diganti (semuanya jika dihilangkan).

## T

`T(value)`

Memberikan teks itu sendiri, dan kosong untuk apa pun yang bukan teks.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.

## TEXT

`TEXT(value, format_text)`

Mengubah angka menjadi teks dalam format yang Anda pilih.

- `value`: Nilai yang akan diproses, atau sel yang berisi nilai tersebut.
- `format_text`: Format angka untuk menampilkan nilai, misalnya "0,00" atau "dd/mm/yyyy".

## TEXTJOIN

`TEXTJOIN(delimiter, ignore_empty, text1, ...)`

Menggabungkan teks dengan pemisah di antaranya.

- `delimiter`: Teks yang diletakkan di antara setiap bagian.
- `ignore_empty`: Benar untuk menghilangkan bagian yang kosong; salah untuk mempertahankannya.
- `text1`: Teks pertama, atau blok sel berisi teks.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## TRIM

`TRIM(text)`

Menghapus spasi berlebih dari sebuah teks, dan menyisakan satu spasi di antara setiap kata.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## UNICHAR

`UNICHAR(number)`

Memberikan karakter yang sesuai dengan nomor Unicode.

- `number`: Angka yang akan diproses.

## UNICODE

`UNICODE(text)`

Memberikan nomor Unicode dari karakter pertama sebuah teks.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## UPPER

`UPPER(text)`

Mengubah teks menjadi huruf kapital.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.

## VALUE

`VALUE(text)`

Mengubah teks yang menyerupai angka menjadi angka.

- `text`: Teks yang akan diproses, atau sel yang berisi teks tersebut.
