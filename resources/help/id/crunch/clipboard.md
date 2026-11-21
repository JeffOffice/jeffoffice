# Gunting, salin, dan tempel

<!-- keywords: gunting, salin, tempel, papan klip, pindahkan, duplikat, tempel khusus, nilai, rumus, format, transpos, lewati kosong, tambah, kurangi, kalikan, bagi, operasi, digabungkan, teks, tab, html, halaman web, program lain, Esc, garis putus-putus berjalan, bingkai, lebar kolom, catatan, komentar, format angka, semua kecuali bingkai, gambar, bitmap, salin sebagai gambar, gambar hasil cetak, sebagaimana dicetak, tempel tautan, hiperlink, impor teks, teks dibatasi, seret teks, koma, titik koma, csv, pemisah, atur tempel default, tempel default, lebar tetap -->

**Salin** (**{key:Copy}**) menempatkan salinan sel yang dipilih di papan klip, tempat yang disediakan komputer untuk menyimpan satu hal dalam satu waktu. **Tempel** (**{key:Paste}**) memasukkan isi papan klip ke dalam sel, mulai dari sel yang dipilih. **Gunting** (**{key:Cut}**) serupa dengan salin, tetapi saat Anda menempel, sel-sel tersebut dipindahkan dan tempat asalnya dikosongkan.

Bingkai putus-putus yang bergerak menunjukkan sel yang disalin atau digunting. Tekan **Esc** untuk menghilangkan bingkai tersebut; sel-sel kemudian tidak lagi siap untuk ditempel. Sel yang digunting juga dilepaskan jika Anda membuat perubahan lain sebelum menempelnya (mengetik, menyisipkan, atau mengurungkan): gunting ulang jika Anda masih ingin memindahkannya.

## Tempel di dalam JeffCrunch

Semua ikut terbawa: nilai, rumus, format, dan sel gabungan. Rumus tetap menunjuk ke tempat yang sama *relatif terhadap posisinya*. Contoh: salin **=A1*2** dari B1 lalu tempel ke B5, maka hasilnya menjadi **=A5*2**. Bagian yang memakai tanda dolar, seperti **$A$1**, tetap seperti semula. Jika Anda memilih blok yang merupakan kelipatan utuh dari ukuran yang disalin, salinan akan diulang untuk mengisi blok tersebut: salin satu sel, pilih dua puluh sel, tempel, maka kedua puluh sel akan menerimanya.

Angka yang ditempel dan terlalu lebar untuk kolom yang masih memakai lebar pertamanya akan membuat kolom itu melebar.

## Gunting memindahkan sel

Saat Anda menggunting sel lalu menempelkannya, sel tersebut benar-benar berpindah. Setiap rumus yang menunjuk ke sel tersebut akan menunjuk ke lokasi barunya. Contoh: B1 berisi **=A1+A2**. Gunting A1:A2 lalu tempel di E4, maka B1 menjadi **=E4+E5**. Isi sel yang tertimpa oleh tempelan akan hilang (**Urungkan** mengembalikannya), dan rumus yang membaca salah satu sel tersebut akan menampilkan **#REF!**, karena sel yang dibacanya sudah tidak ada. Jika sel yang digunting hanya sebagian dari sel gabungan, atau jika Anda menempel menimpa sebagian dari sel gabungan, tidak ada yang dipindahkan: pilih seluruh sel gabungan tersebut. Saat Anda menggunting sel ke lembar lain, rumus tetap membaca sel yang dibacanya sebelumnya (dengan menyebutkan nama lembarnya). Papan klip dikosongkan setelah gunting dan tempel.

## Tempel Khusus

**Ctrl+Alt+V**, atau panah di bawah **Tempel** pada tab Beranda, memungkinkan Anda memilih apa yang akan ditempel:

- **Semua**, **Rumus**, **Nilai**, atau **Format**. *Nilai* mengubah rumus menjadi angka atau teks yang ditampilkannya.
- **Semua Kecuali Bingkai**: semuanya, tetapi sel tetap mempertahankan bingkai yang dimilikinya.
- **Rumus dan Format Angka**, atau **Nilai dan Format Angka**: rumus (atau nilai) disertai cara angka ditampilkan, seperti persen atau tanggal, tetapi tanpa warna, font, atau bingkai. Sel tetap mempertahankan tampilannya.
- **Lebar Kolom**: kolom tempat Anda menempel mendapatkan lebar kolom yang disalin.
- **Komentar dan Catatan**: catatan dari sel yang disalin ditempelkan ke sel tempat Anda menempel. Tidak ada nilai sel yang berubah.
- Sebuah **operasi**: **Tambah**, **Kurangi**, **Kalikan**, atau **Bagi**. Angka yang disalin digabungkan dengan angka yang sudah ada di dalam sel. Contoh: sel berisi 10 dan 20, Anda menyalin 5, lalu memilih *Kalikan*: hasilnya menjadi 50 dan 100.
- **Lewati kosong**: sel kosong dalam salinan membiarkan sel tujuan tetap seperti semula.
- **Transpos**: baris menjadi kolom dan kolom menjadi baris.

