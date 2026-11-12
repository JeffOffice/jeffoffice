# Tabel pivot
<!-- keywords: daftar rumus, bidang terhitung, item terhitung, urutan penyelesaian, bidang rumus, halaman filter laporan, tabel pivot, telusuri detail, telusuri ringkasan, sisipkan tabel pivot, buat, pilih, pindahkan tabel pivot, getpivotdata, buat getpivotdata, segarkan, segarkan semua, ubah sumber data, daftar bidang, bidang, baris, kolom, nilai, filter, filter laporan, total keseluruhan, subtotal, tata letak, ringkas, garis besar, tabel, hapus, pengaturan bidang nilai, jumlah, hitung, rata-rata, tampilkan nilai sebagai, persen dari total, selisih, total berjalan, format angka, pengaturan bidang, urutkan, filter, teratas, kelompok, pisahkan kelompok, tanggal, bulan, tahun, perluas, ciutkan, opsi, gaya, baris berpita, baris kosong, ulangi label, segarkan saat berkas dibuka, bagan pivot, pengiris, linimasa, tampilkan detail, telusuri detail, baris di balik nilai --> 

Tabel pivot adalah laporan kecil yang dibuat dari sebuah daftar panjang. Daftar itu, misalnya, memiliki satu baris untuk setiap penjualan; laporan menjumlahkan penjualan itu per wilayah dan per bulan. Anda tidak perlu menulis rumus. Anda cukup menentukan kolom mana dari daftar yang diletakkan di sisi kiri, kolom mana di bagian atas, dan mana yang dijumlahkan, lalu laporan terisi dengan sendirinya.

## Contoh

Berikut sebuah daftar di sel A1 sampai C7. Setiap baris adalah satu penjualan.

| Wilayah | Bulan | Jumlah |
|---|---|---|
| Timur | Jan | 10 |
| Timur | Jan | 5 |
| Timur | Feb | 20 |
| Barat | Jan | 7 |
| Barat | Feb | 3 |
| Barat | Feb | 4 |

Tabel pivot dengan **Wilayah** di sisi kiri, **Bulan** di bagian atas, dan jumlah **Jumlah** di tengah tampak seperti ini.

| Jumlah dari Jumlah | Bulan | | |
|---|---|---|---|
| Wilayah | Jan | Feb | Total Keseluruhan |
| Timur | 15 | 20 | 35 |
| Barat | 7 | 7 | 14 |
| Total Keseluruhan | 22 | 27 | 49 |

Timur pada Januari bernilai 15 karena dua baris Timur untuk Januari, 10 dan 5, dijumlahkan. Kolom Total Keseluruhan menjumlahkan setiap baris, baris Total Keseluruhan menjumlahkan setiap kolom, dan sel di pojok menjumlahkan semuanya.

Istilah yang dipakai di sini:

- **Bidang** adalah sebuah kolom dari daftar. Wilayah, Bulan, dan Jumlah adalah bidang.
- **Baris**, **Kolom**, dan **Nilai** adalah tiga tempat sebuah bidang dapat diletakkan. Bidang di Baris memberi satu baris untuk setiap entri yang berbeda; bidang di Kolom memberi satu kolom untuk setiap entri; bidang di Nilai adalah yang dijumlahkan. Tempat keempat, **Filter**, berisi bidang yang menyaring baris yang tidak Anda pilih.
- **Cache** (cache tabel pivot) adalah salinan daftar Anda yang disimpan tabel pivot di dalam berkas, sehingga dapat menampilkan total tanpa membaca sel. Laporan dihitung dari cache, bukan dari sel, sehingga tidak berubah ketika sel diubah. **Segarkan** menyalin ulang sel ke dalam cache.

## Membuat tabel pivot

1. Klik sebuah sel di dalam daftar, atau pilih sel yang ingin Anda gunakan. Daftar memerlukan baris nama di atas datanya.
2. Pilih **Sisipkan**, lalu **Tabel Pivot**.
3. Kotak akan menampilkan sel tempat data berada. Ubah jika tidak sesuai. Anda juga dapat mengetik nama sebuah tabel.
4. Pilih tempat tabel pivot akan diletakkan: **Lembar baru** (pilihan standar), atau sebuah sel di lembar ini atau lembar lain. Pilih tempat yang kosong, karena tabel kosong menempati sekitar delapan belas baris dan tiga kolom.
5. Pilih **OK**. Tabel mulai kosong, dan panel **Bidang Tabel Pivot** terbuka. Centang bidang yang Anda inginkan: bidang berisi angka akan dijumlahkan, dan bidang lain menjadi baris. Kemudian pindahkan bidang ke kotak-kotak sesuai bagian berikut.

