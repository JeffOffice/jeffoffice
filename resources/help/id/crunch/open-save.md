# Membuka dan menyimpan buku kerja
<!-- keywords: pulihkan pekerjaan yang belum disimpan, pemulihan otomatis, macet, salin, format lama, opendocument, kompatibilitas, baru, kosong, buku1, tanpa judul, buka, simpan, simpan sebagai, berkas, buku kerja, xlsx, xlsm, xls, ods, csv, tsv, berkas teks, dibatasi pembatas, impor, ekspor, terbaru, disematkan, seret, lepas, memuat, besar, makro, belum disimpan, perubahan, tutup, salinan, rantai perhitungan, templat, xltx, xltm, tidak bisa dibuka, rusak, terenkripsi, kata sandi, simpan salinan, hanya baca, read only, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ots, perbaiki, buka dan perbaiki, templat bawaan, anggaran, faktur, lembar waktu kerja, pinjaman, kalender, buku nilai, lebar tetap, kolom tetap, pemisah desimal, pemisah ribuan, minus di belakang, html, halaman web, htm, tabel dari halaman -->

Saat JeffCrunch dimulai tanpa berkas yang akan dibuka, jendela berisi buku kerja baru yang kosong dengan satu lembar bernama Book1. Setiap buku kerja baru mendapat nomor berikutnya (Book2, Book3, dan seterusnya), dan nomor tidak dipakai lagi setelah jendelanya ditutup. Untuk memulai buku kerja lain, tekan **{key:New}**, atau klik **File**, lalu **New**, dan pilih **Buku Kerja Kosong**; buku kerja itu akan terbuka di jendela tersendiri. Buku kerja baru yang belum Anda ubah akan digantikan saat Anda membuka berkas di jendelanya; buku kerja yang sudah Anda ubah tetap ada, dan berkas dibuka di jendela lain. **Simpan** pada buku kerja baru meminta nama, seperti **Simpan Sebagai**, dan menyimpannya sebagai berkas .xlsx. Menutup buku kerja baru yang belum Anda ubah tidak menanyakan apa pun.

Untuk membuka buku kerja, tekan **{key:Open}**, atau klik **File** lalu **Buka**, dan pilih **Telusuri**. Anda juga dapat menyeret buku kerja dari pengelola berkas ke jendela. Buku kerja yang sudah terbuka di sebuah jendela tidak dibuka lagi: jendela itu yang ditampilkan ke depan.

JeffCrunch membuka buku kerja **.xlsx** dan buku kerja **.xlsm** (yang memuat makro), serta templat **.xltx** dan **.xltm** (templat yang memuat makro). JeffCrunch tidak menjalankan makro, dan membiarkannya tetap utuh: proyek makro, tanda tangannya, serta bagian-bagian yang merujuknya ditulis kembali persis seperti semula. Templat disimpan sebagai templat, dan buku kerja dengan makro sebagai buku kerja dengan makro, karena jenisnya merupakan bagian dari berkas.

## Pulihkan pekerjaan yang belum disimpan

Jika JeffCrunch tertutup secara tiba-tiba, misalnya karena listrik padam, saat dimulai berikutnya akan tampil daftar salinan pekerjaan Anda yang belum disimpan. Pilih salinan yang ingin dibuka lalu klik **Buka**; setiap salinan akan terbuka sebagai buku kerja baru yang belum memiliki berkas, sehingga Anda perlu klik **Simpan** dan memilih nama. **Hapus** membuang salinan yang dicentang, dan **Jangan Sekarang** membiarkannya untuk nanti. Anda dapat meminta daftar ini kapan saja dengan **File > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Lihat [Ubah pengaturan JeffCrunch](options) untuk mengetahui seberapa sering salinan dibuat.

## Jenis berkas lain

JeffCrunch juga membuka buku kerja **.xls** lama, lembar kerja OpenDocument (**.ods** dan **.fods** datar), serta berkas teks berisi nilai (**.csv**, **.tsv**, dan **.txt**). Buku kerja yang dibuka dari salah satu jenis ini disimpan sebagai berkas **.xlsx** baru (atau sebagai salinan **.xls** atau **.ods**: lihat di bawah). Saat Anda menekan **{key:Save}**, JeffCrunch memberi tahu hal itu dan meminta nama, dan berkas yang Anda buka tidak pernah diubah. Gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks dari berkas **.ods** masuk ke berkas baru dengan posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, dan garis tepinya; bagannya ikut serta, digambar dari sel yang dibacanya.

