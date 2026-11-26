# Mencari, mengganti, dan membuka

<!-- keywords: cari, temukan, ganti, mengganti, cari apa, ganti dengan, ganti semua, cari berikutnya, cari teks, ubah, ubah semua, huruf kapital sama, wildcard, bintang, tanda tanya, tilde, cocokkan huruf besar/kecil, seluruh sel, cari berdasarkan format, format, ganti di catatan, buka, referensi, nama, pilih berdasarkan jenis, kosong, konstanta, rumus, kesalahan, wilayah saat ini, array saat ini, perbedaan baris, perbedaan kolom, sel terakhir, sel terlihat, format bersyarat, validasi, Ctrl+F, Ctrl+H, F5, catatan -->

## Mencari

Tekan **Ctrl+F** (atau pilih **Temukan** pada tab Beranda, lalu **Temukan** lagi). Ketik teks di **Cari apa** dan tekan **Enter** untuk sel berikutnya yang memuat teks tersebut, atau pilih **Daftar Semua** untuk mendapatkan daftar setiap sel yang cocok. Klik salah satu baris daftar untuk membuka sel tersebut.

| Opsi | Fungsinya |
| --- | --- |
| **Dimana** | Mencari di lembar ini, atau di setiap lembar yang tampil. Jika lebih dari satu sel dipilih, pencarian pada satu lembar hanya mencari di sel-sel tersebut. |
| **Urutan** | Mengunjungi sel menurut baris (kiri ke kanan, lalu ke bawah) atau menurut kolom (atas ke bawah, lalu ke kanan). **Cari Berikutnya** berhenti pada kecocokan pertama menurut urutan tersebut. |
| **Cari di** | **Rumus** mencari apa yang diketik di setiap sel, **Nilai** mencari apa yang ditampilkan sel, dan **Catatan** mencari di catatan pada sel. |
| **Format…** | Hanya mencari sel yang memiliki format tertentu. **Format…** membuka kotak dialog Format Sel, dimulai dari sel tempat kursor berada: apa pun yang Anda ubah di sana menjadi yang dicari (font tebal, warna isian, format angka). **Format Dari Sel** mengambil seluruh format sel tempat kursor berada, dan **Hapus Format** kembali hanya mencari teks. Kotak dialog menyebutkan apa yang dicari, dan memberi tahu jika format yang dipilih tidak berisi apa pun. Tampilan yang diberikan format bersyarat pada sel tidak dicari, hanya format yang dimiliki sel itu sendiri. Jika **Cari apa** kosong, **Daftar Semua** dan **Cari Berikutnya** menemukan sel yang memiliki format tersebut; jika ada teks, sel harus memiliki keduanya. |
| **Cocokkan huruf besar/kecil** | Huruf besar dan kecil harus sama. |
| **Seluruh sel saja** | Seluruh isi sel harus berupa teks tersebut, bukan hanya sebagian. |

Dalam teks yang Anda ketik di **Cari apa**, **?** mewakili satu karakter apa pun dan **\*** mewakili sederet karakter apa pun (termasuk tidak ada karakter). Ketik **~** di depan ?, \*, atau ~ untuk mencari tanda itu sendiri: **a~\*b** cocok dengan a\*b.

Pencarian berjalan sedikit demi sedikit sehingga Anda tetap dapat bekerja. **Esc** (atau **Batal**) menghentikannya, begitu juga dengan mengubah sel. Teks yang sangat panjang membutuhkan lebih banyak waktu pada setiap langkah kecil dibanding teks pendek, sehingga lembar berisi teks panjang tetap membuat jendela tetap responsif.

Catatan pada sel mengikuti sel tersebut: setelah pengurutan, atau setelah baris atau kolom disisipkan atau dihapus, pencarian catatan menemukan setiap catatan pada sel tempat catatan itu berada sekarang. Catatan pada sel yang dihapus tidak lagi ikut dicari (dan kembali setelah Urungkan). Jika buku kerja telah diubah sedemikian rupa sehingga catatan tidak dapat diikuti melalui semua perubahan, Anda akan diminta menyimpannya terlebih dahulu. Buku kerja yang dibuka dari jenis berkas lama atau dari berkas teks memiliki catatan dalam bentuk yang tidak dicari JeffCrunch, dan Anda akan diberi tahu.

## Mengganti

Tekan **Ctrl+H**, ketik teks baru di **Ganti dengan**, lalu pilih salah satu tombol. **Ganti** memasukkan teks baru ke sel tempat kursor berada jika sel itu memuat teks tersebut, lalu lanjut ke sel berikutnya. **Ganti Semua** melakukannya pada setiap sel yang memuatnya, sebagai satu langkah yang dapat diurungkan; perubahan besar dimasukkan sedikit demi sedikit. Teks baru dimasukkan seolah-olah Anda mengetiknya: angka tetap menjadi angka, dan rumus yang diubah tetap menjadi rumus.

