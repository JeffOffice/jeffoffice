# Tabel pivot
<!-- keywords: file lain, buku kerja lain, sumber eksternal, rumus daftar, bidang terhitung, item terhitung, urutan penyelesaian, bidang rumus, halaman filter laporan, tabel pivot, telusuri detail, telusuri ringkasan, sisipkan tabel pivot, buat, pilih, pindahkan tabel pivot, getpivotdata, buat getpivotdata, segarkan, segarkan semua, ubah sumber data, daftar bidang, bidang, baris, kolom, nilai, filter, filter laporan, total keseluruhan, subtotal, tata letak, ringkas, garis besar, tabular, hapus, pengaturan bidang nilai, jumlah, hitung, rata-rata, tampilkan nilai sebagai, persen dari total, selisih, total berjalan, format angka, pengaturan bidang, urutkan, filter, teratas, kelompokkan, pisahkan kelompok, tanggal, bulan, tahun, perluas, ciutkan, opsi, gaya, baris berpita, baris kosong, ulangi label, segarkan saat file dibuka, bagan pivot, pengiris, linimasa, tampilkan detail, telusuri detail, baris di balik nilai -->

Tabel pivot adalah laporan kecil yang dibuat dari sebuah daftar panjang. Daftar itu, misalnya, memiliki satu baris untuk setiap penjualan; laporan menjumlahkan penjualan itu menurut wilayah dan bulan. Anda tidak perlu menulis rumus apa pun. Anda cukup menentukan kolom mana dari daftar yang diletakkan di sisi kiri, mana yang diletakkan di bagian atas, dan mana yang dijumlahkan. Laporan itu lalu terisi sendiri.

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

Tabel pivot dengan **Wilayah** di sisi kiri, **Bulan** di bagian atas, dan jumlah **Jumlah** di bagian tengah tampak seperti ini.

| Jumlah dari Jumlah | Bulan | | |
|---|---|---|---|
| Wilayah | Jan | Feb | Total Keseluruhan |
| Timur | 15 | 20 | 35 |
| Barat | 7 | 7 | 14 |
| Total Keseluruhan | 22 | 27 | 49 |

Timur pada Januari bernilai 15 karena dua baris Timur untuk Januari, 10 dan 5, dijumlahkan. Kolom Total Keseluruhan menjumlahkan setiap baris, baris Total Keseluruhan menjumlahkan setiap kolom, dan sel di pojok menjumlahkan semuanya.

Istilah yang dipakai di sini:

- **Bidang** adalah sebuah kolom dari daftar. Wilayah, Bulan, dan Jumlah adalah bidang.
- **Baris**, **Kolom**, dan **Nilai** adalah tiga tempat bidang dapat diletakkan. Bidang di Baris menghasilkan satu baris untuk setiap entri yang berbeda; bidang di Kolom menghasilkan satu kolom untuk setiap entri; bidang di Nilai adalah yang dijumlahkan. Tempat keempat, **Filter**, berisi bidang yang menyaring baris yang tidak Anda pilih.
- **Cache** (cache pivot) adalah salinan daftar Anda yang disimpan tabel pivot di dalam berkas, sehingga dapat menampilkan total tanpa membaca sel. Laporan dihitung dari cache, bukan dari sel, sehingga tidak berubah ketika sel berubah. **Segarkan** menyalin ulang sel ke dalam cache.

## Membuat tabel pivot

1. Klik sebuah sel di dalam daftar, atau pilih sel yang ingin dipakai. Daftar memerlukan baris nama di atas datanya.
2. Pilih **Sisipkan**, lalu **Tabel Pivot**.
3. Kotak akan menampilkan sel tempat data berada. Ubah jika tidak sesuai. Anda juga dapat mengetik nama sebuah tabel. Untuk memakai daftar yang ada di berkas lain, pilih **Rentang dalam berkas lain**, pilih berkas dengan **Telusuri**, lalu ketik rentangnya, atau nama tabel atau nama yang Anda buat di berkas itu, misalnya `Sheet1!A1:D100`. JeffCrunch membaca buku kerja (`.xlsx`, `.xlsm`, `.xls`, dan `.ods`) dan berkas `.csv`.
4. Pilih tempat tabel pivot akan diletakkan: **Lembar baru** (bawaan), atau sebuah sel di lembar ini atau lembar lain. Pilih tempat yang kosong, karena tabel kosong menempati sekitar delapan belas baris dan tiga kolom.
5. Pilih **OK**. Tabel mulai dalam keadaan kosong, dan panel **Bidang Tabel Pivot** terbuka. Centang bidang yang Anda inginkan: bidang berisi angka akan dijumlahkan, dan bidang lainnya menjadi baris. Kemudian pindahkan bidang ke kotak seperti dijelaskan pada bagian berikut.

