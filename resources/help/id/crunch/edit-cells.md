# Mengetik di sel

<!-- keywords: simbol, font simbol, karakter khusus, tombol pintas untuk simbol, koreksi otomatis, tanda kutip cerdas, tanda hubung, pecahan, tautan, alamat, kata tebal, bagian sel, teks kaya, format sebagian teks, warnai kata, jalankan, ketik, mengetik, masukkan, edit, ubah, sel, entri, bilah rumus, F2, klik ganda, Tab, Esc, batalkan entri, urungkan, ulangi, hapus, bersihkan, persen, mata uang, uang, tanggal, waktu, angka, teks, tanda kutip tunggal, rumus, sama dengan, ganti, batas, karakter -->

Untuk memasukkan sesuatu ke dalam sel, pilih sel lalu mulailah mengetik. Apa pun yang Anda ketik akan menggantikan isi sel. Tekan **Enter** untuk menyimpannya, atau **Esc** untuk membatalkan entri dan membiarkan sel seperti semula.

## Tiga cara memulai

- **Ketik langsung di sel.** Pilih sel lalu ketik. Karakter pertama yang Anda ketik memulai entri, dan sel menampilkan apa yang Anda ketik selagi Anda mengetik.
- **Edit isi sel.** Tekan **F2**, atau klik ganda sel. Sel terbuka dengan isinya, dan kursor berada di akhir. Gunakan tombol panah untuk bergerak di dalam teks.
- **Gunakan bilah rumus.** Klik di dalam bilah rumus, yang berada di sebelah kotak Nama, lalu ketik di sana. Sel juga menampilkan apa yang Anda ketik.

Saat Anda mengetik langsung di sel, tombol panah menyelesaikan entri dan memindahkan pilihan. Saat Anda mengedit isi sel, tombol panah bergerak di dalam teks. Tekan **F2** untuk berpindah dari satu mode ke mode lain.

## Menyelesaikan entri

| Tombol | Fungsinya |
| --- | --- |
| Enter | Simpan entri, dan pindah ke bawah (atau sesuai pengaturan di **File > Opsi > Pengeditan**) |
| Shift+Enter | Simpan entri, dan pindah ke arah sebaliknya |
| Ctrl+Enter | Masukkan entri ke semua sel yang dipilih, dan tetap di tempat |
| Tab | Simpan entri, dan pindah ke kanan |
| Shift+Tab | Simpan entri, dan pindah ke kiri |
| Tombol panah | Simpan entri, dan pindah satu sel (hanya saat Anda mengetik langsung di sel) |
| Esc | Batalkan entri |
| Alt+Enter | Mulai baris baru di dalam sel |
| Ctrl+; | Masukkan tanggal hari ini (di dalam entri, saat Anda sedang mengetik) |
| Ctrl+Shift+; | Masukkan waktu sekarang |
| Alt+Panah Bawah | Tampilkan daftar kata yang sudah ada di kolom, lalu pilih satu |

**Ctrl+Enter** dengan beberapa sel terpilih memasukkan apa yang Anda ketik ke semuanya. Rumus ditulis untuk sel tempat Anda mengetik, dan di sel-sel lain rumus itu dibaca sebagai salinan darinya: mengetik **=A1** dengan blok E6:F7 terpilih menghasilkan **=B2** di F7. Ini merupakan satu langkah Urungkan.

**Alt+Panah Bawah** pada sebuah sel menampilkan, sesuai urutan dan tanpa pengulangan, kata-kata dalam sel di atas dan di bawahnya yang tidak dipisahkan oleh sel kosong. Memilih salah satu akan mengetiknya. Sel yang memiliki daftar sendiri justru menampilkan daftar tersebut.

Di **File > Opsi > Pengeditan**, Anda dapat memilih apakah **Enter** memindahkan pilihan ke bawah, kanan, atas, atau kiri, atau tidak memindahkannya sama sekali.

Jika Anda mengetik di beberapa sel sepanjang satu baris dengan **Tab**, lalu menekan **Enter**, pilihan akan turun ke baris berikutnya, di kolom tempat Anda mulai. Jika Anda memilih satu blok sel terlebih dahulu, **Enter** dan **Tab** bergerak di dalam blok tersebut.

## Koreksi Otomatis saat mengetik

