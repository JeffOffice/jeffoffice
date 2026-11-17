# Gunting, salin, dan tempel

<!-- keywords: gunting, salin, tempel, papan klip, pindahkan, duplikasi, tempel khusus, nilai, rumus, format, transpos, lewati kosong, tambah, kurangi, kalikan, bagi, operasi, digabungkan, teks, tab, html, halaman web, program lain, Esc, semut berjalan, bingkai, lebar kolom, catatan, komentar, format angka, semua kecuali bingkai, gambar, bitmap, salin sebagai gambar, gambar cetak, seperti dicetak, tempel tautan, hiperlink, impor teks, teks dibatasi, jatuhkan teks, koma, titik koma, csv, pemisah, atur tempel default, tempel default, lebar tetap -->

**Salin** (**{key:Copy}**) menaruh salinan sel yang dipilih di papan klip, tempat yang digunakan komputer untuk menyimpan satu hal dalam satu waktu. **Tempel** (**{key:Paste}**) menaruh isi papan klip ke dalam sel, mulai dari sel yang dipilih. **Gunting** (**{key:Cut}**) mirip dengan salin, tetapi saat Anda menempel, sel dipindahkan dan tempat asalnya dikosongkan.

Bingkai putus-putus yang bergerak menunjukkan sel yang disalin atau digunting. Tekan **Esc** untuk menghilangkan bingkai itu; sel tersebut kemudian tidak lagi siap ditempel. Sel yang digunting juga dilepas jika Anda membuat perubahan lain sebelum menempelkannya (mengetik, menyisipkan, atau mengurungkan): guntinglah lagi jika Anda tetap ingin memindahkannya.

## Tempel di dalam JeffCrunch

Semuanya ikut serta: nilai, rumus, format, dan sel gabungan. Rumus tetap menunjuk ke tempat yang sama *relatif terhadap posisinya*. Contoh: salin **=A1*2** dari B1 lalu tempel ke B5, maka rumus itu menjadi **=A5*2**. Bagian yang diberi tanda dolar, seperti **$A$1**, tetap seperti semula. Jika Anda memilih blok yang merupakan kelipatan bulat dari ukuran yang disalin, salinan akan diulang untuk mengisi blok tersebut: salin satu sel, pilih dua puluh sel, tempel, maka kedua puluh sel akan menerimanya.

Angka yang ditempel dan terlalu lebar untuk kolom yang masih memiliki lebar pertamanya akan melebarkan kolom tersebut.

## Gunting memindahkan sel

Saat Anda menggunting sel lalu menempelkannya, sel benar-benar berpindah. Setiap rumus yang menunjuk ke sel tersebut akan menunjuk ke posisi barunya. Contoh: B1 berisi **=A1+A2**. Gunting A1:A2 lalu tempel di E4, maka B1 menjadi **=E4+E5**. Apa pun yang ada di sel tempat Anda menempel akan hilang (**Urungkan** dapat mengembalikannya), dan rumus yang membaca salah satu sel tersebut akan menampilkan **#REF!**, karena sel yang dibacanya sudah tidak ada. Jika sel yang digunting hanya sebagian dari sel gabungan, atau Anda menempel hanya ke sebagian sel gabungan, tidak ada yang dipindahkan: pilih seluruh sel gabungan. Saat Anda menggunting sel ke lembar lain, rumus tetap membaca sel yang dibacanya sebelumnya (dengan menyebutkan lembarnya). Papan klip dikosongkan setelah gunting dan tempel.

## Tempel Khusus

**Ctrl+Alt+V**, atau panah di bawah **Tempel** pada tab Beranda, memungkinkan Anda memilih apa yang akan ditempel:

