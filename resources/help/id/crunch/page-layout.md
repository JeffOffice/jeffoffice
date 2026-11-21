# Mengatur halaman

<!-- keywords: tema, tema, warna tema, font tema, warna, font, aksen, font judul, font isi, latar belakang, gambar di belakang sel, ubin, kertas dinding, tata letak halaman, margin, orientasi, potret, lanskap, kertas, ukuran, area cetak, pemisah halaman, pemisah, judul cetak, judul, ulangi baris, skala agar pas, muat ke halaman, garis kisi, judul baris dan kolom, opsi lembar, kanan ke kiri, cermin -->

Tab **Tata Letak Halaman** menentukan cara lembar dipotong menjadi halaman saat dicetak atau dibuat menjadi PDF. Setiap lembar menyimpan pilihannya sendiri, dan pilihan itu disimpan di dalam buku kerja. Setiap perubahan adalah satu langkah yang dapat Anda urungkan.

## Tema, warna, dan font

Buku kerja memiliki **tema**: dua belas warna dan dua font (satu untuk judul dan satu untuk teks isi) yang dapat dirujuk oleh sel, gaya sel, gaya tabel, bentuk, dan bagan alih-alih menyalin nilainya. Saat Anda mengubah tema, semua yang merujuknya ikut berubah: sel yang diisi "Aksen 1, lebih terang 40%" akan berubah menjadi Aksen 1 yang baru, lebih terang dengan jumlah yang sama.

- **Tema** memuat tema siap pakai yang sekaligus mengatur warna dan font. **Telusuri Tema** mengambil tema dari berkas tema, buku kerja, dokumen, atau presentasi. **Simpan Tema Saat Ini** menyimpan tema buku kerja ini ke dalam berkas, dan **Atur Ulang ke Tema Buku Kerja** mengembalikan tema yang dimiliki buku kerja saat Anda membukanya.
- **Warna** hanya mengubah dua belas warna. **Buat Warna Tema Baru** memungkinkan Anda menentukan dan menamai warna sendiri; setiap aplikasi JeffOffice kemudian akan mencantumkannya.
- **Font** hanya mengubah font judul dan font isi. Sel yang memakai font tema akan mengikutinya, sedangkan sel yang diatur dengan font berdasarkan nama tetap memakai font tersebut.

Setiap pilihan adalah satu langkah yang dapat Anda urungkan. Tombol warna (Warna Isian, Warna Font, Bingkai, dan Warna Tab) menampilkan warna tema dalam satu baris, dengan versi lebih terang dan lebih gelap di bawahnya. Warna yang Anda pilih dari sana tetap menjadi warna tema, sehingga ikut berubah bersama tema. Warna dari **Warna Standar** atau **Warna Lainnya** tetap seperti yang Anda pilih.

Buku kerja baru dimulai dengan tema milik JeffOffice. Bentuk atau bagan yang Anda buat di sini tetap memakai warna yang diberikan kepadanya; bentuk yang berasal dari berkas dan merujuk warna tema akan mengikuti tema. Buku kerja yang dibuka dari berkas biner lama atau berkas OpenDocument tidak memiliki tema untuk diubah, sehingga tombol-tombol ini tidak menawarkan apa pun untuknya. Saat Anda menyimpan buku kerja sebagai berkas OpenDocument, yang tidak memiliki tema, warnanya ditulis sebagaimana tampilannya.

## Margin, orientasi, dan ukuran

- **Margin** mengatur ruang kosong di sekeliling sel. **Standar**, **Lebar**, dan **Sempit** adalah kumpulan siap pakai. **Margin Kustom** membuka kotak dialog Pengaturan Halaman, tempat Anda dapat mengetik setiap margin dan menentukan jarak header dan footer dari tepi kertas.
- **Orientasi** adalah **Potret** (kertas lebih tinggi daripada lebarnya) atau **Lanskap** (lebih lebar daripada tingginya).
- **Ukuran** adalah kertasnya: A4, Letter, Legal, dan lainnya. **Ukuran Kertas Lainnya** mencantumkan semuanya.

## Area cetak

Secara default, lembar dicetak dengan semua yang berisi sesuatu: nilai, rumus, dan sel dengan bingkai atau isian yang terlihat, serta bagan, gambar, atau bentuk apa pun. Untuk mencetak sebagian saja:

1. Pilih sel yang Anda inginkan.
2. Pilih **Area Cetak > Atur Area Cetak**.

