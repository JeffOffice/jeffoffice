# Membuka dan menyimpan buku kerja
<!-- keywords: pulihkan pekerjaan yang belum disimpan, pemulihan otomatis, macet, salin, format lama, opendocument, kompatibilitas, baru, kosong, buku1, tanpa judul, buka, simpan, simpan sebagai, berkas, buku kerja, xlsx, xlsm, xls, ods, csv, tsv, berkas teks, dibatasi pemisah, impor, ekspor, terbaru, disematkan, seret, lepas, pemuatan, besar, makro, belum disimpan, perubahan, tutup, salinan, rantai perhitungan, templat, xltx, xltm, tidak dapat dibuka, rusak, dienkripsi, kata sandi, simpan salinan, hanya baca, read only, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ots, perbaiki, buka dan perbaiki, templat bawaan, anggaran, faktura, lembar waktu kerja, pinjaman, kalender, buku nilai, lebar tetap, kolom tetap, tanda desimal, tanda ribuan, tanda minus di belakang, baris perintah, opsi pemisah, opsi pengodean, opsi desimal, html, halaman web, htm, tabel dari halaman -->

Saat JeffCrunch dimulai tanpa berkas yang akan dibuka, jendela akan berisi buku kerja baru yang kosong dengan satu lembar bernama Book1. Setiap buku kerja baru mendapat nomor berikutnya (Book2, Book3, dan seterusnya), dan nomor yang sudah dipakai tidak akan dipakai lagi setelah jendelanya ditutup. Untuk memulai buku kerja lain, tekan **{key:New}**, atau klik **File**, lalu **New**, dan pilih **Blank Workbook**; buku kerja itu akan terbuka di jendelanya sendiri. Buku kerja baru yang belum Anda ubah akan digantikan ketika Anda membuka berkas di jendela tersebut; buku kerja yang sudah Anda ubah akan tetap, dan berkas dibuka di jendela lain. **Save** pada buku kerja baru akan meminta nama, begitu juga **Save As**, lalu menyimpannya sebagai berkas .xlsx. Menutup buku kerja baru yang belum Anda ubah tidak menampilkan pertanyaan apa pun.

Untuk membuka buku kerja, tekan **{key:Open}**, atau klik **File** lalu **Open**, dan pilih **Browse**. Anda juga dapat menyeret buku kerja dari pengelola berkas ke jendela. Buku kerja yang sudah terbuka di sebuah jendela tidak dibuka lagi: jendela tersebut yang akan dibawa ke depan.

JeffCrunch membuka buku kerja **.xlsx** dan buku kerja **.xlsm** (yang berisi makro), serta templat **.xltx** dan **.xltm** (templat yang berisi makro). JeffCrunch tidak menjalankan makro, dan membiarkannya tetap utuh: proyek makro, tanda tangannya, dan bagian-bagian yang merujuknya ditulis kembali persis seperti semula. Templat disimpan sebagai templat, dan buku kerja dengan makro disimpan sebagai buku kerja dengan makro, karena jenis itu merupakan bagian dari berkas.

## Pulihkan pekerjaan yang belum disimpan

Jika JeffCrunch tertutup secara tiba-tiba, misalnya karena listrik padam, saat dimulai berikutnya akan muncul daftar salinan pekerjaan Anda yang belum disimpan. Pilih salinan yang ingin dibuka lalu klik **Open**; setiap salinan akan terbuka sebagai buku kerja baru yang belum memiliki berkas, jadi klik **Save** dan pilih nama. **Delete** membuang salinan yang dicentang, dan **Not Now** membiarkannya untuk nanti. Anda dapat meminta daftar ini kapan saja dengan **File > Open > Recover Unsaved Work**. Lihat [Ubah pengaturan JeffCrunch](options) untuk mengetahui seberapa sering salinan dibuat.

## Jenis berkas lain

JeffCrunch juga membuka buku kerja **.xls** yang lebih lama, lembar kerja OpenDocument (**.ods** dan **.fods** yang rata), dan berkas teks berisi nilai (**.csv**, **.tsv**, dan **.txt**). Buku kerja yang dibuka dari salah satu jenis ini disimpan sebagai berkas **.xlsx** baru (atau sebagai salinan **.xls** atau **.ods**: lihat di bawah). Saat Anda menekan **{key:Save}**, JeffCrunch memberi tahu Anda dan meminta nama, dan berkas yang Anda buka tidak pernah diubah. Gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks dalam berkas **.ods** ikut masuk ke berkas baru beserta posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, dan garis tepinya; bagannya ikut serta, digambar dari sel yang dibacanya.

