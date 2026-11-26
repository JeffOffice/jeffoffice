# Bidang

<!-- keywords: bidang, field, fields, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, perbarui bidang, F9, lepas tautan, kunci, kode bidang, nomor halaman, tanggal, NUMPAGES, SEQ, REF, PAGEREF, rumus, Sisipkan Bidang, edit bidang, klik kanan -->

Sebuah **bidang** adalah potongan teks yang dihitung oleh program untuk Anda, misalnya nomor halaman, tanggal hari ini, atau nomor gambar. Bidang terdiri atas dua bagian: **kode**, yang menyatakan apa yang dihitung, dan **hasil**, yaitu teks yang Anda lihat. Kode untuk nomor halaman adalah `PAGE`, dan hasilnya pada halaman ketiga adalah `3`.

## Memasukkan bidang ke dalam teks

1. Klik tempat bidang akan diletakkan.
2. Klik **Sisipkan > Bidang**.
3. Pilih bidang dari daftar. Kodenya muncul di kotak di bawah, dan Anda dapat mengubahnya.
4. Klik **OK**.

Sebagai contoh, pilih **Tanggal**, ubah kodenya menjadi `DATE \@ "MMMM d, yyyy"`, lalu klik **OK**. Teks akan menampilkan tanggal sebagai `November 2, 2026`.

## Mengubah kode bidang

Klik kanan bidang lalu pilih **Edit Bidang**. Sebuah kotak menampilkan kodenya. Ubah kodenya lalu klik **OK**, dan bidang akan dihitung ulang. **Urungkan** mengembalikan bidang lama. Bidang yang terkunci tidak dapat diedit sebelum Anda membukanya kembali. Menu klik kanan juga memiliki **Perbarui Bidang**, **Alihkan Kode Bidang**, **Kunci Bidang**, **Buka Kunci Bidang**, dan **Lepas Tautan Bidang**.

## Memperbarui bidang

Bidang menampilkan hasil yang dimilikinya saat terakhir dihitung. Tekan **F9** untuk menghitung ulang bidang tempat kursor berada. Untuk memperbarui banyak bidang sekaligus, pilih teks yang memuatnya, lalu tekan **F9**. Tekan **Ctrl+A** lalu **F9** untuk memperbarui semua bidang dalam dokumen.

Pembaruan dilakukan dalam satu langkah: **Urungkan** mengembalikan semua hasil. Pembaruan bukan perubahan yang dilacak, sekalipun Lacak Perubahan aktif, karena hasilnya bukan sesuatu yang Anda ketik.

Bidang di header atau footer yang menampilkan nomor halaman, jumlah halaman, atau judul selalu tepat dan tidak perlu diperbarui.

## Menampilkan kode

Tekan **Shift+F9** untuk menampilkan kode bidang tempat kursor berada, dalam kurung kurawal seperti `{ PAGE }`, menggantikan hasilnya. Tekan **Alt+F9** untuk menampilkan kode semua bidang, dan tekan lagi untuk menampilkan hasilnya. Ini hanya mengubah tampilan: berkas disimpan seperti semula, dan dokumen tidak ditandai sebagai berubah.

## Memberi warna latar pada bidang

Untuk melihat di mana saja bidang berada, buka **Berkas > Opsi > Tampilan** lalu pilih **Pewarnaan bidang**. **Tidak Pernah** tidak menampilkan warna latar. **Selalu** memberi warna abu-abu muda pada semua bidang. **Saat Dipilih** hanya memberi warna pada bidang tempat kursor berada. Warna latar tampil di layar dan tidak pernah dicetak atau disimpan dalam berkas PDF.

## Perbarui bidang sebelum mencetak

Centang **Perbarui bidang sebelum mencetak** di **Berkas > Opsi > Tampilan** agar semua bidang dihitung ulang setiap kali Anda mencetak atau menyimpan sebagai berkas PDF, sehingga rujukan halaman, tanggal, dan jumlah benar di atas kertas. Ini adalah satu langkah yang dapat Anda urungkan. Ini tidak dilakukan untuk dokumen yang dibuka hanya untuk dibaca.

## Melepas tautan atau mengunci bidang

- **Ctrl+Shift+F9** mengubah bidang menjadi teks biasa. Hasilnya tetap ada, tetapi tidak lagi berupa bidang.
- **Ctrl+F11** mengunci bidang, sehingga **F9** membiarkannya. **Ctrl+Shift+F11** membuka kuncinya.

## Format

Tambahkan sakelar ke kode untuk mengubah tampilan hasil:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower`, dan `\* Caps` mengubah huruf besar-kecil atau jenis angka. `{ = 21 \* CardText }` menampilkan `twenty-one`.
- `\# "0.00"` mengatur tampilan angka. `{ = 1234.5 \# "#,##0.00" }` menampilkan `1,234.50`.
- `\@ "dddd, MMMM d"` mengatur tampilan tanggal. `M` adalah bulan, dan `m` adalah menit.

## Bidang yang dihitung JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, rumus yang diawali `=`, TOC (juga daftar gambar), INDEX, TOA, CITATION, dan BIBLIOGRAPHY. Tanda tersembunyi TC, XE, dan TA tidak memiliki hasil untuk dihitung: tanda ini memberi tahu tabel, indeks, atau daftar rujukan apa yang harus dicantumkan. Bidang jenis lain tetap mempertahankan hasilnya dan disimpan apa adanya.

DATE, TIME, FILENAME, dan AUTHOR bergantung pada komputer dan waktu, sehingga dapat berubah setiap kali Anda memperbarui.
