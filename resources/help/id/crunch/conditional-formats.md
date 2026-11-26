# Pemformatan bersyarat
<!-- keywords: pemformatan bersyarat, format bersyarat, aturan, sorot, skala warna, bilah data, set ikon, ikon, teratas, terbawah, di atas rata-rata, di bawah rata-rata, duplikat, unik, rumus, berhenti jika benar, prioritas, batang, panah, lampu lalu lintas, gradasi, format diferensial, tombol Pemformatan Bersyarat, hapus aturan, lebih besar dari, lebih kecil dari, di antara, teks yang berisi, nilai duplikat, 10 teratas -->

**Pemformatan bersyarat** adalah aturan yang mengubah tampilan sel ketika isinya memenuhi suatu kondisi. Contoh: sebuah aturan menyatakan "beri warna merah pada sel jika nilainya di bawah 100". Sel yang berisi 80 akan berwarna merah, sel yang berisi 120 tidak, dan ketika Anda mengubah 80 menjadi 150, warna merah itu hilang dengan sendirinya. Format sel itu sendiri tidak diubah: aturan hanya menentukan apa yang ditampilkan.

## Yang ditampilkan JeffCrunch

JeffCrunch menampilkan aturan yang dimiliki sebuah buku kerja:

- **Aturan sorot**: aturan yang mengubah font, isian, bingkai, atau format angka pada sel yang memenuhinya. Kondisinya dapat berupa perbandingan dengan suatu nilai (lebih besar dari, di antara, sama dengan, dan seterusnya), rumus, teks yang berisi, diawali dengan, atau diakhiri dengan kata tertentu, tanggal seperti "minggu lalu", sel yang kosong atau berisi galat, nilai yang termasuk tertinggi atau terendah (berupa angka atau persen), nilai di atas atau di bawah rata-rata, serta nilai yang duplikat atau unik.
- **Skala warna**: setiap sel diisi warna yang berpindah di antara dua atau tiga warna sesuai dengan besar angkanya, dari rendah ke tinggi. Skala dari putih ke hijau membuat angka terbesar paling hijau.
- **Bilah data**: batang di dalam sel yang makin panjang seiring makin besarnya angka. Bilah dapat berwarna padat atau memudar, dapat memiliki bingkai dan sumbu, dan dapat berjalan ke arah sebaliknya, dengan warna lain, untuk angka negatif.
- **Set ikon**: gambar kecil di dalam sel yang menunjukkan bagian rentang tempat angka itu berada, misalnya panah ke atas, panah mendatar, atau panah ke bawah; atau lampu merah, kuning, atau hijau. Tersedia dua puluh set.

Ketika dua aturan sama-sama mengatur sebuah sel, aturan yang lebih dulu di urutan buku kerja yang menang untuk hal yang keduanya atur, dan aturan yang diberi tanda "berhenti jika benar" menghentikan pencarian. Angka pada skala warna, bilah data, atau set ikon dihitung dari sel-sel milik aturan tersebut, dan dihitung ulang setiap kali salah satu sel itu berubah.

## Yang tetap sama

- Aturan mengikuti sel yang menjadi miliknya. Sisipkan atau hapus baris atau kolom, potong dan tempel, dan aturan tetap mencakup sel yang benar; **Urungkan** mengembalikannya.
- Aturan yang tidak dapat digambar JeffCrunch tetap disimpan dalam berkas seperti semula.
- Menyimpan menulis aturan yang dimiliki buku kerja. Ketika Anda membuat atau menghapus aturan pada sebuah lembar, aturan lembar tersebut ditulis ulang beserta perubahan Anda.

## Membuat aturan

1. Pilih sel-selnya. Anda dapat memilih seluruh kolom atau seluruh lembar; aturan disimpan untuk rentang, bukan untuk setiap sel.
2. Klik **Beranda > Pemformatan Bersyarat**.
3. Di bawah **Aturan Sorot Sel**, pilih **Lebih Besar Dari**, **Lebih Kecil Dari**, **Di Antara**, **Sama Dengan**, **Teks yang Berisi**, **Tanggal Terjadi**, atau **Nilai Duplikat**. Di bawah **Aturan Teratas/Terbawah**, pilih **10 Item Teratas**, **10 Item Terbawah**, **10% Teratas**, **10% Terbawah**, **Di Atas Rata-Rata**, atau **Di Bawah Rata-Rata**.
4. Ketik nilainya (atau dua nilai untuk **Di Antara**, teks untuk **Teks yang Berisi**, atau jumlahnya untuk aturan Teratas dan Terbawah), atau pilih tanggalnya. Untuk **10 Item Teratas**, "10" adalah jumlah sel yang mendapat tampilan tersebut. Anda dapat mengubahnya menjadi angka berapa pun.
5. Pilih tampilan sel, misalnya **Isian Merah Muda dengan Teks Merah Tua**, lalu klik **OK**. Contoh di sebelah daftar memperlihatkan tampilannya.

Untuk membuat tampilan sendiri, pilih **Format Kustom...** di akhir daftar. Sebuah kotak terbuka dengan empat tab: **Angka**, **Font** (gaya, garis bawah, coret, dan warna), **Bingkai** (hanya jenis garis tipis), dan **Isian** (warna, atau pola dengan dua warna). Kotak ini dimulai dari tampilan yang tadi Anda pilih, dan bagian yang tidak Anda ubah tetap seperti semula. Klik **OK**, dan contoh akan memperlihatkan tampilan Anda.

