# Mengelola lembar

<!-- keywords: lembar, sheet, tab, tab lembar, tambah, sisipkan, lembar baru, hapus, buang, ganti nama, nama, pindahkan, salin, duplikat, urutkan ulang, seret, sembunyikan, tampilkan, perlihatkan, warna tab, warna, pilih beberapa lembar, Shift+F11, #REF!, referensi 3-D, Pindahkan atau Salin, Pindahkan atau Salin Lembar, ganti nama lembar -->

Buku kerja terdiri atas **lembar**, dan **tab** di bagian bawah jendela adalah nama-nama lembar tersebut. Klik sebuah tab untuk menampilkan lembar itu. Setiap perubahan di bawah ini adalah satu langkah yang dapat Anda urungkan dan disimpan di dalam buku kerja.

## Menambahkan lembar

- Klik tombol **+** setelah tab terakhir untuk menambahkan lembar di akhir.
- Tekan **Shift+F11**, atau gunakan **Beranda > Sisipkan > Sisipkan Lembar**, untuk menambahkan lembar di depan lembar yang sedang Anda lihat.

Lembar baru kosong, mendapat nama berikutnya yang belum dipakai (Sheet4, Sheet5, dan seterusnya), dan langsung terbuka. Sebuah buku kerja dapat memiliki hingga 1.000 lembar; jika sudah mencapai batas itu, JeffCrunch memberi tahu Anda dan tidak menambahkan lembar, karena berkas dengan lebih banyak lembar bukan berkas yang dapat dibuka olehnya.

## Mengganti nama lembar

**Klik dua kali** tab, ketik nama baru, lalu tekan **Enter** (**Esc** untuk membatalkan). Anda juga dapat menggunakan **Beranda > Format > Ganti Nama Lembar**.

Nama lembar dapat terdiri atas hingga 31 karakter. Nama tidak boleh kosong, tidak boleh memuat salah satu karakter **: \ / ? * [ ]**, tidak boleh diawali atau diakhiri dengan tanda kutip tunggal, dan tidak boleh sama dengan nama lembar lain (huruf besar dan kecil tidak dianggap berbeda). Nama "History" dicadangkan untuk program. Jika nama ditolak, JeffCrunch memberi tahu alasannya dan lembar tetap menggunakan namanya semula.

Setiap rumus yang menyebut lembar tersebut mengikuti nama barunya. Contoh: **=Budget!B2** menjadi **='Q1 Budget'!B2** ketika Budget diganti namanya menjadi Q1 Budget. Nama yang menunjuk ke lembar itu, aturan pemformatan bersyarat dan validasi, serta bagan juga ikut berubah.

## Menghapus lembar

Klik kanan tab lalu pilih **Hapus Lembar**, atau gunakan **Beranda > Hapus > Hapus Lembar**. Jika lembar berisi sesuatu, JeffCrunch akan bertanya terlebih dahulu. Rumus pada lembar lain yang membaca lembar yang dihapus akan menampilkan **#REF!**, yang berarti data yang dibacanya sudah tidak ada. **Urungkan** akan mengembalikan lembar dan rumus tersebut.

Sebuah buku kerja selalu menyisakan paling sedikit satu lembar yang ditampilkan.

## Memindahkan atau menyalin lembar

- **Seret** tab ke tempat baru untuk memindahkan lembar.
- Atau gunakan **Beranda > Format > Pindahkan atau Salin Lembar**. Pilih lembar yang akan diletakkan di depan lembar yang dipilih (atau **(setelah lembar terakhir)**), lalu centang **Salin, dan biarkan aslinya di tempatnya** untuk menyalin lembar, bukan memindahkannya. Salinan akan diberi nama seperti **Budget (2)** dan langsung terbuka.

Rumus tetap membaca lembar yang dibacanya, di mana pun lembar-lembar itu berada. Salinan memiliki sel, rumus, format, dan ukuran yang sama dengan lembar asalnya. Referensi tanpa nama lembar (seperti **A1**) membaca sel milik salinan itu sendiri; referensi yang menyebut nama lembar (seperti **Sheet1!A1**) tetap menunjuk lembar tersebut, meskipun lembar itu adalah lembar asal salinan.

Salinan juga memiliki proteksi, filter, tautan, pengaturan halaman dan pemisah halaman, kelompok baris dan kolom, serta tampilan batang data milik lembar asal. Tautan mengikuti baris yang Anda sisipkan atau hapus, di lembar asal sebelum salinan dibuat dan di salinan setelahnya. Salinan tidak memiliki gambar, bagan, catatan, tabel pivot, sparkline, atau nama yang hanya milik lembar asal, karena JeffCrunch belum dapat menyalinnya; program akan memberi tahu Anda apa saja yang tidak tersalin. Lembar yang memiliki tabel disalin beserta salinan tabelnya (masing-masing dengan nama, id, dan bagiannya sendiri, dan rumus pada salinan menyebut tabelnya sendiri). Menghapus lembar juga menghapus tabelnya; rumus pada lembar lain yang menyebut tabel tersebut kemudian menampilkan #REF!, dan Urungkan akan mengembalikannya.