Lembar baru adalah satu langkah tersendiri pada Urungkan, begitu pula tabelnya. Ketika Anda menyimpan sebagai .xlsx, tabel ditulis sebagaimana program asalnya menulisnya, sehingga tabel itu terbuka di sana sebagai tabel pivot. Salinan yang disimpan sebagai .xls atau .ods mempertahankan angka yang ditampilkan tabel di sel-selnya, tetapi tidak tabelnya sendiri, dan penyimpanan akan memberi tahu Anda hal itu.

## GETPIVOTDATA: membaca satu angka dari tabel pivot

`GETPIVOTDATA` memberikan angka yang ditampilkan tabel pivot untuk nilai, baris, dan kolom yang Anda sebutkan. Untuk contoh di atas:

`=GETPIVOTDATA("Jumlah dari Jumlah"; $E$3; "Wilayah"; "Timur"; "Bulan"; "Jan")` memberikan 15.

- Argumen pertama adalah nama nilai, seperti yang ditampilkan tabel (misalnya Jumlah dari Jumlah). Nama bidang yang dijumlahkan (Jumlah) juga berfungsi.
- Argumen kedua adalah sel mana pun dari tabel pivot.
- Setelah itu ada pasangan bidang dan salah satu itemnya. Jika Anda hanya menyebut Wilayah dan Timur, Anda mendapat total Timur, yaitu 35. Jika tidak menyebut apa pun, Anda mendapat total keseluruhan, yaitu 49. Bidang yang Anda sebutkan harus merupakan bidang pertama pada baris, atau bidang pertama pada kolom, sesuai urutannya.
- Jika tabel tidak menampilkan angka itu, sel menampilkan `#REF!`.

Ketika Anda mengklik sebuah nilai tabel pivot saat mengetik rumus, JeffCrunch menuliskan rumus ini untuk Anda, sehingga rumus tetap membaca angka yang benar setelah tabel disusun ulang. Untuk mematikannya, pilih **Opsi** pada tab **Tabel Pivot**, lalu hapus centang pada kotak tentang GETPIVOTDATA.

## Apa yang dilakukan JeffCrunch terhadap tabel pivot

Tabel pivot dalam buku kerja ditampilkan seperti ketika disimpan. Jika Anda tidak mengubah apa pun, berkas disimpan dengan tabel pivot persis seperti semula. Ketika Anda menyegarkannya atau mengubahnya, JeffCrunch menghitung ulang laporan, memasukkan angka baru ke dalam sel, dan menuliskan kembali tabel pivot sehingga program yang membuatnya dapat membukanya tanpa keluhan.

JeffCrunch dapat menghitung tabel pivot yang menjumlahkan, menghitung, merata-ratakan, dan sebagainya; yang mengelompokkan tanggal dan angka; serta yang menampilkan nilai sebagai persentase, selisih, atau total berjalan. Tabel pivot yang menampilkan nilai sebagai peringkat atau sebagai bagian dari induknya, yang memiliki kelompok item yang dibuat secara manual, atau yang memakai model data akan ditampilkan dan disimpan sebagaimana tersimpan, dan perintah akan memberi tahu Anda bahwa tabel itu tidak dapat diubah.

Klik sel mana pun dari tabel pivot. Tab **Tabel Pivot** akan muncul.

## Segarkan

**Segarkan** membaca sel tempat tabel pivot dibuat, lalu menghitung ulang laporan. Entri baru ditambahkan setelah entri yang sudah ada; entri yang sudah tidak ada akan dihapus. Lebar kolom dan tata letak yang Anda pilih tetap. **Segarkan Semua**, pada tab **Data** dan pada tab **Tabel Pivot**, melakukan hal yang sama untuk setiap tabel pivot dalam buku kerja.

Jika laporan akan tumbuh menimpa sel yang sudah berisi sesuatu, JeffCrunch tidak menyegarkan. Ia memberi tahu Anda sel mana yang menghalangi. Kosongkan sel itu atau pindahkan tabel pivot, lalu segarkan lagi.

**Urungkan** membatalkan penyegaran dalam satu langkah.

