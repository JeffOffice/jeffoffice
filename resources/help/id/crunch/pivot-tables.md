# Tabel pivot

<!-- keywords: file lain, buku kerja lain, sumber eksternal, daftar rumus, bidang terhitung, item terhitung, urutan penyelesaian, bidang rumus, halaman filter laporan, tabel pivot, telusuri detail, telusuri ringkasan, sisipkan tabel pivot, buat, pilih, pindahkan tabel pivot, getpivotdata, buat getpivotdata, segarkan, segarkan semua, ubah sumber data, daftar bidang, bidang, baris, kolom, nilai, filter, filter laporan, total keseluruhan, subtotal, tata letak, ringkas, garis besar, tabular, hapus, opsi bidang nilai, jumlah, hitung, rata-rata, tampilkan nilai sebagai, persen dari total, selisih, total berjalan, format angka, pengaturan bidang, urutkan, saring, teratas, kelompokkan, batalkan pengelompokan, tanggal, bulan, tahun, perluas, ciutkan, opsi, gaya, baris berpita, baris kosong, ulangi label, segarkan saat berkas dibuka, bagan pivot, pengiris, garis waktu, tampilkan detail, telusuri detail, baris di balik nilai -->

Tabel pivot adalah laporan kecil yang dibuat dari daftar panjang. Daftar itu, misalnya, memiliki satu baris untuk setiap penjualan; laporan menjumlahkan penjualan itu menurut wilayah dan bulan. Anda tidak perlu menulis rumus apa pun. Anda cukup menentukan kolom mana dari daftar yang diletakkan di samping, kolom mana yang diletakkan di atas, dan mana yang dijumlahkan. Laporan akan terisi dengan sendirinya.

## Contoh

Berikut adalah daftar di sel A1 sampai C7. Setiap baris adalah satu penjualan.

| Wilayah | Bulan | Jumlah |
|---|---|---|
| Timur | Jan | 10 |
| Timur | Jan | 5 |
| Timur | Feb | 20 |
| Barat | Jan | 7 |
| Barat | Feb | 3 |
| Barat | Feb | 4 |

Tabel pivot dengan **Wilayah** di samping, **Bulan** di atas, dan jumlah **Jumlah** di bagian tengah tampak seperti ini.

| Jumlah dari Jumlah | Bulan | | |
|---|---|---|---|
| Wilayah | Jan | Feb | Total Keseluruhan |
| Timur | 15 | 20 | 35 |
| Barat | 7 | 7 | 14 |
| Total Keseluruhan | 22 | 27 | 49 |

Timur pada bulan Januari bernilai 15 karena dua baris Timur untuk Januari, 10 dan 5, dijumlahkan. Kolom Total Keseluruhan menjumlahkan setiap baris, baris Total Keseluruhan menjumlahkan setiap kolom, dan sel di sudut menjumlahkan semuanya.

Istilah yang dipakai di sini:

- **Bidang** adalah kolom dari daftar. Wilayah, Bulan, dan Jumlah adalah bidang.
- **Baris**, **Kolom**, dan **Nilai** adalah tiga tempat bidang dapat diletakkan. Bidang di Baris memberi satu baris untuk setiap entri yang berbeda; bidang di Kolom memberi satu kolom untuk setiap entri; bidang di Nilai adalah yang dijumlahkan. Tempat keempat, **Filter**, berisi bidang yang menyaring baris yang tidak Anda pilih.
- **Cache** (cache tabel pivot) adalah salinan daftar Anda yang disimpan tabel pivot di dalam berkas, sehingga dapat menampilkan total tanpa membaca sel. Laporan dihitung dari cache, bukan dari sel, sehingga laporan tidak berubah ketika sel berubah. **Segarkan** menyalin ulang sel ke dalam cache.

## Membuat tabel pivot

1. Klik sel di dalam daftar, atau pilih sel yang ingin Anda gunakan. Daftar memerlukan baris nama di atas datanya.
2. Pilih **Sisipkan**, lalu **Tabel Pivot**.
3. Kotak akan menampilkan sel tempat data berada. Ubah jika tidak sesuai. Anda juga dapat mengetik nama tabel. Untuk memakai daftar yang ada di berkas lain, pilih **Rentang dalam berkas lain**, pilih berkas dengan **Telusuri**, lalu ketik rentangnya, atau nama tabel atau nama yang Anda buat di berkas itu, misalnya `Sheet1!A1:D100`. JeffCrunch membaca buku kerja (`.xlsx`, `.xlsm`, `.xls`, dan `.ods`) serta berkas `.csv`.
4. Pilih tempat tabel pivot akan diletakkan: **Lembar baru** (bawaan), atau sel di lembar ini atau lembar lain. Pilih tempat yang kosong, karena tabel kosong menempati sekitar delapan belas baris dan tiga kolom.
5. Pilih **OK**. Tabel mulai kosong, dan panel **Bidang Tabel Pivot** terbuka. Centang bidang yang Anda inginkan: bidang berisi angka akan dijumlahkan, dan bidang lain menjadi baris. Kemudian pindahkan bidang ke kotak seperti dijelaskan di bagian berikut.