**Tambahkan Pilihan ke Area Cetak** menambahkan blok sel kedua ke area cetak. Setiap blok dicetak pada halaman tersendiri. **Hapus Area Cetak** kembali mencetak semuanya. Baris atau kolom yang Anda sisipkan atau hapus di dalam area cetak akan mengubah area tersebut bersama sel-selnya.

## Pemisah halaman

Tempat satu halaman berakhir dan halaman berikutnya dimulai dihitung dari kertas, margin, dan ukuran baris serta kolom Anda: sebuah baris atau kolom tidak pernah dipotong menjadi dua. Untuk mengakhiri halaman lebih awal, pilih sel di bawah dan di kanan tempat pemisah seharusnya berada, lalu pilih **Pemisah > Sisipkan Pemisah Halaman**. Jika seluruh baris dipilih, pemisah diletakkan di atasnya, dan jika seluruh kolom dipilih, pemisah diletakkan di sisi kirinya. **Hapus Pemisah Halaman** menghilangkan pemisah di atas atau di kiri sel yang dipilih, dan **Atur Ulang Semua Pemisah Halaman** menghilangkan semua pemisah yang Anda buat. Pemisah halaman yang Anda buat diabaikan selama lembar sedang diperkecil agar pas.

## Tampilan Pemisah Halaman

**Tampilan > Pemisah Halaman** (atau tombol di pojok kanan bawah jendela) menampilkan lembar yang sudah dipotong menjadi halaman tempat lembar itu dicetak. Sel yang dicetak tetap seperti biasa, bagian lainnya diberi naungan, dan setiap halaman memiliki nomornya di atas sel-selnya. Garis putus-putus adalah pemisah yang dihitung JeffCrunch, dan garis utuh adalah pemisah yang Anda buat.

- **Pindahkan pemisah.** Seret garis ke baris atau kolom lain. Garis itu menjadi pemisah yang Anda buat di tempat tersebut, dan halaman sesudahnya dipotong ulang.
- **Hapus pemisah.** Seret garis keluar dari sel yang dicetak, ke nomor baris atau huruf kolom.
- **Ubah area cetak.** Seret tepi biru dari sel yang dicetak. Jika Anda belum mengatur area cetak, sel-sel yang dicetak dengan sendirinya akan menjadi satu area, dengan tepi itu dipindahkan.

Setiap seretan adalah satu langkah Urungkan. Selama lembar sedang diperkecil agar pas, pemisahnya diabaikan, sehingga garis tidak dapat diseret. **Normal** mengembalikan sel seperti semula. Tampilan yang terakhir Anda gunakan untuk setiap lembar disimpan bersama buku kerja.

## Tampilan Tata Letak Halaman

**Tampilan > Tata Letak Halaman** (atau tombol tengah di pojok kanan bawah jendela) menampilkan lembar sebagai halaman tempat lembar itu dicetak, berjejer dari atas ke bawah, masing-masing seperti selembar kertas dengan margin dan jarak antarhalaman. Penggaris berjalan di sepanjang bagian atas dan di sisi kiri. Header dan footer digambar di tempat keduanya akan dicetak, begitu juga gambar di dalamnya.

- **Bekerja di dalam sel.** Klik sel untuk memilihnya, seret untuk memilih beberapa sel, dan gunakan tombol panah, mengetik, F2, serta klik ganda seperti di tampilan Normal. Apa yang Anda ketik masuk ke dalam strip di atas sel pada halamannya.
- **Mulai dari tab Sisipkan.** **Sisipkan > Header dan Footer** menampilkan halaman (jika belum ditampilkan) dan membuka bagian tengah header halaman pertama, siap untuk diketik. Klik bagian lain dari header atau footer untuk mengetik di sana.
- **Mengetik header atau footer.** Arahkan penunjuk ke margin atas atau bawah sebuah halaman: bagian kiri, tengah, dan kanannya akan menyala. Klik salah satunya dan ketik. Tab **Header dan Footer** akan muncul, dengan tombol untuk menyisipkan nomor halaman, jumlah halaman, tanggal, waktu, folder dan nama berkas, nama lembar, serta gambar. Pilihannya memberikan halaman pertama, serta halaman ganjil dan genap, header dan footer tersendiri, dan menentukan apakah header ikut membesar bersama sel dan sejajar dengan margin. **Header** dan **Footer** pada tab itu adalah galeri bentuk siap pakai, seperti **Halaman 1**, **Halaman 1 dari ?**, nama lembar, nama berkas, dan tanggal. Memilih salah satunya menggantikan seluruh header atau footer pada halaman yang sedang Anda ketik (kata-kata yang masih Anda ketik di dalamnya tidak disimpan). **(tidak ada)** mengosongkannya. **Buka Header** dan **Buka Footer** membuka bagian tengah margin tersebut untuk diketik. Tekan **Ctrl+Enter**, atau klik di tempat lain, untuk menyelesaikannya; **Esc** membatalkannya. Setiap header atau footer yang selesai adalah satu langkah Urungkan.
- **Perbesar atau perkecil** dengan penggeser, atau tahan **Ctrl** lalu putar roda mouse.

