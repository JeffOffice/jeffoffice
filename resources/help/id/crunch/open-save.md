# Membuka dan menyimpan buku kerja
<!-- keywords: pulihkan pekerjaan yang belum disimpan, pemulihan otomatis, macet, salinan, format lama, opendocument, kompatibilitas, baru, kosong, buku1, tanpa judul, buka, simpan, simpan sebagai, berkas, buku kerja, xlsx, xlsm, xls, ods, csv, tsv, berkas teks, dibatasi pemisah, impor, ekspor, terbaru, disematkan, seret, lepas, pemuatan, besar, makro, belum disimpan, perubahan, tutup, salinan, rantai perhitungan, templat, xltx, xltm, tidak dapat dibuka, rusak, terenkripsi, kata sandi, simpan salinan, baca-saja, hanya baca, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ots, perbaiki, buka dan perbaiki, templat bawaan, anggaran, faktur, lembar waktu kerja, pinjaman, kalender, buku nilai, lebar tetap, kolom tetap, pemisah desimal, pemisah ribuan, minus di belakang, baris perintah, opsi pemisah, opsi pengodean, opsi desimal, html, halaman web, htm, tabel dari halaman</parameter>

Saat JeffCrunch dimulai tanpa berkas yang akan dibuka, jendela berisi buku kerja baru yang kosong dengan satu lembar, bernama Buku1. Setiap buku kerja baru mendapat nomor berikutnya (Buku2, Buku3, dan seterusnya), dan nomor tidak dipakai lagi setelah jendelanya ditutup. Untuk memulai buku kerja lain, tekan **{key:New}**, atau klik **Berkas**, lalu **Baru**, dan pilih **Buku Kerja Kosong**; buku kerja itu terbuka di jendelanya sendiri. Buku kerja baru yang belum Anda ubah akan digantikan ketika Anda membuka berkas di jendelanya; buku kerja yang sudah Anda ubah tetap ada, dan berkas dibuka di jendela lain. **Simpan** pada buku kerja baru meminta nama, begitu pula **Simpan Sebagai**, dan menyimpannya sebagai berkas .xlsx. Menutup buku kerja baru yang belum Anda ubah tidak menanyakan apa pun.

Untuk membuka buku kerja, tekan **{key:Open}**, atau klik **Berkas** lalu **Buka**, dan pilih **Telusuri**. Anda juga dapat menyeret buku kerja dari pengelola berkas ke jendela. Buku kerja yang sudah terbuka di sebuah jendela tidak dibuka lagi: jendela itu yang tampil di depan.

JeffCrunch membuka buku kerja **.xlsx** dan buku kerja **.xlsm** (yang berisi makro), serta templat **.xltx** dan **.xltm** (templat yang berisi makro). JeffCrunch tidak menjalankan makro, dan membiarkannya utuh: proyek makro, tanda tangannya, dan bagian yang menyebut makro itu ditulis kembali persis seperti semula. Templat disimpan sebagai templat, dan buku kerja berisi makro sebagai buku kerja berisi makro, karena jenis itu merupakan bagian dari berkas.

## Pulihkan pekerjaan yang belum disimpan

Jika JeffCrunch berhenti secara tiba-tiba, misalnya karena listrik padam, saat dimulai berikutnya akan tampil daftar salinan pekerjaan Anda yang belum disimpan. Pilih salinan yang ingin dibuka, lalu klik **Buka**; setiap salinan terbuka sebagai buku kerja baru yang belum punya berkas, sehingga Anda perlu mengklik **Simpan** dan memilih nama. **Hapus** membuang salinan yang dicentang, dan **Jangan Sekarang** menyimpannya untuk nanti. Anda dapat meminta daftar ini kapan saja dengan **Berkas > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Lihat [Ubah pengaturan JeffCrunch](options) untuk mengetahui seberapa sering salinan dibuat.

## Jenis berkas lain

JeffCrunch juga membuka buku kerja **.xls** versi lama, lembar kerja OpenDocument (**.ods** dan **.fods** yang rata), serta berkas teks berisi nilai (**.csv**, **.tsv**, dan **.txt**). Buku kerja yang dibuka dari salah satu jenis ini disimpan sebagai berkas **.xlsx** baru (atau sebagai salinan **.xls** atau **.ods**: lihat di bawah). Saat Anda menekan **{key:Save}**, JeffCrunch memberi tahu hal itu dan meminta nama, dan berkas yang Anda buka tidak pernah diubah. Gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks dari berkas **.ods** masuk ke berkas baru dengan posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, dan garis tepinya; bagannya ikut serta, digambar dari sel yang dibacanya.

