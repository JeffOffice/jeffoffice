# Membuka dan menyimpan publikasi
<!-- keywords: buka, simpan, simpan sebagai, simpan salinan, tutup, berkas terbaru, disematkan, jenis berkas, jpub, ganti nama, cadangan, properti, info, tandai sebagai final, periksa, riwayat versi, nama baru, salin, Ctrl+S, hanya-baca, read only, buka sebagai salinan, jendela baru, templat saya, simpan sebagai templat, perbaiki, buka dan perbaiki, rusak, periksa kompatibilitas, tetap simpan, jangan tanya lagi -->

JeffPub menyimpan pekerjaan Anda dalam berkas .jpub. Program ini juga dapat membuka dan menyimpan berkas .pub. Lihat [Bekerja dengan berkas .pub](pub-files).

## Simpan

Tekan Ctrl+S, klik tombol **Simpan** di Bilah Alat Cepat, atau klik **Berkas > Simpan**. Untuk pertama kali, JeffPub meminta nama dan lokasi. Setelah itu, Simpan menggantikan berkas dengan pekerjaan terbaru Anda.

Tanda bintang (*) setelah nama di bilah judul berarti ada perubahan yang belum disimpan. Simpan sering-sering. Jika JeffPub berhenti tiba-tiba, lihat [Memulihkan pekerjaan yang belum disimpan](recover).

Untuk menyimpan salinan terakhir yang tersimpan dari sebuah berkas, aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffPub lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah nama berkas, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi akan menanyakan apakah Anda ingin menyimpan tanpa salinan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula. Opsi ini ada di **Berkas > Opsi**, pada tab **Umum**. Setiap aplikasi JeffOffice memilikinya.

## Simpan dengan nama baru

Klik **Berkas > Simpan Sebagai**, atau tekan F12. Ketik nama, pilih **Simpan sebagai jenis**, lalu klik **Simpan**. Pilihannya adalah:

- **Publikasi JeffPub (*.jpub)** adalah pilihan yang biasa.
- **Berkas Publikasi .pub (*.pub)** membuat berkas .pub.
- **PDF (*.pdf)** membuat PDF dan membiarkan publikasi Anda tetap seperti semula.
- **Templat JeffPub (*.jpub)** menambahkan publikasi ke **Templat Saya**.

Setelah menyimpan sebagai .jpub atau .pub, bilah judul menampilkan nama baru, dan Anda tetap bekerja di berkas baru itu.

## Simpan salinan

Klik **Berkas > Simpan Salinan**, lalu pilih nama. JeffPub menulis publikasi ke berkas itu, dan Anda tetap bekerja di berkas yang sebelumnya. Nama dan perubahan yang belum disimpan pada berkas yang sebelumnya tetap seperti semula.

## Buka

1. Tekan Ctrl+O, atau klik **Berkas > Buka**.
2. Klik **Telusuri** untuk memilih berkas. Atau klik nama di bawah **Disematkan** atau **Publikasi Terbaru**.

Anda juga dapat menyeret berkas .jpub atau .pub ke halaman. Jika publikasi Anda saat ini memiliki perubahan yang belum disimpan, JeffPub menanyakannya lebih dulu.

Klik kanan nama dalam daftar untuk **Sematkan ke daftar** (nama tetap berada di atas) atau **Hapus dari daftar**. Nama yang berwarna abu-abu berarti JeffPub tidak dapat menemukan berkas itu saat ini. Untuk mengubah jumlah nama terbaru yang ditampilkan, lihat [Mengubah pengaturan JeffPub](options).

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lebih Banyak Cara Membuka**. Anda juga dapat mengklik kanan nama di daftar Terbaru.

- **Buka Hanya-Baca** membuka publikasi hanya untuk dibaca. Anda dapat melihat dan menggulir, tetapi tidak dapat mengubah apa pun. Bilah judul bertuliskan "Read-Only". Jika Anda menekan **Simpan**, JeffPub meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka publikasi tanpa berkas sendiri. **Simpan** akan menanyakan lokasi penyimpanannya.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri.
- **Buka dan Perbaiki** untuk berkas .jpub yang rusak, misalnya berkas yang terpotong saat disalin. JeffPub membaca setiap bagian berkas yang dapat dibaca, dan melewatkan bagian yang tidak dapat dibaca. Publikasi dibuka sebagai salinan bernama "Repaired" ditambah nama berkas, dan sebuah pesan memberi tahu bagian apa saja yang dilewati. Berkas yang rusak tidak diubah.

