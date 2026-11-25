# Urutkan dan filter
<!-- keywords: urutkan, filter, saring, penyaringan otomatis, naik, turun, A-Z, aturan, header, judul, warna, daftar, bulan, hari kerja, huruf kapital, kolom, terbesar, terkecil, rata-rata, antara, diawali dengan, mengandung, tanggal, hapus, terapkan ulang, daftar turun, baris tersembunyi, tabel, Ctrl+Shift+L -->

## Urutkan

Pilih sel di dalam data Anda, lalu pilih **Urutkan A–Z** atau **Urutkan Z–A** (**Urutkan dan Pilih Baris** pada tab Beranda). JeffCrunch mengambil seluruh blok data di sekitar sel, mempertahankan baris judul jika menemukannya (teks di atas angka), lalu mengurutkan baris berdasarkan kolom tempat Anda berada. Jika Anda memilih beberapa sel, hanya sel tersebut yang diurutkan; jika ada data di sebelahnya, JeffCrunch akan menanyakan apakah data itu ikut diurutkan.

Dalam urutan, angka muncul lebih dulu, lalu teks, kemudian TRUE dan FALSE, lalu galat; sel kosong selalu berada di urutan paling akhir. Huruf besar dan huruf kecil dianggap sama, kecuali jika Anda memintanya berbeda. Teks diurutkan dengan cara yang sama di mana pun: menurut huruf dalam bahasa Inggris, dengan huruf beraksen ditempatkan bersama huruf polosnya, apa pun bahasa yang digunakan komputer.

Pilih **Urutkan…** untuk membuka kotak dialog Urutkan:

- Tambahkan aturan untuk setiap kolom yang akan diurutkan. Aturan yang lebih belakang memutuskan urutan jika nilai pada aturan sebelumnya sama. Gunakan **Pindahkan ke Atas** dan **Pindahkan ke Bawah** untuk mengubah urutan aturan, dan **Duplikat Aturan** untuk memulai dari satu aturan.
- Urutkan **Berdasarkan** nilai, **warna isian** sel, atau **warna teks**. Untuk warna, pilih apakah warna itu ditempatkan **Pertama** atau **Terakhir**; baris lain tetap pada urutannya. Warna dihitung dari warna yang dihasilkannya: warna yang dipilih dari palet dan warna dengan merah, hijau, dan biru yang sama dianggap sama.
- Untuk nilai, urutannya dapat **A–Z**, **Z–A**, atau daftar: nama hari dalam seminggu dan nama bulan (lengkap dan singkat, dalam bahasa Inggris dan bahasa Anda), atau daftar yang Anda ketik dengan koma di antara nilainya. Nilai yang tidak ada dalam daftar ditempatkan setelah nilai yang ada dalam daftar.
- **Baris pertama berisi nama kolom** membuat baris pertama tidak ikut diurutkan. **Huruf kapital dihitung** membuat huruf besar dan kecil berbeda (huruf kecil lebih dulu). **Urutkan kolom, bukan baris** mengurutkan kolom berdasarkan nilai pada suatu baris. **Angka dalam teks dihitung berdasarkan nilai** menempatkan "Item 2" sebelum "Item 10" (tanpa opsi ini, huruf dan angka dibandingkan satu per satu, sehingga "Item 10" tampil lebih dulu). Pilihan ini berlaku untuk pengurutan yang sedang Anda buat, dan jendela mengingatnya untuk pengurutan berikutnya serta untuk **Terapkan Ulang**; berkas tidak menyimpannya, sehingga **Terapkan Ulang** setelah Anda membuka kembali berkas akan mengurutkan dengan cara biasa. Pemeriksaan Kompatibilitas akan memberi tahu jika pengurutan yang tersimpan pada filter atau tabel dibuat dengan cara ini (pengurutan biasa berikutnya akan menghapus catatan tersebut). Angka dengan panjang berapa pun dihitung berdasarkan nilainya.

Rumus ikut berpindah bersama barisnya, dan rujukannya ke sel pada baris yang sama tetap berada di baris itu. Rujukan ke sel pada baris lain diubah seolah-olah rumus itu disalin. Apa pun yang terikat pada sel ikut berpindah bersamanya: format sel, catatannya, dan tautannya. Aturan yang berlaku untuk rentang sel (apa yang boleh dimasukkan, dan format bersyarat) tetap pada tempatnya, karena aturan itu berlaku untuk posisi, bukan untuk sel. Pengurutan dapat dibatalkan dalam satu langkah, dan **Esc** menghentikan pengurutan yang besar; pengurutan besar dikerjakan sedikit demi sedikit.