Lembar kerja OpenDocument dibaca beserta lembarnya, nilai, rumus (dengan hasil yang disimpan berkas), format angka, font, isian, bingkai, sel gabungan, lebar kolom dan tinggi baris, baris dan kolom tersembunyi, panel beku, dan nama. Bagan, gambar dan gambar lainnya, komentar, tabel pivot, makro, format bersyarat, dan validasi data belum dibaca; pesan di bagian atas memberi tahu Anda apa saja yang dimiliki berkas itu, dan salinan yang disimpan dari sini tidak akan memilikinya. Lembar kerja yang dilindungi kata sandi akan meminta kata sandi saat dibuka: lihat [Buka buku kerja yang dilindungi kata sandi](open-password).

Berkas teks terdiri dari satu lembar. JeffCrunch mencari tahu bagaimana nilainya dipisahkan (dengan koma, titik koma, tab, atau garis tegak) dan bagaimana hurufnya dikodekan (UTF-8 atau UTF-16, atau kode halaman bahasa Anda), lalu membaca setiap nilai seperti jika Anda mengetiknya dalam bahasa Anda: angka seperti 1.234,5 dalam bahasa Jerman, persen, jumlah uang, tanggal dengan tahunnya (misalnya 3/15/2024, atau 15.03.2024 dalam bahasa Jerman), waktu, serta BENAR atau SALAH (dalam bahasa Anda, misalnya WAHR dan FALSCH dalam bahasa Jerman, dan TRUE atau FALSE dalam bahasa Inggris). Nilai yang diawali tanda sama dengan adalah rumus. Digit yang diawali nol (seperti 007) dan angka dengan lebih dari 15 digit tetap berupa teks, sehingga kode pos atau nomor rekening tidak berubah; begitu pula tanggal berupa hari dan bulan tanpa tahun, dalam urutan dan bahasa apa pun (seperti 3/4, 12.5, 1-2, 1 Maret, atau 25 Des), karena tahunnya harus dikarang, serta teks seperti nama gen MARCH1. Satu lembar memuat 1.048.576 baris dan 16.384 kolom; jika berkas memiliki lebih banyak, JeffCrunch membuka bagian yang muat dan memberi tahu Anda.

Saat Anda membuka berkas teks, sebuah jendela lebih dulu menampilkan beberapa baris pertamanya dan menanyakan cara membacanya: apa yang memisahkan kolom (koma, titik koma, tab, spasi, garis tegak, atau karakter yang Anda ketik; beberapa pemisah berurutan dapat dihitung sebagai satu jika Anda mau), apa yang mengapit kolom yang berisi pemisah, pengodean teks (Unicode, halaman kode Windows, halaman ISO 8859, Mac Roman, dan halaman Asia Timur), serta baris tempat mulai membaca. Di atas setiap kolom, Anda memilih cara membacanya: **Sebagaimana diketik**, **Teks** (agar nomor suku cadang seperti 007 atau nomor rekening panjang tetap utuh), **Tanggal** dengan urutan yang Anda tentukan, atau **Lewati**. Sebagian berkas menyusun nilainya dalam kolom dengan lebar yang sama, dipisahkan spasi, dan tanpa pemisah sama sekali. Centang **Kolom memiliki lebar tetap**, lalu baris-baris pertama berkas akan tampil dalam sebuah kotak. Klik di antara dua karakter untuk memulai kolom di situ, dan klik lagi untuk menghapusnya; atau ketik posisinya pada **Kolom dimulai pada**, misalnya 10, 25, 40. Setiap potongan dipotong pada posisi itu, dan spasi di sekitarnya dihapus. **Pemisah desimal** dan **Pemisah ribuan** memberi tahu JeffCrunch cara menulis angka dalam berkas ini, apa pun bahasa Anda, sehingga 1.234,50 berarti seribu dua ratus tiga puluh empat setengah jika berkas memakai koma sebagai pemisah desimal. Centang **Tanda minus boleh ditempatkan setelah angka** untuk berkas yang menulis -12 sebagai 12-. Pratinjau mengikuti setiap perubahan dan hanya menampilkan paling banyak 200 baris pertama dan 100 kolom pertama, tetapi seluruh berkas dibuka. **Berkas > Buka > Buka Berkas Teks** menanyakan hal ini walaupun nama berkasnya tidak biasa.