Jika data berada di berkas lain, tabel pivot menyimpan salinannya, sehingga tabel tetap berfungsi dan tersimpan tanpa berkas itu. Lihat Data di berkas lain di bawah.

Lembar baru adalah satu langkah Urungkan tersendiri, begitu juga tabelnya. Ketika Anda menyimpan sebagai .xlsx, tabel ditulis seperti cara program asalnya menulisnya, sehingga terbuka di sana sebagai tabel pivot. Salinan yang disimpan sebagai .xls atau .ods menyimpan angka yang ditampilkan tabel di sel-selnya, tetapi tidak tabel itu sendiri, dan penyimpanan akan memberi tahu Anda.

## GETPIVOTDATA: membaca satu angka dari tabel pivot

`GETPIVOTDATA` memberikan angka yang ditampilkan tabel pivot untuk nilai, baris, dan kolom yang Anda sebutkan. Untuk contoh di atas:

`=GETPIVOTDATA("Jumlah dari Jumlah"; $E$3; "Wilayah"; "Timur"; "Bulan"; "Jan")` memberikan 15.

- Argumen pertama adalah nama nilai, seperti yang ditampilkan tabel (misalnya Jumlah dari Jumlah). Nama bidang yang dijumlahkan (Jumlah) juga dapat dipakai.
- Argumen kedua adalah sel mana pun dari tabel pivot.
- Setelah itu datang pasangan bidang dan salah satu itemnya. Jika Anda hanya menyebut Wilayah dan Timur, Anda mendapat total Timur, yaitu 35. Jika tidak menyebut apa pun, Anda mendapat total keseluruhan, yaitu 49. Bidang yang Anda sebutkan harus merupakan bidang pertama pada baris, atau bidang pertama pada kolom, sesuai urutannya.
- Jika tabel tidak menampilkan angka tersebut, sel menampilkan `#REF!`.

Ketika Anda mengklik nilai tabel pivot saat mengetik rumus, JeffCrunch menulis rumus ini untuk Anda, sehingga rumus tetap membaca angka yang benar setelah tabel diatur ulang. Untuk mematikannya, pilih **Opsi** pada tab **Tabel Pivot**, lalu hapus centang pada kotak tentang GETPIVOTDATA.

## Apa yang dilakukan JeffCrunch terhadap tabel pivot

Tabel pivot dalam buku kerja ditampilkan seperti saat disimpan. Jika Anda tidak mengubah apa pun, berkas disimpan dengan tabel pivot persis seperti semula. Ketika Anda menyegarkan atau mengubahnya, JeffCrunch menghitung ulang laporan, memasukkan angka baru ke dalam sel, dan menulis tabel pivot kembali agar program yang membuatnya dapat membukanya tanpa keluhan.

JeffCrunch menghitung tabel pivot yang menjumlahkan, menghitung, merata-ratakan, dan sebagainya; yang mengelompokkan tanggal dan angka; dan yang menampilkan nilai sebagai persentase, selisih, atau total berjalan. Tabel pivot yang menampilkan nilai sebagai peringkat atau sebagai bagian dari induknya, memiliki kelompok item yang dibuat secara manual, atau memakai model data, ditampilkan dan disimpan seperti aslinya, dan perintah akan memberi tahu Anda bahwa perintah tersebut tidak dapat mengubahnya.

Klik sel mana pun pada tabel pivot. Tab **Tabel Pivot** akan muncul.

## Segarkan

**Segarkan** membaca sel tempat tabel pivot dibuat dan menghitung ulang laporan. Entri baru ditambahkan setelah entri yang sudah ada; entri yang sudah tidak ada akan dihapus. Lebar kolom dan tata letak yang Anda pilih tetap dipertahankan. **Segarkan Semua**, pada tab **Data** dan tab **Tabel Pivot**, melakukan hal yang sama untuk setiap tabel pivot dalam buku kerja.

