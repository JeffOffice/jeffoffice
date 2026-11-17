# Membuka dan menyimpan publikasi
<!-- keywords: buka, simpan, simpan sebagai, simpan salinan, tutup, berkas terbaru, disematkan, jenis berkas, jpub, ganti nama, cadangan, properti, info, tandai sebagai final, periksa, riwayat versi, nama baru, salin, Ctrl+S, hanya baca, baca saja, buka sebagai salinan, jendela baru, templat saya, simpan sebagai templat, perbaiki, buka dan perbaiki, rusak, periksa kompatibilitas, tetap simpan, jangan tanya lagi -->

JeffPub menyimpan pekerjaan Anda dalam berkas .jpub. Program ini juga dapat membuka dan menyimpan berkas .pub. Lihat [Bekerja dengan berkas .pub](pub-files).

## Simpan

Tekan Ctrl+S, klik tombol **Simpan** pada bilah alat cepat, atau klik **Berkas > Simpan**. Untuk pertama kali, JeffPub meminta nama dan lokasi. Setelah itu, Simpan menggantikan berkas dengan pekerjaan terbaru Anda.

Tanda bintang (*) setelah nama di bilah judul berarti ada perubahan yang belum disimpan. Simpanlah secara berkala. Jika JeffPub berhenti secara tiba-tiba, lihat [Memulihkan pekerjaan yang belum disimpan](recover).

Untuk menyimpan salinan terakhir yang tersimpan dari suatu berkas, aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffPub lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi bertanya apakah akan menyimpan tanpa cadangan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula. Opsi ini terdapat di **Berkas > Opsi**, pada tab **Umum**. Setiap aplikasi JeffOffice memilikinya.

## Simpan dengan nama baru

Klik **Berkas > Simpan Sebagai**, atau tekan F12. Ketik nama, pilih **Simpan sebagai jenis**, lalu klik **Simpan**. Pilihannya adalah:

- **Publikasi JeffPub (*.jpub)** adalah pilihan yang biasa.
- **Berkas Publikasi .pub (*.pub)** membuat berkas .pub.
- **PDF (*.pdf)** membuat PDF dan membiarkan publikasi Anda tetap seperti semula.
- **Templat JeffPub (*.jpub)** menambahkan publikasi ke **Templat Saya**.

Setelah menyimpan sebagai .jpub atau .pub, bilah judul menampilkan nama baru, dan Anda tetap bekerja di berkas baru.

## Simpan salinan

Klik **Berkas > Simpan Salinan** lalu pilih nama. JeffPub menulis publikasi ke berkas tersebut, dan Anda tetap bekerja di berkas yang sebelumnya. Nama dan perubahan yang belum disimpan pada berkas asal tetap seperti semula.

## Buka

1. Tekan Ctrl+O, atau klik **Berkas > Buka**.
2. Klik **Telusuri** untuk memilih berkas. Atau klik nama di bawah **Disematkan** atau **Publikasi Terbaru**.

Anda juga dapat menyeret berkas .jpub atau .pub ke halaman. Jika publikasi saat ini memiliki perubahan yang belum disimpan, JeffPub menanyakannya lebih dulu.

Klik kanan nama dalam daftar untuk **Sematkan ke daftar** (nama tetap di bagian atas) atau **Hapus dari daftar**. Nama yang berwarna abu-abu berarti JeffPub tidak dapat menemukan berkas itu sekarang. Untuk mengubah jumlah nama terbaru yang ditampilkan, lihat [Mengubah pengaturan JeffPub](options).

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lebih Banyak Cara untuk Membuka**. Anda juga dapat mengklik kanan nama dalam daftar Terbaru.

- **Buka sebagai Baca-Saja** membuka publikasi hanya untuk dibaca. Anda dapat melihat dan menggulir, tetapi tidak dapat mengubah apa pun. Bilah judul menampilkan "Baca-Saja". Jika Anda menekan **Simpan**, JeffPub meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka publikasi tanpa berkas miliknya sendiri. **Simpan** akan menanyakan lokasi penyimpanannya.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri.
- **Buka dan Perbaiki** untuk berkas .jpub yang rusak, misalnya berkas yang terpotong saat disalin. JeffPub membaca setiap bagian berkas yang dapat dibaca, dan membuang bagian yang tidak dapat dibaca. Publikasi dibuka sebagai salinan bernama "Diperbaiki" ditambah nama berkas, dan sebuah pesan memberi tahu bagian apa yang dibuang. Berkas yang rusak tidak diubah.

## Tutup

