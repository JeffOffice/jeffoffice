# Terjemahkan teks
<!-- keywords: terjemahkan, terjemahan, penerjemah, bahasa, bahasa-bahasa, spanyol, prancis, jerman, italia, portugis, belanda, rusia, ukraina, indonesia, luring, di komputer ini, unduh, model, ganti, sisipkan di bawah, offline -->

**Tinjau > Bahasa > Terjemahkan** membuka panel **Terjemahkan**. Panel ini menerjemahkan teks Anda di komputer Anda sendiri. Kata-kata Anda tidak dikirim ke mana pun, dan tidak diperlukan koneksi internet setelah berkas terjemahan untuk suatu bahasa tersedia di komputer Anda.

## Menerjemahkan beberapa teks

1. Pilih teks yang ingin diterjemahkan. Jika tidak ada yang dipilih, panel memakai paragraf tempat kursor berada. Dalam kotak teks yang dipilih tetapi tidak sedang diedit, panel memakai seluruh teks dalam kotak tersebut.
2. Klik **Tinjau > Bahasa > Terjemahkan**. Panel menampilkan teks Anda di bawah **Asli**.
3. Di bawah **Dari**, pilih bahasa teks Anda. Di bawah **Ke**, pilih bahasa yang Anda inginkan. Tombol **Tukar** menukar kedua bahasa tersebut.
4. Klik **Terjemahkan**. Terjemahan muncul di bawah **Terjemahan**. Kata yang dicetak tebal, miring, dan bergaris bawah tetap tebal, miring, dan bergaris bawah.
5. Klik **Ganti** untuk menempatkan terjemahan di tempat teks Anda semula, atau **Sisipkan di Bawah** untuk menambahkannya dalam paragraf baru di bawah teks Anda. Keduanya merupakan satu langkah, sehingga **Urungkan** dapat membatalkannya.

Jika Anda memindahkan kursor atau mengubah teks setelah mengklik **Terjemahkan**, klik **Gunakan Pilihan** untuk membaca teks lagi, lalu klik **Terjemahkan** sekali lagi. Panel tidak akan menimpa teks dengan terjemahan jika teks itu bukan lagi teks yang diterjemahkan.

**Batal** menghentikan terjemahan yang berlangsung terlalu lama, dan menutup panel juga menghentikannya: jendela tidak pernah menunggu terjemahan. Teks yang panjang membutuhkan waktu lebih lama: satu halaman teks membutuhkan beberapa detik di sebagian besar komputer. Panel menerjemahkan paling banyak 1.000.000 huruf sekaligus (sekitar 200 halaman); jika lebih banyak yang dipilih, panel akan memberi tahu Anda dan meminta Anda memilih lebih sedikit. Paragraf panjang dipecah menjadi potongan sekitar 1.500 huruf, sedapat mungkin di akhir kalimat, lalu digabungkan kembali.

## Saat pertama kali menggunakan suatu bahasa

Terjemahan memerlukan sekumpulan berkas, disebut model, untuk setiap arah: satu untuk Inggris ke Jerman, satu lagi untuk Jerman ke Inggris. Setiap model berukuran sekitar 37 MB. Berkas-berkas tersebut bukan bagian dari JeffPub. Saat pertama kali Anda menerjemahkan dengan suatu bahasa, panel memberi tahu apa yang dibutuhkan dan berapa ukurannya, misalnya "Unduh Jerman (74 MB)?". Tidak ada yang diunduh sebelum Anda mengklik tombol tersebut. Panel menampilkan sejauh mana unduhan telah berjalan, dan **Batal** menghentikannya tanpa meninggalkan apa pun.

JeffPub memeriksa setiap berkas dengan sidik jari (checksum SHA-256) sebelum menggunakannya. Berkas yang tidak cocok akan dibuang, dan bahasa tersebut tidak dipasang.

Berkas-berkas disimpan dalam satu folder yang digunakan bersama oleh semua aplikasi JeffOffice (folder bernama JeffOffice di folder data pengguna Anda, di Translation/models), sehingga bahasa yang Anda unduh di satu aplikasi juga siap digunakan di aplikasi lain, dan hanya diunduh sekali. Dua bahasa yang tidak memiliki model sendiri, misalnya Jerman dan Prancis, diterjemahkan melalui bahasa Inggris, sehingga panel memerlukan kedua model tersebut.

## Saat suatu bahasa tidak memiliki model

Jika pasangan bahasa tidak memiliki model, panel memberi tahu hal itu dan menawarkan **Buka di Peramban Sebagai Gantinya**. Tindakan itu membuka situs web terjemahan gratis, LibreTranslate (atau layanan yang alamatnya Anda pilih dalam pengaturan untuk itu), dengan teks Anda. Baru pada saat itulah teks Anda meninggalkan komputer Anda, karena teks dikirim ke situs web tersebut, dan panel tidak lagi menyatakan bahwa teks tetap berada di komputer Anda. Tidak ada yang dikirim sebelum Anda mengklik tombol tersebut.

## Tentang kualitas

Model berukuran kecil, sehingga berjalan cepat di komputer biasa. Model menerjemahkan kalimat dengan baik dan label pendek dengan kurang baik: satu kata atau nama tombol bisa menghasilkan terjemahan yang keliru. Baca terjemahan sebelum Anda menggunakannya. Nama, angka, dan tab tetap seperti semula.

Gambar, tab, dan pemisah baris di dalam paragraf dibiarkan apa adanya. Terjemahan hanya mengisi kata-kata di antaranya.

## Terkait

- [Pemeriksaan ejaan, tesaurus, riset, dan penyambungan kata](proofing)
