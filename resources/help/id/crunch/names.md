# Nama untuk sel dan nilai
<!-- keywords: nama, nama-nama, nama yang ditentukan, nama baru, rentang bernama, lingkup, komentar, nama dari sel, sisipkan nama, terapkan nama, Ctrl+F3, merujuk ke, urutkan nama, nama kesalahan, peringatan, definisi nama, rentang nama -->

Sebuah **nama** adalah kata yang mewakili sebuah sel, sekumpulan sel, sebuah angka, atau sebuah rumus, sehingga =Harga*Jumlah dapat ditulis sebagai pengganti =B2*C2. Nama berlaku untuk seluruh buku kerja, atau untuk satu lembar. Nama pada sebuah lembar menyembunyikan nama buku kerja yang ejaannya sama, pada lembar tersebut.

## Aturan untuk sebuah nama

- Nama diawali dengan huruf, garis bawah, atau garis miring terbalik, dan setelahnya hanya berisi huruf, angka, garis bawah, dan titik. Nama tidak mengandung spasi.
- Panjang nama paling banyak 255 karakter.
- Nama bukan alamat sel, seperti **A1** atau **R2C3**.
- Tidak ada nama lain pada lembar yang sama (atau pada buku kerja) yang ejaannya sama, baik huruf besar maupun kecil.

Saat Anda mengetik, kotak dialog menjelaskan apa yang salah, dan tombol **OK** tidak aktif.

## Membuat nama

1. Pilih sel-sel, lalu klik **Nama Baru** pada tab **Rumus**.
2. Ketikkan nama, pilih tempat nama itu berlaku, dan tentukan apa yang diwakilinya. Sel-sel yang Anda pilih akan diisikan secara otomatis.

Ada cara yang lebih cepat untuk nama yang berlaku di seluruh buku kerja. Pilih sel-sel, klik **Kotak Nama** (di sebelah kiri bilah rumus), ketikkan nama, lalu tekan **Enter**. Jika belum ada nama dengan ejaan tersebut, nama itu dibuat untuk sel-sel yang Anda pilih. Jika sudah ada, **Enter** akan membawa Anda ke sel-sel milik nama tersebut.

**Buat Nama dari Sel** membuat banyak nama sekaligus dari label di sekeliling sekumpulan sel: dengan judul di baris atas dan angka di bawahnya, setiap judul menamai sel-sel di bawahnya. Pilih letak labelnya (baris atas, kolom kiri, baris bawah, atau kolom kanan). Label yang berisi spasi atau tanda yang tidak boleh digunakan dalam nama akan diganti dengan garis bawah. Label yang kosong, berulang, atau sudah menjadi nama dalam buku kerja akan dilewati, dan bilah status menampilkan jumlahnya.

## Melihat semua nama

Klik **Nama** pada tab **Rumus**, atau tekan **Ctrl+F3**. Daftar menampilkan setiap nama, nilai yang dimilikinya saat ini, apa yang diwakilinya, tempat berlakunya, dan komentarnya. Ketik di kotak filter untuk hanya menampilkan nama yang cocok. **Baru**, **Ubah**, dan **Hapus** digunakan untuk membuat, mengubah, dan menghapus nama. Klik judul kolom untuk mengurutkan daftar berdasarkan kolom tersebut, dan klik lagi untuk membalik urutannya. Nama yang menunjuk ke kesalahan, misalnya ke tempat yang sudah dihapus, diberi tanda peringatan kecil di sebelahnya. Buku kerja dengan lebih dari 20.000 nama hanya mendaftarkan 20.000 nama pertama.

Mengubah nama sebuah nama akan mengubahnya di setiap rumus yang memakainya, di setiap lembar, dalam satu langkah. Menghapus sebuah nama membuat rumus yang memakainya menampilkan **#NAME?**. Setiap perubahan dapat dibatalkan.

## Memasukkan nama ke dalam rumus

Saat Anda mengetik rumus, nama-nama yang cocok ditampilkan dalam daftar di bawah sel. **Sisipkan Nama** pada tab **Rumus** menampilkan daftar nama untuk dipilih.

## Memasukkan nama ke dalam rumus yang sudah ditulis

Misalnya Anda membuat nama Tarif untuk $A$1 setelah banyak rumus yang membaca $A$1. Klik **Terapkan Nama** pada tab **Rumus** untuk menuliskan Tarif di tempat-tempat tersebut. Pilih sel-sel yang akan diperiksa terlebih dahulu, atau pilih satu sel untuk memeriksa seluruh lembar. Pilih nama-nama yang akan dimasukkan. Sebuah referensi hanya diubah jika persis sama dengan sel-sel milik nama tersebut, dengan atau tanpa tanda dolar: A1:B2 tidak diubah menjadi nama untuk A1. Hanya nama untuk satu blok sel pada sebuah lembar yang ditawarkan, dan hanya nama yang berlaku untuk seluruh buku kerja. Perubahan ini dilakukan dalam satu langkah, sehingga **Urungkan** akan membatalkannya.