Ketika data berada di berkas lain, tabel pivot menyimpan salinannya, sehingga tabel tetap bekerja dan tersimpan tanpa berkas itu. Lihat Data di berkas lain di bawah.

Lembar baru adalah satu langkah Urungkan tersendiri, begitu pula tabel. Ketika Anda menyimpan sebagai .xlsx, tabel ditulis sebagaimana program aslinya menulisnya, sehingga terbuka di sana sebagai tabel pivot. Salinan yang disimpan sebagai .xls atau .ods menyimpan angka yang ditampilkan tabel di sel-selnya, tetapi tidak menyimpan tabel itu sendiri, dan penyimpanan akan memberi tahu Anda hal tersebut.

## GETPIVOTDATA: membaca satu angka dari tabel pivot

`GETPIVOTDATA` memberikan angka yang ditampilkan tabel pivot untuk nilai, baris, dan kolom yang Anda sebutkan. Untuk contoh di atas:

`=GETPIVOTDATA("Jumlah dari Jumlah", $E$3, "Wilayah", "Timur", "Bulan", "Jan")` memberikan 15.

- Argumen pertama adalah nama nilai, seperti yang ditampilkan tabel (misalnya Jumlah dari Jumlah). Nama bidang yang dijumlahkan (Jumlah) juga dapat digunakan.
- Argumen kedua adalah sel mana pun dari tabel pivot.
- Setelah itu datang pasangan bidang dan salah satu itemnya. Jika Anda hanya menyebut Wilayah dan Timur, Anda mendapat total Timur, 35. Jika tidak menyebut apa pun, Anda mendapat total keseluruhan, 49. Bidang yang Anda sebut harus merupakan bidang pertama pada baris, atau bidang pertama pada kolom, secara berurutan.
- Jika tabel tidak menampilkan angka itu, sel menampilkan `#REF!`.

Ketika Anda mengklik nilai tabel pivot saat mengetik rumus, JeffCrunch menulis rumus ini untuk Anda, sehingga rumus tetap membaca angka yang benar setelah tabel diatur ulang. Untuk mematikannya, pilih **Opsi** pada tab **Tabel Pivot**, lalu hapus centang pada kotak tentang GETPIVOTDATA.

## Apa yang dilakukan JeffCrunch terhadap tabel pivot

Tabel pivot dalam buku kerja ditampilkan seperti saat disimpan. Jika Anda tidak mengubah apa pun, berkas disimpan dengan tabel pivot persis seperti semula. Ketika Anda menyegarkannya atau mengubahnya, JeffCrunch menghitung laporan lagi, memasukkan angka baru ke dalam sel, dan menulis tabel pivot kembali agar program yang membuatnya dapat membukanya tanpa keluhan.

JeffCrunch dapat menghitung tabel pivot yang menjumlahkan, menghitung, merata-ratakan, dan sebagainya, yang mengelompokkan tanggal dan angka, serta yang menampilkan nilai sebagai persentase, selisih, atau total berjalan. Tabel pivot yang menampilkan nilai sebagai peringkat atau sebagai bagian dari induknya, memiliki kelompok item yang dibuat secara manual, atau memakai model data, ditampilkan dan disimpan seperti aslinya, dan perintah akan memberi tahu Anda bahwa tabel itu tidak dapat diubah.

Klik sel mana pun di tabel pivot. Tab **Tabel Pivot** akan muncul.

## Segarkan

**Segarkan** membaca sel tempat tabel pivot dibuat dan menghitung laporan lagi. Entri baru ditambahkan setelah entri yang sudah ada; entri yang sudah hilang dibuang. Lebar kolom dan tata letak yang Anda pilih tetap dipertahankan. **Segarkan Semua**, pada tab **Data** dan tab **Tabel Pivot**, melakukan hal yang sama untuk setiap tabel pivot dalam buku kerja.

Jika laporan akan tumbuh menimpa sel yang sudah berisi sesuatu, JeffCrunch tidak menyegarkan. Ia akan memberi tahu sel mana yang menghalangi. Kosongkan sel itu atau pindahkan tabel pivot, lalu segarkan lagi.

