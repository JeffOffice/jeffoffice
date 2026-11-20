# Mengetik di sel
<!-- keywords: simbol, font simbol, karakter khusus, tombol pintas untuk simbol, koreksi otomatis, tanda kutip cerdas, tanda hubung, pecahan, tautan, alamat, kata tebal, bagian sel, teks kaya, format sebagian teks, beri warna pada kata, mengetik, memasukkan, enter, edit, ubah, sel, isian, bilah rumus, F2, klik dua kali, Tab, Esc, batalkan isian, urungkan, ulangi, hapus, bersihkan, persen, mata uang, uang, tanggal, waktu, angka, teks, tanda kutip tunggal, rumus, sama dengan, ganti, batas, karakter -->

Untuk memasukkan sesuatu ke dalam sel, pilih sel tersebut dan mulai mengetik. Apa pun yang Anda ketik akan menggantikan isi sel. Tekan **Enter** untuk menyimpannya, atau **Esc** untuk membatalkan isian dan membiarkan sel seperti semula.

## Tiga cara memulai

- **Ketik langsung di sel.** Pilih sel lalu ketik. Karakter pertama yang Anda ketik memulai isian, dan sel menampilkan apa yang Anda ketik selama Anda mengetik.
- **Edit isi sel.** Tekan **F2**, atau klik dua kali sel. Sel akan terbuka dengan isinya, dan kursor berada di akhir. Gunakan tombol panah untuk bergerak dalam teks.
- **Gunakan bilah rumus.** Klik di dalam bilah rumus, yang ada di sebelah Kotak Nama, lalu ketik di sana. Sel juga menampilkan apa yang Anda ketik.

Saat Anda mengetik langsung di sel, tombol panah menyelesaikan isian dan memindahkan pilihan. Saat Anda mengedit isi sel, tombol panah bergerak di dalam teks. Tekan **F2** untuk beralih dari satu mode ke mode lainnya.

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
| Ctrl+; | Memasukkan tanggal hari ini (di dalam isian, saat Anda sedang mengetiknya) |
| Ctrl+Shift+; | Memasukkan waktu saat ini |
| Alt+Down | Menampilkan daftar kata yang sudah ada di kolom, dan memilih salah satunya |

**Ctrl+Enter** dengan beberapa sel terpilih memasukkan apa yang Anda ketik ke semuanya. Rumus ditulis untuk sel tempat Anda mengetik, dan di sel lain rumus itu dibaca seperti salinannya: mengetik **=A1** dengan blok E6:F7 terpilih menghasilkan **=B2** di F7. Ini dihitung sebagai satu langkah Urungkan.

**Alt+Down** pada sebuah sel menampilkan daftar kata-kata dari sel di atas dan di bawahnya, sesuai urutan dan hanya sekali untuk setiap kata, selama kata-kata tersebut tidak dipisahkan oleh sel kosong. Memilih salah satu kata akan mengetiknya. Sel yang memiliki daftarnya sendiri akan menampilkan daftar tersebut.

Di **Berkas > Opsi > Pengeditan**, pilih apakah **Enter** memindahkan ke bawah, kanan, atas, atau kiri, atau tidak memindahkan sama sekali.

Jika Anda mengetik di beberapa sel sepanjang satu baris dengan **Tab**, lalu menekan **Enter**, pilihan akan turun ke baris berikutnya, di kolom tempat Anda mulai. Jika Anda memilih satu blok sel terlebih dahulu, **Enter** dan **Tab** akan bergerak melalui blok tersebut.

## Koreksi otomatis saat mengetik

Saat Anda mengetik spasi atau tanda seperti titik setelah sebuah kata, JeffCrunch dapat memperbaiki kata tersebut setelah Anda selesai mengetiknya. Perbaikan ini mencakup kesalahan ketik yang umum (seperti "teh" untuk "the"), dua huruf kapital di awal kata, huruf kecil yang memulai kalimat atau nama hari, dan tanda kutip lurus (yang diubah menjadi tanda kutip melengkung). Dua tanda hubung di antara kata juga diubah menjadi tanda pisah panjang, dan 1/2, 1/4, serta 3/4 diubah menjadi ½, ¼, dan ¾ jika isian berisi kata. Angka seperti 1/2 yang diketik sendirian tetap menjadi tanggal atau pecahan, seperti biasa.

