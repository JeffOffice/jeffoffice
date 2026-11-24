# Membuat slaid dari garis besar
<!-- keywords: slaid dari garis besar, impor garis besar, markdown, garis besar teks, rtf, docx, judul menjadi slaid, daftar poin menjadi slaid, memulai presentasi dari teks, simpan garis besar, ekspor garis besar -->

Anda dapat menulis presentasi sebagai teks biasa dan membiarkan JeffDeck mengubahnya menjadi slaid. Setiap slaid memiliki judul dan isi, dengan tata letak Judul dan Konten, sehingga Tampilan Garis Besar, Atur Ulang Slaid, dan tema tetap dapat digunakan seperti biasa.

## Menambahkan slaid ke presentasi yang sedang dibuka

1. Klik **Beranda > Slaid**, lalu panah kecil di bawah **Slaid Baru**, dan pilih **Slaid dari Garis Besar…**.
2. Pilih berkas. Berkas dapat berupa berkas teks (.txt), berkas Markdown (.md), berkas teks kaya (.rtf), atau berkas dokumen (.docx).
3. Slaid ditambahkan setelah slaid yang sedang Anda buka. Semuanya merupakan satu langkah, sehingga satu kali Ctrl+Z membatalkan semuanya.

## Atau memulai presentasi baru dari garis besar

Pilih **Berkas > Buka** lalu pilih berkas garis besar. JeffDeck membuat presentasi baru darinya. Berkas tersebut tidak pernah diubah, dan **Simpan** akan meminta nama.

## Cara berkas dibaca

**Berkas teks.** Baris yang dimulai dari tepi kiri menjadi judul slaid. Baris di bawahnya menjadi isi slaid. Baris yang menjorok lebih jauh daripada baris sebelumnya masuk satu tingkat lebih dalam (gunakan tab atau spasi). Poin (-, *, atau +) atau angka di awal baris dihapus, karena slaid memiliki poinnya sendiri.

**Berkas Markdown.** Tingkat judul teratas yang digunakan berkas membentuk slaid: # jika berkas memilikinya, jika tidak maka ##. Judul yang lebih kecil menjadi baris isi slaid, satu tingkat lebih dalam untuk setiap langkah. Baris berupa tiga tanda hubung (---) memulai slaid baru. Daftar menjadi tingkat. **Tebal**, *miring*, `kode`, dan tautan tetap tampil seperti aslinya. Blok kode yang diapit tiga tanda backtick pada baris tersendiri menjadi kotak teks dengan font mesin tik. Gambar pada baris tersendiri, ditulis dengan bentuk gambar Markdown (tanda seru, deskripsi di dalam kurung siku, dan nama berkas di dalam kurung biasa), akan ditempatkan pada slaid, tetapi hanya jika berkas gambar berada di folder yang sama dengan garis besar, atau di folder di dalamnya. Gambar dari internet atau dari folder lain tidak pernah dimuat.

**Berkas teks kaya dan .docx.** Paragraf dengan gaya Judul membentuk slaid: gaya judul tingkat teratas yang digunakan berkas menjadi judul, gaya judul yang lebih kecil menjadi baris isi, dan setiap paragraf lain menjadi baris di bawah judul di atasnya.

## Batas

Batas berikut berlaku untuk setiap jenis berkas garis besar, baik teks, Markdown, teks kaya, maupun .docx:

- Hingga 5.000 slaid. Yang melebihi batas ini tidak disertakan.
- Hingga 4.000 karakter dalam satu baris. Baris yang lebih panjang dipotong, begitu juga paragraf Markdown yang barisnya menyatu menjadi lebih panjang dari itu.
- Hingga 500 baris teks pada satu slaid, dan 100.000 baris secara keseluruhan.
- Teks slaid memiliki hingga 9 tingkat, dan baris yang menjorok lebih dalam ditampilkan pada tingkat terdalam.
- Hingga 200 baris kode pada satu slaid.
- Hingga 100 gambar secara keseluruhan dan 4 gambar pada satu slaid. Berkas gambar dapat berukuran hingga 32 MB, dan semua berkas gambar bersama-sama hingga 64 MB. Gambar harus berupa berkas PNG, JPEG, GIF, BMP, WebP, atau TIFF, apa pun namanya, dengan hingga 16 juta piksel (4.000 x 4.000), dan semua gambar bersama-sama hingga 64 juta piksel.

Ukuran berkas itu sendiri juga dibatasi. Berkas teks atau Markdown dapat berukuran hingga 10 MB, dalam UTF-8 atau UTF-16. Berkas teks kaya atau .docx dapat berukuran hingga 96 MB. Berkas yang bukan berkas teks akan ditolak. Gambar dalam berkas teks kaya atau .docx tidak ditempatkan pada slaid, dan JeffDeck memberi tahu Anda berapa banyak gambar yang tidak disertakan.

Jika ada yang tidak disertakan, misalnya gambar yang tidak dapat digunakan atau teks yang melewati batas, JeffDeck memberi tahu Anda apa saja itu. Selama berkas besar dibaca, JeffDeck tetap bekerja, dan menutup jendela akan menghentikan pembacaan.

## Menyimpan salinan sebagai garis besar

Pilih **Berkas > Simpan Salinan** lalu pilih **Garis Besar Markdown (*.md)**. Berkas tersebut berisi judul dan teks setiap slaid, dengan tebal, miring, dan tautan. Gambar, bentuk, tabel, bagan, kotak teks lain, catatan pembicara, dan tampilan slaid tidak disertakan. JeffDeck memberi tahu Anda hal ini saat menyimpan, dan **Berkas > Info > Periksa Kompatibilitas** juga mencantumkannya.

## Terkait

- [Menambah, mengubah, dan menghapus slaid](slides)
- [Menggunakan ulang slaid dari presentasi lain](reuse)
- [Tampilan](views)