**Urungkan** membatalkan penyegaran dalam satu langkah.

## Ubah Sumber Data

**Ubah Sumber Data** meminta sel tempat daftar berada, termasuk baris nama pertamanya. Ketik seperti `Sheet1!A1:D100`, atau ketik nama tabel atau nama yang Anda buat. JeffCrunch membaca sel tersebut dan menghitung laporan lagi. Jika bidang daftar memiliki nama yang sama pada sel baru, posisinya dalam laporan tetap.

Anda juga dapat mengarahkan tabel ke rentang di berkas lain. Pada kotak, pilih **Rentang dalam berkas lain**, pilih berkasnya, lalu ketik rentang di dalamnya. Memilih **Rentang dalam buku kerja ini** akan mengarahkan tabel kembali ke sel di sini. Setiap perubahan tersebut adalah satu langkah Urungkan.

## Data di berkas lain

Tabel pivot yang dibuat dari berkas lain menyimpan data asalnya di dalam buku kerja ini, sehingga Anda dapat membuka, mengubah, dan menyimpan buku kerja ketika berkas lain sudah tidak ada. **Segarkan** membaca berkas lain lagi. Jika berkas itu dipindahkan, hilang, terlalu besar, bukan buku kerja, atau sudah tidak memiliki rentang tersebut, tabel tetap menyimpan data yang dimilikinya, dan sebuah pesan memberi tahu bahwa berkas tidak dapat dibaca. Berkas yang berada di komputer lain dalam jaringan Anda tidak dibaca sampai Anda memilihnya sendiri di **Ubah Sumber Data**. Satu penyegaran membaca setiap berkas sekali, dengan paling banyak 32 berkas, 1 GB, dan 30 detik secara keseluruhan; tabel yang melewati batas itu tetap menyimpan datanya.

Jika sebuah buku kerja menyatakan bahwa tabelnya harus dibaca ulang saat dibuka, dan data tabel berada di berkas lain, JeffCrunch akan bertanya lebih dulu, dengan cara yang sama seperti untuk tautan ke buku kerja lain. Sampai Anda menjawab ya, tidak ada berkas yang dibaca.

Buku kerja yang disimpan sebagai .xlsx mencatat lokasi berkas. Memilih berkas pada kotak akan menyimpan alamat lengkapnya. Buku kerja yang dibuat program lain, yang tabelnya mengarah ke berkas dengan jalur pendek seperti `Prices.xlsx`, tetap menyimpan jalur itu, dan JeffCrunch mencari berkasnya di folder yang sama dengan buku kerja. Ketika Anda menggunakan **Simpan Sebagai** untuk menyimpan buku kerja di folder lain, jalur seperti itu ditulis ulang agar tetap menunjuk berkas yang sama. Penyegaran tidak mengikuti tautan yang dimiliki berkas lain ke berkas lain lagi.

## Hapus

**Hapus** memiliki dua pilihan. **Hapus Filter** menghapus setiap filter dan menampilkan kembali setiap entri. **Hapus Semua** mengeluarkan setiap bidang dari laporan dan membiarkannya kosong, siap untuk dibangun kembali.

## Telusuri detail dan telusuri ringkasan

## Halaman Filter Laporan

Filter berada di atas tabel pivot dan memungkinkan Anda melihat satu item pada satu waktu, misalnya satu bulan. **Tampilkan Halaman Filter Laporan** pada tab **Tabel Pivot** membuat halaman untuk setiap item sekaligus. Setiap halaman adalah lembar baru, dinamai menurut item, dengan salinan tabelnya sendiri. Filter pada setiap salinan diatur ke item tersebut. Jika tabel memiliki lebih dari satu filter, Anda memilih filter yang akan dipakai. Satu **Urungkan** menghapus semua halaman.

Ketika laporan memiliki lebih dari satu bidang pada baris, item dari bidang pertama dapat dibuka atau ditutup. Klik sel dengan nama item, lalu pilih **Telusuri Ringkasan** untuk menutupnya: baris dari bidang berikutnya di bawahnya disembunyikan dan hanya totalnya yang tetap tampil. **Telusuri Detail** membukanya kembali. Item dari bidang terakhir tidak memiliki apa pun di bawahnya. Setiap tindakan adalah satu **Urungkan**.

## Pilih dan pindahkan

