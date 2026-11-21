# Pembaca layar, kontras tinggi, dan teks alternatif
<!-- keywords: aksesibilitas, pembaca layar, screen reader, narrator, tunanetra, buta, penglihatan lemah, kontras tinggi, mode gelap, tema gelap, garis fokus, teks alternatif, alt text, deskripsi gambar, zoom, perbesar, hanya keyboard, papan ketik saja -->

JeffPub dibuat agar orang yang menggunakan pembaca layar, papan ketik, atau warna kontras tinggi tetap dapat memakainya.

## Pembaca layar

**Pembaca layar** adalah program yang membacakan isi layar. JeffPub bekerja dengan pembaca layar seperti Narrator di Windows. Setiap kontrol memiliki nama dan deskripsi. Ini mencakup tab, tombol, galeri, dan tombol warna pada pita, daftar Halaman, halaman, panel tugas, dan bilah status.

Objek pada halaman juga dibacakan. Pada halaman, tekan **Tab** untuk memilih objek berikutnya: pembaca layar menyebutkan jenisnya dan isinya, misalnya "Kotak teks: Diskon musim semi" atau "Gambar: lumbung merah", lalu posisi dan ukurannya. Gambar tanpa teks alternatif dibacakan sebagai "Gambar, tanpa teks alternatif", sehingga Anda tahu mana yang masih memerlukannya. Tekan **Enter** untuk mengetik di kotak teks; pembaca layar kemudian dapat membacakan teksnya, baris atau kata tempat kursor berada, dan bagian yang Anda pilih.

- Daftar Halaman membacakan setiap halaman sebagai "Halaman 1", "Halaman 2", dan seterusnya. Jika Anda memberi nama pada halaman, nama itu mengikuti, misalnya "Halaman 3, Sampul".
- Area halaman disebut "Halaman". Deskripsinya memberi tahu tombol yang dipakai: Tab memilih objek berikutnya, tombol panah memindahkannya, Enter menyunting teksnya, dan Delete menghapusnya.
- Tab dibacakan sebagai tab pita. Tab yang muncul saat Anda memilih sesuatu juga menyebut nama grupnya, misalnya Alat Gambar.

## Menggunakan papan ketik

Anda dapat menjangkau pita, daftar Halaman, dan objek pada halaman hanya dengan papan ketik. Tekan F6 untuk berpindah di antara pita, daftar Halaman, dan halaman. Tekan Alt untuk melihat Tip Tombol. Garis berwarna menunjukkan kontrol yang sedang memiliki fokus. Lihat [Gunakan JeffPub dengan papan ketik](keyboard).

## Mengubah warna

- **Terang atau gelap**: klik **Berkas > Opsi**, lalu pada tab **Umum**, atur **Tema** ke **Terang**, **Gelap**, atau **Gunakan pengaturan sistem**. Perubahan langsung terlihat.
- **Kontras tinggi**: di Windows, saat Anda mengaktifkan tema kontras tinggi, JeffPub mengikutinya. JeffPub memakai warna yang dipilih Windows, dan ikut berubah saat tema diaktifkan atau dinonaktifkan.

## Membuat gambar lebih mudah dipahami

**Teks alternatif** (alternative text) adalah deskripsi singkat tentang gambar atau objek lain. Pembaca layar membacakannya. JeffPub juga menyertakannya saat Anda menyimpan halaman web dan e-book.

1. Klik kanan gambar atau objek.
2. Klik **Format Objek…**.
3. Klik tab **Teks Alternatif**.
4. Ketik deskripsi pada kotak **Teks alternatif**.
5. Klik **OK**.

Tuliskan apa yang ditampilkan gambar, dalam beberapa kata, misalnya "Dua anak menanam pohon". Ikon dan kode batang yang Anda sisipkan otomatis mendapat teks alternatif.

## Memeriksa halaman

Klik **Tinjau > Periksa Aksesibilitas** untuk mendaftar hal yang mungkin sulit digunakan sebagian orang. Selain gambar dan bentuk tanpa teks alternatif, pemeriksa menemukan lima hal lain.

- **Teks yang sulit dibaca.** Teks memerlukan **kontras** yang cukup dengan latar di belakangnya: isian kotaknya, atau warna halaman jika kotak tidak berisi. Pemeriksa menghitung kontras sebagai rasio. Rasio 4,5 banding 1 cukup untuk teks biasa, dan 3 banding 1 cukup untuk teks besar (18 poin atau lebih, atau 14 poin atau lebih dengan huruf tebal). Misalnya, abu-abu #777777 di atas putih adalah 4,48 banding 1, sedikit kurang, sedangkan #767676 adalah 4,54 banding 1. Pilih warna teks yang lebih gelap atau latar yang lebih terang. Jika teks berada di atas gambar, pemeriksa tidak dapat menghitungnya, sehingga ia menambahkan saran untuk memeriksa kontras dengan mata.
- **Objek dibaca dalam urutan yang berbeda.** Pembaca layar dan PDF bertag membaca objek halaman dari tumpukan paling belakang ke paling depan. Jika urutannya tidak dari atas ke bawah, lalu dari kiri ke kanan, pemeriksa mendaftar objek yang salah tempat sebagai saran. Klik **Urutan Baca** untuk melihat objek sesuai urutan dibacanya. Pilih satu objek lalu klik **Baca Lebih Awal** atau **Baca Kemudian**, atau seret. Setiap pemindahan menjadi satu langkah Urungkan, dan juga mengubah objek mana yang berada di depan.
- **Tabel tanpa baris judul.** Pembaca layar menggunakan baris pertama tabel untuk menamai kolomnya. Tabel yang baris pertamanya tidak ditandai sebagai baris judul didaftar sebagai peringatan. Klik masalahnya, lalu klik **Gunakan baris pertama sebagai header** untuk mengaktifkan baris judul. Ini menjadi satu langkah Urungkan.
- **Tabel dengan sel gabungan.** Pembaca layar dapat kehilangan posisi dalam tabel saat sel digabungkan, sehingga tabel yang memiliki sel gabungan didaftar sebagai peringatan. Pertahankan setiap sel tabel secara utuh: pisahkan sel yang digabung, atau gunakan dua tabel.
- **Tautan yang tidak menjelaskan tujuannya.** Pembaca layar dapat membacakan tautan pada halaman sebagai daftar, sehingga teks setiap tautan sebaiknya menyebutkan halaman yang akan dibukanya. Tautan yang teksnya berupa alamatnya sendiri, hanya "di sini", "klik di sini", "baca selengkapnya", atau kata serupa, atau kosong, didaftar sebagai peringatan. Ubah teksnya, misalnya "Laporan tahunan kami" alih-alih "klik di sini". 50 pertama didaftar satu per satu, dan sisanya dihitung dalam satu baris.

## Memperbesar halaman

Perbesar tampilan untuk membaca teks kecil. Gunakan kontrol **Zoom** pada bilah status, tahan Ctrl dan putar roda mouse, atau pilih ukuran pada kotak **Zoom** di tab **Tampilan**. Penggeser zoom berkisar dari sekitar 10% hingga 1600%.

## Terkait

- [Gunakan JeffPub dengan papan ketik](keyboard)
- [Tambahkan dan sesuaikan gambar](pictures)
- [Ubah pengaturan JeffPub](options)
- [Simpan sebagai gambar, halaman web, atau e-book](export)
