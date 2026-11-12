# Format angka dan bahasa Anda
<!-- keywords: format angka, angka, desimal, tanda desimal, tempat desimal, ribuan, pemisah, pengelompokan, koma, titik, bahasa, locale, mata uang, persen, persentase, tanggal, waktu, ilmiah, pecahan, teks, akuntansi, umum, kustom, kode, contoh, Format Sel, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, tambah desimal, kurangi desimal, gaya koma, pemisah rumus, titik koma -->

Sebuah sel berisi angka, dan **format angka**-nya menentukan bagaimana angka itu ditulis di layar. Format hanya mengubah apa yang Anda lihat, bukan apa yang tersimpan di dalam sel. Contoh: sel berisi 0,256. Dengan format Persen, sel menampilkan **25,6%**, dengan Mata Uang menampilkan **Rp0,26**, dan dengan Umum menampilkan **0,256**. Klik sel itu, dan bilah rumus tetap menampilkan 0,256.

## Angka ditulis sesuai cara komputer Anda

JeffCrunch menggunakan tanda-tanda dari bahasa yang diatur pada komputer Anda. Dalam bahasa Inggris (Amerika Serikat), seribu dua ratus tiga puluh empat setengah ditulis **1,234.5**. Dalam bahasa Jerman ditulis **1.234,5**, dan dalam bahasa Prancis **1 234,5**. Buku kerjanya sendiri tidak berubah: ia menyimpan satu bentuk netral, sehingga berkas yang sama menampilkan 1,234.5 bagi satu orang dan 1.234,5 bagi orang lain.

Hal ini berlaku di mana pun angka ditampilkan atau diketik:

- Sel, salinan sel, dan lebar yang diberikan **Sesuaikan Lebar Kolom** kepada kolom mengikuti tanda-tanda bahasa Anda.
- **Bilah rumus** dan sel yang Anda edit dengan **F2** menampilkan angka sebagaimana Anda mengetiknya. Jika bahasa Anda menulis desimal dengan koma, ketik **2,5**, bukan 2.5.
- Dalam bahasa yang menulis desimal dengan koma, rumus memakai **titik koma** di antara argumennya: **=SUM(A1;A2)**. Nama-nama fungsi tetap dalam bahasa Inggris.
- Tanggal dan waktu mengikuti kode format: format yang berbunyi "bulan, hari, tahun" menampilkannya dalam urutan itu, dan format yang dibuat untuk bahasa Anda (yang sudah tersedia dalam daftar) menggunakan urutan dan nama dari bahasa Anda.

Teks yang diberi tanda kutip oleh format, seperti "satuan", ditampilkan apa adanya.

## Memilih format

1. Pilih sel-selnya.
2. Gunakan grup **Angka** pada tab **Beranda**, atau tekan **Ctrl+1** (di Mac, **Command+1**) untuk membuka **Format Sel**, yang terbuka pada tab **Angka**.

Kotak di grup **Angka** memuat format yang umum: Umum, Angka, Mata Uang, Akuntansi, Tanggal Pendek, Tanggal Panjang, Waktu, Persentase, Pecahan, Ilmiah, Teks, dan Kustom. **Format Angka Lainnya** di bagian akhir membuka jendela Format Sel. Tombol-tombol di bawah kotak adalah:

| Tombol | Fungsinya |
| --- | --- |
| Mata Uang | Menampilkan angka sebagai uang, dengan simbol mata uang dari bahasa Anda (**Ctrl+Shift+4**). Panah di sebelahnya menawarkan **Format Angka Akuntansi** dengan pilihan mata uang (dolar AS, poundsterling Inggris, euro, yen Jepang, yuan Tiongkok, atau franc Swiss), Umum, dan **Format Angka Lainnya**. |
| Gaya Persen | Mengalikan dengan 100 dan menambahkan tanda persen, tanpa desimal (**Ctrl+Shift+5**). Klik lagi untuk menghapusnya. |
| Gaya Koma | Mengelompokkan digit per ribuan, menampilkan dua desimal, dan menyejajarkan tanda desimal ke bawah dalam sebuah kolom, tanpa simbol. |
| Tambah Desimal, Kurangi Desimal | Menambah atau mengurangi satu tempat desimal. |
| Umum (di kotak, dan di panah sebelah Mata Uang) | Menghapus format: angka ditampilkan apa adanya (**Ctrl+Shift+~**). |

## Tab Angka pada Format Sel

Pilih **Kategori** di sebelah kiri; opsi untuk kategori itu muncul di sebelah kanan, dan **Contoh** menunjukkan bagaimana sel pertama yang dipilih akan tampak, dalam bahasa Anda.

- **Angka**: tempat desimal, apakah digit dikelompokkan per ribuan, dan bagaimana angka negatif tampil (tanda minus, merah, atau dalam tanda kurung).
- **Mata Uang** dan **Akuntansi**: tempat desimal dan simbol. Akuntansi menyejajarkan simbol dan tanda desimal ke bawah dalam sebuah kolom.
- **Tanggal** dan **Waktu**: daftar format siap pakai, dengan urutan dan nama dari bahasa Anda.
- **Persentase**, **Ilmiah**: tempat desimal.
- **Pecahan**: format siap pakai seperti "hingga satu digit" (1/4), "hingga dua digit" (21/25), "sebagai seperempat", dan "sebagai seperenam belas".
- **Khusus**: format siap pakai untuk kode pos, kode pos plus 4, nomor telepon, dan nomor jaminan sosial.
- **Teks**: sel menyimpan apa yang Anda ketik sebagai teks, sehingga angka seperti 00123 mempertahankan nolnya.
- **Kustom**: ketik kode Anda sendiri, atau mulailah dari salah satu dalam daftar (kode yang umum, dan yang sudah digunakan buku kerja).

**Kode** adalah teks pendek yang memberi tahu JeffCrunch cara menulis sebuah angka. Contoh: **#,##0.00** berarti "kelompokkan digit per ribuan dan selalu tampilkan dua desimal", sehingga 1234.5 ditampilkan sebagai 1,234.50. Setiap **0** adalah digit yang selalu tampil, setiap **#** adalah digit yang hanya tampil jika diperlukan, dan **%** mengalikan dengan 100. Sebuah kode dapat memiliki hingga empat bagian yang dipisahkan titik koma, untuk angka positif, angka negatif, nol, dan teks.

## Yang disimpan

Kode disimpan di dalam buku kerja dalam bentuk netralnya, sehingga buku kerja terbaca dengan benar di komputer mana pun. Pemformatan adalah satu langkah yang dapat Anda urungkan, dan buku kerja yang sudah memakai sebuah format tetap mempertahankannya dengan persis: format yang ada dalam daftar akan ditemukan kembali, dan tidak ada yang dibuat dua kali.
