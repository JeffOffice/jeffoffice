# Tabel pivot
<!-- keywords: file lain, buku kerja lain, sumber eksternal, rumus daftar, bidang terhitung, item terhitung, urutan penyelesaian, bidang rumus, halaman filter laporan, tabel pivot, telusuri detail, telusuri ringkasan, sisipkan tabel pivot, buat, pilih, pindahkan tabel pivot, getpivotdata, buat getpivotdata, segarkan, segarkan semua, ubah sumber data, daftar bidang, bidang, baris, kolom, nilai, filter, filter laporan, total keseluruhan, subtotal, tata letak, ringkas, garis besar, tabular, hapus, pengaturan bidang nilai, jumlah, hitung, rata-rata, tampilkan nilai sebagai, persen dari total, selisih, total berjalan, format angka, pengaturan bidang, urutkan, filter, teratas, kelompokkan, pisahkan kelompok, tanggal, bulan, tahun, perluas, ciutkan, opsi, gaya, baris berpita, baris kosong, ulangi label, segarkan saat file dibuka, bagan pivot, pengiris, linimasa, tampilkan detail, telusuri detail, baris di balik nilai -->

Tabel pivot adalah laporan kecil yang dibuat dari daftar panjang. Daftar itu memiliki satu baris untuk setiap penjualan, misalnya; laporan menjumlahkan penjualan menurut wilayah dan bulan. Anda tidak perlu menulis rumus apa pun. Anda menentukan kolom mana dari daftar yang diletakkan di samping, kolom mana yang diletakkan di atas, dan mana yang dijumlahkan, lalu laporan terisi dengan sendirinya.

## Contoh

Berikut daftar di sel A1 sampai C7. Setiap baris adalah satu penjualan.

| Wilayah | Bulan | Jumlah |
|---|---|---|
| Timur | Jan | 10 |
| Timur | Jan | 5 |
| Timur | Feb | 20 |
| Barat | Jan | 7 |
| Barat | Feb | 3 |
| Barat | Feb | 4 |

Tabel pivot dengan **Wilayah** di samping, **Bulan** di atas, dan jumlah **Jumlah** di tengah terlihat seperti ini.

| Jumlah dari Jumlah | Bulan | | |
|---|---|---|---|
| Wilayah | Jan | Feb | Total Keseluruhan |
| Timur | 15 | 20 | 35 |
| Barat | 7 | 7 | 14 |
| Total Keseluruhan | 22 | 27 | 49 |

Timur pada Januari bernilai 15 karena dua baris Timur untuk Januari, 10 dan 5, dijumlahkan. Kolom Total Keseluruhan menjumlahkan setiap baris, baris Total Keseluruhan menjumlahkan setiap kolom, dan sel di pojok menjumlahkan semuanya.

Istilah yang dipakai di sini:

- **Bidang** adalah kolom dari daftar. Wilayah, Bulan, dan Jumlah adalah bidang.
- **Baris**, **Kolom**, dan **Nilai** adalah tiga tempat bidang dapat diletakkan. Bidang di Baris memberi satu baris untuk setiap entri yang berbeda; bidang di Kolom memberi satu kolom untuk setiap entri; bidang di Nilai adalah yang dijumlahkan. Tempat keempat, **Filter**, berisi bidang yang menyaring baris yang tidak Anda pilih.
- **Cache** (cache pivot) adalah salinan daftar Anda yang disimpan tabel pivot di dalam berkas, sehingga dapat menampilkan total tanpa membaca sel. Laporan dihitung dari cache, bukan dari sel, sehingga tidak berubah ketika sel berubah. **Segarkan** menyalin ulang sel ke dalam cache.

## Membuat tabel pivot

