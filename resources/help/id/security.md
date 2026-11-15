# Keamanan: apa yang dapat dijangkau berkas

<!-- keywords: pusat kepercayaan, trust center, keamanan, makro, berkas tertaut, tautan, gambar tertaut, media tertaut, internet, diunduh, hanya baca, dilindungi, tautan eksternal, program, aman -->

Sebuah berkas dapat menunjuk ke hal lain: berkas, halaman web, film, bahkan program. Halaman **Keamanan** adalah tempat Anda memutuskan apa yang dilakukan JeffPub terhadap hal tersebut. Klik **Berkas > Opsi**, lalu tab **Keamanan**. Klik **OK** untuk menyimpan perubahan, atau **Batal** untuk membuangnya.

Pilihan ini berlaku untuk semua aplikasi JeffOffice: jika Anda mengubah satu pilihan di satu aplikasi, pilihan itu ikut berubah di semua aplikasi. Baris untuk sesuatu yang tidak dimiliki aplikasi akan menampilkan **Tidak berlaku** dan berwarna abu-abu.

## Makro

Makro adalah program kecil yang disimpan di dalam berkas. JeffPub menyimpannya di dalam berkas, sehingga makro tetap ada saat Anda menyimpan, dan JeffPub **tidak pernah menjalankannya**. Tidak ada pengaturan yang menjalankannya.

## Berkas tertaut dan tautan buku kerja

Bagian ini membahas gambar yang **ditautkan** ke sebuah berkas, bukan disimpan di dalam publikasi. Pilihan Anda menentukan apa yang dilakukan JeffPub saat membuka publikasi yang memiliki gambar seperti itu.

- **Perbarui** adalah pilihan yang umum. Saat publikasi dibuka, JeffPub membaca ulang berkas setiap gambar tertaut jika berkas tersebut berada di folder publikasi, sehingga gambar tetap mutakhir. JeffPub tidak pernah menanyakannya.
- **Jangan pernah perbarui** membiarkan setiap gambar seperti saat disimpan. Setiap gambar akan menampilkan **Belum diperbarui** sampai Anda menggunakan **Perbarui Tautan** padanya. **Ekspor sebagai PDF** dan **Cetak** kemudian akan memberi tahu Anda di bilah status bahwa pratinjau digunakan untuk gambar yang tidak memiliki salinan utuh di dalam publikasi.

JeffPub hanya membaca berkas yang berada di folder publikasi (atau folder yang Anda izinkan). Berkas tertaut lainnya menunggu sampai Anda memintanya.

## Film dan suara tertaut

Bagian ini tidak berlaku di JeffPub, karena JeffPub tidak memiliki film atau suara tertaut untuk diputar. Baris ini berwarna abu-abu.

## Berkas dari internet atau komputer lain

Berkas yang Anda unduh, atau yang berada di komputer lain dalam jaringan Anda, mungkin tidak aman. Pilihan ini menentukan cara JeffPub membukanya.

- **Buka sebagai hanya-baca dulu** adalah pilihan yang umum. Berkas dibuka agar Anda dapat membacanya, tetapi tidak dapat diubah. Sebuah bilah di bawah berkas akan menyatakan hal itu. Klik **Aktifkan Pengeditan** pada bilah tersebut untuk mengubah berkas itu saja (sampai Anda menutup program). Saat Anda menyimpannya di tempat yang sama setelah itu, JeffPub menghapus tanda unduhan dari berkas. Atau gunakan **Simpan Sebagai** untuk membuat salinan, atau ubah pengaturan ini ke pilihan berikutnya.
- **Buka secara normal** membuka setiap berkas dengan cara yang sama.

Berkas yang Anda buka dengan **Buka sebagai Salinan**, **Buka dan Perbaiki**, atau **Buka Hanya Baca** dari layar **Berkas** akan terbuka dengan cara tersebut, apa pun pengaturan ini.

## Tautan yang menjalankan program

Tautan yang akan menjalankan program atau skrip (berkas berakhiran .exe, .bat, .sh, dan sebagainya) **selalu ditolak**. JeffPub memberi tahu Anda, dan tidak ada yang dijalankan. Tautan ke berkas di komputer lain tidak pernah dibuka tanpa bertanya kepada Anda terlebih dahulu. Tidak ada pengaturan yang mengubah hal ini.

## Terkait

- [Ubah pengaturan JeffPub](options)
- [Gambar](pictures)
