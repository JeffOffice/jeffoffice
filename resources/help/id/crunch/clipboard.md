# Memotong, menyalin, dan menempel

<!-- keywords: potong, salin, tempel, papan klip, pindahkan, duplikat, khusus, nilai, rumus, format, transpos, lewati kosong, tambah, kurangi, kalikan, bagi, operasi, digabungkan, teks, tab, html, halaman web, program lain, Esc, semut berjalan, bingkai, lebar kolom, catatan, komentar, format angka, semua kecuali bingkai, gambar, bitmap, salin sebagai gambar, gambar cetak, seperti dicetak, tempel tautan, hiperlink, impor teks, pisahkan teks, pemisah, atur tempel default, tempel default, lebar tetap -->

**Salin** (**{key:Copy}**) menempatkan salinan sel yang dipilih ke papan klip, yaitu tempat penyimpanan sementara di komputer untuk satu hal dalam satu waktu. **Tempel** (**{key:Paste}**) memasukkan isi papan klip ke dalam sel, dimulai dari sel yang dipilih. **Potong** (**{key:Cut}**) serupa dengan salin, tetapi saat Anda menempel, sel dipindahkan dan tempat asalnya dikosongkan.

Bingkai putus-putus yang bergerak menunjukkan sel yang disalin atau dipotong. Tekan **Esc** untuk menghilangkan bingkai tersebut; sel kemudian tidak lagi siap untuk ditempel. Sel yang dipotong juga dilepas jika Anda membuat perubahan lain sebelum menempelnya (mengetik, menyisipkan, atau mengurungkan): potong kembali jika Anda masih ingin memindahkannya.

## Tempel di dalam JeffCrunch

Semuanya ikut tersalin: nilai, rumus, format, dan sel gabungan. Rumus tetap merujuk ke tempat yang sama *relatif terhadap posisinya*. Contoh: salin **=A1*2** dari B1 lalu tempel ke B5, maka rumus itu menjadi **=A5*2**. Bagian yang diberi tanda dolar, seperti **$A$1**, tetap seperti semula. Jika Anda memilih blok yang merupakan kelipatan utuh dari ukuran yang disalin, salinan akan diulang untuk mengisi blok tersebut: salin satu sel, pilih dua puluh sel, tempel, dan keduanya akan mendapatkan isi yang sama.

Angka yang ditempel dan terlalu lebar untuk kolom yang masih memiliki lebar pertamanya akan melebarkan kolom tersebut.

## Potong memindahkan sel

Saat Anda memotong sel lalu menempelkannya, sel benar-benar berpindah. Setiap rumus yang merujuk ke sel tersebut akan merujuk ke lokasi barunya. Contoh: B1 berisi **=A1+A2**. Potong A1:A2 lalu tempel di E4, maka B1 menjadi **=E4+E5**. Isi sel yang tertimpa oleh tempelan akan hilang (**Urungkan** mengembalikannya), dan rumus yang membaca salah satu sel tersebut akan menampilkan **#REF!**, karena sel yang dibacanya sudah tidak ada. Jika sel yang dipotong hanya sebagian dari sel gabungan, atau Anda menempel hanya di sebagian sel gabungan, tidak ada yang dipindahkan: pilih seluruh sel gabungan. Saat Anda memotong sel ke lembar lain, rumus tetap membaca sel yang dibacanya sebelumnya (dengan keterangan lembarnya). Papan klip dikosongkan setelah pemotongan dan penempelan.

## Tempel Khusus

**Ctrl+Alt+V**, atau panah di bawah **Tempel** pada tab Beranda, memungkinkan Anda memilih apa yang akan ditempel:

- **Semua**, **Rumus**, **Nilai**, atau **Format**. *Nilai* mengubah rumus menjadi angka atau teks yang ditampilkannya.
- **Semua Kecuali Bingkai**: semuanya, tetapi sel tetap mempertahankan bingkai yang dimilikinya.
- **Rumus dan Format Angka**, atau **Nilai dan Format Angka**: rumus (atau nilai) disertai cara angka ditampilkan, seperti persen atau tanggal, tetapi tanpa warna, font, atau bingkai. Sel tetap mempertahankan tampilannya semula.
- **Lebar Kolom**: kolom tempat Anda menempel mendapatkan lebar kolom yang Anda salin.
- **Komentar dan Catatan**: catatan dari sel yang disalin ditempelkan ke sel tujuan. Tidak ada nilai sel yang berubah.
- Sebuah **operasi**: **Tambah**, **Kurangi**, **Kalikan**, atau **Bagi**. Angka yang disalin digabungkan dengan angka yang sudah ada di dalam sel. Contoh: sel berisi 10 dan 20, Anda menyalin 5, dan memilih *Kalikan*: hasilnya menjadi 50 dan 100.
- **Lewati kosong**: sel kosong dalam salinan membiarkan sel tujuannya tetap seperti semula.
- **Transpos**: baris menjadi kolom dan kolom menjadi baris.

## Tempel Tautan, Tempel sebagai Hiperlink, dan Tempel Impor Teks