Jika laporan akan tumbuh menimpa sel yang sudah berisi sesuatu, JeffCrunch tidak menyegarkan. Program akan memberi tahu sel mana yang menghalangi. Hapus isinya atau pindahkan tabel pivot, lalu segarkan lagi.

**Urungkan** membatalkan penyegaran dalam satu langkah.

## Ubah Sumber Data

**Ubah Sumber Data** meminta sel tempat daftar berada, termasuk baris nama pertamanya. Ketikkan seperti `Sheet1!A1:D100`, atau ketik nama sebuah tabel atau nama yang Anda buat. JeffCrunch membaca sel tersebut dan menghitung ulang laporan. Jika sebuah bidang daftar memiliki nama yang sama di sel baru, tempatnya dalam laporan tetap dipertahankan.

Anda juga dapat mengarahkan tabel ke rentang di berkas lain. Di dalam kotak, pilih **Rentang dalam berkas lain**, pilih berkasnya, lalu ketik rentangnya di sana. Memilih **Rentang dalam buku kerja ini** akan mengarahkan tabel kembali ke sel di sini. Kedua perubahan ini adalah satu langkah Urungkan.

## Data di berkas lain

Tabel pivot yang dibuat dari berkas lain menyimpan data asalnya di dalam buku kerja ini, sehingga Anda dapat membuka, mengubah, dan menyimpan buku kerja ketika berkas lain tidak ada. **Segarkan** membaca ulang berkas lain. Jika berkas sudah dipindahkan, hilang, terlalu besar, bukan buku kerja, atau tidak lagi memiliki rentang tersebut, tabel tetap menyimpan data yang dimilikinya, dan sebuah pesan menyatakan bahwa berkas tidak dapat dibaca. Berkas yang berada di komputer lain di jaringan Anda tidak dibaca sampai Anda sendiri memilihnya di **Ubah Sumber Data**. Satu penyegaran membaca setiap berkas sekali, dan paling banyak 32 berkas, 1 GB, serta 30 detik secara keseluruhan; tabel yang melewati batas itu tetap menyimpan datanya.

Jika sebuah buku kerja menyatakan bahwa tabelnya harus dibaca ulang saat dibuka, dan data tabel berada di berkas lain, JeffCrunch akan bertanya terlebih dahulu, dengan cara yang sama seperti untuk tautan ke buku kerja lain. Sampai Anda menjawab ya, tidak ada berkas yang dibaca.

Buku kerja yang disimpan sebagai .xlsx menyimpan lokasi berkasnya. Memilih berkas di dalam kotak akan menyimpan alamat lengkapnya. Buku kerja yang dibuat program lain, yang tabelnya mengarah ke berkas dengan jalur pendek seperti `Harga.xlsx`, tetap menyimpan jalur itu, dan JeffCrunch mencari berkas di folder yang sama dengan buku kerja. Ketika Anda memakai **Simpan Sebagai** untuk menyimpan buku kerja di folder lain, jalur seperti itu ditulis ulang agar tetap menunjuk berkas yang sama. Penyegaran tidak mengikuti tautan yang dimiliki berkas lain ke berkas-berkas lainnya.

## Hapus

**Hapus** memiliki dua pilihan. **Hapus Filter** menghapus setiap filter dan menampilkan kembali semua entri. **Hapus Semua** mengeluarkan setiap bidang dari laporan dan membuat laporan kosong, siap dibangun kembali.

## Telusuri ringkasan dan telusuri detail

## Halaman Filter Laporan

Filter berada di atas tabel pivot dan memungkinkan Anda melihat satu item pada satu waktu, misalnya satu bulan. **Tampilkan Halaman Filter Laporan** pada tab **Tabel Pivot** membuat halaman untuk setiap item sekaligus. Setiap halaman adalah lembar baru, dinamai sesuai item, dengan salinan tabelnya sendiri. Filter pada setiap salinan diatur ke item tersebut. Jika tabel memiliki lebih dari satu filter, Anda memilih filter yang akan dipakai. Satu **Urungkan** menghapus semua halaman.

Ketika laporan memiliki lebih dari satu bidang pada baris, sebuah item dari bidang pertama dapat dibuka atau ditutup. Klik sel yang berisi nama item dan pilih **Telusuri Ringkasan** untuk menutupnya: baris bidang berikutnya di bawahnya disembunyikan dan hanya totalnya yang tersisa. **Telusuri Detail** membukanya kembali. Item dari bidang terakhir tidak memiliki apa pun di bawahnya. Setiap tindakan adalah satu **Urungkan**.

