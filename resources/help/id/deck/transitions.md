# Transisi antar slaid
<!-- keywords: transisi, transition, morph, fade, pudar, push, dorong, wipe, sapu, split, belah, reveal, ungkap, cover, tutup, uncover, buka, zoom, potong, cut, opsi efek, objek, kata, karakter, pasangkan, nama, arah, durasi, lanjut slaid, saat klik mouse, setelah, otomatis, pengaturan waktu, terapkan ke semua, pratinjau, bintang -->

**Transisi** adalah efek yang diputar saat presentasi slaid berpindah dari satu slaid ke slaid berikutnya. Transisi menjadi milik slaid yang masuk. Presentasi slaid memutarnya saat bergerak maju ke slaid tersebut.

## Menambahkan transisi

1. Klik slaid di panel slaid.
2. Klik tab **Transisi**. Di galeri **Transisi ke Slaid Ini**, klik sebuah efek: **Tidak Ada**, **Morph**, **Fade**, **Push**, **Wipe**, **Split**, **Reveal**, **Cover**, **Uncover**, **Zoom**, atau **Cut**. Gambar slaid di panel slaid akan mendapatkan bintang kecil sebagai tanda bahwa slaid tersebut memiliki transisi.
3. Klik **Transisi > Pratinjau > Pratinjau** untuk melihat transisi diputar ke slaid, di atas area slaid. Tekan Esc untuk menghentikannya.

Pilih **Tidak Ada** untuk menghapus transisi.

## Mengatur cara transisi berjalan

Klik **Opsi Efek** untuk pilihan efek tersebut:

- **Push**, **Wipe**, **Reveal**, **Cover**, dan **Uncover** memiliki arah: **Dari Kanan**, **Dari Kiri**, **Dari Bawah**, atau **Dari Atas**.
- **Split** dan **Zoom** berjalan **Keluar** atau **Masuk**.
- **Morph** memindahkan **Objek**, **Kata**, atau **Karakter**. Lihat di bawah.
- **Fade** dan **Cut** tidak memiliki pilihan. Potongan langsung melewati layar hitam.

## Morph

**Morph** memindahkan hal-hal yang dimiliki bersama oleh dua slaid. **Objek** adalah satu benda pada slaid: bentuk, gambar, kotak teks, garis, atau tabel. Presentasi berpindah dari satu slaid ke slaid berikutnya. Setiap objek yang ada di kedua slaid meluncur dari posisinya di slaid pertama ke posisinya di slaid kedua. Sepanjang jalan, objek dapat membesar atau mengecil, berputar, dan berubah warna. Objek yang hanya ada di slaid pertama memudar. Objek yang hanya ada di slaid kedua muncul secara bertahap.

Untuk membuat morph:

1. Klik kanan slaid di panel slaid dan pilih **Duplikasi Slaid**. Salinan memiliki objek yang sama.
2. Pada salinan, pindahkan objek, perbesar, putar, atau ubah warnanya. Anda juga dapat mengubah teksnya, atau menambah dan menghapus objek.
3. Klik salinan, lalu klik **Transisi > Transisi ke Slaid Ini > Morph**. Morph berlangsung 2 detik. Anda dapat mengubahnya dengan **Durasi**.
4. Klik **Pratinjau** untuk menontonnya, atau mulai presentasi slaid.

### Objek mana yang dipasangkan

JeffDeck memasangkan objek di slaid pertama dengan objek di slaid kedua. Pemasangan dicoba dengan cara berikut, sesuai urutan ini. Setiap objek hanya masuk paling banyak satu pasangan.

1. **Nama yang diawali !!.** Dua objek dengan nama yang sama, dan nama tersebut diawali dua tanda seru, selalu menjadi pasangan. Keduanya boleh objek apa pun, bahkan lingkaran dan bintang.
2. **Nama yang sama.** Dua objek dengan jenis yang sama dan nama yang sama menjadi pasangan. Kotak teks dan bentuk merupakan jenis yang berbeda.
3. **Teks yang sama.** Dua kotak teks dengan kata yang sama menjadi pasangan, apa pun namanya. Begitu pula dua bentuk dengan kata yang sama.
4. **Gambar yang sama.** Dua gambar dari citra yang sama menjadi pasangan, meskipun namanya atau ukurannya berbeda.
5. **Jenis bentuk yang sama.** Dua lingkaran menjadi pasangan. Begitu pula dua garis, atau dua tabel.