**Pilih** pada tab **Tabel Pivot** memilih sebagian laporan: **Seluruh Tabel Pivot**, **Nilai** (angkanya), atau **Label** (nama di samping; jika laporan tidak memiliki nama di samping, nama di atas). **Pindahkan Tabel Pivot…** meminta sel pada lembar yang sama dan menggambar ulang laporan dengan sel pertamanya di sana. Sel yang ditinggalkan dikosongkan. Rumus yang menunjuk ke sel laporan akan mengikutinya, seperti saat Anda memotong dan menempel. Jika tempat baru sudah berisi sesuatu, JeffCrunch tidak memindahkan laporan dan memberi tahu Anda. Satu **Urungkan** mengembalikannya.

## Panel bidang

**Daftar Bidang** pada tab **Tabel Pivot** membuka panel **Bidang Tabel Pivot**. Panel ini menampilkan bidang dari daftar dengan kotak di sebelahnya, dan di bawahnya empat tempat: **Filter**, **Kolom**, **Baris**, dan **Nilai**.

- Centang bidang untuk menambahkannya. Bidang yang hanya berisi angka masuk ke **Nilai** dan dijumlahkan; bidang lain masuk ke **Baris**.
- Hapus centang bidang untuk mengeluarkannya dari semua tempat.
- Klik bidang di salah satu dari empat tempat, lalu pilih **Pindahkan Ke** untuk memasukkannya ke tempat lain. Kata **Pindahkan Naik** dan **Pindahkan Turun** mengubah urutannya di tempatnya, dan **Hapus Bidang** mengeluarkannya.
- Dengan lebih dari satu nilai, baris bernama **Nilai** muncul di **Kolom**. Pindahkan ke **Baris** untuk meletakkan nilai di samping.
- **Pengaturan Bidang Nilai** (klik ganda pada nilai, atau tombolnya) mengatur nama yang ditampilkan laporan untuk nilai dan cara penjumlahannya: **Jumlah**, **Hitung**, **Hitung Angka**, **Rata-rata**, **Maks**, **Min**, **Hasil Kali**, **StdDev** dan **StdDevp** (simpangan baku sampel, atau dari semua nilai), serta **Var** dan **Varp** (varians dari hal yang sama). Pengaturan ini juga mengatur cara nilai ditampilkan dan format angkanya. Lihat bagian berikutnya.
- **Pengaturan Bidang** (klik ganda pada bidang di Filter, Kolom, atau Baris, atau tombolnya) mengatur satu bidang. Lihat "Pengaturan Bidang".

Setiap perubahan adalah satu langkah **Urungkan**.

## Tampilkan Nilai Sebagai, dan format angka nilai

Dalam **Pengaturan Bidang Nilai**, **Tampilkan nilai sebagai** mengubah apa yang ditampilkan sebuah sel laporan.

- **% dari Total Keseluruhan**: sel dibagi total seluruh laporan. Dalam contoh, Timur pada Januari adalah 15 dari 49, yaitu 30,6%.
- **% dari Total Kolom** dan **% dari Total Baris**: sel dibagi total kolomnya, atau total barisnya.
- **Indeks**: perbandingan sel dengan nilai yang akan dihasilkan baris dan kolomnya jika data tersebar merata. Angka di atas 1 berarti lebih dari bagian yang merata.
- **Selisih Dari**, **% Dari**, dan **% Selisih Dari**: sel dibandingkan dengan item lain dari sebuah bidang. Pilih **Bidang dasar** (bidang di Baris atau Kolom) dan **Item dasar** (salah satu itemnya, atau item sebelumnya atau sesudahnya). Misalnya, dengan Bulan sebagai bidang dasar dan Jan sebagai item dasar, Februari menunjukkan seberapa lebih besar atau lebih kecil dibanding Januari. Item dasar itu sendiri dikosongkan untuk selisih, dan bernilai 100% untuk **% Dari**.
- **Total Berjalan Dalam**: setiap sel menjumlahkan sel-sel di depannya sepanjang bidang dasar.
- **Peringkat dari Terkecil ke Terbesar** dan **Peringkat dari Terbesar ke Terkecil**: setiap sel menampilkan urutannya, 1, 2, 3, dan seterusnya, di antara sel-sel bidang dasar dalam baris atau kolom yang sama. Peringkat 1 adalah angka terkecil, atau terbesar.
- **% dari Total Baris Induk** dan **% dari Total Kolom Induk**: setiap sel menampilkan bagian induknya yang diwakilinya. Induk sebuah item adalah item satu tingkat di atasnya, misalnya tahun yang memuat sebuah bulan. Item tanpa induk ditampilkan sebagai bagian dari total keseluruhan.

**Format angka** menerima kode format, seperti `0.00`, `#,##0`, atau `0.0%`. Kosongkan untuk mempertahankan format yang dimiliki sel. Nilai yang ditampilkan sebagai persentase mendapat `0.00%` kecuali Anda memilih format lain. Sel laporan langsung memakai format itu. **Urungkan** membatalkannya bersama angkanya.

