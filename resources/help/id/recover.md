# Dapatkan kembali pekerjaan yang belum disimpan
<!-- keywords: pulihkan, pemulihan, pemulihan otomatis, pemulihan otomatis, simpan otomatis, crash, pekerjaan hilang, belum disimpan, mati listrik, pulihkan, beku, tertutup tanpa sengaja, cadangan -->

Jika JeffPub tertutup secara tiba-tiba, atau listrik padam, pekerjaan terakhir Anda mungkin masih aman. Selama Anda bekerja, JeffPub menyimpan salinan cadangan dari setiap publikasi yang memiliki perubahan belum Anda simpan. Fitur ini disebut **pemulihan otomatis**.

## Cara kerja pemulihan otomatis

- Setiap beberapa menit, JeffPub menyimpan salinan cadangan dari setiap publikasi yang memiliki perubahan belum disimpan. Nilai bawaannya adalah setiap 10 menit.
- JeffPub menyimpan salinan tersebut dalam folder pemulihan otomatis, di dalam folder tempat JeffPub menyimpan data internalnya. Anda tidak perlu membuka folder itu sendiri.
- Salinan cadangan tidak pernah menggantikan berkas asli Anda.
- Salinan akan dihapus saat Anda menyimpan publikasi, menutupnya, atau keluar dari JeffPub dengan cara biasa.

Pemulihan otomatis tidak menggantikan penyimpanan. Jika JeffPub berhenti, Anda dapat kehilangan pekerjaan dari beberapa menit terakhir. Tekan Ctrl+S sering-sering.

## Pulihkan pekerjaan Anda

Saat Anda menjalankan JeffPub lagi setelah mengalami kerusakan, jendela **Pulihkan Pekerjaan yang Belum Disimpan** akan terbuka dengan sendirinya. Setiap baris menampilkan nama publikasi, lokasi berkas disimpan (atau "belum pernah disimpan"), dan waktu salinan dibuat.

1. Beri tanda centang pada setiap publikasi yang ingin Anda pulihkan. Hapus tanda centang pada yang lain.
2. Klik **Buka**.
3. Periksa pekerjaan Anda, lalu tekan Ctrl+S.

Publikasi yang dipulihkan menampilkan "(Dipulihkan)" pada bilah judul hingga Anda menyimpannya. Ctrl+S akan meminta nama, dan dimulai di folder berkas asli, dengan nama asli. Pilih nama asli untuk menggantikan berkas, atau pilih nama baru untuk menyimpan keduanya.

Gambar yang ditautkan ke berkas ditemukan seperti jika publikasi dibuka di folder berkas asli: gambar yang berada di folder tersebut, atau di dalam folder di dalamnya, dimuat seperti biasa. Gambar yang ditautkan ke berkas di tempat lain akan menampilkan pratinjau yang tersimpan dan ditandai **Belum Diperbarui**; pilih gambar tersebut lalu klik **Perbarui Tautan** di [Pengelola Grafis](graphics-manager). Publikasi yang belum pernah disimpan tidak memiliki folder, sehingga semua gambar tertautnya menunggu **Perbarui Tautan**.

Tombol lain di jendela ini adalah:

- **Hapus** menghapus salinan yang dicentang secara permanen, setelah meminta konfirmasi.
- **Jangan Sekarang** menutup jendela dan membiarkan salinan tetap ada. JeffPub akan menawarkannya lagi saat dijalankan berikutnya.

JeffPub hanya menawarkan salinan dari sesi yang berakhir secara tiba-tiba. JeffPub tidak menawarkan salinan dari jendela JeffPub yang masih terbuka.

## Temukan pekerjaan yang dipulihkan nanti

Jika Anda sudah menutup jendela dan ingin mendapatkan kembali salinan Anda, klik **Berkas > Buka**, lalu **Pulihkan Pekerjaan yang Belum Disimpan**. Jika tidak ada yang dapat dipulihkan, JeffPub akan memberi tahu Anda.

## Ubah seberapa sering menyimpan

Klik **Berkas > Opsi**, lalu tab **Simpan**. Ubah **Simpan informasi pemulihan otomatis setiap** menjadi angka dari 1 hingga 120 menit. Semakin kecil angkanya, semakin sedikit pekerjaan yang dapat hilang. Waktu baru berlaku penuh saat Anda menjalankan JeffPub berikutnya.

## Terkait

- [Buka dan simpan publikasi](open-save)
- [Ubah pengaturan JeffPub](options)
- [Dapatkan bantuan dan laporkan masalah](support)
