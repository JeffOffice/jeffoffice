# Urutkan dan filter
<!-- keywords: urutkan, sort, filter, saring, penyaring, filter otomatis, autofilter, naik, menaik, turun, menurun, A-Z, aturan, header, judul, warna, daftar, bulan, hari kerja, huruf kapital, kolom, terbesar, terkecil, rata-rata, di antara, diawali dengan, mengandung, tanggal, hapus, terapkan ulang, daftar turun-bawah, baris tersembunyi, tabel, Ctrl+Shift+L -->

## Urutkan

Pilih sel di dalam data Anda, lalu pilih **Urutkan A–Z** atau **Urutkan Z–A** (**Urutkan dan Pilih Baris** pada tab Beranda). JeffCrunch mengambil seluruh blok data di sekitar sel, mempertahankan baris judul jika menemukannya (teks di atas angka), lalu menyusun baris berdasarkan kolom tempat Anda berada. Jika beberapa sel dipilih, hanya sel-sel itu yang diurutkan; jika ada data di sebelahnya, JeffCrunch akan bertanya apakah data tersebut ikut dibawa.

Dalam urutan, angka tampil lebih dulu, lalu teks, lalu TRUE dan FALSE, lalu galat; sel kosong selalu ditempatkan paling akhir. Huruf kapital dan huruf kecil dianggap sama kecuali Anda memintanya berbeda. Teks diurutkan dengan cara yang sama di mana pun: menurut huruf-huruf dalam bahasa Inggris, dengan huruf beraksen ditempatkan bersama huruf polosnya, apa pun bahasa yang digunakan komputer.

Pilih **Urutkan…** untuk membuka kotak dialog Urutkan:

- Tambahkan aturan untuk setiap kolom yang akan diurutkan. Aturan yang lebih bawah memutuskan urutan jika nilai pada aturan di atasnya sama. Gunakan **Pindah ke Atas** dan **Pindah ke Bawah** untuk mengubah urutan aturan, dan **Duplikat Aturan** untuk memulai dari aturan yang sudah ada.
- Urutkan **Berdasarkan** nilai, **warna isian** sel, atau **warna teks**. Untuk warna, pilih apakah warna itu ditempatkan **Pertama** atau **Terakhir**; baris lain tetap mempertahankan urutannya. Sebuah warna dihitung dari warna yang dihasilkannya: warna yang dipilih dari palet dan warna dengan kadar merah, hijau, dan biru yang sama dianggap sama.
- Untuk nilai, urutannya dapat berupa **A–Z**, **Z–A**, atau daftar: nama hari dalam seminggu dan nama bulan (lengkap dan singkat, dalam bahasa Inggris dan bahasa Anda), atau daftar yang Anda ketik dengan koma di antara nilai-nilainya. Nilai yang tidak ada dalam daftar ditempatkan setelah nilai yang ada dalam daftar.
- **Baris pertama berisi nama kolom** membuat baris pertama tidak ikut diurutkan. **Huruf kapital dihitung** membuat huruf kapital dan huruf kecil berbeda (huruf kecil lebih dulu). **Urutkan kolom, bukan baris** menyusun kolom berdasarkan nilai pada suatu baris.

Rumus ikut bergerak bersama barisnya, dan rujukannya ke sel pada baris yang sama tetap berada di baris itu. Rujukan ke sel pada baris lain diubah seolah-olah rumus itu disalin. Apa pun yang terikat pada sel ikut berpindah bersamanya: format sel, catatannya, dan tautannya. Aturan yang berlaku untuk suatu rentang sel (apa yang boleh dimasukkan, dan format bersyarat) tetap pada tempatnya, karena aturan tersebut menutupi tempat, bukan sel. Pengurutan dapat dibatalkan dalam satu langkah, dan **Esc** menghentikan pengurutan yang besar; pengurutan besar dikerjakan sedikit demi sedikit.

Beberapa sel tidak dapat diurutkan, dan Anda akan diberi tahu alasannya: sel yang dikunci pada lembar yang dilindungi, bagian dari larik, sel gabungan yang tidak semuanya berukuran sama (atau yang mencakup lebih dari satu baris), sel gabungan yang terpotong oleh tepi pilihan, dan baris judul tabel.

## Filter