## Pengaturan Bidang

Pilih bidang di kotak **Filter**, **Kolom**, atau **Baris**, lalu pilih **Pengaturan Bidang** pada tab **Tabel Pivot** (atau klik ganda).

- **Nama kustom** mengubah nama yang ditampilkan laporan untuk bidang.
- **Subtotal**: **Otomatis** menampilkan total untuk setiap kelompok bidang, **Tidak ada** tidak menampilkannya.
- **Tampilkan label item dalam**: bentuk ringkas, garis besar, atau tabel, seperti pada Tata Letak Laporan.
- **Sisipkan baris kosong setelah setiap item** menempatkan baris kosong setelah setiap kelompok.
- **Ulangi label item** menampilkan nama item luar pada setiap baris kelompoknya (bentuk garis besar dan tabel).
- **Urutkan**: pertahankan urutan item seperti sekarang (manual), atau urutkan A ke Z atau Z ke A berdasarkan label. **Terkecil ke terbesar menurut nilai** dan **Terbesar ke terkecil menurut nilai** mengurutkan item berdasarkan angka pada bidang nilai. Laporan mempertahankan pengurutan ketika data berubah.
- **Saring** menyembunyikan item berdasarkan labelnya (diawali dengan, diakhiri dengan, berisi, sama dengan) atau berdasarkan nilai (lebih besar dari, lebih kecil dari, di antara, dan sebagainya), atau hanya menyimpan beberapa item teratas atau terbawah menurut nilai. Filter dihitung ketika Anda memilih OK. Filter menyembunyikan item yang ditinggalkannya; kotak centang di bawah memperlihatkan item mana, dan Anda dapat mencentang item untuk menampilkannya kembali.
- **Tampilkan item ini** adalah daftar item bidang dengan kotak centang di setiap item. Hapus centang untuk menyembunyikan item, dan total tidak akan menyertakannya.

## Kelompokkan tanggal dan angka

Pilih bidang berisi tanggal atau angka di **Baris** atau **Kolom**, lalu pilih **Kelompokkan Bidang** pada tab **Tabel Pivot**. Kelompok dibuat dari data, sehingga kolom itu hanya boleh berisi angka atau tanggal, tanpa teks dan tanpa sel kosong.

- **Tanggal**: centang bagian yang akan dikelompokkan: **Detik**, **Menit**, **Jam**, **Hari**, **Bulan**, **Kuartal**, atau **Tahun**. Bagian paling halus yang Anda centang tetap berada pada bidang. Untuk setiap bagian yang lebih kasar, JeffCrunch menambahkan bidang yang dinamai menurut bagian itu (misalnya **Tahun**), dan meletakkannya sebelum bidang tanggal di tempat yang sama. **Mulai dari** dan **Berakhir pada** adalah tanggal pertama dan terakhir; biarkan seperti semula untuk memakai tanggal paling awal dan paling akhir dalam data. Tanggal sebelum awal atau sesudah akhir masuk ke dua kelompok tambahan, yang diawali dengan `<` dan `>`.
- **Angka**: **Mulai dari**, **Berakhir pada**, dan **Per** membuat kelompok seperti 1-10, 11-20, dan seterusnya.

**Batalkan Pengelompokan** menghapus kelompok dan bidang yang ditambahkan untuknya. Penyegaran membuat kelompok lagi dari data baru. Ketika dua tabel pivot dibuat dari data yang sama, kelompok tidak dapat dibuat, karena kelompok adalah bagian dari data yang mereka pakai bersama.

## Bidang terhitung

Bidang terhitung adalah bidang yang Anda buat dari sebuah rumus. Rumus itu memakai bidang lain dari daftar. Misalnya, bonus sebesar sepersepuluh jumlah adalah `=Jumlah*0,1`.

Pilih **Bidang Terhitung** pada tab **Tabel Pivot**. Ketik nama dan rumus. Tulis nama bidang di dalam rumus, atau klik ganda pada bidang di daftar **Bidang** untuk memasukkannya. Beri tanda kutip tunggal pada nama yang memiliki spasi, seperti `='Harga satuan'*Jumlah`. Anda dapat memakai angka, `+ - * / ^ %`, perbandingan, dan beberapa fungsi seperti `IF`, `ROUND`, `SUM`, `MIN`, `MAX`, `AVERAGE`, dan `ABS`.

Pilih **Tambah**. Bidang baru ditempatkan di kotak **Nilai**. Bidang itu juga tampil di daftar bidang, tempat Anda dapat mencentangnya atau menghapus centangnya seperti bidang lain. Bidang ini tidak dapat diletakkan di baris, kolom, atau filter.

