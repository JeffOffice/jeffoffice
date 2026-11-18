# Pack and Go: kirim buku kerja beserta yang dibutuhkannya
<!-- keywords: pack and go, kemas, paket, kirim, bagikan, komputer lain, zip, folder, font, tautan berkas, tautan gambar, gambar hilang, salin, bawa, flash disk, USB, lisensi, tautan eksternal, buku kerja tertaut, sel, bagan -->

Saat Anda mengirim buku kerja kepada orang lain, gambar dan buku kerja lain yang ditautkannya bisa hilang di tengah jalan. **Pack and Go** menyimpan buku kerja bersama berkas-berkas tersebut serta font-nya, dalam satu berkas ZIP atau satu folder, sehingga tampilannya sama di komputer lain.

## Cara menggunakan

1. Klik **Berkas**, lalu klik **Ekspor**.
2. Klik **Pack and Go: Simpan untuk Komputer Lain** untuk membuat satu berkas ZIP, atau **Pack and Go: Simpan ke Folder** untuk membuat folder baru.
3. Pilih lokasi penyimpanan. Untuk berkas ZIP, ketik nama berkas. Untuk folder, pilih folder tempat folder baru akan dibuat.
4. Baca pesan yang muncul. Pesan itu menunjukkan lokasi paket, jumlah berkas tertaut dan berkas font yang dimasukkan, serta hal-hal yang tidak dapat dikemas.

Berkas **ZIP** adalah satu berkas yang berisi berkas-berkas lain, sehingga mudah dikirim. Penerima membukanya untuk mengeluarkan berkas-berkasnya. Folder lebih baik jika Anda ingin menyalin semuanya ke flash disk.

## Apa saja yang dimasukkan ke dalam paket

- **Buku kerja itu sendiri**, dengan nama yang sama seperti sekarang. Ini adalah salinan. Berkas Anda sendiri tidak berubah.
- **Gambar tertaut.** Gambar dalam buku kerja dapat ditautkan ke berkasnya. Paket berisi salinan setiap gambar tersebut. Gambar yang disisipkan langsung ke buku kerja sudah berada di dalamnya.
- **Buku kerja tertaut.** Sebuah rumus dapat mengambil angka dari buku kerja lain. Paket juga berisi salinan buku kerja tersebut, dan tautannya kini menunjuk ke salinan itu. Font yang dimaksud adalah font yang dipakai pada sel, bagan, dan bentuk.
- **Font.** Font adalah tampilan huruf. Paket berisi berkas font yang disertakan dengan program dan yang dipakai buku kerja, dalam folder bernama **fonts**. Pack and Go tidak pernah mengemas font yang lisensinya melarang pemindahan. Pesan akan menyebutkan setiap font yang dilewatkan. Font yang Anda pasang sendiri di komputer tidak ikut dikemas, sehingga penerima mungkin memerlukannya.

## Bagaimana tautan berubah

Di salinan, setiap tautan yang berhasil dikemas menunjuk ke berkas di dalam paket, dengan nama pendek, misalnya **linked/photo.png**. Tautan tidak lagi memakai jalur di komputer Anda, sehingga tetap berfungsi di mana pun paket dibuka. Jika dua berkas memiliki nama yang sama, berkas kedua akan diberi angka tambahan.

Tautan yang tidak dapat diikuti Pack and Go akan tetap seperti semula, dan pesan akan mencantumkannya. Pack and Go tidak mengemas:

- Berkas yang tidak dapat ditemukan.
- Alamat web. Alamat tersebut tetap seperti semula.
- Berkas di komputer lain pada jaringan Anda.
- Tautan yang naik keluar dari folder buku kerja, misalnya **../../catatan.png**.
- Dokumen, buku kerja, atau objek lain yang tidak berada di folder buku kerja (atau folder di dalamnya), meskipun tautannya menyebutkan jalur lengkap. Hanya gambar, film, dan suara yang dikemas dari lokasi mana pun.
- Tautan yang keluar dari folder buku kerja melalui pintasan ke folder lain (tautan simbolik).
- Tautan ke buku kerja itu sendiri.
- Berkas yang bukan gambar, film, suara, atau dokumen. Program tidak pernah dikemas.

Pack and Go tidak memeriksa tautan lain di dalam berkas tertaut, sehingga berkas yang menautkan kembali ke buku kerja ini tidak dapat menyebabkan perulangan.

## Batasan

Agar paket tidak membengkak, Pack and Go akan menanyakan terlebih dahulu jika berkas tertaut lebih dari 200 atau totalnya lebih dari 256 MB. Pilih **Tidak** dan tidak ada yang disimpan. Pack and Go tidak pernah mengemas lebih dari 2.000 berkas atau lebih dari 1 GB, apa pun jawaban Anda. Berkas yang melebihi batas itu tidak dimasukkan dan dicantumkan dalam pesan.

Jika buku kerja Anda memakai kata sandi, salinan di dalam paket memakai kata sandi yang sama.