Jika beberapa objek dapat menjadi pasangan, objek yang paling dekat satu sama lain dipasangkan lebih dulu.

Nama objek berasal dari berkas presentasi. Presentasi yang dibuat di tempat lain mempertahankan nama yang diberikan pada objeknya, dan JeffDeck menggunakannya.

### Cara sepasang objek bergerak

- Dua objek dengan bentuk yang sama menjadi satu objek yang bergerak. Posisi, ukuran, putaran, warna isian, warna garis, dan tingkat transparansinya berubah sedikit demi sedikit. Pemotongan gambar juga ikut berubah.
- Dua objek yang bentuknya tidak sama, seperti lingkaran dan persegi, bergerak ke tempat yang sama sementara yang satu memudar menjadi yang lain.
- Jika teks sepasang objek tidak sama, teks lama memudar menjadi teks baru.
- Objek yang disembunyikan oleh animasi tidak diikutsertakan. Objek yang dipindahkan oleh animasi diambil dari posisi akhirnya.

### Yang bergerak: Objek, Kata, atau Karakter

Klik **Opsi Efek** untuk memilih:

- **Objek.** Teks sepasang objek tetap berada di dalam objeknya. Teks lama memudar menjadi teks baru.
- **Kata.** Kata yang ada di kedua teks bergerak dari posisi lama ke posisi baru. Kata lainnya memudar keluar dan masuk. Tanda baca seperti koma dihitung sebagai kata tersendiri.
- **Karakter.** Sama seperti kata, tetapi satu huruf atau angka sekaligus.

**Kata** dan **Karakter** berlaku untuk kotak teks dan bentuk. Teks yang lebih dari 16.000 karakter, teks yang naik turun, dan teks dalam bentuk yang dibalik diputar sebagai **Objek**. Demikian pula pasangan yang tidak memiliki kata atau karakter yang sama.

Morph juga merupakan transisi di program presentasi lain (versi 2019 dan yang lebih baru). JeffDeck membacanya dan menyimpannya dengan cara yang sama. Program yang lebih lama memutar morph sebagai pudar.

## Mengatur waktu

Grup **Pengaturan Waktu** mengatur berapa lama efek berlangsung dan cara presentasi meninggalkan slaid.

- **Durasi** adalah berapa detik efek berlangsung. Transisi baru berlangsung 0,7 detik. Anda dapat mengetik dari 0,01 sampai 59.
- **Saat Klik Mouse** aktif secara default. Presentasi berpindah dari slaid ini ketika presenter mengklik. Matikan untuk berpindah dengan tombol seperti Spasi atau Kanan. Animasi slaid tetap diputar saat diklik.
- **Setelah** membuat presentasi berpindah sendiri setelah jumlah detik yang Anda ketik. Anda dapat menggunakan **Setelah** bersama **Saat Klik Mouse**: presentasi berpindah pada mana pun yang terjadi lebih dulu.

**Terapkan ke Semua** memberikan transisi dan pengaturan waktu dari slaid yang sedang Anda buka ke setiap slaid.

Masing-masing langkah ini dapat dibatalkan dengan Ctrl+Z.

## Transisi dari berkas lain

Presentasi yang dibuat di tempat lain dapat memiliki transisi yang tidak memiliki efek di JeffDeck. Transisi tersebut diputar sebagai pudar, dan disimpan apa adanya, sehingga tetap berfungsi di tempat asalnya.

## Terkait

- [Tab Transisi](tab-transitions)
- [Animasi](animations)
- [Memutar presentasi slaid dan menggunakan Tampilan Presenter](show)