1. Klik sel di dalam daftar, atau pilih sel yang ingin Anda gunakan. Daftar memerlukan baris nama di atas datanya.
2. Pilih **Sisipkan**, lalu **Tabel Pivot**.
3. Kotak menampilkan sel tempat data berada. Ubah jika belum sesuai. Anda juga dapat mengetik nama tabel. Untuk memakai daftar yang ada di berkas lain, pilih **Rentang dalam berkas lain**, pilih berkasnya dengan **Telusuri**, lalu ketik rentangnya, atau nama tabel atau nama yang Anda buat di berkas itu, misalnya `Sheet1!A1:D100`. JeffCrunch membaca buku kerja (`.xlsx`, `.xlsm`, `.xls`, dan `.ods`) serta berkas `.csv`.
4. Pilih tempat tabel pivot: **Lembar baru** (bawaan), atau sel di lembar ini atau lembar lain. Pilih tempat yang kosong, karena tabel kosong menempati sekitar delapan belas baris dan tiga kolom.
5. Pilih **OK**. Tabel mulai kosong dan panel **Bidang Tabel Pivot** terbuka. Centang bidang yang Anda inginkan: bidang berisi angka akan dijumlahkan, dan bidang lain menjadi baris. Kemudian pindahkan bidang ke kotak seperti dijelaskan pada bagian berikut.

Ketika data berada di berkas lain, tabel pivot menyimpan salinannya, sehingga tabel tetap berfungsi dan tersimpan tanpa berkas itu. Lihat Data di berkas lain di bawah.

Lembar baru adalah satu langkah Urungkan tersendiri, begitu pula tabelnya. Saat Anda menyimpan sebagai .xlsx, tabel ditulis sebagaimana program aslinya menulisnya, sehingga terbuka di sana sebagai tabel pivot. Salinan yang disimpan sebagai .xls atau .ods menyimpan angka yang ditampilkan tabel di selnya, tetapi tidak menyimpan tabelnya, dan penyimpanan akan memberi tahu Anda.

## GETPIVOTDATA: membaca satu angka dari tabel pivot

`GETPIVOTDATA` memberikan angka yang ditampilkan tabel pivot untuk nilai, baris, dan kolom yang Anda sebutkan. Untuk contoh di atas:

`=GETPIVOTDATA("Jumlah dari Jumlah", $E$3, "Wilayah", "Timur", "Bulan", "Jan")` memberikan 15.

- Argumen pertama adalah nama nilai, seperti yang ditampilkan tabel (misalnya Jumlah dari Jumlah). Nama bidang yang dijumlahkan (Jumlah) juga berlaku.
- Argumen kedua adalah sel mana pun dari tabel pivot.
- Setelah itu ada pasangan bidang dan salah satu itemnya. Jika hanya Wilayah dan Timur yang disebutkan, Anda mendapat total Timur, 35. Jika tidak ada yang disebutkan, Anda mendapat total keseluruhan, 49. Bidang yang Anda sebutkan harus merupakan bidang pertama di baris, atau bidang pertama di kolom, sesuai urutannya.
- Jika tabel tidak menampilkan angka itu, sel menampilkan `#REF!`.

Ketika Anda mengklik nilai tabel pivot saat mengetik rumus, JeffCrunch menuliskan rumus ini untuk Anda, sehingga tetap membaca angka yang benar setelah tabel diatur ulang. Untuk mematikannya, pilih **Opsi** pada tab **Tabel Pivot** dan hapus centang kotak tentang GETPIVOTDATA.

## Yang dilakukan JeffCrunch terhadap tabel pivot

Tabel pivot dalam buku kerja ditampilkan seperti saat disimpan. Jika Anda tidak mengubah apa pun, berkas disimpan dengan tabel pivot persis seperti semula. Ketika Anda menyegarkannya atau mengubahnya, JeffCrunch menghitung ulang laporan, memasukkan angka baru ke dalam sel, dan menulis kembali tabel pivot agar program yang membuatnya dapat membukanya tanpa keluhan.

JeffCrunch dapat menghitung tabel pivot yang menjumlahkan, menghitung, merata-ratakan, dan sebagainya, yang mengelompokkan tanggal dan angka, serta yang menampilkan nilai sebagai persentase, selisih, atau total berjalan. Tabel pivot yang menampilkan nilai sebagai peringkat atau sebagai persentase dari induk, memiliki kelompok item yang dibuat secara manual, atau memakai model data ditampilkan dan disimpan seperti aslinya, dan perintah akan memberi tahu Anda bahwa tabel itu tidak dapat diubah.

