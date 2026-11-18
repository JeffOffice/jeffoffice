# Pack and Go: kirim dokumen beserta yang dibutuhkannya
<!-- keywords: pack and go, kemas, paket, kirim, bagikan, komputer lain, zip, folder, map, font, huruf, tautan berkas, gambar tertaut, gambar hilang, salin, bawa, flash disk, USB, lisensi, INCLUDEPICTURE, INCLUDETEXT, bidang -->

Saat Anda mengirim dokumen kepada seseorang, gambar dan font yang digunakannya bisa hilang di tengah jalan. **Pack and Go** menyimpan dokumen beserta yang dibutuhkannya dalam satu berkas ZIP atau satu folder, sehingga tampilannya sama di komputer lain.

## Cara menggunakan

1. Klik **Berkas**, lalu klik **Ekspor**.
2. Klik **Pack and Go: Save for Another Computer** untuk membuat satu berkas ZIP, atau **Pack and Go: Save to a Folder** untuk membuat folder baru.
3. Pilih tempat penyimpanan. Untuk berkas ZIP, ketikkan nama. Untuk folder, pilih folder tempat folder baru akan dibuat.
4. Baca pesan yang muncul. Pesan itu menyebutkan lokasi paket, berapa banyak berkas tertaut dan berkas font yang dimasukkan, serta hal apa pun yang tidak dapat dikemas.

**Berkas ZIP** adalah satu berkas yang berisi berkas-berkas lain, sehingga mudah dikirim. Penerima membuka berkas itu untuk mengeluarkan isinya. Folder lebih cocok jika Anda ingin menyalin semuanya ke sebuah drive.

## Yang masuk ke dalam paket

- **Dokumen itu sendiri**, dengan nama yang sama seperti sekarang. Ini adalah salinan. Berkas Anda sendiri tidak berubah.
- **Gambar tertaut.** Gambar tertaut menampilkan gambar dari sebuah berkas, bukan menyimpan gambarnya sendiri. Paket menyimpan salinan dari setiap gambar tersebut.
- **Berkas yang dinamai oleh bidang.** Bidang adalah kode yang mengisi hasilnya sendiri. Bidang **INCLUDEPICTURE** menampilkan gambar dari sebuah berkas, dan bidang **INCLUDETEXT** menampilkan teks dari dokumen lain. Paket menyimpan salinan dari setiap berkas tersebut, dan bidang kini menunjuk ke salinan itu.
- **Font.** Font adalah tampilan huruf. Paket menyimpan berkas font yang disertakan dengan program dan yang digunakan dokumen, di dalam folder bernama **fonts**. Pack and Go tidak pernah mengemas font yang lisensinya melarang untuk dibawa. Pesan akan menyebutkan setiap font yang dibiarkan keluar. Font yang Anda pasang sendiri di komputer tidak ikut dikemas, sehingga penerima mungkin memerlukannya sendiri.

## Cara tautan berubah

Dalam salinan, setiap tautan yang dikemas menunjuk ke berkas di dalam paket, dengan nama pendek, misalnya **linked/photo.png**. Tautan tidak lagi memakai jalur di komputer Anda, sehingga tetap berfungsi di mana pun paket dibuka. Jika dua berkas memiliki nama yang sama, berkas kedua akan diberi tambahan angka.

Tautan yang tidak dapat diikuti Pack and Go akan tetap seperti semula, dan pesan akan mencantumkannya. Pack and Go tidak mengemas:

- Berkas yang tidak dapat ditemukan.
- Alamat web. Alamat tersebut tetap seperti semula.
- Berkas di komputer lain pada jaringan Anda.
- Tautan yang naik keluar dari folder dokumen, misalnya **../../notes.png**.
- Dokumen, buku kerja, atau objek lain yang tidak berada di dalam folder dokumen itu sendiri (atau folder di dalamnya), meskipun tautannya menyebutkan jalur lengkap. Hanya gambar, film, dan suara yang dikemas dari lokasi mana pun.
- Tautan yang keluar dari folder dokumen melalui pintasan ke folder lain (symbolic link).
- Tautan ke dokumen itu sendiri.
- Berkas yang bukan gambar, film, suara, atau dokumen. Program tidak pernah dikemas.

Pack and Go tidak mencari tautan tambahan di dalam berkas tertaut, sehingga berkas yang menaut kembali ke dokumen ini tidak dapat menyebabkan perulangan.

## Batasan

Agar paket tidak membengkak, Pack and Go akan bertanya terlebih dahulu jika ada lebih dari 200 berkas tertaut, atau lebih dari 256 MB berkas tertaut. Pilih **Tidak** dan tidak ada yang disimpan. Pack and Go tidak pernah mengemas lebih dari 2.000 berkas, atau lebih dari 1 GB, apa pun jawaban Anda. Berkas di luar batas itu dibiarkan keluar dan dicantumkan dalam pesan.

Jika dokumen Anda menggunakan kata sandi, salinan dalam paket memiliki kata sandi yang sama.
