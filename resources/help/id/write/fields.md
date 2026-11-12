# Bidang
<!-- keywords: bidang, field, fields, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, perbarui bidang, update field, F9, lepas tautan, unlink, kunci, lock, kode bidang, field codes, nomor halaman, page number, tanggal, date, NUMPAGES, SEQ, REF, PAGEREF, rumus, formula, sisipkan bidang, Insert Field, edit bidang, edit field, klik kanan, right-click -->

**Bidang** adalah teks yang dihitung oleh program untuk Anda, misalnya nomor halaman, tanggal hari ini, atau nomor gambar. Bidang memiliki dua bagian: **kode**, yang menyatakan apa yang harus dihitung, dan **hasil**, yaitu teks yang Anda lihat. Kode untuk nomor halaman adalah `PAGE`, dan hasilnya pada halaman ketiga adalah `3`.

## Menyisipkan bidang ke dalam teks

1. Klik tempat bidang akan ditempatkan.
2. Klik **Sisipkan > Bidang**.
3. Pilih bidang dari daftar. Kodenya muncul di kotak di bawah, dan Anda dapat mengubahnya.
4. Klik **OK**.

Misalnya, pilih **Tanggal**, ubah kodenya menjadi `DATE \@ "MMMM d, yyyy"`, lalu klik **OK**. Teks menampilkan tanggal sebagai `November 2, 2026`.

## Mengubah kode bidang

Klik kanan bidang lalu pilih **Edit Bidang**. Sebuah kotak menampilkan kodenya. Ubah kodenya dan klik **OK**, lalu bidang akan dihitung ulang. **Urungkan** mengembalikan bidang lama. Bidang yang dikunci tidak dapat diedit sebelum Anda membukanya. Menu klik kanan juga memiliki **Perbarui Bidang**, **Alihkan Kode Bidang**, **Kunci Bidang**, **Buka Kunci Bidang**, dan **Lepas Tautan Bidang**.

## Memperbarui bidang

Sebuah bidang menampilkan hasil yang dimilikinya saat terakhir kali dihitung. Tekan **F9** untuk menghitung ulang bidang tempat kursor berada. Untuk memperbarui banyak bidang sekaligus, pilih teks yang memuatnya, lalu tekan **F9**. Tekan **Ctrl+A** lalu **F9** untuk memperbarui semua bidang dalam dokumen.

Pembaruan dilakukan dalam satu langkah: **Urungkan** mengembalikan semua hasil. Pembaruan bukan perubahan yang dilacak, meskipun Lacak Perubahan aktif, karena hasilnya bukan sesuatu yang Anda ketik.

Bidang di header atau footer yang menampilkan nomor halaman, jumlah halaman, atau judul selalu benar dan tidak perlu diperbarui.

## Menampilkan kode

Tekan **Shift+F9** untuk menampilkan kode bidang tempat kursor berada, dalam kurung kurawal seperti `{ PAGE }`, menggantikan hasilnya. Tekan **Alt+F9** untuk menampilkan kode semua bidang, dan tekan lagi untuk menampilkan hasilnya. Ini hanya mengubah apa yang Anda lihat: berkas tetap disimpan seperti semula, dan dokumen tidak ditandai sebagai berubah.

## Melepas tautan atau mengunci bidang

- **Ctrl+Shift+F9** mengubah bidang menjadi teks biasa. Hasilnya tetap ada, tetapi tidak lagi menjadi bidang.
- **Ctrl+F11** mengunci bidang sehingga **F9** tidak mengubahnya. **Ctrl+Shift+F11** membuka kuncinya.

## Format

Tambahkan sakelar ke dalam kode untuk mengubah tampilan hasil:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower`, dan `\* Caps` mengubah huruf besar/kecil atau jenis angka. `{ = 21 \* CardText }` menampilkan `twenty-one`.
- `\# "0.00"` mengatur tampilan angka. `{ = 1234.5 \# "#,##0.00" }` menampilkan `1,234.50`.
- `\@ "dddd, MMMM d"` mengatur tampilan tanggal. `M` adalah bulan, dan `m` adalah menit.

## Bidang yang dihitung JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, rumus yang diawali dengan `=`, TOC (juga daftar gambar), INDEX, TOA, CITATION, dan BIBLIOGRAPHY. Tanda tersembunyi TC, XE, dan TA tidak memiliki hasil yang perlu dihitung: tanda tersebut memberi tahu tabel, indeks, atau daftar rujukan apa yang harus dicantumkan. Bidang jenis lain mempertahankan hasil yang dimilikinya dan disimpan seperti semula.

DATE, TIME, FILENAME, dan AUTHOR bergantung pada komputer dan waktu, sehingga hasilnya dapat berubah setiap kali Anda memperbaruinya.