- **Semua**, **Rumus**, **Nilai**, atau **Format**. *Nilai* mengubah rumus menjadi angka atau teks yang ditampilkannya.
- **Semua Kecuali Bingkai**: semuanya, tetapi sel tetap mempertahankan bingkai yang dimilikinya.
- **Rumus dan Format Angka**, atau **Nilai dan Format Angka**: rumus (atau nilai) disertai cara angka ditampilkan, seperti persen atau tanggal, tetapi tanpa warna, font, atau bingkai. Sel tetap mempertahankan tampilannya semula.
- **Lebar Kolom**: kolom tempat Anda menempel mendapat lebar kolom yang disalin.
- **Komentar dan Catatan**: catatan dari sel yang disalin ditempatkan pada sel tempat Anda menempel. Tidak ada nilai sel yang berubah.
- Sebuah **operasi**: **Tambah**, **Kurangi**, **Kalikan**, atau **Bagi**. Angka yang disalin digabungkan dengan angka yang sudah ada di sel. Contoh: sel berisi 10 dan 20, Anda menyalin 5, dan memilih *Kalikan*: hasilnya menjadi 50 dan 100.
- **Lewati kosong**: sel kosong dalam salinan membiarkan sel tujuan tetap seperti semula.
- **Transpos**: baris menjadi kolom dan kolom menjadi baris.

## Tempel Tautan, Tempel sebagai Hiperlink, dan Tempel Impor Teks

Tiga pilihan lain ada di panah di bawah **Tempel**. Pilihan tersebut berfungsi pada sel yang Anda salin dalam buku kerja yang sama, kecuali yang terakhir.

- **Tempel Tautan** menaruh rumus di setiap sel yang menunjuk ke sel yang Anda salin. Contoh: salin A1:B2, pilih D4, lalu gunakan **Tempel Tautan**: D4 menjadi **=Sheet1!$A$1**. Saat A1 berubah, D4 ikut berubah. Sel yang disalin dan kosong akan menampilkan 0.
- **Tempel sebagai Hiperlink** menaruh kata-kata yang sama dengan yang ditampilkan sel yang disalin di setiap sel, dan menjadikan masing-masing sebagai tautan ke sel yang disalin. Tahan **Ctrl** lalu klik salah satunya untuk pergi ke sel tersebut. Fitur ini berfungsi untuk hingga 2.000 sel sekaligus.
- **Tempel Impor Teks** digunakan untuk teks dari program lain. Fitur ini membuka jendela yang sama dengan saat membuka berkas teks: Anda menentukan apa yang memisahkan bagian-bagian (koma, tab, spasi, titik koma, garis tegak, atau yang lain), apa yang membatasi suatu bagian, baris pertama yang digunakan, dan cara membaca setiap kolom (sebagaimana diketik, sebagai teks, sebagai tanggal, atau dilewati). Beberapa teks menyejajarkan bagian-bagiannya dalam kolom dengan lebar yang sama, dengan spasi di antaranya. Centang **Kolom memiliki lebar tetap**, lalu tentukan di mana setiap kolom dimulai: setiap baris dipotong pada tempat-tempat tersebut, dan spasi di sekitar setiap bagian dihapus. Pilih *teks* untuk kolom kode seperti 007, agar angka nol tetap ada. Bagian-bagian tersebut masuk ke dalam sel, mulai dari sel yang dipilih, sebagai satu langkah yang dapat diurungkan.

## Teks dengan koma, titik koma, atau kolom

Jika teks yang Anda tempel dari program lain jelas merupakan tabel, JeffCrunch membuka jendela yang sama dengan saat membuka berkas teks, sehingga Anda dapat memeriksa cara pemisahannya sebelum apa pun masuk ke dalam sel. Ini hanya terjadi pada **Tempel** biasa (**{key:Paste}**), tidak pernah pada **Tempel Khusus** atau salah satu pilihannya. Ini terjadi jika teks memiliki setidaknya dua baris dan tidak ada tab, serta salah satu dari kondisi berikut terpenuhi: setiap baris memiliki jumlah bagian yang sama, minimal tiga, yang dipisahkan koma, titik koma, atau garis tegak, dengan isi di setiap kolom (bagian dalam tanda kutip boleh berisi koma); atau bagian-bagiannya sejajar dalam kolom dengan lebar yang sama, dengan celah spasi yang sama di setiap baris (minimal tiga baris, dan minimal dua celah). Angka dengan pemisah ribuan berupa koma, seperti 1.234, serta baris kode atau daftar yang diakhiri koma atau titik koma tidak dianggap sebagai tabel. Jendela dimulai dengan cara teks tersebut terdeteksi terpisah. Jika angka menggunakan koma dan bagian-bagiannya dipisahkan titik koma, koma adalah tanda desimal. Klik **OK** untuk memasukkan bagian-bagian tersebut, atau **Batal** untuk tidak memasukkan apa pun.

