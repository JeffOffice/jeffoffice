# Gabung surat massal dan katalog
<!-- keywords: gabung surat massal, surat massal, cetak surat massal, penerima, daftar alamat, label, surat berantai, csv, xlsx, vcard, bidang gabungan, blok alamat, katalog, daftar produk, gabung email, pratinjau -->

Gabung surat massal mengisi satu publikasi dengan rincian dari sebuah daftar. Anda menulis surat sekali saja, dan JeffPub membuat salinannya untuk setiap orang, dengan nama dan alamat orang tersebut di tempatnya. Gabung katalog mengulang satu tata letak untuk setiap item dalam daftar.

Alat-alatnya ada pada tab **Surat Massal**. Untuk langkah demi langkah yang terpandu, klik **Surat Massal > Mulai > Mulai Surat Massal > Pemandu Cetak Surat Massal Langkah demi Langkah** untuk membuka panel **Cetak Surat Massal**.

## Pilih penerima Anda

Klik **Surat Massal > Mulai > Pilih Penerima**:

- **Gunakan Daftar yang Ada…**: pilih berkas CSV, berkas teks yang dipisahkan tab, lembar kerja .xlsx (lembar pertama), atau berkas kontak .vcf. Pada berkas CSV, teks, atau lembar kerja, baris pertama harus berisi nama kolom.
- **Pilih dari Kontak…**: pilih berkas kontak (.vcf atau .vcard). Sebagian besar buku alamat dan ponsel dapat menyimpan kontak sebagai berkas seperti ini; setiap kontak menjadi satu penerima, dengan kolom nama, alamat, telepon, dan email.
- **Buat Daftar Baru…**: isi jendela **Daftar Alamat Baru**, dengan **Entri Baru** untuk setiap orang. Setelah itu JeffPub menawarkan untuk menyimpan daftar sebagai berkas CSV.

Untuk memeriksa daftar, klik **Surat Massal > Mulai > Edit Daftar Penerima**. Di jendela **Penerima Surat Massal**, hilangkan centang pada baris untuk melewati orang tersebut, klik judul kolom untuk mengurutkan, dan ketik di **Saring penerima** untuk mempersempit daftar. **Bedakan penerima berdasarkan** memilih kolom yang membedakan setiap orang; daftar disimpan beserta baris yang tidak dicentang di dalam publikasi Anda, dan ketika Anda memilih berkas daftar yang sama lagi, orang yang sama akan tetap dikecualikan, meskipun Anda sudah mengurutkan barisnya terlebih dahulu. **Temukan Duplikat** menghilangkan centang pada data yang berulang, dan **Validasi Alamat** menyorot baris dengan alamat yang hilang atau kode pos yang janggal.

## Tambahkan bidang gabungan

Bidang gabungan adalah penampung, seperti «Nama Depan», yang menjadi nilai sebenarnya untuk setiap orang. Klik di dalam kotak teks (JeffPub akan menambahkan satu jika Anda belum punya), lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Blok Alamat**: alamat surat yang dibentuk dari kolom bernama seperti Nama Depan, Nama Belakang, dan Alamat Baris 1.
- **Baris Salam**: "Yth." dan nama orang tersebut, atau "Yth. Sahabat," jika tidak ada nama.
- **Sisipkan Bidang Gabungan**: pilih kolom mana pun.
- **Bidang Gambar**: pilih kolom berisi nama berkas gambar. JeffPub menambahkan bingkai kosong dan mencari berkas tersebut di dekat daftar Anda. Setelah Anda menyimpan publikasi dan membukanya lagi, gambar yang berkasnya berada di luar folder publikasi tidak akan tampil sampai Anda memilih daftar penerima lagi (**Surat Massal > Pilih Penerima > Gunakan Daftar yang Ada…**). Setelah itu, gambar di folder daftar dan di folder di dalamnya akan tampil sampai Anda menutup publikasi. JeffPub tidak mengingat hal ini untuk lain waktu. Atau, simpan berkas gambar di folder publikasi, atau di folder di dalamnya, dan gambar akan tampil begitu publikasi dibuka. Cara ini mencegah publikasi yang dikirim seseorang kepada Anda membaca gambar dari komputer Anda secara otomatis. Gambar setiap rekaman akan tampil saat Anda melakukan pratinjau hasil, mencetak, menyimpan PDF, atau menggabungkan ke publikasi baru atau katalog.

## Tampilkan angka, harga, tanggal, atau waktu sesuai keinginan

Bidang gabungan menampilkan nilai persis seperti yang tersimpan dalam daftar. Untuk mengubahnya, klik di samping bidang (atau pilih bidang itu, atau beberapa bidang), lalu klik **Surat Massal > Tulis & Sisipkan Bidang > Format Bidang Gabungan…**. Di bawah **Tampilkan nilai sebagai**, pilih:

- **Angka**, dengan **Tempat desimal** 0 hingga 4. Misalnya, 1234,5 dengan 1 tempat desimal ditampilkan sebagai 1.234,5.
- **Mata uang**, dengan simbol mata uang wilayah Anda: 1234,5 ditampilkan sebagai Rp1.234,50 di tempat simbol tersebut digunakan.
- **Persen**, misalnya 0,256 ditampilkan sebagai 25,6% dengan 1 tempat desimal.
- **Tanggal**, dengan **Gaya** seperti Pendek, Panjang, Tahun, Bulan, hari, Bulan dan hari, atau Bulan dan tahun.
- **Waktu**, dengan **Gaya** Pendek, 12 jam, atau 24 jam.

Baris **Contoh** menampilkan hasil pilihan Anda. Nilai dibaca dan ditampilkan dalam bahasa teks pada bidang tersebut (bahasa Inggris Amerika jika belum diatur), bukan bahasa komputer Anda. Dalam bahasa Inggris Amerika, 03/04/2026 berarti 4 Maret, sedangkan dalam bahasa yang menempatkan hari di depan, seperti Jerman atau Inggris Britania, artinya 3 April; dalam bahasa Jerman, 1.234,5 adalah angka dengan koma desimal. Tahun dua digit dibaca sebagai 2000 hingga 2029 untuk 00 sampai 29, dan 1930 hingga 1999 untuk 30 sampai 99. Angka dalam tanda kurung, seperti (500), dianggap negatif. Nilai yang bukan angka, tanggal, atau waktu sesuai jenis yang Anda pilih akan ditampilkan persis seperti dalam daftar: sebuah kata, "nan" atau "inf" (seseorang mungkin bernama Nan), nilai kosong, waktu dengan format tanggal, atau tanggal dengan format waktu. Pratinjau dan hasil gabungan akhir sama-sama menampilkan nilai yang sudah diformat. Pilihan ini dapat dibatalkan satu langkah dengan Urungkan, dan disimpan bersama publikasi.

## Pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil > Pratinjau Hasil** untuk melihat detail sebenarnya menggantikan bidang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk membolak-balik orang-orang tersebut, atau **Temukan Penerima** untuk melompat ke data yang cocok.

## Selesaikan penggabungan

Klik **Surat Massal > Selesai > Selesai & Gabungkan**:

- **Gabungkan ke Printer…** membuka halaman **Cetak**. Centang **Cetak semua rekaman surat massal**, lalu cetak.
- **Gabungkan ke Publikasi Baru** membuat publikasi baru dengan salinan halaman Anda untuk setiap orang.
- **Gabungkan ke PDF…** menyimpan satu PDF yang berisi halaman setiap orang.
- **Gabungkan ke Email…** membuat satu berkas email .eml untuk setiap orang, dengan halaman pertama sebagai gambar. Pilih kolom email, ketik subjek, dan pilih folder. JeffPub tidak mengirim email tersebut. Buka berkasnya di program email Anda.
- **Simpan Masing-masing sebagai Berkas…** membuat satu publikasi .jpub untuk setiap orang, di folder yang Anda pilih. Beri nama berkas dengan kolom dalam kurung kurawal, seperti {Nama Belakang}, {Nama Depan}. Anda akan diminta konfirmasi sebelum lebih dari 200 berkas, dan sekali jika berkas dengan nama yang sama sudah ada. Klik **Batal** untuk berhenti di antara dua berkas.
- **Ekspor Daftar Penerima…** menyimpan daftar sebagai berkas CSV.

## Gabung katalog

Klik **Sisipkan > Halaman > Halaman Katalog** untuk membuka panel **Gabung Katalog**.

1. Pilih daftar produk, seperti di atas.
2. Klik **Sisipkan Area Katalog**. Sebuah kotak berlabel "Area katalog" akan muncul. Atur **Baris** dan **Kolom**, serta posisi dan ukurannya. Letakkan bidang untuk satu item di sel pertama dengan mengklik dua kali bidang tersebut dalam daftar. Bidang itu akan diulang di sel-sel lainnya. Pilih versi "(gambar)" dari sebuah bidang untuk menampilkan gambar.
3. Lakukan pratinjau dengan panah. Setiap halaman menampilkan item sebanyak jumlah sel pada area tersebut. Kemudian gabungkan ke publikasi baru, PDF, atau printer.

**Hapus Area Katalog** menghapus area tersebut.

## Terkait

- [Cetak publikasi](printing)
- [Tambahkan dan format kotak teks](text-boxes)
- [Tab Surat Massal](tab-mailings)