Klik sel mana pun di tabel pivot. Tab **Tabel Pivot** akan muncul.

## Segarkan

**Segarkan** membaca sel tempat tabel pivot dibuat dan menghitung ulang laporan. Entri baru ditambahkan setelah entri yang sudah ada; entri yang sudah tidak ada akan dibuang. Lebar kolom dan tata letak yang Anda pilih tetap dipertahankan. **Segarkan Semua**, pada tab **Data** dan pada tab **Tabel Pivot**, melakukan hal yang sama untuk semua tabel pivot dalam buku kerja.

Jika laporan akan melebar ke atas sel yang sudah berisi, JeffCrunch tidak menyegarkan. Ia akan memberi tahu Anda sel mana yang menghalangi. Kosongkan sel tersebut atau pindahkan tabel pivot, lalu segarkan lagi.

**Urungkan** membatalkan penyegaran dalam satu langkah.

## Ubah Sumber Data

**Ubah Sumber Data** meminta sel tempat daftar berada, termasuk baris pertama berisi nama. Ketik seperti `Sheet1!A1:D100`, atau ketik nama tabel atau nama yang Anda buat. JeffCrunch membacanya dan menghitung ulang laporan. Jika sebuah bidang daftar memiliki nama yang sama di sel baru, posisinya di laporan tetap dipertahankan.

Anda juga dapat mengarahkan tabel ke rentang di berkas lain. Di kotak, pilih **Rentang dalam berkas lain**, pilih berkasnya, lalu ketik rentangnya di dalamnya. Memilih **Rentang dalam buku kerja ini** mengarahkan tabel kembali ke sel di sini. Setiap perubahan tersebut adalah satu langkah Urungkan.

## Data di berkas lain

Tabel pivot yang dibuat dari berkas lain menyimpan data asalnya di dalam buku kerja ini, sehingga Anda dapat membuka, mengubah, dan menyimpan buku kerja ketika berkas lain sudah tidak ada. **Segarkan** membaca ulang berkas lain. Jika berkas telah dipindahkan, hilang, terlalu besar, bukan buku kerja, atau tidak lagi memiliki rentang tersebut, tabel tetap menyimpan data yang dimilikinya, dan sebuah pesan memberi tahu bahwa berkas tidak dapat dibaca. Berkas yang berada di komputer lain dalam jaringan Anda tidak dibaca sampai Anda sendiri memilihnya di **Ubah Sumber Data**. Satu penyegaran membaca setiap berkas sekali, dan paling banyak 32 berkas, 1 GB, dan 30 detik secara keseluruhan; tabel yang melebihi batas itu tetap menyimpan datanya.

Jika sebuah buku kerja menyatakan bahwa tabelnya harus dibaca ulang saat dibuka, dan data tabel berada di berkas lain, JeffCrunch akan bertanya terlebih dahulu, dengan cara yang sama seperti untuk tautan ke buku kerja lain. Sampai Anda menjawab ya, tidak ada berkas yang dibaca.

Buku kerja yang disimpan sebagai .xlsx menyimpan lokasi berkas. Memilih berkas di kotak akan menyimpan alamat lengkapnya. Buku kerja yang dibuat program lain, yang tabelnya mengarah ke berkas dengan jalur pendek seperti `Prices.xlsx`, tetap mempertahankan jalur itu, dan JeffCrunch mencari berkas di folder yang sama dengan buku kerja. Ketika Anda memakai **Simpan Sebagai** untuk menyimpan buku kerja di folder lain, jalur seperti itu ditulis ulang agar tetap menunjuk berkas yang sama. Penyegaran tidak mengikuti tautan apa pun yang dimiliki berkas lain ke berkas lainnya.

## Hapus

**Hapus** memiliki dua pilihan. **Hapus Filter** menghapus semua filter dan menampilkan kembali semua entri. **Hapus Semua** mengeluarkan setiap bidang dari laporan dan membiarkannya kosong, siap dibangun kembali.

## Telusuri detail dan telusuri ringkasan

## Halaman Filter Laporan