## Tombol Opsi Tempel

Segera setelah Anda menempel sel yang disalin di JeffCrunch, tombol kecil **Opsi Tempel** muncul di dekat sel terakhir yang Anda tempel. Klik tombol itu untuk memilih cara lain memasukkan sel yang sama: **Pertahankan Format Sumber** (semuanya), **Nilai**, **Rumus**, **Pemformatan**, **Tanpa Bingkai**, **Rumus dan Pemformatan Angka**, **Nilai dan Pemformatan Angka**, **Pertahankan Lebar Kolom Sumber**, atau **Transpos**. JeffCrunch membatalkan tempelan tersebut dan melakukannya ulang dengan cara itu. Perubahan ini dihitung sebagai satu langkah: satu kali **Urungkan** membatalkan seluruh tempelan, berapa pun kali Anda mengubah pilihan. Contoh: salin A1:B2, dengan B1 berisi **=A1*2**, lalu tempel di F5. Pilih **Nilai** dan G5 berisi angka 2 alih-alih **=F5*2**; pilih **Pertahankan Format Sumber** dan rumus kembali.

Dari papan ketik, tekan tombol **Ctrl** (tombol **Control** pada Mac) setelah menempel untuk membuka menu tombol tersebut, lalu gunakan tombol panah dan **Enter** untuk memilih. Tombol ini hilang saat Anda mengubah sel atau hal lain di buku kerja, menyalin sesuatu yang lain, menekan **Esc**, atau menggulir sel terakhir yang ditempel keluar dari pandangan (tombol akan muncul lagi saat Anda menggulir kembali). Tombol ini tidak ditawarkan setelah gunting dan tempel, atau untuk teks dari program lain.

## Tempel Tautan, Tempel sebagai Hiperlink, dan Tempel Impor Teks

Tiga pilihan lain ada di panah di bawah **Tempel**. Pilihan ini bekerja pada sel yang Anda salin di buku kerja yang sama, kecuali yang terakhir.

- **Tempel Tautan** menempatkan rumus di setiap sel yang menunjuk ke sel yang disalin. Contoh: salin A1:B2, pilih D4, lalu gunakan **Tempel Tautan**: D4 menjadi **=Sheet1!$A$1**. Ketika A1 berubah, D4 ikut berubah. Sel yang disalin dan kosong akan menampilkan 0.
- **Tempel sebagai Hiperlink** menempatkan kata-kata yang sama dengan yang ditampilkan sel yang disalin di setiap sel, dan menjadikan setiap sel sebagai tautan ke sel yang disalin. Tahan **Ctrl** lalu klik salah satunya untuk pergi ke sel tersebut. Fitur ini bekerja untuk hingga 2.000 sel sekaligus.
- **Tempel Impor Teks** digunakan untuk teks dari program lain. Fitur ini membuka jendela yang sama dengan saat membuka berkas teks: Anda menentukan apa yang memisahkan bagian-bagian teks (koma, tab, spasi, titik koma, batang, atau yang lain), apa yang membatasi sebuah bagian, baris pertama yang akan digunakan, dan cara membaca setiap kolom (apa adanya, sebagai teks, sebagai tanggal, atau diabaikan). Beberapa teks menyusun bagian-bagiannya dalam kolom dengan lebar yang sama, dengan spasi di antaranya. Centang **Kolom memiliki lebar tetap**, lalu tentukan di mana setiap kolom dimulai: setiap baris dipotong pada tempat-tempat tersebut, dan spasi di sekitar setiap bagian dihapus. Pilih *teks* untuk kolom kode seperti 007 agar angka nolnya tetap ada. Bagian-bagian tersebut dimasukkan ke dalam sel, mulai dari sel yang dipilih, sebagai satu langkah yang dapat diurungkan.

## Teks dengan koma, titik koma, atau kolom

Ketika teks yang Anda tempel dari program lain jelas merupakan tabel, JeffCrunch membuka jendela yang sama dengan saat membuka berkas teks, sehingga Anda dapat memeriksa cara pemisahannya sebelum apa pun masuk ke dalam sel. Ini hanya terjadi saat **Tempel** biasa (**{key:Paste}**), tidak pernah dengan **Tempel Khusus** atau salah satu pilihannya. Ini terjadi jika teks memiliki minimal dua baris dan tanpa tab, dan salah satu dari dua kondisi berikut terpenuhi: setiap baris memiliki jumlah bagian yang sama, minimal tiga, yang dipisahkan koma, titik koma, atau batang, dengan isi di setiap kolom (bagian dalam tanda kutip boleh berisi koma); atau bagian-bagiannya sejajar dalam kolom dengan lebar yang sama, dengan celah spasi yang sama di setiap baris (minimal tiga baris, dan minimal dua celah). Angka dengan koma pemisah ribuan, seperti 1,234, serta baris kode atau daftar yang berakhir dengan koma atau titik koma, tidak dianggap sebagai tabel. Jendela dimulai dengan cara teks tersebut terdeteksi terpisah. Jika angka memakai koma dan bagian-bagiannya dipisahkan titik koma, maka koma adalah tanda desimal. Klik **OK** untuk memasukkan bagian-bagian tersebut, atau **Batal** untuk tidak memasukkan apa pun.

