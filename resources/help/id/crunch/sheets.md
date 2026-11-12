# Bekerja dengan lembar

<!-- keywords: lembar, sheet, tab, tab lembar, tambah, sisipkan, lembar baru, hapus, hapus lembar, ganti nama, nama, pindah, salin, duplikat, atur ulang urutan, seret, sembunyikan, tampilkan, tampilkan lembar, warna tab, warna, pilih beberapa lembar, Shift+F11, #REF!, referensi 3-D, Pindahkan atau Salin -->

Buku kerja terdiri atas **lembar**, dan **tab** di bagian bawah jendela adalah nama-namanya. Klik tab untuk menampilkan lembar tersebut. Setiap perubahan di bawah ini adalah satu langkah yang dapat diurungkan dan disimpan di dalam buku kerja.

## Menambah lembar

- Klik tombol **+** setelah tab terakhir untuk menambahkan lembar di akhir.
- Tekan **Shift+F11**, atau gunakan **Beranda > Sisipkan > Sisipkan Lembar**, untuk menambahkan lembar di depan lembar yang sedang Anda lihat.

Lembar baru kosong, mendapat nama kosong berikutnya (Sheet4, Sheet5, dan seterusnya), dan langsung terbuka. Satu buku kerja dapat memiliki hingga 1.000 lembar; jika melebihi itu, JeffCrunch memberi tahu dan tidak menambahkan lembar, karena berkas dengan lebih banyak lembar bukan berkas yang dapat dibukanya.

## Mengganti nama lembar

**Klik dua kali** tab, ketik nama baru, lalu tekan **Enter** (**Esc** membatalkannya). Anda juga dapat menggunakan **Beranda > Format > Ganti Nama Lembar**.

Nama dapat terdiri atas hingga 31 karakter. Nama tidak boleh kosong, tidak boleh memuat **: \ / ? * [ ]**, tidak boleh diawali atau diakhiri tanda kutip tunggal, dan tidak boleh sama dengan nama lembar lain (huruf besar dan kecil tidak dianggap berbeda). Nama "History" dicadangkan untuk program. Jika nama ditolak, JeffCrunch menjelaskan alasannya dan lembar tetap memakai nama lamanya.

Setiap rumus yang menyebut lembar akan mengikuti nama baru. Contoh: **=Budget!B2** menjadi **='Q1 Budget'!B2** saat Budget diganti namanya menjadi Q1 Budget. Nama yang mengacu ke lembar, aturan pemformatan bersyarat dan validasi, serta bagan juga ikut berubah.

## Menghapus lembar

Klik kanan tab lalu pilih **Hapus Lembar**, atau gunakan **Beranda > Hapus > Hapus Lembar**. Jika lembar berisi sesuatu, JeffCrunch akan bertanya terlebih dahulu. Rumus pada lembar lain yang membaca lembar yang dihapus akan menampilkan **#REF!**, yang berarti data yang dibacanya sudah tidak ada. **Urungkan** akan mengembalikan lembar dan rumus tersebut.

Buku kerja selalu menyimpan setidaknya satu lembar yang ditampilkan.

## Memindahkan atau menyalin lembar

- **Seret** tab ke tempat baru untuk memindahkan lembar.
- Atau gunakan **Beranda > Format > Pindahkan atau Salin Lembar**. Pilih lembar yang akan berada di depan lembar yang dipilih (atau **(setelah lembar terakhir)**), lalu centang **Salin, dan biarkan aslinya di tempatnya** untuk menyalin lembar, bukan memindahkannya. Salinan diberi nama seperti **Budget (2)** dan langsung terbuka.

Rumus tetap membaca lembar yang dibacanya, di mana pun lembar itu berada. Salinan memiliki sel, rumus, format, dan ukuran yang sama dengan lembar asalnya. Referensi tanpa nama lembar (misalnya **A1**) membaca sel milik salinan itu sendiri; referensi yang menyebut nama lembar (misalnya **Lembar1!A1**) tetap menunjuk lembar tersebut, meskipun itu adalah lembar asal salinan.

Salinan juga memiliki proteksi, filter, tautan, pengaturan halaman dan pemisah halaman, kelompok baris dan kolom, serta tampilan batang data dari lembar asal; tautan mengikuti baris yang Anda sisipkan atau hapus, di lembar asli sebelum salinan dibuat dan di salinan sesudahnya. Salinan tidak memiliki gambar, bagan, catatan, tabel pivot, sparkline, atau nama yang hanya milik lembar asal, karena JeffCrunch belum dapat menyalinnya; program akan memberi tahu Anda jika hal ini terjadi. Lembar yang memiliki tabel disalin beserta salinan tabelnya (masing-masing dengan nama, id, dan bagian sendiri, dan rumus pada salinan menyebut tabelnya sendiri). Menghapus lembar juga menghapus tabelnya; rumus pada lembar lain yang menyebut tabel tersebut akan menampilkan #REF!, dan **Urungkan** akan mengembalikannya.

## Menyembunyikan dan menampilkan lembar

Klik kanan tab lalu pilih **Sembunyikan Lembar** (atau gunakan **Beranda > Format > Sembunyikan Lembar**). Lembar tidak ditampilkan, tetapi tetap ada di dalam buku kerja dan rumus tetap membacanya. Untuk menampilkannya kembali, klik kanan tab mana pun lalu pilih **Tampilkan Lembar**, kemudian klik namanya dalam daftar. Daftar kosong jika tidak ada lembar yang disembunyikan.

## Memberi warna tab

Klik kanan tab lalu pilih **Warna Tab**, kemudian pilih warna, atau pilih **Tanpa Warna**. Warna ditampilkan sebagai garis di bawah nama. Warna yang Anda pilih dari palet tema disimpan sebagai warna biasa sesuai warna yang tampil saat ini.

## Beberapa lembar sekaligus

**Ctrl+klik** tab untuk memilih lebih dari satu, atau klik satu tab lalu **Shift+klik** tab lain untuk memilih semua tab di antaranya. Tab yang dipilih diberi warna latar. **Hapus**, **Sembunyikan**, **Warna Tab**, dan **Pindahkan atau Salin** kemudian berlaku pada semuanya sekaligus sebagai satu langkah. Klik tab yang belum dipilih untuk memilih tab itu saja.

Apa pun yang Anda ketik atau format saat beberapa lembar dipilih hanya berpengaruh pada lembar yang sedang Anda lihat.

## Yang tidak dapat dilakukan

- Buku kerja yang memiliki rumus atas sekumpulan lembar, seperti **=SUM(Sheet1:Sheet3!A1)**, belum dapat ditambahi, dihapus, diganti namanya, atau dipindahkan lembarnya, karena hal itu akan mengubah lembar mana yang dicakup rumus.
- Setelah Anda menyimpan, **Urungkan** tidak dapat kembali melewati lembar yang Anda tambahkan, salin, hapus, ganti namanya, atau pindahkan sebelum penyimpanan. Berkas yang tersimpan memuat lembar-lembar tersebut seperti apa adanya. Menyembunyikan, menampilkan, dan warna tab masih dapat diurungkan.
