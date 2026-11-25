# Gunting, salin, dan tempel

<!-- keywords: gunting, salin, tempel, papan klip, pindahkan, duplikat, khusus, nilai, rumus, format, transpos, lewati kosong, tambah, kurangi, kalikan, bagi, operasi, digabung, teks, tab, html, halaman web, program lain, Esc, semut berjalan, bingkai, lebar kolom, catatan, komentar, format angka, semua kecuali bingkai, gambar, bitmap, salin sebagai gambar, gambar cetak, sesuai cetakan, tempel tautan, hiperlink, impor teks, teks dibatasi, seret teks, koma, titik koma, csv, pemisah, atur tempel default, tempel default, lebar tetap -->

**Salin** (**{key:Copy}**) menempatkan salinan sel yang dipilih ke papan klip, yaitu tempat penyimpanan sementara di komputer untuk satu hal dalam satu waktu. **Tempel** (**{key:Paste}**) memasukkan isi papan klip ke dalam sel, mulai dari sel yang dipilih. **Gunting** (**{key:Cut}**) mirip dengan salin, tetapi ketika Anda menempel, sel dipindahkan dan tempat lama dikosongkan.

Bingkai putus-putus yang bergerak menunjukkan sel yang telah disalin atau digunting. Tekan **Esc** untuk menghilangkan bingkai itu; sel kemudian tidak lagi siap ditempel. Sel yang digunting juga dilepas jika Anda membuat perubahan lain sebelum menempelkannya (mengetik, menyisipkan, atau mengurungkan): gunting lagi jika Anda masih ingin memindahkannya.

## Tempel di dalam JeffCrunch

Semuanya ikut tersalin: nilai, rumus, format, dan sel gabungan. Rumus tetap menunjuk ke tempat yang sama *relatif terhadap posisinya*. Contoh: salin **=A1*2** dari B1 lalu tempel ke B5, maka rumus itu menjadi **=A5*2**. Bagian yang diberi tanda dolar, seperti **$A$1**, tetap apa adanya. Jika Anda memilih blok yang ukurannya merupakan kelipatan bulat dari yang disalin, salinan akan diulang untuk mengisi blok itu: salin satu sel, pilih dua puluh sel, tempel, dan kedua puluh sel itu akan menerimanya.

Angka yang ditempel dan terlalu lebar untuk kolom yang masih memakai lebar pertamanya akan membuat kolom itu melebar.

## Gunting memindahkan sel

Ketika Anda menggunting sel lalu menempelkannya, sel benar-benar berpindah. Setiap rumus yang menunjuk ke sel tersebut akan menunjuk ke posisi barunya. Contoh: B1 berisi **=A1+A2**. Gunting A1:A2 lalu tempel di E4, maka B1 menjadi **=E4+E5**. Isi sel yang ditimpa saat menempel akan hilang (**Urungkan** mengembalikannya), dan rumus yang membaca salah satu sel tersebut akan menampilkan **#REF!**, karena sel yang dibacanya sudah tidak ada. Jika sel yang digunting hanya sebagian dari sel gabungan, atau Anda menempel hanya di sebagian sel gabungan, tidak ada yang dipindahkan: pilih seluruh sel gabungan. Ketika Anda menggunting sel ke lembar lain, rumus tetap membaca sel yang dibacanya sebelumnya (dengan keterangan lembarnya). Papan klip dikosongkan setelah gunting dan tempel.

## Tempel Khusus

**Ctrl+Alt+V**, atau panah di bawah **Tempel** pada tab Beranda, memungkinkan Anda memilih apa yang akan ditempel:

- **Semua**, **Rumus**, **Nilai**, atau **Format**. *Nilai* mengubah rumus menjadi angka atau teks yang ditampilkannya.
- **Semua Kecuali Bingkai**: semuanya, tetapi sel tetap memakai bingkai yang dimilikinya.
- **Rumus dan Format Angka**, atau **Nilai dan Format Angka**: rumus (atau nilai) disertai cara angkanya ditampilkan, seperti persen atau tanggal, tetapi tanpa warna, font, atau bingkai. Sel tetap mempertahankan tampilannya semula.
- **Lebar Kolom**: kolom tempat Anda menempel mendapat lebar kolom yang disalin.
- **Komentar dan Catatan**: catatan dari sel yang disalin dipasang pada sel tempat Anda menempel. Tidak ada nilai sel yang berubah.
- Sebuah **operasi**: **Tambah**, **Kurangi**, **Kalikan**, atau **Bagi**. Angka yang disalin digabungkan dengan angka yang sudah ada di sel. Contoh: sel berisi 10 dan 20, Anda menyalin 5, lalu memilih *Kalikan*: hasilnya menjadi 50 dan 100.
- **Lewati kosong**: sel kosong dalam salinan membiarkan sel tujuannya tetap seperti semula.
- **Transpos**: baris menjadi kolom dan kolom menjadi baris.