## Ubah Sumber Data

**Ubah Sumber Data** meminta sel tempat daftar berada, termasuk baris nama pertamanya. Ketik seperti `Sheet1!A1:D100`, atau ketik nama sebuah tabel atau nama yang Anda buat. JeffCrunch membaca sel tersebut dan menghitung ulang laporan. Jika sebuah bidang daftar memiliki nama yang sama di sel baru, posisinya dalam laporan tetap.

## Hapus

**Hapus** memiliki dua pilihan. **Hapus Filter** menghapus setiap filter dan menampilkan kembali semua entri. **Hapus Semua** mengeluarkan setiap bidang dari laporan dan membuatnya kosong, siap dibangun ulang.

## Telusuri detail dan telusuri ringkasan

## Halaman Filter Laporan

Filter berada di atas tabel pivot dan memungkinkan Anda melihat satu item pada satu waktu, misalnya satu bulan. **Tampilkan Halaman Filter Laporan** pada tab **Tabel Pivot** membuat halaman untuk setiap item sekaligus. Setiap halaman adalah lembar baru, dinamai sesuai item, dengan salinan tabelnya sendiri. Filter pada setiap salinan diatur ke item tersebut. Jika tabel memiliki lebih dari satu filter, Anda memilih filter yang akan dipakai. Satu kali **Urungkan** menghapus semua halaman.

Ketika sebuah laporan memiliki lebih dari satu bidang pada barisnya, item dari bidang pertama dapat dibuka atau ditutup. Klik sel yang berisi nama item, lalu pilih **Telusuri Ringkasan** untuk menutupnya: baris bidang berikutnya di bawahnya disembunyikan dan hanya totalnya yang tinggal. **Telusuri Detail** membukanya kembali. Item dari bidang terakhir tidak memiliki apa pun di bawahnya. Masing-masing adalah satu kali **Urungkan**.

## Pilih dan pindahkan

**Pilih** pada tab **Tabel Pivot** memilih sebagian laporan: **Seluruh Tabel Pivot**, **Nilai** (angkanya), atau **Label** (nama di sisi kiri; jika laporan tidak memiliki nama di sisi kiri, nama di bagian atas). **Pindahkan Tabel Pivot…** meminta sebuah sel pada lembar yang sama dan menggambar ulang laporan dengan sel pertamanya di sana. Sel yang ditinggalkan dikosongkan. Rumus yang menunjuk ke sel laporan akan mengikutinya, seperti ketika Anda memotong dan menempel. Jika tempat baru sudah berisi sesuatu, JeffCrunch tidak memindahkan laporan dan memberi tahu Anda. Satu kali **Urungkan** memindahkannya kembali.

## Panel bidang

**Daftar Bidang** pada tab **Tabel Pivot** membuka panel **Bidang Tabel Pivot**. Panel ini mencantumkan bidang daftar dengan sebuah kotak di sebelahnya, dan di bawahnya empat tempat: **Filter**, **Kolom**, **Baris**, dan **Nilai**.

- Centang sebuah bidang untuk menambahkannya. Bidang yang hanya berisi angka masuk ke **Nilai** dan dijumlahkan; bidang lain masuk ke **Baris**.
- Hapus centang sebuah bidang untuk mengeluarkannya dari semua tempat.
- Klik sebuah bidang di salah satu dari empat tempat, lalu pilih **Pindahkan Ke** untuk memasukkannya ke tempat lain. Kata **Pindah ke Atas** dan **Pindah ke Bawah** mengubah urutannya di tempatnya, dan **Hapus Bidang** mengeluarkannya.
- Jika ada lebih dari satu nilai, sebuah baris bernama **Nilai** muncul di **Kolom**. Pindahkan ke **Baris** untuk menempatkan nilai di sisi kiri.
- **Pengaturan Bidang Nilai** (klik ganda pada sebuah nilai, atau tombolnya) mengatur nama yang ditampilkan laporan untuk nilai dan cara penjumlahannya: **Jumlah**, **Hitung**, **Hitung Angka**, **Rata-rata**, **Maks**, **Min**, **Kali**, **StdDev** dan **StdDevp** (simpangan baku sampel, atau dari semua nilai), serta **Var** dan **Varp** (varians dari hal yang sama). Pengaturan ini juga menentukan cara nilai ditampilkan dan format angkanya. Lihat bagian berikutnya.
- **Pengaturan Bidang** (klik ganda pada sebuah bidang di Filter, Kolom, atau Baris, atau tombolnya) mengatur satu bidang. Lihat "Pengaturan Bidang".