Lembar kerja OpenDocument dibaca beserta lembar, nilai, rumus (dengan hasil yang disimpan dalam berkas), format angka, font, isian, bingkai, sel gabungan, lebar kolom dan tinggi baris, baris serta kolom tersembunyi, panel beku, dan nama. Bagan, gambar dan gambar lain, komentar, tabel pivot, makro, pemformatan bersyarat, serta validasi data belum dibaca; sebuah pesan di bagian atas memberi tahu apa saja yang dimiliki berkas, dan salinan yang disimpan dari sini tidak akan memilikinya. Lembar kerja yang dilindungi kata sandi akan memintanya saat dibuka: lihat [Buka buku kerja yang dilindungi kata sandi](open-password).

Berkas teks hanya berisi satu lembar. JeffCrunch mencari tahu bagaimana nilainya dipisahkan (dengan koma, titik koma, tab, atau tanda bar vertikal) dan bagaimana hurufnya dikodekan (UTF-8 atau UTF-16, atau halaman kode bahasa Anda), lalu membaca setiap nilai seperti jika Anda mengetikkannya dalam bahasa Anda: angka seperti 1.234,5 dalam bahasa Jerman, persen, jumlah uang, tanggal dengan tahunnya (misalnya 3/15/2024, atau 15.03.2024 dalam bahasa Jerman), waktu, serta TRUE atau FALSE (dalam bahasa Anda, misalnya WAHR dan FALSCH dalam bahasa Jerman, dan dalam bahasa Inggris). Nilai yang diawali tanda sama dengan adalah rumus. Angka yang diawali nol (seperti 007) dan angka lebih dari 15 digit tetap berupa teks, sehingga kode pos atau nomor rekening tidak berubah; begitu pula tanggal dengan hari dan bulan tanpa tahun, dalam urutan apa pun dan dalam bahasa apa pun (seperti 3/4, 12.5, 1-2, 1 Maret, atau 25 Des), karena tahunnya harus dikarang, serta teks seperti nama gen MARCH1. Sebuah lembar memuat 1.048.576 baris dan 16.384 kolom; jika berkas memiliki lebih banyak, JeffCrunch membuka bagian yang muat dan memberi tahu Anda.

Saat Anda membuka berkas teks, sebuah jendela lebih dulu menampilkan beberapa baris pertamanya dan menanyakan cara membacanya: apa yang memisahkan kolom (salah satu dari koma, titik koma, tab, spasi, bar, atau karakter yang Anda ketik, dengan beberapa pemisah berurutan dihitung sebagai satu bila diinginkan), apa yang mengapit kolom yang memuat pemisah, pengodean teks (Unicode, halaman kode Windows, halaman ISO 8859, Mac Roman, dan halaman Asia Timur), serta baris tempat mulai membaca. Di atas setiap kolom, Anda memilih cara membacanya: **Sebagaimana diketik**, **Teks** (untuk menyimpan nomor suku cadang seperti 007 atau nomor rekening yang panjang apa adanya), **Tanggal** dengan urutan yang Anda tentukan, atau **Abaikan**. Sebagian berkas menyusun nilainya dalam kolom dengan lebar yang sama, dipisahkan spasi, tanpa pemisah sama sekali. Centang **Kolom memiliki lebar tetap**, dan baris-baris pertama berkas akan tampil dalam sebuah kotak. Klik di antara dua karakter untuk memulai kolom di situ, dan klik lagi untuk menghapusnya; atau ketik posisinya di **Kolom dimulai pada**, misalnya 10, 25, 40. Setiap bagian dipotong di posisi tersebut, dan spasi di sekitarnya dihapus. **Pemisah desimal** dan **Pemisah ribuan** memberi tahu JeffCrunch cara angka dalam berkas ini ditulis, terlepas dari bahasa yang Anda gunakan, sehingga 1.234,50 berarti seribu dua ratus tiga puluh empat setengah jika berkas memakai koma sebagai pemisah desimal. Centang **Tanda minus dapat muncul setelah angka** untuk berkas yang menulis -12 sebagai 12-. Pratinjau mengikuti setiap perubahan dan hanya menampilkan paling banyak 200 baris pertama dan 100 kolom pertama, tetapi seluruh berkas tetap dibuka. **File > Buka > Buka Berkas Teks** selalu menanyakan hal ini, meskipun nama berkasnya tidak biasa.

