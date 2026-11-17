# Menggunting, menyalin, dan menempel

<!-- keywords: gunting, salin, tempel, papan klip, pindahkan, duplikat, tempel khusus, nilai, rumus, format, transpos, lewati kosong, tambah, kurangi, kalikan, bagi, operasi, digabungkan, teks, tab, html, halaman web, program lain, Esc, semut berjalan, bingkai, lebar kolom, catatan, komentar, format angka, semua kecuali bingkai, gambar, bitmap, salin sebagai gambar, gambar cetak, sebagaimana tercetak, tempel tautan, hiperlink, impor teks, teks dibatasi, jatuhkan teks, koma, titik koma, csv, pemisah, atur tempel default, tempel default, lebar tetap -->

**Salin** (**{key:Copy}**) menaruh salinan sel yang dipilih ke papan klip, yaitu tempat penyimpanan sementara yang menyimpan satu hal dalam satu waktu. **Tempel** (**{key:Paste}**) memasukkan isi papan klip ke dalam sel, dimulai dari sel yang dipilih. **Gunting** (**{key:Cut}**) serupa dengan salin, tetapi saat Anda menempel, sel-sel tersebut dipindahkan dan tempat lamanya dikosongkan.

Bingkai putus-putus yang bergerak menunjukkan sel yang disalin atau digunting. Tekan **Esc** untuk menghilangkan bingkai tersebut; sel-sel tidak lagi siap ditempel. Sel yang digunting juga dilepas jika Anda membuat perubahan lain sebelum menempelkannya (mengetik, menyisipkan, atau mengurungkan): gunting ulang jika Anda masih ingin memindahkannya.

## Tempel di dalam JeffCrunch

Semuanya ikut tersalin: nilai, rumus, format, dan sel gabungan. Rumus tetap menunjuk ke tempat yang sama *relatif terhadap posisinya*. Contoh: salin **=A1*2** dari B1 lalu tempel ke B5, maka rumus itu menjadi **=A5*2**. Bagian yang diberi tanda dolar, seperti **$A$1**, tetap seperti semula. Jika Anda memilih blok yang merupakan kelipatan utuh dari ukuran yang Anda salin, salinan akan diulang untuk mengisi blok tersebut: salin satu sel, pilih dua puluh sel, tempel, dan kedua puluh sel akan menerimanya.

Angka yang ditempel dan terlalu lebar untuk kolom yang masih memakai lebar pertamanya akan memperlebar kolom tersebut.

## Gunting memindahkan sel

Saat Anda menggunting sel lalu menempelkannya, sel-sel tersebut benar-benar berpindah. Setiap rumus yang menunjuk ke sel tersebut akan menunjuk ke posisi barunya. Contoh: B1 berisi **=A1+A2**. Gunting A1:A2 lalu tempel di E4, maka B1 menjadi **=E4+E5**. Isi sel yang tertimpa oleh tempelan akan hilang (**Urungkan** mengembalikannya), dan rumus yang membaca salah satu sel tersebut akan menampilkan **#REF!**, karena sel yang dibacanya sudah tidak ada. Jika sel yang digunting hanya sebagian dari sel gabungan, atau Anda menempel hanya di sebagian sel gabungan, tidak ada yang dipindahkan: pilih seluruh sel gabungan. Saat Anda menggunting sel ke lembar lain, rumus tetap membaca sel yang dibacanya sebelumnya (dengan menyebutkan lembarnya). Papan klip dikosongkan setelah gunting dan tempel.

## Tempel Khusus

**Ctrl+Alt+V**, atau panah di bawah **Tempel** pada tab Beranda, memungkinkan Anda memilih apa yang akan ditempel:

- **Semua**, **Rumus**, **Nilai**, atau **Format**. *Nilai* mengubah rumus menjadi angka atau teks yang ditampilkannya.
- **Semua Kecuali Bingkai**: semuanya, tetapi sel tetap memakai bingkai yang dimilikinya.
- **Rumus dan Format Angka**, atau **Nilai dan Format Angka**: rumus (atau nilai) disertai cara angka ditampilkan, seperti persen atau tanggal, tetapi tanpa warna, font, atau bingkai. Sel tetap mempertahankan tampilan yang dimilikinya.
- **Lebar Kolom**: kolom tujuan tempel mendapat lebar kolom yang Anda salin.
- **Komentar dan Catatan**: catatan dari sel yang disalin dipasang pada sel tujuan tempel. Tidak ada nilai sel yang berubah.
- Sebuah **operasi**: **Tambah**, **Kurangi**, **Kalikan**, atau **Bagi**. Angka yang Anda salin digabungkan dengan angka yang sudah ada di sel. Contoh: sel berisi 10 dan 20, Anda menyalin 5, dan memilih *Kalikan*: hasilnya menjadi 50 dan 100.
- **Lewati Kosong**: sel kosong pada salinan membiarkan sel tujuan tetap seperti semula.
- **Transpos**: baris menjadi kolom dan kolom menjadi baris.

## Tempel Tautan, Tempel sebagai Hiperlink, dan Tempel Impor Teks

Tiga pilihan lain ada di panah di bawah **Tempel**. Ketiganya bekerja pada sel yang Anda salin dalam buku kerja yang sama, kecuali yang terakhir.

- **Tempel Tautan** menaruh rumus di setiap sel yang menunjuk ke sel yang Anda salin. Contoh: salin A1:B2, pilih D4, lalu gunakan **Tempel Tautan**: D4 menjadi **=Sheet1!$A$1**. Saat A1 berubah, D4 ikut berubah. Sel yang disalin dan kosong akan menampilkan 0.
- **Tempel sebagai Hiperlink** menaruh teks yang sama dengan yang ditampilkan sel yang disalin di setiap sel, dan menjadikan masing-masing sebagai tautan ke sel yang disalin. Tahan **Ctrl** lalu klik salah satunya untuk pergi ke sel tersebut. Fitur ini bekerja untuk hingga 2.000 sel sekaligus.
- **Tempel Impor Teks** digunakan untuk teks dari program lain. Fitur ini membuka jendela yang sama dengan jendela saat membuka berkas teks: Anda menentukan apa yang memisahkan bagian-bagian (koma, tab, spasi, titik koma, garis tegak, atau yang lain), apa yang mengapit suatu bagian, baris pertama yang akan digunakan, dan cara membaca setiap kolom (apa adanya, sebagai teks, sebagai tanggal, atau dilewati). Beberapa teks menyusun bagian-bagiannya dalam kolom dengan lebar yang sama, dengan spasi di antaranya. Centang **Kolom memiliki lebar tetap**, lalu tentukan di mana setiap kolom dimulai: setiap baris dipotong pada tempat tersebut, dan spasi di sekitar setiap bagian dihilangkan. Pilih *teks* untuk kolom kode seperti 007, agar angka nol di depannya tetap ada. Bagian-bagian tersebut dimasukkan ke dalam sel, dimulai dari sel yang dipilih, sebagai satu langkah yang dapat Anda urungkan.

## Teks dengan koma, titik koma, atau kolom

Jika teks yang Anda tempel dari program lain jelas merupakan tabel, JeffCrunch akan membuka jendela yang sama dengan jendela saat membuka berkas teks, sehingga Anda dapat memeriksa cara pemisahannya sebelum sesuatu masuk ke sel. Ini hanya terjadi saat memakai **Tempel** biasa (**{key:Paste}**), tidak pernah dengan **Tempel Khusus** atau salah satu pilihannya. Hal ini terjadi bila teks memiliki paling sedikit dua baris dan tidak ada tab, serta salah satu dari berikut berlaku: setiap baris memiliki jumlah bagian yang sama, paling sedikit tiga, yang dipisahkan koma, titik koma, atau garis tegak, dengan isi di setiap kolom (bagian dalam tanda kutip boleh berisi koma); atau bagian-bagiannya tersusun dalam kolom dengan lebar yang sama, dengan celah spasi yang sama di setiap baris (paling sedikit tiga baris, dan paling sedikit dua celah). Angka dengan pemisah ribuan, seperti 1,234, serta baris kode atau daftar yang diakhiri koma atau titik koma tidak dianggap sebagai tabel. Jendela dimulai dengan cara pemisahan yang terdeteksi pada teks. Jika angka memakai koma dan bagian-bagiannya dipisahkan titik koma, maka koma adalah tanda desimal. Klik **OK** untuk memasukkan bagian-bagian tersebut, atau **Batal** untuk tidak memasukkan apa pun.

