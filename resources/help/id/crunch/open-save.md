# Membuka dan menyimpan buku kerja
<!-- keywords: pulihkan pekerjaan yang belum disimpan, pemulihan otomatis, macet, salin, format lama, opendocument, kompatibilitas, baru, kosong, book1, tanpa judul, buka, simpan, simpan sebagai, berkas, buku kerja, xlsx, xlsm, xls, ods, csv, tsv, berkas teks, dibatasi pemisah, impor, ekspor, terbaru, disematkan, seret, lepas, memuat, besar, makro, belum disimpan, perubahan, tutup, salinan, rantai penghitungan, templat, xltx, xltm, tidak dapat dibuka, rusak, terenkripsi, kata sandi, simpan salinan, hanya baca, baca saja, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ots, perbaiki, buka dan perbaiki, templat bawaan, anggaran, faktura, lembar waktu kerja, pinjaman, kalender, buku nilai, lebar tetap, kolom tetap, pemisah desimal, pemisah ribuan, minus di belakang, baris perintah, opsi pemisah, opsi pengodean, opsi desimal, html, halaman web, htm, tabel dari halaman, tabel dari dokumen, pemisah, buka berkas teks, file

Saat JeffCrunch dimulai tanpa berkas yang akan dibuka, jendela berisi buku kerja baru yang kosong dengan satu lembar bernama Book1. Setiap buku kerja baru mendapat nomor berikutnya (Book2, Book3, dan seterusnya), dan nomor yang sudah dipakai tidak akan dipakai lagi setelah jendelanya ditutup. Untuk memulai buku kerja lain, tekan **{key:New}**, atau klik **File**, lalu **Baru**, dan pilih **Buku Kerja Kosong**; buku kerja itu terbuka di jendela tersendiri. Buku kerja baru yang belum Anda ubah akan digantikan ketika Anda membuka berkas di jendelanya; buku kerja yang sudah Anda ubah tetap ada, dan berkas itu dibuka di jendela lain. **Simpan** pada buku kerja baru meminta nama, seperti **Simpan Sebagai**, dan menyimpannya sebagai berkas .xlsx. Menutup buku kerja baru yang belum Anda ubah tidak menanyakan apa pun.

Untuk membuka buku kerja, tekan **{key:Open}**, atau klik **File** lalu **Buka**, dan pilih **Telusuri**. Anda juga dapat menyeret buku kerja dari pengelola berkas ke jendela. Buku kerja yang sudah terbuka di sebuah jendela tidak dibuka lagi: jendela itu yang tampil di depan.

JeffCrunch membuka buku kerja **.xlsx** dan buku kerja **.xlsm** (yang berisi makro), serta templat **.xltx** dan **.xltm** (templat yang berisi makro). JeffCrunch tidak menjalankan makro dan membiarkannya tetap utuh: proyek makro, tanda tangannya, serta bagian yang mengacu padanya ditulis kembali persis seperti semula. Templat disimpan sebagai templat, dan buku kerja dengan makro sebagai buku kerja dengan makro, karena jenis berkas itu bagian dari berkasnya.

## Memulihkan pekerjaan yang belum disimpan

Jika JeffCrunch berhenti mendadak, misalnya karena listrik padam, saat dimulai berikutnya akan tampil daftar salinan pekerjaan Anda yang belum disimpan. Pilih salinan yang ingin dibuka dan klik **Buka**; setiap salinan terbuka sebagai buku kerja baru yang belum memiliki berkas, jadi klik **Simpan** dan pilih nama. **Hapus** membuang salinan yang dicentang, dan **Jangan Sekarang** menyimpannya untuk nanti. Anda dapat meminta daftar ini kapan saja dengan **File > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Lihat [Ubah pengaturan JeffCrunch](options) untuk mengetahui seberapa sering salinan dibuat.

## Jenis berkas lain

JeffCrunch juga membuka buku kerja .xls lama, lembar sebar OpenDocument (**.ods** dan **.fods** yang datar), serta berkas teks berisi nilai (**.csv**, **.tsv**, dan **.txt**). Buku kerja yang dibuka dari salah satunya disimpan sebagai berkas .xlsx baru (atau sebagai salinan .xls atau .ods: lihat di bawah). Saat Anda menekan **{key:Save}**, JeffCrunch memberi tahu hal itu dan meminta nama, dan berkas yang Anda buka tidak pernah diubah. Gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks dalam berkas .ods masuk ke berkas baru dengan posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, dan garis tepinya; bagannya ikut, digambar dari sel yang dibacanya.