Tiga pilihan lain terdapat di panah di bawah **Tempel**. Ketiganya bekerja pada sel yang Anda salin dalam buku kerja yang sama, kecuali yang terakhir.

- **Tempel Tautan** memasukkan rumus ke setiap sel yang merujuk ke sel yang disalin. Contoh: salin A1:B2, pilih D4, lalu gunakan **Tempel Tautan**: D4 menjadi **=Sheet1!$A$1**. Saat A1 berubah, D4 ikut berubah. Sel yang disalin dan kosong akan menampilkan 0.
- **Tempel sebagai Hiperlink** memasukkan kata yang sama dengan yang ditampilkan sel yang disalin ke setiap sel, dan menjadikan masing-masing sebagai tautan ke sel yang disalin. Tahan **Ctrl** lalu klik salah satunya untuk berpindah ke sel tersebut. Fitur ini bekerja untuk hingga 2.000 sel sekaligus.
- **Tempel Impor Teks** digunakan untuk teks dari program lain. Fitur ini membuka jendela yang sama dengan saat membuka berkas teks: Anda menentukan apa yang memisahkan bagian-bagian (koma, tab, spasi, titik koma, garis tegak, atau yang lain), apa yang mengapit suatu bagian dengan tanda kutip, baris pertama yang digunakan, dan cara membaca setiap kolom (apa adanya, sebagai teks, sebagai tanggal, atau dilewati). Sebagian teks menyusun bagian-bagiannya dalam kolom dengan lebar yang sama, dengan spasi di antaranya. Centang **Kolom memiliki lebar tetap**, lalu tentukan di mana setiap kolom dimulai: setiap baris akan dipotong di tempat-tempat tersebut, dan spasi di sekitar setiap bagian dihapus. Pilih *teks* untuk kolom kode seperti 007, agar angka nol tetap ada. Bagian-bagian tersebut dimasukkan ke dalam sel, dimulai dari sel yang dipilih, sebagai satu langkah yang dapat diurungkan.

## Atur Tempel Default

**Atur Tempel Default** adalah pilihan terakhir di panah di bawah **Tempel**. Pilihan ini menentukan apa yang dimasukkan oleh tombol **Tempel** dan **{key:Paste}** saat Anda menyalin sel: semuanya (pada awalnya), atau hanya nilai, hanya rumus, hanya format, dan seterusnya. Contoh: pilih *Nilai*, dan rumus yang disalin akan masuk sebagai angka yang ditampilkannya. Pilihan ini tetap disimpan untuk penggunaan berikutnya. Pilihan ini tidak mengubah **Tempel Khusus** atau pilihan di menu, dan teks dari program lain ditempel apa adanya. Pilih *Semua* untuk kembali ke pengaturan semula.

## Menu Tempel dan gambar

Panah di bawah **Tempel** menampilkan pilihan yang biasa, dan juga **Tanpa Bingkai**, **Rumus dan Format Angka**, **Nilai dan Format Angka**, **Pertahankan Lebar Kolom Sumber**, dan **Komentar dan Catatan**. Pilihan tersebut sama dengan pilihan di Tempel Khusus, masing-masing hanya dengan satu klik.

Panah di samping **Salin** memiliki **Salin sebagai Gambar** dan **Salin sebagai Bitmap**. Masing-masing menempatkan gambar sel yang dipilih ke papan klip, digambar seperti yang Anda lihat, dengan garis kisi. Bitmap berukuran sesuai layar. Gambar memiliki titik dua kali lebih banyak, sehingga tetap tajam saat diperbesar. **Salin sebagai Gambar Cetak** menggambar sel seperti yang akan ditampilkan di kertas: garis kisi hanya ada jika pengaturan halaman mencetaknya. Tempelkan ke JeffWrite atau JeffDeck sebagai gambar. Jika sel tidak muat dalam satu gambar, pilih lebih sedikit sel.

## Antarprogram

Saat Anda menyalin, JeffCrunch juga menempatkan sel ke papan klip sebagai teks (sel dipisahkan dengan tab, satu baris per baris) dan sebagai tabel untuk halaman web, sehingga program lain dapat mengambilnya. Saat Anda menempel sesuatu yang tidak berasal dari JeffCrunch, misalnya teks dengan tab, atau tabel dari halaman web atau lembar kerja lain, setiap bagian akan masuk ke sel masing-masing dan dibaca seperti saat Anda mengetiknya: angka, tanggal, dan rumus yang diawali tanda sama dengan. Menyalin dari buku kerja lain yang dibuka di JeffCrunch membawa nilai, rumus, dan format angka, tetapi tidak format lainnya.

## Yang tidak dapat ditempel

JeffCrunch akan memberi tahu Anda dan tidak mengubah apa pun jika penempelan akan menjangkau sebagian dari **rumus array**, di atas baris nama kolom sebuah **tabel**, ke dalam sel terkunci pada **lembar yang dilindungi**, melewati akhir lembar, atau ke lebih dari 100.000 sel. Seluruh **rumus array** dapat dipotong dan dipindahkan (pilih semua selnya); sebagian darinya, tabel data, atau baris nama tabel tidak dapat.