## Pilih dan pindahkan

**Pilih** pada tab **Tabel Pivot** memilih sebagian laporan: **Seluruh Tabel Pivot**, **Nilai** (angka), atau **Label** (nama di sisi kiri; jika laporan tidak memiliki nama di sisi kiri, nama di bagian atas). **Pindahkan Tabel Pivot…** meminta sel pada lembar yang sama dan menggambar ulang laporan dengan sel pertamanya di sana. Sel yang ditinggalkan dikosongkan. Rumus yang mengarah ke sel laporan akan mengikutinya, sama seperti saat Anda memotong dan menempel. Jika tempat baru sudah berisi sesuatu, JeffCrunch tidak memindahkan laporan dan memberi tahu Anda. Satu **Urungkan** memindahkannya kembali.

## Panel bidang

**Daftar Bidang** pada tab **Tabel Pivot** membuka panel **Bidang Tabel Pivot**. Panel ini menampilkan bidang dari daftar dengan kotak di sebelahnya, dan di bawahnya empat tempat: **Filter**, **Kolom**, **Baris**, dan **Nilai**.

- Centang sebuah bidang untuk menambahkannya. Bidang yang hanya berisi angka masuk ke **Nilai** dan dijumlahkan; bidang lainnya masuk ke **Baris**.
- Hapus centang sebuah bidang untuk mengeluarkannya dari semua tempat.
- Klik sebuah bidang di salah satu dari empat tempat, lalu pilih **Pindahkan Ke** untuk memasukkannya ke tempat lain. Kata **Pindah Atas** dan **Pindah Bawah** mengubah urutannya di tempatnya, dan **Hapus Bidang** mengeluarkannya.
- Jika ada lebih dari satu nilai, sebuah baris bernama **Nilai** muncul di **Kolom**. Pindahkan ke **Baris** untuk meletakkan nilai di sisi kiri.
- **Pengaturan Bidang Nilai** (klik dua kali sebuah nilai, atau tombolnya) mengatur nama yang ditampilkan laporan untuk nilai dan cara penjumlahannya: **Jumlah**, **Hitung**, **Hitung Angka**, **Rata-rata**, **Maks**, **Min**, **Hasil Kali**, **StdDev** dan **StdDevp** (simpangan baku sampel, atau dari semua nilai), serta **Var** dan **Varp** (varians dari keduanya). Pengaturan ini juga mengatur cara nilai ditampilkan dan format angkanya. Lihat bagian berikutnya.
- **Pengaturan Bidang** (klik dua kali sebuah bidang di Filter, Kolom, atau Baris, atau tombolnya) mengatur satu bidang. Lihat "Pengaturan Bidang".

Setiap perubahan adalah satu langkah **Urungkan**.

## Tampilkan Nilai Sebagai, dan format angka sebuah nilai

Di **Pengaturan Bidang Nilai**, **Tampilkan nilai sebagai** mengubah apa yang ditampilkan sebuah sel laporan.

- **% dari Total Keseluruhan**: sel dibagi total seluruh laporan. Dalam contoh, Timur pada Januari adalah 15 dari 49, yaitu 30,6%.
- **% dari Total Kolom** dan **% dari Total Baris**: sel dibagi total kolomnya, atau total barisnya.
- **Indeks**: seberapa besar sel dibandingkan dengan nilai yang akan didapat baris dan kolomnya jika data tersebar merata. Angka di atas 1 berarti lebih banyak dari bagian yang merata.
- **Selisih Dari**, **% Dari**, dan **% Selisih Dari**: sel dibandingkan dengan item lain dari sebuah bidang. Pilih **Bidang dasar** (bidang di Baris atau Kolom) dan **Item dasar** (salah satu itemnya, atau item sebelum atau sesudahnya). Misalnya, dengan Bulan sebagai bidang dasar dan Jan sebagai item dasar, Februari menampilkan seberapa banyak lebih atau kurang dibanding Januari. Item dasar itu sendiri dibiarkan kosong untuk selisih, dan bernilai 100% untuk **% Dari**.
- **Total Berjalan Dalam**: setiap sel menjumlahkan sel-sel sebelumnya sepanjang bidang dasar.
- **Peringkat dari Terkecil ke Terbesar** dan **Peringkat dari Terbesar ke Terkecil**: setiap sel menampilkan urutannya, 1, 2, 3, dan seterusnya, di antara sel-sel bidang dasar pada baris atau kolom yang sama. Peringkat 1 adalah angka terkecil, atau terbesar.
- **% dari Total Baris Induk** dan **% dari Total Kolom Induk**: setiap sel menampilkan bagian induknya yang diwakilinya. Induk sebuah item adalah item satu tingkat di atasnya, seperti tahun yang memuat sebuah bulan. Item tanpa induk ditampilkan sebagai bagian dari total keseluruhan.