Lembar sebar OpenDocument dibaca beserta lembarnya, nilainya, rumusnya (dengan hasil yang disimpan berkas), format angka, font, isian, bingkai, sel gabungan, lebar kolom dan tinggi baris, baris dan kolom tersembunyi, pembekuan panel, dan namanya. Bagan, gambar dan gambar lainnya, komentar, tabel pivot, makro, format bersyarat, dan validasi data belum dibaca; sebuah pesan di bagian atas memberi tahu Anda apa saja yang dimiliki berkas itu, dan salinan yang disimpan dari sini tidak akan memilikinya. Lembar sebar yang dilindungi kata sandi akan memintanya saat dibuka: lihat [Membuka buku kerja yang dilindungi kata sandi](open-password).

Berkas teks adalah satu lembar. JeffCrunch mencari tahu bagaimana nilainya dipisahkan (dengan koma, titik koma, tab, atau garis tegak) dan bagaimana hurufnya dikodekan (UTF-8 atau UTF-16, atau halaman kode bahasa Anda), lalu membaca setiap nilai seperti jika Anda mengetiknya dalam bahasa Anda: angka seperti 1.234,5 dalam bahasa Jerman, persen, jumlah uang, tanggal dengan tahunnya (seperti 3/15/2024, atau 15.03.2024 dalam bahasa Jerman), waktu, serta TRUE atau FALSE (dalam bahasa Anda, seperti WAHR dan FALSCH dalam bahasa Jerman, dan dalam bahasa Inggris). Nilai yang diawali tanda sama dengan adalah rumus. Digit yang diawali angka nol (seperti 007) dan angka dengan lebih dari 15 digit tetap berupa teks, sehingga kode pos atau nomor rekening tidak berubah; begitu pula tanggal dengan hari dan bulan tanpa tahun, dalam urutan apa pun dan bahasa apa pun (seperti 3/4, 12.5, 1-2, March 1, atau Dec 25), karena tahunnya harus dikarang, serta teks seperti nama gen MARCH1. Satu lembar memuat 1.048.576 baris dan 16.384 kolom; jika berkas memiliki lebih banyak, JeffCrunch membuka bagian yang muat dan memberi tahu Anda.

Saat Anda membuka berkas teks, sebuah jendela lebih dulu menampilkan baris pertamanya dan menanyakan cara membacanya: apa yang memisahkan kolom (koma, titik koma, tab, spasi, garis tegak, atau karakter yang Anda ketik, dengan beberapa pemisah berurutan dihitung sebagai satu jika Anda mau), apa yang memberi tanda kutip pada nilai yang memuat pemisah, pengodean teks (Unicode, halaman kode Windows, halaman ISO 8859, Mac Roman, dan halaman Asia Timur), serta baris tempat mulai membaca. Di atas setiap kolom, Anda memilih cara membacanya: **Seperti yang diketik**, **Teks** (untuk mempertahankan nomor suku cadang seperti 007 atau nomor rekening yang panjang apa adanya), **Tanggal** dengan urutan yang Anda tentukan, atau **Abaikan**. Sebagian berkas menyejajarkan nilainya dalam kolom berlebar sama, dengan spasi di antaranya dan tanpa pemisah. Centang **Kolom memiliki lebar tetap**, lalu baris awal berkas akan tampil dalam kotak. Klik di antara dua karakter untuk memulai kolom di sana, dan klik lagi untuk menghapusnya; atau ketik posisinya di **Kolom dimulai pada**, misalnya 10, 25, 40. Setiap bagian dipotong di posisi tersebut, dan spasi di sekitarnya dihapus. **Pemisah desimal** dan **Pemisah ribuan** memberi tahu JeffCrunch cara menulis angka dalam berkas ini, apa pun yang dipakai bahasa Anda, sehingga 1.234,50 berarti seribu dua ratus tiga puluh empat setengah jika berkas memakai koma sebagai pemisah desimal. Centang **Tanda minus bisa berada setelah angka** untuk berkas yang menulis -12 sebagai 12-. Pratinjau mengikuti setiap perubahan dan hanya menampilkan paling banyak 200 baris pertama dan 100 kolom, tetapi seluruh berkas tetap dibuka. **File > Buka > Buka Berkas Teks** akan menanyakan hal ini meskipun nama berkasnya tidak biasa.