Setiap perubahan adalah satu langkah **Urungkan**.

## Tampilkan Nilai Sebagai, dan format angka sebuah nilai

Dalam **Pengaturan Bidang Nilai**, **Tampilkan nilai sebagai** mengubah apa yang ditampilkan sebuah sel laporan.

- **% dari Total Keseluruhan**: sel dibagi dengan total seluruh laporan. Dalam contoh, Timur pada Januari adalah 15 dari 49, yaitu 30,6%.
- **% dari Total Kolom** dan **% dari Total Baris**: sel dibagi dengan total kolomnya, atau barisnya.
- **Indeks**: perbandingan sel dengan nilai yang akan diperoleh baris dan kolomnya jika data tersebar merata. Angka di atas 1 berarti lebih dari bagian yang merata.
- **Selisih Dari**, **% Dari**, dan **% Selisih Dari**: sel dibandingkan dengan item lain dari sebuah bidang. Pilih **Bidang dasar** (bidang di Baris atau Kolom) dan **Item dasar** (salah satu itemnya, atau item sebelumnya atau berikutnya). Misalnya, dengan Bulan sebagai bidang dasar dan Jan sebagai item dasar, Februari menunjukkan seberapa banyak lebih atau kurang dibandingkan Januari. Item dasar itu sendiri dikosongkan untuk selisih, dan bernilai 100% untuk **% Dari**.
- **Total Berjalan Dalam**: setiap sel menjumlahkan sel-sel sebelumnya sepanjang bidang dasar.
- **Peringkat dari Terkecil ke Terbesar** dan **Peringkat dari Terbesar ke Terkecil**: setiap sel menampilkan urutannya, 1, 2, 3, dan seterusnya, di antara sel-sel bidang dasar pada baris atau kolom yang sama. Peringkat 1 adalah angka terkecil, atau terbesar.
- **% dari Total Baris Induk** dan **% dari Total Kolom Induk**: setiap sel menampilkan bagian dari induknya. Induk sebuah item adalah item satu tingkat di atasnya, seperti tahun yang memuat sebuah bulan. Item tanpa induk ditampilkan sebagai bagian dari total keseluruhan.

**Format angka** menerima kode format, seperti `0.00`, `#,##0`, atau `0.0%`. Biarkan kosong untuk mempertahankan format yang dimiliki sel. Nilai yang ditampilkan sebagai persentase mendapat `0.00%` kecuali Anda memilih format. Sel-sel laporan langsung memakai format itu. **Urungkan** membatalkannya bersama angkanya.

## Pengaturan Bidang

Pilih sebuah bidang pada kotak **Filter**, **Kolom**, atau **Baris**, lalu pilih **Pengaturan Bidang** pada tab **Tabel Pivot** (atau klik ganda padanya).

- **Nama kustom** mengubah nama yang ditampilkan laporan untuk bidang itu.
- **Subtotal**: **Otomatis** menampilkan total untuk setiap kelompok bidang, **Tidak ada** tidak menampilkannya.
- **Tampilkan label item dalam**: bentuk ringkas, garis besar, atau tabel, seperti pada Tata Letak Laporan.
- **Sisipkan baris kosong setelah setiap item** menempatkan baris kosong setelah setiap kelompok.
- **Ulangi label item** menampilkan nama item luar pada setiap baris kelompoknya (bentuk garis besar dan tabel).
- **Urutkan**: pertahankan urutan item (manual), atau urutkan A ke Z atau Z ke A berdasarkan label. **Terkecil ke terbesar berdasarkan nilai** dan **Terbesar ke terkecil berdasarkan nilai** mengurutkan item berdasarkan angka pada bidang nilai. Laporan mempertahankan pengurutan ketika data berubah.
- **Filter** menyaring item berdasarkan labelnya (diawali dengan, diakhiri dengan, mengandung, sama dengan) atau berdasarkan nilai (lebih besar dari, lebih kecil dari, di antara, dan sebagainya), atau hanya menyimpan beberapa item teratas atau terbawah berdasarkan nilai. Filter dihitung ketika Anda memilih OK. Filter menyembunyikan item yang disaringnya; kotak centang di bawah menunjukkan item mana, dan Anda dapat mencentang sebuah item untuk menampilkannya kembali.
- **Tampilkan item ini** adalah daftar item bidang dengan sebuah centang di setiap item. Hapus centang untuk menyembunyikan item, dan total tidak akan menyertakannya.