Lembar kerja OpenDocument dibaca beserta lembar, nilai, rumus (dengan hasil yang disimpan dalam berkas), format angka, font, isian, bingkai, sel gabungan, lebar kolom dan tinggi baris, baris dan kolom tersembunyi, pembekuan panel, dan nama. Bagan, gambar dan gambar lainnya, komentar, tabel pivot, makro, format bersyarat, dan validasi data belum dibaca; pesan di bagian atas akan memberi tahu Anda apa saja yang dimiliki berkas tersebut, dan salinan yang disimpan dari sini tidak akan memilikinya. Lembar kerja yang dilindungi kata sandi akan meminta kata sandi saat dibuka: lihat [Buka buku kerja yang dilindungi kata sandi](open-password).

Berkas teks hanya berisi satu lembar. JeffCrunch menemukan cara nilainya dipisahkan (dengan koma, titik koma, tab, atau garis tegak) dan cara hurufnya dikodekan (UTF-8 atau UTF-16, atau halaman kode bahasa Anda), lalu membaca setiap nilai seperti cara Anda mengetiknya dalam bahasa Anda: angka seperti 1.234,5 dalam bahasa Jerman, persen, jumlah uang, tanggal dengan tahunnya (misalnya 3/15/2024, atau 15.03.2024 dalam bahasa Jerman), waktu, TRUE atau FALSE (dalam bahasa Anda, misalnya WAHR dan FALSCH dalam bahasa Jerman, dan dalam bahasa Inggris). Nilai yang diawali tanda sama dengan adalah rumus. Digit yang diawali angka nol (seperti 007) dan angka lebih dari 15 digit tetap berupa teks, sehingga kode pos atau nomor rekening tidak berubah; begitu pula tanggal yang hanya berisi hari dan bulan tanpa tahun, dalam urutan apa pun dan bahasa apa pun (seperti 3/4, 12.5, 1-2, March 1, atau Dec 25), karena tahunnya harus dikarang, serta teks seperti nama gen MARCH1. Satu lembar memuat 1.048.576 baris dan 16.384 kolom; jika berkas memiliki lebih banyak, JeffCrunch membuka bagian yang muat dan memberi tahu Anda.

Saat Anda membuka berkas teks, jendela akan lebih dulu menampilkan beberapa baris pertamanya dan menanyakan cara membacanya: apa yang memisahkan isian (koma, titik koma, tab, spasi, garis tegak, atau karakter yang Anda ketik, dengan beberapa karakter berturut-turut dihitung sebagai satu jika Anda mau), apa yang memberi tanda kutip pada isian yang berisi pemisah, pengodean teks (Unicode, halaman kode Windows, halaman ISO 8859, Mac Roman, dan halaman Asia Timur), serta baris tempat mulai. Di atas setiap kolom, Anda memilih cara membacanya: **As typed** (sesuai ketikan), **Text** (untuk mempertahankan nomor suku cadang seperti 007 atau nomor rekening panjang apa adanya), **Date** dengan urutan yang Anda tentukan, atau **Leave out** (abaikan). Beberapa berkas menyusun nilainya dalam kolom dengan lebar sama, dipisahkan spasi, tanpa pemisah sama sekali. Centang **The columns have a fixed width**, dan baris-baris awal berkas akan ditampilkan dalam sebuah kotak. Klik di antara dua karakter untuk memulai kolom di sana, dan klik lagi untuk menghapusnya; atau ketik posisinya di **Columns begin at**, misalnya 10, 25, 40. Setiap bagian dipotong di posisi tersebut, dan spasi di sekitarnya dihapus. **Decimal mark** dan **Thousands mark** memberi tahu JeffCrunch cara penulisan angka dalam berkas ini, terlepas dari bahasa Anda, sehingga 1.234,50 berarti seribu dua ratus tiga puluh empat setengah jika berkas memakai koma sebagai tanda desimal. Centang **A minus sign may come after the number** untuk berkas yang menulis -12 sebagai 12-. Pratinjau mengikuti setiap perubahan dan menampilkan paling banyak 200 baris pertama dan 100 kolom, tetapi seluruh berkas tetap dibuka. **File > Open > Open a Text File** akan menanyakan cara membacanya meskipun nama berkasnya tidak biasa.

