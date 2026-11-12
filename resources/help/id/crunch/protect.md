# Melindungi lembar atau buku kerja
<!-- keywords: lindungi, perlindungan, kata sandi, sandi, kunci, sel terkunci, buka kunci, hapus perlindungan, struktur, lembar, hanya baca, izinkan, format sel, sisipkan baris, urutkan, filter, hash, izinkan rentang edit, rentang pengeditan, kata sandi rentang -->

## Melindungi lembar

**Tinjau > Lindungi Lembar** mencegah orang mengubah lembar secara tidak sengaja. Pada lembar yang dilindungi, sel yang **terkunci** tidak dapat diubah. Setiap sel terkunci secara default hingga Anda mengubahnya: pilih sel yang boleh diubah, buka **Format Sel > Proteksi**, lalu hapus centang **Terkunci**. (Pengaturan ini hanya berpengaruh setelah lembar dilindungi.)

Kotak dialog menanyakan apa saja yang masih boleh dilakukan orang pada lembar tersebut: memilih sel terkunci atau tidak terkunci, memformat sel, mengubah kolom atau baris, menyisipkan atau menghapus baris dan kolom, memasukkan tautan, mengurutkan, menggunakan filter, serta mengubah objek gambar dan catatan. Tindakan yang tidak Anda centang akan ditolak, disertai pesan yang menjelaskan apa yang ditolak.

**Kata sandi** bersifat opsional. Jika Anda memberikannya, lembar hanya dapat dibuka proteksinya dengan kata sandi tersebut. JeffCrunch menyimpan bentuk kata sandi yang telah diacak (hash bergaram, diulang 100.000 kali), bukan kata sandinya sendiri, sehingga JeffCrunch tidak dapat menampilkan kembali kata sandi tersebut nanti. Simpan kata sandi Anda di tempat yang aman. Kata sandi yang dibuat program lain, baik dalam bentuk lama maupun baru, diperiksa dengan cara yang sama.

Melindungi dan membuka perlindungan adalah langkah yang dapat diurungkan. Untuk menghapus perlindungan, gunakan **Hapus Perlindungan Lembar**, lalu ketik kata sandi jika ada.

Beberapa hal tetap tidak diizinkan pada lembar yang dilindungi, apa pun yang Anda izinkan: menggabungkan sel, serta menghapus baris atau kolom yang berisi sel terkunci.

## Mengizinkan perubahan pada sebagian sel: Izinkan Rentang Edit

**Tinjau > Izinkan Rentang Edit** menampilkan daftar **rentang** yang boleh diubah orang meskipun lembar dilindungi. Rentang adalah blok sel, misalnya A1:C5. Klik **Baru** untuk menambahkan rentang. Beri **judul**, ketik sel-sel yang dicakup (gunakan spasi di antara blok), dan jika perlu, beri **kata sandi**. Rentang tanpa kata sandi dapat diubah oleh siapa saja. Rentang dengan kata sandi hanya dapat diubah setelah seseorang mengetikkan kata sandinya pada percobaan pertama mengubah salah satu sel di dalamnya. Setelah itu, rentang tetap terbuka hingga buku kerja ditutup.

**Ubah** mengubah rentang, dan **Hapus** menghapusnya. Anda dapat menyiapkan rentang sebelum melindungi lembar. Rentang disimpan bersama buku kerja, dan kata sandinya disimpan dalam bentuk acak yang sama seperti kata sandi lembar. Izinkan Rentang Edit adalah langkah yang dapat diurungkan. Saat Anda menyisipkan atau menghapus baris, kolom, atau sel, rentang ikut berpindah bersama sel-selnya, menyusut atau membesar, atau hilang jika semua selnya dihapus. Urungkan untuk mengembalikannya.

## Melindungi buku kerja

**Tinjau > Lindungi Buku Kerja** mengunci **struktur**: tidak ada lembar yang dapat ditambahkan, dihapus, diganti nama, dipindahkan, disembunyikan, atau ditampilkan. Pilihan ini juga dapat meminta agar jendela tetap seperti semula. Perlindungan ini juga dapat memakai kata sandi. **Hapus Perlindungan Buku Kerja** untuk menghapusnya.
