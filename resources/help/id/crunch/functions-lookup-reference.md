# Fungsi pencarian dan referensi
<!-- keywords: functions, ADDRESS, AREAS, COLUMN, COLUMNS, FILTER, FORMULATEXT, GETPIVOTDATA, HLOOKUP, HYPERLINK, INDEX, INDIRECT, LOOKUP, MATCH, OFFSET, ROW, ROWS, SORT, SORTBY, TRANSPOSE, UNIQUE, VLOOKUP, XLOOKUP, XMATCH -->

Berikut adalah fungsi pencarian dan referensi yang dapat dihitung oleh JeffCrunch. Setiap fungsi menampilkan cara penulisannya, apa yang dilakukannya, dan fungsi masing-masing argumennya. Argumen dalam kurung siku bersifat opsional, dan "..." berarti Anda dapat menambahkan argumen terakhir sebanyak yang Anda perlukan. Untuk mencobanya, lihat [Pilih fungsi](insert-function). Saat Anda mengetik fungsi di sebuah sel, tip kecil di bawah sel menampilkan argumennya, dan klik pada nama fungsi di dalam tip akan membuka entri fungsi ini.

## ADDRESS

`ADDRESS(row_num, column_num, [abs_num], [a1], [sheet_text])`

Menuliskan alamat sebuah sel sebagai teks, dari nomor baris dan kolomnya.

- `row_num`: Nomor baris dalam blok, dengan baris pertama dihitung sebagai 1.
- `column_num`: Nomor kolom dalam blok, dengan kolom pertama dihitung sebagai 1.
- `abs_num` (opsional): Seberapa banyak alamat dikunci: 1 seluruhnya, 2 hanya baris, 3 hanya kolom, 4 tidak ada yang dikunci.
- `a1` (opsional): Benar atau dikosongkan untuk membaca alamat sebagai huruf kolom dan nomor baris; salah untuk membacanya sebagai nomor baris dan kolom.
- `sheet_text` (opsional): Nama lembar yang akan ditempatkan di depan alamat.

## AREAS

`AREAS(reference)`

Memberikan jumlah blok sel terpisah dalam sebuah referensi.

- `reference`: Sebuah sel atau blok sel.

## COLUMN

`COLUMN([reference])`

Memberikan nomor kolom dari sebuah sel.

- `reference` (opsional): Sebuah sel atau blok sel.

## COLUMNS

`COLUMNS(array)`

Menghitung jumlah kolom dalam sebuah blok sel.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.

## FILTER

`FILTER(array, include, [if_empty])`