Untuk menyalin lembar ke **buku kerja lain** yang sedang terbuka, pilih buku kerja tersebut pada **Ke buku kerja** di kotak yang sama. Kotak ini menampilkan apa saja yang tidak akan dimiliki salinan (untuk saat ini, gambar, bagan, format bersyarat, tabel, catatan, dan cara lembar dicetak) dan memberi tahu Anda sebelum apa pun disalin. Rumus yang membaca lembar lain dari buku kerja pertama menjadi tautan ke berkas buku kerja tersebut, sehingga buku kerja pertama harus pernah disimpan sebagai berkas .xlsx minimal satu kali. Salinan menampilkan nilai yang dimiliki buku kerja pertama saat ini, dan nilai itu berubah ketika tautan diperbarui (**Perbarui Nilai** pada **Data > Edit Tautan**). Jika buku kerja pertama memiliki perubahan yang belum disimpan, tautan akan membaca berkas tersimpannya sekali ketika diperbarui, jadi simpanlah buku kerja itu agar nilai-nilai ini tetap. Rumus yang membaca lembar yang disalin itu sendiri tetap berada di salinan. Sel yang menggunakan nama, tabel, larik, atau tumpahan, atau yang membaca buku kerja ketiga, disalin sebagai nilai yang ditampilkannya, dan kotak akan menyebutkan jumlahnya. Cara lembar ditampilkan (baris dan kolom yang dibekukan, zoom, garis kisi, dan cara lembar membaca), serta tinggi baris dan lebar kolom bawaannya, ikut disalin. Salinan memiliki tampilan yang sama: format, font, dan warna ditambahkan ke buku kerja lain (warna dari tema buku kerja pertama tetap dipertahankan sebagai warna yang ditampilkannya jika kedua tema berbeda). Kedua buku kerja harus menghitung tanggal dari tahun yang sama (1900 atau 1904). **Urungkan** di buku kerja lain akan menghapus salinan itu; buku kerja pertama tidak berubah. Memindahkan lembar ke buku kerja lain belum tersedia.

## Menyembunyikan dan menampilkan lembar

Klik kanan tab lalu pilih **Sembunyikan Lembar** (atau gunakan **Beranda > Format > Sembunyikan Lembar**). Lembar tidak ditampilkan, tetapi tetap berada di dalam buku kerja dan rumus tetap membacanya. Untuk menampilkannya kembali, klik kanan salah satu tab lalu pilih **Tampilkan Lembar**, kemudian klik namanya di dalam daftar. Daftar kosong jika tidak ada lembar yang disembunyikan.

## Memberi warna tab

Klik kanan tab lalu pilih **Warna Tab**, kemudian pilih sebuah warna, atau pilih **Tanpa Warna**. Warna akan tampil sebagai garis di bawah nama. Warna yang Anda pilih dari palet tema disimpan sebagai warna biasa sesuai warna yang tampil saat ini.

## Beberapa lembar sekaligus

**Ctrl+klik** tab untuk memilih lebih dari satu, atau klik satu tab lalu **Shift+klik** tab lain untuk memilih semua tab di antaranya. Tab yang dipilih diberi warna latar. **Hapus**, **Sembunyikan**, **Warna Tab**, dan **Pindahkan atau Salin** kemudian bekerja pada semuanya sebagai satu langkah. Klik tab yang belum dipilih untuk memilih tab itu saja.

Apa pun yang Anda ketik atau format saat beberapa lembar dipilih hanya mengubah lembar yang sedang Anda lihat.

## Yang tidak dapat dilakukan

- Buku kerja yang memiliki rumus atas sekumpulan lembar, seperti **=SUM(Sheet1:Sheet3!A1)**, belum dapat ditambahi, dihapus, diganti namanya, atau dipindahkan lembarnya, karena tindakan itu akan mengubah lembar mana saja yang dicakup rumus.
- Setelah Anda menyimpan, **Urungkan** tidak dapat mundur melewati lembar yang Anda tambahkan, salin, hapus, ganti namanya, atau pindahkan sebelum penyimpanan. Berkas tersimpan memuatnya seperti apa adanya. Menyembunyikan, menampilkan, dan warna tab masih dapat diurungkan.