Bidang terhitung dihitung di setiap sel laporan. Nama bidang mewakili total bidang itu pada sel, sehingga `=Jumlah*Jumlah` mengkuadratkan total sel. Rumus ini tidak mengkuadratkan setiap baris lalu menjumlahkannya. Total keseluruhan memakai total seluruh daftar.

**Daftar Rumus** (di samping **Bidang Terhitung**) membuat lembar baru yang mencantumkan setiap bidang terhitung dan rumusnya, serta setiap item terhitung dengan rumusnya, bidangnya, dan urutan penyelesaiannya. **Urungkan** menghapus sel-sel itu, dan Urungkan lagi menghapus lembarnya.

Untuk mengubah bidang, pilih namanya pada kotak **Nama**. Rumusnya akan tampil. Ubah, lalu pilih **Ubah**. **Hapus** menghapus bidang dan nilainya. Setiap tindakan adalah satu langkah **Urungkan**. Bidang terhitung disimpan bersama berkas dan dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak dapat memilikinya, karena bidang terhitung adalah bagian dari data yang mereka pakai bersama.

## Item terhitung

Item terhitung adalah item yang Anda buat dari item lain pada bidang yang sama. Misalnya, bidang Wilayah memiliki item Timur dan Barat. Item baru bernama Keduanya dapat berisi `=Timur+Barat`.

Klik sel di tabel pivot, lalu pilih **Item Terhitung** pada tab **Tabel Pivot**. Pilih **Bidang**. Bidang itu harus berada di baris atau kolom. Ketik **Nama** dan **Rumus**. Tulis nama item di dalam rumus, atau klik ganda pada item di daftar **Item** untuk memasukkannya. Beri tanda kutip tunggal pada nama yang memiliki spasi, seperti `='Timur besar'+Barat`. Anda dapat memakai angka, `+ - * / ^ %`, dan tanda kurung.

Item baru mendapat satu baris di laporan. Di setiap kolom, baris itu menampilkan rumus yang dihitung dari baris item yang disebutkannya. Item baru ikut dihitung dalam total. Total dari penjumlahan, seperti total keseluruhan atau subtotal, menambahkan baris item baru ke baris item lainnya. Jadi dengan Timur 35, Barat 14, dan Keduanya (Timur ditambah Barat) 49, total keseluruhannya adalah 98. Jika item yang disebut rumus disembunyikan, sel menampilkan **#REF!**. Jika rumus menyebut item yang tidak ada, sel menampilkan **#NAME?**.

Untuk mengubah item, pilih namanya pada kotak **Nama**. Rumusnya akan tampil. Ubah, lalu pilih **Ubah**. **Hapus** menghapus item dan barisnya. Setiap tindakan adalah satu langkah **Urungkan**. Item terhitung disimpan bersama berkas dan dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak dapat memilikinya, karena item terhitung adalah bagian dari data yang mereka pakai bersama.

### Urutan penyelesaian

Kadang satu sel memiliki dua item terhitung. Satu berada di barisnya dan yang lain di kolomnya. Urutan penghitungannya dapat mengubah hasil. Misalkan item baris adalah `=Timur-Barat` dan item kolom adalah `=Jan+1`. Jika dihitung dalam satu urutan, sel bernilai (Timur - Barat) + 1. Dalam urutan lain, sel bernilai (Timur + 1) - (Barat + 1), yang angkanya berbeda.

**Urutan Penyelesaian** pada tab **Tabel Pivot** mencantumkan item terhitung sesuai urutan penghitungannya. Item yang lebih rendah dalam daftar dihitung terakhir. Pilih sebuah item lalu klik **Pindahkan Naik** atau **Pindahkan Turun**. Urutan itu disimpan bersama berkas sebagai urutan item, dan program lain membacanya dengan cara yang sama. **Daftar Rumus** menampilkan posisi setiap item di dalamnya.

## Menampilkan baris di balik sebuah nilai

Klik ganda pada nilai tabel pivot, atau pilih **Tampilkan Detail** pada tab **Tabel Pivot**. JeffCrunch membuat lembar baru bernama Detail 1, 2, dan seterusnya, berisi baris-baris daftar yang membentuk angka tersebut. Nama bidang berada di baris pertama. F2 tetap mengedit sel, jika sel itu dapat diedit. **Urungkan** menghapus baris-baris itu, dan Urungkan lagi menghapus lembarnya.

## Perluas dan ciutkan