Beberapa sel tidak dapat diurutkan, dan Anda akan diberi tahu alasannya: sel yang terkunci pada lembar yang dilindungi, bagian dari array, sel gabungan yang tidak berukuran sama (atau yang membentang lebih dari satu baris), sel gabungan yang terpotong oleh tepi pilihan, dan baris header tabel.

## Filter

Pilih sel di dalam data Anda, lalu pilih **Filter** (**Ctrl+Shift+L**). Tombol muncul pada setiap sel di baris pertama. Klik salah satunya untuk melihat:

- **Urutkan A–Z** dan **Urutkan Z–A**, serta **Urutkan berdasarkan Warna**.
- Daftar nilai dalam kolom dengan kotak centang, kotak pencarian, dan **(Semua)**. Kolom berisi tanggal didaftarkan menurut tahun, bulan, dan hari, dan nilai yang bukan tanggal di dalamnya didaftarkan di bawahnya. Hapus centang pada yang tidak Anda inginkan, lalu pilih **OK**. Pencarian menampilkan nilai yang memuat semua kata Anda, lalu **(Semua yang Ditemukan)** mencentang atau menghapus centang hanya pada nilai tersebut. Daftar mencakup seluruh kolom. Jika kolom memiliki lebih banyak nilai berbeda daripada yang dapat ditampung daftar, JeffCrunch memberi tahu dan daftar dipotong; gunakan kondisi untuk sisanya.
- **Pertahankan Warna**, berdasarkan warna isian atau warna teks.
- **Kondisi Angka**, **Kondisi Teks**, atau **Kondisi Tanggal**: sama dengan, di atas, paling sedikit, di bawah, paling banyak, di antara, yang terbesar atau terkecil (item atau persen), di atas atau di bawah rata-rata, diawali dengan, diakhiri dengan, mengandung, tanggal dalam suatu periode (hari ini, minggu ini, bulan berikutnya, kuartal sebelumnya, sejauh tahun ini, satu kuartal atau satu bulan pada tahun mana pun), serta **Kondisi Sendiri** berupa dua perbandingan yang digabung dengan And atau Or. Dalam teks kondisi, ? dan \* berarti satu karakter apa pun dan rangkaian karakter apa pun.
- **Tampilkan Semua** pada kolom.

Tombol berbentuk corong menandakan kolom yang memiliki filter. Baris yang tidak cocok disembunyikan dan tetap menyimpan nomor barisnya. **Hapus** (pada menu **Urutkan dan Pilih Baris**) menghapus semua kondisi dan menampilkan baris; **Terapkan Ulang** menghitung ulang kondisi setelah Anda mengubah suatu nilai. **Filter** sekali lagi akan mematikan tombol-tombolnya.

Baris yang disembunyikan oleh filter dipisahkan dari baris yang Anda sembunyikan sendiri: **Tampilkan Baris** tidak menampilkan baris yang disembunyikan filter, dan menghapus filter tidak menampilkan baris yang Anda sembunyikan sendiri. Pengurutan blok yang difilter hanya mengurutkan baris yang tampil, di antara mereka sendiri.

Filter bekerja pada seluruh lembar, hingga 1.048.576 baris, sedikit demi sedikit; **Hapus** dan mematikan filter bekerja untuk ukuran berapa pun. Permintaan yang diajukan kepada filter dibatasi hingga 40 juta pemeriksaan (jumlah baris dikali jumlah kolom yang memiliki kondisi, ditambah sedikit untuk nilai yang didaftarkan kondisi); filter yang meminta lebih dari itu akan memberi tahu dan tidak dibuat, dan Anda dapat memfilter lebih sedikit baris atau memakai lebih sedikit kondisi. Filter lembar tidak mencakup sel dalam tabel: tabel memiliki tombolnya sendiri, yang bekerja dengan cara yang sama (lihat [Tabel](tables)).

Daftar nilai dalam kondisi disimpan seperti cara berkas menuliskannya, tanpa bahasa pada angka dan tanggal (1,234.50), dan sebuah nilai cocok dengan sel yang menampilkannya dalam bahasa Anda atau dengan cara itu; berkas yang difilter dalam satu bahasa akan memfilter sel yang sama dalam bahasa lain.

Filter disimpan di dalam berkas beserta kondisi dan baris yang disembunyikannya, sehingga berkas terbuka dengan baris yang sama tampil. Saat membuka kembali berkas, tombol corong tampil pada setiap kolom yang memiliki kondisi; baris yang disembunyikan dalam rentang filter dihitung sebagai milik filter.
