# Tabel
<!-- keywords: tabel, tabel, hapus duplikat, duplikat, ringkas, hapus gaya, referensi terstruktur, baris header, baris total, kolom terhitung, berpita, berselang-seling, gaya, tampilan tabel, gaya tabel baru, gaya tabel kustom, tombol filter, Ctrl+T, konversi, sel biasa, ubah ukuran, nama kolom, [@Kolom], Tabel1[Kolom] -->

Tabel adalah blok sel dengan baris nama di atas kolom-kolomnya, yang diperlakukan sebagai satu kesatuan: tabel memiliki nama, tampilan, tombol untuk memfilter dan mengurutkan barisnya, dan bertambah besar saat Anda menambahkan data. Rumus dapat merujuk kolom tabel dengan namanya, bukan dengan alamat sel.

## Membuat tabel

Pilih sel-selnya, atau klik sebuah sel di dalam data, lalu pilih **Tabel** pada tab **Sisipkan** (**Ctrl+T**). JeffCrunch akan menanyakan sel mana yang dicakup tabel dan apakah baris pertama berisi nama-nama kolom. Nama yang berupa angka, tanggal, atau rumus akan menjadi teks yang ditampilkannya, nama kosong menjadi Kolom1, Kolom2, dan seterusnya, serta dua kolom dengan nama yang sama diberi nomor. Jika tidak ada nama, tabel tidak memiliki baris nama, dan kolomnya disebut Kolom1, Kolom2, dan seterusnya dalam rumus.

Galeri **Tampilan Tabel** (tab **Beranda**, atau tab **Tabel**) menampilkan berbagai tampilan (**Hapus**, di bawah galeri pada tab **Tabel**, untuk menghapus tampilan dari tabel): terang, sedang, dan gelap, masing-masing dalam warna hitam serta enam warna dari tema buku kerja. Memilih salah satu untuk sel yang bukan tabel akan menjadikannya tabel. Tampilan digambar di bawah apa pun yang sudah dimiliki sel: isian, bingkai, warna teks, atau huruf tebal yang Anda atur pada sel tetap dipertahankan.

Untuk membuat tampilan sendiri, pilih **Gaya Tabel Baru** di bagian bawah galeri. Ketik sebuah nama. Lalu pilih warna isian, warna teks, dan huruf tebal untuk setiap bagian yang ingin Anda ubah: baris header, garis-garis baris, baris total, dan kolom pertama. Tampilan baru akan muncul dalam grup **Kustom** di bagian atas galeri, dan langsung diterapkan pada tabel tempat Anda berada. Tampilan itu disimpan di dalam berkas, sehingga tetap ada saat berkas dibuka kembali.

## Tab Tabel

Tab **Tabel** tampil selama kursor berada di dalam tabel.

- **Nama** adalah nama yang digunakan rumus untuk memanggil tabel. Setiap rumus yang memakai nama lama akan mengikuti perubahan. Nama terdiri atas huruf, angka, garis bawah, dan titik, diawali dengan huruf atau garis bawah, dan tidak boleh sama dengan nama tabel lain, nama di dalam buku kerja, atau alamat sel seperti A1.
- **Baris Nama**, **Baris Total**, **Baris Berselang-seling**, **Kolom Berselang-seling**, **Kolom Pertama**, **Kolom Terakhir**, dan **Tombol Filter** dapat dicentang atau dibatalkan untuk tabel.
- **Ubah Ukuran Tabel…** meminta sel-sel yang dicakup tabel. Sel pertamanya tetap; baris dan kolom dapat ditambahkan di bagian bawah dan kanan, atau dihapus.
- **Ringkas dengan Tabel Pivot** membuat tabel pivot dari sel-sel tabel (lihat [Tabel pivot](pivot-tables)). Tabel pivot membaca tabel melalui namanya, sehingga ikut berubah saat tabel bertambah besar.
- **Hapus Duplikat…** menghapus baris yang berulang. Sebuah baris dianggap berulang jika menampilkan nilai yang sama, di semua kolom yang Anda centang, dengan baris di atasnya. Huruf besar dan kecil dianggap sama. Baris pertama tetap dipertahankan, dan baris di bawah baris yang dihapus akan naik. Tabel menjadi lebih pendek, dan baris totalnya ikut naik. Satu kali **Urungkan** akan mengembalikan semuanya. Sel di bawah tabel tetap di tempatnya. Rumus yang menunjuk ke sel tabel, di lembar mana pun, akan mengikuti sel tersebut seperti saat Anda menghapus baris; rumus yang menunjuk baris yang dihapus akan menampilkan #REF!. Fitur ini tidak berfungsi pada tabel yang sedang difilter.
- **Kembali ke Sel Biasa** mengubah tabel menjadi sel biasa. Sel-sel tetap menyimpan isinya. Rumus yang menyebut nama tabel atau kolomnya akan menyebut sel-selnya sebagai gantinya.