Memberikan baris atau kolom dari sebuah blok yang lolos suatu uji.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `include`: Sebuah kolom atau baris berisi nilai benar dan salah, satu untuk setiap baris atau kolom blok: benar berarti disimpan.
- `if_empty` (opsional): Nilai yang diberikan jika tidak ada baris atau kolom yang lolos (galat #CALC! jika dikosongkan).

## FORMULATEXT

`FORMULATEXT(reference)`

Memberikan rumus sebuah sel sebagai teks.

- `reference`: Sebuah sel atau blok sel.

## GETPIVOTDATA

`GETPIVOTDATA(data_field, pivot_table, [field1], [item1], ...)`

Memberikan nilai yang ditampilkan tabel pivot untuk sebuah nilai dan item dari baris dan kolomnya. Sebuah sel dari tabel adalah argumen kedua; bidang yang disebutkan harus merupakan bidang pertama dari baris atau kolom.

- `data_field`: Nama nilai yang akan diberikan, dalam tanda kutip, seperti yang ditampilkan tabel pivot ("Sum of Amount").
- `pivot_table`: Sel mana pun dari tabel pivot.
- `field1` (opsional): Nama sebuah bidang dari baris atau kolom, dalam tanda kutip (dapat dikosongkan bersama itemnya).
- `item1` (opsional): Item dari bidang tersebut yang nilainya akan diberikan, dalam tanda kutip.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## HLOOKUP

`HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])`

Mencari sebuah nilai di baris pertama tabel dan memberikan nilai dari kolom yang sama.

- `lookup_value`: Nilai yang dicari.
- `table_array`: Blok sel tempat pencarian dilakukan; nilai yang dicari harus berada di kolom pertamanya.
- `row_index_num`: Nomor baris tabel tempat nilai akan diambil, dengan baris pertama dihitung sebagai 1.
- `range_lookup` (opsional): Salah untuk mencari kecocokan persis; benar atau dikosongkan untuk mencari nilai terdekat di bawahnya (kolom pertama harus diurutkan naik).

## HYPERLINK

`HYPERLINK(link_location, [friendly_name])`

Membuat sel yang membuka sebuah tautan saat diklik.

- `link_location`: Tujuan tautan: alamat web, berkas, atau lokasi dalam buku kerja ini.
- `friendly_name` (opsional): Teks yang ditampilkan di sel (tautannya sendiri jika dikosongkan).

## INDEX

`INDEX(array, row_num, [column_num], [area_num])`

Memberikan nilai pada baris dan kolom tertentu dari sebuah blok sel.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `row_num`: Nomor baris dalam blok, dengan baris pertama dihitung sebagai 1.
- `column_num` (opsional): Nomor kolom dalam blok, dengan kolom pertama dihitung sebagai 1.
- `area_num` (opsional): Blok mana yang digunakan jika argumen pertama menyebutkan beberapa blok (1 untuk yang pertama).

## INDIRECT

`INDIRECT(ref_text, [a1])`

Memberikan sel yang ditunjuk oleh alamat yang ditulis sebagai teks.

- `ref_text`: Alamat yang ditulis sebagai teks, misalnya "B5".
- `a1` (opsional): Benar atau dikosongkan untuk membaca alamat sebagai huruf kolom dan nomor baris; salah untuk membacanya sebagai nomor baris dan kolom.

## LOOKUP

`LOOKUP(lookup_value, lookup_vector, [result_vector])`

Mencari sebuah nilai dalam satu baris atau kolom dan memberikan nilai pada posisi yang sama di baris atau kolom lain.

- `lookup_value`: Nilai yang dicari.
- `lookup_vector`: Satu baris atau satu kolom sel tempat pencarian dilakukan.
- `result_vector` (opsional): Satu baris atau satu kolom sel tempat hasil akan diambil.

## MATCH

`MATCH(lookup_value, lookup_array, [match_type])`

Memberikan posisi sebuah nilai dalam daftar.

- `lookup_value`: Nilai yang dicari.
- `lookup_array`: Blok sel, satu baris atau satu kolom, tempat pencarian dilakukan.
- `match_type` (opsional): 1 atau dikosongkan mencari nilai terbesar yang tidak melebihinya (daftar harus diurutkan naik); 0 mencari kecocokan persis; -1 mencari nilai terkecil yang tidak kurang darinya (daftar harus diurutkan turun).

## OFFSET

`OFFSET(reference, rows, cols, [height], [width])`

Memberikan sel-sel yang berjarak tertentu dari baris dan kolom sebuah sel awal.

- `reference`: Sebuah sel atau blok sel.
- `rows`: Berapa baris akan dipindahkan: positif ke bawah, negatif ke atas.
- `cols`: Berapa kolom akan dipindahkan: positif ke kanan, negatif ke kiri.
- `height` (opsional): Tinggi hasil dalam jumlah baris.
- `width` (opsional): Lebar hasil dalam jumlah kolom.

## ROW

`ROW([reference])`

Memberikan nomor baris dari sebuah sel.

- `reference` (opsional): Sebuah sel atau blok sel.

## ROWS

`ROWS(array)`

Menghitung jumlah baris dalam sebuah blok sel.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.

## SORT

`SORT(array, [sort_index], [sort_order], [by_col])`

Memberikan sebuah blok dengan barisnya atau kolomnya diurutkan.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `sort_index` (opsional): Nomor kolom (atau baris) yang menjadi dasar pengurutan (1 jika dikosongkan).
- `sort_order` (opsional): 1 atau dikosongkan untuk urutan naik; -1 untuk urutan turun.
- `by_col` (opsional): Benar untuk bekerja pada kolom; salah atau dikosongkan untuk bekerja pada baris.

## SORTBY

`SORTBY(array, by_array1, [sort_order1], ...)`

Memberikan sebuah blok yang diurutkan mengikuti satu atau beberapa blok lain.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `by_array1`: Blok sel yang menjadi dasar pengurutan blok pertama; panjangnya harus sama dengan blok pertama.
- `sort_order1` (opsional): 1 atau dikosongkan untuk urutan naik; -1 untuk urutan turun.
- `...`: Sebanyak tambahan yang Anda perlukan, dengan jenis yang sama seperti argumen sebelumnya.

## TRANSPOSE

`TRANSPOSE(array)`

Mengubah baris menjadi kolom dan kolom menjadi baris.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.

## UNIQUE

`UNIQUE(array, [by_col], [exactly_once])`

Memberikan baris atau kolom yang berbeda dari sebuah blok.

- `array`: Sebuah blok sel, atau daftar nilai yang ditulis dalam kurung kurawal.
- `by_col` (opsional): Benar untuk bekerja pada kolom; salah atau dikosongkan untuk bekerja pada baris.
- `exactly_once` (opsional): Benar untuk hanya memberikan item yang muncul sekali; salah atau dikosongkan untuk memberikan setiap item yang berbeda.

## VLOOKUP

`VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])`

Mencari sebuah nilai di kolom pertama tabel dan memberikan nilai dari baris yang sama.

- `lookup_value`: Nilai yang dicari.
- `table_array`: Blok sel tempat pencarian dilakukan; nilai yang dicari harus berada di kolom pertamanya.
- `col_index_num`: Nomor kolom tabel tempat nilai akan diambil, dengan kolom pertama dihitung sebagai 1.
- `range_lookup` (opsional): Salah untuk mencari kecocokan persis; benar atau dikosongkan untuk mencari nilai terdekat di bawahnya (kolom pertama harus diurutkan naik).

## XLOOKUP

`XLOOKUP(lookup_value, lookup_array, return_array, [if_not_found], [match_mode], [search_mode])`

Mencari sebuah nilai dalam daftar dan memberikan item yang cocok dari daftar lain.

- `lookup_value`: Nilai yang dicari.
- `lookup_array`: Blok sel, satu baris atau satu kolom, tempat pencarian dilakukan.
- `return_array`: Blok sel tempat hasil kecocokan akan diambil.
- `if_not_found` (opsional): Nilai yang diberikan jika tidak ada kecocokan (galat #N/A jika dikosongkan).
- `match_mode` (opsional): 0 atau dikosongkan untuk kecocokan persis, -1 untuk nilai terdekat yang lebih kecil, 1 untuk nilai terdekat yang lebih besar, 2 untuk memakai wildcard.
- `search_mode` (opsional): 1 atau dikosongkan untuk mencari dari yang pertama, -1 dari yang terakhir, 2 atau -2 untuk pencarian biner pada daftar yang diurutkan.

## XMATCH

`XMATCH(lookup_value, lookup_array, [match_mode], [search_mode])`

Memberikan posisi sebuah nilai dalam daftar, dengan pilihan cara pencocokan dan arah pencarian.

- `lookup_value`: Nilai yang dicari.
- `lookup_array`: Blok sel, satu baris atau satu kolom, tempat pencarian dilakukan.
- `match_mode` (opsional): 0 atau dikosongkan untuk kecocokan persis, -1 untuk nilai terdekat yang lebih kecil, 1 untuk nilai terdekat yang lebih besar, 2 untuk memakai wildcard.
- `search_mode` (opsional): 1 atau dikosongkan untuk mencari dari yang pertama, -1 dari yang terakhir, 2 atau -2 untuk pencarian biner pada daftar yang diurutkan.
