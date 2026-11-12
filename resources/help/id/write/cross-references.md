# Referensi Silang
<!-- keywords: cross-reference, cross reference, REF, PAGEREF, NOTEREF, lihat halaman, lihat gambar, di atas, di bawah, nomor judul, tautan -->

**Referensi silang** adalah kata-kata dalam teks Anda yang menunjuk ke tempat lain, misalnya "lihat Gambar 2 di halaman 7". Kata-kata ini berupa bidang, sehingga jika gambar atau halaman berubah, Anda cukup memperbarui bidang tersebut dan kata-katanya akan mengikuti.

## Menyisipkan referensi silang

1. Klik tempat kata-kata itu akan ditempatkan.
2. Klik **References > Cross-reference**.
3. Pilih **Reference type**: judul, penanda, catatan kaki, catatan akhir, item bernomor, atau keterangan seperti Figure atau Table.
4. Pilih **Insert reference to**: teks, nomor, nomor halaman, atau "above" dan "below".
5. Pilih item dari daftar, biarkan **Insert as hyperlink** tercentang jika Anda ingin melompat ke sana, lalu klik **Insert**.

Sebagai contoh, untuk menulis "Lihat judul Hasil di halaman 4": ketik `Lihat judul `, sisipkan referensi ke judul tersebut dengan **Heading text**, ketik ` di halaman `, lalu sisipkan referensi kedua ke judul yang sama dengan **Page number**.

## Mengikuti referensi

Tahan **Ctrl** lalu klik referensi yang berupa hiperlink. Kursor akan berpindah ke tempat yang dirujuk.

## Menjaga kata-kata tetap benar

Setelah Anda menambah halaman, mengubah judul, atau memindahkan gambar, pilih teks lalu tekan **F9**. Setiap referensi akan menghitung ulang kata-katanya. Jika tempat yang dirujuk sudah dihapus, bidang akan menampilkan `Error: the bookmark this refers to is missing.`, dan Anda dapat memilih target baru.

## Apa yang terjadi di balik layar

JeffWrite menandai target dengan penanda tersembunyi (namanya diawali `_Ref`) dan membuat bidang yang merujuk ke penanda itu: `REF` untuk teks dan angka, `PAGEREF` untuk nomor halaman, dan `NOTEREF` untuk catatan. Program lain yang membuka berkas tersebut tetap dapat menggunakan referensi itu.
