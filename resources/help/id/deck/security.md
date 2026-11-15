# Keamanan: apa yang boleh diakses sebuah berkas
<!-- keywords: pusat kepercayaan, keamanan, makro, berkas tertaut, tautan, gambar tertaut, media tertaut, internet, diunduh, hanya-baca, dilindungi, tautan eksternal, program, aman -->

Sebuah berkas dapat menunjuk ke hal lain: berkas, halaman web, film, bahkan program. Halaman **Keamanan** adalah tempat Anda menentukan apa yang dilakukan JeffDeck terhadap hal tersebut. Klik **Berkas > Opsi**, lalu tab **Keamanan**. Klik **OK** untuk menyimpan perubahan, atau **Batal** untuk membuangnya.

Pilihan ini berlaku untuk semua aplikasi JeffOffice: ubah satu pilihan di satu aplikasi, maka perubahannya juga berlaku di semua aplikasi. Baris untuk sesuatu yang tidak dimiliki sebuah aplikasi bertuliskan **Tidak berlaku** dan berwarna abu-abu.

## Makro

Makro adalah program kecil yang disimpan di dalam berkas. JeffDeck menyimpannya di dalam berkas, sehingga makro tetap ada saat Anda menyimpan, dan JeffDeck **tidak pernah menjalankannya**. Tidak ada pengaturan yang menjalankannya.

## Berkas tertaut dan tautan buku kerja

Bagian ini tidak berlaku di JeffDeck karena JeffDeck tidak memiliki berkas tertaut untuk diperbarui. Baris ini berwarna abu-abu.

## Film dan suara tertaut

Bagian ini berkaitan dengan film atau suara yang ditautkan slaid, bukan yang disimpan di dalam presentasi.

- **Jangan pernah putar** adalah pilihan yang umum. Film atau suara yang tertaut menyatakan bahwa ia tertaut dan tidak diputar.
- **Tanyakan dulu** meminta izin sekali untuk setiap berkas tertaut saat pertama kali diputar.
- **Putar** memutarnya tanpa bertanya.

Bahkan dengan **Putar**, hanya berkas film atau suara yang namanya ada di komputer ini yang akan diputar. Alamat web, berkas di komputer lain, program, tautan atau berkas khusus, dan berkas yang bukan film atau suara sungguhan tidak pernah diputar. Berkas tertaut dapat berukuran hingga 256 MB. Berkas dibaca di latar belakang selama paling lama 20 detik, sehingga drive yang lambat tidak dapat membuat JeffDeck membeku. Drive jaringan yang tampak seperti drive lokal (drive yang dipetakan, atau folder di bawah /Volumes atau /mnt) dibaca seperti berkas lokal, sehingga bisa lambat. Dengan **Tanyakan dulu**, presentasi slaid meminta izin satu kali, saat dimulai, untuk semua berkas tertaut di dalamnya.

## Berkas dari internet atau komputer lain

Berkas yang Anda unduh, atau yang berada di komputer lain dalam jaringan Anda, mungkin tidak aman. Pilihan ini menentukan cara JeffDeck membukanya.

- **Buka sebagai hanya-baca dulu** adalah pilihan yang umum. Berkas dibuka agar Anda dapat membacanya, tetapi tidak dapat diubah. Sebuah bilah di bawah berkas menyatakannya. Klik **Aktifkan Pengeditan** pada bilah tersebut untuk mengubah berkas itu saja (sampai Anda menutup program). Saat Anda menyimpannya di tempat yang sama setelah itu, tanda unduhan dihapus dari berkas. Atau gunakan **Simpan Sebagai** untuk membuat salinan, atau ubah pengaturan ini ke pilihan berikutnya.
- **Buka normal** membuka setiap berkas dengan cara yang sama.

Berkas yang Anda buka dengan **Buka sebagai Salinan**, **Buka dan Perbaiki**, atau **Buka Hanya-Baca** dari layar **Berkas** akan dibuka dengan cara tersebut, apa pun pengaturan ini.

## Tautan yang menjalankan program

Tautan yang akan menjalankan program atau skrip (berkas yang berakhiran .exe, .bat, .sh, dan sebagainya) **selalu ditolak**. JeffDeck memberi tahu Anda, dan tidak ada yang dijalankan. Tautan ke berkas di komputer lain tidak pernah diikuti tanpa bertanya kepada Anda terlebih dahulu. Tidak ada pengaturan yang mengubah hal ini.

## Terkait

- [Ubah pengaturan JeffDeck](options)
- [Jalankan presentasi slaid](show)