Filter berada di atas tabel pivot dan memungkinkan Anda melihat satu item pada satu waktu, misalnya satu bulan. **Tampilkan Halaman Filter Laporan** pada tab **Tabel Pivot** membuat satu halaman untuk setiap item sekaligus. Setiap halaman adalah lembar baru, diberi nama sesuai item, dengan salinan tabelnya sendiri. Filter pada setiap salinan diatur ke item tersebut. Jika tabel memiliki lebih dari satu filter, Anda memilih filter yang akan dipakai. Satu **Urungkan** menghapus semua halaman itu.

Ketika sebuah laporan memiliki lebih dari satu bidang di barisnya, item dari bidang pertama dapat dibuka atau ditutup. Klik sel berisi nama item dan pilih **Telusuri Ringkasan** untuk menutupnya: baris bidang berikutnya di bawahnya disembunyikan dan hanya totalnya yang tersisa. **Telusuri Detail** membukanya kembali. Item dari bidang terakhir tidak memiliki apa pun di bawahnya. Setiap tindakan adalah satu **Urungkan**.

## Pilih dan pindahkan

**Pilih** pada tab **Tabel Pivot** memilih sebagian laporan: **Seluruh Tabel Pivot**, **Nilai** (angkanya), atau **Label** (nama di samping; jika laporan tidak memiliki nama di samping, nama di atas). **Pindahkan Tabel Pivot…** meminta sel pada lembar yang sama dan menggambar ulang laporan dengan sel pertamanya di sana. Sel yang ditinggalkan akan dikosongkan. Rumus yang menunjuk ke sel laporan akan mengikutinya, seperti saat Anda memotong dan menempel. Jika tempat baru sudah berisi sesuatu, JeffCrunch tidak memindahkan laporan dan memberi tahu Anda. Satu **Urungkan** memindahkannya kembali.

## Panel bidang

**Daftar Bidang** pada tab **Tabel Pivot** membuka panel **Bidang Tabel Pivot**. Panel ini mencantumkan bidang dari daftar dengan kotak di sebelahnya, dan di bawahnya empat tempat: **Filter**, **Kolom**, **Baris**, dan **Nilai**.

- Centang bidang untuk menambahkannya. Bidang yang hanya berisi angka masuk ke **Nilai** dan dijumlahkan; yang lain masuk ke **Baris**.
- Hapus centang bidang untuk mengeluarkannya dari semua tempat.
- Klik bidang di salah satu dari empat tempat, lalu pilih **Pindahkan Ke** untuk memasukkannya ke tempat lain. Kata **Pindah ke Atas** dan **Pindah ke Bawah** mengubah urutannya di tempatnya, dan **Hapus Bidang** mengeluarkannya.
- Jika ada lebih dari satu nilai, baris bernama **Nilai** muncul di **Kolom**. Pindahkan ke **Baris** untuk meletakkan nilai di samping.
- **Pengaturan Bidang Nilai** (klik ganda pada nilai, atau tombolnya) mengatur nama yang ditampilkan laporan untuk nilai dan cara penjumlahannya: **Jumlah**, **Hitung**, **Hitung Angka**, **Rata-rata**, **Maks**, **Min**, **Hasil Kali**, **StdDev** dan **StdDevp** (simpangan baku sampel, atau dari semua nilai), serta **Var** dan **Varp** (varians dari keduanya). Pengaturan ini juga mengatur cara nilai ditampilkan dan format angkanya. Lihat bagian berikutnya.
- **Pengaturan Bidang** (klik ganda pada bidang di Filter, Kolom, atau Baris, atau tombolnya) mengatur satu bidang. Lihat "Pengaturan Bidang".

Setiap perubahan adalah satu langkah **Urungkan**.

## Tampilkan Nilai Sebagai, dan format angka nilai

Di **Pengaturan Bidang Nilai**, **Tampilkan nilai sebagai** mengubah apa yang ditampilkan sel laporan.