Anda juga dapat menyatakan cara membaca berkas teks saat memulai JeffCrunch dari baris perintah, sehingga tidak ada jendela yang muncul. **--separator** menyatakan apa yang memisahkan kolom: **comma**, **semicolon**, **tab**, **space**, **bar**, atau satu karakter. **--encoding** menyatakan cara huruf dikodekan: **utf-8**, **utf-8-bom**, **utf-16le**, **utf-16be**, **latin1**, **mac-roman**, **windows-1252** (atau halaman lain seperti **windows-1250**), atau **iso-8859-N**. **--decimal** menyatakan tanda desimal pada angka: **point** atau **comma**. Jika salah satunya dihilangkan, JeffCrunch mencari tahu bagian itu seperti biasa. Contoh: JeffCrunch --separator semicolon --decimal comma --encoding windows-1252 harga.csv. Nilai yang tidak dipahami JeffCrunch (pemisah kosong, akhir baris sebagai pemisah, atau karakter yang sama untuk pemisah dan pemisah desimal) akan disebutkan di layar, dan tidak ada yang dibuka. Opsi ini hanya berlaku untuk berkas teks yang disebutkan pada baris perintah itu. Berkas yang Anda buka kemudian dengan **Berkas > Buka**, atau seret ke jendela, akan menanyakan seperti biasa.

JeffCrunch juga membuka **halaman web** (**.html** dan **.htm**), serta halaman yang disimpan dengan nama buku kerja. Setiap tabel dalam halaman menjadi satu lembar, dinamai **Tabel1**, **Tabel2**, dan seterusnya. Sel dibaca seolah-olah Anda mengetiknya: angka tetap angka, dan tanggal tetap tanggal. Sel yang digabung lintas beberapa kolom atau baris menyisakan sel kosong, sehingga sel lain tetap berada di kolomnya. Kata di luar tabel, gambar, dan tampilan halaman tidak dibaca, dan halaman tanpa tabel tidak dapat dibuka. Tidak ada yang dinamai halaman itu diambil dari internet.

Untuk menyimpan hanya nilai dari lembar yang sedang Anda lihat, klik **Berkas**, lalu **Simpan Sebagai**, dan pilih jenis berkas **.csv** (atau **.tsv** dan **.txt**, yang dipisahkan tab). Sebelum menulis berkas, JeffCrunch memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat disimpan dalam berkas itu (misalnya lembar lain, rumus, gambar, bagan, catatan, dan makro), lalu menanyakan cara menulisnya, dan apakah akan **Simpan Nilai Saja**, **Simpan sebagai .xlsx** untuk tetap menyimpan semuanya, atau **Batal**:

- **Pemisah.** Koma, titik koma, tab, spasi, garis tegak, atau karakter yang Anda ketik. Berkas **.csv** memakai titik koma jika bahasa Anda menulis koma pada angka desimal, dan koma di tempat lain; berkas **.tsv** atau **.txt** memakai tab.
- **Pengodean teks.** UTF-8 dengan tanda (agar program lain membaca huruf beraksen dengan benar), UTF-8 tanpa tanda, UTF-16, atau halaman kode seperti Eropa Barat (1252), halaman ISO 8859, Mac Roman, serta halaman Jepang, Tionghoa, Korea, Ibrani, Arab, Baltik, Vietnam, dan Thai. Huruf yang tidak memiliki byte dalam halaman kode ditulis sebagai tanda tanya, dan bilah status menyebutkan jumlahnya.
- **Akhir baris.** Windows (carriage return dan line feed), Linux dan Mac (line feed), atau Mac lama (carriage return).
- **Sel.** **Sebagaimana tampak di sel** menulis apa yang Anda lihat. **Sebagaimana disimpan** menulis setiap digit angka (0,1 dan bukan 10%), serta tanggal sebagai teks ISO seperti 2026-03-04.

Teks diberi tanda kutip jika berisi pemisah, tanda kutip, akhir baris, atau spasi di salah satu ujungnya. Lembar lain, rumus, format, dan pengaturan tidak ikut tersimpan dalam berkas, dan buku kerja Anda tidak berubah karena menulis berkas tersebut.

