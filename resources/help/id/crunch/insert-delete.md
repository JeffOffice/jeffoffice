# Menyisipkan dan menghapus baris, kolom, dan sel
<!-- keywords: sisipkan sel yang dipotong, sisipkan sel yang disalin, geser ke kanan, geser ke bawah, sisipkan, hapus, buang, baris, kolom, sel, geser, pindahkan, tambah, kosong, Ctrl+plus, Ctrl+minus, klik kanan, header, urungkan, referensi, #REF!, rumus, nama, gabungan, tabel, array, tepi -->

Anda dapat menambahkan baris, kolom, atau sel kosong di tengah data Anda, dan menghapus baris, kolom, atau sel. Semua yang ada di bawah atau di sampingnya akan bergeser untuk memberi ruang atau menutup celah, dan rumus yang merujuk ke sel yang berpindah tetap merujuk ke sel tersebut.

## Menyisipkan

1. Pilih nomor baris, huruf kolom, atau sel tempat Anda ingin menambahkan yang baru. Untuk menambahkan tiga baris, pilih tiga baris.
2. Tekan **Ctrl dan tombol plus** (**Ctrl dan tombol minus** untuk menghapus), atau gunakan **Beranda > Sisipkan**, atau klik kanan pada pilihan lalu pilih **Sisipkan**.

Jika yang dipilih adalah seluruh baris atau kolom, baris atau kolom baru langsung disisipkan sekaligus, di atas baris atau di sebelah kiri kolom yang dipilih. Jika yang dipilih adalah sel, sebuah jendela kecil akan menanyakan bagaimana sel-sel setelahnya harus bergeser:

- **Geser sel ke kanan** atau **Geser sel ke bawah** hanya memindahkan sel di kolom (atau baris) pilihan Anda, dan membiarkan bagian lain lembar tetap di tempatnya.
- **Seluruh baris** atau **Seluruh kolom** menambahkan seluruh baris atau kolom.

## Menyisipkan sel yang dipotong atau disalin

Untuk menempatkan sel yang Anda potong atau salin di antara sel lain, bukan menimpanya, potong atau salin dahulu (**Ctrl+X** atau **Ctrl+C**). Kemudian pilih sel tempat sel tersebut harus diletakkan, dan gunakan **Beranda > Sisipkan > Sisipkan Sel yang Dipotong atau Disalin** (perintah ini berbunyi **Sisipkan Sel yang Dipotong** atau **Sisipkan Sel yang Disalin** setelah ada yang bisa ditempatkan), atau klik kanan pada sel dan pilih perintah tersebut. Sebuah jendela kecil akan menanyakan apakah sel yang ada di sana harus **digeser ke kanan** atau **digeser ke bawah** untuk memberi ruang.

Tombol **Sisipkan** dan **Ctrl dan tombol plus** (juga **Ctrl+Shift dan tombol plus**) juga melakukan hal ini, tetapi hanya ketika maksud Anda sudah jelas:

- ketika sel **dipotong** dan pemotongan masih menunggu (bingkai putus-putusnya terlihat) pada lembar tempat sel itu berasal, dan
- ketika sel **disalin** dan Anda belum mengubah apa pun sejak itu (tidak ada pengetikan, pengeditan lain, atau Urungkan maupun Ulangi).

Dalam kasus lain, tombol dan tombol pintas tetap menyisipkan sel kosong seperti biasa. Setelah Anda mengubah sesuatu, salinan tetap tersedia untuk **Tempel** dan untuk perintah di menu. **Esc** mengakhiri pemotongan atau penyalinan: bingkainya hilang, dan tombol serta tombol pintas kembali menyisipkan sel kosong. Jika sel dipotong pada satu lembar dan Anda menekan tombol pada lembar lain, sel kosong akan disisipkan; perintah di menu akan memberi tahu bahwa sel yang dipotong hanya dapat diletakkan di antara sel lain pada lembar asalnya. Jika yang dipilih adalah seluruh baris atau kolom, perintah ini tidak aktif, dan Sisipkan menambahkan baris atau kolom seperti sebelumnya.

