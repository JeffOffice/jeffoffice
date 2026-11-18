# Mengetik di sel
<!-- keywords: simbol, font simbol, karakter khusus, tombol pintas untuk simbol, koreksi otomatis, tanda kutip cerdas, tanda hubung, pecahan, tautan, alamat, kata tebal, bagian dari sel, teks kaya, format sebagian teks, warnai kata, ketik, mengetik, masukkan, sunting, ubah, sel, isian, bilah rumus, F2, klik dua kali, Tab, Esc, batalkan, urungkan, ulangi, hapus, kosongkan, persen, mata uang, uang, tanggal, waktu, angka, teks, apostrof, rumus, sama dengan, ganti, batas, karakter -->

Untuk memasukkan sesuatu ke dalam sel, pilih sel tersebut lalu mulai mengetik. Apa pun yang Anda ketik akan menggantikan isi sel. Tekan **Enter** untuk menyimpannya, atau **Esc** untuk membatalkan isian dan membiarkan sel seperti semula.

## Tiga cara memulai

- **Ketik langsung di sel.** Pilih sel lalu ketik. Karakter pertama yang Anda ketik memulai isian, dan sel menampilkan apa yang Anda ketik saat Anda mengetik.
- **Sunting isi sel.** Tekan **F2**, atau klik dua kali sel. Sel terbuka dengan isinya, dan kursor berada di akhir. Gunakan tombol panah untuk bergerak di dalam teks.
- **Gunakan bilah rumus.** Klik di bilah rumus, yang berada di sebelah kotak Nama, lalu ketik di sana. Sel juga menampilkan apa yang Anda ketik.

Saat Anda mengetik langsung di sel, tombol panah menyelesaikan isian dan memindahkan pilihan. Saat Anda menyunting isi sel, tombol panah bergerak di dalam teks. Tekan **F2** untuk berpindah dari satu mode ke mode lainnya.

## Menyelesaikan isian

| Tombol | Fungsinya |
| --- | --- |
| Enter | Menyimpan isian, dan pindah ke bawah (atau sesuai pengaturan di **Berkas > Opsi > Pengeditan**) |
| Shift+Enter | Menyimpan isian, dan pindah ke arah sebaliknya |
| Ctrl+Enter | Memasukkan isian ke semua sel yang dipilih, dan kursor tetap di tempatnya |
| Tab | Menyimpan isian, dan pindah ke kanan |
| Shift+Tab | Menyimpan isian, dan pindah ke kiri |
| Tombol panah | Menyimpan isian, dan pindah satu sel (hanya saat Anda mengetik langsung di sel) |
| Esc | Membatalkan isian |
| Alt+Enter | Memulai baris baru di dalam sel |
| Ctrl+; | Memasukkan tanggal hari ini (dalam isian, saat Anda sedang mengetiknya) |
| Ctrl+Shift+; | Memasukkan waktu saat ini |
| Alt+Panah Bawah | Menampilkan kata-kata yang sudah ada di kolom, lalu memilih salah satunya |

**Ctrl+Enter** dengan beberapa sel terpilih memasukkan apa yang Anda ketik ke semuanya. Rumus ditulis untuk sel tempat Anda mengetik, dan di setiap sel lain rumus itu berfungsi seperti salinannya: mengetik **=A1** dengan blok E6:F7 terpilih menghasilkan **=B2** di F7. Ini dihitung sebagai satu langkah Urungkan.

**Alt+Panah Bawah** pada sebuah sel menampilkan, berurutan dan tanpa pengulangan, kata-kata yang ada di sel di atas dan di bawahnya, selama tidak dipisahkan oleh sel kosong. Memilih salah satunya akan mengetiknya. Sel yang memiliki daftar sendiri menampilkan daftar tersebut.

Di **Berkas > Opsi > Pengeditan**, Anda dapat memilih apakah **Enter** memindahkan kursor ke bawah, kanan, atas, atau kiri, atau tidak memindahkannya sama sekali.

Jika Anda mengetik di beberapa sel sepanjang baris dengan **Tab**, lalu menekan **Enter**, pilihan akan turun ke baris berikutnya, di kolom tempat Anda mulai. Jika Anda lebih dulu memilih blok sel, **Enter** dan **Tab** bergerak melalui blok tersebut.

## Koreksi Otomatis saat mengetik

Saat Anda mengetik spasi atau tanda baca seperti titik setelah sebuah kata, JeffCrunch dapat memperbaiki kata tersebut begitu Anda selesai mengetiknya. Perbaikan ini mencakup salah ketik yang umum (seperti "teh" menjadi "the"), dua huruf kapital di awal kata, huruf kecil yang memulai kalimat atau nama hari, dan tanda kutip lurus (menjadi tanda kutip melengkung). Dua tanda hubung di antara kata diubah menjadi tanda pisah panjang, dan 1/2, 1/4, serta 3/4 diubah menjadi ½, ¼, dan ¾ jika isiannya memuat kata. Angka seperti 1/2 yang diketik sendirian tetap menjadi tanggal atau pecahan, seperti biasa.

