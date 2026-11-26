# Cara rumus ditampilkan dan dihitung
<!-- keywords: tumpah, tumpahan, #SPILL!, larik dinamis, rentang tumpahan, A1#, kesalahan berikutnya, kesalahan sebelumnya, telusuri kesalahan, lacak kesalahan, hitung sebelum menyimpan, rumus, rumus-rumus, hitung, perhitungan, hitung ulang, fungsi, fungsi-fungsi, nilai, hasil, tersimpan, F9, sirkular, tampilkan rumus, bilah rumus, kesalahan, titik, menunjuk, klik sel, referensi, F4, pemeriksaan kesalahan, abaikan kesalahan, evaluasi rumus, langkah, jendela pantau, pantau, let, beri nama nilai, fungsi sel, fungsi info, filterxml, xml, xpath, presisi sesuai tampilan, atur presisi sesuai tampilan, bulatkan, pembulatan, desimal, digit-->

Rumus adalah sel yang menghitung nilainya dari sel lain, misalnya =SUM(B3:B8). Sel menampilkan nilainya, dan **bilah rumus** menampilkan rumusnya.

## Yang dilakukan JeffCrunch terhadap rumus

Saat Anda membuka buku kerja, berkas menyimpan nilai terakhir dari setiap rumus, sehingga lembar dapat langsung ditampilkan. Kemudian JeffCrunch menghitung ulang setiap rumus, sedikit demi sedikit, agar jendela tetap cepat. Bilah status menampilkan **Menghitung** dengan persen selama proses berlangsung. Setelah selesai, sel berisi hasil perhitungan JeffCrunch sendiri. Untuk menghitung ulang semuanya, klik **Hitung Ulang Semua** pada tab **Rumus**, atau tekan **F9**.

## Saat JeffCrunch tidak dapat menghitung rumus

JeffCrunch memiliki lebih dari 400 fungsi, dan jumlahnya terus bertambah. Rumus yang memakai fungsi yang belum dimiliki JeffCrunch tetap menyimpan nilai yang disimpan berkas untuknya. Begitu pula rumus yang merujuk ke buku kerja lain. Sel yang bergantung pada rumus tersebut dihitung dari nilai yang tersimpan.

## Fungsi yang memerlukan beberapa catatan

- **Obligasi dan pinjaman.** PRICE, YIELD, DURATION, ODDFPRICE, dan fungsi obligasi lainnya menghitung jumlah hari sesuai **basis** yang Anda berikan (0 berarti 30 hari per bulan dalam setahun 360 hari, 1 berarti hari sebenarnya, dan seterusnya). YIELD, ODDFYIELD, ODDLYIELD, dan RATE mendapatkan hasilnya dengan mencoba-coba nilai hingga harga atau pembayaran cocok. Perhitungan berhenti ketika hasilnya tepat hingga sekitar 15 digit, sehingga digit terakhir dapat berbeda dari program lain.
- **Distribusi dan uji.** Fungsi yang berakhiran .DIST, .INV, .TEST, serta nama lama seperti NORMDIST dan TDIST, memberikan hasil yang sama. Nama lama tetap dipertahankan agar berkas lama masih dapat digunakan. Fungsi *.INV juga dicari dengan mencoba-coba nilai.
- **Daftar yang mengisi beberapa sel.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE, dan LINEST menghasilkan sekumpulan nilai. Ketik satu dalam satu sel dan kumpulan itu akan tumpah ke sel di bawah dan di sampingnya (lihat **Rumus yang mengisi sel di sampingnya**), atau masukkan sebagai rumus larik (**Ctrl+Shift+Enter**) pada sel-sel yang harus diisi.
- **Teks dalam aksara lain.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB, dan REPLACEB menghitung setiap karakter sebagai satu, seperti pada komputer yang tidak diatur untuk teks dua-byte. PHONETIC memberikan teks sel itu sendiri, karena JeffCrunch tidak menyimpan bacaan teks Jepang.
- **CONVERT.** Satuan ditulis sebagai teks, dengan huruf besar dan kecil seperti yang tertera pada daftar fungsi: "kg", "lbm", "mi", "F". Awalan metrik dapat ditempatkan di depan satuan yang menerimanya, seperti "km" atau "mL".

