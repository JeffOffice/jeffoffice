# Cari, ganti, dan buka

<!-- keywords: cari, ganti, ubah, temukan, cari apa, ganti dengan, ganti semua, cari berikutnya, cari, ubah menjadi, ubah semua, huruf kapital sama, wildcard, bintang, tanda tanya, tilde, cocokkan huruf besar/kecil, seluruh sel, cari berdasarkan format, format, ganti di catatan, buka, referensi, nama, pilih berdasarkan jenis, sel kosong, konstanta, rumus, kesalahan, wilayah saat ini, array saat ini, perbedaan baris, perbedaan kolom, sel terakhir, sel terlihat, format bersyarat, validasi, Ctrl+F, Ctrl+H, F5, catatan -->

## Cari

Tekan **Ctrl+F** (atau pilih **Temukan** pada tab Beranda, lalu **Temukan** lagi). Ketik teks di **Temukan apa** lalu tekan **Enter** untuk sel berikutnya yang memuatnya, atau pilih **Daftar Semua** untuk mendapatkan daftar setiap sel yang cocok. Klik salah satu baris daftar untuk membuka sel tersebut.

| Opsi | Fungsinya |
| --- | --- |
| **Dalam** | Mencari pada lembar ini, atau pada setiap lembar yang ditampilkan. Jika lebih dari satu sel dipilih, pencarian pada satu lembar hanya melihat sel-sel tersebut. |
| **Urutan** | Mengunjungi sel menurut baris (kiri ke kanan, lalu ke bawah) atau menurut kolom (atas ke bawah, lalu ke kanan). **Cari Berikutnya** berhenti pada kecocokan pertama dalam urutan itu. |
| **Cari di** | **Rumus** mencari apa yang diketik di setiap sel, **Nilai** mencari apa yang ditampilkan, dan **Catatan** mencari di catatan pada sel. |
| **Format…** | Hanya mencari sel yang memiliki format tertentu. **Format…** membuka kotak dialog Format Sel, dimulai dari sel tempat kursor berada: apa yang Anda ubah di sana menjadi yang dicari (misalnya huruf tebal, warna isian, atau format angka). **Format Dari Sel** mengambil setiap bagian format sel tempat kursor berada, dan **Hapus Format** kembali hanya mencari teks. Kotak dialog menyatakan apa yang dicari, dan memberi tahu jika format yang Anda pilih tidak berisi apa pun. Tampilan yang diberikan format bersyarat pada sel tidak dicari, hanya format yang dimiliki sel itu sendiri. Jika **Temukan apa** kosong, **Daftar Semua** dan **Cari Berikutnya** menemukan sel yang memiliki format tersebut; jika berisi teks, sebuah sel harus memiliki keduanya. |
| **Cocokkan huruf besar/kecil** | Huruf kapital dan huruf kecil harus sama. |
| **Hanya sel utuh** | Seluruh isi sel harus berupa teks tersebut, bukan hanya sebagian. |

Dalam teks yang Anda ketik di **Temukan apa**, **?** mewakili satu karakter apa pun dan **\*** mewakili sejumlah karakter apa pun (termasuk tanpa karakter). Ketik **~** di depan ?, \*, atau ~ untuk mencari tanda itu sendiri: **a~\*b** cocok dengan a\*b.

Pencarian berjalan sedikit demi sedikit, sehingga Anda tetap dapat bekerja. **Esc** (atau **Batal**) menghentikannya, begitu juga dengan mengubah sel. Teks yang sangat panjang membutuhkan lebih banyak waktu pada setiap langkah kecil dibandingkan teks pendek, sehingga lembar berisi banyak teks panjang tetap membuat jendela tetap responsif.

Catatan pada sel mengikuti sel: setelah pengurutan, atau setelah baris atau kolom disisipkan atau dihapus, pencarian catatan menemukan setiap catatan pada sel tempatnya berada sekarang. Catatan pada sel yang dihapus tidak lagi ikut dalam pencarian (dan kembali setelah Urungkan). Jika buku kerja telah diubah sedemikian rupa sehingga catatan tidak dapat diikuti di semua perubahan, Anda diminta menyimpannya terlebih dahulu. Buku kerja yang dibuka dari jenis berkas lama atau dari berkas teks memiliki catatan dalam bentuk yang tidak dicari JeffCrunch, dan Anda akan diberi tahu hal itu.

