# Memilih sel dan berpindah

<!-- keywords: pilih, pilihan, sel, rentang, baris, kolom, klik, seret, shift, ctrl, panah, home, end, page up, page down, tab, enter, seluruh baris, seluruh kolom, kotak nama, buka, pindahkan sel, seret batas, gulir, bilah status, jumlah, rata-rata, hitung, select, cell, range -->

**Memilih** sel berarti menentukan sel mana yang akan terpengaruh oleh perintah berikutnya. Sel yang dipilih diberi warna latar dengan bingkai hijau di sekelilingnya. **Sel aktif** adalah sel tempat kursor berada, dan sel ini tetap berwarna putih di dalam pilihan.

## Dengan mouse

- **Klik** sel untuk memilihnya. Klik di dalam sel gabungan untuk memilih seluruh sel gabungan tersebut.
- **Seret** melintasi sel untuk memilih satu blok. Seret melewati tepi jendela dan lembar akan ikut bergulir.
- **Shift+klik** sel lain untuk memilih dari sel aktif sampai ke sel tersebut.
- Klik **huruf kolom** atau **nomor baris** untuk memilih seluruh kolom atau baris. Seret melintasi huruf atau nomor, atau gunakan Shift+klik, untuk memilih beberapa kolom atau baris sekaligus.
- Klik **sudut** tempat huruf dan nomor bertemu untuk memilih seluruh isi lembar.
- Tahan **Ctrl** saat Anda mengklik atau menyeret untuk **menambahkan rentang** ke pilihan. Rentang yang Anda tambahkan diberi warna, dan rentang yang berisi kursor memiliki bingkai hijau. Tombol **Delete** menghapus semua rentang sekaligus. Perintah yang hanya bekerja pada satu rentang, seperti Salin, Isi, Urutkan, Temukan dalam pilihan, Ganti Semua, atau format, akan memberi tahu Anda dan tidak mengubah apa pun; klik sekali untuk kembali ke satu rentang. Berpindah ke lembar lain, atau menyisipkan dan menghapus baris atau kolom, juga membuat pilihan kembali menjadi satu rentang.
- **Seret batas** pilihan untuk memindahkan sel ke tempat lain. Arahkan kursor ke tepi bingkai hijau hingga penunjuk berubah menjadi empat panah. Garis putus-putus menunjukkan tempat sel akan dijatuhkan. Apa pun yang merujuk ke sel tersebut, seperti rumus, akan ikut berpindah. Tahan **Ctrl** saat menyeret untuk menyalin sel. Untuk memindahkan sel ke lembar lain, mulailah menyeret batas, lalu tahan **Alt** dan lepaskan di atas tab lembar tersebut: sel akan dipindahkan ke sel yang sama pada lembar itu (dengan Ctrl juga, sel akan disalin). Menahan **Alt** sebelum Anda menekan tombol mouse tidak memulai jenis seretan ini. Sel yang sudah berada di tempat tujuannya akan ditimpa, seperti saat Anda memindahkan sel dalam satu lembar, dan **Urungkan** mengembalikan kedua lembar dalam satu langkah. Lembar tujuan harus mengizinkan hal ini: lembar yang dilindungi, serta sel yang akan memotong sel gabungan, tabel, atau tabel pivot, ditolak dan tidak ada yang diubah. Lembar tersembunyi tidak memiliki tab, dan tombol **Lembar baru** bukan tab, sehingga keduanya tidak dapat menerima sel. Menekan dan melepas **Alt** saat menyeret tidak menampilkan KeyTips. Untuk melakukan hal yang sama dari keyboard, potong sel, buka lembar lain, lalu tempel. Tahan **Shift** saat menyeret untuk menyisipkan sel di antara sel lain, bukan menimpanya. Untuk melakukannya tanpa mouse, potong atau salin sel, lalu gunakan **Sisipkan Sel yang Dipotong atau Disalin** (lihat [Sisipkan dan hapus](insert-delete)). Jika Anda hanya mengklik batas tanpa memindahkan, sel akan dipilih seperti biasa.
- Putar **roda mouse** untuk menggulir tiga baris. Tahan **Shift** untuk menggulir ke samping, atau tahan **Ctrl** untuk memperbesar tampilan.

## Dengan keyboard

Tombol panah memindahkan sel aktif, dengan melewati baris dan kolom yang disembunyikan. Tahan **Shift** untuk memperluas pilihan. **Ctrl** dengan panah akan pergi ke tepi data: ke ujung deretan sel yang berisi sesuatu, atau, dari ujung deretan, ke sel berisi berikutnya. Lihat [Menggunakan JeffCrunch dengan keyboard](keyboard) untuk semua tombol.

**Ctrl+A** memilih blok sel di sekitar sel aktif, dibatasi oleh baris dan kolom kosong; tekan lagi untuk memilih seluruh lembar. Jika pilihan sudah lebih besar daripada blok tersebut, Ctrl+A langsung memilih seluruh lembar. **Ctrl+Shift+*** hanya memilih blok. Tekan **End**, lalu panah, untuk pergi ke tepi data seperti yang dilakukan Ctrl dengan panah; bilah status akan menampilkan **End** sampai Anda menekan panah, menekan tombol lain, atau mengklik.

## Membuka sel

Ketik alamat di **Kotak Nama**, seperti **C5**, **A1:D10**, **B:D**, atau **3:7**, lalu tekan **Enter**. Tekan **F5** (atau Ctrl+G) untuk membuka kotak dialog Buka, lalu ketik alamat di sana. Beri koma di antara alamat, misalnya **A1:B2,D4**, untuk memilih beberapa rentang sekaligus.

## Angka dari pilihan

Saat Anda memilih dua sel atau lebih, bilah status menampilkan **rata-rata**, **jumlah hitung**, dan **jumlah** dari sel-sel tersebut. Jumlah hitung adalah banyaknya sel yang berisi sesuatu; rata-rata dan jumlah hanya memakai angka. Baris dan kolom tersembunyi tidak ikut dihitung. Untuk menyalin pilihan sebagai teks, tekan **Ctrl+C**.

## Isi bilah status

Di sisi kiri bilah status, tertulis **Siap** saat Anda dapat bekerja, **Enter** saat Anda mengetik di dalam sel, **Edit** setelah Anda menekan F2, **Titik** saat Anda dapat mengklik atau menekan panah untuk memasukkan alamat sel ke dalam rumus, dan **End** setelah Anda menekan tombol End. Saat filter aktif, bilah status juga menampilkan berapa baris yang tersisa dari filter, misalnya **3 dari 8 rekaman ditemukan**.