- Sel yang dipotong **dipindahkan**: sel meninggalkan tempat lamanya, dan apa pun yang merujuk ke sel itu ikut berpindah. Hasilnya sama seperti menahan **Shift** sambil menyeret bingkai pilihan, dan semuanya terjadi atau tidak sama sekali: jika ada bagian yang ditolak, tidak ada yang berubah.
- Sel yang disalin **ditempatkan** sebagai salinan, seperti yang akan dilakukan **Tempel** (sebagaimana tertulis di **Atur Tempel Default**, dengan rumus yang disesuaikan untuk tempat barunya), dan salinan tetap siap sehingga Anda dapat menempatkannya lagi di tempat lain.
- Sel dimasukkan sekali, di sel kiri atas dari yang Anda pilih, meskipun Anda memilih lebih banyak sel daripada yang Anda salin.
- Jika daftar difilter, sel akan dimasukkan ke baris yang tampak, seperti saat menempel, dan ruang yang dibuat juga mencakup baris yang disembunyikan filter di antaranya. Baris yang Anda sembunyikan secara manual dihitung seperti baris lainnya.
- Setiap tindakan ini adalah satu langkah yang dapat Anda urungkan. Sel yang dipotong hanya dapat disisipkan di antara sel lain pada lembar tempat asalnya; untuk memindahkannya ke lembar lain, gunakan **Tempel**.
- Sel tidak dapat disisipkan ke dalam jalurnya sendiri (sel yang dipotong dari A3:A4 tidak dapat diletakkan di antara A3 dan A4), dan lembar yang dilindungi akan menolaknya. JeffCrunch akan menjelaskan alasannya dan tidak mengubah apa pun.

## Menghapus

Pilih yang ingin Anda hapus, lalu tekan **Ctrl dan tombol minus**, atau gunakan **Beranda > Hapus**. Jika yang dipilih adalah sel, jendela akan menanyakan apakah akan **menggeser sel ke kiri**, **menggeser sel ke atas**, atau menghapus **seluruh baris** atau **seluruh kolom**. Menghapus dihitung sebagai satu langkah, dan **Urungkan** akan mengembalikan semua yang terhapus, termasuk rumusnya.

## Yang terjadi kemudian

Rumus yang merujuk ke sel yang berpindah akan merujuk ke sel tersebut di tempat barunya. Contoh: di B1 Anda punya **=SUM(A1:A5)**. Jika Anda menyisipkan baris di atas baris 3, rumusnya menjadi **=SUM(A1:A6)**, karena rentangnya bertambah untuk memuat baris baru. Jika Anda menghapus baris 3, rumusnya menjadi **=SUM(A1:A4)**. Rumus yang merujuk ke sel yang Anda hapus menampilkan **#REF!**, yang berarti sel yang dibutuhkannya sudah tidak ada. **Urungkan** akan mengembalikan sel dan rumus tersebut.

Hal-hal berikut juga mengikuti sel: rumus di setiap lembar, nama (seperti area cetak dan judul cetak), sel gabungan, sel yang menjadi sasaran aturan validasi dan format bersyarat, tabel, bagan, gambar, catatan, hiperlink, dan pemisah halaman.

## Yang tidak dapat dilakukan

- Sel tidak dapat disisipkan atau dihapus di sebagian **rumus array** atau tabel data. Pindahkan secara utuh, atau biarkan saja. Sel yang dibaca tabel data sebagai masukannya juga tidak dapat dihapus.
- **Tabel pivot** belum dapat dipindahkan atau dipotong: baris, kolom, atau sel tidak dapat disisipkan atau dihapus jika itu akan memindahkan atau mengubahnya, dan sel tidak dapat dipotong dari atau ke dalamnya. Bagian lain lembar tetap dapat diubah seperti biasa. Tabel pivot diubah dengan perintah pada tab **Tabel Pivot** (segarkan, ubah sumber data, bidang, dan tata letak), bukan dengan mengetik di selnya.
- **Tabel** tidak dapat dipotong melintang: Anda tidak dapat menyisipkan kolom di dalamnya, dan Anda tidak dapat menghapus baris header atau semua barisnya. Sisipkan atau hapus baris di dalamnya, atau ubah sel di sekitarnya.
- **Lembar yang dilindungi** tidak dapat disisipi atau dihapus baris, kolom, atau selnya. Untuk melakukannya, pilih Hapus Perlindungan Lembar pada tab Tinjau.
- Sel di ujung lembar yang berisi sesuatu tidak dapat didorong keluar dari tepi. Hapus dulu sel tersebut.
- Setelah Anda menyimpan, **Urungkan** tetap dapat membatalkan apa yang Anda lakukan sebelumnya, termasuk baris dan kolom yang Anda sisipkan (pada penyimpanan berikutnya, baris dan kolom tersebut dihapus lagi). Satu hal yang tidak dapat dikembalikan adalah baris, kolom, atau sel yang **Anda hapus**: berkas yang tersimpan sudah kehilangan isi di dalamnya, sehingga **Urungkan** berhenti di penghapusan terakhir yang telah Anda simpan, dan baris di bagian bawah jendela akan menyebutkan hal itu saat Anda menyimpan.
- Langkah yang memindahkan atau menghapus begitu banyak sel sehingga **Urungkan** tidak dapat menyimpannya (lebih dari sekitar 128 MB) akan menanyakan terlebih dahulu, seperti yang dilakukan program lembar kerja yang sudah dikenal: "Ini tidak dapat diurungkan." Jika Anda melanjutkan, langkah itu dilakukan dan **Urungkan** tidak akan kembali ke sana atau ke apa pun yang terjadi sebelumnya.