## MOD: sisa bagi

**MOD(n, d)** memberikan sisa dari **n** setelah sebanyak mungkin kelipatan utuh **d** dikurangkan: hasilnya n - d * INT(n / d). Contoh: **=MOD(10, 3)** menghasilkan 1, dan **=MOD(-10, 3)** menghasilkan 2, karena INT(-10 / 3) adalah -4 dan -10 - 3 * -4 adalah 2. Hasilnya memiliki tanda dari **d**. Jika **d** bernilai 0, hasilnya #DIV/0!.

## Tampilkan rumus

Klik **Tampilkan Rumus** pada tab **Rumus**, atau tekan **Ctrl+`** (di Mac, tombol Control, bukan tombol Command), untuk melihat rumus di setiap sel sebagai ganti nilainya. Kolom yang berisi rumus dilebarkan, hingga batas tertentu, agar rumus terlihat utuh. Lebarnya kembali seperti semula saat Anda mematikan fitur ini, dan lebar kolom dalam berkas Anda tidak berubah.

## Mengubah rumus

Untuk mengubah rumus, pilih selnya, tekan **F2**, edit, lalu tekan **Enter**. Anda juga dapat mengeditnya di bilah rumus. Saat Anda mengubah sebuah sel, rumus yang bergantung padanya dihitung ulang di latar belakang. Lihat [Mengetik di sel](edit-cells).

## Menulis rumus

Ketik **=** dan huruf-huruf awal sebuah fungsi, lalu daftar singkat akan menampilkan fungsi dan [nama](names) yang diawali huruf tersebut, dengan satu kalimat keterangan untuk setiap item. Tekan **Tab** untuk memilih item yang ditandai. Gunakan panah **Bawah** dan **Atas** untuk menandai item lain, lalu **Tab** atau **Enter**. **Esc** menutup daftar. Setelah Anda berada di dalam tanda kurung sebuah fungsi, argumennya ditampilkan di bawah sel, dengan argumen yang sedang Anda ketik ditebalkan. Daftar dan argumen tersebut sama dengan yang ditampilkan **Pilih Fungsi**. Argumen dipisahkan dengan koma, atau dengan titik koma pada bahasa yang menggunakan koma sebagai tanda desimal.

## Nama fungsi dalam bahasa Anda

Saat JeffCrunch ditampilkan dalam bahasa Jerman, Prancis, Spanyol, Italia, Belanda, Portugis, atau Rusia, Anda dapat mengetik rumus dengan nama fungsi yang digunakan bahasa tersebut, misalnya `=SUMME(A1;A2)` dalam bahasa Jerman atau `=SOMME(A1;A2)` dalam bahasa Prancis. Nama bahasa Inggris juga berfungsi, sehingga rumus yang Anda salin dari web tidak perlu diubah. Bilah rumus, sel, daftar fungsi, dan **Pilih Fungsi** menampilkan nama dalam bahasa Anda.

Buku kerja tidak berubah saat Anda mengganti bahasa. Berkas selalu menyimpan nama bahasa Inggris, sehingga buku kerja yang dibuat dalam satu bahasa dapat dibuka dan dibaca dengan benar dalam bahasa lain, dan dalam program lain mana pun. Nama lokal tersedia untuk bahasa Jerman, Prancis, Spanyol, Italia, Belanda, Portugis (Brasil dan Portugal), dan Rusia; nama bahasa Inggris selalu berfungsi di setiap bahasa. Program bahasa Ukraina dan Indonesia tetap menggunakan nama bahasa Inggris untuk hampir semua fungsi. Jika suatu bahasa memiliki dua nama umum untuk satu fungsi, keduanya dikenali dan yang lebih umum ditampilkan. Fungsi yang tidak memiliki nama lazim dalam bahasa Anda tetap menggunakan nama bahasa Inggris. TRUE dan FALSE juga mengikuti bahasa Anda. Nilai galat dalam bahasa Anda, misalnya `#NV` dalam bahasa Jerman, dikenali saat Anda mengetik rumus dan ditampilkan di bilah rumus, tetapi sel yang berisi galat selalu menampilkan bentuk standar, seperti `#N/A`.