**Format angka** menerima kode format, seperti `0,00`, `#.##0`, atau `0,0%`. Biarkan kosong untuk mempertahankan format yang dimiliki sel. Nilai yang ditampilkan sebagai persentase mendapat `0,00%` kecuali Anda memilih format. Sel laporan langsung memakai format tersebut. **Urungkan** mengembalikannya bersama angkanya.

## Pengaturan Bidang

Pilih sebuah bidang pada kotak **Filter**, **Kolom**, atau **Baris**, lalu pilih **Pengaturan Bidang** pada tab **Tabel Pivot** (atau klik dua kali).

- **Nama kustom** mengubah nama yang ditampilkan laporan untuk bidang.
- **Subtotal**: **Otomatis** menampilkan total untuk setiap kelompok bidang, **Tidak ada** tidak menampilkannya.
- **Tampilkan label item dalam**: bentuk ringkas, garis besar, atau tabel, seperti pada Tata Letak Laporan.
- **Sisipkan baris kosong setelah setiap item** menempatkan baris kosong setelah setiap kelompok.
- **Ulangi label item** menampilkan nama item luar pada setiap baris kelompoknya (bentuk garis besar dan tabel).
- **Urutkan**: pertahankan urutan item (manual), atau urutkan A ke Z atau Z ke A menurut label. **Terkecil ke terbesar menurut nilai** dan **Terbesar ke terkecil menurut nilai** mengurutkan item berdasarkan angka pada bidang nilai. Laporan mempertahankan pengurutan saat data berubah.
- **Filter** menyaring item berdasarkan labelnya (diawali dengan, diakhiri dengan, berisi, sama dengan) atau berdasarkan nilai (lebih besar dari, lebih kecil dari, di antara, dan sebagainya), atau hanya menyimpan beberapa item teratas atau terbawah berdasarkan nilai. Filter dihitung ketika Anda memilih OK. Item yang ditinggalkan disembunyikan; kotak centang di bawah menunjukkan item mana, dan Anda dapat mencentang item untuk menampilkannya kembali.
- **Tampilkan item ini** adalah daftar item bidang dengan satu kotak centang di setiap item. Hapus centang untuk menyembunyikan item, dan total akan mengabaikannya.

## Kelompokkan tanggal dan angka

Pilih bidang berisi tanggal atau angka di **Baris** atau **Kolom**, lalu pilih **Kelompokkan Bidang** pada tab **Tabel Pivot**. Kelompok dibuat dari data, sehingga kolom tersebut hanya boleh berisi angka atau tanggal, tanpa teks dan tanpa sel kosong.

- **Tanggal**: centang bagian yang akan dikelompokkan: **Detik**, **Menit**, **Jam**, **Hari**, **Bulan**, **Kuartal**, atau **Tahun**. Bagian paling rinci yang Anda centang tetap berada pada bidang. Untuk setiap bagian yang lebih kasar, JeffCrunch menambahkan bidang yang dinamai sesuai bagian itu (misalnya **Tahun**), dan meletakkannya sebelum bidang tanggal pada tempat yang sama. **Mulai dari** dan **Berakhir pada** adalah tanggal pertama dan terakhir; biarkan seperti semula untuk memakai tanggal paling awal dan paling akhir dalam data. Tanggal sebelum awal atau sesudah akhir masuk ke dua kelompok tambahan, yang diawali dengan `<` dan `>`.
- **Angka**: **Mulai dari**, **Berakhir pada**, dan **Per** membuat kelompok seperti 1-10, 11-20, dan seterusnya.

**Pisahkan Kelompok** menghapus kelompok dan bidang yang ditambahkan untuk kelompok tersebut. Penyegaran membuat kelompok kembali dari data baru. Jika dua tabel pivot dibuat dari data yang sama, kelompok tidak dapat dibuat, karena kelompok adalah bagian dari data yang mereka pakai bersama.