Bidang yang item-itemnya memiliki item lain di bawahnya dapat menampilkan atau menyembunyikan baris di bawah itemnya. **Perluas Seluruh Bidang** dan **Ciutkan Seluruh Bidang** melakukannya untuk setiap item bidang yang Anda pilih di panel.

## Opsi Tabel Pivot

**Opsi** pada tab **Tabel Pivot** membuka kotak dengan pilihan untuk tabel.

- **Nama** adalah nama tabel pivot. Tidak ada tabel pivot lain dalam buku kerja yang boleh memakai nama itu.
- **Untuk nilai galat tampilkan** dan **Untuk sel kosong tampilkan** memasukkan teks Anda sendiri ke dalam sel tersebut.
- **Gabungkan dan pusatkan sel dengan label**, **Sesuaikan lebar kolom saat tabel diperbarui**, dan **Pertahankan format sel saat tabel diperbarui** disimpan bersama tabel.
- **Tampilkan total keseluruhan untuk baris** dan **untuk kolom** sama dengan pilihan Total Keseluruhan.
- **Segarkan data saat berkas dibuka** membaca ulang sel setiap kali buku kerja dibuka. Ini adalah satu langkah Urungkan, seperti penyegaran lainnya.

## Gaya: header, pita, dan galeri

**Opsi Gaya** pada tab **Tabel Pivot** menyalakan atau mematikan baris header, kolom pertama (header baris), serta pita baris dan kolom. **Gaya** memiliki galeri tampilan, dan **Hapus Gaya** menghilangkan tampilan. Tampilan ini digambar dengan cara yang sama seperti tampilan tabel. **Baris Kosong** dan **Ulangi Label** mengatur baris kosong dan label yang diulang untuk setiap bidang pada baris sekaligus.

## Desain: subtotal, total keseluruhan, dan tata letak

- **Subtotal** menampilkan atau menyembunyikan total setiap kelompok, di bagian atas atau bawah kelompok. Ketika ada lebih dari satu bidang di Baris, total setiap bidang luar menjadi subtotal.
- **Total Keseluruhan** memiliki empat pilihan: mati untuk baris dan kolom, nyala untuk keduanya, nyala untuk baris saja (kolom total di kanan, satu untuk setiap baris), dan nyala untuk kolom saja (baris total di bawah).
- **Tata Letak Laporan** adalah bentuk label. Dalam bentuk **ringkas**, semua bidang Baris berbagi satu kolom, dan kelompok baru ditampilkan sedikit ke kanan. Dalam bentuk **garis besar**, setiap bidang memiliki kolomnya sendiri, dan setiap kelompok memiliki satu baris sendiri di atas itemnya. Dalam bentuk **tabel**, setiap bidang memiliki kolomnya sendiri dan nama kelompok berada di baris pertamanya, dengan subtotal di bawah kelompok.

## Daftar yang sama, dengan filter

Pindahkan **Bulan** dari **Kolom** ke **Filter**. Laporan kehilangan kolomnya, dan sebuah baris di atasnya menampilkan **Bulan** dan **(Semua)**. Angkanya sekarang berupa total setiap wilayah untuk semua bulan.

## Bagan pivot

Klik sel di tabel pivot lalu pilih **Sisipkan**, kemudian **Bagan Pivot**. JeffCrunch membuat bagan kolom dari nilai tabel. Total keseluruhan tidak disertakan. Bagan ditandai sebagai bagan tabel pivot dalam berkas. Bagan membaca sel-sel tabel, sehingga angkanya berubah ketika Anda menyegarkan tabel atau mengubah tata letaknya. Jika tabel bertambah baris atau kolom, buat bagan lagi, karena bagan menyimpan sel tempat ia dibuat. Bagan pivot yang berasal dari berkas ditampilkan seperti saat disimpan.

## Pengiris dan garis waktu

Pengiris adalah sekumpulan tombol yang menyaring tabel pivot, dan garis waktu adalah penggeser yang melakukan hal serupa untuk tanggal.

Klik sel di tabel pivot, lalu klik **Pengiris** pada tab Tabel Pivot. Panel Pengiris terbuka dan mencantumkan pengiris serta garis waktu dalam buku kerja. Sebuah pengiris memiliki satu tombol untuk setiap item bidangnya. Tombol berwarna biru adalah item yang ditampilkan tabel.

Klik sebuah tombol untuk hanya menampilkan item itu. Klik lagi untuk menampilkan semua item. Tahan **Ctrl** lalu klik untuk menambahkan item ke item yang ditampilkan, atau untuk menghapusnya. Setiap klik adalah satu langkah Urungkan, dan Urungkan mengembalikan tabel. Ketika Anda menyimpan, berkas menyimpan pilihan pengiris.