- **% dari Total Keseluruhan**: sel dibagi total seluruh laporan. Dalam contoh, Timur pada Januari adalah 15 dari 49, yaitu 30,6%.
- **% dari Total Kolom** dan **% dari Total Baris**: sel dibagi total kolomnya, atau total barisnya.
- **Indeks**: perbandingan sel dengan nilai yang akan dihasilkan baris dan kolomnya jika data tersebar merata. Angka di atas 1 berarti lebih dari bagian yang merata.
- **Selisih Dari**, **% Dari**, dan **% Selisih Dari**: sel dibandingkan dengan item lain dari suatu bidang. Pilih **Bidang dasar** (bidang di Baris atau Kolom) dan **Item dasar** (salah satu itemnya, atau item sebelumnya atau sesudahnya). Misalnya, dengan Bulan sebagai bidang dasar dan Jan sebagai item dasar, Februari menunjukkan seberapa besar lebih atau kurang dibanding Januari. Item dasar itu sendiri dikosongkan untuk selisih, dan bernilai 100% untuk **% Dari**.
- **Total Berjalan Dalam**: setiap sel menjumlahkan sel-sel sebelumnya sepanjang bidang dasar.
- **Peringkat dari Terkecil ke Terbesar** dan **Peringkat dari Terbesar ke Terkecil**: setiap sel menampilkan posisinya, 1, 2, 3, dan seterusnya, di antara sel-sel bidang dasar pada baris atau kolom yang sama. Peringkat 1 adalah angka terkecil, atau terbesar.
- **% dari Total Baris Induk** dan **% dari Total Kolom Induk**: setiap sel menampilkan bagian mana dari induknya. Induk suatu item adalah item satu tingkat di atasnya, seperti tahun yang memuat sebuah bulan. Item tanpa induk ditampilkan sebagai bagian dari total keseluruhan.

**Format angka** menerima kode format, seperti `0.00`, `#,##0`, atau `0.0%`. Kosongkan untuk mempertahankan format yang dimiliki sel. Nilai yang ditampilkan sebagai persentase mendapat `0.00%` kecuali Anda memilih format. Sel laporan langsung memakai format itu. **Urungkan** mengembalikannya bersama angkanya.

## Pengaturan Bidang

Pilih bidang di kotak **Filter**, **Kolom**, atau **Baris**, lalu pilih **Pengaturan Bidang** pada tab **Tabel Pivot** (atau klik ganda).

- **Nama kustom** mengubah nama yang ditampilkan laporan untuk bidang itu.
- **Subtotal**: **Otomatis** menampilkan total untuk setiap kelompok bidang, **Tidak ada** tidak menampilkannya.
- **Tampilkan label item dalam**: bentuk ringkas, garis besar, atau tabel, seperti pada Tata Letak Laporan.
- **Sisipkan baris kosong setelah setiap item** meletakkan baris kosong setelah setiap kelompok.
- **Ulangi label item** menampilkan nama item luar di setiap baris kelompoknya (bentuk garis besar dan tabel).
- **Urutkan**: pertahankan urutan item (manual), atau urutkan A ke Z atau Z ke A menurut label. **Terkecil ke terbesar menurut nilai** dan **Terbesar ke terkecil menurut nilai** mengurutkan item menurut angka pada bidang nilai. Laporan mempertahankan urutan saat data berubah.
- **Filter** menyaring item berdasarkan labelnya (diawali dengan, diakhiri dengan, berisi, sama dengan) atau berdasarkan nilai (lebih besar dari, lebih kecil dari, di antara, dan seterusnya), atau hanya mempertahankan beberapa item teratas atau terbawah menurut nilai. Filter dihitung ketika Anda memilih OK. Filter menyembunyikan item yang disaringnya; kotak centang di bawah menunjukkan item mana, dan Anda dapat mencentang item untuk menampilkannya kembali.
- **Tampilkan item ini** adalah daftar item bidang dengan satu kotak centang masing-masing. Hapus centang untuk menyembunyikan item, dan total tidak menyertakannya.

## Kelompokkan tanggal dan angka

Pilih bidang berisi tanggal atau angka di **Baris** atau **Kolom**, lalu pilih **Kelompokkan Bidang** pada tab **Tabel Pivot**. Kelompok dibuat dari data, sehingga kolom tersebut hanya boleh berisi angka atau tanggal, tanpa teks dan tanpa sel kosong.