Anda juga dapat menentukan cara membaca berkas teks saat memulai JeffCrunch dari baris perintah, sehingga tidak ada jendela yang muncul. **--separator** menentukan apa yang memisahkan isian: **comma**, **semicolon**, **tab**, **space**, **bar**, atau satu karakter. **--encoding** menentukan cara huruf dikodekan: **utf-8**, **utf-8-bom**, **utf-16le**, **utf-16be**, **latin1**, **mac-roman**, **windows-1252** (atau halaman lain seperti **windows-1250**), atau **iso-8859-N**. **--decimal** menentukan tanda pada angka: **point** atau **comma**. Jika salah satunya dihilangkan, JeffCrunch mencari bagian itu sendiri seperti biasa. Contoh: JeffCrunch --separator semicolon --decimal comma --encoding windows-1252 harga.csv. Nilai yang tidak dipahami JeffCrunch (pemisah kosong, akhir baris sebagai pemisah, atau karakter yang sama untuk pemisah dan tanda desimal) akan ditampilkan di layar, dan tidak ada yang dibuka. Opsi ini hanya berlaku untuk berkas teks yang disebutkan pada baris perintah tersebut. Berkas yang Anda buka kemudian dengan **File > Open**, atau yang Anda lepas ke jendela, tetap akan menanyakan seperti biasa.

JeffCrunch juga membuka **halaman web** (**.html** dan **.htm**), dan halaman yang disimpan dengan nama buku kerja. Setiap tabel di halaman menjadi satu lembar, dinamai **Table1**, **Table2**, dan seterusnya. Sel dibaca seolah Anda yang mengetiknya: angka tetap angka, dan tanggal tetap tanggal. Sel yang digabung di beberapa kolom atau baris akan menyisakan sel kosong, sehingga sel lain tetap berada di kolomnya. Kata-kata di luar tabel, gambar, dan tampilan halaman tidak dibaca, dan halaman tanpa tabel tidak akan terbuka. Tidak ada yang disebut halaman tersebut yang diambil dari internet.

Untuk menyimpan hanya nilai dari lembar yang sedang Anda lihat, klik **File**, lalu **Save As**, dan pilih jenis berkas **.csv** (atau **.tsv** dan **.txt**, yang dipisahkan tab). Sebelum menulis berkas, JeffCrunch memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat disimpan dalam berkas tersebut (misalnya lembar lain, rumus, gambar, bagan, catatan, dan makro), lalu menanyakan cara menulisnya, dan apakah akan **Save Values Only**, **Save as .xlsx** untuk menyimpan semuanya, atau **Cancel**:

- **Separator.** Koma, titik koma, tab, spasi, garis tegak, atau karakter yang Anda ketik. Berkas **.csv** memakai titik koma jika bahasa Anda menulis koma pada angka desimal, dan koma di tempat lain; berkas **.tsv** atau **.txt** memakai tab.
- **Text encoding.** UTF-8 dengan tanda (agar program lain membaca huruf beraksen dengan benar), UTF-8 tanpa tanda, UTF-16, atau halaman kode seperti Western European (1252), halaman ISO 8859, Mac Roman, atau halaman Jepang, Tionghoa, Korea, Ibrani, Arab, Baltik, Vietnam, dan Thai. Huruf yang tidak memiliki byte dalam halaman kode akan ditulis sebagai tanda tanya, dan bilah status menyebutkan jumlahnya.
- **Line ends.** Windows (carriage return dan line feed), Linux dan Mac (line feed), atau Mac lama (carriage return).
- **Cells.** **As shown in the cells** menulis apa yang Anda lihat. **As stored** menulis setiap digit angka (0,1 dan bukan 10%), dan tanggal sebagai teks ISO seperti 2026-03-04.

Teks diberi tanda kutip jika berisi pemisah, tanda kutip, pemutusan baris, atau spasi di salah satu ujungnya. Lembar lain, rumus, format, dan pengaturan tidak masuk ke dalam berkas, dan buku kerja Anda tidak berubah karena penulisan berkas tersebut.

Buku kerja yang berasal dari berkas teks akan ditulis kembali dalam bentuk yang dimiliki berkas tersebut: pemisah, akhir baris, dan pengodean yang sama. Jika Anda menyimpannya sebagai salinan tanpa mengubah sel, salinannya identik dengan berkas asli, byte demi byte, dan tidak ada pertanyaan yang diajukan.

