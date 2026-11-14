# Menerjemahkan teks

<!-- keywords: menerjemahkan, terjemahan, penerjemah, bahasa, bahasa, spanyol, prancis, jerman, italia, portugis, belanda, rusia, ukraina, indonesia, luring, di komputer ini, unduh, model, ganti, sisipkan di bawah -->

**Tinjau > Bahasa > Terjemahkan** membuka panel **Terjemahkan**. Panel ini menerjemahkan teks Anda di komputer Anda sendiri. Kata-kata Anda tidak dikirim ke mana pun, dan koneksi internet tidak diperlukan setelah berkas terjemahan untuk suatu bahasa ada di komputer Anda.

## Menerjemahkan beberapa teks

1. Pilih teks yang ingin Anda terjemahkan. Jika tidak ada yang dipilih, panel menggunakan paragraf tempat kursor berada. Di kotak teks yang dipilih tetapi tidak sedang diedit, panel menggunakan seluruh teks dalam kotak tersebut.
2. Klik **Tinjau > Bahasa > Terjemahkan**. Panel menampilkan teks Anda di bawah **Asli**.
3. Di bawah **Dari**, pilih bahasa teks Anda. Di bawah **Ke**, pilih bahasa tujuan. Tombol **Tukar** menukar kedua bahasa tersebut.
4. Klik **Terjemahkan**. Terjemahan muncul di bawah **Terjemahan**. Kata yang cetak tebal, miring, dan bergaris bawah tetap cetak tebal, miring, dan bergaris bawah.
5. Klik **Ganti** untuk menempatkan terjemahan di tempat teks Anda semula, atau **Sisipkan di Bawah** untuk menambahkannya dalam paragraf baru di bawah teks Anda. Keduanya adalah satu langkah, sehingga **Urungkan** dapat membatalkannya.

Jika Anda memindahkan kursor atau mengubah teks setelah mengklik **Terjemahkan**, klik **Gunakan Pilihan** untuk membaca ulang teks, lalu klik **Terjemahkan** lagi. Panel tidak akan menempatkan terjemahan di atas teks yang bukan lagi teks yang diterjemahkannya.

**Batal** menghentikan terjemahan yang memakan waktu terlalu lama, dan menutup panel juga menghentikannya: jendela tidak pernah menunggunya. Teks panjang membutuhkan waktu lebih lama: satu halaman teks membutuhkan beberapa detik di sebagian besar komputer. Panel menerjemahkan paling banyak 1.000.000 huruf sekaligus (sekitar 200 halaman); jika lebih banyak yang dipilih, panel memberi tahu Anda dan meminta Anda memilih lebih sedikit. Panel memotong paragraf panjang menjadi bagian-bagian sekitar 1.500 huruf, di akhir kalimat jika memungkinkan, lalu menyatukannya kembali.

## Saat pertama kali menggunakan suatu bahasa

Terjemahan memerlukan sekumpulan berkas, disebut model, untuk setiap arah: satu untuk Inggris ke Jerman, satu lagi untuk Jerman ke Inggris. Setiap model berukuran sekitar 37 MB. Berkas-berkas tersebut bukan bagian dari JeffPub. Saat pertama kali Anda menerjemahkan dengan suatu bahasa, panel memberi tahu apa yang dibutuhkan dan ukurannya, misalnya "Unduh Jerman (74 MB)?". Tidak ada yang diunduh sebelum Anda mengklik tombol tersebut. Panel menampilkan sejauh mana unduhan berjalan, dan **Batal** menghentikannya serta tidak meninggalkan apa pun.

JeffPub memeriksa setiap berkas dengan sidik jari (checksum SHA-256) sebelum menggunakannya. Berkas yang tidak cocok akan dibuang, dan bahasa tersebut tidak dipasang.

Berkas-berkas disimpan dalam satu folder yang digunakan bersama oleh semua aplikasi JeffOffice (folder bernama JeffOffice di folder data pengguna Anda, dalam Translation/models), sehingga bahasa yang Anda unduh di satu aplikasi siap digunakan di aplikasi lain, dan hanya diunduh sekali. Dua bahasa yang tidak memiliki model sendiri, seperti Jerman dan Prancis, melalui bahasa Inggris, sehingga panel memerlukan kedua model.

## Saat suatu bahasa tidak memiliki model

Jika sepasang bahasa tidak memiliki model, panel memberi tahu Anda dan menawarkan **Buka di Peramban Anda Sebagai Gantinya**. Tombol itu membuka situs web terjemahan gratis, LibreTranslate (atau layanan yang alamatnya Anda pilih di pengaturannya), dengan teks Anda. Hanya dengan cara itu teks Anda meninggalkan komputer Anda, karena dikirim ke situs web tersebut, dan panel tidak lagi menyatakan bahwa teks tetap berada di komputer Anda. Tidak ada yang dikirim sebelum Anda mengklik tombol tersebut.

## Tentang kualitas

Model berukuran kecil, sehingga berjalan cepat di komputer biasa. Model menerjemahkan kalimat dengan baik dan label singkat kurang baik: satu kata atau nama tombol dapat diterjemahkan secara keliru. Bacalah terjemahan sebelum Anda menggunakannya. Nama, angka, dan tab tetap dipertahankan seperti semula.

Gambar, tab, dan pemisah baris di dalam paragraf dibiarkan apa adanya. Terjemahan mengisi kata-kata di antaranya.

## Terkait

- [Ejaan, tesaurus, riset, dan penyambungan kata](proofing)