Untuk mengganti di catatan, pilih **Catatan** pada **Cari di** di halaman Ganti. **Ganti Semua** kemudian mengubah kata-kata pada setiap catatan yang memuat teks tersebut, di lembar ini atau di setiap lembar, sebagai satu langkah yang dapat diurungkan, dan **Ganti** mengubah catatan pada sel tempat kursor berada lalu lanjut ke berikutnya. Catatan yang berupa percakapan komentar dibiarkan apa adanya (ubah catatan tersebut pada tab Tinjau), dan Anda akan diberi tahu berapa banyak yang dibiarkan. Tidak ada yang diubah, dan Anda akan diberi tahu alasannya, jika lembar yang memiliki catatan untuk diubah sedang dilindungi, atau jika catatan akan kehilangan semua katanya atau melebihi 32.767 karakter. Catatan yang katanya diganti akan kehilangan tanggal penulisannya, seperti saat Anda mengedit katanya. Catatan disimpan bersama buku kerja, dalam setiap bentuk yang memuat catatan. **Nilai** dapat dicari tetapi tidak dapat diganti, karena nilai adalah hasil yang dihitung oleh rumus.

Tidak ada yang diubah, dan Anda akan diberi tahu alasannya, jika sel yang akan diubah terkunci pada lembar yang dilindungi, merupakan bagian dari array atau tabel data, berada di baris pertama sebuah tabel, atau tidak akan menjadi rumus lagi.

## Buka

Tekan **F5** (atau **Ctrl+G**), atau ketik di Kotak Nama. Anda dapat memasukkan sel seperti B7, rentang seperti B3:D9, seluruh kolom atau baris (C:E, 4:9), sel pada lembar lain (Sheet2!A1, atau 'Lembar saya'!A1), atau nama sebuah rentang. Nama yang mewakili sel yang sama pada sekumpulan lembar, seperti Jan:Dec!B2, akan membawa Anda ke lembar pertama dari kumpulan tersebut.

## Pilih berdasarkan jenis

Pilih **Pilih Berdasarkan Jenis** pada menu **Temukan** untuk memilih sel berdasarkan jenisnya:

- **Sel kosong**, **Konstanta** (yang diketik langsung), **Rumus**, dan **Sel yang menampilkan kesalahan**. Untuk konstanta dan rumus, Anda dapat mengabaikan angka, teks, TRUE dan FALSE, atau kesalahan.
- **Blok di sekitar sel tempat kursor berada**, dan **Sel terakhir yang digunakan**.
- **Rumus array tempat kursor berada** memilih semua sel dari rumus array tersebut.
- **Sel yang berbeda dari sel kursor pada barisnya** dan **pada kolomnya** menemukan sel yang tidak sama dengan sel tempat kursor berada. Pilih sebuah blok terlebih dahulu, lalu letakkan kursor pada sel yang seharusnya menjadi acuan sel lain. Pada setiap baris (atau kolom) dalam blok, sebuah sel dipilih jika berisi angka atau teks yang berbeda, atau berisi rumus yang tidak sama setelah nama selnya dipindahkan ke posisi sel tersebut. Sel yang berisi rumus juga dipilih jika sel kursor tidak berisi rumus. Jika Anda hanya memilih satu sel, JeffCrunch memeriksa blok di sekitarnya.
- **Sel yang dibaca rumus dalam pilihan** dan **Sel dengan rumus yang membaca pilihan** memilih sel pada lembar ini yang digunakan oleh rumus dari sel yang dipilih, atau sel yang rumusnya menggunakan sel yang dipilih. Jika hanya satu sel yang dipilih, pencarian dimulai dari sel tersebut. Centang **telusuri sampai setiap tingkat** untuk melanjutkan dari setiap sel yang ditemukan (sel yang dibacanya, dan seterusnya); tanpa itu, hanya tingkat pertama yang dipilih. Pencarian dimulai dari paling banyak 1.000 sel yang dipilih, dan bilah status memberi tahu jika pencarian berhenti karena batas.
- **Sel yang ditampilkan** (bukan pada baris atau kolom yang disembunyikan), **Sel dengan format bersyarat**, dan **Sel dengan aturan untuk apa yang boleh dimasukkan** (semuanya, atau hanya yang memiliki aturan sama dengan sel tempat kursor berada).

Blok pertama yang ditemukan akan dipilih dan blok lainnya diberi warna latar; warna tersebut hilang saat Anda mengklik sebuah sel.
