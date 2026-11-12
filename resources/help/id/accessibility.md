# Pembaca layar, kontras tinggi, dan teks alternatif
<!-- keywords: aksesibilitas, pembaca layar, narrator, tunanetra, low vision, kontras tinggi, mode gelap, tema gelap, garis fokus, teks alternatif, alternative text, deskripsi gambar, zoom, perbesar, pembesaran, hanya keyboard -->

JeffPub dibuat agar orang yang menggunakan pembaca layar, keyboard, atau warna kontras tinggi tetap dapat memakainya.

## Pembaca layar

**Pembaca layar** adalah program yang membacakan isi layar. JeffPub bekerja dengan pembaca layar seperti Narrator di Windows. Setiap kontrol memiliki nama dan deskripsi. Ini mencakup tab, tombol, galeri, dan tombol warna pada pita, daftar Halaman, halaman, panel tugas, serta bilah status.

Objek pada halaman juga dibacakan. Pada halaman, tekan **Tab** untuk memilih objek berikutnya: pembaca layar akan menyebutkan jenisnya dan isinya, misalnya "Kotak teks: Diskon musim semi", "Gambar: sebuah lumbung merah", lalu posisi dan ukurannya. Gambar tanpa teks alternatif dibacakan sebagai "Gambar, tanpa teks alternatif", sehingga Anda tahu gambar mana yang masih memerlukannya. Tekan **Enter** untuk mengetik di dalam kotak teks; pembaca layar kemudian dapat membacakan teksnya, baris atau kata tempat kursor berada, dan apa yang Anda pilih.

- Daftar Halaman membacakan setiap halaman sebagai "Halaman 1", "Halaman 2", dan seterusnya. Jika Anda memberi nama pada halaman, nama tersebut mengikuti, misalnya "Halaman 3, Sampul".
- Area halaman disebut "Halaman". Deskripsinya memberi tahu tombol yang digunakan: Tab memilih objek berikutnya, tombol panah memindahkannya, Enter menyunting teksnya, dan Delete menghapusnya.
- Tab dibacakan sebagai tab pita. Tab yang muncul saat Anda memilih sesuatu juga menyebutkan grupnya, misalnya Alat Gambar.

## Menggunakan keyboard

Anda dapat mencapai pita, daftar Halaman, dan objek pada halaman hanya dengan keyboard. Tekan F6 untuk berpindah di antara pita, daftar Halaman, dan halaman. Tekan Alt untuk melihat KeyTip. Garis berwarna menunjukkan kontrol yang sedang memiliki fokus. Lihat [Gunakan JeffPub dengan keyboard](keyboard).

## Mengubah warna

- **Gelap atau terang**: klik **Berkas > Opsi**, lalu pada tab **Umum**, atur **Tema** ke **Terang**, **Gelap**, atau **Gunakan pengaturan sistem**. Perubahan langsung terlihat.
- **Kontras tinggi**: di Windows, saat Anda mengaktifkan tema kontras tinggi, JeffPub mengikutinya. JeffPub memakai warna yang dipilih Windows, dan berubah saat tema diaktifkan atau dinonaktifkan.

## Membuat gambar lebih mudah dipahami

**Teks alternatif** (alternative text) adalah deskripsi singkat tentang gambar atau objek lain. Pembaca layar membacakannya. JeffPub juga menyertakannya saat Anda menyimpan halaman web dan e-book.

1. Klik kanan gambar atau objek.
2. Klik **Format Objek…**.
3. Klik tab **Teks Alternatif**.
4. Ketik deskripsi pada kotak **Teks alternatif**.
5. Klik **OK**.

Jelaskan apa yang ditampilkan gambar, dalam beberapa kata, misalnya "Dua anak menanam pohon". Ikon dan kode batang yang Anda sisipkan otomatis mendapat teks alternatif.

## Memeriksa halaman

Klik **Tinjau > Periksa Aksesibilitas** untuk menampilkan daftar hal yang mungkin sulit digunakan oleh sebagian orang. Selain gambar dan bentuk tanpa teks alternatif, pemeriksa juga menemukan tiga hal lain.

- **Teks yang sulit dibaca.** Teks memerlukan **kontras** yang cukup dengan latar di belakangnya: isian kotak teksnya, atau warna halaman jika kotak tidak memiliki isian. Pemeriksa menghitung kontras sebagai rasio. Rasio 4,5 banding 1 cukup untuk teks biasa, dan 3 banding 1 cukup untuk teks besar (18 pt atau lebih, atau 14 pt atau lebih jika tebal). Misalnya, abu-abu #777777 di atas putih memiliki rasio 4,48 banding 1, sedikit di bawah batas, sedangkan #767676 memiliki rasio 4,54 banding 1. Pilih warna teks yang lebih gelap atau latar yang lebih terang. Jika teks berada di atas gambar, pemeriksa tidak dapat menentukannya, sehingga ia menambahkan saran untuk memeriksa kontras dengan mata.
- **Objek dibaca dalam urutan yang berbeda.** Pembaca layar dan PDF bertag membaca objek halaman dari belakang tumpukan ke depan. Jika urutan itu tidak dari atas ke bawah, lalu dari kiri ke kanan, pemeriksa mendaftar objek yang tidak sesuai posisinya sebagai saran. Klik **Urutan Baca** untuk melihat objek dalam urutan pembacaannya. Pilih satu objek lalu klik **Pindahkan Lebih Awal** atau **Pindahkan Lebih Akhir**, atau seret objek tersebut. Setiap pemindahan adalah satu langkah Urungkan, dan juga mengubah objek mana yang berada di depan.
- **Tabel tanpa baris header.** Pembaca layar menggunakan baris pertama tabel untuk menamai kolomnya. Tabel yang baris pertamanya tidak ditandai sebagai baris header akan dicantumkan sebagai peringatan. Klik masalahnya, lalu klik **Gunakan baris pertama sebagai header** untuk mengaktifkan baris header. Tindakan ini adalah satu langkah Urungkan.

## Memperbesar halaman

Perbesar tampilan untuk membaca teks kecil. Gunakan kontrol **Zoom** pada bilah status, tahan Ctrl sambil memutar roda mouse, atau pilih ukuran pada kotak **Zoom** di tab **Tampilan**. Penggeser zoom berkisar dari sekitar 10% hingga 1600%.

## Terkait

- [Gunakan JeffPub dengan keyboard](keyboard)
- [Tambahkan dan atur gambar](pictures)
- [Ubah pengaturan JeffPub](options)
- [Simpan sebagai gambar, halaman web, atau e-book](export)
