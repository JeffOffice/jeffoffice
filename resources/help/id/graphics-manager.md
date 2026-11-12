# Pengelola Grafis

<!-- keywords: pengelola grafis, graphics manager, gambar, pictures, gambar tertaut, linked picture, tautkan ke berkas, link to file, perbarui tautan, update link, ubah tautan, change link, sematkan gambar, embed picture, tersemat, embedded, hilang, missing, diubah, modified, belum diperbarui, not updated, di luar folder, tautkan ulang, relink, cari berkas gambar, find picture file, status gambar, picture status, resolusi, resolution -->

**Pengelola Grafis** menampilkan daftar setiap gambar dalam publikasi dan memberi tahu Anda asal setiap gambar. Ini paling penting untuk gambar yang Anda tautkan ke berkasnya, karena gambar tertaut bergantung pada berkas di luar publikasi. Untuk cara menautkan gambar, lihat [Tambahkan dan atur gambar](pictures).

## Membuka Pengelola Grafis

Klik **Tampilan > Tampilkan > Pengelola Grafis**. Panel akan terbuka di sisi kanan. Klik tombol yang sama lagi, atau **X** di bagian atas panel, untuk menutupnya.

## Membaca daftar

Setiap baris menampilkan gambar kecil, nama berkas, halaman, jenis berkas, ukurannya dalam KB, dan **status**. Kotak di atas daftar mengurutkan baris menurut halaman, nama, ukuran, atau jenis. Di bawah daftar, rincian gambar yang dipilih memuat statusnya, berkasnya, apakah publikasi menyimpan seluruh gambar, ukurannya dalam piksel, dan **resolusi efektif**: berapa banyak piksel gambar yang jatuh pada setiap inci halaman. Sekitar 300 sudah baik untuk cetak.

| Status | Artinya |
|---|---|
| Tersemat | Gambar disimpan di dalam publikasi. Tidak ada yang diperlukan dari luar publikasi. |
| Tertaut | Gambar terhubung ke sebuah berkas, dan berkas itu berada di tempat yang diharapkan JeffPub, tidak berubah. |
| Hilang | Gambar terhubung ke berkas yang sudah tidak ada. Jika publikasi tidak menyimpan salinan, halaman menampilkan pratinjau kecil yang tampak buram, dan itulah yang akan tercetak. |
| Diubah | Berkas telah berubah sejak tautan dibuat atau terakhir diperbarui. Klik **Perbarui Tautan** untuk memuat perubahannya. |
| Belum diperbarui | Gambar terhubung ke berkas di luar folder publikasi, jadi JeffPub belum membuka berkas tersebut. Halaman menampilkan apa yang disimpan publikasi. Klik **Perbarui Tautan** untuk memakai berkas tersebut. Lihat "Gambar dari luar folder publikasi" di bawah. |

## Fungsi tombol

- **Buka** memilih gambar pada halamannya.
- **Simpan Sebagai…** menyimpan salinan gambar ke sebuah berkas.
- **Ganti…** memasukkan berkas lain ke dalam bingkai yang sama.
- **Perbarui Tautan** membaca berkas lagi. Gunakan untuk gambar **Diubah**, setelah Anda mengembalikan berkas yang **Hilang**, atau untuk gambar **Belum diperbarui**. Setiap gambar yang berbagi tautan yang sama ikut diperbarui, dan gambar tetap berada di tempatnya dengan lebar yang sama.
- **Ubah Tautan…** mengarahkan gambar ke berkas lain, misalnya saat Anda memindahkan berkas ke folder lain.
- **Sematkan Gambar** mengubah tautan menjadi salinan yang disimpan di dalam publikasi. Gambar kemudian tampil sebagai **Tersemat**. Jika berkas hilang, hanya pratinjau kecil yang dapat disematkan, dan bilah status akan memberi tahu Anda.

Ketiga tombol tautan berwarna abu-abu kecuali gambar yang dipilih berstatus tertaut. Setiap tombol merupakan satu langkah, jadi **Ctrl+Z** membatalkannya.

## Gambar dari luar folder publikasi

Saat Anda membuka publikasi, JeffPub membaca berkas gambar tertaut yang berada di folder yang sama dengan publikasi, atau di dalam folder tersebut. JeffPub tidak memeriksa berkas di tempat lain, baik di komputer maupun di jaringan Anda.

Alasannya: publikasi yang dikirim seseorang kepada Anda dapat menyebut berkas apa pun. Jika JeffPub membuka apa pun yang disebut publikasi, publikasi orang asing dapat menarik salah satu gambar pribadi Anda ke halamannya, yang kemudian mungkin Anda simpan atau kirim kembali tanpa menyadarinya. Di jaringan, sekadar memeriksa apakah sebuah berkas ada juga dapat mengirim info masuk Windows Anda ke komputer lain.

Jadi gambar yang ditautkan ke berkas di tempat lain akan menampilkan apa yang disimpan publikasi: seluruh gambar jika publikasi menyimpannya, jika tidak maka pratinjau kecil yang tampak buram. Statusnya **Belum diperbarui**, dan rinciannya menyatakan bahwa berkas berada di luar folder.

Untuk memakai berkas tersebut, pilih gambar lalu klik **Perbarui Tautan**, atau klik **Ubah Tautan…** dan pilih berkasnya sendiri. Itu merupakan persetujuan Anda untuk gambar tersebut, dan berlaku hingga Anda menutup publikasi. Gambar lain tetap **Belum diperbarui** sampai Anda melakukan hal yang sama untuk masing-masing gambar.

Menyimpan publikasi mempertahankan tautan seperti semula, sehingga saat Anda membuka publikasi lagi, gambar kembali berstatus **Belum diperbarui**. Jika ingin gambar dimuat secara otomatis, simpan gambar di folder publikasi atau di dalam folder tersebut.

Mencetak, menyimpan PDF, dan menyimpan e-book menggunakan apa yang ditampilkan halaman. Jika gambar berstatus **Belum diperbarui**, klik **Perbarui Tautan** sebelum mencetak. [Pemeriksa Desain](design-checker) mencantumkan gambar-gambar ini.

## Memperbaiki gambar yang hilang

1. Pilih gambar **Hilang** dalam daftar.
2. Klik **Ubah Tautan…** lalu pilih berkasnya. Atau kembalikan berkas ke tempat semula, lalu klik **Perbarui Tautan**.
3. Jika berkas benar-benar hilang, klik **Sematkan Gambar** untuk mempertahankan pratinjau, atau **Ganti…** untuk memakai gambar lain.

[Pemeriksa Desain](design-checker) juga mencantumkan gambar tertaut yang hilang.

## Terkait

- [Tambahkan dan atur gambar](pictures)
- [Periksa desain Anda](design-checker)
- [Simpan sebagai gambar, halaman web, atau e-book](export)
