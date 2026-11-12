# Periksa desain Anda

<!-- keywords: pemeriksa desain, periksa, kesalahan, masalah, resolusi rendah, meluap, teks tidak muat, kotak teks kosong, di luar halaman, preflight, pemeriksaan akhir, bukti cetak, gambar tertaut, font hilang, design checker -->

**Pemeriksa Desain** menelusuri publikasi Anda untuk mencari hal-hal yang sering bermasalah. Pemeriksa ini menemukan teks yang tidak muat, kotak kosong, objek yang menggantung di luar halaman, dan gambar yang akan tercetak buruk. Jalankan sebelum Anda mencetak atau mengirim berkas ke percetakan.

## Jalankan Pemeriksa Desain

Klik **Tinjau > Periksa > Jalankan Pemeriksa Desain**. Panel **Pemeriksa Desain** terbuka di sisi kanan. Anda juga dapat mengklik **Berkas > Info**, lalu **Jalankan Pemeriksa Desain**.

Panel ini memiliki dua kotak centang, keduanya aktif sejak awal:

- **Jalankan pemeriksaan desain umum** mencari masalah tata letak.
- **Jalankan pemeriksaan penerbitan akhir** mencari hal-hal yang penting saat Anda mencetak.

Biarkan keduanya aktif untuk melihat semua jenis masalah. Daftar diperbarui saat Anda bekerja. Jika tidak ada yang perlu diperbaiki, daftar menampilkan "Tidak ada masalah yang ditemukan."

## Perbaiki masalah

1. Klik sebuah item dalam daftar di bawah **Pilih item untuk diperbaiki**.
2. Klik **Buka Item**. Atau klik dua kali item tersebut.

JeffPub membuka halaman tersebut dan memilih objeknya. Perbaiki objek itu, dan item akan hilang dari daftar.

## Yang ditemukan

| Pesan | Artinya | Yang perlu dilakukan |
|---|---|---|
| Halaman tidak memiliki konten | Halaman itu kosong. | Tambahkan sesuatu, atau hapus halaman. |
| Objek sebagian berada di luar halaman | Sebagian objek berada di luar halaman. | Pindahkan, atau biarkan jika itu area bleed. |
| Kotak teks kosong | Sebuah kotak teks tidak berisi teks. | Ketikkan sesuatu di dalamnya, atau hapus. |
| Cerita dengan teks meluap | Cerita adalah teks yang mengalir melalui kotak-kotak tertaut. Cerita ini memiliki lebih banyak teks daripada yang muat. | Perbesar kotak, potong teks, atau [tautkan kotak lain](linked-text). |
| Teks diperkecil jauh agar pas | Memperkecil teks agar pas dengan kotaknya membuat ukuran teks kurang dari setengah ukuran semula. | Perbesar kotak atau gunakan teks yang lebih sedikit. |
| Placeholder gambar kosong | Bingkai gambar tidak berisi gambar. | Klik bingkai itu untuk menambahkan gambar, atau hapus. |
| Gambar tidak diskalakan secara proporsional | Gambar diregangkan atau dipadatkan. | Ubah ukurannya untuk memulihkan bentuknya. |
| Gambar tertaut, bukan tersemat | Gambar terikat pada sebuah berkas dan hanya pratinjau kecil yang disimpan dalam publikasi. Gambar yang disisipkan dengan **Sisipkan dan Tautkan** disimpan utuh dan tidak tercantum. | Simpan berkas asli bersama publikasi Anda, atau klik **Sematkan Gambar** di [Pengelola Grafis](graphics-manager). |
| Gambar tertaut hilang | Berkas yang dituju gambar tertaut sudah tidak ada, sehingga halaman hanya menampilkan pratinjau kecilnya, dan itulah yang akan tercetak. | Di Pengelola Grafis, klik **Ubah Tautan…** untuk mencari berkasnya, atau kembalikan berkas itu lalu klik **Perbarui Tautan**. |
| Gambar tertaut hanya menampilkan pratinjau hingga Perbarui Tautan | Gambar ditautkan ke berkas di luar folder publikasi, yang tidak dibuka JeffPub secara otomatis. Halaman menampilkan pratinjau kecil, dan itulah yang akan tercetak. | Di Pengelola Grafis, pilih gambar itu lalu klik **Perbarui Tautan**. |
| Gambar beresolusi rendah | Gambar memiliki kurang dari 150 ppi (piksel per inci) pada ukuran cetaknya. | Gunakan gambar yang lebih besar, atau cetak dengan ukuran lebih kecil. |
| Objek memiliki transparansi atau efek (mungkin tercetak berbeda) | Area tembus pandang dan efek dapat tampak berbeda di atas kertas. | Cetak halaman uji. |
| Font "nama" tidak terpasang | Sebuah font dalam teks Anda tidak ada di komputer ini, sehingga font lain menggantikannya. | Pasang font tersebut, atau pilih font lain. |

Setiap pesan diakhiri dengan nomor halaman, seperti "(Halaman 2)".

## Sebelum berkas untuk percetakan

Saat Anda membuat berkas PDF/X, JeffPub memberi peringatan tentang gambar beresolusi rendah dan font yang hilang, lalu menawarkan untuk membuka Pemeriksa Desain. Lihat [Berkas PDF dan percetakan](pdf).

## Terkait

- [Berkas PDF dan percetakan](pdf)
- [Cetak publikasi](printing)
- [Alirkan teks di antara kotak teks](linked-text)
- [Tambah dan atur gambar](pictures)
- [Pengelola Grafis](graphics-manager)