## Ganti

Tekan **Ctrl+H**, ketik teks baru di **Ganti dengan**, lalu pilih salah satu tombol. **Ganti** memasukkan teks baru ke sel tempat kursor berada jika sel itu memuat teks yang dicari, lalu lanjut ke sel berikutnya. **Ganti Semua** melakukannya di setiap sel yang memuatnya, sebagai satu langkah yang dapat diurungkan; perubahan besar dilakukan sedikit demi sedikit. Teks baru dimasukkan seolah-olah Anda mengetiknya: angka tetap menjadi angka, dan rumus yang diubah tetap menjadi rumus.

Untuk mengganti di catatan, pilih **Catatan** pada **Cari di** di halaman Ganti. **Ganti Semua** kemudian mengubah kata-kata pada setiap catatan yang memuat teks tersebut, pada lembar ini atau pada setiap lembar, sebagai satu langkah yang dapat diurungkan, dan **Ganti** mengubah catatan pada sel tempat kursor berada lalu lanjut ke berikutnya. Catatan yang berupa percakapan komentar dibiarkan apa adanya (ubah catatan tersebut pada tab Tinjau), dan Anda diberi tahu berapa banyak yang dibiarkan. Tidak ada yang diubah, dan Anda diberi tahu alasannya, jika lembar yang berisi catatan untuk diubah sedang dilindungi, atau jika sebuah catatan akan kehilangan semua katanya atau melebihi 32.767 karakter. Catatan yang katanya diganti akan kehilangan tanggal penulisannya, sama seperti saat Anda mengedit katanya. Catatan disimpan bersama buku kerja, dalam setiap bentuk yang memuat catatan. **Nilai** dapat dicari tetapi tidak dapat diganti, karena nilai adalah hasil perhitungan sebuah rumus.

Tidak ada yang diubah, dan Anda diberi tahu alasannya, jika sel yang akan diubah terkunci pada lembar yang dilindungi, merupakan bagian dari array atau tabel data, berada di baris pertama sebuah tabel, atau tidak lagi menjadi rumus setelah diubah.

## Buka

Tekan **F5** (atau **Ctrl+G**), atau ketik di kotak Nama. Anda dapat memasukkan sel seperti B7, rentang seperti B3:D9, seluruh kolom atau baris (C:E, 4:9), sel pada lembar lain (Sheet2!A1, atau 'Lembar saya'!A1), atau nama sebuah rentang.

## Pilih berdasarkan jenis

Pilih **Pilih Berdasarkan Jenis** pada menu **Temukan** untuk memilih sel menurut jenisnya:

- **Sel kosong**, **Konstanta** (yang diketik langsung), **Rumus**, dan **Sel yang menampilkan kesalahan**. Untuk konstanta dan rumus, Anda dapat mengecualikan angka, teks, TRUE dan FALSE, atau kesalahan.
- **Blok di sekitar sel tempat kursor berada**, dan **Sel terakhir yang digunakan**.
- **Rumus array tempat kursor berada** memilih semua sel dari rumus array tersebut.
- **Sel yang berbeda dari sel kursor pada barisnya** dan **pada kolomnya** menemukan sel yang tidak sama dengan sel tempat kursor berada. Pilih blok terlebih dahulu, dan letakkan kursor pada sel yang harus dijadikan acuan bagi yang lain. Pada setiap baris (atau kolom) dalam blok, sebuah sel dipilih jika berisi angka atau teks yang berbeda, atau jika berisi rumus yang tidak sama setelah referensi selnya digeser ke posisi sel acuan. Sel yang berisi rumus juga dipilih jika sel kursor tidak memiliki rumus. Jika Anda hanya memilih satu sel, JeffCrunch memeriksa blok di sekitarnya.
- **Sel yang terlihat** (bukan pada baris atau kolom tersembunyi), **Sel dengan format bersyarat**, dan **Sel dengan aturan untuk isian yang diizinkan** (semuanya, atau hanya yang memiliki aturan sama dengan sel tempat kursor berada).

Blok pertama yang ditemukan dipilih dan blok lainnya diberi warna latar; warna itu hilang saat Anda mengklik sel.