Saat Anda mengetik spasi atau tanda baca seperti titik setelah sebuah kata, JeffCrunch dapat memperbaiki kata tersebut selagi Anda menyelesaikannya. Fitur ini memperbaiki kesalahan ketik yang umum (misalnya "teh" menjadi "the"), dua huruf kapital di awal kata, huruf kecil yang memulai kalimat atau nama hari, dan tanda kutip lurus (menjadi tanda kutip melengkung). Fitur ini juga mengubah dua tanda hubung di antara kata menjadi tanda pisah panjang, serta 1/2, 1/4, dan 3/4 menjadi ½, ¼, dan ¾ bila entri memuat kata. Angka seperti 1/2 yang diketik sendirian tetap menjadi tanggal atau pecahan, seperti biasa.

Setiap perbaikan merupakan langkah tersendiri. Tekan **Ctrl+Z** tepat setelah sebuah perbaikan untuk membatalkan perbaikan itu saja dan mempertahankan apa yang Anda ketik. Tidak ada yang diubah dalam rumus: entri yang diawali tanda sama dengan, atau tanda plus atau minus lalu rumus, tetap seperti yang Anda ketik. Alamat web (misalnya https://example.org) atau alamat email yang menjadi seluruh entri akan menjadi tautan di selnya saat Anda menyelesaikan entri, jika **Ubah alamat web menjadi tautan saat mengetik** diaktifkan.

Aktifkan atau nonaktifkan semua ini di **File > Opsi > Pemeriksaan**. Kata yang salah eja ditandai selagi Anda mengetik: lihat [Ejaan](notes-links).

## Memformat sebagian teks

Beberapa kata dalam sel dapat tampil berbeda dari sisanya. Misalnya, satu kata dapat dibuat tebal atau merah.

1. Tekan F2 untuk mengedit sel (atau klik ganda sel).
2. Pilih kata yang ingin diubah, dengan Shift dan tombol panah atau dengan mouse.
3. Gunakan **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Font**, **Ukuran Font**, atau **Warna Font** pada tab **Beranda**, atau Ctrl+B, Ctrl+I, dan Ctrl+U.
4. Tekan Enter.

Hanya kata yang Anda pilih yang berubah. Bagian sel lainnya tetap mengikuti tampilan sel itu sendiri. Kotak entri menampilkan kata-kata dalam satu tampilan selagi Anda mengedit, dan sel menampilkan setiap bagian dengan tampilannya sendiri saat Anda menekan Enter. Jika kemudian Anda mengetik atau menghapus kata, pemformatannya ikut berpindah bersama kata tersebut. Memilih font atau ukuran tanpa kata yang dipilih mengubah seluruh sel, seperti biasa.

## Bilah mini

Sebuah bilah kecil berisi pemformatan yang paling sering Anda gunakan muncul mengambang di tempat Anda bekerja, sehingga Anda tidak perlu naik ke pita. Bilah ini tidak pernah mengambil alih keyboard dari sel: apa yang sedang Anda ketik tetap di tempatnya.

- **Teks terpilih dalam sel:** pilih kata dengan mouse saat Anda mengedit sel, dan bilah akan mengambang di atasnya. Bilah ini memiliki kotak **font** dan **ukuran**, **Tebal**, **Miring**, **Garis Bawah**, **Tambah Ukuran Font**, **Kurangi Ukuran Font**, dan **Warna Font**. Perintah tersebut hanya mengubah kata yang Anda pilih, sama seperti di pita.
- **Sel terpilih:** klik kanan sel, dan bilah berada di bagian atas menu pintas. Bilah ini juga memiliki **Warna Isian**, **Bingkai**, **Mata Uang**, **Gaya Persen**, **Gaya Koma**, **Tambah Desimal**, **Kurangi Desimal**, **Gabungkan dan Tengahkan**, dan **Salin Format**. Ini adalah perintah pita itu sendiri, dan masing-masing merupakan satu langkah yang dapat diurungkan.

Bilah di atas teks terpilih memudar saat Anda menggerakkan penunjuk menjauh darinya, dan hilang bila penunjuk sudah sangat jauh. Bilah juga hilang saat Anda mengetik, menekan Esc, atau menggulir. Untuk mematikan bilah, hapus centang **Tampilkan Bilah Alat Mini saat Memilih** pada halaman **Umum** di **File > Opsi**; aplikasi JeffOffice lainnya mengikuti pilihan ini. Tombol Analisis Cepat yang muncul di bawah rentang terpilih tidak tersedia pada versi ini.

Sel yang memiliki pemformatan di dalam teksnya dari program lain akan menampilkannya, mempertahankannya, dan menyimpannya kembali. Sel yang Anda ubah tetap mempertahankan pemformatannya. Pemformatan di dalam teks hanya berlaku untuk teks, bukan untuk angka atau rumus. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tetap menyimpan kata-katanya, dan penyimpanan akan memberi tahu bahwa pemformatan di dalamnya tidak dipertahankan.

## Yang Anda ketik menjadi

JeffCrunch membaca apa yang Anda ketik sesuai pengaturan komputer Anda. Tanda desimal, tanda ribuan, simbol mata uang, dan urutan hari serta bulan dalam tanggal semuanya berasal dari pengaturan tersebut. Angka yang Anda ketik di sel yang belum memiliki format sendiri akan diberi format yang menampilkannya seperti cara Anda mengetiknya.

| Anda mengetik | Sel berisi | Dan menampilkan |
| --- | --- | --- |
| 1234.5 | angka 1234.5 | 1234.5 |
| 5% | angka 0,05 | 5% |
| $12.50 | angka 12,5 | $12.50 |
| 1 1/2 | angka 1,5 | 1 1/2 |
| 3/4/2025 | tanggal, sebagai jumlah hari | 3/4/2025 |
| 10:30 PM | waktu, sebagai bagian dari hari | 10:30 PM |
| TRUE | nilai logika benar | TRUE |
| #N/A | galat #N/A | #N/A |
| =B2*2 | rumus | hasil rumus |
| selain itu | teks | teksnya |

Sel yang sudah memiliki format sendiri, seperti persen atau tanggal, tetap mempertahankannya. Sel dengan format **Teks** menyimpan semua yang Anda ketik sebagai teks, kecuali rumus.

Untuk menyimpan sesuatu yang tampak seperti angka sebagai teks, seperti kode **007**, ketik tanda kutip tunggal terlebih dahulu: **'007**. Tanda kutip tunggal itu bukan bagian dari teks. Saat Anda mengedit sel, tanda tersebut kembali terlihat.

## Rumus

Rumus diawali dengan tanda sama dengan. Misalnya, **=B2*2** menggandakan angka di B2. Nama fungsi boleh Anda ketik dengan huruf kapital atau huruf kecil; JeffCrunch akan menulisnya dengan huruf kapital.

Nama fungsi selalu memakai nama bahasa Inggris, di semua bahasa: ketik **SUM**, bukan nama yang diberikan bahasa Anda. Tanda pemisah mengikuti pengaturan komputer Anda. Jika tanda desimal berupa koma, seperti dalam bahasa Jerman dan Prancis, Anda memisahkan argumen fungsi dengan titik koma dan menulis koma pada angka: **=SUM(A1;A2)*0,5**. Bilah rumus dan **F2** menampilkan rumus dengan cara yang sama. Jika tanda desimal berupa titik, Anda memisahkan argumen dengan koma: **=SUM(A1,A2)*0.5**. Berkas selalu menyimpan bentuk yang dibaca setiap program, dengan koma sebagai pemisah argumen dan titik pada angka, sehingga berkas terbuka dengan sama di mana pun.

Jika sebuah rumus memiliki kesalahan, misalnya tanda kurung yang tidak ditutup, JeffCrunch akan memberi tahu apa yang salah, dan entri tetap seperti yang Anda ketik, dengan kursor di dekat kesalahan, agar Anda dapat memperbaikinya. Tidak ada yang diubah sampai rumus benar.

Sebuah sel dapat menampung hingga 32.767 karakter, dan sebuah rumus hingga 8.192 karakter. JeffCrunch memberi tahu Anda jika entri lebih panjang dari itu.

## Menghapus isi sel

Pilih sel lalu tekan **Delete**. Ini menghapus isi sel dan mempertahankan formatnya, dan merupakan satu langkah yang dapat diurungkan. JeffCrunch dapat menghapus hingga 100.000 sel sekaligus.

Pada tab **Beranda**, tombol **Hapus** memiliki menu. **Hapus Semua** mengosongkan sel, formatnya, catatannya, komentarnya (utas dengan balasan), dan tautannya. Utas dengan lebih dari 500 komentar tetap dipertahankan, dan JeffCrunch memberi tahu Anda, karena Urungkan tidak dapat mengembalikan komentar yang tidak sempat dibaca. **Hapus Format** mempertahankan isi sel dan mengembalikan format biasa. **Hapus Isi** sama dengan **Delete**. **Hapus Komentar dan Catatan** hanya menghapus catatan dan komentar dari sel yang dipilih. **Hapus Tautan** hanya menghapus tautan, dan sel tetap memiliki teksnya. Pilihan yang tidak berlaku akan diredupkan bila sel yang dipilih tidak memiliki apa pun untuk dihapus.

## Urungkan dan Ulangi

**Ctrl+Z** mengurungkan perubahan terakhir, dan **Ctrl+Y** mengulanginya kembali. Keduanya juga ada di bilah alat cepat di bagian atas. Setiap sel yang Anda ketik merupakan satu langkah, begitu pula setiap penghapusan. Urungkan mengembalikan sel seperti semula: nilainya, rumusnya, formatnya, serta rumus-rumus yang bergantung padanya. JeffCrunch mengingat hingga 1.000 langkah, lebih sedikit jika langkah-langkah tersebut mengubah sangat banyak sel, agar tidak memakan banyak memori.

**Kembali beberapa langkah sekaligus.** Pada bilah alat cepat, **Urungkan** dan **Ulangi** masing-masing memiliki panah di sebelahnya. Klik panah untuk melihat langkah-langkahnya, yang terbaru di atas (hingga 100 langkah ditampilkan). Gerakkan penunjuk ke bawah daftar untuk menandai berapa langkah yang Anda maksud (baris di bawah daftar menyebutkan, misalnya, "Urungkan 3 tindakan"), lalu klik untuk menjalankannya. Memilih langkah ketiga akan mengurungkan tiga langkah pertama. Langkah-langkah tetap berurutan, sehingga Ulangi mengembalikannya dengan cara yang sama.

## Ulangi

**F4** menjalankan kembali perintah terakhir Anda pada sel yang sekarang dipilih. Contoh: buat **A1** tebal, pilih **C4:C9**, lalu tekan **F4**, dan sel-sel di C4:C9 menjadi tebal, dalam satu langkah yang dapat diurungkan. Ulangi berlaku untuk pemformatan, gaya sel, penghapusan, serta penyisipan dan penghapusan baris dan kolom. Tip pada tombol, atau perintah **Ulangi**, menyebutkan apa yang akan dilakukan ("Ulangi Tebal"). **Ctrl+Y** juga mengulangi, tetapi hanya bila tidak ada langkah untuk diulang kembali (selama ada, **Ctrl+Y** adalah Ulangi). Saat Anda mengetik rumus, **F4** tetap mengubah referensi antara relatif dan tetap, seperti sebelumnya. Perintah yang membuka kotak dialog, serta pengetikan di sel, tidak diulang.

## Selagi JeffCrunch menghitung rumus

Saat Anda mengubah sebuah sel, JeffCrunch menghitung rumus-rumus yang bergantung padanya di latar belakang. Sel yang Anda ubah langsung menampilkan isi barunya, dan sel-sel lain diperbarui sesaat kemudian; bilah status menampilkan **Menghitung** sampai proses selesai. Anda tetap dapat mengetik selama itu.

## Yang tidak dapat diubah

- Sel yang merupakan bagian dari **rumus array** atau tabel data tidak dapat diubah sendiri. Untuk membuat rumus array, pilih sel-sel yang akan diisinya, ketik rumusnya, lalu tekan **Ctrl+Shift+Enter**. Satu rumus kemudian mengisi semua sel, dan bilah rumus menampilkannya dalam kurung kurawal, misalnya **{=A1:A3*2}**. Untuk mengubahnya, pilih salah satu sel array, tekan **F2**, ubah rumusnya, lalu tekan **Ctrl+Shift+Enter** lagi. Untuk menghapusnya, pilih semua selnya dan tekan **Delete**. Keduanya berlaku untuk seluruh array, dalam satu langkah Urungkan. F2 pada sel mana pun di dalam array membuka rumus array tersebut, dan lembar yang dilindungi atau rentang berkata sandi akan menolak atau meminta sandi seperti untuk sel lainnya. Menyalin array, atau sebagian darinya, dan mengisi dari salah satu selnya, menghasilkan nilai yang ditampilkannya, bukan rumus.
- Pada **lembar yang dilindungi**, sel yang terkunci tidak dapat diubah. Setiap sel terkunci kecuali buku kerja menyatakan sebaliknya. JeffCrunch memberi tahu Anda saat Anda mencobanya. Untuk mengubah sel, pilih Hapus Perlindungan Lembar pada tab Tinjau.
- Baris pertama sebuah **tabel** berisi nama kolomnya. Ketik nama baru di salah satu sel tersebut untuk mengganti nama kolom: rumus yang memakai nama lama akan ikut berubah, dan tabel dalam berkas tetap valid. Nama yang sudah dimiliki kolom lain akan diberi angka di belakangnya, nama tidak boleh kosong, dan **Delete** tidak menghapus baris itu.
- Buku kerja yang tidak dapat dibaca secara utuh hanya dapat dilihat, tidak dapat diubah atau disimpan.
- Fungsi yang belum dimiliki JeffCrunch membuat rumus baru tanpa nilai di selnya; buku kerja akan meminta program yang membukanya untuk menghitung rumus tersebut.
- Pemformatan, pengurutan, dan temukan serta ganti akan hadir pada versi mendatang. Penyisipan dan penghapusan, potong dan tempel, isi, serta ukuran baris dan kolom dibahas di [Sisipkan dan hapus baris, kolom, dan sel](insert-delete), [Potong, salin, dan tempel](clipboard), [Isi sel dan jumlahkan](fill-sum), dan [Ubah lebar kolom dan tinggi baris](sizes).

## Memasukkan simbol

Klik **Sisipkan > Simbol > Simbol** untuk membuka kotak Simbol. Pilih karakter lalu klik **Sisipkan**, atau klik ganda karakter tersebut. Karakter akan masuk di posisi kursor entri yang sedang Anda ketik. Jika Anda tidak sedang mengetik, entri sel itu sendiri akan terbuka dengan simbol ditambahkan di akhirnya, dan **Enter** memasukkannya ke sel. Contoh: dengan sel berisi kata *Total*, memilih tanda euro akan menghasilkan entri *Total€*. Simbol yang terakhir Anda gunakan ditampilkan lebih dulu.

**Ambil karakter dari sebuah font.** Pilih font di kotak **Font**, lalu pilih karakternya. Karakter tersebut masuk dalam bagian tersendiri yang diatur dengan font itu, dan sisa teks sel mempertahankan font sel. Contoh: pada sel berisi *Total*, sebuah tanda centang yang dipilih dari font simbol menghasilkan *Total✓*, dengan hanya tanda centang yang memakai font tersebut. Dengan **(teks normal)** yang dipilih, font sel menggambar simbol, sehingga simbol yang tidak dimiliki font akan tampak sebagai kotak. Dalam rumus, hanya karakternya yang dimasukkan.

**Karakter khusus.** Tab **Karakter Khusus** mencantumkan spasi tanpa pemisah kata dan tanda hubung, tanda hubung opsional, tanda pisah en dan em, spasi en dan em, tanda kutip, serta pemisah tanpa lebar, masing-masing dengan namanya. Klik ganda salah satunya untuk memasukkannya.

**Beri simbol tombol pintas.** Pilih simbol atau karakter khusus, klik **Tombol Pintas**, tekan tombol yang Anda inginkan, misalnya **Ctrl+Alt+Shift+J**, lalu klik **OK**. Tombol tersebut kemudian memasukkan simbol itu, dengan cara yang sama seperti kotak. Biarkan tombol kosong dan klik **OK** untuk menghapusnya. JeffCrunch memberi tahu Anda jika perintah lain sudah memakai tombol tersebut. Simbol menjadi sebuah perintah, sehingga juga tercantum di **File > Opsi > Sesuaikan > Papan Ketik**, tempat Anda dapat mengubah tombolnya (lihat [Pintasan keyboard](keyboard)).

## Buku kerja baru

Di **File > Opsi > Buku Kerja Baru**, pilih font, ukuran font, dan jumlah lembar yang dimiliki setiap buku kerja baru. Buku kerja yang sudah ada tidak diubah.