Anda juga dapat menentukan cara membaca berkas teks saat memulai JeffCrunch dari baris perintah, sehingga tidak ada jendela yang bertanya. **--separator** menentukan apa yang memisahkan kolom: **comma** (koma), **semicolon** (titik koma), **tab**, **space** (spasi), **bar** (garis tegak), atau satu karakter. **--encoding** menentukan pengodean hurufnya: **utf-8**, **utf-8-bom**, **utf-16le**, **utf-16be**, **latin1**, **mac-roman**, **windows-1252** (atau halaman lain seperti **windows-1250**), atau **iso-8859-N**. **--decimal** menentukan tanda desimal pada angka: **point** (titik) atau **comma** (koma). Jika salah satunya dihilangkan, JeffCrunch mencari tahu bagian itu seperti biasa. Contoh: JeffCrunch --separator semicolon --decimal comma --encoding windows-1252 harga.csv. Nilai yang tidak dimengerti JeffCrunch (pemisah kosong, akhir baris sebagai pemisah, atau karakter yang sama untuk pemisah dan tanda desimal) akan diberitahukan di layar, dan tidak ada yang dibuka. Opsi ini hanya berlaku untuk berkas teks yang disebut pada baris perintah itu. Berkas yang Anda buka kemudian dengan **File > Buka**, atau seret ke jendela, akan ditanyakan seperti biasa.

JeffCrunch juga membuka **halaman web** (**.html** dan **.htm**), dan halaman yang disimpan dengan nama buku kerja. Setiap tabel dalam halaman menjadi satu lembar, bernama **Table1**, **Table2**, dan seterusnya. Sel dibaca seolah Anda mengetiknya: angka tetap angka, dan tanggal tetap tanggal. Sel yang digabung di beberapa kolom atau baris akan menyisakan sel kosong, sehingga sel lain tetap berada di kolomnya. Kata-kata di luar tabel, gambar, dan tampilan halaman tidak dibaca, dan halaman tanpa tabel tidak akan terbuka. Tidak ada yang dinamai halaman itu yang diambil dari internet.

Untuk menyimpan hanya nilai dari lembar yang sedang Anda lihat, klik **File**, lalu **Simpan Sebagai**, dan pilih jenis berkas **.csv** (atau **.tsv** dan **.txt**, dipisahkan tab). Sebelum menulis berkas, JeffCrunch memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat disimpan dalam berkas itu (misalnya lembar lain, rumus, gambar, bagan, catatan, dan makro), lalu menanyakan cara menulisnya, dan apakah akan **Simpan Nilai Saja**, **Simpan sebagai .xlsx** untuk menyimpan semuanya, atau **Batal**:

- **Pemisah.** Koma, titik koma, tab, spasi, garis tegak, atau karakter yang Anda ketik. Berkas **.csv** memakai titik koma jika bahasa Anda menulis koma pada angkanya, dan koma di tempat lain; berkas **.tsv** atau **.txt** memakai tab.
- **Pengodean teks.** UTF-8 dengan tanda (agar program lain membaca huruf beraksen dengan benar), UTF-8 tanpa tanda, UTF-16, atau halaman kode seperti Eropa Barat (1252), halaman ISO 8859, Mac Roman, serta halaman Jepang, Tionghoa, Korea, Ibrani, Arab, Baltik, Vietnam, dan Thai. Huruf yang tidak memiliki byte di halaman kode itu ditulis sebagai tanda tanya, dan bilah status menyebutkan jumlahnya.
- **Akhir baris.** Windows (carriage return dan line feed), Linux dan Mac (line feed), atau Mac lama (carriage return).
- **Sel.** **Seperti yang ditampilkan di sel** menulis apa yang Anda lihat. **Seperti yang disimpan** menulis setiap digit angka (0,1 dan bukan 10%), dan tanggal sebagai teks ISO seperti 2026-03-04.

Teks diberi tanda kutip jika memuat pemisah, tanda kutip, akhir baris, atau spasi di kedua ujungnya. Lembar lain, rumus, format, dan pengaturan tidak masuk ke berkas, dan buku kerja Anda tidak berubah karena menulis berkas seperti ini.

Buku kerja yang berasal dari berkas teks ditulis kembali dalam bentuk seperti semula: pemisah, akhir baris, dan pengodean yang sama. Jika Anda menyimpannya sebagai salinan tanpa mengubah sel, salinannya sama persis dengan berkas asli, byte demi byte, dan tidak ada yang ditanyakan.

## Menyimpan salinan sebagai .xls atau .ods