Jika pengaturan regional komputer Anda menggunakan koma sebagai tanda desimal, argumen fungsi dipisahkan dengan titik koma. Larik dalam kurung kurawal dengan lebih dari satu baris ditampilkan dengan titik koma di antara nilai dalam satu baris dan garis miring terbalik di antara baris, misalnya `{1;2\3;4}`, dan Anda dapat mengetiknya dengan cara itu. Larik yang diketik tanpa garis miring terbalik dibaca seperti cara berkas menulisnya: koma di antara nilai dalam satu baris, titik koma di antara baris, dan titik sebagai tanda desimal.

Untuk hanya mengetik nama bahasa Inggris, klik **Berkas > Opsi > Rumus** lalu centang **Gunakan nama fungsi bahasa Inggris**.

## Menunjuk sel

Alih-alih mengetik alamat sel, Anda dapat menunjuknya. Saat mengetik rumus, letakkan kursor di tempat sel dapat ditulis (tepat setelah **=**, operator seperti **+**, tanda kurung buka, atau koma), lalu klik sel tersebut. Alamatnya akan ditulis untuk Anda. Seret di atas sel untuk menulis rentang seperti B2:D5. Klik lagi sebelum mengetik hal lain, dan sel baru menggantikan sel yang baru saja ditulis. Tahan **Ctrl** lalu klik untuk menambahkan alamat lain setelah koma. Klik huruf kolom untuk menulis seluruh kolom seperti A:A, atau nomor baris untuk menulis seluruh baris seperti 3:3, dan seret di sepanjang huruf atau nomor untuk B:D atau 2:5.

Tombol panah juga berfungsi. Tekan panah dan sebuah penunjuk bergerak menjauh dari sel tempat Anda mengetik, dan alamatnya ditulis; tekan panah lagi untuk bergerak lebih jauh, atau tahan **Shift** untuk memperluas penunjuk ke seluruh blok. Bilah status menampilkan **Tunjuk** saat tombol panah sedang menunjuk, **Enter** saat Anda mengetik, dan **Edit** saat tombol panah menggerakkan kursor di dalam teks. Tekan **F2** untuk beralih antara menunjuk dan mengedit teks.

Untuk menunjuk sel pada lembar lain, klik tab lembar tersebut saat kursor berada di tempat sel dapat ditulis, lalu klik selnya. Nama lembar ditulis bersama alamat, diapit tanda kutip jika diperlukan, seperti 'Lembar Saya'!B3. **Enter** membawa Anda kembali ke lembar awal dan menempatkan rumus di selnya. **Esc** membatalkan semuanya, termasuk perpindahan lembar.

Anda juga dapat menyeret batas blok yang diberi garis tepi pada lembar untuk memindahkan alamat tersebut, atau menyeret kotak sudutnya untuk memperluasnya; rumus berubah saat Anda menyeret.

Tekan **F4** dengan kursor berada di dalam atau tepat setelah sebuah alamat untuk mengunci bagian-bagiannya dengan tanda dolar: A1 menjadi $A$1, lalu A$1, lalu $A1, lalu kembali ke A1.

Setiap alamat dalam rumus diberi warna di dalam teks, dan sel yang dirujuknya diberi garis tepi di lembar dengan warna yang sama, sehingga Anda dapat melihat sekilas apa yang dibaca rumus. Klik setelah alamat lengkap, seperti =A1, mengakhiri entri dan memilih sel yang Anda klik, sama seperti untuk teks lainnya.

## Tabel data