## Tombol Opsi Tempel

Segera setelah Anda menempel sel yang disalin di JeffCrunch, tombol kecil **Opsi Tempel** muncul di dekat sel terakhir yang ditempel. Klik tombol itu untuk memilih cara lain memasukkan sel yang sama: **Pertahankan Format Sumber** (semuanya), **Nilai**, **Rumus**, **Format**, **Tanpa Bingkai**, **Rumus dan Pemformatan Angka**, **Nilai dan Pemformatan Angka**, **Pertahankan Lebar Kolom Sumber**, atau **Transpos**. JeffCrunch membatalkan tempelan itu dan melakukannya lagi dengan cara tersebut. Perubahan ini hanya satu langkah: satu kali **Urungkan** membatalkan seluruh tempelan, berapa kali pun Anda mengubah pilihan. Contoh: salin A1:B2, dengan B1 berisi **=A1*2**, lalu tempel di F5. Pilih **Nilai** dan G5 berisi angka 2 alih-alih **=F5*2**; pilih **Pertahankan Format Sumber** dan rumusnya kembali.

Dari papan ketik, ketuk tombol **Ctrl** (tombol **Control** pada Mac) setelah menempel untuk membuka menu tombol itu, lalu gunakan tombol panah dan **Enter** untuk memilih. Tombol itu hilang ketika Anda mengubah sel atau hal lain dalam buku kerja, menyalin sesuatu yang lain, menekan **Esc**, atau menggulir sel terakhir yang ditempel keluar dari pandangan (tombol itu muncul kembali ketika Anda menggulir kembali). Tombol ini tidak ditawarkan setelah gunting dan tempel, atau untuk teks dari program lain.

## Tempel Tautan, Tempel sebagai Hiperlink, dan Tempel Impor Teks

Tiga pilihan lain tersedia di panah di bawah **Tempel**. Ketiganya bekerja pada sel yang Anda salin di buku kerja yang sama, kecuali yang terakhir.

- **Tempel Tautan** memasukkan rumus ke setiap sel yang menunjuk ke sel yang disalin. Contoh: salin A1:B2, pilih D4, lalu gunakan **Tempel Tautan**: D4 menjadi **=Sheet1!$A$1**. Ketika A1 berubah, D4 ikut berubah. Sel yang disalin dan kosong akan menampilkan 0.
- **Tempel sebagai Hiperlink** memasukkan kata yang sama dengan yang ditampilkan sel yang disalin ke setiap sel, dan menjadikan setiap sel itu tautan ke sel yang disalin. Tahan **Ctrl** dan klik salah satunya untuk pergi ke sel tersebut. Fitur ini bekerja pada hingga 2.000 sel sekaligus.
- **Tempel Impor Teks** dipakai untuk teks dari program lain. Fitur ini membuka jendela yang sama dengan saat membuka berkas teks: Anda menentukan apa yang memisahkan bagian-bagian teks (koma, tab, spasi, titik koma, garis tegak, atau yang lain), apa yang membatasi sebuah bagian, baris pertama yang digunakan, dan cara membaca setiap kolom (apa adanya, sebagai teks, sebagai tanggal, atau dilewati). Sebagian teks menyusun bagian-bagiannya dalam kolom dengan lebar yang sama, dengan spasi di antaranya. Centang **Kolom memiliki lebar tetap**, lalu tentukan di mana setiap kolom dimulai: setiap baris dipotong pada posisi tersebut, dan spasi di sekitar setiap bagian dihapus. Pilih *teks* untuk kolom kode seperti 007, agar angka nolnya tetap ada. Bagian-bagian itu masuk ke dalam sel, mulai dari sel yang dipilih, sebagai satu langkah yang dapat diurungkan.

## Teks dengan koma, titik koma, atau kolom