Buku kerja yang berasal dari berkas teks ditulis kembali dalam bentuk yang dimiliki berkas tersebut: pemisah, akhir baris, dan pengodean yang sama. Jika Anda menyimpannya sebagai salinan tanpa mengubah sel apa pun, salinannya adalah berkas yang sama, byte demi byte, dan tidak ada yang ditanyakan.

## Menyimpan salinan sebagai .xls atau .ods

Untuk membuat salinan yang dapat dibuka program lama, atau program yang membaca OpenDocument, klik **Berkas**, lalu **Simpan Sebagai**, dan pilih jenis berkas **.xls** atau **.ods**. Salinan dibuat dari buku kerja Anda seperti keadaannya sekarang, termasuk perubahan yang belum disimpan, dan memuat:

- nilai, beserta teks, angka, nilai benar-salah, dan galatnya;
- rumus, dengan hasil yang dimilikinya sekarang (rumus array tetap berupa rumus array);
- format angka, font, isian, bingkai, perataan, dan apakah sel dikunci;
- sel gabungan, lebar kolom dan tinggi baris, baris dan kolom tersembunyi, garis tepi baris dan kolom, serta panel beku;
- lembar, dengan urutan, nama, dan keterangan mana yang tersembunyi, serta nama yang Anda tentukan (dan area cetak);
- bagan setiap lembar (hanya pada salinan **.ods**): kolom, batang, garis, area, lingkaran, donat, dan bagan sebar, dengan seri dan sel yang dibacanya, kategori, judul, keterangan, sumbu (judul, skala, format angka), label data, warna, dan posisinya (yang tidak memiliki tempat dalam bagan **.ods**, seperti garis tren, bilah kesalahan, dan sumbu sekunder, akan disebutkan dalam pesan);
- gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks setiap lembar (hanya pada salinan **.ods**), dengan posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, garis tepi, kata-katanya, dan gambar itu sendiri beserta potongannya (Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument);
- tautan, dan cara setiap lembar dicetak (kertas, margin, orientasi, skala, header dan footer, serta pemisah halaman).

Berkas **.xls** juga memuat tabel data, dan menyimpan pengaturan perhitungan buku kerja. Salinan tidak dapat memuat semuanya. Sebelum JeffCrunch menulis berkas, ia memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat disimpan dalam berkas itu: bagan, gambar dan gambar lainnya (untuk salinan **.xls**; salinan **.ods** menyimpannya, termasuk bagan, kecuali diagram), catatan, tabel pivot, tabel, format bersyarat, validasi data, filter, proteksi lembar, kontrol formulir, makro, dan tautan ke buku kerja lain (rumus yang memakainya tetap menyimpan nilai yang dimilikinya). Rumus yang tidak dapat dinyatakan dalam bentuk lama (fungsi yang tidak dimilikinya, atau sel setelah kolom IV atau baris 65.536) ditulis sebagai nilai hasilnya, dan pesan menghitung jumlahnya. Bentuk lama juga terbatas pada 65.536 baris, 256 kolom, 255 lembar, dan 32.767 karakter dalam satu sel, serta sekitar 4.000 format sel dan 56 warna: apa pun yang tidak muat akan dipotong, dan pesan menyebutkan jumlahnya. Anda kemudian dapat memilih **Simpan Salinan**, **Simpan sebagai .xlsx** untuk tetap menyimpan semuanya, atau **Batal**.

Buku kerja Anda tidak berubah karena pembuatan salinan, dan tidak ditandai sebagai sudah disimpan. Berkas yang Anda buka tidak pernah ditulis ulang: beri salinan nama lain. Salinan dari berkas **.xls** atau **.ods** yang Anda buka dapat disimpan sebagai jenis yang lain.

## Simpan salinan

Klik **Berkas > Simpan Salinan**, lalu pilih nama. JeffCrunch menulis buku kerja ke berkas itu, termasuk perubahan yang belum disimpan, dan Anda tetap bekerja di buku kerja semula. Nama dan perubahan yang belum disimpan pada buku kerja tetap seperti semula. Anda dapat memilih jenis berkas apa pun yang ditawarkan **Simpan Sebagai**. Salinan tidak pernah ditulis di atas berkas buku kerja itu sendiri.