Setiap perbaikan adalah satu langkah tersendiri. Tekan **{key:Undo}** tepat setelah sebuah perbaikan untuk membatalkan perbaikan itu saja dan mempertahankan apa yang Anda ketik. Tidak ada yang diubah di dalam rumus: isian yang diawali tanda sama dengan, atau tanda plus atau minus lalu rumus, tetap seperti yang Anda ketik. Alamat web (seperti https://example.org) atau alamat email yang menjadi seluruh isian akan menjadi tautan di selnya saat Anda menyelesaikan isian, jika **Ubah alamat web menjadi tautan saat mengetik** diaktifkan.

Aktifkan atau nonaktifkan ini di **Berkas > Opsi > Pemeriksaan**. Kata yang salah eja ditandai saat Anda mengetik: lihat [Ejaan](notes-links).

## Memformat sebagian teks

Beberapa kata dalam sel dapat tampil berbeda dari sisa teks. Misalnya, satu kata dapat dibuat tebal atau merah.

1. Tekan F2 untuk mengedit sel (atau klik dua kali).
2. Pilih kata yang ingin diubah, dengan Shift dan tombol panah atau dengan mouse.
3. Gunakan **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Font**, **Ukuran Font**, atau **Warna Font** pada tab **Beranda**, atau tekan Ctrl+B, Ctrl+I, dan Ctrl+U.
4. Tekan Enter.

Hanya kata yang Anda pilih yang berubah. Sisa sel tetap mempertahankan tampilan sel itu sendiri. Kotak isian menampilkan kata-kata dalam satu tampilan saat Anda mengedit, dan sel menampilkan setiap bagian dengan tampilannya masing-masing saat Anda menekan Enter. Jika Anda kemudian mengetik atau menghapus kata, format akan ikut berpindah bersama kata tersebut. Memilih font atau ukuran tanpa kata yang dipilih akan mengubah seluruh sel, seperti biasa.

## Bilah Alat Mini

Sebuah bilah kecil berisi format yang paling sering Anda gunakan akan muncul di dekat tempat Anda bekerja, sehingga Anda tidak perlu naik ke pita. Bilah ini tidak pernah mengambil alih papan ketik dari sel: apa yang sedang Anda ketik tetap di tempatnya.

- **Teks terpilih di dalam sel:** pilih kata dengan mouse saat Anda mengedit sel, dan bilah akan muncul di atasnya. Bilah ini memiliki kotak **font** dan **ukuran**, **Tebal**, **Miring**, **Garis Bawah**, **Perbesar Ukuran Font**, **Perkecil Ukuran Font**, dan **Warna Font**. Perintah ini hanya mengubah kata yang Anda pilih, sama seperti pada pita.
- **Sel terpilih:** klik kanan sel, dan bilah berada di bagian atas menu pintasan. Bilah ini juga memiliki **Warna Isian**, **Bingkai**, **Mata Uang**, **Gaya Persen**, **Gaya Koma**, **Tambah Desimal**, **Kurangi Desimal**, **Gabungkan dan Tengahkan**, serta **Salin Format**. Ini adalah perintah pita yang sama, dan masing-masing adalah satu langkah yang dapat diurungkan.

Bilah di atas teks terpilih memudar saat Anda menggerakkan penunjuk menjauh darinya, dan hilang ketika penunjuk berada cukup jauh. Bilah juga hilang saat Anda mengetik, menekan Esc, atau menggulir. Untuk mematikan bilah, hapus centang **Tampilkan Bilah Alat Mini saat Memilih** pada halaman **Umum** di **Berkas > Opsi**; aplikasi JeffOffice lainnya juga mengikuti pilihan ini. Tombol Analisis Cepat yang muncul di bawah rentang terpilih tidak tersedia dalam versi ini.

Sel yang memiliki format di dalam teksnya dari program lain akan menampilkan, mempertahankan, dan menyimpan kembali format tersebut. Sel yang Anda ubah tetap mempertahankan formatnya. Format di dalam teks hanya berlaku untuk teks, bukan untuk angka atau rumus. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tetap menyimpan katanya, dan penyimpanan akan memberi tahu Anda bahwa format di dalam kata tersebut tidak dipertahankan.

## Apa yang Anda ketik menjadi

JeffCrunch membaca apa yang Anda ketik sesuai pengaturan komputer Anda. Tanda desimal, tanda ribuan, simbol mata uang, dan urutan hari serta bulan dalam tanggal semuanya berasal dari pengaturan tersebut. Angka yang Anda ketik di sel yang belum memiliki format sendiri akan diberi format yang menampilkannya sesuai cara Anda mengetiknya.

| Yang Anda ketik | Isi sel | Yang ditampilkan |
| --- | --- | --- |
| 1234.5 | angka 1234.5 | 1234.5 |
| 5% | angka 0,05 | 5% |
| $12.50 | angka 12,5 | $12.50 |
| 1 1/2 | angka 1,5 | 1 1/2 |
| 3/4/2025 | tanggal, sebagai jumlah hari | 3/4/2025 |
| 10:30 PM | waktu, sebagai bagian dari sehari | 10:30 PM |
| TRUE | nilai logika benar | TRUE |
| #N/A | galat #N/A | #N/A |
| =B2*2 | rumus | hasil rumus |
| yang lain | teks | teks itu sendiri |

Sel yang sudah memiliki format sendiri, seperti persen atau tanggal, akan mempertahankannya. Sel yang formatnya **Teks** akan menyimpan semua yang Anda ketik sebagai teks, kecuali rumus.

Untuk menyimpan sesuatu yang tampak seperti angka sebagai teks, misalnya kode **007**, ketik tanda kutip tunggal terlebih dahulu: **'007**. Tanda kutip tunggal tersebut bukan bagian dari teks. Saat Anda mengedit sel, tanda itu akan terlihat lagi.

## Rumus

Rumus diawali dengan tanda sama dengan. Misalnya, **=B2*2** menggandakan angka di B2. Nama fungsi boleh diketik dengan huruf kapital atau huruf kecil; JeffCrunch akan menulisnya dengan huruf kapital.

Nama fungsi selalu menggunakan bahasa Inggris, dalam bahasa apa pun: ketik **SUM**, bukan nama yang diberikan bahasa Anda. Tanda pemisah di antara fungsi mengikuti pengaturan komputer Anda. Jika tanda desimal berupa koma, seperti dalam bahasa Jerman dan Prancis, Anda memisahkan argumen fungsi dengan titik koma dan menulis koma di dalam angka: **=SUM(A1;A2)*0,5**. Bilah rumus dan **F2** menampilkan rumus dengan cara yang sama. Jika tanda desimal berupa titik, Anda memisahkan argumen dengan koma: **=SUM(A1,A2)*0.5**. Berkas selalu menyimpan bentuk yang dapat dibaca semua program, dengan koma di antara argumen dan titik di dalam angka, sehingga berkas akan terbuka dengan sama di mana pun.

Jika rumus memiliki kesalahan, misalnya tanda kurung yang tidak pernah ditutup, JeffCrunch akan memberi tahu apa yang salah, dan isian tetap seperti yang Anda ketik, dengan kursor di dekat kesalahan agar Anda dapat memperbaikinya. Tidak ada yang diubah sampai rumus benar.

Sebuah sel dapat berisi hingga 32.767 karakter, dan sebuah rumus hingga 8.192. JeffCrunch memberi tahu Anda jika isian lebih panjang dari itu.

## Menghapus isi sel

Pilih sel lalu tekan **Delete**. Ini menghapus isi sel dan mempertahankan formatnya, dan merupakan satu langkah yang dapat diurungkan. JeffCrunch dapat menghapus hingga 100.000 sel sekaligus.

Pada tab **Beranda**, tombol **Hapus** memiliki menu. **Hapus Semua** mengosongkan sel, format-nya, catatannya, komentarnya (utas dengan balasan), dan tautannya. Utas yang memiliki lebih dari 500 komentar tetap disimpan, dan JeffCrunch memberi tahu Anda, karena Urungkan tidak dapat mengembalikan komentar yang tidak sempat dibacanya. **Hapus Format** mempertahankan isi sel dan mengembalikan format biasa. **Hapus Isi** sama dengan **Delete**. **Hapus Komentar dan Catatan** hanya menghapus catatan dan komentar dari sel yang dipilih. **Hapus Hiperlink** hanya menghapus tautannya, dan sel tetap mempertahankan teksnya. Pilihan akan dibuat redup jika sel yang dipilih tidak memiliki apa pun untuk dihapus.

## Urungkan dan Ulangi

**{key:Undo}** mengurungkan perubahan terakhir, dan **{key:Redo}** mengulanginya. Keduanya juga ada di bilah alat cepat di bagian atas. Setiap sel yang Anda isi adalah satu langkah, begitu juga setiap penghapusan. Urungkan mengembalikan sel seperti semula: nilainya, rumusnya, formatnya, dan rumus lain yang bergantung padanya. JeffCrunch mengingat hingga 1.000 langkah, lebih sedikit jika langkah-langkah tersebut mengubah sangat banyak sel, agar tidak memakai banyak memori.

## Saat JeffCrunch menghitung rumus

Saat Anda mengubah sebuah sel, JeffCrunch menghitung rumus yang bergantung padanya di latar belakang. Sel yang Anda ubah langsung menampilkan isi barunya, dan sel lain diperbarui sebentar kemudian; bilah status menampilkan **Menghitung** sampai selesai. Anda tetap dapat mengetik selama itu.

## Yang tidak dapat diubah

- Sel yang menjadi bagian dari **rumus array** atau tabel data tidak dapat diubah sendiri. Untuk membuat rumus array, pilih sel-sel yang akan diisinya, ketik rumusnya, lalu tekan **Ctrl+Shift+Enter**. Satu rumus kemudian mengisi semua sel, dan bilah rumus menampilkannya dengan kurung kurawal, misalnya **{=A1:A3*2}**. Untuk mengubahnya, pilih salah satu sel array, tekan **F2**, ubah rumusnya, lalu tekan **Ctrl+Shift+Enter** lagi. Untuk menghapusnya, pilih semua selnya dan tekan **Delete**. Keduanya bekerja pada seluruh array, dalam satu langkah Urungkan. F2 pada sel mana pun di dalam array akan membuka rumus array tersebut, dan lembar yang dilindungi atau rentang yang memakai sandi akan menolak atau meminta sandi seperti untuk sel lainnya. Menyalin array, atau sebagiannya, dan mengisi dari salah satu selnya akan menghasilkan nilai yang ditampilkannya, bukan rumus.
- Pada **lembar yang dilindungi**, sel yang terkunci tidak dapat diubah. Setiap sel terkunci kecuali buku kerja menyatakan sebaliknya. JeffCrunch akan memberi tahu Anda saat Anda mencobanya. Untuk mengubah sel, pilih **Hapus Perlindungan Lembar** pada tab **Tinjau**.
- Baris pertama **tabel** berisi nama kolomnya. Ketik nama baru di salah satu sel tersebut untuk mengganti nama kolom: rumus yang memakai nama lama akan mengikuti, dan tabel dalam berkas tetap valid. Nama yang sudah dimiliki kolom lain akan diberi angka di belakangnya, nama tidak boleh kosong, dan **Delete** tidak menghapus baris itu.
- Buku kerja yang tidak dapat dibaca secara utuh dapat dilihat, tetapi tidak dapat diubah atau disimpan.
- Fungsi yang belum dimiliki JeffCrunch akan membuat rumus baru tanpa nilai di sel; buku kerja akan meminta program yang membukanya untuk menghitung rumus tersebut.
- Pemformatan, pengurutan, serta temukan dan ganti akan tersedia pada versi berikutnya. Menyisipkan dan menghapus, potong dan tempel, isi, serta ukuran baris dan kolom dibahas di [Sisipkan dan hapus baris, kolom, dan sel](insert-delete), [Potong, salin, dan tempel](clipboard), [Isi sel dan jumlahkan](fill-sum), dan [Ubah lebar kolom dan tinggi baris](sizes).

## Memasukkan simbol

Klik **Sisipkan > Simbol > Simbol** untuk membuka kotak Simbol. Pilih karakter lalu klik **Sisipkan**, atau klik dua kali karakter tersebut. Karakter akan dimasukkan di posisi kursor isian yang sedang Anda ketik. Jika Anda tidak sedang mengetik, isian sel itu sendiri akan terbuka dengan simbol ditambahkan di akhirnya, dan **Enter** akan memasukkannya ke sel. Contoh: dengan sel yang berisi kata *Total*, memilih tanda euro menghasilkan isian *Total€*. Simbol yang terakhir Anda gunakan ditampilkan lebih dahulu.

**Ambil karakter dari sebuah font.** Pilih font di kotak **Font**, lalu pilih karakternya. Karakter akan dimasukkan dalam bagian tersendiri yang diatur dengan font tersebut, dan sisa teks sel tetap menggunakan font sel. Contoh: pada sel yang berisi *Total*, tanda centang yang dipilih dari font simbol menghasilkan *Total✓*, dengan hanya tanda centang itu yang menggunakan font tersebut. Dengan **(teks normal)** yang dipilih, font sel akan menggambar simbol, sehingga simbol yang tidak ada di font tersebut tampil sebagai kotak. Dalam rumus, hanya karakter itu yang dimasukkan.

**Karakter khusus.** Tab **Karakter Khusus** mencantumkan spasi dan tanda hubung tanpa pemisah kata, tanda hubung opsional, tanda pisah en dan em, spasi en dan em, tanda kutip, serta pemisah opsional tanpa lebar, masing-masing dengan namanya. Klik dua kali salah satunya untuk memasukkannya.

**Beri simbol tombol pintas.** Pilih simbol atau karakter khusus, klik **Tombol Pintas**, tekan tombol yang Anda inginkan, misalnya **Ctrl+Alt+Shift+J**, lalu klik **OK**. Tombol tersebut kemudian memasukkan simbol itu, sama seperti kotak tadi. Kosongkan tombol dan klik **OK** untuk menghapusnya. JeffCrunch memberi tahu Anda jika perintah lain sudah memakai tombol tersebut. Simbol itu menjadi sebuah perintah, sehingga juga tercantum di **Berkas > Opsi > Sesuaikan > Papan Ketik**, tempat Anda dapat mengubah tombolnya (lihat [Pintasan papan ketik](keyboard)).

## Buku kerja baru

Di **Berkas > Opsi > Buku Kerja Baru**, pilih font, ukuran font, dan jumlah lembar yang dimiliki setiap buku kerja baru di awal. Buku kerja yang sudah ada tidak diubah.