## Menyimpan salinan sebagai .xls atau .ods

Untuk membuat salinan yang dapat dibuka program lama, atau program yang membaca OpenDocument, klik **File**, lalu **Save As**, dan pilih jenis berkas **.xls** atau **.ods**. Salinan dibuat dari buku kerja Anda sebagaimana adanya sekarang, termasuk perubahan yang belum Anda simpan, dan memuat:

- nilai, dengan teksnya, angkanya, nilai benar-salahnya, dan galatnya;
- rumus, dengan hasil yang dimilikinya sekarang (rumus array tetap berupa rumus array);
- format angka, font, isian, bingkai, perataan, dan apakah sel dikunci;
- sel gabungan, lebar kolom dan tinggi baris, baris dan kolom tersembunyi, garis tepi baris dan kolom, serta pembekuan panel;
- lembar, dalam urutannya, dengan namanya dan lembar mana yang tersembunyi, serta nama yang Anda tentukan (dan area cetak);
- bagan setiap lembar (hanya pada salinan **.ods**): kolom, batang, garis, area, lingkaran, donat, dan bagan sebar, dengan seri dan sel yang dibacanya, kategori, judul, keterangan, sumbu (judul, skala, format angka), label data, warna, dan posisinya (yang tidak dimiliki bagan **.ods**, seperti garis tren, bilah kesalahan, dan sumbu sekunder, akan disebutkan dalam pesan);
- gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks setiap lembar (hanya pada salinan **.ods**), beserta posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, garis tepi, kata-katanya, dan gambar itu sendiri dengan pemotongannya (Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument);
- tautan, dan cara setiap lembar dicetak (kertas, margin, orientasi, skala, header dan footer, serta pemisah halaman).

Berkas **.xls** juga memuat tabel data, dan menyimpan pengaturan perhitungan buku kerja. Salinan tidak dapat memuat semuanya. Sebelum JeffCrunch menulis berkas, ia memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat masuk ke berkas tersebut: bagan, gambar dan gambar lainnya (untuk salinan **.xls**; salinan **.ods** menyimpannya, termasuk bagan, kecuali diagram), catatan, tabel pivot, tabel, format bersyarat, validasi data, filter, proteksi lembar, kontrol formulir, makro, dan tautan ke buku kerja lain (rumus yang memakainya tetap menyimpan nilai yang dimilikinya). Rumus yang tidak dapat dinyatakan dalam bentuk lama (fungsi yang tidak dimilikinya, atau sel di luar kolom IV atau baris 65.536) akan ditulis sebagai nilai hasilnya, dan pesan menghitung jumlahnya. Bentuk lama juga dibatasi pada 65.536 baris, 256 kolom, 255 lembar, dan 32.767 karakter dalam satu sel, serta sekitar 4.000 format sel dan 56 warna: yang tidak muat akan dipotong, dan pesan menyebutkan jumlahnya. Setelah itu Anda dapat memilih **Save Copy**, **Save as .xlsx** untuk menyimpan semuanya, atau **Cancel**.

Buku kerja Anda tidak berubah karena penyalinan, dan tidak ditandai sebagai sudah disimpan. Berkas yang Anda buka tidak pernah ditulis ulang: berikan nama lain untuk salinan. Salinan dari berkas **.xls** atau **.ods** yang Anda buka dapat disimpan dalam jenis lainnya.

## Simpan salinan

Klik **File > Save a Copy** dan pilih nama. JeffCrunch menulis buku kerja ke berkas tersebut, termasuk perubahan yang belum Anda simpan, dan Anda tetap bekerja di buku kerja yang sama. Nama dan perubahan yang belum disimpan tetap seperti semula. Anda dapat memilih jenis berkas apa pun yang ditawarkan **Save As**. Salinan tidak pernah ditulis di atas berkas buku kerja itu sendiri.

## Simpan salinan cadangan

Klik **File > Options**, lalu pada halaman **General** aktifkan **Always create backup copy**. Setiap kali Anda menyimpan, JeffCrunch lebih dulu menyalin berkas lama ke berkas bernama "Backup of" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi akan menanyakan apakah tetap menyimpan tanpa cadangan. Jawabannya adalah No kecuali Anda memilih Yes, dan berkas tetap seperti semula.

