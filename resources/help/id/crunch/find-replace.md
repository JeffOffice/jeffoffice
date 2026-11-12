# Cari, ganti, dan buka

<!-- keywords: cari, ganti, ubah, temukan, cari apa, ganti dengan, ganti semua, cari berikutnya, cari teks, ubah menjadi, ubah semua, huruf kapital sama, karakter pengganti, bintang, tanda tanya, tilde, cocokkan huruf besar kecil, seluruh sel, buka, referensi, nama, pilih berdasarkan jenis, kosong, konstanta, rumus, kesalahan, wilayah saat ini, array saat ini, perbedaan baris, perbedaan kolom, sel terakhir, sel terlihat, format bersyarat, validasi, Ctrl+F, Ctrl+H, F5, catatan -->

## Cari

Tekan **Ctrl+F** (atau pilih **Temukan** pada tab Beranda, lalu **Temukan** lagi). Ketik teks di **Temukan apa** dan tekan **Enter** untuk sel berikutnya yang memuatnya, atau pilih **Daftar Semua** untuk mendapatkan daftar setiap sel yang cocok. Klik salah satu baris daftar untuk pergi ke sel tersebut.

| Opsi | Fungsinya |
| --- | --- |
| **Dalam** | Mencari di lembar ini, atau di setiap lembar yang ditampilkan. Jika lebih dari satu sel dipilih, pencarian pada satu lembar hanya melihat sel-sel tersebut. |
| **Urutan** | Mengunjungi sel menurut baris (kiri ke kanan, lalu ke bawah) atau menurut kolom (atas ke bawah, lalu ke kanan). **Cari Berikutnya** berhenti di kecocokan pertama dalam urutan itu. |
| **Cari dalam** | **Rumus** mencari apa yang diketik di setiap sel, **Nilai** mencari apa yang ditampilkan, dan **Catatan** mencari catatan pada sel. |
| **Cocokkan huruf besar/kecil** | Huruf kapital dan huruf kecil harus sama. |
| **Seluruh sel saja** | Seluruh isi sel harus berupa teks tersebut, bukan hanya sebagiannya. |

Dalam teks yang Anda ketik di **Temukan apa**, **?** mewakili satu karakter apa pun dan **\*** mewakili rangkaian karakter apa pun (termasuk tidak ada). Ketik **~** di depan ?, \*, atau ~ untuk mencari tanda itu sendiri: **a~\*b** cocok dengan a\*b.

Pencarian berjalan sedikit demi sedikit sehingga Anda tetap dapat bekerja. **Esc** (atau **Batal**) menghentikannya, begitu juga dengan mengubah sel. Teks yang sangat panjang membutuhkan lebih banyak waktu pada setiap langkah kecil dibandingkan teks pendek, sehingga lembar yang berisi banyak teks panjang tetap membuat jendela tetap responsif.

Catatan pada sel mengikuti sel tersebut: setelah diurutkan, atau setelah baris atau kolom disisipkan atau dihapus, pencarian catatan menemukan setiap catatan pada sel tempat catatan itu berada sekarang. Catatan pada sel yang telah dihapus tidak lagi ikut dicari (dan kembali setelah Urungkan). Jika buku kerja telah diubah sedemikian rupa sehingga catatan tidak dapat diikuti melalui semua perubahan itu, Anda akan diminta untuk menyimpannya terlebih dahulu. Buku kerja yang dibuka dari jenis berkas lama atau dari berkas teks memiliki catatan dalam bentuk yang tidak dapat dicari oleh JeffCrunch, dan Anda akan diberi tahu hal tersebut.

## Ganti

Tekan **Ctrl+H**, ketik teks baru di **Ganti dengan**, lalu pilih tombol. **Ganti** memasukkan teks baru ke sel tempat kursor berada jika sel itu memuat teks tersebut, lalu lanjut ke sel berikutnya. **Ganti Semua** melakukannya pada setiap sel yang memuatnya, sebagai satu langkah yang dapat diurungkan; perubahan besar dimasukkan sedikit demi sedikit. Teks baru dimasukkan seolah-olah Anda mengetiknya: angka tetap menjadi angka, dan rumus yang diubah tetap menjadi rumus.

Tidak ada yang diubah, dan Anda akan diberi tahu alasannya, jika sel yang akan diubah terkunci pada lembar yang dilindungi, merupakan bagian dari array atau tabel data, berada di baris pertama sebuah tabel, atau tidak lagi menjadi rumus.

## Buka

Tekan **F5** (atau **Ctrl+G**), atau ketik di Kotak Nama. Anda dapat memasukkan sel seperti B7, rentang seperti B3:D9, seluruh kolom atau baris (C:E, 4:9), sel pada lembar lain (Sheet2!A1, atau 'Lembar saya'!A1), atau nama sebuah rentang.

## Pilih berdasarkan jenis

Pilih **Pilih Berdasarkan Jenis** pada menu **Temukan** untuk memilih sel menurut jenisnya:

- **Sel kosong**, **Konstanta** (yang diketik), **Rumus**, dan **Sel yang menampilkan kesalahan**. Untuk konstanta dan rumus, Anda dapat mengabaikan angka, teks, TRUE dan FALSE, atau kesalahan.
- **Blok di sekitar sel tempat kursor berada**, dan **Sel terakhir yang digunakan**.
- **Rumus array tempat kursor berada** memilih semua sel dari rumus array tersebut.
- **Sel yang berbeda dari sel kursor pada barisnya** dan **pada kolomnya** menemukan sel-sel yang tidak sama dengan sel tempat kursor berada. Pilih blok terlebih dahulu, lalu letakkan kursor pada sel yang harus dicocokkan oleh sel lainnya. Pada setiap baris (atau kolom) di dalam blok, sebuah sel dipilih jika berisi angka atau teks yang berbeda, atau jika berisi rumus yang tidak sama setelah referensi selnya dipindahkan ke posisi sel tersebut. Sel yang berisi rumus juga dipilih jika sel kursor tidak berisi rumus. Jika Anda hanya memilih satu sel, JeffCrunch memeriksa blok di sekitarnya.
- **Sel yang ditampilkan** (tidak termasuk baris atau kolom tersembunyi), **Sel dengan format bersyarat**, dan **Sel dengan aturan untuk isian yang diizinkan** (semuanya, atau hanya yang memiliki aturan sama dengan sel tempat kursor berada).

Blok pertama yang ditemukan dipilih dan blok lainnya diberi warna latar; warna tersebut hilang saat Anda mengklik sel.