## Tutup

Klik **Berkas > Tutup**, atau tekan Ctrl+F4. JeffPub menanyakan apakah perubahan perlu disimpan, lalu menampilkan halaman kosong.

## Info: properti, Tandai sebagai Final, Periksa, dan versi

**Berkas > Info** menampilkan lokasi berkas, dengan **Tampilkan di Folder** dan **Salin Jalur**. Halaman ini juga menampilkan fakta tentang publikasi Anda: jumlah halaman, ukuran halaman, gambar, kata, dan tanggal. Ketik di dalam kotak untuk mengatur **Judul**, **Pengarang**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**, lalu klik **Properti Kustom** untuk properti milik Anda sendiri. Perubahan disimpan bersama publikasi, dan **Ctrl+Z** membatalkannya. Halaman ini juga membuka informasi bisnis Anda, **Pengaturan Cetak Komersial** (lihat [Berkas PDF dan percetakan](pdf)), dan [Pemeriksa Desain](design-checker).

- **Periksa Kompatibilitas** menampilkan hal-hal dari publikasi Anda yang tidak dapat disimpan dalam berkas .pub, tanpa menyimpan apa pun. Misalnya objek tersembunyi, beberapa tampilan daftar, dan hal lain yang tidak ditulis oleh penulis .pub. Saat Anda menyimpan sebagai berkas .pub, JeffPub menampilkan daftar yang sama **sebelum** menulis apa pun. Pilih **Tetap Simpan**, **Simpan sebagai .jpub** (berkas .jpub disimpan di sebelahnya, dan tidak ada yang hilang), atau **Batal**. Centang **Jangan tanyakan lagi untuk berkas ini**, dan JeffPub tidak akan menanyakan lagi untuk berkas itu sampai Anda menutup program.
- **Tandai sebagai Final** menyimpan publikasi lalu membatasinya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa berkas hanya-baca, dan sebuah catatan memberi tahu mereka bahwa berkas itu ditandai sebagai final. Ini bukan kunci: pilih **Tandai sebagai Final** lagi untuk mengedit. Berkas .pub tidak dapat menyimpan tanda ini, jadi simpan dulu sebagai berkas .jpub.
- **Periksa Publikasi** menampilkan hal-hal yang ada di dalam berkas dan mungkin tidak ingin Anda bagikan, lalu menghapusnya: komentar dan nama orang yang menulisnya, pengarang dan nama lain dalam properti, informasi bisnis, daftar surat massal, serta objek yang diletakkan di area coret-coret di samping halaman.
- **Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, menghapus pengarang, orang yang terakhir menyimpan berkas, perusahaan, dan manajer dari berkas setiap kali Anda menyimpan, baik untuk berkas .jpub maupun .pub. Ini adalah kotak yang sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.
- **Riwayat Versi** menyimpan salinan publikasi yang diberi nama di komputer Anda. Simpan publikasi terlebih dahulu. Klik **Simpan Versi…**, lalu nanti pilih salah satu untuk **Buka Salinan** darinya, **Kembali ke Versi Ini** (publikasi seperti keadaannya sekarang disimpan lebih dulu sebagai versi), atau **Hapus**.

Berkas .jpub tidak memiliki format untuk kata sandi, jadi JeffPub tidak memiliki kartu kata sandi. JeffPub belum memiliki pemeriksaan aksesibilitas, jadi halaman ini tidak memiliki kartu untuk itu.

## Jika Windows tidak mengizinkan JeffPub menyimpan

Keamanan Windows dapat memblokir program menyimpan di folder seperti Dokumen. JeffPub kemudian menampilkan pesan dengan langkah-langkah untuk mengizinkannya. Atau simpan di folder lain.

## Terkait

- [Memulihkan pekerjaan yang belum disimpan](recover)
- [Bekerja dengan berkas .pub](pub-files)
- [Memulai publikasi baru](create)
- [Mengubah pengaturan JeffPub](options)