JeffCrunch juga membuka **halaman web** (**.html** dan **.htm**), serta halaman yang disimpan dengan nama buku kerja. Setiap tabel dalam halaman menjadi sebuah lembar, dinamai **Table1**, **Table2**, dan seterusnya. Sel dibaca seolah-olah Anda mengetiknya: angka menjadi angka, dan tanggal menjadi tanggal. Sel yang digabung di beberapa kolom atau baris menyisakan sel kosong, sehingga sel lain tetap berada di kolomnya. Kata-kata di luar tabel, gambar, dan tampilan halaman tidak dibaca, dan halaman tanpa tabel tidak dapat dibuka. Tidak ada yang dirujuk halaman yang diambil dari internet.

Untuk menyimpan hanya nilai dari lembar yang sedang Anda lihat, klik **File**, lalu **Simpan Sebagai**, dan pilih jenis berkas **.csv** (atau **.tsv** dan **.txt**, yang dipisahkan tab). Sebelum menulis berkas, JeffCrunch memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat disimpan dalam berkas itu (misalnya lembar lain, rumus, gambar, bagan, catatan, dan makro), lalu menanyakan cara menulisnya, serta apakah akan **Simpan Hanya Nilai**, **Simpan sebagai .xlsx** untuk tetap menyimpan semuanya, atau **Batal**:

- **Pemisah.** Koma, titik koma, tab, spasi, bar vertikal, atau karakter yang Anda ketik. **.csv** dimulai dengan titik koma jika bahasa Anda menulis koma pada angka desimal, dan dengan koma di tempat lain; **.tsv** atau **.txt** dengan tab.
- **Pengodean teks.** UTF-8 dengan tanda (agar program lain membaca huruf beraksen dengan benar), UTF-8 tanpa tanda, UTF-16, atau halaman kode seperti Eropa Barat (1252), halaman ISO 8859, Mac Roman, serta halaman Jepang, Tionghoa, Korea, Ibrani, Arab, Baltik, Vietnam, dan Thai. Huruf yang tidak memiliki byte dalam halaman kode ditulis sebagai tanda tanya, dan bilah status menyebutkan jumlahnya.
- **Akhir baris.** Windows (carriage return dan line feed), Linux dan Mac (line feed), atau Mac lama (carriage return).
- **Sel.** **Sebagaimana ditampilkan di sel** menulis apa yang Anda lihat. **Sebagaimana disimpan** menulis setiap digit angka (0,1 dan bukan 10%), dan tanggal sebagai teks ISO seperti 2026-03-04.

Teks diapit tanda kutip jika memuat pemisah, tanda kutip, pemisah baris, atau spasi di salah satu ujungnya. Lembar lain, rumus, format, dan pengaturan tidak ikut disimpan dalam berkas, dan buku kerja Anda tidak berubah karena menulis berkas tersebut.

Buku kerja yang berasal dari berkas teks ditulis kembali dalam bentuk yang dimiliki berkas tersebut: pemisah, akhir baris, dan pengodean yang sama. Jika Anda menyimpannya sebagai salinan tanpa mengubah sel apa pun, salinannya identik dengan berkas asli, byte demi byte, dan JeffCrunch tidak menanyakan apa pun.

## Menyimpan salinan sebagai .xls atau .ods

Untuk membuat salinan yang dapat dibuka program lama, atau program yang membaca OpenDocument, klik **File**, lalu **Simpan Sebagai**, dan pilih jenis berkas **.xls** atau **.ods**. Salinan dibuat dari buku kerja Anda seperti keadaannya sekarang, termasuk perubahan yang belum Anda simpan, dan memuat:

- nilai, beserta teksnya, angkanya, nilai benar-salah, dan galatnya;
- rumus, dengan hasil yang kini dimilikinya (rumus array tetap berupa rumus array);
- format angka, font, isian, bingkai, perataan, dan apakah sel dikunci;
- sel gabungan, lebar kolom dan tinggi baris, baris serta kolom tersembunyi, garis tepi baris dan kolom, serta panel beku;
- lembar, dengan urutan, nama, dan mana yang tersembunyi, serta nama yang Anda tentukan (dan area cetak);
- bagan setiap lembar (hanya pada salinan **.ods**): kolom, batang, garis, area, lingkaran, donat, dan bagan sebar, dengan seri dan sel yang dibacanya, kategori, judul, keterangan, sumbu (judul, skala, format angka), label data, warna, dan posisinya (yang tidak dapat ditampung bagan **.ods**, seperti garis tren, bilah kesalahan, dan sumbu sekunder, disebutkan dalam pesan);
- gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks setiap lembar (hanya pada salinan **.ods**), dengan posisi, ukuran, putaran, urutan, nama, teks alternatif, isian, garis tepi, kata-katanya, serta gambar itu sendiri beserta pemotongannya (Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument);
- tautan, serta cara setiap lembar dicetak (kertas, margin, orientasi, skala, header dan footer, dan pemisah halaman).

Berkas **.xls** juga memuat tabel data, dan mempertahankan pengaturan perhitungan buku kerja. Salinan tidak dapat memuat semuanya. Sebelum JeffCrunch menulis berkas, ia memberi tahu apa saja yang dimiliki buku kerja Anda dan tidak dapat disimpan dalam berkas itu: bagan, gambar dan gambar lain (untuk salinan **.xls**; salinan **.ods** tetap menyimpannya, termasuk bagan, kecuali diagram), catatan, tabel pivot, tabel, pemformatan bersyarat, validasi data, filter, perlindungan lembar, kontrol formulir, makro, serta tautan ke buku kerja lain (rumus yang memakainya tetap menyimpan nilai yang dimilikinya). Rumus yang tidak dapat dinyatakan dalam bentuk lama (fungsi yang tidak dimilikinya, atau sel di luar kolom IV atau baris 65.536) ditulis sebagai nilai hasilnya, dan pesan menghitung jumlahnya. Bentuk lama juga berhenti pada 65.536 baris, 256 kolom, 255 lembar, dan 32.767 karakter dalam satu sel, serta sekitar 4.000 format sel dan 56 warna: bagian yang tidak muat dipotong, dan pesan menyebutkan berapa banyak. Anda kemudian dapat memilih **Simpan Salinan**, **Simpan sebagai .xlsx** untuk tetap menyimpan semuanya, atau **Batal**.

Buku kerja Anda tidak berubah karena pembuatan salinan, dan tidak ditandai sebagai sudah disimpan. Berkas yang Anda buka tidak pernah ditulis ulang: beri salinan nama lain. Salinan dari berkas **.xls** atau **.ods** yang Anda buka dapat disimpan dalam jenis yang lain.

## Simpan salinan

Klik **File > Simpan Salinan** lalu pilih nama. JeffCrunch menulis buku kerja ke berkas itu, termasuk perubahan yang belum Anda simpan, dan Anda tetap bekerja di buku kerja yang tadi. Nama dan perubahan yang belum disimpan tetap seperti semula. Anda dapat memilih jenis berkas apa pun yang ditawarkan **Simpan Sebagai**. Salinan tidak pernah ditulis di atas berkas milik buku kerja itu sendiri.

## Simpan salinan cadangan

Klik **File > Opsi**, lalu pada halaman **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffCrunch lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi akan menanyakan apakah tetap menyimpan tanpa cadangan. Jawabannya Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula.

## Simpan sebagai templat

