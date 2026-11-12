# Pemformatan bersyarat

<!-- keywords: pemformatan bersyarat, format bersyarat, aturan, sorot, skala warna, batang data, set ikon, ikon, teratas, terbawah, di atas rata-rata, di bawah rata-rata, duplikat, unik, rumus, berhenti jika benar, prioritas, batang, panah, lampu lalu lintas, gradasi, format diferensial, tombol Pemformatan Bersyarat, hapus aturan, lebih besar dari, lebih kecil dari, antara, teks yang berisi, nilai duplikat, 10 teratas -->

**Pemformatan bersyarat** adalah aturan yang mengubah tampilan sel saat isinya memenuhi suatu kondisi. Contoh: sebuah aturan menyatakan "beri warna merah pada sel jika nilainya di bawah 100". Sel yang berisi 80 menjadi merah, sel yang berisi 120 tidak, dan ketika Anda mengubah 80 menjadi 150, warna merah hilang dengan sendirinya. Format sel itu sendiri tidak diubah: aturan hanya menentukan apa yang digambar.

## Yang ditampilkan JeffCrunch

JeffCrunch menampilkan aturan yang dimiliki buku kerja:

- **Aturan sorot**: aturan yang mengubah font, isian, bingkai, atau format angka pada sel yang memenuhinya. Kondisinya dapat berupa perbandingan dengan suatu nilai (lebih besar dari, antara, sama dengan, dan seterusnya), rumus, teks yang berisi, diawali dengan, atau diakhiri dengan kata tertentu, tanggal seperti "minggu lalu", sel yang kosong atau berisi galat, nilai yang termasuk tertinggi atau terendah (angka atau persen), nilai di atas atau di bawah rata-rata, serta nilai yang duplikat atau unik.
- **Skala warna**: setiap sel diisi dengan warna yang berpindah di antara dua atau tiga warna seiring angkanya naik dari rendah ke tinggi. Skala dari putih ke hijau membuat angka terbesar paling hijau.
- **Batang data**: batang di dalam sel yang panjangnya mengikuti besarnya angka. Batang dapat berwarna pejal atau memudar, dapat memiliki bingkai, dapat memiliki sumbu, dan dapat berjalan ke arah sebaliknya, dengan warna lain, untuk angka negatif.
- **Set ikon**: gambar kecil di dalam sel yang menunjukkan bagian rentang tempat angka itu berada, misalnya panah naik, panah mendatar, atau panah turun; atau lampu merah, kuning, atau hijau. Tersedia dua puluh set.

Jika dua aturan sama-sama mengatur sel yang sama, aturan yang lebih dulu muncul dalam urutan buku kerja yang menang untuk hal yang keduanya atur, dan aturan yang ditandai "berhenti jika benar" menghentikan pencarian. Angka pada skala warna, batang data, atau set ikon dihitung dari sel-sel aturan itu sendiri, dan dihitung ulang ketika salah satu sel tersebut berubah.

## Yang tetap sama

- Aturan mengikuti sel yang menjadi miliknya. Sisipkan atau hapus baris atau kolom, potong dan tempel, dan aturan tetap mencakup sel yang benar; **Urungkan** mengembalikannya.
- Aturan yang tidak digambar oleh JeffCrunch tetap disimpan dalam berkas seperti semula.
- Menyimpan akan menulis aturan yang dimiliki buku kerja. Ketika Anda membuat atau menghapus aturan pada sebuah lembar, aturan lembar tersebut ditulis ulang dengan perubahan Anda.

## Membuat aturan

1. Pilih sel-selnya.
2. Klik **Beranda > Pemformatan Bersyarat**.
3. Di bawah **Highlight Cells Rules** (Aturan Sorot Sel), pilih **Greater Than** (Lebih Besar Dari), **Less Than** (Lebih Kecil Dari), **Between** (Antara), **Equal To** (Sama Dengan), **Text That Contains** (Teks yang Berisi), atau **Duplicate Values** (Nilai Duplikat). Di bawah **Top/Bottom Rules** (Aturan Atas/Bawah), pilih **Top 10 Items** (10 Item Teratas), **Bottom 10 Items** (10 Item Terbawah), **Top 10 %**, **Bottom 10 %**, **Above Average** (Di Atas Rata-Rata), atau **Below Average** (Di Bawah Rata-Rata).
4. Ketikkan nilainya (atau dua nilai untuk **Between**, teksnya untuk **Text That Contains**, atau jumlahnya untuk aturan Atas dan Bawah). Untuk **Top 10 Items**, "10" adalah jumlah sel yang mendapat tampilan tersebut. Anda dapat mengubahnya menjadi angka berapa pun.
5. Pilih tampilan sel, misalnya **Light Red Fill with Dark Red Text** (Isian Merah Muda dengan Teks Merah Tua), lalu klik **OK**.

Aturan langsung digambar. Setiap aturan baru ditempatkan paling depan, sehingga jika dua aturan ingin mengubah hal yang sama, aturan yang lebih baru yang menang. Membuat aturan merupakan satu langkah yang dapat Anda urungkan.

## Menghapus aturan

**Clear Rules from Selected Cells** (Hapus Aturan dari Sel yang Dipilih) menghapus setiap aturan yang ada pada sel mana pun yang dipilih. **Clear Rules from This Sheet** (Hapus Aturan dari Lembar Ini) menghapus semua aturan pada lembar. Masing-masing merupakan satu langkah yang dapat Anda urungkan. Pilihan tidak melakukan apa-apa jika tidak ada aturan untuk dihapus.

## Yang belum tersedia

- Batang data, skala warna, dan set ikon dapat ditampilkan tetapi belum dapat dibuat. Jendela yang mendaftar, mengurutkan, dan mengedit semua aturan (**Manage Rules**, Kelola Aturan) belum tersedia. Aturan yang memakai rumus belum dapat dibuat dari menu.
- Lembar yang memiliki batang data dalam format berkas yang lebih baru tetap menyimpannya seperti semula, dan JeffCrunch tidak membuat atau menghapus aturan pada lembar tersebut. Lakukan perubahan itu di program yang membuat buku kerjanya.
- Buku kerja yang dibuka dari jenis berkas lama tetap menyimpan aturannya saat Anda menyimpannya sebagai buku kerja .xlsx, dan Anda dapat menambahkan aturan setelahnya.