Pilih sel di dalam data Anda, lalu pilih **Filter** (**Ctrl+Shift+L**). Sebuah tombol muncul pada setiap sel di baris pertama. Klik salah satunya untuk melihat:

- **Urutkan A–Z** dan **Urutkan Z–A**, serta **Urutkan Berdasarkan Warna**.
- Daftar nilai dalam kolom dengan kotak centang, kotak pencarian, dan **(Semua)**. Kolom berisi tanggal didaftar menurut tahun, bulan, dan hari, dan nilai dalam kolom yang bukan tanggal didaftar di bawahnya. Hapus centang pada yang tidak Anda inginkan, lalu **OK**. Pencarian menampilkan nilai yang memuat semua kata Anda, lalu **(Semua yang Ditemukan)** mencentang atau menghapus centang hanya pada nilai-nilai tersebut. Daftar ini mencakup seluruh kolom. Jika kolom memiliki lebih banyak nilai berbeda daripada yang dapat ditampung daftar, JeffCrunch akan memberi tahu dan daftar dipotong; gunakan kondisi untuk sisanya.
- **Pertahankan Warna**, menurut warna isian atau warna teks.
- **Kondisi Angka**, **Kondisi Teks**, atau **Kondisi Tanggal**: sama dengan, di atas, minimal, di bawah, paling banyak, di antara, terbesar atau terkecil (item atau persen), di atas atau di bawah rata-rata, diawali dengan, diakhiri dengan, mengandung, tanggal dalam suatu periode (hari ini, minggu ini, bulan berikutnya, kuartal sebelumnya, sejauh tahun ini, satu kuartal atau satu bulan pada tahun mana pun), dan **Kondisi Sendiri** berupa dua perbandingan yang digabungkan dengan Dan atau Atau. Dalam teks kondisi, ? dan \* berarti satu karakter apa pun dan rangkaian karakter apa pun.
- **Tampilkan Semua** pada kolom.

Tombol berbentuk corong menandakan kolom yang memiliki filter. Baris yang tidak cocok disembunyikan dan tetap menyimpan nomor barisnya. **Hapus** (pada menu **Urutkan dan Pilih Baris**) menghapus semua kondisi dan menampilkan semua baris; **Terapkan Ulang** menghitung kembali kondisi setelah Anda mengubah suatu nilai. **Filter** sekali lagi akan mematikan tombol-tombol tersebut.

Baris yang disembunyikan oleh filter dipisahkan dari baris yang Anda sembunyikan sendiri: **Tampilkan Baris** tidak menampilkan baris yang disembunyikan filter, dan menghapus filter tidak menampilkan baris yang Anda sembunyikan sendiri. Pengurutan blok yang difilter hanya mengurutkan baris yang tampil, di antara baris-baris itu saja.

Filter bekerja pada seluruh lembar, hingga 1.048.576 baris, sedikit demi sedikit; **Hapus** dan mematikan filter bekerja untuk ukuran berapa pun. Pekerjaan yang diminta dari sebuah filter dibatasi 40 juta pemeriksaan (jumlah baris dikali jumlah kolom yang memiliki kondisi, ditambah sejumlah kecil untuk nilai-nilai yang didaftar kondisi); filter yang meminta lebih dari itu akan memberi tahu dan tidak dibuat, dan Anda dapat menyaring lebih sedikit baris atau memakai lebih sedikit kondisi. Filter lembar tidak mencakup sel tabel: tabel memiliki tombolnya sendiri, yang bekerja dengan cara yang sama (lihat [Tabel](tables)).

Daftar nilai dalam kondisi disimpan sebagaimana sebuah berkas menuliskannya, tanpa bahasa pada angka dan tanggal (1,234.50), dan sebuah nilai cocok dengan sel yang menampilkannya dalam bahasa Anda atau dalam bentuk tersebut; berkas yang difilter dalam satu bahasa akan menyaring sel yang sama dalam bahasa lain.

Filter disimpan dalam berkas beserta kondisi dan baris yang disembunyikannya, sehingga berkas terbuka dengan baris yang sama tampil. Saat berkas dibuka kembali, tombol corong muncul pada setiap kolom yang memiliki kondisi; baris yang disembunyikan dalam rentang filter dihitung sebagai milik filter.
