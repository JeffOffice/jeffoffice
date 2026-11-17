# Membuka buku kerja yang dilindungi kata sandi

<!-- keywords: kata sandi, sandi, dilindungi, terenkripsi, enkripsi, buka, terkunci, kata sandi salah, batal, xlsx, xls, ods, proteksi berkas, kata sandi bawaan, pribadi -->

Beberapa buku kerja dilindungi kata sandi saat disimpan: tanpa kata sandi itu, tidak ada program yang dapat membaca isi berkasnya. Ini berbeda dengan **Lindungi Lembar** dan **Lindungi Buku Kerja** (lihat [Melindungi lembar atau buku kerja](protect)), yang hanya mencegah perubahan. Buku kerja yang dilindungi kata sandi tidak dapat dibuka sama sekali sebelum kata sandinya diketik. Halaman ini menjelaskan cara membuka dan cara membuat buku kerja seperti itu.

## Membuka buku kerja

Buka berkas seperti Anda membuka berkas lain: tekan **{key:Open}**, pilih berkasnya, atau seret ke jendela. JeffCrunch mengetahui bahwa berkas itu dilindungi dan meminta kata sandi sebelum membaca apa pun:

1. Ketik kata sandi di kotak yang bertuliskan **This file is protected with a password.** Huruf akan tampil sebagai titik.
2. Klik **Open**.

Jika kata sandinya tidak benar, JeffCrunch memberi tahu Anda dan meminta lagi, berapa kali pun Anda mau. Klik **Cancel** untuk berhenti: tidak ada yang dibuka, dan buku kerja yang sudah terbuka tetap seperti semula. Kata sandi harus persis sama: huruf besar, spasi, dan aksen ikut diperhitungkan.

JeffCrunch tidak menuliskan kata sandi ke mana pun. Kata sandi disimpan di memori selama buku kerja terbuka, agar **Save** dapat melindungi berkas itu lagi, dan dihapus saat Anda menutup buku kerja. Saat Anda membuka berkas itu lagi, JeffCrunch akan menanyakannya kembali.

## Yang dapat dibuka

- **Buku kerja .xlsx dan .xlsm** yang dilindungi dengan salah satu dari dua cara yang digunakan program asli sejak 2007: cara yang lebih baru (kuncinya 128, 192, atau 256 bit, dan JeffCrunch juga memeriksa bahwa tidak ada yang mengubah berkas setelah dilindungi) dan cara yang lebih lama (AES dengan kunci 128, 192, atau 256 bit).
- **Buku kerja .xls** yang dilindungi dengan salah satu dari tiga cara pada format itu: pengacakan sederhana pada berkas paling lama, cara 40-bit, dan cara yang lebih kuat yang menggunakan layanan kunci bawaan komputer.
- **Lembar sebar .ods** yang dilindungi kata sandi, dalam ketiga cara yang pernah ditulis LibreOffice dan OpenOffice: yang terbaru (LibreOffice 25 dan yang lebih baru mengunci seluruh berkas dengan kunci 256-bit, dan membuat kunci dari kata sandi dengan metode yang sengaja dibuat lambat serta memakai banyak memori, sehingga membuka berkas seperti ini memerlukan waktu sedikit lebih lama), cara 256-bit pada versi sebelumnya, dan cara yang paling lama.

Berkas .xls yang lebih lama memiliki batasan pada kata sandinya sendiri. Pengacakan sederhana pada berkas paling lama hanya memakai 15 karakter pertama dari kata sandi dan hanya dapat memuat huruf, angka, dan tanda dari kumpulan karakter Eropa Barat (Latin-1): kata sandi dengan huruf lain (misalnya Yunani atau Mandarin) tidak pernah diterima sebagai kata sandi yang benar pada cara ini. Cara 40-bit menerima huruf apa pun, tetapi hanya 16 karakter pertama. JeffCrunch menulis salinan .xls dengan kata sandi dalam cara yang lebih kuat (yang menggunakan layanan kunci bawaan komputer, dengan kunci 128-bit) karena dua cara lainnya lebih lemah. Cara ini memakai 255 karakter pertama dari kata sandi, yang boleh berupa huruf apa pun.

Beberapa berkas dikunci dengan kata sandi yang diketahui semua orang, supaya dapat dibuka siapa pun tetapi tidak berubah secara tidak sengaja. JeffCrunch membuka berkas seperti itu tanpa bertanya.

## Saat berkas tidak dapat dibuka

- **"The password is not correct."** Kata sandi yang Anda ketik bukan kata sandi yang digunakan untuk melindungi berkas. Coba lagi dengan memperhatikan huruf besar dan spasi; JeffCrunch tidak dapat memulihkan kata sandi yang hilang.
- **"This file is protected in a way this program can't open."** Berkas dikunci dengan cara selain kata sandi (misalnya kunci yang tersimpan dalam sertifikat), atau dalam bentuk yang tidak dibaca JeffCrunch.
- **"This file's password protection asks for more memory or time than this program allows, so it is not opened."** Berkas dari LibreOffice 25 atau yang lebih baru menentukan seberapa banyak memori dan waktu yang dipakai untuk memeriksa kata sandi. JeffCrunch menerima hingga sepuluh kali lipat dari yang ditetapkan LibreOffice sendiri (640 MiB memori), agar berkas yang dibuat untuk menyita komputer Anda tidak dapat membebani sistem. Berkas yang ditulis sendiri oleh LibreOffice berada jauh di dalam batas itu.
- **"The file is damaged, or it was changed after it was protected."** Kata sandinya benar, tetapi isi berkas tidak sama dengan yang dikunci: berkas terpotong, rusak, atau diubah program lain. Salin ulang dari berkas aslinya.

## Menyimpan dengan kata sandi

Klik **File**, lalu **Protect with Password** (atau **Review > Protect with Password**), ketik kata sandi dua kali, dan klik **Protect**. Buku kerja akan ditulis dalam keadaan dilindungi saat Anda menyimpannya berikutnya: siapa pun yang membuka berkas harus mengetik kata sandinya. Untuk mengganti kata sandi, pilih perintah yang sama dan ketik kata sandi baru; untuk menghapusnya, klik **Remove Password**, lalu simpan.

Buku kerja yang Anda buka dengan kata sandi tetap memilikinya selama terbuka, sehingga **Save** menulis berkas itu dalam keadaan dilindungi lagi dengan kata sandi yang sama. JeffCrunch menyimpan kata sandi di memori hanya sampai Anda menutup buku kerja, lalu menghapusnya. Berkas yang Anda buka dengan kata sandi tidak akan kehilangan perlindungannya secara tidak sengaja: salinan yang disimpan sebagai berkas .xls dilindungi dengan kata sandi yang sama dalam cara .xls yang lebih kuat, dan salinan yang disimpan sebagai .ods atau .ots dilindungi dengan cara yang dijelaskan standar OpenDocument, sehingga program lain yang mengikuti standar tersebut dapat membuka salinan itu dengan kata sandi. Salinan berupa teks tidak dapat dilindungi, jadi JeffCrunch akan memberi tahu Anda lebih dulu bahwa salinan itu tidak akan dilindungi, dan Anda dapat membatalkannya.

Gunakan kata sandi yang panjang, dan simpan di tempat yang aman: tidak ada seorang pun, termasuk kami, yang dapat membuka berkas tanpa kata sandi itu.