Templat adalah buku kerja yang Anda gunakan sebagai titik awal. Templat dapat memuat rumus, gaya, dan lembar. Klik **File > Simpan sebagai Templat** lalu ketik nama. Templat (berkas **.xltx**) disimpan di **Templat Saya**, dan buku kerja Anda tidak berubah. Di **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.xltx)**, atau **Templat lembar sebar OpenDocument (*.ots)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai buku kerja dari templat Anda, klik **File > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Buku kerja baru akan terbuka dengan sel dan lembar dari templat itu. Buku kerja tersebut belum memiliki berkas sendiri, sehingga **Simpan** akan meminta nama, dan templatnya tidak berubah. Untuk menghapus templat dari daftar, klik kanan padanya lalu pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Cara Lain untuk Membuka**. Anda juga dapat mengklik kanan nama dalam daftar Terbaru.

- **Buka Hanya-Baca** membuka buku kerja hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah sel. Bilah judul menampilkan "(Hanya Baca)". Jika Anda menekan **Simpan**, JeffCrunch meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka buku kerja tanpa berkas sendiri. **Simpan** akan menanyakan tempat penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka buku kerja di jendela tersendiri, meskipun jendela tempat Anda berada kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .xlsx atau .xls yang rusak. Lihat di bawah.

## Buka dan Perbaiki

Terkadang berkas .xlsx menjadi rusak. Unduhan yang terhenti di tengah jalan, atau disk yang mengalami kesalahan. Berkas .xlsx adalah kumpulan bagian-bagian yang lebih kecil, seperti lembar, gaya, dan teks bersama. **Buka dan Perbaiki** membaca setiap bagian yang dapat dibaca. Bagian yang terhenti di tengah akan disimpan sampai titik berhentinya, sehingga Anda mungkin hanya mendapatkan baris-baris pertama dari sebuah lembar. Bagian yang tidak dapat dibaca akan dilewati. Jika daftar bagian atau tautan di antaranya hilang, JeffCrunch membuatnya kembali.

Buku kerja hasil perbaikan terbuka tanpa berkas sendiri, dengan nama seperti "Perbaikan anggaran". Sebuah pesan memberi tahu Anda apa saja yang dipulihkan sampai titik putus dan apa yang dilewati. Berkas yang rusak tidak diubah. Simpan buku kerja hasil perbaikan dengan nama baru, dan periksa angka-angkanya sebelum Anda mempercayainya.

### Berkas .xls

Berkas .xls juga dapat rusak. Jika bungkus luarnya terpotong, **Buka dan Perbaiki** mencari awal buku kerja di dalam berkas dan membacanya dari sana. Lembar yang terpotong mempertahankan sel-sel sebelum titik potong. Apa pun yang berada setelah titik potong hilang, dan Anda akan diberi tahu. Buku kerja hasil perbaikan dibuka sebagai salinan, jadi simpan dengan nama baru.

## Buku kerja besar

JeffCrunch membaca buku kerja di latar belakang. Baris-baris pertama dari lembar yang terbuka saat buku kerja disimpan muncul hampir seketika, bahkan jika buku kerjanya sangat besar. Bilah status di kiri bawah memberi tahu apa yang sedang dikerjakan: **Membuka**, **Membaca** sebuah lembar dengan jumlah sel yang sudah dibaca, **Menghitung** dengan persen, lalu **Siap**.

Anda dapat menggulir, memilih, dan menyalin selama pembacaan berlangsung. Tab lembar lain aktif ketika lembar tersebut selesai dibaca. Buku kerja yang rusak dibuka sejauh yang masih dapat dibaca, dan sebuah pesan menyebutkan apa yang dilewati.

## Jika berkas tidak dapat dibuka

Jika JeffCrunch tidak dapat membuka berkas, ia menjelaskan alasannya dengan kata-kata yang sederhana dan membiarkan buku kerja yang sudah terbuka tetap seperti semula. Ia membedakan berkas yang kosong dari berkas yang bukan buku kerja sama sekali (misalnya dokumen PDF, gambar, dokumen teks, atau presentasi); paket zip yang terpotong atau rusak, yang biasanya berarti unduhan atau penyalinan terganggu, sehingga salinlah kembali dari sumber aslinya; buku kerja yang dilindungi kata sandi, yang akan dimintai kata sandinya (lihat [Buka buku kerja yang dilindungi kata sandi](open-password)); dan berkas majemuk yang tidak memuat buku kerja.