## Simpan salinan cadangan

Klik **Berkas > Opsi**, dan pada halaman **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffCrunch lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi bertanya apakah akan menyimpan tanpa cadangan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula.

## Simpan sebagai templat

Templat adalah buku kerja yang Anda gunakan sebagai titik awal. Templat dapat memuat rumus, gaya, dan lembar. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat (berkas **.xltx**) disimpan di **Templat Saya**, dan buku kerja Anda tidak berubah. Dalam **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.xltx)**, atau **Templat lembar sebar OpenDocument (*.ots)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai buku kerja dari templat Anda, klik **Berkas > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Buku kerja baru terbuka dengan sel dan lembar dari templat. Buku kerja itu belum punya berkas sendiri, sehingga **Simpan** meminta nama, dan templat tidak berubah. Untuk menghapus templat dari daftar, klik kanan padanya dan pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Cara Lain untuk Membuka**. Anda juga dapat mengklik kanan sebuah nama dalam daftar Terbaru.

- **Buka Hanya-Baca** membuka buku kerja hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah sel. Bilah judul menampilkan "Hanya Baca". Jika Anda menekan **Simpan**, JeffCrunch meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka buku kerja tanpa berkas sendiri. **Simpan** menanyakan tempat penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka buku kerja di jendela sendiri, meskipun jendela yang Anda gunakan kosong.
- **Buka dan Perbaiki** untuk berkas .xlsx atau .xls yang rusak. Lihat di bawah.

## Buka dan Perbaiki

Kadang-kadang sebuah berkas .xlsx rusak. Unduhan terhenti di tengah jalan, atau disk mengalami kesalahan. Berkas .xlsx adalah kumpulan bagian yang lebih kecil, seperti lembar, gaya, dan teks bersama. **Buka dan Perbaiki** membaca setiap bagian yang dapat dibaca. Bagian yang terhenti di tengah dipertahankan sampai titik ia berhenti, sehingga Anda mungkin hanya mendapatkan baris-baris pertama dari sebuah lembar. Bagian yang tidak dapat dibaca dibuang. Jika daftar bagian atau tautan di antaranya hilang, JeffCrunch membuatnya kembali.

Buku kerja yang diperbaiki terbuka tanpa berkas sendiri, dengan nama seperti "Anggaran yang diperbaiki". Sebuah pesan memberi tahu Anda apa saja yang dipertahankan sampai titik terputus dan apa yang dibuang. Berkas yang rusak tidak diubah. Simpan buku kerja yang diperbaiki dengan nama baru, dan periksa angkanya sebelum Anda mempercayainya.

### Berkas .xls

Berkas .xls juga dapat rusak. Jika pembungkus luarnya terpotong atau daftarnya rusak, **Buka dan Perbaiki** mengikuti pembungkus sejauh yang dapat dibaca, dan jika itu gagal, mencari awal buku kerja di dalam berkas lalu membaca dari situ. Lembar yang terpotong mempertahankan sel sebelum potongan. Bagian setelah potongan hilang, dan Anda diberi tahu. Buku kerja yang diperbaiki terbuka sebagai salinan, sehingga simpanlah dengan nama baru.

## Buku kerja besar

JeffCrunch membaca buku kerja di latar belakang. Baris-baris pertama dari lembar yang terbuka ketika buku kerja disimpan muncul hampir seketika, bahkan jika buku kerja itu sangat besar. Bilah status di kiri bawah memberi tahu apa yang sedang dilakukan: **Membuka**, **Membaca** sebuah lembar dengan jumlah sel yang sudah dibaca, **Menghitung** dengan persen, dan kemudian **Siap**.

Anda dapat menggulir, memilih, dan menyalin selama pembacaan berlangsung. Tab lembar lain aktif ketika lembar itu sudah dibaca. Buku kerja yang rusak dibuka sejauh dapat dibaca, dan sebuah pesan menyebutkan apa yang dibuang.

## Jika berkas tidak dapat dibuka

