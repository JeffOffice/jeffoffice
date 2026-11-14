# Menerjemahkan teks
<!-- keywords: terjemahkan, terjemahan, penerjemah, bahasa, bahasa-bahasa, spanyol, prancis, jerman, italia, portugis, belanda, rusia, ukraina, indonesia, luring, di komputer ini, unduh, unduhan, model, ganti, sisipkan di bawah, lacak perubahan -->

**Tinjau > Bahasa > Terjemahkan** membuka panel **Terjemahkan** di sisi kanan. Panel ini menerjemahkan teks Anda di komputer Anda sendiri. Kata-kata Anda tidak dikirim ke mana pun, dan tidak diperlukan koneksi internet setelah berkas terjemahan untuk suatu bahasa tersimpan di komputer Anda.

## Menerjemahkan beberapa teks

1. Pilih teks yang ingin Anda terjemahkan. Jika tidak ada yang dipilih, panel menggunakan paragraf tempat kursor berada.
2. Klik **Terjemahkan** dalam grup **Bahasa**. Panel menampilkan teks Anda di bawah **Asli**.
3. Di bawah **Dari**, pilih bahasa teks Anda. JeffWrite memulai dengan bahasa yang ditandai pada teks. Di bawah **Ke**, pilih bahasa yang Anda inginkan. Tombol **Tukar** menukar kedua bahasa tersebut.
4. Klik **Terjemahkan**. Terjemahan muncul di bawah **Terjemahan**. Kata yang dicetak tebal, miring, atau bergaris bawah tetap tebal, miring, atau bergaris bawah, dan warna atau ukuran dipertahankan di tempat kata-kata tersebut sebelumnya memiliki tampilan yang sama.
5. Klik **Ganti** untuk menempatkan terjemahan di tempat teks Anda semula, atau **Sisipkan di Bawah** untuk menambahkannya sebagai paragraf baru di bawah teks Anda. Keduanya merupakan satu langkah, sehingga **Urungkan** dapat membatalkannya. Kata-kata hasil terjemahan diberi tanda bahasanya, sehingga pemeriksaan ejaan memeriksanya dalam bahasa tersebut.

Jika Anda memindahkan kursor atau mengubah teks setelah mengklik **Terjemahkan**, klik **Gunakan Pilihan** untuk membaca ulang teks, lalu klik **Terjemahkan** lagi. Panel tidak akan menimpa teks dengan terjemahan jika teks itu bukan lagi teks yang diterjemahkan.

Saat **Lacak Perubahan** aktif, **Ganti** dan **Sisipkan di Bawah** dicatat sebagai perubahan yang dilacak, seperti ketikan Anda, sehingga peninjau dapat melihat apa yang ada sebelumnya. Kata-kata hasil terjemahan kemudian mengambil tampilan yang akan dihasilkan oleh ketikan Anda. Kata-kata yang telah dihapus oleh perubahan yang dilacak dibiarkan apa adanya: tidak diterjemahkan, dan tidak disalin oleh **Sisipkan di Bawah**.

**Batal** menghentikan terjemahan yang memakan waktu terlalu lama, begitu pula menutup panel: jendela tidak pernah menunggunya. Teks panjang membutuhkan waktu lebih lama: satu halaman teks membutuhkan beberapa detik di kebanyakan komputer. Panel menerjemahkan paling banyak 1.000.000 huruf sekaligus (sekitar 200 halaman); jika lebih banyak yang dipilih, panel memberi tahu Anda dan meminta Anda memilih lebih sedikit. Panel memotong paragraf panjang menjadi bagian-bagian sekitar 1.500 huruf, di akhir kalimat jika memungkinkan, lalu menyatukannya kembali.

## Saat pertama kali menggunakan suatu bahasa

Terjemahan memerlukan sekumpulan berkas, disebut model, untuk setiap arah: satu untuk Inggris ke Jerman, satu lagi untuk Jerman ke Inggris. Setiap model berukuran sekitar 37 MB. Berkas-berkas tersebut bukan bagian dari JeffWrite. Pertama kali Anda menerjemahkan dengan suatu bahasa, panel memberi tahu apa yang dibutuhkan dan ukurannya, misalnya "Unduh Jerman (74 MB)?". Tidak ada yang diunduh sebelum Anda mengklik tombol itu. Panel menampilkan sejauh mana unduhan telah berjalan, dan **Batal** menghentikannya tanpa meninggalkan sisa apa pun.

JeffWrite memeriksa setiap berkas dengan sidik jari (checksum SHA-256) sebelum menggunakannya. Berkas yang tidak cocok dibuang, dan bahasa tersebut tidak dipasang.

Berkas-berkas disimpan dalam satu folder yang dipakai bersama oleh semua aplikasi JeffOffice (folder bernama JeffOffice di folder data pengguna Anda, di Translation/models), sehingga bahasa yang Anda unduh di satu aplikasi siap digunakan di aplikasi lain, dan hanya diunduh sekali. Dua bahasa yang tidak memiliki model sendiri, seperti Jerman dan Prancis, diterjemahkan melalui Inggris, sehingga panel memerlukan kedua model.

## Ketika suatu bahasa tidak memiliki model

Jika pasangan bahasa tidak memiliki model, panel memberi tahu hal itu dan menawarkan **Buka di Peramban Anda Sebagai Gantinya**. Tombol itu membuka situs web terjemahan gratis, LibreTranslate (atau layanan yang alamatnya Anda pilih di pengaturan untuknya), dengan teks Anda. Baru pada saat itulah teks Anda meninggalkan komputer Anda, karena dikirim ke situs web tersebut, dan panel tidak lagi menyatakan bahwa teks tetap berada di komputer Anda. Tidak ada yang dikirim sebelum Anda mengklik tombol itu.

## Tentang kualitas

Model berukuran kecil, sehingga berjalan cepat di komputer biasa. Model menerjemahkan kalimat dengan baik dan label singkat kurang baik: satu kata atau nama tombol dapat menghasilkan terjemahan yang keliru. Baca terjemahan sebelum Anda menggunakannya. Nama, angka, dan tab tetap dibiarkan sebagaimana adanya.

Gambar, tab, dan pemisah baris di dalam paragraf dibiarkan apa adanya. Terjemahan mengisi kata-kata di antaranya.

## Terkait

- [Menghitung kata, mengatur bahasa, memeriksa aksesibilitas, dan mencari kata](proofing-tools)
