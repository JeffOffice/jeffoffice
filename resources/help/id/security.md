# Keamanan: sejauh mana berkas boleh menjangkau

<!-- keywords: pusat kepercayaan, keamanan, makro, berkas tertaut, tautan, gambar tertaut, media tertaut, internet, diunduh, hanya-baca, dilindungi, tautan eksternal, program, aman -->

Sebuah berkas dapat menunjuk ke hal lain: berkas, halaman web, film, bahkan program. Halaman **Keamanan** adalah tempat Anda menentukan apa yang dilakukan JeffPub terhadap hal itu. Klik **Berkas > Opsi**, lalu tab **Keamanan**. Klik **OK** untuk menyimpan perubahan, atau **Batal** untuk membuangnya.

Pilihan ini berlaku untuk semua aplikasi JeffOffice: ubah satu pilihan di satu aplikasi, dan perubahannya berlaku di semua aplikasi. Baris untuk sesuatu yang tidak dimiliki aplikasi akan menampilkan **Tidak berlaku** dan berwarna abu-abu.

## Makro

Makro adalah program kecil yang disimpan di dalam sebuah berkas. JeffPub **tidak pernah menjalankannya**, dan tidak ada pengaturan yang menjalankannya. JeffPub juga tidak menyimpan makro dari sebuah publikasi: saat Anda menyimpan berkas `.pub` yang telah dibuka, salinannya tidak memiliki makro, jadi simpan berkas asli jika Anda masih memerlukannya.

## Berkas tertaut dan tautan buku kerja

Bagian ini membahas gambar yang **ditautkan** ke sebuah berkas, bukan disimpan di dalam publikasi. Pilihannya adalah apa yang dilakukan JeffPub saat membuka publikasi yang memilikinya.

- **Perbarui** adalah pilihan yang umum. Saat publikasi dibuka, JeffPub membaca ulang berkas setiap gambar tertaut jika berkas itu berada di folder publikasi, sehingga gambarnya selalu mutakhir. JeffPub tidak akan bertanya.
- **Jangan pernah perbarui** membiarkan setiap gambar seperti saat disimpan. Setiap gambar menampilkan **Belum diperbarui** sampai Anda menggunakan **Perbarui Tautan** pada gambar tersebut. **Ekspor ke PDF** dan **Cetak** kemudian memberi tahu Anda di bilah status bahwa pratinjau digunakan untuk gambar yang tidak memiliki salinan utuh di dalam publikasi.

JeffPub hanya membaca berkas yang berada di folder publikasi (atau folder yang sudah Anda izinkan). Berkas tertaut lain menunggu sampai Anda memintanya.

## Media tertaut

Bagian ini tidak berlaku di JeffPub karena JeffPub tidak memiliki film atau suara tertaut untuk diputar. Baris ini berwarna abu-abu.

## Berkas dari internet atau komputer lain

Berkas yang Anda unduh, atau yang berada di komputer lain di jaringan Anda, mungkin tidak aman. Pilihan ini menentukan cara JeffPub membukanya.

- **Buka sebagai hanya-baca dulu** adalah pilihan yang umum. Berkas dibuka agar Anda dapat membacanya, tetapi tidak dapat diubah. Bilah di bawah berkas akan menyatakan hal itu. Klik **Aktifkan Pengeditan** pada bilah itu untuk mengubah berkas tersebut (hingga Anda menutup program). Saat Anda menyimpannya di tempat yang sama, JeffPub menghapus tanda unduhan dari berkas. Atau gunakan **Simpan Sebagai** untuk membuat salinan, atau ubah pengaturan ini ke pilihan berikutnya.
- **Buka normal** membuka setiap berkas dengan cara yang sama.

Berkas yang Anda buka dengan **Buka Salinan**, **Buka dan Perbaiki**, atau **Buka Hanya-Baca** dari layar **Berkas** akan dibuka dengan cara itu, apa pun pengaturan ini.

## Tautan yang menjalankan program

Tautan yang akan menjalankan program atau skrip (berkas berakhiran .exe, .bat, .sh, dan sebagainya) **selalu ditolak**. JeffPub memberi tahu Anda, dan tidak ada yang dijalankan. Tautan ke berkas di komputer lain tidak pernah dibuka tanpa menanyakan Anda terlebih dahulu. Tidak ada pengaturan yang mengubah hal ini.

## Lihat juga

- [Ubah pengaturan JeffPub](options)
- [Gambar](pictures)