## Mengelompokkan tanggal dan angka

Pilih bidang berisi tanggal atau angka di **Baris** atau **Kolom**, lalu pilih **Kelompokkan Bidang** pada tab **Tabel Pivot**. Kelompok dibuat dari data, sehingga kolom tersebut hanya boleh berisi angka atau tanggal, tanpa teks dan tanpa sel kosong.

- **Tanggal**: centang bagian yang akan dikelompokkan: **Detik**, **Menit**, **Jam**, **Hari**, **Bulan**, **Kuartal**, atau **Tahun**. Bagian terkecil yang Anda centang tetap berada pada bidang. Untuk setiap bagian yang lebih besar, JeffCrunch menambahkan sebuah bidang, dinamai sesuai bagian itu (misalnya **Tahun**), dan meletakkannya sebelum bidang tanggal di tempat yang sama. **Mulai pada** dan **Berakhir pada** adalah tanggal pertama dan terakhir; biarkan seperti itu untuk memakai tanggal terkecil dan terbesar dalam data. Tanggal sebelum awal atau sesudah akhir masuk ke dua kelompok tambahan, yang diawali dengan `<` dan `>`.
- **Angka**: **Mulai pada**, **Berakhir pada**, dan **Dengan** membuat kelompok seperti 1-10, 11-20, dan seterusnya.

**Pisahkan Kelompok** menghapus kelompok dan bidang yang ditambahkan untuknya. Segarkan membuat kelompok kembali dari data baru. Ketika dua tabel pivot dibuat dari data yang sama, kelompok tidak dapat dibuat, karena kelompok adalah bagian dari data yang mereka pakai bersama.

## Bidang terhitung

Bidang terhitung adalah bidang yang Anda buat dari sebuah rumus. Rumus itu memakai bidang lain dari daftar. Misalnya, bonus sebesar sepersepuluh dari jumlah adalah `=Jumlah*0,1`.

Pilih **Bidang Terhitung** pada tab **Tabel Pivot**. Ketik sebuah nama dan rumus. Tulis nama bidang di dalam rumus, atau klik ganda pada bidang di daftar **Bidang** untuk memasukkannya. Tulis nama yang memiliki spasi di antara tanda kutip tunggal, seperti `='Harga satuan'*Jumlah`. Anda dapat memakai angka, `+ - * / ^ %`, perbandingan, dan beberapa fungsi seperti `IF`, `ROUND`, `SUM`, `MIN`, `MAX`, `AVERAGE`, dan `ABS`.

Pilih **Tambah**. Bidang baru dimasukkan ke kotak **Nilai**. Bidang itu juga tampil di daftar bidang, tempat Anda dapat mencentangnya atau menghapus centangnya seperti bidang lain. Bidang ini tidak dapat diletakkan di baris, kolom, atau filter.

Bidang terhitung dihitung di setiap sel laporan. Nama sebuah bidang mewakili total bidang itu pada sel, sehingga `=Jumlah*Jumlah` mengkuadratkan total sel, bukan mengkuadratkan setiap baris lalu menjumlahkannya. Total keseluruhan memakai total dari seluruh daftar.

**Daftar Rumus** (di sebelah **Bidang Terhitung**) membuat lembar baru yang menyebutkan setiap bidang terhitung dan rumusnya, serta setiap item terhitung dengan rumusnya, bidangnya, dan urutan penyelesaiannya. **Urungkan** menghapus sel-sel itu, dan **Urungkan** sekali lagi menghapus lembarnya.

Untuk mengubah sebuah bidang, pilih namanya pada kotak **Nama**. Rumusnya akan tampil. Ubah, lalu pilih **Ubah**. **Hapus** menghapus bidang beserta nilainya. Masing-masing adalah satu langkah **Urungkan**. Bidang terhitung disimpan bersama berkas dan dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak boleh memilikinya, karena bidang terhitung adalah bagian dari data yang mereka pakai bersama.

## Item terhitung

Item terhitung adalah item yang Anda buat dari item lain pada bidang yang sama. Misalnya, bidang Wilayah memiliki item Timur dan Barat. Item baru bernama Keduanya dapat berisi `=Timur+Barat`.