- **Tanggal**: centang bagian yang akan dikelompokkan: **Detik**, **Menit**, **Jam**, **Hari**, **Bulan**, **Kuartal**, atau **Tahun**. Bagian terkecil yang Anda centang tetap berada pada bidang. Untuk setiap bagian yang lebih besar, JeffCrunch menambahkan bidang, diberi nama sesuai bagiannya (misalnya **Tahun**), dan meletakkannya sebelum bidang tanggal di tempat yang sama. **Mulai dari** dan **Berakhir pada** adalah tanggal pertama dan terakhir; biarkan seperti semula untuk memakai tanggal terkecil dan terbesar dalam data. Tanggal sebelum awal atau sesudah akhir masuk ke dua kelompok tambahan, yang diawali dengan `<` dan `>`.
- **Angka**: **Mulai dari**, **Berakhir pada**, dan **Dengan** membuat kelompok seperti 1-10, 11-20, dan seterusnya.

**Pisahkan Kelompok** menghapus kelompok dan bidang yang ditambahkan untuknya. Segarkan membuat kelompok kembali dari data baru. Ketika dua tabel pivot dibuat dari data yang sama, kelompok tidak dapat dibuat, karena kelompok merupakan bagian dari data yang mereka pakai bersama.

## Bidang terhitung

Bidang terhitung adalah bidang yang Anda buat dari sebuah rumus. Ia memakai bidang lain dari daftar. Misalnya, bonus sebesar sepersepuluh jumlah adalah `=Jumlah*0,1`.

Pilih **Bidang Terhitung** pada tab **Tabel Pivot**. Ketik nama dan rumus. Tulis nama bidang di dalam rumus, atau klik ganda pada bidang di daftar **Bidang** untuk memasukkannya. Beri tanda kutip tunggal pada nama yang mengandung spasi, seperti `='Harga satuan'*Jumlah`. Anda dapat memakai angka, `+ - * / ^ %`, perbandingan, dan beberapa fungsi seperti `IF`, `ROUND`, `SUM`, `MIN`, `MAX`, `AVERAGE`, dan `ABS`.

Pilih **Tambah**. Bidang baru dimasukkan ke kotak **Nilai**. Bidang itu juga tampil di daftar bidang, tempat Anda dapat mencentangnya atau menghapus centangnya seperti bidang lain. Bidang terhitung tidak dapat masuk ke baris, kolom, atau filter.

Bidang terhitung dihitung di setiap sel laporan. Nama bidang mewakili total bidang itu di dalam sel, sehingga `=Jumlah*Jumlah` mengkuadratkan total sel. Rumus ini tidak mengkuadratkan setiap baris lalu menjumlahkannya. Total keseluruhan memakai total seluruh daftar.

**Daftar Rumus** (di samping **Bidang Terhitung**) membuat lembar baru yang mencantumkan setiap bidang terhitung beserta rumusnya, dan setiap item terhitung beserta rumus, bidang, dan urutan penyelesaiannya. **Urungkan** menghapus sel-sel itu, dan **Urungkan** lagi menghapus lembarnya.

Untuk mengubah bidang, pilih namanya di kotak **Nama**. Rumusnya akan tampil. Ubah dan pilih **Ubah**. **Hapus** menghapus bidang beserta nilainya. Setiap tindakan adalah satu langkah **Urungkan**. Bidang terhitung disimpan bersama berkas dan tetap dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak dapat memilikinya, karena bidang terhitung merupakan bagian dari data yang mereka pakai bersama.

## Item terhitung

Item terhitung adalah item yang Anda buat dari item lain pada bidang yang sama. Misalnya, bidang Wilayah memiliki item Timur dan Barat. Item baru bernama Keduanya dapat berisi `=Timur+Barat`.

Klik sel di tabel pivot, lalu pilih **Item Terhitung** pada tab **Tabel Pivot**. Pilih **Bidang**. Bidang itu harus berada di baris atau kolom. Ketik **Nama** dan **Rumus**. Tulis nama item di dalam rumus, atau klik ganda pada item di daftar **Item** untuk memasukkannya. Beri tanda kutip tunggal pada nama yang mengandung spasi, seperti `='Timur Besar'+Barat`. Anda dapat memakai angka, `+ - * / ^ %`, dan tanda kurung.