## Bidang terhitung

Bidang terhitung adalah bidang yang Anda buat dari sebuah rumus. Bidang ini memakai bidang lain dari daftar. Misalnya, bonus sebesar sepersepuluh dari jumlah adalah `=Jumlah*0,1`.

Pilih **Bidang Terhitung** pada tab **Tabel Pivot**. Ketik nama dan rumus. Tuliskan nama bidang di dalam rumus, atau klik dua kali pada bidang di daftar **Bidang** untuk memasukkannya. Tuliskan nama yang mengandung spasi di antara tanda kutip tunggal, seperti `='Harga satuan'*Jumlah`. Anda dapat memakai angka, `+ - * / ^ %`, perbandingan, dan beberapa fungsi seperti `IF`, `ROUND`, `SUM`, `MIN`, `MAX`, `AVERAGE`, dan `ABS`.

Pilih **Tambah**. Bidang baru ditempatkan pada kotak **Nilai**. Bidang ini juga muncul di daftar bidang, tempat Anda dapat mencentang atau menghapus centangnya seperti bidang lain. Bidang ini tidak dapat ditempatkan di baris, kolom, atau filter.

Bidang terhitung dihitung pada setiap sel laporan. Nama bidang mewakili total bidang itu pada sel, sehingga `=Jumlah*Jumlah` mengkuadratkan total sel. Rumus ini tidak mengkuadratkan setiap baris lalu menjumlahkannya. Total keseluruhan memakai total seluruh daftar.

**Daftar Rumus** (di samping **Bidang Terhitung**) membuat lembar baru yang menyebutkan setiap bidang terhitung beserta rumusnya, dan setiap item terhitung beserta rumusnya, bidangnya, dan urutan penyelesaiannya. **Urungkan** menghapus sel-sel tersebut, dan **Urungkan** lagi menghapus lembarnya.

Untuk mengubah bidang, pilih namanya pada kotak **Nama**. Rumusnya akan tampil. Ubah rumus lalu pilih **Ubah**. **Hapus** menghapus bidang dan nilainya. Setiap tindakan adalah satu langkah **Urungkan**. Bidang terhitung disimpan bersama berkas dan tetap dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak dapat memilikinya, karena bidang terhitung adalah bagian dari data yang mereka pakai bersama.

## Item terhitung

Item terhitung adalah item yang Anda buat dari item lain pada bidang yang sama. Misalnya, bidang Wilayah memiliki item Timur dan Barat. Item baru bernama Keduanya dapat berupa `=Timur+Barat`.

Klik sebuah sel pada tabel pivot, lalu pilih **Item Terhitung** pada tab **Tabel Pivot**. Pilih **Bidang**. Bidang itu harus berada di baris atau kolom. Ketik **Nama** dan **Rumus**. Tuliskan nama item di dalam rumus, atau klik dua kali pada item di daftar **Item** untuk memasukkannya. Tuliskan nama yang mengandung spasi di antara tanda kutip tunggal, seperti `='Timur besar'+Barat`. Anda dapat memakai angka, `+ - * / ^ %`, dan tanda kurung.

Item baru mendapat satu baris pada laporan. Di setiap kolom, item ini menampilkan rumus yang dihitung dari baris item yang disebutkannya. Item baru ikut dihitung dalam total. Total hasil penjumlahan, seperti total keseluruhan atau subtotal, menambahkan baris item baru ke baris item lainnya. Jadi, dengan Timur 35, Barat 14, dan Keduanya (Timur ditambah Barat) 49, total keseluruhan adalah 98. Jika item yang disebut rumus disembunyikan, sel menampilkan **#REF!**. Jika rumus menyebut item yang tidak ada, sel menampilkan **#NAME?**.

Untuk mengubah item, pilih namanya pada kotak **Nama**. Rumusnya akan tampil. Ubah rumus lalu pilih **Ubah**. **Hapus** menghapus item dan barisnya. Setiap tindakan adalah satu langkah **Urungkan**. Item terhitung disimpan bersama berkas dan tetap dipertahankan ketika laporan disegarkan. Dua tabel pivot yang dibuat dari data yang sama tidak dapat memilikinya, karena item terhitung adalah bagian dari data yang mereka pakai bersama.

### Urutan penyelesaian

