# Mengetik di sel

<!-- keywords: simbol, font simbol, karakter khusus, tombol pintas untuk simbol, koreksi otomatis, tanda kutip cerdas, tanda hubung, pecahan, tautan, alamat, kata tebal, bagian sel, teks kaya, format sebagian teks, warnai kata, mengetik, memasukkan, masuk, ubah, sel, isi, bilah rumus, F2, klik dua kali, Tab, Esc, batal, urungkan, ulangi, hapus, bersihkan, persen, mata uang, uang, tanggal, waktu, angka, teks, tanda kutip tunggal, rumus, sama dengan, ganti, batas, karakter -->

Untuk memasukkan sesuatu ke dalam sel, pilih sel lalu mulai mengetik. Apa yang Anda ketik menggantikan isi sel. Tekan **Enter** untuk menyimpannya, atau **Esc** untuk membatalkannya dan membiarkan sel seperti semula.

## Tiga cara memulai

- **Ketik langsung di sel.** Pilih sel lalu ketik. Karakter pertama yang Anda ketik memulai isian, dan sel menampilkan apa yang Anda ketik selagi mengetik.
- **Ubah isi sel.** Tekan **F2**, atau klik dua kali sel. Sel terbuka dengan isinya, dan kursor berada di akhir. Gunakan tombol panah untuk bergerak dalam teks.
- **Gunakan bilah rumus.** Klik di bilah rumus, yang berada di sebelah kotak Nama, lalu ketik di sana. Sel juga menampilkan apa yang Anda ketik.

Saat Anda mengetik langsung di sel, tombol panah menyelesaikan isian dan memindahkan pilihan. Saat Anda mengubah isi sel, tombol panah bergerak di dalam teks. Tekan **F2** untuk beralih dari satu mode ke mode lainnya.

## Menyelesaikan isian

| Tombol | Fungsinya |
| --- | --- |
| Enter | Menyimpan isian, dan pindah ke bawah (atau sesuai pengaturan di **Berkas > Opsi > Pengeditan**) |
| Shift+Enter | Menyimpan isian, dan pindah ke arah sebaliknya |
| Ctrl+Enter | Memasukkan isian ke semua sel yang dipilih, dan tetap di tempat Anda berada |
| Tab | Menyimpan isian, dan pindah ke kanan |
| Shift+Tab | Menyimpan isian, dan pindah ke kiri |
| Tombol panah | Menyimpan isian, dan pindah satu sel (hanya saat Anda mengetik langsung di sel) |
| Esc | Membatalkan isian |
| Alt+Enter | Memulai baris baru di dalam sel |
| Ctrl+; | Memasukkan tanggal hari ini (di dalam isian, saat Anda mengetiknya) |
| Ctrl+Shift+; | Memasukkan waktu sekarang |
| Alt+Panah Bawah | Menampilkan daftar kata yang sudah ada di kolom, dan memilih salah satu |

**Ctrl+Enter** dengan beberapa sel dipilih memasukkan apa yang Anda ketik ke semuanya. Rumus ditulis untuk sel tempat Anda mengetik, dan di sel lainnya rumus itu dibaca sebagai salinannya: mengetik **=A1** dengan blok E6:F7 dipilih menghasilkan **=B2** di F7. Ini merupakan satu langkah Urungkan.

**Alt+Panah Bawah** pada sebuah sel menampilkan, berurutan dan masing-masing sekali, kata-kata di sel atas dan bawahnya yang tidak dipisahkan oleh sel kosong. Memilih salah satu akan mengetiknya. Sel yang memiliki daftarnya sendiri akan menampilkan daftar tersebut.

Di **Berkas > Opsi > Pengeditan**, pilih apakah **Enter** pindah ke bawah, kanan, atas, atau kiri, atau tidak berpindah.

Jika Anda mengetik di beberapa sel sepanjang baris dengan **Tab**, lalu menekan **Enter**, pilihan akan turun ke baris berikutnya, di kolom tempat Anda mulai. Jika Anda memilih blok sel terlebih dahulu, **Enter** dan **Tab** bergerak melalui blok tersebut.

## Koreksi otomatis saat mengetik

