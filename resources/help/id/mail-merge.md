# Cetak surat massal dan gabung katalog
<!-- keywords: cetak surat massal, surat massal, gabung, penerima, daftar alamat, label, surat massal, csv, xlsx, vcard, bidang gabungan, blok alamat, katalog, daftar produk, gabung email, pratinjau -->

Cetak surat massal mengisi satu publikasi dengan data dari sebuah daftar. Anda menulis surat sekali, lalu JeffPub membuat salinannya untuk setiap orang, dengan nama dan alamat orang tersebut di tempatnya. Gabung katalog mengulang satu tata letak untuk setiap item dalam daftar.

Alat-alatnya ada di tab **Surat Massal**. Untuk langkah demi langkah, klik **Surat Massal > Mulai > Cetak Surat Massal > Pemandu Cetak Surat Massal Langkah demi Langkah** untuk membuka panel **Cetak Surat Massal**.

## Pilih penerima Anda

Klik **Surat Massal > Mulai > Pilih Penerima**:

- **Gunakan Daftar yang Ada…**: pilih berkas CSV, berkas teks yang dipisahkan tab, lembar kerja .xlsx (lembar pertama), atau berkas kontak .vcf. Pada berkas CSV, teks, atau lembar kerja, baris pertama harus berisi nama kolom.
- **Pilih dari Kontak…**: pilih berkas kontak (.vcf atau .vcard). Sebagian besar buku alamat dan ponsel dapat menyimpan kontak sebagai berkas seperti ini; setiap kontak menjadi satu penerima, dengan kolom nama, alamat, telepon, dan email.
- **Ketik Daftar Baru…**: isi jendela **Daftar Alamat Baru**, dengan **Entri Baru** untuk setiap orang. Setelah itu, JeffPub menawarkan untuk menyimpan daftar tersebut sebagai berkas CSV.

Untuk memeriksa daftar, klik **Surat Massal > Mulai > Edit Daftar Penerima**. Di jendela **Penerima Surat Massal**, hapus centang pada baris untuk melewati orang tersebut, klik judul kolom untuk mengurutkan, dan ketik di **Saring penerima** untuk mempersempit daftar. **Temukan Duplikat** menghapus centang pada data yang berulang, dan **Validasi Alamat** menyorot baris yang alamatnya kosong atau kode posnya janggal.

## Tambahkan bidang gabungan

Bidang gabungan adalah tempat penampung, seperti «Nama Depan», yang menjadi nilai sungguhan untuk setiap orang. Klik di dalam kotak teks (JeffPub menambahkannya jika Anda belum membuatnya), lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Blok Alamat**: alamat surat yang disusun dari kolom bernama seperti Nama Depan, Nama Belakang, dan Alamat Baris 1.
- **Baris Salam**: "Yth." diikuti nama orang tersebut, atau "Yth. Sahabat," jika tidak ada nama.
- **Sisipkan Bidang Gabungan**: pilih kolom apa pun.
- **Bidang Gambar**: pilih kolom berisi nama berkas gambar. JeffPub menambahkan bingkai kosong dan mencari berkas gambar di sebelah daftar Anda. Setelah Anda menyimpan publikasi dan membukanya kembali, gambar yang berkasnya berada di luar folder publikasi tidak akan tampil sampai Anda memilih daftar penerima lagi (**Surat Massal > Pilih Penerima > Gunakan Daftar yang Ada…**). Setelah itu, gambar di folder daftar tersebut, dan di folder di dalamnya, akan tampil sampai Anda menutup publikasi. JeffPub tidak mengingat hal ini untuk lain kali. Atau, simpan berkas gambar di folder publikasi, atau di folder yang berada di dalamnya, maka gambar akan tampil begitu publikasi dibuka. Cara ini mencegah publikasi yang dikirim seseorang kepada Anda membaca gambar dari komputer Anda sendiri secara otomatis. Gambar setiap catatan tampil saat Anda melihat pratinjau hasil, mencetak, menyimpan PDF, atau menggabungkan ke publikasi baru atau katalog.

## Tampilkan angka, harga, tanggal, atau waktu sesuai keinginan

Bidang gabungan menampilkan nilai persis seperti yang ada di daftar. Untuk mengubahnya, klik di samping bidang (atau pilih bidangnya, atau beberapa bidang), lalu klik **Surat Massal > Tulis & Sisipkan Bidang > Format Bidang Gabungan…**. Di bawah **Tampilkan nilai sebagai**, pilih:

- **Angka**, dengan **Tempat desimal** 0 sampai 4. Misalnya, 1234,5 dengan 1 tempat desimal ditampilkan sebagai 1.234,5.
- **Mata uang**, dengan tanda mata uang wilayah Anda: 1234,5 ditampilkan sebagai $1.234,50 jika itu tanda yang berlaku.
- **Persen**, misalnya 0,256 ditampilkan sebagai 25,6% dengan 1 tempat desimal.
- **Tanggal**, dengan **Gaya** seperti Pendek, Panjang, Tahun, Bulan, hari, Bulan dan hari, atau Bulan dan tahun.
- **Waktu**, dengan **Gaya** Pendek, 12 jam, atau 24 jam.

Baris **Contoh** menampilkan hasil dari pilihan Anda. Nilai dibaca dan ditampilkan dalam bahasa teks pada bidang tersebut (bahasa Inggris Amerika jika belum diatur), bukan bahasa komputer Anda. Dalam bahasa Inggris Amerika, 03/04/2026 berarti 4 Maret, sedangkan dalam bahasa yang hari ditulis lebih dulu, seperti bahasa Jerman atau Inggris Britania, artinya 3 April; dalam bahasa Jerman, 1.234,5 adalah angka dengan koma desimal. Tahun dua digit dibaca sebagai 2000 sampai 2029 untuk 00 sampai 29, dan 1930 sampai 1999 untuk 30 sampai 99. Angka dalam tanda kurung, seperti (500), adalah negatif. Nilai yang bukan angka, tanggal, atau waktu sesuai jenis yang Anda pilih akan ditampilkan persis seperti di daftar: sebuah kata, "nan" atau "inf" (seseorang mungkin bernama Nan), nilai kosong, waktu dengan format tanggal, atau tanggal dengan format waktu. Pratinjau dan hasil akhir gabungan sama-sama menampilkan nilai yang sudah diformat. Pilihan ini merupakan satu langkah **Urungkan**, dan disimpan bersama publikasi.

## Pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil > Pratinjau Hasil** untuk melihat data sungguhan di tempat bidang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk membolak-balik orang-orang tersebut, atau **Temukan Penerima** untuk melompat ke data yang cocok.

## Selesaikan penggabungan

Klik **Surat Massal > Selesai > Selesai & Gabungkan**:

- **Gabungkan ke Printer…** membuka halaman **Cetak**. Centang **Cetak semua rekaman surat massal**, lalu cetak.
- **Gabungkan ke Publikasi Baru** membuat publikasi baru berisi salinan halaman Anda untuk setiap orang.
- **Gabungkan ke PDF…** menyimpan satu berkas PDF berisi halaman setiap orang.
- **Gabung ke Email…** membuat satu berkas email .eml untuk setiap orang, dengan halaman pertama sebagai gambar. Pilih kolom email, ketik subjek, lalu pilih folder. JeffPub tidak mengirim email tersebut. Buka berkas-berkas itu di program email Anda.
- **Ekspor Daftar Penerima…** menyimpan daftar sebagai berkas CSV.

## Gabung katalog

Klik **Sisipkan > Halaman > Halaman Katalog** untuk membuka panel **Gabung Katalog**.

1. Pilih daftar produk, seperti dijelaskan di atas.
2. Klik **Sisipkan Area Katalog**. Sebuah kotak berlabel "Area katalog" muncul. Atur **Baris** dan **Kolom**, serta posisi dan ukurannya. Letakkan bidang untuk satu item di sel pertama dengan mengklik dua kali bidang tersebut dalam daftar. Bidang itu akan berulang di sel-sel lainnya. Pilih versi "(gambar)" dari sebuah bidang untuk menampilkan gambar.
3. Lihat pratinjau dengan tombol panah. Setiap halaman menampilkan sejumlah item sesuai jumlah sel pada area tersebut. Setelah itu, gabungkan ke publikasi baru, PDF, atau printer.

**Hapus Area Katalog** menghapus area tersebut.

## Terkait

- [Cetak publikasi](printing)
- [Tambahkan dan format kotak teks](text-boxes)
- [Tab Surat Massal](tab-mailings)