Untuk membuat salinan yang dapat dibuka program lama, atau program yang membaca OpenDocument, klik **File**, lalu **Simpan Sebagai**, dan pilih jenis berkas **.xls** atau **.ods**. Salinan dibuat dari buku kerja Anda seperti keadaannya sekarang, dengan perubahan yang belum Anda simpan, dan memuat:

- nilai, beserta teksnya, angkanya, nilai benar-salahnya, dan galatnya;
- rumus, dengan hasil yang dimilikinya sekarang (rumus array tetap sebagai rumus array);
- format angka, font, isian, bingkai, perataan, dan apakah sel dikunci;
- sel gabungan, lebar kolom dan tinggi baris, baris dan kolom tersembunyi, garis tepi baris dan kolom, serta pembekuan panel;
- lembar, dengan urutan, nama, dan mana yang tersembunyi, serta nama yang Anda tentukan (dan area cetaknya);
- bagan setiap lembar (hanya dalam salinan **.ods**): kolom, batang, garis, area, lingkaran, donat, dan bagan sebar, dengan seri dan sel yang dibacanya, kategori, judul, keterangan, sumbu (judul, skala, format angka), label data, warna, dan tempatnya (yang tidak dapat disimpan bagan .ods, seperti garis tren, bilah kesalahan, dan sumbu sekunder, akan disebutkan dalam pesan);
- gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks setiap lembar (hanya dalam salinan **.ods**), dengan posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, garis tepi, kata-katanya, dan gambar itu sendiri beserta pemotongannya (Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument);
- tautan, dan cara setiap lembar dicetak (kertas, margin, orientasi, skala, header dan footer, serta pemisah halaman).

Berkas **.xls** juga memuat tabel data, dan menyimpan pengaturan penghitungan buku kerja. Salinan tidak dapat memuat semuanya. Sebelum JeffCrunch menulis berkas, ia memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat masuk ke berkas itu: bagan, gambar dan gambar lainnya (untuk salinan .xls; salinan .ods menyimpannya, termasuk bagan, kecuali diagram), catatan, tabel pivot, tabel, format bersyarat, validasi data, filter, proteksi lembar, kontrol formulir, makro, dan tautan ke buku kerja lain (rumus yang memakainya tetap menyimpan nilai yang dimilikinya). Rumus yang tidak dapat dinyatakan dalam bentuk lama (fungsi yang tidak dikenalnya, atau sel di luar kolom IV atau baris 65.536) ditulis sebagai nilai hasilnya, dan pesan menghitung jumlahnya. Bentuk lama juga berhenti di 65.536 baris, 256 kolom, 255 lembar, dan 32.767 karakter per sel, serta sekitar 4.000 format sel dan 56 warna: yang tidak muat akan dipotong, dan pesan menyebutkan jumlahnya. Anda kemudian dapat memilih **Simpan Salinan**, **Simpan sebagai .xlsx** untuk menyimpan semuanya, atau **Batal**.

Buku kerja Anda tidak berubah karena pembuatan salinan, dan tidak ditandai sebagai tersimpan. Berkas yang Anda buka tidak pernah ditulis ulang: beri salinan nama lain. Salinan dari berkas .xls atau .ods yang Anda buka dapat disimpan sebagai jenis yang lain.

## Simpan salinan

Klik **File > Simpan Salinan** dan pilih nama. JeffCrunch menulis buku kerja ke berkas itu, dengan perubahan yang belum Anda simpan, dan Anda tetap bekerja di buku kerja semula. Nama dan perubahan yang belum disimpan tetap seperti semula. Anda dapat memilih jenis berkas apa pun yang ditawarkan **Simpan Sebagai**. Salinan tidak pernah ditulis di atas berkas buku kerja itu sendiri.

## Simpan salinan cadangan

Klik **File > Opsi**, lalu pada halaman **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffCrunch lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, program akan menanyakan apakah tetap disimpan tanpa cadangan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula.

## Simpan sebagai templat

Templat adalah buku kerja yang Anda gunakan sebagai titik awal. Templat dapat memuat rumus, gaya, dan lembar. Klik **File > Simpan sebagai Templat** dan ketik nama. Templat (berkas **.xltx**) disimpan di **Templat Saya**, dan buku kerja Anda tidak berubah. Pada **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.xltx)**, atau **Templat lembar sebar OpenDocument (*.ots)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai buku kerja dari templat, klik **File > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Buku kerja baru terbuka dengan sel dan lembar dari templat itu. Buku kerja itu belum memiliki berkas sendiri, jadi **Simpan** akan meminta nama, dan templatnya tidak berubah. Untuk menghapus templat dari daftar, klik kanan padanya dan pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Cara Lain untuk Membuka**. Anda juga dapat mengklik kanan sebuah nama pada daftar Terbaru.