Kadang satu sel memiliki dua item terhitung. Yang satu berada di barisnya dan yang lain di kolomnya. Urutan penghitungannya dapat mengubah hasilnya. Misalkan item baris adalah `=Timur-Barat` dan item kolom adalah `=Jan+1`. Jika dihitung dengan satu urutan, sel bernilai (Timur - Barat) + 1. Dengan urutan lain, sel bernilai (Timur + 1) - (Barat + 1), yang merupakan angka berbeda.

**Urutan Penyelesaian** pada tab **Tabel Pivot** menampilkan item terhitung sesuai urutan penghitungannya. Item yang lebih rendah dalam daftar dihitung paling akhir. Pilih sebuah item lalu klik **Pindah Atas** atau **Pindah Bawah**. Urutan ini disimpan bersama berkas sebagai urutan item, dan program lain membacanya dengan cara yang sama. **Daftar Rumus** menampilkan posisi setiap item di dalamnya.

## Tampilkan baris di balik sebuah nilai

Klik dua kali sebuah nilai pada tabel pivot, atau pilih **Tampilkan Detail** pada tab **Tabel Pivot**. JeffCrunch membuat lembar baru, dinamai Detail 1, 2, dan seterusnya, yang berisi baris-baris daftar yang membentuk angka tersebut. Nama bidang ditulis pada baris pertama. F2 tetap dapat mengedit sel, jika sel tersebut dapat diedit. **Urungkan** menghapus baris-baris itu, dan **Urungkan** lagi menghapus lembarnya.

## Perluas dan ciutkan

Bidang yang item-itemnya memiliki item di bawahnya dapat menampilkan atau menyembunyikan baris di bawah itemnya. **Perluas Seluruh Bidang** dan **Ciutkan Seluruh Bidang** melakukannya untuk setiap item dari bidang yang Anda pilih di panel.

## Opsi Tabel Pivot

**Opsi** pada tab **Tabel Pivot** membuka kotak dengan pilihan untuk tabel.

- **Nama** adalah nama tabel pivot. Tidak ada tabel pivot lain dalam buku kerja yang boleh memakai nama itu.
- **Untuk nilai galat tampilkan** dan **Untuk sel kosong tampilkan** memasukkan teks Anda sendiri ke dalam sel tersebut.
- **Gabungkan dan pusatkan sel dengan label**, **Sesuaikan lebar kolom saat tabel diperbarui**, dan **Pertahankan format sel saat tabel diperbarui** disimpan bersama tabel.
- **Tampilkan total keseluruhan untuk baris** dan **untuk kolom** sama dengan pilihan Total Keseluruhan.
- **Segarkan data saat berkas dibuka** membaca sel ulang setiap kali buku kerja dibuka. Ini adalah satu langkah Urungkan, seperti penyegaran lainnya.

## Gaya: header, pita, dan galeri

**Opsi Gaya** pada tab **Tabel Pivot** menghidupkan atau mematikan baris header, kolom pertama (header baris), serta pita baris dan kolom. **Gaya** memiliki galeri tampilan, dan **Hapus Gaya** menghilangkan tampilan itu. Tampilan ini digambar dengan cara yang sama seperti tampilan tabel. **Baris Kosong** dan **Ulangi Label** mengatur baris kosong dan label yang diulang untuk setiap bidang baris sekaligus.

## Desain: subtotal, total keseluruhan, dan tata letak

- **Subtotal** menampilkan atau menyembunyikan total setiap kelompok, di bagian atas atau bawah kelompok. Ketika ada lebih dari satu bidang di Baris, total setiap bidang luar menjadi subtotal.
- **Total Keseluruhan** memiliki empat pilihan: mati untuk baris dan kolom, hidup untuk keduanya, hidup hanya untuk baris (satu kolom total di sebelah kanan, untuk setiap baris), dan hidup hanya untuk kolom (satu baris total di bagian bawah).
- **Tata Letak Laporan** adalah bentuk label. Dalam bentuk **ringkas**, semua bidang Baris berbagi satu kolom, dan kelompok baru ditampilkan sedikit ke kanan. Dalam bentuk **garis besar**, setiap bidang memiliki kolomnya sendiri, dan setiap kelompok memiliki baris tersendiri di atas itemnya. Dalam bentuk **tabel**, setiap bidang memiliki kolomnya sendiri dan nama kelompok berada di baris pertamanya, dengan subtotal di bawah kelompok.