Klik sebuah sel di tabel pivot, lalu pilih **Item Terhitung** pada tab **Tabel Pivot**. Pilih **Bidang**. Bidang itu harus berada di baris atau kolom. Ketik **Nama** dan **Rumus**. Tulis nama item di dalam rumus, atau klik ganda pada item di daftar **Item** untuk memasukkannya. Tulis nama yang memiliki spasi di antara tanda kutip tunggal, seperti `='Timur besar'+Barat`. Anda dapat memakai angka, `+ - * / ^ %`, dan tanda kurung.

Item baru mendapat satu baris di laporan. Di setiap kolom, item ini menampilkan rumus yang dihitung dari baris item yang disebutkannya. Item baru ikut dihitung dalam total. Total berupa jumlah, seperti total keseluruhan atau subtotal, menambahkan baris item baru ke baris item lainnya. Jadi dengan Timur bernilai 35, Barat 14, dan Keduanya (Timur ditambah Barat) 49, total keseluruhannya adalah 98. Jika item yang disebut rumus disembunyikan, sel menampilkan **#REF!**. Jika rumus menyebut item yang tidak ada, sel menampilkan **#NAME?**.

Untuk mengubah sebuah item, pilih namanya pada kotak **Nama**. Rumusnya akan tampil. Ubah, lalu pilih **Ubah**. **Hapus** menghapus item beserta barisnya. Masing-masing adalah satu langkah **Urungkan**. Item terhitung disimpan bersama berkas dan dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak boleh memilikinya, karena item terhitung adalah bagian dari data yang mereka pakai bersama.

### Urutan penyelesaian

Kadang satu sel memiliki dua item terhitung. Satu berada di barisnya dan yang lain di kolomnya. Urutan penghitungannya dapat mengubah hasilnya. Misalkan item baris adalah `=Timur-Barat` dan item kolom adalah `=Jan+1`. Jika dihitung dengan satu urutan, sel itu bernilai (Timur - Barat) + 1. Dengan urutan lain, nilainya (Timur + 1) - (Barat + 1), yang merupakan angka berbeda.

**Urutan Penyelesaian** pada tab **Tabel Pivot** mencantumkan item terhitung sesuai urutan penghitungannya. Item yang lebih rendah dalam daftar dihitung terakhir. Pilih sebuah item lalu klik **Pindah ke Atas** atau **Pindah ke Bawah**. Urutan ini disimpan bersama berkas sebagai urutan item, dan program lain membacanya dengan cara yang sama. **Daftar Rumus** menampilkan posisi setiap item di dalamnya.

## Tampilkan baris di balik sebuah nilai

Klik ganda sebuah nilai pada tabel pivot, atau pilih **Tampilkan Detail** pada tab **Tabel Pivot**. JeffCrunch membuat lembar baru, dinamai Detail 1, 2, dan seterusnya, berisi baris-baris daftar yang membentuk angka tersebut. Nama bidang berada di baris pertama. F2 tetap dapat mengedit sel, jika sel itu dapat diedit. **Urungkan** menghapus baris-baris tersebut, dan **Urungkan** sekali lagi menghapus lembarnya.

## Perluas dan ciutkan

Bidang yang item-itemnya memiliki item lain di bawahnya dapat menampilkan atau menyembunyikan baris di bawah itemnya. **Perluas Seluruh Bidang** dan **Ciutkan Seluruh Bidang** melakukannya untuk setiap item dari bidang yang Anda pilih di panel.

## Opsi Tabel Pivot

**Opsi** pada tab **Tabel Pivot** membuka kotak dengan pilihan tabel.

- **Nama** adalah nama tabel pivot. Tidak ada tabel pivot lain dalam buku kerja yang boleh memakai nama itu.
- **Untuk nilai galat tampilkan** dan **Untuk sel kosong tampilkan** memasukkan teks sendiri ke dalam sel-sel tersebut.
- **Gabungkan dan pusatkan sel dengan label**, **Sesuaikan lebar kolom saat tabel diperbarui**, dan **Pertahankan format sel saat tabel diperbarui** disimpan bersama tabel.
- **Tampilkan total keseluruhan untuk baris** dan **untuk kolom** sama dengan pilihan Total Keseluruhan.
- **Segarkan data saat berkas dibuka** membaca ulang sel setiap kali buku kerja dibuka. Ini adalah satu langkah Urungkan, seperti penyegaran lainnya.

## Gaya: header, pita, dan galeri

