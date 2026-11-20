# Memilih sel dan berpindah

<!-- keywords: pilih, pilihan, sel, rentang, baris, kolom, klik, seret, shift, ctrl, panah, home, end, page up, page down, tab, enter, seluruh baris, seluruh kolom, kotak nama, buka, pindahkan sel, seret batas, gulir, bilah status, jumlah, rata-rata, hitung -->

**Memilih** sel berarti menentukan sel mana yang akan terkena perintah berikutnya. Sel yang dipilih diberi warna latar dengan bingkai hijau di sekelilingnya. **Sel aktif** adalah sel yang ditempati kursor, dan sel ini tetap berwarna putih di dalam pilihan.

## Dengan mouse

- **Klik** sebuah sel untuk memilihnya. Klik di dalam sel gabungan untuk memilih seluruh sel gabungan tersebut.
- **Seret** melintasi sel untuk memilih satu blok. Jika Anda menyeret melewati tepi jendela, lembar akan bergulir.
- **Shift+klik** sel lain untuk memilih dari sel aktif sampai ke sel tersebut.
- Klik **huruf kolom** atau **nomor baris** untuk memilih seluruh kolom atau baris. Seret melintasi huruf atau nomor, atau gunakan Shift+klik, untuk memilih beberapa kolom atau baris sekaligus.
- Klik **sudut** tempat huruf dan nomor bertemu untuk memilih semuanya.
- Tahan **Ctrl** saat Anda mengklik atau menyeret untuk **menambahkan rentang** ke pilihan. Rentang yang ditambahkan diberi warna latar, dan rentang yang ditempati kursor memiliki bingkai hijau. Tombol **Delete** menghapus semua rentang sekaligus. Perintah yang hanya bekerja pada satu rentang, seperti Salin, Isi, Urutkan, Temukan dalam pilihan, Ganti Semua, atau format, akan memberi tahu Anda dan tidak mengubah apa pun; klik sekali untuk kembali ke satu rentang. Berpindah ke lembar lain, atau menyisipkan maupun menghapus baris atau kolom, juga membuat pilihan kembali menjadi satu rentang.
- **Seret batas** pilihan untuk memindahkan sel ke tempat lain. Arahkan penunjuk ke tepi bingkai hijau hingga penunjuk menampilkan empat panah. Garis putus-putus menunjukkan tempat sel akan diletakkan. Apa pun yang merujuk ke sel tersebut, seperti rumus, ikut berpindah. Tahan **Ctrl** saat menyeret untuk menyalin sel. Tahan **Shift** saat menyeret untuk menyisipkan sel di antara sel lain, bukan menimpanya. Untuk melakukan hal yang sama tanpa mouse, potong atau salin sel, lalu gunakan **Sisipkan Sel yang Dipotong atau Disalin** (lihat [Sisipkan dan hapus](insert-delete)). Jika Anda hanya mengklik batas tanpa memindahkan, sel akan dipilih seperti biasa.
- Putar **roda mouse** untuk menggulir tiga baris. Tahan **Shift** untuk menggulir ke samping, atau tahan **Ctrl** untuk memperbesar tampilan.

## Dengan keyboard

Tombol panah memindahkan sel aktif dan melewati baris serta kolom yang tersembunyi. Tahan **Shift** untuk memperluas pilihan. **Ctrl** dengan panah akan menuju tepi data: ke ujung deretan sel yang berisi sesuatu, atau, dari ujung deretan, ke sel berikutnya yang berisi sesuatu. Lihat [Menggunakan JeffCrunch dengan keyboard](keyboard) untuk semua tombol.

**Ctrl+A** memilih blok sel di sekitar sel aktif, dibatasi oleh baris dan kolom kosong; tekan lagi untuk memilih seluruh lembar. Jika pilihan sudah lebih besar dari blok tersebut, Ctrl+A langsung memilih seluruh lembar. **Ctrl+Shift+*** hanya memilih blok. Tekan **End**, lalu panah, untuk menuju tepi data seperti yang dilakukan Ctrl dengan panah; bilah status menampilkan **End** sampai Anda menekan panah, menekan tombol lain, atau mengklik.

## Buka sel

Ketik alamat di **Kotak Nama**, misalnya **C5**, **A1:D10**, **B:D**, atau **3:7**, lalu tekan **Enter**. Tekan **F5** (atau Ctrl+G) untuk membuka Buka, lalu ketik alamat di sana. Pisahkan alamat dengan koma, misalnya **A1:B2,D4**, untuk memilih beberapa rentang sekaligus.

## Angka dari pilihan

Saat Anda memilih dua sel atau lebih, bilah status menampilkan **rata-rata**, **hitung**, dan **jumlah** dari sel-sel tersebut. Hitung adalah jumlah sel yang berisi sesuatu; rata-rata dan jumlah hanya menggunakan angka. Baris dan kolom yang tersembunyi tidak dihitung. Untuk menyalin pilihan sebagai teks, tekan **Ctrl+C**.

## Arti bilah status

Bagian kiri bilah status menampilkan **Siap** saat Anda dapat bekerja, **Masukkan** saat Anda mengetik di dalam sel, **Edit** setelah Anda menekan F2, **Point** saat Anda dapat mengklik atau menekan panah ke sebuah sel untuk memasukkan alamatnya ke dalam rumus, dan **End** setelah Anda menekan tombol End. Saat filter aktif, bilah status juga menampilkan berapa baris yang tersisa dari filter, misalnya **3 dari 8 rekaman ditemukan**.