Hanya halaman yang terlihat yang digambar, sehingga lembar dengan ribuan halaman dapat dibuka sekaligus. Tampilan yang terakhir Anda gunakan untuk setiap lembar disimpan bersama buku kerja.

## Judul cetak

**Judul Cetak** mengulang baris di bagian atas, dan kolom di sisi kiri, pada setiap halaman setelah halaman tempat baris atau kolom itu berada, sehingga baris judul ada di setiap halaman. Ketik baris (misalnya **1:3**) dan kolom (misalnya **A:B**) pada tab Lembar di kotak dialog Pengaturan Halaman.

## Skala agar pas

**Lebar** dan **Tinggi** memperkecil sel hingga lembar tidak lebih lebar, atau tidak lebih tinggi, daripada jumlah halaman tersebut. **Otomatis** membiarkan arah tersebut memakai halaman sebanyak yang dibutuhkan. **Skala** mencetak dengan persentase dari ukuran penuh, dari 10 sampai 400, dan bekerja selama Lebar dan Tinggi sama-sama **Otomatis**. Jika memperkecil agar pas akan membuat huruf sangat kecil, layar Cetak akan memberi tahu Anda.

## Opsi lembar

**Tampilkan Garis Kisi** dan **Tampilkan Judul** hanya mengubah layar. **Cetak Garis Kisi** mencetak garis di antara sel pada kertas, dan **Cetak Judul** mencetak nomor baris dan huruf kolom pada kertas.

**Lembar Kanan-ke-Kiri** membuat lembar dibaca dari kanan. Kolom A pindah ke tepi kanan, dan kolom-kolom dihitung ke arah kiri. Nomor baris juga pindah ke tepi kanan. Gunakan ini untuk bahasa yang ditulis dari kanan ke kiri, seperti bahasa Arab dan Ibrani. Pilihan ini hanya mengubah tampilan lembar di layar, dan disimpan bersama lembar, sehingga kembali saat Anda membuka buku kerja. Ini bukan langkah yang dapat diurungkan: klik lagi untuk mematikannya.

## Latar belakang

**Latar Belakang** (dalam grup **Pengaturan Halaman**) menempatkan gambar di belakang sel lembar. Pilih gambar PNG, JPEG, GIF, atau BMP. Gambar diulang seperti ubin untuk mengisi lembar, dan ikut bergerak saat Anda menggulir. Sel yang memiliki warna isian akan menutupi gambar, jadi berikan isian pada sel yang ingin mudah Anda baca. Gambar hanya tampil di layar: gambar tidak dicetak dan tidak ikut masuk ke PDF.

**Hapus Latar Belakang** menghilangkan gambar. Keduanya adalah langkah yang dapat Anda urungkan. Gambar disimpan bersama lembar dan kembali saat Anda membuka buku kerja. Pada salinan .ods, gambar disimpan sebagai gambar latar halaman, yang mungkin dicetak oleh program lain. Berkas gambar yang lebih besar dari 8 MB, atau yang memiliki terlalu banyak titik, ditolak, begitu pula gambar yang akan membuat semua gambar buku kerja (latar belakang, header, dan footer) melebihi 64 MB jika digabungkan. Gambar dengan lebih dari 2.400 titik pada satu sisinya tetap diulang sebagai ubin dengan ukuran aslinya.

## Pilihan lainnya

Panah kecil di pojok grup **Pengaturan Halaman** membuka kotak dialog Pengaturan Halaman, yang memuat semua ini dan lebih banyak lagi: [Kotak dialog Pengaturan Halaman](page-setup).