**Opsi Gaya** pada tab **Tabel Pivot** menyalakan atau mematikan baris header, kolom pertama (header baris), dan pita baris serta kolom. **Gaya** memiliki galeri tampilan, dan **Hapus Gaya** menghilangkan tampilan itu. Tampilan ini digambar dengan cara yang sama seperti tampilan tabel. **Baris Kosong** dan **Ulangi Label** mengatur baris kosong dan label yang diulang untuk setiap bidang baris sekaligus.

## Rancangan: subtotal, total keseluruhan, dan tata letak

- **Subtotal** menampilkan atau menyembunyikan total setiap kelompok, di bagian atas atau bawah kelompok. Jika ada lebih dari satu bidang di Baris, total setiap bidang luar merupakan subtotal.
- **Total Keseluruhan** memiliki empat pilihan: mati untuk baris dan kolom, nyala untuk keduanya, nyala untuk baris saja (satu kolom total di sebelah kanan, untuk setiap baris), dan nyala untuk kolom saja (satu baris total di bagian bawah).
- **Tata Letak Laporan** adalah bentuk label. Dalam bentuk **ringkas**, semua bidang Baris berbagi satu kolom, dan kelompok baru ditampilkan sedikit ke kanan. Dalam bentuk **garis besar**, setiap bidang memiliki kolomnya sendiri, dan setiap kelompok memiliki baris tersendiri di atas itemnya. Dalam bentuk **tabel**, setiap bidang memiliki kolomnya sendiri dan nama kelompok berada di baris pertamanya, dengan subtotal di bawah kelompok.

## Daftar yang sama, dengan filter

Pindahkan **Bulan** dari **Kolom** ke **Filter**. Laporan kehilangan kolomnya, dan sebuah baris di atasnya menampilkan **Bulan** dan **(Semua)**. Angkanya kini berupa total setiap wilayah untuk semua bulan.

## Bagan pivot

Klik sebuah sel di tabel pivot, lalu pilih **Sisipkan**, kemudian **Bagan Pivot**. JeffCrunch membuat bagan kolom dari nilai tabel. Bagan itu tidak menyertakan total keseluruhan. Bagan ditandai dalam berkas sebagai bagan milik tabel pivot tersebut. Bagan membaca sel tabel, sehingga angkanya berubah ketika Anda menyegarkan tabel atau mengubah tata letaknya. Jika tabel bertambah baris atau kolom, buat bagan lagi, karena bagan menyimpan sel tempat ia dibuat. Bagan pivot yang berasal dari berkas ditampilkan sebagaimana tersimpan.

## Pengiris dan linimasa

Pengiris adalah sekumpulan tombol yang menyaring tabel pivot, dan linimasa adalah penggeser yang melakukan hal yang sama untuk tanggal.

Klik sebuah sel di tabel pivot, lalu klik **Pengiris** pada tab Tabel Pivot. Panel Pengiris terbuka dan mencantumkan pengiris dan linimasa dalam buku kerja. Pengiris memiliki satu tombol untuk setiap item bidangnya. Tombol yang berwarna biru adalah item yang ditampilkan tabel.

Klik sebuah tombol untuk hanya menampilkan item itu. Klik lagi untuk menampilkan semua item. Tahan **Ctrl** dan klik untuk menambahkan item ke yang sudah ditampilkan, atau menghapusnya. Setiap klik adalah satu langkah Urungkan, dan Urungkan mengembalikan tabel. Ketika Anda menyimpan, berkas mempertahankan pilihan pengiris.

**Koneksi Laporan** menyebutkan tabel pivot mana yang disaring oleh sebuah pengiris. Linimasa menampilkan rentang tanggal yang dipilihnya. JeffCrunch menampilkan pengiris di panel ini, bukan di atas sel, dan belum mengubah linimasa. Keduanya disimpan dalam berkas sebagaimana tersimpan.

## Batas

Tabel pivot dapat memiliki hingga 2.000 bidang, daftar hingga lima juta baris, dan laporan hingga empat juta sel. Berkas yang melebihi batas ini ditampilkan sebagaimana tersimpan dan tidak dapat diubah. Membaca seratus ribu baris dengan dua puluh kolom dan menghitung laporannya kembali memakan waktu sekitar setengah detik pada komputer tempat pengukuran ini dilakukan; komputer yang lebih lambat membutuhkan waktu lebih lama.