Contoh: salin baris-baris berikut dari surel.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Tempelkan dengan kolom ketiga, misalnya harga, dan jendela akan menampilkan tiga kolom, dengan **Koma** dicentang. Satu kalimat dalam satu baris, atau baris dengan jumlah koma yang berbeda, atau baris hanya dengan dua bagian, akan ditempel sebagai kata tanpa jendela. Teks dengan tab langsung masuk ke dalam sel, seperti biasa. Untuk membuka jendela bagi teks apa pun, pilih **Tempel Impor Teks** (di atas).

Hal yang sama terjadi saat Anda menyeret teks dari program lain dan melepaskannya ke kisi: teks masuk di sel yang dipilih. Pelepasan di tempat lain pada jendela tidak melakukan apa-apa.

## Atur Tempel Default

**Atur Tempel Default** adalah pilihan terakhir di panah di bawah **Tempel**. Pilihan ini menentukan apa yang dimasukkan oleh tombol **Tempel** dan **{key:Paste}** ketika Anda menyalin sel: semuanya (pada awalnya), atau hanya nilai, hanya rumus, hanya pemformatan, dan seterusnya. Contoh: pilih *Nilai*, dan rumus yang disalin akan masuk sebagai angka yang ditampilkannya. Pilihan ini disimpan untuk penggunaan berikutnya. Pilihan ini tidak mengubah **Tempel Khusus** atau pilihan di menu, dan teks dari program lain ditempel apa adanya. Pilih *Semua* untuk kembali ke semula.

## Menu Tempel dan gambar

Panah di bawah **Tempel** menampilkan pilihan biasa, dan juga **Tanpa Bingkai**, **Rumus dan Pemformatan Angka**, **Nilai dan Pemformatan Angka**, **Pertahankan Lebar Kolom Sumber**, dan **Komentar dan Catatan**. Pilihan tersebut sama dengan yang ada di Tempel Khusus, cukup satu klik masing-masing.

Panah di sebelah **Salin** memiliki **Salin sebagai Gambar** dan **Salin sebagai Bitmap**. Masing-masing menaruh gambar sel yang dipilih di papan klip, digambar seperti yang Anda lihat, beserta garis kisinya. Bitmap berukuran sebesar layar. Gambar memiliki dua kali lipat jumlah titik, sehingga tetap tajam saat diperbesar. **Salin sebagai Gambar Cetak** menggambar sel seperti yang akan ditampilkan di kertas: garis kisi hanya ada jika pengaturan halaman mencetaknya. Tempelkan ke JeffWrite atau JeffDeck sebagai gambar. Jika sel tidak muat dalam satu gambar, pilih lebih sedikit sel.

## Antarprogram

Saat Anda menyalin, JeffCrunch juga menaruh sel di papan klip sebagai teks (sel dipisahkan tab, satu baris per baris) dan sebagai tabel untuk halaman web, sehingga program lain dapat menggunakannya. Saat Anda menempel sesuatu yang tidak berasal dari JeffCrunch, misalnya teks dengan tab, atau tabel dari halaman web atau lembar kerja lain, setiap bagian masuk ke sel sendiri dan dibaca seolah-olah Anda yang mengetiknya: angka, tanggal, dan rumus yang diawali tanda sama dengan. Menyalin dari buku kerja lain yang terbuka di JeffCrunch membawa nilai, rumus, dan format angka, tetapi tidak format lainnya. Tempelkan di JeffDeck atau JeffPub, dan hasilnya menjadi tabel yang mempertahankan font, warna latar, bingkai, serta angka seperti yang ditampilkan (lihat **Tempel sel sebagai tabel** di Bantuan program tersebut).

## Yang tidak dapat ditempel

JeffCrunch memberi tahu Anda dan tidak mengubah apa pun jika penempelan akan menjangkau sebagian dari **rumus array**, menimpa baris nama kolom sebuah **tabel**, masuk ke sel terkunci pada **lembar yang dilindungi**, melewati akhir lembar, atau masuk ke lebih dari 100.000 sel. Seluruh **rumus array** dapat digunting dan dipindahkan (pilih semua selnya); sebagian darinya, tabel data, atau baris nama kolom sebuah tabel tidak dapat.
