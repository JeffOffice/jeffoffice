# Pack and Go: membawa presentasi Anda
<!-- keywords: pack and go, paket, kemas, kirim, bagikan, komputer lain, zip, folder, font, tautan berkas, tautan gambar, gambar hilang, salin, bawa, flash disk, lisensi, film, video, suara, audio, tampilan, presentasi -->

Sebuah presentasi dapat memutar film atau suara dari berkas yang tidak berada di dalamnya. Di komputer lain, berkas itu tidak ada, sehingga tayangan akan terputus. **Pack and Go** menyimpan presentasi beserta berkas tertautnya dan font-nya dalam satu berkas ZIP atau satu folder.

## Cara menggunakannya

1. Klik **Berkas**, lalu klik **Ekspor**.
2. Klik **Pack and Go: Save for Another Computer** untuk membuat satu berkas ZIP, atau **Pack and Go: Save to a Folder** untuk membuat folder baru.
3. Pilih tempat penyimpanan. Untuk berkas ZIP, ketikkan nama. Untuk folder, pilih folder tempat folder baru akan dibuat.
4. Baca pesan yang muncul. Pesan tersebut menyebutkan lokasi paket, berapa banyak berkas tertaut dan berkas font yang dimasukkan, serta hal apa pun yang tidak dapat dikemas.

Berkas **ZIP** adalah satu berkas yang memuat berkas-berkas lain, sehingga mudah dikirim. Penerima membukanya untuk mengeluarkan berkasnya. Folder lebih baik jika Anda ingin menyalin semuanya ke sebuah drive.

## Isi paket

- **Presentasinya sendiri**, dengan nama yang sama seperti sekarang. Ini berupa salinan. Berkas Anda sendiri tidak berubah.
- **Film dan suara tertaut.** Film atau suara dapat ditautkan, sehingga presentasi memutarnya dari berkasnya alih-alih menyimpannya di dalam presentasi. Paket memuat salinan setiap berkas tersebut, sehingga tayangan berjalan sama di komputer lain. Jika Anda tidak memasukkan berkas tersebut ke dalam presentasi, cara ini membuatnya ikut terbawa.
- **Gambar tertaut.** Paket memuat salinan setiap gambar yang ditautkan ke berkasnya.
- **Font.** Font adalah tampilan huruf. Paket memuat berkas font yang disertakan program dan digunakan presentasi, di dalam folder bernama **fonts**. Pack and Go tidak pernah mengemas font yang lisensinya melarang pembawaan. Pesan akan menyebutkan setiap font yang dilewati. Font yang Anda pasang sendiri di komputer tidak dikemas, sehingga penerima mungkin memerlukannya.

## Cara tautan berubah

Dalam salinan, setiap tautan yang dikemas mengarah ke berkas di dalam paket dengan nama pendek, misalnya **linked/photo.png**. Tautan tidak lagi memakai jalur di komputer Anda, sehingga tetap berfungsi di mana pun paket dibuka. Jika dua berkas memiliki nama yang sama, berkas kedua akan diberi tambahan angka.

Tautan yang tidak dapat diikuti Pack and Go akan tetap seperti semula, dan pesan akan mencantumkannya. Pack and Go tidak mengemas:

- Berkas yang tidak dapat ditemukan.
- Alamat web. Alamat tersebut tetap seperti semula.
- Berkas di komputer lain pada jaringan Anda.
- Tautan yang naik dan keluar dari folder presentasi, misalnya **../../notes.png**.
- Dokumen, buku kerja, atau objek lain yang tidak berada di folder presentasi itu sendiri (atau folder di dalamnya), bahkan jika tautannya memuat jalur lengkap. Hanya gambar, film, dan suara yang dikemas dari lokasi mana pun.
- Tautan yang keluar dari folder presentasi melalui pintasan ke folder lain (symbolic link).
- Tautan ke presentasi itu sendiri.
- Berkas yang bukan gambar, film, suara, atau dokumen. Program tidak pernah dikemas.

Pack and Go tidak memeriksa tautan lain di dalam berkas yang dikemas, sehingga berkas yang menautkan kembali ke presentasi ini tidak dapat menyebabkan perulangan tanpa henti.

## Batasan

Agar paket tidak membengkak, Pack and Go menanyakan terlebih dahulu jika ada lebih dari 200 berkas tertaut, atau lebih dari 256 MB. Pilih **Tidak** dan tidak ada yang disimpan. Pack and Go tidak pernah mengemas lebih dari 2.000 berkas, atau lebih dari 1 GB, apa pun jawaban Anda. Berkas di luar batas tersebut tidak dimasukkan dan disebutkan dalam pesan.

Jika presentasi Anda menggunakan kata sandi, salinan di dalam paket memiliki kata sandi yang sama.
