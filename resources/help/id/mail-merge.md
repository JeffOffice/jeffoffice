# Cetak surat massal dan gabung katalog
<!-- keywords: cetak surat massal, gabung, penerima, daftar alamat, label, surat massal, csv, xlsx, vcard, bidang gabungan, blok alamat, katalog, daftar produk, gabung email, pratinjau -->

Cetak surat massal mengisi satu publikasi dengan rincian dari sebuah daftar. Anda menulis surat sekali saja, lalu JeffPub membuat salinan untuk setiap orang, dengan nama dan alamat orang tersebut di tempatnya. Gabung katalog mengulang satu tata letak untuk setiap item dalam daftar.

Alat-alatnya ada di tab **Surat Massal**. Untuk jalur terpandu, klik **Surat Massal > Mulai > Cetak Surat Massal > Pemandu Cetak Surat Massal Langkah demi Langkah** untuk membuka panel **Cetak Surat Massal**.

## Pilih penerima Anda

Klik **Surat Massal > Mulai > Pilih Penerima**:

- **Gunakan Daftar yang Ada…**: pilih berkas CSV, berkas teks yang dipisahkan tab, lembar kerja .xlsx (lembar pertama), atau berkas kontak .vcf. Pada berkas CSV, teks, atau lembar kerja, baris pertama harus berisi nama kolom.
- **Pilih dari Kontak…**: pilih berkas kontak (.vcf atau .vcard). Sebagian besar buku alamat dan ponsel dapat menyimpan kontak mereka sebagai berkas tersebut; setiap kontak menjadi satu penerima, dengan kolom nama, alamat, telepon, dan email.
- **Ketik Daftar Baru…**: isi jendela **Daftar Alamat Baru**, dengan menggunakan **Entri Baru** untuk setiap orang. JeffPub kemudian menawarkan untuk menyimpan daftar sebagai berkas CSV.

Untuk memeriksa daftar, klik **Surat Massal > Mulai > Edit Daftar Penerima**. Di jendela **Mail Merge Recipients**, hapus centang pada baris untuk melewati orang tersebut, klik judul kolom untuk mengurutkan, dan ketik di **Saring penerima** untuk mempersempit daftar. **Temukan Duplikat** menghapus centang pada data yang berulang, dan **Validasi Alamat** menyorot baris dengan alamat yang hilang atau kode pos yang janggal.

## Tambahkan bidang gabungan

Bidang gabungan adalah penampung, seperti «First Name», yang menjadi nilai sebenarnya untuk setiap orang. Klik di dalam kotak teks (JeffPub akan menambahkannya jika Anda belum membuatnya), lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Blok Alamat**: alamat surat yang disusun dari kolom bernama seperti First Name, Last Name, dan Address Line 1.
- **Baris Salam**: "Dear" dan nama orang tersebut, atau "Dear Friend," jika tidak ada nama.
- **Sisipkan Bidang Gabungan**: pilih kolom mana pun.
- **Bidang Gambar**: pilih kolom berisi nama berkas gambar. JeffPub menambahkan bingkai kosong dan mencari berkas tersebut di dekat daftar Anda. Setelah Anda menyimpan publikasi dan membukanya kembali, gambar yang berkasnya berada di luar folder publikasi tidak akan tampil hingga Anda memilih daftar penerima lagi (**Surat Massal > Pilih Penerima > Gunakan Daftar yang Ada…**). Setelah itu, gambar di folder daftar, dan di folder di dalamnya, akan tampil hingga Anda menutup publikasi. JeffPub tidak mengingat hal ini untuk lain kali. Atau simpan berkas gambar di folder publikasi, atau di folder di dalamnya, dan gambar akan tampil begitu publikasi dibuka. Cara ini mencegah publikasi yang dikirim seseorang kepada Anda membaca gambar dari komputer Anda secara otomatis. Gambar setiap rekaman tampil saat Anda melihat pratinjau hasil, mencetak, menyimpan PDF, atau menggabungkan ke publikasi baru atau katalog.

## Lihat pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil > Pratinjau Hasil** untuk melihat rincian sebenarnya menggantikan bidang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk membolak-balik orang-orang tersebut, atau **Cari Penerima** untuk melompat ke data yang cocok.

## Selesaikan penggabungan

Klik **Surat Massal > Selesai > Selesai & Gabungkan**:

- **Gabungkan ke Printer...** membuka halaman **Cetak**. Centang **Cetak semua rekaman surat massal**, lalu cetak.
- **Gabungkan ke Publikasi Baru** membuat publikasi baru berisi salinan halaman Anda untuk setiap orang.
- **Gabungkan ke PDF…** menyimpan satu PDF berisi halaman semua orang.
- **Gabung ke Email…** membuat satu berkas email .eml per orang, dengan halaman pertama sebagai gambar. Pilih kolom email, ketik subjek, lalu pilih folder. JeffPub tidak mengirimkannya. Buka berkas tersebut di program email Anda.
- **Ekspor Daftar Penerima…** menyimpan daftar sebagai berkas CSV.

## Gabung katalog

Klik **Sisipkan > Halaman > Halaman Katalog** untuk membuka panel **Gabung Katalog**.

1. Pilih daftar produk, seperti di atas.
2. Klik **Sisipkan Area Katalog**. Sebuah kotak berlabel "Catalog area" muncul. Atur **Baris** dan **Kolom**, serta posisi dan ukurannya. Letakkan bidang untuk satu item di sel pertama dengan mengklik dua kali bidang tersebut dalam daftar. Bidang itu akan diulang di sel lainnya. Pilih versi "(picture)" dari suatu bidang untuk gambar.
3. Lihat pratinjau dengan panah. Setiap halaman menampilkan item sebanyak jumlah sel pada area tersebut. Lalu gabungkan ke publikasi baru, PDF, atau printer.

**Hapus Area Katalog** menghilangkan area tersebut.

## Terkait

- [Cetak publikasi](printing)
- [Tambahkan dan format kotak teks](text-boxes)
- [Tab Surat Massal](tab-mailings)