- **Buka Hanya-Baca** membuka buku kerja hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah sel. Bilah judul menampilkan "Hanya Baca". Jika Anda menekan **Simpan**, JeffCrunch meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka buku kerja tanpa berkas sendiri. **Simpan** akan menanyakan lokasinya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka buku kerja di jendela tersendiri, bahkan ketika jendela yang sedang Anda gunakan kosong.
- **Buka dan Perbaiki** untuk berkas .xlsx atau .xls yang rusak. Lihat di bawah.

## Buka dan Perbaiki

Kadang-kadang berkas .xlsx menjadi rusak. Unduhan yang terhenti di tengah jalan, atau disk yang membuat kesalahan. Berkas .xlsx adalah kumpulan bagian yang lebih kecil, seperti lembar, gaya, dan teks bersama. **Buka dan Perbaiki** membaca setiap bagian yang masih dapat dibaca. Bagian yang terhenti di tengah dipertahankan sampai tempat ia berhenti, sehingga Anda mungkin hanya mendapatkan baris pertama dari sebuah lembar. Bagian yang tidak dapat dibaca dilewati. Jika daftar bagian atau tautan di antaranya hilang, JeffCrunch membuatnya kembali.

Buku kerja hasil perbaikan terbuka tanpa berkas sendiri, dengan nama seperti "Anggaran yang diperbaiki". Sebuah pesan memberi tahu Anda apa yang dipertahankan sampai titik putus dan apa yang dilewati. Berkas yang rusak tidak diubah. Simpan buku kerja hasil perbaikan dengan nama baru, dan periksa angkanya sebelum Anda mempercayainya.

### Berkas .xls

Berkas .xls juga dapat rusak. Jika pembungkus luarnya terpotong atau daftarnya rusak, **Buka dan Perbaiki** mengikuti pembungkus sejauh yang dapat dibaca, dan jika itu gagal, mencari awal buku kerja di dalam berkas lalu membaca dari sana. Program memulihkan sebanyak mungkin. Lembar yang terpotong mempertahankan sel sebelum potongan, dan apa pun yang berada setelah potongan itu hilang. Jika daftar yang menyatakan bagaimana potongan berkas saling terhubung rusak, program mengambil setiap potongan untuk mengikuti potongan sebelumnya, dan menampilkan apa yang ditemukannya di sana. Sel-sel itu mungkin milik salinan lama lembar atau bagian lain dari berkas, sehingga pesan menyebutkan lembar dan baris tempat sel-sel itu dimulai, dan meminta Anda membandingkannya dengan cadangan jika Anda memilikinya. Jika tebakan itu berada sebelum lembar pertama, pesan menyatakan bahwa seluruh buku kerja disusun kembali. Pesan juga menyebutkan setiap lembar yang terpotong beserta baris terakhir selnya, dan lembar mana yang kosong karena datanya tidak dapat ditemukan. Pesan hanya menyatakan bahwa lembar dibaca utuh, atau bahwa format sel dan font dipulihkan, jika tidak ada yang harus ditebak dalam berkas. Buku kerja hasil perbaikan terbuka sebagai salinan, jadi simpanlah dengan nama baru.

## Buku kerja besar

JeffCrunch membaca buku kerja di latar belakang. Baris pertama dari lembar yang terbuka ketika buku kerja disimpan muncul hampir seketika, meskipun buku kerjanya sangat besar. Bilah status di kiri bawah memberi tahu apa yang sedang dilakukan: **Membuka**, **Membaca** sebuah lembar dengan jumlah sel yang sudah dibaca, **Menghitung** dengan persen, lalu **Siap**.

Anda dapat menggulir, memilih, dan menyalin selama pembacaan berlangsung. Tab lembar lain aktif ketika lembar itu dibaca. Buku kerja yang rusak dibuka sejauh yang dapat dibaca, dan sebuah pesan menyebutkan apa yang dilewati.

## Jika berkas tidak dapat dibuka