## Simpan sebagai templat

Templat adalah buku kerja yang Anda gunakan sebagai titik awal. Templat dapat berisi rumus, gaya, dan lembar. Klik **File > Save as Template**, lalu ketik nama. Templat (berkas **.xltx**) disimpan di **My Templates**, dan buku kerja Anda tidak berubah. Di **Save As**, Anda juga dapat memilih **Templates (*.xltx)**, atau **OpenDocument spreadsheet templates (*.ots)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai buku kerja dari templat Anda, klik **File > New**. Templat Anda tercantum di bawah **My Templates**. Klik salah satunya. Buku kerja baru akan terbuka dengan sel dan lembar dari templat tersebut. Buku kerja itu belum memiliki berkas sendiri, sehingga **Save** akan meminta nama, dan templat tidak berubah. Untuk menghapus templat dari daftar, klik kanan padanya lalu pilih **Remove from My Templates**.

## Cara lain membuka berkas

Pada halaman **Open**, klik **More Ways to Open**. Anda juga dapat mengklik kanan nama di daftar Terbaru.

- **Open Read-Only** membuka buku kerja hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah sel. Bilah judul bertuliskan "Read-Only". Jika Anda menekan **Save**, JeffCrunch meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Open as Copy** membuka buku kerja tanpa berkas sendiri. **Save** akan menanyakan tempat penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Open in New Window** membuka buku kerja di jendelanya sendiri, meskipun jendela tempat Anda berada masih kosong.
- **Open and Repair** digunakan untuk berkas .xlsx atau .xls yang rusak. Lihat di bawah.

## Buka dan perbaiki

Terkadang berkas .xlsx menjadi rusak. Unduhan yang terhenti di tengah jalan, atau disk yang mengalami kesalahan, dapat menjadi penyebabnya. Berkas .xlsx adalah kumpulan bagian yang lebih kecil, seperti lembar, gaya, dan teks bersama. **Open and Repair** membaca setiap bagian yang dapat dibaca. Bagian yang terhenti di tengah akan dipertahankan sampai titik ia berhenti, sehingga Anda mungkin hanya mendapatkan baris-baris awal sebuah lembar. Bagian yang tidak dapat dibaca akan dibuang. Jika daftar bagian atau tautan di antaranya hilang, JeffCrunch akan membuatnya kembali.

Buku kerja yang diperbaiki terbuka tanpa berkas sendiri, dengan nama seperti "Repaired budget". Sebuah pesan memberi tahu Anda apa saja yang dipertahankan sampai titik putus dan apa saja yang dibuang. Berkas yang rusak tidak diubah. Simpan buku kerja yang diperbaiki dengan nama baru, dan periksa angka-angkanya sebelum Anda mempercayainya.

### Berkas .xls

Berkas .xls juga dapat rusak. Jika bungkus luarnya terpotong, **Open and Repair** mencari awal buku kerja di dalam berkas dan membacanya dari sana. Lembar yang terpotong akan mempertahankan sel sebelum titik potong. Apa pun yang berada setelah titik potong akan hilang, dan Anda akan diberi tahu. Buku kerja yang diperbaiki terbuka sebagai salinan, sehingga simpanlah dengan nama baru.

## Buku kerja besar

JeffCrunch membaca buku kerja di latar belakang. Baris-baris pertama dari lembar yang terbuka saat buku kerja disimpan muncul hampir seketika, bahkan ketika buku kerjanya sangat besar. Bilah status di kiri bawah memberi tahu apa yang sedang dikerjakan: **Opening**, **Reading** sebuah lembar dengan jumlah sel yang sudah dibaca, **Calculating** dengan persen, lalu **Ready**.

Anda dapat menggulir, memilih, dan menyalin selagi buku kerja dibaca. Tab lembar lain akan aktif ketika lembar tersebut selesai dibaca. Buku kerja yang rusak dibuka sejauh yang dapat dibaca, dan sebuah pesan menyebutkan apa saja yang dibuang.

## Jika berkas tidak dapat dibuka

Ketika JeffCrunch tidak dapat membuka berkas, ia menjelaskan alasannya dengan kata-kata sederhana, dan membiarkan buku kerja yang sudah terbuka tetap seperti semula. Ia membedakan berkas yang kosong dari berkas yang bukan buku kerja sama sekali (misalnya dokumen PDF, gambar, dokumen teks, atau presentasi); paket zip yang terpotong atau rusak, yang biasanya berarti unduhan atau penyalinan terputus, sehingga salinlah kembali dari sumber aslinya; buku kerja yang dilindungi kata sandi, yang akan diminta kata sandinya (lihat [Buka buku kerja yang dilindungi kata sandi](open-password)); dan berkas majemuk yang tidak berisi buku kerja.