Klik **Berkas > Tutup**, atau tekan Ctrl+F4. JeffPub menanyakan apakah perubahan akan disimpan, lalu menampilkan halaman kosong.

## Info: properti, Tandai sebagai Final, Periksa, dan versi

**Berkas > Info** menampilkan lokasi berkas, dengan **Tampilkan di Folder** dan **Salin Jalur**. Halaman ini menampilkan fakta tentang publikasi Anda: jumlah halaman, ukuran halaman, gambar, kata, dan tanggal. Ketik pada kotak untuk mengatur **Judul**, **Pengarang**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**, lalu klik **Properti Kustom** untuk properti buatan Anda sendiri. Perubahan disimpan bersama publikasi, dan **Ctrl+Z** membatalkannya. Halaman ini juga membuka informasi bisnis Anda, **Pengaturan Cetak Komersial** (lihat [Berkas PDF dan percetakan](pdf)), dan [Pemeriksa Desain](design-checker).

- **Periksa Kompatibilitas** mencantumkan apa saja dari publikasi Anda yang tidak dapat disimpan dalam berkas .pub, tanpa menyimpan apa pun: objek tersembunyi, beberapa tampilan daftar, dan apa pun lainnya yang tidak ditulis oleh penulis .pub. Saat Anda menyimpan sebagai berkas .pub, JeffPub menampilkan daftar yang sama **sebelum** menulis apa pun. Pilih **Tetap Simpan**, **Simpan sebagai .jpub** (berkas .jpub dibuat di sebelahnya, dan tidak ada yang hilang), atau **Batal**. Centang **Jangan tanyakan lagi untuk berkas ini**, dan JeffPub tidak akan bertanya lagi tentang berkas itu sampai Anda menutup program.
- **Selalu Dibuka sebagai Baca-Saja** membuat publikasi selalu terbuka hanya untuk dibaca setiap kali dibuka. Pilihan ini disimpan bersama berkas .jpub, dan bilah status menampilkannya saat seseorang membukanya. Untuk mengeditnya, pilih **Selalu Dibuka sebagai Baca-Saja** pada halaman yang sama untuk mematikannya. Ini hanya pengingat dan bukan kunci. Berkas .pub tidak dapat menyimpannya (penulis .pub tidak menyimpan properti semacam ini, dan JeffPub tidak membacanya dari berkas .pub), jadi simpan dulu sebagai berkas .jpub. Periksa Kompatibilitas akan mencantumkannya untuk berkas .pub.
- **Tandai sebagai Final** menyimpan publikasi lalu menahannya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa berkas hanya-baca, dan sebuah catatan memberi tahu bahwa berkas itu telah ditandai sebagai final. Ini bukan kunci: pilih **Ditandai sebagai Final** lagi untuk mengeditnya. Berkas .pub tidak dapat menyimpan tanda ini, jadi simpan dulu sebagai berkas .jpub.
- **Periksa Publikasi** mencantumkan apa saja yang ada dalam berkas dan mungkin tidak ingin Anda bagikan, lalu menghapusnya: komentar dan nama orang yang menulisnya, pengarang dan nama lain dalam properti, informasi bisnis, daftar gabung surat, serta objek yang disimpan di area coret-coret di samping halaman.
- **Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, menghapus pengarang, orang yang terakhir menyimpan berkas, perusahaan, dan manajer dari berkas setiap kali Anda menyimpan, baik untuk berkas .jpub maupun .pub. Kotak ini sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.
- **Riwayat Versi** menyimpan salinan bernama dari publikasi di komputer Anda. Simpan publikasi terlebih dahulu. Klik **Simpan Versi…**, lalu nanti pilih salah satu untuk **Buka Salinan** darinya, **Kembali ke Versi Ini** (publikasi yang ada saat ini disimpan sebagai versi lebih dulu), atau **Hapus**.

Berkas .jpub tidak memiliki format untuk kata sandi, sehingga JeffPub tidak memiliki kartu kata sandi. JeffPub belum memiliki pemeriksaan aksesibilitas, sehingga halaman ini tidak memiliki kartu untuk itu.

## Jika Windows tidak mengizinkan JeffPub menyimpan

Keamanan Windows dapat memblokir program menyimpan di folder seperti Dokumen. JeffPub kemudian menampilkan pesan berisi langkah-langkah untuk mengizinkannya. Atau simpan di folder lain.

## Terkait

- [Memulihkan pekerjaan yang belum disimpan](recover)
- [Bekerja dengan berkas .pub](pub-files)
- [Membuat publikasi baru](create)
- [Mengubah pengaturan JeffPub](options)
