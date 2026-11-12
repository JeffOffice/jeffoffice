# Menyisipkan dan menghapus baris, kolom, dan sel
<!-- keywords: sisipkan, hapus, buang, baris, kolom, sel, geser, pindahkan, tambah, kosong, Ctrl+plus, Ctrl+minus, klik kanan, header, urungkan, referensi, #REF!, rumus, nama, digabungkan, tabel, array, tepi -->

Anda dapat menambahkan baris, kolom, atau sel kosong di tengah data, serta menghapus baris, kolom, atau sel. Semua yang ada di bawah atau di samping akan bergeser untuk memberi ruang atau menutup celah, dan rumus yang mengacu ke sel yang berpindah tetap mengacu ke sel tersebut.

## Menyisipkan

1. Pilih nomor baris, huruf kolom, atau sel tempat Anda ingin menambahkan yang baru. Untuk menambahkan tiga baris, pilih tiga baris.
2. Tekan **Ctrl dan tombol plus** (**Ctrl dan tombol minus** untuk menghapus), atau pilih **Beranda > Sisipkan**, atau klik kanan pada pilihan lalu pilih **Sisipkan**.

Jika yang dipilih adalah seluruh baris atau kolom, baris atau kolom baru akan disisipkan sekaligus, di atas baris atau di sebelah kiri kolom yang dipilih. Jika yang dipilih adalah sel, sebuah jendela kecil akan menanyakan bagaimana sel-sel setelahnya harus bergeser:

- **Geser sel ke kanan** atau **Geser sel ke bawah** hanya menggeser sel-sel pada kolom (atau baris) pilihan Anda, dan membiarkan bagian lain lembar tetap di tempatnya.
- **Seluruh baris** atau **Seluruh kolom** menambahkan baris atau kolom secara utuh.

## Menghapus

Pilih bagian yang ingin dihapus lalu tekan **Ctrl dan tombol minus**, atau pilih **Beranda > Hapus**. Jika yang dipilih adalah sel, jendela akan menanyakan apakah akan **menggeser sel ke kiri**, **menggeser sel ke atas**, atau menghapus **seluruh baris** atau **seluruh kolom**. Penghapusan dilakukan dalam satu langkah, dan **Urungkan** akan mengembalikan semua yang terhapus, termasuk rumusnya.

## Yang terjadi selanjutnya

Rumus yang mengacu ke sel yang berpindah akan mengacu ke sel tersebut di posisi barunya. Contoh: di B1 terdapat **=SUM(A1:A5)**. Jika Anda menyisipkan baris di atas baris 3, rumus akan menjadi **=SUM(A1:A6)**, karena rentangnya bertambah untuk mencakup baris baru. Jika Anda menghapus baris 3, rumus akan menjadi **=SUM(A1:A4)**. Rumus yang mengacu ke sel yang telah dihapus akan menampilkan **#REF!**, yang berarti sel yang dibutuhkan sudah tidak ada. **Urungkan** akan mengembalikan sel dan rumus tersebut.

Hal berikut juga ikut bergeser bersama sel: rumus di semua lembar, nama (seperti area cetak dan judul cetak), sel yang digabungkan, sel tempat aturan validasi dan pemformatan bersyarat berlaku, tabel, bagan, gambar, catatan, hiperlink, dan pemisah halaman.

## Yang tidak dapat dilakukan

- Sel tidak dapat disisipkan atau dihapus pada sebagian **rumus array** atau tabel data. Pindahkan secara utuh, atau biarkan saja. Sel yang dibaca tabel data sebagai masukan juga tidak dapat dihapus.
- **Tabel pivot** belum dapat dipindahkan atau dipotong: baris, kolom, atau sel tidak dapat disisipkan atau dihapus jika tindakan itu akan memindahkan atau mengubahnya, dan sel tidak dapat dipotong dari atau ke dalamnya. Bagian lain lembar tetap dapat diubah seperti biasa. Tabel pivot diubah dengan perintah pada tab **Tabel Pivot** (segarkan, ubah sumber data, bidang, dan tata letak), bukan dengan mengetik di selnya.
- **Tabel** tidak dapat dipotong secara melintang: Anda tidak dapat menyisipkan kolom di dalamnya, dan tidak dapat menghapus baris judulnya atau semua barisnya. Sisipkan atau hapus baris di dalamnya, atau ubah sel di sekitarnya.
- **Lembar yang dilindungi** tidak dapat disisipi atau dihapus barisnya, kolomnya, atau selnya. Untuk melakukannya, pilih Hapus Perlindungan Lembar pada tab Tinjau.
- Sel di ujung lembar yang berisi sesuatu tidak dapat didorong keluar dari tepi. Hapus dahulu sel-sel tersebut.
- Setelah Anda menyimpan, **Urungkan** tetap dapat membatalkan tindakan Anda sebelumnya, termasuk baris dan kolom yang Anda sisipkan (pada penyimpanan berikutnya, baris dan kolom itu dihapus lagi). Hal yang tidak dapat dikembalikan adalah baris, kolom, atau sel yang sudah **dihapus**: berkas yang tersimpan telah kehilangan isi di dalamnya, sehingga **Urungkan** berhenti pada penghapusan terakhir yang Anda simpan. Baris di bagian bawah jendela akan menyatakan hal ini saat Anda menyimpan.
- Langkah yang memindahkan atau menghapus begitu banyak sel sehingga **Urungkan** tidak dapat menyimpannya (lebih dari sekitar 128 MB) akan meminta konfirmasi terlebih dahulu, seperti yang dilakukan program lembar kerja pada umumnya: "Ini tidak dapat diurungkan." Jika Anda melanjutkan, tindakan itu langsung dilakukan dan **Urungkan** tidak dapat membatalkannya, beserta semua tindakan sebelumnya.