Ketika teks yang Anda tempel dari program lain jelas merupakan tabel, JeffCrunch membuka jendela yang sama dengan saat membuka berkas teks, sehingga Anda dapat memeriksa cara pemisahannya sebelum apa pun masuk ke sel. Ini hanya terjadi pada **Tempel** biasa (**{key:Paste}**), tidak pernah pada **Tempel Khusus** atau salah satu pilihannya. Ini terjadi ketika teks memiliki setidaknya dua baris dan tidak ada tab, dan salah satu dari berikut berlaku: setiap baris memiliki jumlah bagian yang sama, setidaknya tiga, yang dipisahkan koma, titik koma, atau garis tegak, dengan isi di setiap kolom (bagian dalam tanda kutip boleh berisi koma); atau bagian-bagiannya tersusun dalam kolom dengan lebar yang sama, dengan celah spasi yang sama di setiap baris (setidaknya tiga baris, dan setidaknya dua celah). Angka dengan pemisah ribuan berupa koma, seperti 1,234, serta baris kode atau daftar yang diakhiri koma atau titik koma tidak dianggap sebagai tabel. Jendela dimulai dengan cara teks terdeteksi terpisah. Jika angka memakai koma dan bagian-bagiannya dipisahkan titik koma, koma itu adalah tanda desimal. Klik **OK** untuk memasukkan bagian-bagiannya, atau **Batal** untuk tidak memasukkan apa pun.

Contoh: salin baris berikut dari sebuah email.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Tempelkan dengan kolom ketiga, misalnya harga, dan jendela akan menampilkan tiga kolom, dengan **Koma** tercentang. Satu kalimat dalam satu baris, atau baris yang jumlah komanya berbeda, atau hanya dua bagian per baris, ditempel sebagai kata, tanpa jendela. Teks dengan tab langsung masuk ke sel, seperti biasa. Untuk membuka jendela bagi teks apa pun, pilih **Tempel Impor Teks** (di atas).

Hal yang sama terjadi ketika Anda menyeret teks dari program lain dan melepaskannya di kisi: teks masuk mulai dari sel yang dipilih. Pelepasan di bagian lain jendela tidak melakukan apa-apa.

## Atur Tempel Default

**Atur Tempel Default** adalah pilihan terakhir di panah di bawah **Tempel**. Pilihan ini menentukan apa yang dimasukkan tombol **Tempel** dan **{key:Paste}** ketika Anda menyalin sel: semuanya (pada awalnya), atau hanya nilainya, hanya rumusnya, hanya formatnya, dan seterusnya. Contoh: pilih *Nilai*, lalu rumus yang disalin masuk sebagai angka yang ditampilkannya. Pilihan ini disimpan untuk penggunaan berikutnya. Pilihan ini tidak mengubah **Tempel Khusus** atau pilihan dalam menu, dan teks dari program lain ditempel apa adanya. Pilih *Semua* untuk kembali.

Pilihan yang sama ada di halaman **Pengeditan** pada **Opsi**, dalam dua kotak: satu untuk sel yang disalin di buku kerja ini, dan satu untuk sel yang disalin di buku kerja lain. Sampai Anda memilih satu untuk buku kerja lain, pilihan itu mengikuti yang pertama.

## Menu Tempel dan gambar

Panah di bawah **Tempel** mencantumkan pilihan biasa, serta **Tanpa Bingkai**, **Rumus dan Pemformatan Angka**, **Nilai dan Pemformatan Angka**, **Pertahankan Lebar Kolom Sumber**, dan **Komentar dan Catatan**. Pilihan-pilihan itu sama dengan yang ada di Tempel Khusus, masing-masing dengan satu klik.

Panah di sebelah **Salin** memiliki **Salin sebagai Gambar** dan **Salin sebagai Bitmap**. Masing-masing menempatkan gambar sel yang dipilih ke papan klip, digambar seperti yang Anda lihat, dengan garis kisinya. Bitmap berukuran sebesar layar. Gambar memiliki dua kali lipat jumlah titik, sehingga tetap tajam ketika diperbesar. **Salin sebagai Gambar Cetak** menggambar sel seperti yang akan ditampilkan kertas: garis kisi hanya ada jika pengaturan halaman mencetaknya. Tempel gambar itu ke JeffWrite atau JeffDeck sebagai gambar. Jika sel tidak muat dalam satu gambar, pilih sel yang lebih sedikit.