Tabel data (tabel what-if) menunjukkan hasil rumus untuk banyak nilai dari satu atau dua sel. Saat Anda membuka buku kerja yang memilikinya, JeffCrunch menghitung selnya. Untuk tabel dengan dua masukan, rumus berada di sel pojok kiri atas tabel, nilai yang dicoba secara mendatar dimasukkan ke sel masukan baris, dan nilai yang dicoba secara vertikal dimasukkan ke sel masukan kolom. Untuk tabel dengan satu masukan, nilai ditempatkan menurun di kolom di sebelah kirinya (dan rumusnya berada di baris di atasnya), atau mendatar di baris di atasnya (dan rumusnya berada di kolom di sebelah kirinya). Setiap sel tabel adalah hasil rumus, dan setiap rumus yang bergantung pada sel masukan, dengan masukan diatur ke nilai tersebut; tidak ada bagian lain dalam buku kerja yang diubah. Dalam mode **Otomatis**, tabel dihitung bersama rumus, dan rumus yang membaca sel-sel tabel ikut diperbarui. Membuat tabel data baru belum tersedia. Tabel yang tidak dapat dihitung (rumus yang bergantung pada masukan memakai fungsi yang tidak dimiliki JeffCrunch, atau berubah dengan sendirinya, seperti RAND atau NOW) tetap menyimpan nilai yang ada di berkasnya.

## Menghitung secara manual

Klik **Mode Perhitungan** pada tab **Rumus** untuk memilih cara buku kerja menghitung rumusnya:

- **Otomatis** menghitung rumus saat Anda mengubah sel.
- **Otomatis, Tabel Manual** sama, tetapi membiarkan sel tabel data tetap seperti semula hingga Anda menekan **F9**.
- **Manual** tidak menghitung apa pun, termasuk tabel data, hingga Anda memintanya. Bilah status menampilkan **Belum diperbarui** setelah ada perubahan. Tekan **F9** (**Hitung Ulang Semua**) untuk menghitung semuanya, atau **Shift+F9** (**Hitung Ulang Lembar Ini**) untuk menghitung rumus pada lembar yang sedang Anda buka, beserta apa pun yang menunggu. Buku kerja yang diatur ke Manual dibuka dengan hasil yang tersimpan dalam berkasnya. **Hitung Sebelum Menyimpan**, juga pada menu **Mode Perhitungan**, dicentang sejak awal: saat Anda menyimpan buku kerja Manual yang belum diperbarui, rumus dihitung lebih dulu sehingga berkas berisi angka terbaru. Matikan opsi ini untuk menyimpan angka apa adanya.

Pilihan ini disimpan bersama buku kerja, dan merupakan satu langkah yang dapat Anda urungkan.

## Rumus yang membaca dirinya sendiri

Rumus yang membaca hasilnya sendiri, langsung atau melalui sel lain, tidak dapat dihitung dengan cara biasa. JeffCrunch memberi tahu Anda sekali, menyebutkan sel-selnya, dan memberi garis tepi pada sel tersebut di lembar; sel-sel itu tetap menyimpan nilai yang dimilikinya. Klik **Referensi Sirkular** pada tab **Rumus** untuk melihat daftarnya, dan klik ganda sel dalam daftar untuk pergi ke sana.

Beberapa model memang memerlukan lingkaran seperti ini. Pilih **Iterasi Rumus Sirkular** pada menu **Mode Perhitungan**, dan rumus akan dihitung berulang kali, setiap kali membaca nilai terbaru dari rumus lain, hingga tidak ada nilai yang berubah lebih dari jumlah yang Anda tentukan atau batas jumlah putaran tercapai.

## Simpan angka seperti yang ditampilkan sel

Sebuah sel dapat menampilkan lebih sedikit digit daripada yang disimpannya. Jika sebuah sel berisi 2,675 dan menampilkan 2,68, rumus yang membacanya tetap menggunakan 2,675. Terkadang Anda ingin rumus menggunakan apa yang terlihat. Dalam kasus itu, pilih **Atur Presisi sesuai Tampilan** pada menu **Opsi Perhitungan** di tab **Rumus**.