Jika JeffCrunch tidak dapat membuka sebuah berkas, ia menjelaskan alasannya dengan bahasa yang sederhana, dan buku kerja yang sedang terbuka tetap seperti semula. Ia membedakan berkas yang kosong dari berkas yang bukan buku kerja sama sekali (misalnya dokumen PDF, gambar, dokumen teks, atau presentasi); paket zip yang terpotong atau rusak, yang biasanya berarti unduhan atau penyalinan terputus, sehingga salin ulang dari sumber aslinya; buku kerja yang dilindungi kata sandi, yang akan diminta kata sandinya (lihat [Buka buku kerja yang dilindungi kata sandi](open-password)); dan berkas majemuk yang tidak memuat buku kerja.

## Buku kerja terbaru

Halaman **Buka** pada layar Berkas mencantumkan buku kerja yang baru Anda buka. Klik kanan salah satunya untuk **menyematkannya** ke bagian atas daftar, atau untuk menghapusnya dari daftar.

## Menyimpan

Tekan **{key:Save}**, atau klik **Berkas** lalu **Simpan**, untuk menyimpan perubahan ke dalam buku kerja yang Anda buka. Untuk mempertahankan file asli dan membuat berkas baru, klik **Berkas** lalu **Simpan Sebagai**, dan pilih nama. Buku kerja berisi makro disimpan sebagai berkas **.xlsm**, templat sebagai berkas **.xltx** atau **.xltm**, dan lainnya sebagai berkas **.xlsx**. Nama yang berakhiran jenis buku kerja lain tidak diterima, dan pesan menyebutkan akhiran yang harus digunakan; nama berakhiran **.xls**, **.ods**, **.csv**, **.tsv**, dan **.txt** membuat salinan, seperti dijelaskan di atas.

JeffCrunch hanya menyimpan apa yang Anda ubah. Semua yang lain dalam berkas, seperti lembar lain, bagan, gambar, makro, dan pengaturan yang tidak ditampilkan JeffCrunch, disalin seperti semula, byte demi byte. Pada lembar yang Anda ubah, hanya sel yang Anda ubah yang ditulis ulang, beserta rumus yang bergantung padanya, yang mendapat nilai barunya. Jika Anda tidak mengubah sel apa pun, berkas dibiarkan begitu saja.

Jika program lain mengubah berkas setelah Anda membukanya, JeffCrunch tidak menimpanya tanpa bertanya, karena itu akan menghilangkan perubahan program lain. Ia menanyakan apakah akan **Simpan Salinan**, **Timpa** berkas, atau **Batal**.

Berkas baru ditulis lebih dulu di sebelah berkas lama, dan hanya menggantikan berkas lama setelah lengkap. Jika terjadi kesalahan, Anda melihat pesan, dan berkas yang ada tetap seperti semula.

Jika rumus masih dihitung ketika Anda menyimpan, JeffCrunch menunggu sampai selesai, sehingga berkas memuat hasilnya. Jika Anda menekan **Esc** untuk menghentikan perhitungan selama itu, penyimpanan dibatalkan, dan bilah status menyebutkannya; simpan lagi ketika Anda siap. JeffCrunch tidak menyertakan satu hal dalam berkas: daftar urutan rumus terakhir dihitung oleh sebuah program (rantai perhitungan). Program yang membuka buku kerja berikutnya akan membuatnya kembali. Jika JeffCrunch tidak dapat menghitung beberapa rumus, atau Anda menghentikan perhitungan, berkas meminta program lain menghitung ulang semua rumus ketika membukanya.

## Menutup dengan perubahan

Jika Anda menutup buku kerja yang memiliki perubahan belum disimpan, JeffCrunch menanyakan apakah akan **Simpan** perubahan itu, **Jangan Simpan**, atau **Batal** dan tetap bekerja. Tanda bintang (*) pada judul jendela menunjukkan adanya perubahan yang belum disimpan.

## Templat bawaan

Pada **Berkas > Baru**, di bawah buku kerja kosong, terdapat buku kerja siap pakai yang dikelompokkan: **Keuangan** (anggaran bulanan, jadwal pinjaman, dan register buku cek), **Bisnis** (faktur, laporan pengeluaran, lembar waktu kerja, daftar inventaris, catatan jarak tempuh, dan laporan penjualan), **Perencanaan** (kalender dan rencana proyek), serta **Pendidikan** (buku nilai). Setiap templat menampilkan gambar kecil bagian atasnya. Klik salah satu untuk memulai buku kerja baru darinya. Ketik di kotak kuning. Angka lainnya dihitung untuk Anda. Buku kerja belum memiliki berkas sendiri sampai Anda menyimpannya.