Contoh: salin baris-baris berikut dari sebuah email.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Jika Anda menempelkannya dengan kolom ketiga, seperti harga, jendela akan menampilkan tiga kolom dengan **Koma** tercentang. Satu kalimat dalam satu baris, atau baris yang jumlah komanya berbeda, atau baris yang hanya memiliki dua bagian, ditempel sebagai kata tanpa jendela. Teks yang berisi tab langsung masuk ke sel, seperti biasa. Untuk membuka jendela bagi teks apa pun, pilih **Tempel Impor Teks** (di atas).

Hal yang sama terjadi saat Anda menyeret teks dari program lain dan melepaskannya di kisi: teks masuk mulai dari sel yang dipilih. Pelepasan di tempat lain pada jendela tidak melakukan apa pun.

## Atur Tempel Default

**Atur Tempel Default** adalah pilihan terakhir pada panah di bawah **Tempel**. Pilihan ini menentukan apa yang dimasukkan oleh tombol **Tempel** dan **{key:Paste}** saat Anda menyalin sel: semuanya (pada awalnya), atau hanya nilai, hanya rumus, hanya pemformatan, dan seterusnya. Contoh: pilih *Nilai*, maka rumus yang disalin akan masuk sebagai angka yang ditampilkannya. Pilihan ini disimpan untuk penggunaan berikutnya. Pilihan ini tidak mengubah **Tempel Khusus** atau pilihan pada menu, dan teks dari program lain ditempel apa adanya. Pilih *Semua* untuk kembali ke semula.

## Menu Tempel dan gambar

Panah di bawah **Tempel** mencantumkan pilihan yang biasa, dan juga **Tanpa Bingkai**, **Rumus dan Pemformatan Angka**, **Nilai dan Pemformatan Angka**, **Pertahankan Lebar Kolom Sumber**, serta **Komentar dan Catatan**. Pilihan-pilihan ini sama dengan yang ada di Tempel Khusus, masing-masing cukup dengan satu klik.

Panah di sebelah **Salin** memiliki **Salin sebagai Gambar** dan **Salin sebagai Bitmap**. Masing-masing menaruh gambar sel yang dipilih ke papan klip, digambar seperti yang Anda lihat, dengan garis kisi. Bitmap berukuran sebesar layar. Gambar memiliki dua kali lipat titik, sehingga tetap tajam saat diperbesar. **Salin sebagai Gambar Cetak** menggambar sel sebagaimana akan tampak di kertas: garis kisi hanya ada jika pengaturan halaman mencetaknya. Tempelkan gambar tersebut ke JeffWrite atau JeffDeck sebagai gambar. Jika sel tidak muat dalam satu gambar, pilih lebih sedikit sel.

## Antarprogram

Saat Anda menyalin, JeffCrunch juga menaruh sel di papan klip sebagai teks (sel dipisahkan tab, satu baris per baris) dan sebagai tabel untuk halaman web, sehingga program lain dapat menggunakannya. Saat Anda menempel sesuatu yang tidak berasal dari JeffCrunch, seperti teks berisi tab, atau tabel dari halaman web atau lembar kerja lain, setiap bagian masuk ke sel sendiri dan dibaca seolah-olah Anda mengetiknya: angka, tanggal, dan rumus yang diawali tanda sama dengan. Menyalin dari buku kerja lain yang terbuka di JeffCrunch membawa nilai, rumus, dan format angka, tetapi tidak format lainnya.

## Yang tidak dapat ditempel

JeffCrunch akan memberi tahu Anda dan tidak mengubah apa pun jika tempelan akan menjangkau sebagian dari **rumus array**, di atas baris nama kolom sebuah **tabel**, ke dalam sel terkunci pada **lembar yang dilindungi**, melewati akhir lembar, atau ke lebih dari 100.000 sel. Seluruh **rumus array** dapat digunting dan dipindahkan (pilih semua selnya); sebagian darinya, tabel data, atau baris nama tabel tidak dapat.
