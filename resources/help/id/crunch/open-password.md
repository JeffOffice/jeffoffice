# Membuka buku kerja yang dilindungi kata sandi
<!-- keywords: kata sandi, password, dilindungi, dienkripsi, enkripsi, buka, terkunci, kata sandi salah, batal, xlsx, xls, ods, perlindungan berkas, kata sandi bawaan, pribadi -->

Beberapa buku kerja dilindungi kata sandi saat disimpan: tanpa kata sandi tersebut, tidak ada program yang dapat membaca isi berkas. Ini berbeda dengan **Lindungi Lembar** dan **Lindungi Buku Kerja** (lihat [Melindungi lembar atau buku kerja](protect)), yang hanya mencegah perubahan. Buku kerja yang dilindungi kata sandi tidak dapat dibuka sama sekali sebelum kata sandinya diketik. Halaman ini menjelaskan cara membuka dan membuat buku kerja seperti itu.

## Membuka buku kerja

Buka berkas seperti Anda membuka berkas lain: tekan **{key:Open}**, pilih berkasnya, atau seret ke jendela. JeffCrunch mengenali bahwa berkas dilindungi dan meminta kata sandi sebelum membaca apa pun:

1. Ketik kata sandi di kotak yang bertuliskan **Berkas ini dilindungi kata sandi.** Huruf akan tampil sebagai titik.
2. Klik **Buka**.

Jika kata sandinya salah, JeffCrunch memberi tahu dan meminta lagi, berapa kali pun Anda mau. Klik **Batal** untuk berhenti: tidak ada yang dibuka, dan buku kerja yang sudah terbuka tetap seperti semula. Kata sandi harus tepat: huruf kapital, spasi, dan aksen ikut diperhitungkan.

JeffCrunch tidak menuliskan kata sandi ke mana pun. Kata sandi disimpan di memori selama buku kerja terbuka, agar **Simpan** dapat melindungi berkas lagi, dan dihapus saat Anda menutup buku kerja. Saat Anda membuka berkas itu lagi, JeffCrunch akan menanyakannya kembali.

## Yang dapat dibuka

- **Buku kerja .xlsx dan .xlsm** yang dilindungi dengan salah satu dari dua cara yang digunakan program asli sejak 2007: cara yang lebih baru (kuncinya 128, 192, atau 256 bit, dan JeffCrunch juga memeriksa bahwa tidak ada yang mengubah berkas setelah dilindungi) dan cara yang lebih lama (AES dengan kunci 128, 192, atau 256 bit).
- **Buku kerja .xls** yang dilindungi dengan salah satu dari tiga cara pada format itu: pengacakan sederhana pada berkas paling lama, jenis 40-bit, dan jenis yang lebih kuat yang menggunakan layanan kunci dari komputer.
- **Lembar sebar .ods** yang dilindungi kata sandi, dalam ketiga cara yang pernah ditulis LibreOffice dan OpenOffice: yang terbaru (LibreOffice 25 dan yang lebih baru mengunci seluruh berkas dengan kunci 256-bit, dan membuat kunci dari kata sandi dengan metode yang sengaja dibuat lambat serta memakai banyak memori, sehingga membuka berkas seperti ini memerlukan waktu sedikit lebih lama), jenis 256-bit pada versi sebelumnya, dan jenis paling lama.

Berkas .xls yang lebih lama memiliki batasan pada kata sandi itu sendiri. Pengacakan sederhana pada berkas paling lama hanya memakai 15 karakter pertama kata sandi dan hanya dapat memuat huruf, angka, serta tanda dari kumpulan karakter Eropa Barat (Latin-1): kata sandi dengan huruf lain (seperti Yunani atau Tionghoa) tidak pernah dianggap benar di sana. Jenis 40-bit menerima huruf apa pun, tetapi hanya 16 karakter pertama. JeffCrunch tidak menulis berkas .xls dengan kata sandi.

Beberapa berkas dikunci dengan kata sandi yang diketahui semua orang, agar dapat dibuka siapa saja tetapi tidak berubah secara tidak sengaja. JeffCrunch membuka berkas seperti itu tanpa bertanya.

## Jika berkas tidak dapat dibuka

- **"Kata sandi tidak benar."** Kata sandi yang Anda ketik bukan kata sandi yang dipakai untuk melindungi berkas. Coba lagi dengan memperhatikan huruf kapital dan spasi; JeffCrunch tidak dapat memulihkan kata sandi yang hilang.
- **"Berkas ini dilindungi dengan cara yang tidak dapat dibuka program ini."** Berkas dikunci oleh sesuatu selain kata sandi (misalnya kunci yang tersimpan dalam sertifikat), atau dalam bentuk yang tidak dapat dibaca JeffCrunch.
- **"Perlindungan kata sandi berkas ini memerlukan memori atau waktu lebih besar daripada yang diizinkan program ini, sehingga tidak dibuka."** Berkas dari LibreOffice 25 atau yang lebih baru menetapkan berapa banyak memori dan waktu yang dibutuhkan untuk memeriksa kata sandi. JeffCrunch menerima hingga sepuluh kali lipat dari yang ditetapkan LibreOffice sendiri (640 MiB memori), agar berkas yang dibuat untuk membebani komputer Anda tidak dapat dibuka. Berkas yang ditulis LibreOffice sendiri jauh berada di bawah batas itu.
- **"Berkas rusak, atau diubah setelah dilindungi."** Kata sandinya benar, tetapi isi berkas tidak sama dengan yang dikunci: berkas terpotong, rusak, atau diubah oleh program lain. Salin ulang dari sumber aslinya.

## Menyimpan dengan kata sandi

Klik **Berkas**, lalu **Lindungi dengan Kata Sandi** (atau **Tinjau > Lindungi dengan Kata Sandi**), ketik kata sandi dua kali, dan klik **Lindungi**. Buku kerja akan ditulis dalam keadaan terlindungi saat Anda menyimpannya berikutnya: siapa pun yang membuka berkas harus mengetik kata sandinya. Untuk mengganti kata sandi, pilih perintah yang sama dan ketik yang baru; untuk menghapusnya, klik **Hapus Kata Sandi**, lalu simpan.

Buku kerja yang Anda buka dengan kata sandi tetap menyimpannya selama terbuka, sehingga **Simpan** menulis berkas dalam keadaan terlindungi lagi dengan kata sandi yang sama. JeffCrunch hanya menyimpan kata sandi di memori sampai Anda menutup buku kerja, lalu menghapusnya. Berkas yang Anda buka dengan kata sandi tidak akan menjadi tidak terlindungi secara tidak sengaja: salinan yang disimpan sebagai berkas .ods atau .ots dilindungi dengan kata sandi yang sama, dengan cara yang dijelaskan standar OpenDocument, sehingga program lain yang mengikuti standar itu dapat membukanya dengan kata sandi tersebut. Salinan .xls atau teks tidak dapat dilindungi, sehingga JeffCrunch akan memberi tahu Anda lebih dulu bahwa salinan itu tidak akan dilindungi, dan Anda dapat membatalkannya.

Gunakan kata sandi yang panjang, dan simpan di tempat yang aman: tidak seorang pun, termasuk kami, dapat membuka berkas tanpa kata sandi tersebut.