Ketika JeffCrunch tidak dapat membuka berkas, ia menjelaskan alasannya dengan kata-kata sederhana, dan membiarkan buku kerja yang sedang terbuka seperti semula. Ia membedakan berkas yang kosong dari berkas yang sama sekali bukan buku kerja (misalnya dokumen PDF, gambar, dokumen teks, atau presentasi); paket zip yang terpotong atau rusak, yang biasanya berarti unduhan atau salinan terputus, sehingga salinlah lagi dari sumber aslinya; buku kerja yang dilindungi kata sandi, yang akan diminta (lihat [Membuka buku kerja yang dilindungi kata sandi](open-password)); serta berkas gabungan yang tidak memuat buku kerja.

## Buku kerja terbaru

Halaman **Buka** pada layar File mencantumkan buku kerja yang baru saja Anda buka. Klik kanan salah satunya untuk **menyematkannya** di bagian atas daftar, atau menghapusnya dari daftar.

## Menyimpan

Tekan **{key:Save}**, atau klik **File** lalu **Simpan**, untuk menyimpan perubahan ke buku kerja yang Anda buka. Untuk mempertahankan berkas asli dan membuat berkas baru, klik **File** lalu **Simpan Sebagai**, dan pilih nama. Buku kerja dengan makro disimpan sebagai berkas **.xlsm**, templat sebagai **.xltx** atau **.xltm**, dan lainnya sebagai berkas **.xlsx**. Nama yang berakhiran jenis buku kerja lain tidak diterima, dan pesan menyebutkan akhiran yang harus dipakai; nama yang berakhiran **.xls**, **.ods**, **.csv**, **.tsv**, dan **.txt** membuat salinan, seperti dijelaskan di atas.

JeffCrunch hanya menyimpan apa yang Anda ubah. Semua yang lain dalam berkas, seperti lembar lain, bagan, gambar, makro, dan pengaturan yang tidak ditampilkan JeffCrunch, disalin seperti semula, byte demi byte. Pada lembar yang Anda ubah, hanya sel yang diubah yang ditulis ulang, beserta rumus yang bergantung padanya, yang mendapatkan nilai barunya. Jika Anda tidak mengubah sel apa pun, berkas dibiarkan begitu saja.

Jika program lain mengubah berkas setelah Anda membukanya, JeffCrunch tidak menimpanya tanpa bertanya, karena hal itu akan menghilangkan perubahan program lain. Ia menanyakan apakah akan **Simpan Salinan**, **Timpa** berkas, atau **Batal**.

Berkas baru ditulis lebih dulu di samping berkas lama, dan hanya menggantikannya setelah lengkap. Jika terjadi kesalahan, Anda melihat pesan, dan berkas yang semula ada tetap seperti sebelumnya.

Jika rumus masih dihitung ketika Anda menyimpan, JeffCrunch menunggu sampai selesai, sehingga berkas memuat hasilnya. Jika Anda menekan **Esc** untuk menghentikan penghitungan sementara itu, penyimpanan dibatalkan, dan bilah status menyatakannya; simpan lagi ketika Anda siap. JeffCrunch membiarkan satu hal di luar berkas: daftar urutan rumus terakhir dihitung oleh suatu program (rantai penghitungan). Program yang membuka buku kerja berikutnya akan membuatnya kembali. Jika JeffCrunch tidak dapat menghitung beberapa rumus, atau Anda menghentikan penghitungan, berkas meminta program tersebut menghitung ulang setiap rumus ketika membuka buku kerja.

## Menutup dengan perubahan

Jika Anda menutup buku kerja yang memiliki perubahan belum disimpan, JeffCrunch menanyakan apakah akan **Simpan** perubahan itu, **Jangan Simpan**, atau **Batal** dan melanjutkan pekerjaan. Tanda bintang (*) pada judul jendela menunjukkan adanya perubahan yang belum disimpan.

## Templat bawaan

Pada **File > Baru**, di bawah buku kerja kosong, terdapat buku kerja siap pakai dalam beberapa kelompok: **Keuangan** (anggaran bulanan, jadwal pinjaman, dan register buku cek), **Bisnis** (faktur, laporan pengeluaran, lembar waktu kerja, daftar inventaris, catatan jarak tempuh, dan laporan penjualan), **Perencanaan** (kalender dan rencana proyek), serta **Pendidikan** (buku nilai). Setiap templat menampilkan gambar kecil di bagian atasnya. Klik salah satu untuk memulai buku kerja baru darinya. Isi kotak berwarna kuning. Angka lainnya dihitung untuk Anda. Buku kerja itu tidak memiliki berkas sendiri sampai Anda menyimpannya.