## Buku kerja terbaru

Halaman **Open** pada layar File mencantumkan buku kerja yang baru-baru ini Anda buka. Klik kanan pada salah satunya untuk **menyematkannya** di bagian atas daftar, atau menghapusnya dari daftar.

## Menyimpan

Tekan **{key:Save}**, atau klik **File** lalu **Save**, untuk menyimpan perubahan Anda ke dalam buku kerja yang Anda buka. Untuk mempertahankan berkas asli dan membuat berkas baru, klik **File** lalu **Save As**, dan pilih nama. Buku kerja dengan makro disimpan sebagai berkas **.xlsm**, templat sebagai berkas **.xltx** atau **.xltm**, dan buku kerja lainnya sebagai berkas **.xlsx**. Nama yang berakhiran jenis buku kerja lain tidak diterima, dan pesan akan menyebutkan akhiran yang harus digunakan; nama yang berakhiran **.xls**, **.ods**, **.csv**, **.tsv**, dan **.txt** membuat salinan, seperti dijelaskan di atas.

JeffCrunch hanya menyimpan apa yang Anda ubah. Semua bagian lain dalam berkas, seperti lembar lain, bagan, gambar, makro, dan pengaturan yang tidak ditampilkan JeffCrunch, disalin apa adanya, byte demi byte. Pada lembar yang Anda ubah, hanya sel yang Anda ubah yang ditulis ulang, beserta rumus yang bergantung padanya, yang akan memperoleh nilai barunya. Jika Anda tidak mengubah sel apa pun, berkas dibiarkan begitu saja.

Jika program lain mengubah berkas setelah Anda membukanya, JeffCrunch tidak akan menimpanya tanpa bertanya, karena itu akan menghilangkan perubahan program lain tersebut. Ia akan menanyakan apakah Anda ingin **Save a Copy**, **Overwrite** berkas tersebut, atau **Cancel**.

Berkas baru ditulis lebih dulu di sebelah berkas lama, dan baru menggantikan berkas lama setelah lengkap. Jika terjadi kesalahan, Anda akan melihat pesan, dan berkas yang ada tetap seperti semula.

Jika rumus masih dihitung saat Anda menyimpan, JeffCrunch menunggu sampai selesai, sehingga berkas memuat hasilnya. Jika Anda menekan **Esc** untuk menghentikan perhitungan selama itu, penyimpanan dibatalkan, dan bilah status memberi tahu Anda; simpan lagi ketika Anda siap. JeffCrunch tidak memasukkan satu hal ke dalam berkas: daftar urutan rumus terakhir kali dihitung oleh suatu program (rantai perhitungan). Program yang membuka buku kerja berikutnya akan membuatnya kembali. Jika JeffCrunch tidak dapat menghitung beberapa rumus, atau Anda menghentikan perhitungan, berkas akan meminta program tersebut untuk menghitung ulang semua rumus ketika buku kerja dibuka.

## Menutup dengan perubahan

Jika Anda menutup buku kerja yang memiliki perubahan belum disimpan, JeffCrunch menanyakan apakah akan **Save** perubahan tersebut, **Don't Save**, atau **Cancel** dan melanjutkan pekerjaan. Tanda bintang (*) pada judul jendela menunjukkan adanya perubahan yang belum disimpan.

## Templat bawaan

Pada **File > New**, di bawah buku kerja kosong, terdapat buku kerja siap pakai yang dikelompokkan: **Finance** (anggaran bulanan, jadwal pinjaman, dan register buku cek), **Business** (faktur, laporan pengeluaran, lembar waktu kerja, daftar inventaris, catatan jarak tempuh, dan laporan penjualan), **Planning** (kalender dan rencana proyek), dan **Education** (buku nilai). Setiap templat menampilkan gambar kecil di bagian atasnya. Klik salah satu untuk memulai buku kerja baru darinya. Isi kotak berwarna kuning. Angka lainnya dihitung untuk Anda. Buku kerja tersebut belum memiliki berkas sendiri sampai Anda menyimpannya.