## Antarprogram

Ketika Anda menyalin, JeffCrunch juga menempatkan sel ke papan klip sebagai teks (sel dipisahkan tab, satu baris per baris) dan sebagai tabel untuk halaman web, sehingga program lain dapat mengambilnya. Ketika Anda menempel sesuatu yang tidak berasal dari JeffCrunch, seperti teks dengan tab, atau tabel dari halaman web atau lembar kerja lain, setiap bagian masuk ke sel sendiri dan dibaca seolah-olah Anda yang mengetiknya: angka, tanggal, dan rumus yang diawali tanda sama dengan. Menyalin dari buku kerja lain yang sedang dibuka di JeffCrunch membawa nilai, rumus, dan format angka, tetapi tidak format lainnya. Tempelkan di JeffDeck atau JeffPub, dan hasilnya menjadi tabel yang mempertahankan font, warna latar, bingkai, serta angka seperti yang ditampilkan (lihat **Tempel sel sebagai tabel** pada Bantuan program tersebut).

## Yang tidak dapat ditempel

JeffCrunch memberi tahu Anda dan tidak mengubah apa pun ketika tempelan akan masuk ke sebagian **rumus larik**, ke baris nama kolom sebuah **tabel**, ke sel terkunci pada **lembar yang dilindungi**, melewati akhir lembar, atau ke lebih dari 100.000 sel. Seluruh **rumus larik** dapat digunting dan dipindahkan (pilih semua selnya); sebagiannya, tabel data, atau baris nama tabel tidak bisa.

## Panel Papan Klip

Panel Papan Klip menyimpan 24 hal terakhir yang Anda salin atau gunting di program jendela ini, sehingga Anda dapat menempel yang lebih lama tanpa menyalinnya lagi. Klik panah kecil di sudut grup **Papan Klip** pada tab **Beranda** (atau tekan **Alt**, lalu **H**, lalu **FO**) untuk membukanya. Sebuah panel muncul di samping halaman.

Setiap item menampilkan pratinjau kecil dan jenis isinya: teks, teks berformat, sel, gambar, bentuk, atau slaid. Yang terbaru berada di paling atas. Klik sebuah item untuk menempelkannya di tempat kursor atau pilihan berada. Penempelan dilakukan seperti tempel biasa: satu langkah **Urungkan** membatalkannya, dan tombol Opsi Tempel muncul seperti biasa.

- **Tempel Semua** menempel setiap item, dari yang paling lama, sehingga urutannya sama dengan urutan Anda menyalinnya.
- **Hapus** menghapus item yang Anda pilih. **Kosongkan Semua** menghapus semuanya.
- Panel hanya mengumpulkan item selama panel itu terbuka. Untuk terus mengumpulkan setelah Anda menutupnya, centang **Tetap kumpulkan saat panel ini ditutup**. Pilihan ini diingat, dan pengumpulan dimulai ketika program dijalankan, sebelum Anda membuka panel.
- Untuk juga mengumpulkan apa yang Anda salin di program lain, centang **Juga kumpulkan yang disalin di program lain**. Hanya teks biasa dan gambar yang disimpan, dan fitur ini dimatikan lagi setiap kali Anda membuka program. Dengan pilihan ini aktif, salinan dari program lain dibaca seluruhnya sebelum program ini memutuskan apakah akan menyimpannya, sehingga salinan yang sangat besar dapat membuat program ini berhenti sejenak dan memakai memori sebesar salinan itu. Hanya satu bentuk yang dibaca: teksnya jika ada teks, jika tidak satu gambar. Dengan pilihan ini nonaktif, tidak ada yang disalin dari program lain yang dibaca.

Item disimpan hanya di memori program. Tidak ada yang disimpan ke berkas, dan daftar dikosongkan ketika Anda menutup jendela terakhir program. Item tetap ada dalam daftar setelah dokumen asalnya ditutup. Item yang lebih besar dari 8 MB tidak disimpan, dan panel memberi tahu Anda ketika itu terjadi; gambar dihitung 4 byte untuk setiap titik (piksel), sehingga gambar dengan lebih dari sekitar 2 juta piksel terlalu besar. Semua item bersama-sama dibatasi 64 MB, dan yang paling lama dihapus lebih dulu. Apa pun yang ditandai rahasia oleh pengelola kata sandi tidak pernah disimpan.