Saat Anda mengetik spasi atau tanda baca seperti titik setelah sebuah kata, JeffCrunch dapat memperbaiki kata tersebut begitu Anda selesai menulisnya. Fitur ini memperbaiki salah ketik yang umum (seperti "teh" menjadi "the"), dua huruf kapital di awal kata, huruf kecil yang memulai kalimat atau nama hari, serta tanda kutip lurus (diubah menjadi tanda kutip melengkung). Fitur ini juga mengubah dua tanda hubung di antara kata menjadi tanda pisah panjang, serta 1/2, 1/4, dan 3/4 menjadi ½, ¼, dan ¾ jika isian memuat kata. Angka seperti 1/2 yang diketik sendirian tetap menjadi tanggal atau pecahan, seperti biasa.

Setiap perbaikan merupakan satu langkah tersendiri. Tekan **Ctrl+Z** tepat setelah sebuah perbaikan untuk membatalkan perbaikan itu saja dan mempertahankan apa yang Anda ketik. Tidak ada yang diubah dalam rumus: isian yang diawali tanda sama dengan, atau tanda plus atau minus lalu rumus, tetap seperti yang Anda ketik. Alamat web (seperti https://example.org) atau alamat email yang menjadi seluruh isian akan menjadi tautan di selnya saat Anda menyelesaikan isian, jika **Ubah alamat web menjadi tautan saat mengetik** aktif.

Aktifkan atau nonaktifkan fitur ini di **Berkas > Opsi > Pemeriksaan**. Kata yang salah eja ditandai saat Anda mengetik: lihat [Ejaan](notes-links).

## Format sebagian teks

Beberapa kata dalam sel dapat tampil berbeda dari sisanya. Misalnya, satu kata dapat dibuat tebal atau merah.

1. Tekan F2 untuk mengubah sel (atau klik dua kali).
2. Pilih kata yang ingin diubah, dengan Shift dan tombol panah atau dengan mouse.
3. Gunakan **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Font**, **Ukuran Font**, atau **Warna Font** pada tab **Beranda**, atau Ctrl+B, Ctrl+I, dan Ctrl+U.
4. Tekan Enter.

Hanya kata yang Anda pilih yang berubah. Bagian sel lainnya tetap mengikuti tampilan sel itu sendiri. Kotak isian menampilkan kata-kata dalam satu tampilan saat Anda mengedit, dan sel menampilkan setiap bagian dengan tampilannya masing-masing saat Anda menekan Enter. Jika kemudian Anda mengetik atau menghapus kata, format akan ikut bergerak bersama kata tersebut. Memilih font atau ukuran tanpa kata yang dipilih akan mengubah seluruh sel, seperti biasa.

Sel yang memiliki format di dalam teks dari program lain akan menampilkannya, mempertahankannya, dan menyimpannya kembali. Sel yang Anda ubah tetap mempertahankan formatnya. Format di dalam teks hanya berlaku untuk teks, bukan untuk angka atau rumus. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tetap menyimpan kata-katanya, dan penyimpanan akan memberi tahu bahwa format di dalamnya tidak dipertahankan.

## Yang Anda ketik menjadi apa

JeffCrunch membaca apa yang Anda ketik sesuai pengaturan komputer Anda. Tanda desimal, tanda ribuan, simbol mata uang, dan urutan hari serta bulan dalam tanggal semuanya berasal dari pengaturan tersebut. Angka yang Anda ketik di sel yang belum memiliki format sendiri akan diberi format yang menampilkannya seperti cara Anda mengetiknya.

| Yang Anda ketik | Isi sel | Dan tampil sebagai |
| --- | --- | --- |
| 1234.5 | angka 1234.5 | 1234.5 |
| 5% | angka 0.05 | 5% |
| $12.50 | angka 12.5 | $12.50 |
| 1 1/2 | angka 1.5 | 1 1/2 |
| 3/4/2025 | tanggal, sebagai jumlah hari | 3/4/2025 |
| 10:30 PM | waktu, sebagai bagian dari hari | 10:30 PM |
| TRUE | nilai logika benar | TRUE |
| #N/A | galat #N/A | #N/A |
| =B2*2 | rumus | hasil rumus |
| selain itu | teks | teks tersebut |

Sel yang sudah memiliki format sendiri, seperti persen atau tanggal, tetap mempertahankannya. Sel yang formatnya **Teks** menyimpan semua yang Anda ketik sebagai teks, kecuali rumus.

Untuk menyimpan sesuatu yang tampak seperti angka sebagai teks, seperti kode **007**, ketik tanda kutip satu di depannya: **'007**. Tanda kutip satu itu bukan bagian dari teks. Saat Anda mengubah sel, tanda tersebut terlihat kembali.

## Rumus

Rumus diawali dengan tanda sama dengan. Misalnya, **=B2*2** menggandakan angka di B2. Nama fungsi boleh Anda ketik dengan huruf kapital atau huruf kecil; JeffCrunch akan menuliskannya dengan huruf kapital.

Nama fungsi selalu menggunakan bahasa Inggris, di semua bahasa: ketik **SUM**, bukan nama yang digunakan bahasa Anda. Tanda pemisah di antaranya mengikuti pengaturan komputer Anda. Jika tanda desimal berupa koma, seperti di Jerman dan Prancis, Anda memisahkan argumen fungsi dengan titik koma dan menulis koma pada angka: **=SUM(A1;A2)*0,5**. Bilah rumus dan **F2** menampilkan rumus dengan cara yang sama. Jika tanda desimal berupa titik, Anda memisahkan argumen dengan koma: **=SUM(A1,A2)*0.5**. Berkas selalu menyimpan bentuk yang dibaca oleh setiap program, dengan koma di antara argumen dan titik pada angka, sehingga berkas terbuka dengan sama di mana pun.

Jika rumus memiliki kesalahan, seperti tanda kurung yang tidak ditutup, JeffCrunch memberi tahu apa yang salah, dan isian tetap seperti yang Anda ketik, dengan kursor di dekat kesalahan agar Anda dapat memperbaikinya. Tidak ada yang diubah sampai rumusnya benar.

Sebuah sel dapat berisi hingga 32.767 karakter, dan sebuah rumus hingga 8.192. JeffCrunch memberi tahu Anda jika isian lebih panjang dari itu.

## Menghapus isi sel

Pilih sel lalu tekan **Delete**. Ini menghapus isi sel dan mempertahankan formatnya, dan merupakan satu langkah yang dapat diurungkan. JeffCrunch dapat menghapus hingga 100.000 sel sekaligus.

Pada tab **Beranda**, tombol **Hapus** memiliki menu. **Hapus Semua** mengosongkan sel, formatnya, catatannya, komentarnya (utas beserta balasannya), dan tautannya. Utas dengan lebih dari 500 komentar tetap disimpan, dan JeffCrunch memberi tahu Anda, karena Urungkan tidak dapat mengembalikan komentar yang tidak pernah dibacanya. **Hapus Format** mempertahankan isi sel dan mengembalikan format biasa. **Hapus Isi** sama dengan **Delete**. **Hapus Komentar dan Catatan** hanya menghapus catatan dan komentar dari sel yang dipilih. **Hapus Tautan** hanya menghapus tautan, dan sel tetap memiliki teksnya. Pilihan yang tidak tersedia akan dibuat redup jika sel yang dipilih tidak memiliki apa pun untuk dihapus.

## Urungkan dan Ulangi

**Ctrl+Z** mengurungkan perubahan terakhir, dan **Ctrl+Y** mengulanginya kembali. Keduanya juga tersedia di bilah alat cepat di bagian atas. Setiap sel yang Anda isi merupakan satu langkah, dan begitu pula setiap penghapusan. Urungkan mengembalikan sel seperti semula: nilainya, rumusnya, formatnya, dan rumus-rumus yang bergantung padanya. JeffCrunch mengingat hingga 1.000 langkah, lebih sedikit jika langkah-langkah itu mengubah sangat banyak sel, agar tidak memakan banyak memori.

## Selama JeffCrunch menghitung rumus

Saat Anda mengubah sebuah sel, JeffCrunch menghitung rumus-rumus yang bergantung padanya di latar belakang. Sel yang Anda ubah langsung menampilkan isi barunya, sedangkan sel lainnya diperbarui sebentar kemudian; bilah status menampilkan **Menghitung** sampai proses selesai. Anda tetap dapat mengetik selama itu.

## Yang tidak dapat diubah

- Sel yang merupakan bagian dari **rumus array** atau tabel data tidak dapat diubah sendiri.
- Pada **lembar yang dilindungi**, sel yang terkunci tidak dapat diubah. Setiap sel terkunci kecuali buku kerja menyatakan sebaliknya. JeffCrunch memberi tahu Anda saat Anda mencobanya. Untuk mengubah sel, pilih Hapus Perlindungan Lembar pada tab Tinjau.
- Baris pertama sebuah **tabel** berisi nama kolomnya. Ketik nama baru di salah satu sel tersebut untuk mengganti nama kolom: rumus yang memakai nama lama akan mengikuti, dan tabel di berkas tetap valid. Nama yang sudah dimiliki kolom lain akan diberi angka di belakangnya, nama tidak boleh kosong, dan **Delete** tidak menghapus baris itu.
- Buku kerja yang tidak dapat dibaca seluruhnya dapat dilihat, tetapi tidak dapat diubah atau disimpan.
- Fungsi yang belum dimiliki JeffCrunch membuat rumus baru tanpa nilai di selnya; buku kerja akan meminta program yang membukanya untuk menghitung rumus tersebut.
- Pemformatan, pengurutan, serta temukan dan ganti akan hadir pada versi berikutnya. Menyisipkan dan menghapus, potong dan tempel, isi, serta ukuran baris dan kolom dijelaskan di [Sisipkan dan hapus baris, kolom, dan sel](insert-delete), [Potong, salin, dan tempel](clipboard), [Isi sel dan jumlahkan](fill-sum), dan [Ubah lebar kolom dan tinggi baris](sizes).

## Memasukkan simbol

Klik **Sisipkan > Simbol > Simbol** untuk membuka kotak Simbol. Pilih karakter lalu klik **Sisipkan**, atau klik dua kali karakter tersebut. Karakter akan dimasukkan di posisi kursor isian yang sedang Anda ketik. Jika Anda tidak sedang mengetik, isian sel itu sendiri akan terbuka dengan simbol ditambahkan di akhirnya, dan **Enter** memasukkannya ke sel. Contoh: dengan sel yang berisi kata *Total*, memilih tanda euro menghasilkan isian *Total€*. Simbol yang terakhir Anda gunakan ditampilkan lebih dulu.

**Ambil karakter dari sebuah font.** Pilih font pada kotak **Font**, lalu pilih karakternya. Karakter akan dimasukkan dalam bagian tersendiri yang diatur dengan font tersebut, dan sisa teks sel tetap menggunakan font sel. Contoh: pada sel yang berisi *Total*, tanda centang yang dipilih dari font simbol menghasilkan *Total✓*, dengan hanya tanda centang yang menggunakan font tersebut. Jika **(teks normal)** yang dipilih, font sel akan menggambar simbol, sehingga simbol yang tidak dimiliki font itu tampil sebagai kotak. Dalam rumus, hanya karakternya yang dimasukkan.

**Karakter khusus.** Tab **Karakter Khusus** memuat spasi tanpa pemisah dan tanda hubung, tanda hubung opsional, tanda pisah en dan em, spasi en dan em, tanda kutip, serta pemisah opsional tanpa lebar, masing-masing dengan namanya. Klik dua kali salah satunya untuk memasukkannya.

**Beri simbol tombol pintas.** Pilih simbol atau karakter khusus, klik **Tombol Pintas**, tekan tombol yang Anda inginkan, misalnya **Ctrl+Alt+Shift+J**, lalu klik **OK**. Tombol tersebut kemudian memasukkan simbol itu, dengan cara yang sama seperti kotak. Kosongkan tombolnya dan klik **OK** untuk menghapusnya. JeffCrunch memberi tahu Anda jika perintah lain sudah menggunakan tombol tersebut. Simbol menjadi sebuah perintah, sehingga juga tercantum di **Berkas > Opsi > Sesuaikan > Papan Ketik**, tempat Anda dapat mengubah tombolnya (lihat [Pintasan papan ketik](keyboard)).

## Buku kerja baru

Di **Berkas > Opsi > Buku Kerja Baru**, pilih font, ukuran font, dan jumlah lembar yang dimiliki setiap buku kerja baru. Buku kerja yang sudah ada tidak akan berubah.