Pengiris yang sudah ada di dalam berkas juga digambar di atas sel, di tempat berkas menempatkannya. Pengiris menampilkan judulnya dan satu tombol untuk setiap item. Tombol berwarna biru adalah item yang ditampilkan tabel. Klik sebuah tombol untuk hanya menampilkan item itu, tahan **Ctrl** lalu klik untuk menambah atau mengurangi item, dan klik corong di kanan atas untuk menampilkan semua item lagi. Setiap klik adalah satu langkah Urungkan. Ketika tabel disaring dengan cara lain, atau Anda mengklik Urungkan, tombol-tombol mengikutinya. Ketika pengiris terlalu kecil untuk menampilkan semua tombolnya, bilah gulir muncul di sisi kanannya: putar roda mouse di atas pengiris, klik di atas atau di bawah gagang bilah, atau seret gagangnya untuk melihat sisanya. Tahan **Shift** lalu klik tombol kedua untuk memilih seluruh rangkaian item dari tombol pertama yang Anda klik sampai tombol ini; tahan **Shift** dan **Ctrl** untuk menambahkan rangkaian itu ke item yang sudah dipilih. Pengiris yang dicetak tidak dapat digulir, sehingga yang dicetak hanya tombol yang muat, dan baris terakhirnya menyebutkan berapa item lagi yang ada. Pengiris yang menyaring tabel biasa, bukan tabel pivot, atau yang tabel pivotnya tidak ada dalam berkas, digambar dengan judulnya dan kalimat yang menyatakan bahwa JeffCrunch belum dapat menggunakannya; pengiris itu tetap disimpan dalam berkas seperti semula. Buku kerja yang dibuka hanya untuk dibaca, dan lembar yang dilindungi dari perubahan tabel pivot, tidak membiarkan klik menyaring tabel. Pengiris ikut tercetak bersama lembar kecuali berkas menyatakan sebaliknya. Mengklik tombol pengiris menyaringnya, sedangkan mengklik judul atau tepinya memilih pengiris sebagai objek, seperti gambar. Pengiris yang dipilih memiliki gagang: seret untuk memindahkannya, seret gagang untuk mengubah ukurannya, atau tekan tombol panah untuk memindahkannya. Tahan **Ctrl** atau **Shift** lalu klik judul untuk memilihnya bersama objek lain. Klik kanan pengiris untuk menu berisi **Hapus** dan panel **Pengiris**. Tekan **Delete** untuk menghapus pengiris yang dipilih: berkas kemudian tidak memiliki pengiris itu, dan filter yang ditaruh pengiris pada tabel pivot tetap berlaku sampai Anda mengubahnya. Memindahkan, mengubah ukuran, dan menghapus masing-masing adalah satu langkah Urungkan, dan buku kerja yang hanya dibuka untuk dibaca, atau lembar yang dilindungi dari perubahan objek, tidak mengizinkannya.

Untuk mengoperasikan pengiris dengan keyboard, tekan **F6** untuk pergi ke pengiris pertama di lembar (**Shift+F6** ke yang terakhir). Lalu gunakan tombol panah, **Home**, **End**, **Page Up**, dan **Page Down** untuk bergerak di antara tombolnya, **Spasi** atau **Enter** untuk memilih tombol, **Ctrl+Spasi** untuk menambahkan atau menghapus tombol dari item yang dipilih, **Shift+Spasi** untuk memilih rangkaian dari tombol terakhir yang dipilih, dan **Ctrl+Alt+C** untuk menampilkan semua item lagi. Tekan **Esc** untuk melepaskan tombol (tombol panah kemudian memindahkan pengiris, dan **Delete** menghapusnya; saat berada di antara tombol, **Delete** tidak melakukan apa pun), dan **Esc** lagi untuk kembali ke sel. **F6** berpindah ke pengiris berikutnya, dan setelah yang terakhir kembali ke sel.

**Koneksi Laporan** menyatakan tabel pivot mana yang disaring pengiris. Garis waktu menampilkan rentang tanggal yang dipilihnya. JeffCrunch belum menggambar garis waktu di atas sel atau mengubahnya. Keduanya disimpan dalam berkas seperti saat disimpan.

## Batas

Tabel pivot dapat memiliki hingga 2.000 bidang, daftar hingga lima juta baris, dan laporan hingga empat juta sel. Berkas yang melewati batas ini ditampilkan seperti saat disimpan dan tidak dapat diubah. Membaca ulang seratus ribu baris dengan dua puluh kolom dan menghitung laporannya memerlukan sekitar setengah detik pada komputer tempat pengukuran dilakukan; komputer yang lebih lambat memerlukan waktu lebih lama.