Aturan langsung digambar. Setiap aturan baru ditempatkan paling awal, sehingga ketika dua aturan ingin mengubah hal yang sama, aturan yang lebih baru yang menang. Membuat aturan adalah satu langkah yang dapat Anda urungkan.

## Bilah data, skala warna, dan set ikon

Klik **Beranda > Pemformatan Bersyarat**, lalu **Bilah Data**, **Skala Warna**, atau **Set Ikon**, dan klik salah satu pilihan. Aturan langsung diterapkan pada sel yang dipilih, tanpa jendela yang perlu diisi, sebagai satu langkah yang dapat Anda urungkan.

- **Bilah Data** tersedia dalam enam warna, masing-masing sebagai isian gradasi atau isian padat.
- **Skala Warna** memiliki dua belas pilihan, dengan dua warna atau tiga warna. Nama pilihan menyebutkan warna untuk angka tertinggi lebih dulu.
- **Set Ikon** memiliki dua puluh set: arah (panah dan segitiga), bentuk (lampu lalu lintas dan tanda), indikator (simbol dan bendera), dan peringkat (bintang, kuartal, dan kotak). Ikon membagi rentang menjadi langkah-langkah yang sama besar.

**Aturan Baru** di akhir menu membuka satu jendela untuk setiap jenis aturan, termasuk aturan dengan rumus sendiri, serta bilah data, skala warna, atau set ikon dengan pengaturan Anda sendiri.

## Kelola Aturan

**Beranda > Pemformatan Bersyarat > Kelola Aturan** menampilkan daftar aturan lembar sesuai urutan kerjanya. Gunakan **Aturan Baru**, **Edit Aturan**, dan **Hapus Aturan**; **Pindah ke Atas** dan **Pindah ke Bawah** mengubah urutan; ketik sel lain pada **Berlaku untuk**; dan centang **Berhenti jika benar**. Edit Aturan mengubah semua yang dimiliki sebuah aturan:

- untuk **bilah data**: arti bilah terpendek dan terpanjang (nilai terendah atau tertinggi, angka, persen, persentil, atau rumus), isian gradasi atau padat, warnanya, bingkai, warna bilah negatif, letak sumbu, arah bilah, dan **Tampilkan bilah saja**;
- untuk **skala warna**: dua atau tiga warna, dan arti setiap warna;
- untuk **set ikon**: gaya ikon, nilai tempat setiap ikon dimulai, **Balik urutan ikon**, dan **Tampilkan ikon saja**;
- untuk aturan lainnya: nilai, teks, tanggal, atau rumus, serta tampilannya. Aturan yang dibawa dari berkas dengan tampilan sendiri tercantum dengan tampilan **Format Kustom**, dan contoh memperlihatkan tampilan aslinya. Jika Anda mengubah hal lain pada aturan, tampilannya tetap persis seperti yang dimiliki berkas. Pilih **Format Kustom...** untuk mengubah tampilan.

Menekan **OK** menerapkan semua perubahan Anda sebagai satu langkah yang dapat diurungkan. Aturan yang tidak Anda ubah tetap persis seperti yang dimiliki berkas, di posisi mana pun dalam daftar; aturan yang Anda ubah ditulis ulang. Rumus sebuah aturan dibaca dari sel kiri atas rentangnya, dan rumus yang Anda ketik untuk aturan baru dibaca seolah-olah ditulis di sel tempat kursor berada. Lembar yang dilindungi dan tidak mengizinkan sel diformat tidak menerima aturan.

## Yang disimpan jenis berkas lain

Hanya buku kerja .xlsx yang menyimpan pemformatan bersyarat. Menyimpan sebagai jenis berkas lama (.xls atau .ods) akan membuangnya, dan Periksa Kompatibilitas akan memberitahukannya, dengan menyebutkan bilah data, skala warna, dan set ikon jika lembar memilikinya. Berkas .csv tidak menyimpan format apa pun. Pemformatan bersyarat dicetak seperti yang tampak di layar.

## Hapus aturan

**Hapus Aturan dari Sel yang Dipilih** menghapus setiap aturan yang ada pada sel mana pun yang dipilih. **Hapus Aturan dari Lembar Ini** menghapus semua aturan pada lembar. **Hapus Aturan dari Tabel Ini** dan **Hapus Aturan dari Tabel Pivot Ini** menghapus aturan yang ada pada tabel (atau tabel pivot) tempat sel yang dipilih berada, dan hanya aktif ketika pilihan berada di dalam satu tabel. Setiap pilihan adalah satu langkah yang dapat Anda urungkan. Sebuah pilihan tidak melakukan apa-apa jika tidak ada aturan untuk dihapus.

## Yang belum didukung

- Lembar yang memiliki bilah data dalam bentuk berkas yang lebih baru tetap menyimpannya seperti semula, dan JeffCrunch tidak membuat, mengubah, atau menghapus aturan pada lembar tersebut. Lakukan perubahan itu di program yang membuat buku kerjanya.
- Buku kerja yang dibuka dari jenis berkas lama tetap mempertahankan aturannya ketika Anda menyimpannya sebagai buku kerja .xlsx, dan Anda dapat menambahkan aturan setelahnya.
- Tampilan sendiri tidak dapat memiliki garis sedang, tebal, ganda, atau miring, garis diagonal, isian gradasi, atau jenis dan ukuran font: hal-hal tersebut bukan bagian dari yang dapat diubah sebuah aturan.