JeffCrunch akan memperingatkan Anda terlebih dahulu, karena digit yang tersembunyi akan hilang. Pilih **Batal** dan tidak ada yang berubah. Pilih **OK** dan setiap angka dalam buku kerja dibulatkan sesuai yang ditampilkan selnya. Sel yang menampilkan dua desimal tetap memiliki dua desimal. Sel yang menampilkan persen tetap berupa persen. Sel yang berisi tanggal tetap mempertahankan harinya, dan sel yang berisi waktu mempertahankan bagian waktu terkecil yang ditampilkan, seperti menit. Sel yang menampilkan **Umum** mempertahankan semua digitnya. Buku kerja yang besar menampilkan catatan dengan tombol **Batal** selama proses ini. Klik Batal saat catatan itu masih tampil, dan tidak ada yang diubah.

Setelah itu, setiap angka yang Anda ketik atau tempel, dan setiap hasil rumus, dibulatkan dengan cara yang sama. Rumusnya sendiri tidak diubah. Pilihan ini disimpan bersama buku kerja.

Pembulatan adalah satu langkah yang dapat Anda urungkan, dan **Urungkan** mengembalikan angka-angka tersebut. Memilih **Atur Presisi sesuai Tampilan** lagi akan mematikan fitur ini. Itu hanya menghentikan pembulatan mulai saat itu. Angka yang sudah dibulatkan tetap dibulatkan.

## Melihat apa yang dibaca sebuah rumus

Pilih sebuah sel lalu klik **Tampilkan Sumber Sel Ini** untuk menggambar panah dari sel-sel yang dibaca rumusnya. Klik lagi untuk melangkah satu tingkat lebih jauh ke belakang. **Tampilkan Penggunaan Sel Ini** menggambar panah ke rumus-rumus yang membaca sel tersebut, dan setiap klik melangkah satu tingkat lebih jauh. Halaman kecil mewakili sel pada lembar lain. Paling banyak 2.000 panah digambar, dan bilah status memberi tahu jika ada lebih banyak. **Hapus Panah** menghapusnya, dan panah juga hilang setiap kali buku kerja diubah.

Saat sebuah sel menampilkan galat seperti #DIV/0!, pilih sel tersebut lalu klik **Telusuri Kesalahan**. Panah digambar ke sel-sel yang dibacanya dan juga menampilkan galat. Klik lagi untuk melangkah satu tingkat lebih jauh ke belakang. Jika tidak ada sel yang dibacanya menampilkan galat, bilah status memberi tahu bahwa sel ini yang menyebabkan galat, dan itulah sel yang perlu diperbaiki.

Untuk memeriksa setiap galat pada lembar, klik **Kesalahan Berikutnya** atau **Kesalahan Sebelumnya**. Sel-sel yang menampilkan galat dikunjungi baris demi baris, dan setelah yang terakhir, pencarian dimulai lagi dari yang pertama. Setiap sel dipilih, dan panah digambar hingga kembali ke sel yang menyebabkan galat. Bilah status menyebutkan galat yang sedang Anda lihat dan jumlah galat seluruhnya.

## Pemeriksaan Kesalahan

