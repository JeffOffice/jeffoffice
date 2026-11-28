# Keamanan: apa yang boleh dijangkau berkas

<!-- keywords: pusat kepercayaan, keamanan, makro, berkas tertaut, tautan, gambar tertaut, media tertaut, internet, diunduh, hanya baca, dilindungi, tautan eksternal, program, aman -->

Sebuah berkas dapat menunjuk ke hal-hal lain: berkas, halaman web, film, bahkan program. Halaman **Keamanan** adalah tempat Anda menentukan apa yang dilakukan JeffCrunch terhadap hal itu. Klik **Berkas > Opsi**, lalu tab **Keamanan**. Klik **OK** untuk menyimpan perubahan, atau **Batal** untuk membuangnya.

Pilihan ini berlaku untuk semua aplikasi JeffOffice: ubah satu pilihan di satu aplikasi, dan pilihan itu ikut berubah di semua aplikasi. Baris untuk sesuatu yang tidak dimiliki sebuah aplikasi bertuliskan **Tidak berlaku** dan berwarna abu-abu.

## Makro

Makro adalah program kecil yang disimpan di dalam berkas. JeffCrunch menyimpannya di dalam berkas, sehingga makro tetap ada saat Anda menyimpan, dan JeffCrunch **tidak pernah menjalankannya**. Tidak ada pengaturan yang menjalankannya.

## Berkas tertaut dan tautan buku kerja

Bagian ini membahas berkas lain yang dibaca sebuah buku kerja, seperti buku kerja tempat ia mengambil angka, yang disebut **tautan**, berkas yang dibaca kueri-kuerinya, dan berkas tempat tabel pivotnya dibuat. Pilihan ini menentukan apa yang dilakukan JeffCrunch saat membuka buku kerja yang membaca berkas-berkas tersebut, dan saat Anda menggunakan **Segarkan** atau **Segarkan Semua** pada kueri dan tabel pivotnya.

- **Tanyakan dulu** adalah pilihan yang biasa. Sebelum JeffCrunch membaca salah satu berkas tersebut, ia menampilkan jalur lengkap berkas itu dan bertanya. Klik **Baca Berkas** hanya jika Anda mempercayai asal buku kerja tersebut. **Jangan Baca**, tombol standarnya, tidak membaca apa pun dan membiarkan buku kerja seperti semula. Pilihan ini bertanya setiap kali Anda membuka buku kerja, lalu sekali untuk setiap berkas. Setelah Anda mengklik **Baca Berkas** untuk sebuah berkas, JeffCrunch tidak lagi bertanya tentang berkas yang sama itu di jendela tersebut, sampai Anda menutup jendelanya.
- **Perbarui** membaca berkas tanpa bertanya hanya jika Anda sudah pernah menyetujui buku kerja yang sama membaca berkas yang sama itu. Untuk setiap berkas lain, termasuk berkas yang disebut dalam buku kerja yang baru dikirim seseorang kepada Anda, JeffCrunch tetap bertanya terlebih dahulu dengan cara yang sama.
- **Jangan pernah perbarui** tidak membaca apa pun saat buku kerja dibuka, dan tidak menampilkan pertanyaan. Angka yang disimpan berkas tetap seperti semula. Saat Anda mengklik **Segarkan** sendiri, itu merupakan permintaan, sehingga JeffCrunch menampilkan pertanyaan pada saat itu. Setelah Anda mengklik **Baca Berkas**, JeffCrunch tidak lagi bertanya tentang berkas yang sama itu di jendela tersebut.

## Yang diingat

Saat Anda mengklik **Baca Berkas** atau **Baca Berkas-Berkas**, JeffCrunch mengingat pasangan tersebut: buku kerja ini, dan berkas itu, masing-masing dengan jalur lengkapnya. Hanya pasangannya yang diingat. Jika buku kerja dipindahkan atau diganti namanya, diedit untuk membaca berkas lain, atau berkas diganti dengan pintasan ke berkas lain, JeffCrunch bertanya lagi. Buku kerja yang belum pernah Anda simpan tidak memiliki jalur, sehingga tidak ada yang diingat tentangnya setelah Anda menutupnya.

Pasangan-pasangan itu hanya disimpan dalam pengaturan Anda sendiri di komputer ini, tidak pernah di dalam buku kerja, sehingga sebuah buku kerja tidak dapat menambahkannya, dan siapa pun yang menerima buku kerja dari Anda tidak mendapatkannya. Pengaturan menyimpan bentuk jalur yang sudah diacak, bukan jalur itu sendiri. Hingga 2.000 pasangan disimpan; jika lebih, pasangan yang paling lama tidak digunakan akan dilupakan lebih dulu.

**Opsi > Keamanan** menampilkan berapa banyak pasangan yang diingat (**Persetujuan yang diingat**). Klik **Lupakan Semua** untuk melupakan semuanya sekaligus. Tidak ada buku kerja yang diubah, dan setiap buku kerja akan bertanya lagi saat dibuka berikutnya.

Buku kerja yang menyatakan "jangan pernah" pada **Permintaan saat Mulai** miliknya sendiri (**Data > Edit Tautan**) selalu dibiarkan. Buku kerja di komputer lain tidak pernah dibaca saat berkas dibuka, apa pun pilihan Anda. Anda tetap dapat menggunakan **Perbarui Nilai** di **Edit Tautan** kapan saja.

## Media tertaut: film dan suara

Bagian ini tidak berlaku di JeffCrunch: ia tidak memiliki film atau suara tertaut untuk diputar. Barisnya berwarna abu-abu.

## Berkas dari internet atau komputer lain

Berkas yang Anda unduh, atau yang berada di komputer lain dalam jaringan Anda, mungkin tidak aman. Pilihan ini menentukan cara JeffCrunch membukanya.

- **Buka sebagai hanya baca terlebih dahulu** adalah pilihan yang biasa. Berkas dibuka agar Anda dapat membacanya, tetapi tidak dapat diubah. Sebuah bilah di bawah berkas menyatakan hal itu. Klik **Aktifkan Pengeditan** pada bilah tersebut untuk mengubah berkas itu saja (sampai Anda menutup program). Saat Anda menyimpannya di tempat yang sama setelahnya, tanda unduhan dihapus dari berkas. Atau gunakan **Simpan Sebagai** untuk membuat salinan, atau ubah pengaturan ini ke pilihan berikutnya.
- **Buka secara normal** membuka setiap berkas dengan cara yang sama.

Berkas yang Anda buka dengan **Buka Salinan**, **Buka dan Perbaiki**, atau **Buka Hanya-Baca** dari layar **Berkas** akan dibuka dengan cara tersebut, apa pun pengaturan ini.

## Tautan yang memulai program

Tautan yang akan memulai program atau skrip (berkas berakhiran .exe, .bat, .sh, dan sebagainya) **selalu ditolak**. JeffCrunch memberi tahu Anda, dan tidak ada yang dijalankan. Tautan ke berkas di komputer lain tidak pernah diikuti tanpa bertanya kepada Anda terlebih dahulu. Tidak ada pengaturan yang mengubah hal ini.

## Terkait

- [Ubah pengaturan JeffCrunch](options)
- [Catatan dan tautan](notes-links)