## Daftar yang sama, sebuah filter

Pindahkan **Bulan** dari **Kolom** ke **Filter**. Laporan kehilangan kolom-kolomnya, dan sebuah baris di atasnya menampilkan **Bulan** dan **(Semua)**. Angkanya sekarang adalah total setiap wilayah untuk semua bulan.

## Bagan pivot

Klik sebuah sel pada tabel pivot lalu pilih **Sisipkan**, kemudian **Bagan Pivot**. JeffCrunch membuat bagan kolom dari nilai tabel. Total keseluruhan tidak disertakan. Bagan ditandai sebagai bagan tabel pivot di dalam berkas. Bagan membaca sel tabel, sehingga angkanya berubah ketika Anda menyegarkan tabel atau mengubah tata letaknya. Jika tabel bertambah baris atau kolom, buat bagan lagi, karena bagan menyimpan sel tempat ia dibuat. Bagan pivot yang berasal dari berkas ditampilkan seperti saat disimpan.

## Pengiris dan linimasa

Pengiris adalah sekumpulan tombol yang menyaring tabel pivot, sedangkan linimasa adalah penggeser yang melakukan hal yang sama untuk tanggal.

Klik sebuah sel pada tabel pivot, lalu klik **Pengiris** pada tab Tabel Pivot. Panel Pengiris terbuka dan menampilkan pengiris dan linimasa dalam buku kerja. Sebuah pengiris memiliki satu tombol untuk setiap item bidangnya. Tombol berwarna biru adalah item yang ditampilkan tabel.

Klik sebuah tombol untuk hanya menampilkan item itu. Klik lagi untuk menampilkan semua item. Tahan **Ctrl** lalu klik untuk menambahkan item ke yang sudah ditampilkan, atau untuk menghapus satu item. Setiap klik adalah satu langkah Urungkan, dan Urungkan mengembalikan tabel. Ketika Anda menyimpan, berkas menyimpan pilihan pengiris.

Pengiris yang sudah ada di dalam berkas juga digambar di atas sel, di tempat berkas menempatkannya. Pengiris menampilkan keterangannya dan satu tombol untuk setiap item. Tombol berwarna biru adalah item yang ditampilkan tabel. Klik sebuah tombol untuk hanya menampilkan item itu, tahan **Ctrl** lalu klik untuk menambahkan atau menghapus item, dan klik corong di pojok kanan atas untuk menampilkan semua item lagi. Setiap klik adalah satu langkah Urungkan. Ketika tabel disaring dengan cara lain, atau Anda mengklik Urungkan, tombol-tombol akan mengikutinya. Jika pengiris terlalu kecil untuk menampilkan semua tombolnya, baris terakhir bertuliskan "Item lainnya" beserta jumlahnya (dan berapa yang dipilih); klik baris itu untuk membuka panel Pengiris, yang menampilkan setiap item. Pengiris yang menyaring tabel selain tabel pivot, atau yang tabel pivotnya tidak ada di dalam berkas, digambar dengan keterangannya serta satu kalimat yang menyatakan bahwa JeffCrunch belum dapat menggunakannya; pengiris itu tetap disimpan di dalam berkas seperti semula. Buku kerja yang terbuka hanya untuk dibaca, dan lembar yang dilindungi dari perubahan tabel pivot, tidak membiarkan klik menyaring tabel. Pengiris ikut tercetak bersama lembar kecuali berkas menyatakan sebaliknya. Anda belum dapat memindahkan, mengubah ukuran, atau menghapus pengiris pada lembar.

**Tampilkan Laporan Koneksi** menyatakan tabel pivot mana yang disaring oleh sebuah pengiris. Linimasa menampilkan rentang tanggal yang dipilihnya. JeffCrunch belum menggambar linimasa di atas sel atau mengubahnya. Keduanya disimpan di dalam berkas seperti saat disimpan.

## Batas

Tabel pivot dapat memiliki hingga 2.000 bidang, daftar hingga lima juta baris, dan laporan hingga empat juta sel. Berkas yang melewati batas ini ditampilkan seperti saat disimpan dan tidak dapat diubah. Membaca seratus ribu baris dengan dua puluh kolom lalu menghitung ulang laporan memakan waktu sekitar setengah detik pada komputer tempat ini diukur; komputer yang lebih lambat membutuhkan waktu lebih lama.
