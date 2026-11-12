# Kotak dialog Page Setup

<!-- keywords: page setup, pengaturan halaman, dialog, kotak dialog, header, footer, nomor halaman, tanggal, halaman pertama, ganjil genap, hitam putih, draf, catatan, kesalahan, urutan halaman, pusatkan di halaman, nomor halaman awal, kode header dan footer, kualitas cetak, dpi, titik per inci -->

Bukalah dari tanda panah kecil di sudut grup **Pengaturan Halaman** pada tab **Tata Letak Halaman**. Kotak dialog ini memiliki empat tab. **OK** mengubah lembar yang sedang Anda buka, sebagai satu langkah yang dapat Anda urungkan.

## Halaman

- **Arah kertas**: tegak atau melebar.
- **Ukuran sel saat dicetak**: persen dari ukuran penuh, atau **Perkecil agar pas**, dengan jumlah halaman ke samping dan ke bawah. "Berapa pun jumlahnya" membiarkan arah tersebut memakai halaman sebanyak yang dibutuhkan.
- **Kertas**: ukuran kertas.
- **Kualitas cetak**: jumlah titik per inci yang diminta dari printer. Titik yang lebih banyak menghasilkan cetakan yang lebih halus, dan sebagian printer membutuhkan waktu lebih lama. Jika sebuah buku kerja meminta angka yang tidak ada dalam daftar, JeffCrunch tetap mempertahankannya.
- **Mulai nomor halaman dari**: nomor yang diberikan kepada halaman pertama. Jika tidak diisi, halaman pertama bernomor 1, dan buku kerja yang dicetak utuh menomori halamannya secara berurutan dari satu lembar ke lembar berikutnya.

## Margin

Ketikkan ruang di sekeliling sel, serta seberapa jauh header dan footer berada dari tepi atas dan tepi bawah kertas. Kotak-kotak ini menggunakan sentimeter jika komputer Anda diatur ke wilayah metrik, dan inci di tempat lain. **Mendatar di halaman** dan **Ke bawah halaman** menempatkan sel di tengah halaman.

## Header dan Footer

**Header** adalah baris kata di bagian atas setiap halaman, dan **footer** adalah baris di bagian bawah. Pilih salah satu dari daftar, atau pilih **Edit header** atau **Edit footer** untuk menulis sendiri. Masing-masing memiliki bagian kiri, tengah, dan kanan, serta tombol untuk menyisipkan nomor halaman, jumlah halaman, tanggal, waktu, nama berkas, folder, dan nama lembar. Isi ini berubah dari halaman ke halaman, jadi dihitung saat halaman dicetak. Untuk memasukkan gambar ke sebuah bagian, klik di bagian tersebut lalu pilih **Gambar…**, kemudian pilih berkas PNG, JPEG, GIF, atau BMP. Gambar dibuat tidak lebih dari 2 inci (5 cm) pada sisi terpanjangnya, dan dicetak di tempat kode `&G` berada. **Hapus Gambar** menghapusnya. Satu bagian hanya memuat satu gambar, dan halaman pertama serta halaman genap masing-masing dapat memiliki gambar sendiri.

Anda juga dapat mengatur header dan footer yang berbeda untuk halaman pertama, serta untuk halaman ganjil dan genap, dan memilih apakah huruf membesar dan mengecil mengikuti sel, dan apakah header serta footer sejajar dengan margin.

Di balik tombol-tombol tersebut terdapat kode pendek yang disimpan dalam berkas, dan Anda dapat mengetikkannya sendiri: **&P** adalah nomor halaman, **&N** jumlah halaman, **&D** tanggal, **&T** waktu, **&F** nama berkas, **&A** nama lembar, dan **&&** adalah tanda ampersand. **&B**, **&I**, dan **&U** menyalakan dan mematikan huruf tebal, miring, dan garis bawah, **&12** mengatur ukuran menjadi 12 poin, dan **&"Arial,Bold"** memilih font. **&L**, **&C**, dan **&R** memulai bagian kiri, tengah, dan kanan.

## Lembar

- **Sel yang akan dicetak**: area cetak, berupa blok seperti **A1:D20, F1:G9**. Biarkan kosong untuk mencetak semuanya.
- **Ulangi baris ini di bagian atas** dan **Ulangi kolom ini di sisi kiri**: judul cetak, seperti **1:3** dan **A:B**.
- **Cetak juga**: garis di antara sel, nomor baris dan huruf kolom, hanya hitam putih (tanpa warna atau isian), dan draf cepat (tanpa garis di antara sel, serta tanpa gambar, bagan, atau bentuk).
- **Catatan** dan **Sel yang menampilkan kesalahan**: apa yang dilakukan terhadap keduanya. Pengaturan untuk kesalahan mengubah apa yang ditampilkan halaman: seperti yang ditampilkan sel, tidak ada apa-apa, dua tanda hubung, atau #N/A. Untuk catatan: **Jangan sertakan** tidak mencetak catatan apa pun. **Tampilkan setelah halaman terakhir** mencetak setiap catatan setelah halaman terakhir lembar, pada halaman tersendiri yang membawa nomor halaman berikutnya serta header dan footer lembar, dengan format "Sel A2: penulis: isi catatan", sesuai urutan sel. **Tampilkan di tempatnya** disimpan dalam berkas untuk program lain, tetapi JeffCrunch belum mencetak catatan pada sel.
- **Halaman berikutnya**: ke bawah lalu ke samping, atau ke samping lalu ke bawah.