## Menu sel

Klik kanan sebuah sel di dalam tabel dan pilih **Tabel**: **Sisipkan Baris di Atas**, **Sisipkan Baris di Bawah**, **Sisipkan Kolom di Kiri**, **Sisipkan Kolom di Kanan**, **Hapus Baris Tabel**, **Hapus Kolom Tabel**, **Baris Total**, dan **Kembali ke Sel Biasa**. Perintah-perintah ini menambah atau menghapus seluruh baris dan kolom lembar, seperti perintah Sisipkan dan Hapus; baris yang disisipkan tepat setelah akhir tabel akan dimasukkan ke dalam tabel.

## Baris total

Aktifkan **Baris Total**, dan baris di bawah tabel akan berisi kata Total pada kolom pertamanya serta sebuah total pada kolom terakhirnya. Setiap sel pada baris ini memiliki tombol: pilih apa yang ditampilkan total kolom tersebut (rata-rata, jumlah data, jumlah angka, nilai terbesar, nilai terkecil, jumlah, simpangan baku, varians, atau **Rumus Sendiri**). Total adalah SUBTOTAL dari kolomnya, sehingga baris yang disembunyikan oleh filter tidak ikut dihitung.

## Menambah data ke tabel

- Ketik pada baris di bawah tabel, atau pada kolom di sebelahnya, dan tabel akan bertambah untuk mencakup sel tersebut. **Urungkan** akan membatalkan pertambahan itu dan tetap menyisakan apa yang Anda ketik.
- Rumus yang diketik pada sel di kolom yang selainnya kosong akan dimasukkan ke setiap sel di kolom tersebut: ini disebut kolom terhitung. Tekan **Urungkan** segera setelahnya, dan rumus tetap berada pada satu sel saja. Rumus yang diketik di atas kolom terhitung akan mengubah seluruh kolom jika sel lainnya berisi rumus lama; jika sebagian tidak, sel tersebut berdiri sendiri.
- Baris yang disisipkan di dalam tabel akan menerima rumus dari kolom terhitung. Kolom yang disisipkan di dalamnya diberi nama pada baris nama. Menghapus baris atau kolom di dalam tabel akan membuatnya lebih kecil; baris nama dan setiap kolom tidak dapat dihapus semuanya.

## Memfilter dan mengurutkan tabel

Tombol pada baris nama hanya memfilter dan mengurutkan baris tabel. Cara kerjanya sama seperti tombol filter lembar (lihat [Urutkan dan filter](sort-filter)). Tabel tidak dapat difilter selama sebuah filter lembar, atau filter tabel lain, mencakup baris yang sama, karena masing-masing perlu mengetahui baris mana yang menjadi miliknya.

## Rumus yang menyebut tabel

- `Penjualan[Jumlah]` adalah sel-sel kolom Jumlah, tanpa nama dan total kolomnya.
- `[@Jumlah]` adalah sel pada baris rumus itu sendiri, dan ditulis `Penjualan[[#This Row],[Jumlah]]` di dalam berkas.
- `Penjualan[#Headers]`, `Penjualan[#Data]`, `Penjualan[#Totals]`, dan `Penjualan[#All]` masing-masing adalah nama, data, total, dan seluruh tabel, sedangkan `Penjualan[[#Headers],[Jumlah]]` adalah satu kolom dari bagian nama tersebut.
- `Penjualan[[Wilayah]:[Jumlah]]` adalah kolom-kolom dari Wilayah hingga Jumlah.

Ketik `[` setelah nama tabel, dan daftar kolomnya akan muncul. Saat Anda mengetik rumus, mengklik sel tabel akan menulis rujukan seperti `Penjualan[@Jumlah]` atau `Penjualan[Jumlah]` (mengklik seluruh kolom data akan menulis bentuk yang kedua). Mengganti nama kolom pada header-nya akan mengubahnya di setiap rumus; menyisipkan dan menghapus kolom tetap membuat rumus merujuk ke kolom yang dimaksud, dan kolom yang dihapus akan membuat #REF! pada rumus yang menyebutnya.