## Buku kerja terbaru

Halaman **Buka** pada layar Berkas mencantumkan buku kerja yang baru Anda buka. Klik kanan salah satunya untuk **menyematkannya** ke bagian atas daftar, atau untuk menghapusnya dari daftar.

## Menyimpan

Tekan **{key:Save}**, atau klik **File** lalu **Simpan**, untuk menyimpan perubahan ke buku kerja yang Anda buka. Untuk menyimpan yang asli dan membuat berkas baru, klik **File** lalu **Simpan Sebagai**, dan pilih nama. Buku kerja dengan makro disimpan sebagai berkas **.xlsm**, templat sebagai berkas **.xltx** atau **.xltm**, dan lainnya sebagai berkas **.xlsx**. Nama yang berakhir dengan jenis buku kerja lain tidak diterima, dan pesan akan menyebutkan akhiran yang harus digunakan; nama yang berakhir dengan **.xls**, **.ods**, **.csv**, **.tsv**, dan **.txt** menghasilkan salinan, seperti dijelaskan di atas.

JeffCrunch hanya menyimpan apa yang Anda ubah. Semua yang lain dalam berkas, seperti lembar lain, bagan, gambar, makro, dan pengaturan yang tidak ditampilkan JeffCrunch, disalin seperti semula, byte demi byte. Pada lembar yang Anda ubah, hanya sel yang Anda ubah yang ditulis ulang, beserta rumus yang bergantung padanya, yang menerima nilai barunya. Jika Anda tidak mengubah sel apa pun, berkas dibiarkan begitu saja.

Jika program lain mengubah berkas setelah Anda membukanya, JeffCrunch tidak akan menimpanya tanpa bertanya, karena hal itu akan menghilangkan perubahan dari program lain tersebut. Ia akan menanyakan apakah akan **Simpan Salinan**, **Timpa** berkas, atau **Batal**.

Berkas baru ditulis lebih dulu di samping berkas lama, dan hanya menggantikan berkas lama setelah selesai dengan lengkap. Jika ada yang gagal, Anda akan melihat pesan, dan berkas yang semula ada tetap seperti sebelumnya.

Jika rumus masih dihitung saat Anda menyimpan, JeffCrunch menunggu sampai selesai, sehingga berkas memuat hasilnya. Jika Anda menekan **Esc** untuk menghentikan perhitungan sementara itu, penyimpanan dibatalkan, dan bilah status menyebutkannya; simpan lagi saat Anda menginginkannya. JeffCrunch tidak menyertakan satu hal dalam berkas: daftar urutan rumus terakhir dihitung oleh suatu program (rantai perhitungan). Program yang membuka buku kerja berikutnya akan membuatnya kembali. Jika JeffCrunch tidak dapat menghitung sebagian rumus, atau Anda menghentikan perhitungan, berkas meminta program tersebut menghitung ulang semua rumus saat membuka buku kerja.

## Menutup dengan perubahan

Jika Anda menutup buku kerja yang memiliki perubahan yang belum disimpan, JeffCrunch menanyakan apakah akan **Simpan** perubahan itu, **Jangan Simpan**, atau **Batal** dan melanjutkan pekerjaan. Tanda bintang (*) pada judul jendela menunjukkan adanya perubahan yang belum disimpan.

## Templat bawaan

Pada **File > Baru**, di bawah buku kerja kosong, terdapat buku kerja siap pakai yang dikelompokkan: **Keuangan** (anggaran bulanan, jadwal pinjaman, dan register buku cek), **Bisnis** (faktur, laporan pengeluaran, lembar waktu kerja, daftar inventaris, catatan jarak tempuh, dan laporan penjualan), **Perencanaan** (kalender dan rencana proyek), dan **Pendidikan** (buku nilai). Setiap templat menampilkan gambar kecil di bagian atasnya. Klik salah satunya untuk memulai buku kerja baru darinya. Ketik di kotak berwarna kuning. Angka-angka lainnya dihitung untuk Anda. Buku kerja tidak memiliki berkas sendiri sampai Anda menyimpannya.