Contoh: salin baris-baris berikut dari sebuah surel.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Jika Anda menempelkannya dengan kolom ketiga, misalnya harga, jendela akan menampilkan tiga kolom dengan **Koma** tercentang. Satu kalimat dalam satu baris, atau baris dengan jumlah koma yang berbeda, atau hanya dua bagian per baris, ditempel sebagai kata-kata tanpa jendela. Teks dengan tab langsung masuk ke dalam sel, seperti biasa. Untuk membuka jendela untuk teks apa pun, pilih **Tempel Impor Teks** (di atas).

Hal yang sama terjadi saat Anda menyeret teks dari program lain dan melepaskannya di kisi: teks masuk mulai dari sel yang dipilih. Pelepasan di tempat lain pada jendela tidak melakukan apa-apa.

## Atur Tempel Default

**Atur Tempel Default** adalah pilihan terakhir di panah di bawah **Tempel**. Pilihan ini menentukan apa yang dimasukkan tombol **Tempel** dan **{key:Paste}** saat Anda telah menyalin sel: semuanya (pada awalnya), atau hanya nilai, hanya rumus, hanya pemformatan, dan seterusnya. Contoh: pilih *Nilai*, dan rumus yang disalin akan masuk sebagai angka yang ditampilkannya. Pilihan ini disimpan untuk penggunaan berikutnya. Pilihan ini tidak mengubah **Tempel Khusus** atau pilihan di menu, dan teks dari program lain ditempel apa adanya. Pilih *Semua* untuk kembali.

Pilihan yang sama juga ada di halaman **Pengeditan** pada **Opsi**, dalam dua kotak: satu untuk sel yang Anda salin di buku kerja ini, dan satu untuk sel yang Anda salin di buku kerja lain. Sampai Anda memilih untuk buku kerja lain, pilihannya mengikuti yang pertama.

## Menu Tempel dan gambar

Panah di bawah **Tempel** memuat pilihan biasa, dan juga **Tanpa Bingkai**, **Rumus dan Pemformatan Angka**, **Nilai dan Pemformatan Angka**, **Pertahankan Lebar Kolom Sumber**, dan **Komentar dan Catatan**. Pilihan tersebut sama dengan pilihan di Tempel Khusus, masing-masing hanya dengan satu klik.

Panah di samping **Salin** memiliki **Salin sebagai Gambar** dan **Salin sebagai Bitmap**. Masing-masing menempatkan gambar dari sel yang dipilih di papan klip, digambar seperti yang Anda lihat, dengan garis kisi. Bitmap berukuran sebesar layar. Gambar memiliki titik dua kali lebih banyak, sehingga tetap tajam saat diperbesar. **Salin sebagai Gambar Cetak** menggambar sel seperti tampilannya di kertas: garis kisi hanya ada jika pengaturan halaman mencetaknya. Tempelkan ke JeffWrite atau JeffDeck sebagai gambar. Jika sel tidak muat dalam satu gambar, pilih lebih sedikit sel.

## Antarprogram

Saat Anda menyalin, JeffCrunch juga menempatkan sel di papan klip sebagai teks (sel dipisahkan tab, satu baris per baris) dan sebagai tabel untuk halaman web, sehingga program lain dapat menggunakannya. Saat Anda menempel sesuatu yang tidak berasal dari JeffCrunch, teks dengan tab, atau tabel dari halaman web atau lembar kerja lain, setiap bagian masuk ke sel tersendiri dan dibaca seolah-olah Anda mengetiknya: angka, tanggal, dan rumus yang diawali tanda sama dengan. Menyalin dari buku kerja lain yang terbuka di JeffCrunch membawa nilai, rumus, dan format angka, tetapi tidak format lainnya. Tempelkan di JeffDeck atau JeffPub, dan hasilnya menjadi tabel yang mempertahankan font, warna latar, bingkai, serta angka seperti yang ditampilkan (lihat **Tempel sel sebagai tabel** di Bantuan mereka).

## Yang tidak dapat ditempel

JeffCrunch memberi tahu Anda dan tidak mengubah apa pun jika tempelan akan menjangkau sebagian dari **rumus array**, menimpa baris nama kolom dari sebuah **tabel**, masuk ke sel terkunci pada **lembar yang dilindungi**, melewati ujung lembar, atau memasuki lebih dari 100.000 sel. Seluruh **rumus array** dapat digunting dan dipindahkan (pilih semua selnya); sebagian dari rumus tersebut, tabel data, atau baris nama tabel tidak dapat.