Item baru mendapat satu baris di laporan. Di setiap kolom, item itu menampilkan rumus yang dihitung dari baris item yang disebutnya. Item baru ikut dihitung dalam total. Total berupa penjumlahan, seperti total keseluruhan atau subtotal, menjumlahkan baris item baru dengan baris item lainnya. Jadi dengan Timur bernilai 35, Barat 14, dan Keduanya (Timur ditambah Barat) 49, total keseluruhan adalah 98. Jika item yang disebut rumus disembunyikan, sel menampilkan **#REF!**. Jika rumus menyebut item yang tidak ada, sel menampilkan **#NAME?**.

Untuk mengubah item, pilih namanya di kotak **Nama**. Rumusnya akan tampil. Ubah dan pilih **Ubah**. **Hapus** menghapus item beserta barisnya. Setiap tindakan adalah satu langkah **Urungkan**. Item terhitung disimpan bersama berkas dan tetap dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak dapat memilikinya, karena item terhitung merupakan bagian dari data yang mereka pakai bersama.

### Urutan penyelesaian

Kadang-kadang satu sel memiliki dua item terhitung. Satu berada di barisnya dan yang lain di kolomnya. Urutan penghitungannya dapat mengubah hasilnya. Misalkan item baris adalah `=Timur-Barat` dan item kolom adalah `=Jan+1`. Jika dihitung dalam satu urutan, sel bernilai (Timur - Barat) + 1. Dalam urutan lain, sel bernilai (Timur + 1) - (Barat + 1), yang merupakan angka berbeda.

**Urutan Penyelesaian** pada tab **Tabel Pivot** mencantumkan item terhitung sesuai urutan penghitungannya. Item yang lebih rendah dalam daftar dihitung paling akhir. Pilih sebuah item lalu klik **Pindah ke Atas** atau **Pindah ke Bawah**. Urutan itu disimpan bersama berkas sebagai urutan item, dan program lain membacanya dengan cara yang sama. **Daftar Rumus** menunjukkan posisi setiap item di dalamnya.

## Tampilkan baris di balik nilai

Klik ganda pada nilai tabel pivot, atau pilih **Tampilkan Detail** pada tab **Tabel Pivot**. JeffCrunch membuat lembar baru, diberi nama Detail 1, 2, dan seterusnya, berisi baris dari daftar yang membentuk angka tersebut. Nama bidang berada di baris pertama. F2 tetap dapat mengedit sel, jika sel itu dapat diedit. **Urungkan** menghapus baris-baris itu, dan **Urungkan** lagi menghapus lembarnya.

## Perluas dan ciutkan

Bidang yang item-itemnya memiliki item lain di bawahnya dapat menampilkan atau menyembunyikan baris di bawah itemnya. **Perluas Seluruh Bidang** dan **Ciutkan Seluruh Bidang** melakukannya untuk setiap item dari bidang yang Anda pilih di panel.

## Opsi Tabel Pivot

**Opsi** pada tab **Tabel Pivot** membuka kotak dengan pilihan tabel.

- **Nama** adalah nama tabel pivot. Tidak ada tabel pivot lain dalam buku kerja yang boleh memakai nama itu.
- **Untuk nilai galat tampilkan** dan **Untuk sel kosong tampilkan** memasukkan teks Anda sendiri ke dalam sel-sel tersebut.
- **Gabungkan dan pusatkan sel dengan label**, **Sesuaikan lebar kolom saat tabel diperbarui**, dan **Pertahankan pemformatan sel saat tabel diperbarui** disimpan bersama tabel.
- **Tampilkan total keseluruhan untuk baris** dan **untuk kolom** sama dengan pilihan Total Keseluruhan.
- **Segarkan data saat berkas dibuka** membaca ulang sel setiap kali buku kerja dibuka. Ini adalah satu langkah Urungkan, seperti penyegaran lainnya.

## Gaya: header, pita, dan galeri