Pilih **Pemeriksaan Kesalahan** pada grup **Periksa** di tab Rumus untuk memeriksa lembar dari sel-sel yang mungkin keliru. Fitur ini mencari rumus yang menghasilkan galat (seperti #DIV/0!), angka yang disimpan sebagai teks (yang mungkin dilewati rumus), dan lainnya: rumus yang tidak sama dengan rumus di sebelahnya, tanggal yang disimpan sebagai teks dengan tahun dua digit, rumus di sel yang tidak dikunci, rumus yang tidak menyertakan angka di sebelah sel yang digunakannya (hanya untuk fungsi yang menjumlahkan suatu rentang, seperti **SUM**, **AVERAGE**, **COUNT**, **MIN**, atau **MAX**, ketika rentang itu berdiri sendiri sebagai argumen), dan rumus yang memakai sel kosong.

Kotak dialog menampilkan satu sel setiap kali, membawa Anda ke sel tersebut di lembar, dan menjelaskan dalam satu kalimat apa masalahnya. **Berikutnya** dan **Sebelumnya** berpindah di antara sel. Segitiga kecil berwarna hijau di pojok kiri atas sel menandai sel tersebut di lembar. Klik segitiga itu untuk membuka menu di samping sel: menu itu menjelaskan masalahnya, menawarkan perbaikan jika ada, dan memiliki **Abaikan Kesalahan** serta **Pemeriksaan Kesalahan…**. Jika sel dapat diperbaiki, sebuah tombol dengan perbaikannya muncul (untuk rumus yang tidak menyertakan angka, **Sertakan Sel**); tombol itu mengubah rumus, dan satu langkah Urungkan mengembalikan rumus lama. **Abaikan Kesalahan** mengeluarkan sel dari daftar, menghilangkan segitigananya, dan disimpan dalam berkas. Satu langkah Urungkan akan mengembalikannya. Jika tidak ada yang keliru, Anda akan diberi tahu. Untuk memilih jenis mana yang ditandai, buka **Opsi** lalu halaman **Rumus**: ada kotak centang untuk setiap jenis, dan kotak yang Anda hapus centangnya menghentikan jenis tersebut dari penandaan. Pilihan ini berlaku untuk program, sehingga berlaku untuk setiap jendela yang terbuka. Sebuah sel dapat memiliki lebih dari satu masalah, dan **Abaikan Kesalahan** hanya mengabaikan masalah yang sedang ditampilkan.

## Evaluasi Rumus

Gunakan ini untuk melihat bagaimana sebuah rumus mencapai nilainya. Pilih sel yang berisi rumus lalu pilih **Evaluasi Rumus**. Rumus ditampilkan dengan bagian berikutnya bergaris bawah. Tekan **Evaluasi** untuk menghitung bagian itu; nilainya menggantikan bagian tersebut. Bagian-bagian diproses dengan urutan: setiap sel tunggal dari kiri, lalu setiap fungsi beserta tanda kurungnya, dimulai dari yang paling dalam, lalu sisanya. **Mulai Ulang** memulai dari awal. Jika suatu bagian tidak dapat dihitung, kotak dialog menjelaskan alasannya dan membiarkan rumus seperti semula.

## Jendela Pantau

**Jendela Pantau** adalah jendela kecil yang tetap terlihat saat Anda bekerja. Jendela ini mendaftar sel yang ingin Anda awasi, dengan lembar, sel, nilai, dan rumusnya, bahkan jika sel tersebut berada di lembar lain. Pilih sel lalu tekan **Tambah Pantauan**. Pilih baris dalam daftar lalu tekan **Hapus Pantauan**. Klik ganda sebuah baris untuk pergi ke sel. Nilai berubah saat lembar dihitung ulang. Daftar disimpan selama buku kerja terbuka.

## Rumus yang mengisi sel di sampingnya

Saat sebuah rumus menghasilkan sekumpulan nilai, kumpulan itu mengisi sel di bawah dan di samping sel tempat Anda mengetiknya. Ini disebut tumpahan. Coba =SEQUENCE(3) di A1: A1 menampilkan 1, dan A2 serta A3 menampilkan 2 dan 3. Sebuah rumus tumpah saat memanggil fungsi yang menghasilkan kumpulan (SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, TRANSPOSE, XLOOKUP, atau MMULT), menulis kumpulan dalam kurung kurawal, memakai rentang tumpahan (di bawah), atau melakukan operasi aritmetika pada sebuah rentang, seperti =A1:A3*2.

- **Sel yang diisi tidak memiliki rumus sendiri.** Sel tersebut hanya menampilkan nilai. Saat Anda memilih salah satunya, bilah rumus menampilkan rumus dari sel yang mengisinya, dengan warna abu-abu. Menekan Enter pada sel itu tanpa perubahan tidak mengubah apa pun. Untuk mengubah kumpulan, ubahlah sel pertama.
- **Kumpulan mengikuti hasilnya.** Jika rumus menghasilkan lebih banyak atau lebih sedikit nilai setelah sesuatu yang dibacanya berubah, kumpulan bertambah atau berkurang, dan rumus yang membaca sel-sel tersebut dihitung ulang.
- **Sel yang menghalangi menyebabkan #SPILL!.** Jika ada sel tempat kumpulan akan masuk yang berisi nilai atau rumus, sel pertama menampilkan **#SPILL!** dan tidak ada yang diisi. Kosongkan sel yang menghalangi dan kumpulan langsung terisi. Mengetik di sel yang diisi kumpulan juga menyebabkan hal yang sama: sel pertama menampilkan #SPILL!.
- **Kumpulan memiliki batas ukuran.** Kumpulan yang lebih besar dari lembar, atau lebih dari 200.000 sel, menghasilkan #SPILL!.
- **A1# adalah seluruh kumpulan.** Tulis sel yang berisi rumus diikuti **#**, seperti =SUM(A1#), untuk memakai seluruh kumpulan sebagaimana adanya saat ini, dari lembar ini atau lembar lain (=SUM(Sheet1!A1#)). Jika A1 tidak menumpah, A1# menjadi #REF!.

Tumpahan disimpan bersama berkas sebagai larik dinamis, sehingga program asal berkas juga menampilkannya sebagai tumpahan. Menyimpan ke bentuk .xls atau .ods yang lebih lama mempertahankan nilainya, dan rumusnya sebagai rumus larik pada kumpulan tersebut, jika bentuk lama memiliki fungsi yang dipakai. Periksa Kompatibilitas akan menyebutkan rumus-rumus ini.

## Beri nama nilai di dalam rumus

**LET** memberi nama pada sebuah nilai di tengah rumus, sehingga Anda menulisnya sekali dan dapat menggunakannya kembali. Pasangan nama dan nilai ditulis lebih dulu, dan perhitungannya di akhir. Dalam =LET(harga, B2, pajak, harga*0,08, harga+pajak), nama **harga** mewakili nilai di B2, **pajak** dihitung darinya, dan bagian terakhir menjumlahkannya. Sebuah nama dapat memakai nama sebelumnya, dan satu LET dapat memuat hingga 126 pasangan. Sebuah nama hanya berlaku di dalam LET-nya sendiri, dan LET di dalam LET lain boleh memakai nama yang sama lagi. Sebuah nama juga dapat mewakili rentang sel, seperti =LET(baris, B2:B9, SUM(baris)/COUNT(baris)).

## Tanyakan tentang sel atau buku kerja

**CELL** memberikan informasi tentang sebuah sel. Argumen pertamanya menentukan apa yang diberikan: "address", "row", "col", "contents", "type", "format" (cara angka ditampilkan, seperti "F2" untuk dua desimal, "P0" untuk persen, atau "D1" untuk tanggal), "color" (1 jika angka negatif ditampilkan berwarna), "parentheses" (1 jika angka positif ditampilkan dalam kurung), "prefix" (tanda teks yang rata kiri, kanan, atau tengah), "protect" (1 jika sel dikunci), "width" (lebar kolom dalam karakter), dan "filename" (folder, berkas, dan lembar, seperti /home/saya/[Anggaran.xlsx]Sheet1; kosong sampai buku kerja disimpan). **INFO** memberikan informasi tentang program dan buku kerja: "directory", "numfile" (berapa banyak lembar yang ditampilkan), "origin", "osversion", "recalc" (Otomatis atau Manual), "release", dan "system".

## Ambil nilai dari XML

**FILTERXML** menerima teks yang berisi XML dan sebuah XPath, yaitu cara singkat untuk menyatakan bagian mana yang Anda inginkan, lalu memberikan nilai yang ditemukannya. =FILTERXML(A1, "//item/price") memberikan setiap harga dalam teks di A1, satu di bawah yang lain. Gunakan "/shop/item[2]/name" untuk item kedua, "//item[@id='b']/name" untuk item yang id-nya b, atau "count(//item)" untuk jumlah item. Jika teks bukan XML yang valid, atau jalurnya tidak valid, hasilnya #VALUE!. Jika tidak ada yang cocok, hasilnya #N/A. XML yang memiliki tipe dokumen ditolak.