Setiap perbaikan adalah langkah tersendiri. Tekan **Urungkan** tepat setelah sebuah perbaikan untuk membatalkan perbaikan itu saja dan mempertahankan apa yang Anda ketik. Tidak ada yang diubah dalam rumus: isian yang diawali tanda sama dengan, atau tanda plus atau minus lalu sebuah rumus, tetap seperti yang Anda ketik. Alamat web (seperti https://example.org) atau alamat email yang menjadi seluruh isian akan menjadi tautan di sel tersebut saat Anda menyelesaikan isian, jika **Ubah alamat web menjadi tautan saat mengetik** diaktifkan.

Anda dapat menyalakan atau mematikan fitur ini di **Berkas > Opsi > Pemeriksaan**. Kata yang salah eja ditandai saat Anda mengetik: lihat [Ejaan](notes-links).

## Memformat sebagian teks

Beberapa kata dalam sel dapat tampak berbeda dari sisanya. Misalnya, satu kata dapat dibuat tebal atau merah.

1. Tekan F2 untuk menyunting sel (atau klik dua kali sel).
2. Pilih kata yang ingin diubah, dengan Shift dan tombol panah, atau dengan mouse.
3. Gunakan **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Font**, **Ukuran Font**, atau **Warna Font** pada tab **Beranda**, atau Ctrl+B, Ctrl+I, dan Ctrl+U.
4. Tekan Enter.

Hanya kata yang Anda pilih yang berubah. Bagian sel lainnya tetap mengikuti tampilan sel itu sendiri. Kotak isian menampilkan kata-kata dalam satu tampilan saat Anda menyunting, dan sel menampilkan setiap bagian dengan tampilannya sendiri saat Anda menekan Enter. Jika kemudian Anda mengetik atau menghapus kata, pemformatannya ikut bergerak bersama kata tersebut. Memilih font atau ukuran tanpa ada kata yang dipilih akan mengubah seluruh sel, seperti biasa.

## Bilah mini

Sebuah bilah kecil berisi pemformatan yang paling sering Anda gunakan muncul di dekat tempat Anda bekerja, sehingga Anda tidak perlu naik ke pita. Bilah ini tidak pernah mengambil alih keyboard dari sel: apa yang sedang Anda ketik tetap di tempatnya.

- **Teks terpilih di dalam sel:** pilih kata dengan mouse saat menyunting sel, dan bilah akan muncul di atasnya. Bilah ini memiliki kotak **font** dan **ukuran**, **Tebal**, **Miring**, **Garis Bawah**, **Perbesar Ukuran Font**, **Perkecil Ukuran Font**, dan **Warna Font**. Perintah ini hanya mengubah kata yang Anda pilih, sama seperti di pita.
- **Sel terpilih:** klik kanan sel, dan bilah berada di bagian atas menu pintasan. Bilah ini juga memiliki **Warna Isian**, **Bingkai**, **Mata Uang**, **Gaya Persen**, **Gaya Koma**, **Tambah Desimal**, **Kurangi Desimal**, **Gabungkan dan Tengahkan**, dan **Salin Format**. Ini adalah perintah pita yang sama, dan setiap perintah adalah satu langkah yang dapat diurungkan.

Bilah di atas teks terpilih akan memudar saat Anda menjauhkan penunjuk, dan hilang jika penunjuk berada cukup jauh. Bilah juga hilang saat Anda mengetik, menekan Esc, atau menggulir. Untuk mematikan bilah, hapus centang **Tampilkan Bilah Alat Mini saat Memilih** pada halaman **Umum** di **Berkas > Opsi**; aplikasi JeffOffice lainnya juga mengikuti pilihan ini. Tombol Analisis Cepat yang muncul di bawah rentang terpilih tidak tersedia dalam versi ini.

Sel yang memiliki pemformatan di dalam teks dari program lain akan menampilkannya, menyimpannya, dan menyimpannya kembali. Sel yang Anda ubah tetap mempertahankan pemformatannya. Pemformatan di dalam teks hanya berlaku untuk teks, bukan untuk angka atau rumus. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tetap menyimpan kata-katanya, dan penyimpanan akan memberi tahu Anda bahwa pemformatan di dalam kata-kata tersebut tidak dipertahankan.

## Hasil dari apa yang Anda ketik

JeffCrunch membaca apa yang Anda ketik sesuai pengaturan komputer Anda. Tanda desimal, tanda ribuan, tanda mata uang, dan urutan hari serta bulan dalam tanggal berasal dari pengaturan tersebut. Angka yang Anda ketik di sel yang belum memiliki format sendiri akan diberi format yang menampilkannya seperti cara Anda mengetiknya.

| Anda mengetik | Sel menyimpan | Dan menampilkan |
| --- | --- | --- |
| 1234,5 | angka 1234,5 | 1234,5 |
| 5% | angka 0,05 | 5% |
| Rp12.500 | angka 12500 | Rp12.500 |
| 1 1/2 | angka 1,5 | 1 1/2 |
| 3/4/2025 | tanggal, sebagai jumlah hari | 3/4/2025 |
| 22:30 | waktu, sebagai bagian dari hari | 22:30 |
| TRUE | nilai logika benar | TRUE |
| #N/A | galat #N/A | #N/A |
| =B2*2 | sebuah rumus | hasil rumus |
| selain itu | teks | teks itu sendiri |

Sel yang sudah memiliki format sendiri, seperti persen atau tanggal, tetap mempertahankannya. Sel dengan format **Teks** menyimpan semua yang Anda ketik sebagai teks, kecuali rumus.

Untuk menyimpan sesuatu yang tampak seperti angka sebagai teks, seperti kode **007**, ketik tanda kutip satu terlebih dahulu: **'007**. Tanda kutip itu bukan bagian dari teks. Saat Anda menyunting sel, tanda kutip itu terlihat lagi.

## Rumus

Rumus diawali dengan tanda sama dengan. Misalnya, **=B2*2** melipatgandakan angka di B2. Nama fungsi dapat Anda ketik dengan huruf kapital atau huruf kecil; JeffCrunch menulisnya dengan huruf kapital.

Nama fungsi selalu memakai bahasa Inggris, di semua bahasa, jadi ketik **SUM**, bukan nama yang diberikan bahasa Anda. Tanda baca di antaranya mengikuti pengaturan komputer Anda. Jika tanda desimal berupa koma, seperti di Jerman dan Prancis, Anda memisahkan argumen fungsi dengan titik koma dan menulis koma pada angka: **=SUM(A1;A2)*0,5**. Bilah rumus dan **F2** menampilkan rumus dengan cara yang sama. Jika tanda desimal berupa titik, Anda memisahkan argumen dengan koma: **=SUM(A1,A2)*0.5**. Berkas selalu menyimpan bentuk yang dibaca oleh setiap program, yaitu dengan koma di antara argumen dan titik pada angka, sehingga berkas terbuka dengan sama di mana pun.

Jika sebuah rumus memiliki kesalahan, misalnya kurung yang tidak ditutup, JeffCrunch memberi tahu apa yang salah. Isian tetap seperti yang Anda ketik, dengan kursor di dekat kesalahan, sehingga Anda dapat memperbaikinya. Tidak ada yang diubah sampai rumus benar.

Sebuah sel dapat menampung hingga 32.767 karakter, dan sebuah rumus hingga 8.192 karakter. JeffCrunch memberi tahu Anda jika isian lebih panjang dari itu.

## Menghapus isi sel

Pilih sel lalu tekan **Delete**. Ini menghapus isi sel dan mempertahankan formatnya, dan merupakan satu langkah yang dapat diurungkan. JeffCrunch dapat menghapus hingga 100.000 sel sekaligus.

Pada tab **Beranda**, tombol **Hapus** memiliki menu. **Hapus Semua** mengosongkan sel, format, catatan, komentar (utas dengan balasan), dan tautannya. Utas dengan lebih dari 500 komentar tetap disimpan, dan JeffCrunch memberi tahu Anda, karena Urungkan tidak dapat mengembalikan komentar yang tidak sempat dibaca. **Hapus Format** mempertahankan isi sel dan mengembalikan format biasa. **Hapus Isi** sama dengan **Delete**. **Hapus Komentar dan Catatan** hanya menghapus catatan dan komentar dari sel yang dipilih. **Hapus Hiperlink** hanya menghapus tautan, dan sel tetap menyimpan teksnya. Pilihan yang tidak tersedia berarti sel yang dipilih tidak memiliki apa pun untuk dihapus.

## Urungkan dan Ulangi

**Urungkan** membatalkan perubahan terakhir, dan **Ulangi** mengerjakannya kembali. Keduanya juga ada di bilah alat cepat di bagian atas. Setiap sel yang Anda isi adalah satu langkah, begitu pula setiap penghapusan. Urungkan mengembalikan sel seperti semula: nilai, rumus, format, dan rumus lain yang bergantung padanya. JeffCrunch menyimpan hingga 1.000 langkah, lebih sedikit jika langkah tersebut mengubah sangat banyak sel, agar tidak memakan banyak memori.

## Selagi JeffCrunch menghitung rumus

Saat Anda mengubah sebuah sel, JeffCrunch menghitung rumus yang bergantung padanya di latar belakang. Sel yang Anda ubah langsung menampilkan isi barunya, dan sel lain diperbarui sesaat kemudian; bilah status menampilkan **Menghitung** sampai proses selesai. Anda tetap dapat mengetik selama itu.

## Yang tidak dapat diubah

- Sel yang merupakan bagian dari **rumus larik** atau tabel data tidak dapat diubah sendiri.
- Pada **lembar yang dilindungi**, sel yang terkunci tidak dapat diubah. Semua sel terkunci kecuali buku kerja menyatakan sebaliknya. JeffCrunch memberi tahu Anda saat Anda mencobanya. Untuk mengubah sel, pilih Hapus Perlindungan Lembar pada tab Tinjau.
- Baris pertama **tabel** berisi nama kolomnya. Ketik nama baru di salah satu sel tersebut untuk mengganti nama kolom: rumus yang memakai nama lama ikut menyesuaikan, dan tabel dalam berkas tetap valid. Nama yang sudah dimiliki kolom lain akan diberi angka di belakangnya, nama tidak boleh kosong, dan **Delete** tidak menghapus baris itu.
- Buku kerja yang tidak dapat dibaca secara utuh dapat dilihat, tetapi tidak dapat diubah atau disimpan.
- Fungsi yang belum dimiliki JeffCrunch membuat rumus baru tanpa nilai di sel; buku kerja meminta program yang membukanya berikutnya untuk menghitung rumus tersebut.
- Pemformatan, pengurutan, serta temukan dan ganti akan hadir pada versi berikutnya. Menyisipkan dan menghapus, potong dan tempel, isi otomatis, serta ukuran baris dan kolom dibahas di [Sisipkan dan hapus baris, kolom, dan sel](insert-delete), [Potong, salin, dan tempel](clipboard), [Isi sel dan jumlahkan](fill-sum), dan [Ubah lebar kolom dan tinggi baris](sizes).

## Memasukkan simbol

Klik **Sisipkan > Simbol > Simbol** untuk membuka kotak Simbol. Pilih karakter lalu klik **Sisipkan**, atau klik dua kali karakter tersebut. Karakter akan masuk di posisi kursor pada isian yang sedang Anda ketik. Jika Anda tidak sedang mengetik, isian sel itu sendiri akan terbuka dengan simbol ditambahkan di akhirnya, dan **Enter** memasukkannya ke sel. Contoh: dengan sel berisi kata *Total*, memilih tanda euro menghasilkan isian *Total€*. Simbol yang terakhir Anda gunakan ditampilkan lebih dulu.

**Ambil karakter dari sebuah font.** Pilih font di kotak **Font**, lalu pilih karakternya. Karakter itu masuk dalam bagian tersendiri yang diatur dalam font tersebut, dan sisa teks sel tetap memakai font sel. Contoh: pada sel berisi *Total*, tanda centang yang dipilih dari font simbol menghasilkan *Total✓*, dengan hanya tanda centang yang memakai font tersebut. Dengan **(teks normal)** yang dipilih, font sel menggambar simbol itu, sehingga simbol yang tidak dimiliki font akan tampak sebagai kotak. Dalam rumus, hanya karakter itu yang dimasukkan.

**Karakter khusus.** Tab **Karakter Khusus** mencantumkan spasi tanpa pemisah kata dan tanda hubung tanpa pemisah, tanda hubung opsional, tanda pisah en dan em, spasi en dan em, tanda kutip, serta pemisah tanpa lebar, masing-masing dengan namanya. Klik dua kali salah satunya untuk memasukkannya.

**Beri simbol tombol pintas.** Pilih simbol atau karakter khusus, klik **Tombol Pintas**, tekan tombol yang Anda inginkan, misalnya **Ctrl+Alt+Shift+J**, lalu klik **OK**. Tombol tersebut kemudian memasukkan simbol itu, sama seperti kotak simbol. Kosongkan tombol lalu klik **OK** untuk menghapusnya. JeffCrunch memberi tahu Anda jika tombol itu sudah dipakai perintah lain. Simbol itu menjadi sebuah perintah, sehingga juga tercantum di **Berkas > Opsi > Sesuaikan > Papan Ketik**, tempat Anda dapat mengubah tombolnya (lihat [Pintasan papan ketik](keyboard)).

## Buku kerja baru

Di **Berkas > Opsi > Buku Kerja Baru**, pilih font, ukuran font, dan jumlah lembar yang dimiliki setiap buku kerja baru sejak awal. Buku kerja yang sudah ada tidak diubah.