**Opsi Gaya** pada tab **Tabel Pivot** menyalakan atau mematikan baris header, kolom pertama (header baris), serta pita baris dan kolom. **Gaya** memiliki galeri tampilan, dan **Hapus Gaya** menghapus tampilan itu. Tampilan ini digambar dengan cara yang sama seperti tampilan tabel. **Baris Kosong** dan **Ulangi Label** mengatur baris kosong dan label yang diulang untuk semua bidang baris sekaligus.

## Desain: subtotal, total keseluruhan, dan tata letak

- **Subtotal** menampilkan atau menyembunyikan total setiap kelompok, di bagian atas atau bawah kelompok. Jika ada lebih dari satu bidang di Baris, total setiap bidang luar merupakan subtotal.
- **Total Keseluruhan** memiliki empat pilihan: nonaktif untuk baris dan kolom, aktif untuk keduanya, aktif hanya untuk baris (kolom total di sebelah kanan, satu untuk setiap baris), dan aktif hanya untuk kolom (baris total di bagian bawah).
- **Tata Letak Laporan** adalah bentuk label. Dalam bentuk **ringkas**, semua bidang Baris berbagi satu kolom, dan kelompok baru ditampilkan sedikit ke kanan. Dalam bentuk **garis besar**, setiap bidang memiliki kolomnya sendiri, dan setiap kelompok memiliki baris tersendiri di atas itemnya. Dalam bentuk **tabel**, setiap bidang memiliki kolomnya sendiri dan nama kelompok berada di baris pertamanya, dengan subtotal di bawah kelompok.

## Daftar yang sama, dengan filter

Pindahkan **Bulan** dari **Kolom** ke **Filter**. Laporan kehilangan kolomnya, dan sebuah baris di atasnya menampilkan **Bulan** dan **(Semua)**. Angkanya sekarang adalah total setiap wilayah untuk semua bulan.

## Bagan pivot

Klik sel di tabel pivot lalu pilih **Sisipkan**, kemudian **Bagan Pivot**. JeffCrunch membuat bagan kolom dari nilai tabel. Total keseluruhan tidak disertakan. Bagan ditandai dalam berkas sebagai bagan milik tabel pivot. Bagan membaca sel tabel, sehingga angkanya berubah ketika Anda menyegarkan tabel atau mengubah tata letaknya. Jika tabel bertambah baris atau kolom, buat bagan kembali, karena bagan menyimpan sel tempat ia dibuat. Bagan pivot yang berasal dari berkas ditampilkan sebagaimana disimpan.

## Pengiris dan linimasa

Pengiris adalah sekumpulan tombol yang menyaring tabel pivot, dan linimasa adalah penggeser yang melakukan hal yang sama untuk tanggal.

Klik sel di tabel pivot, lalu klik **Pengiris** pada tab Tabel Pivot. Panel Pengiris terbuka dan mencantumkan pengiris serta linimasa dalam buku kerja. Pengiris memiliki satu tombol untuk setiap item bidangnya. Tombol berwarna biru adalah item yang ditampilkan tabel.

Klik sebuah tombol untuk hanya menampilkan item itu. Klik lagi untuk menampilkan semua item. Tahan **Ctrl** dan klik untuk menambahkan item ke yang sudah ditampilkan, atau untuk menghapusnya. Setiap klik adalah satu langkah Urungkan, dan Urungkan mengembalikan tabel. Ketika Anda menyimpan, berkas menyimpan pilihan pengiris.

**Laporan Koneksi** menyebutkan tabel pivot mana yang disaring oleh pengiris. Linimasa menampilkan rentang tanggal yang dipilihnya. JeffCrunch menampilkan pengiris di panel ini, bukan di atas sel, dan belum mengubah linimasa. Keduanya disimpan dalam berkas sebagaimana disimpan semula.

## Batas

Tabel pivot dapat memiliki hingga 2.000 bidang, daftar hingga lima juta baris, dan laporan hingga empat juta sel. Berkas yang melebihi batas ini ditampilkan sebagaimana disimpan dan tidak dapat diubah. Membaca seratus ribu baris dengan dua puluh kolom dan menghitung ulang laporan memakan waktu sekitar setengah detik pada komputer tempat pengukuran ini dilakukan; komputer yang lebih lambat membutuhkan waktu lebih lama.
