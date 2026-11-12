# Cara rumus ditampilkan dan dihitung
<!-- keywords: tumpah, tumpah, #SPILL!, rumus array dinamis, rentang tumpahan, A1#, kesalahan berikutnya, kesalahan sebelumnya, telusuri kesalahan, telusuri kesalahan, hitung sebelum menyimpan, rumus, rumus, hitung, perhitungan, hitung ulang, fungsi, fungsi, nilai, hasil, tersimpan, F9, sirkular, tampilkan rumus, bilah rumus, kesalahan, titik, menunjuk, klik sel, referensi, F4, pemeriksaan kesalahan, abaikan kesalahan, evaluasi rumus, langkah, jendela pantau, pantau, LET, beri nama nilai, fungsi sel, fungsi info, filterxml, xml, xpath-->

Sebuah **rumus** adalah sel yang menghitung nilainya dari sel lain, misalnya =SUM(B3:B8). Sel menampilkan nilainya, dan **bilah rumus** menampilkan rumusnya.

## Apa yang dilakukan JeffCrunch terhadap rumus

Saat Anda membuka buku kerja, berkas menyimpan nilai terakhir dari setiap rumus, sehingga lembar dapat langsung ditampilkan. Kemudian JeffCrunch menghitung ulang setiap rumus, sedikit demi sedikit, agar jendela tetap cepat. Bilah status menampilkan **Menghitung** dengan persentase selama proses itu berjalan. Setelah selesai, sel berisi hasil perhitungan JeffCrunch sendiri. Untuk menghitung ulang semuanya, klik **Hitung Ulang Semua** pada tab **Rumus**, atau tekan **F9**.

## Saat JeffCrunch tidak dapat menghitung rumus

JeffCrunch memiliki lebih dari 400 fungsi, dan jumlahnya terus bertambah. Rumus yang memakai fungsi yang belum dimiliki JeffCrunch akan tetap menyimpan nilai yang disimpan berkas untuknya. Begitu pula rumus yang merujuk ke buku kerja lain. Sel yang bergantung padanya dihitung dari nilai yang tersimpan.

## Fungsi yang memerlukan beberapa catatan

- **Obligasi dan pinjaman.** PRICE, YIELD, DURATION, ODDFPRICE, dan fungsi obligasi lainnya menghitung jumlah hari sesuai **basis** yang Anda berikan (0 berarti 30 hari per bulan dalam tahun 360 hari, 1 berarti hari sebenarnya, dan seterusnya). YIELD, ODDFYIELD, ODDLYIELD, dan RATE mencari jawabannya dengan mencoba nilai hingga harga atau pembayaran sesuai. Perhitungan berhenti ketika jawabannya tepat hingga sekitar 15 digit, sehingga digit terakhir dapat berbeda dari program lain.
- **Distribusi dan uji.** Fungsi yang berakhiran .DIST, .INV, .TEST, serta nama lama seperti NORMDIST dan TDIST, memberikan hasil yang sama: nama lama tetap dipertahankan agar berkas lama tetap berfungsi. Fungsi *.INV juga dicari dengan mencoba nilai.
- **Daftar yang mengisi beberapa sel.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE, dan LINEST menghasilkan blok nilai. Ketik satu di sel tunggal dan blok itu akan tumpah ke sel di bawah dan di sampingnya (lihat **Rumus yang mengisi sel di sampingnya**), atau masukkan sebagai rumus array (**Ctrl+Shift+Enter**) pada sel-sel yang harus diisi.
- **Teks dalam aksara lain.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB, dan REPLACEB menghitung setiap karakter sebagai satu, seperti pada komputer yang tidak diatur untuk teks dua-byte. PHONETIC memberikan teks sel itu sendiri, karena JeffCrunch tidak menyimpan bacaan teks Jepang.
- **CONVERT.** Satuan ditulis sebagai teks, dengan huruf besar dan kecil seperti yang tertera pada daftar fungsi: "kg", "lbm", "mi", "F". Awalan metrik dapat diletakkan di depan satuan yang menerimanya, seperti "km" atau "mL".

## MOD: sisa bagi

**MOD(n, d)** memberikan sisa dari **n** setelah sebanyak mungkin kelipatan utuh **d** dikeluarkan: hasilnya n - d * INT(n / d). Contoh: **=MOD(10, 3)** menghasilkan 1, dan **=MOD(-10, 3)** menghasilkan 2, karena INT(-10 / 3) adalah -4 dan -10 - 3 * -4 adalah 2. Hasilnya memiliki tanda dari **d**. Jika **d** bernilai 0, hasilnya #DIV/0!.

## Tampilkan rumus

Klik **Tampilkan Rumus** pada tab **Rumus**, atau tekan **Ctrl+`** (di Mac, tombol Control, bukan tombol Command), untuk melihat rumus di setiap sel sebagai ganti nilainya. Kolom yang berisi rumus dibuat lebih lebar, hingga batas tertentu, agar rumus tampil utuh; kolom kembali ke lebarnya semula saat fitur ini dimatikan, dan lebar kolom dalam berkas Anda tidak berubah.

## Mengubah rumus

Untuk mengubah rumus, pilih selnya, tekan **F2**, edit, lalu tekan **Enter**. Anda juga dapat mengeditnya di bilah rumus. Ketika Anda mengubah sebuah sel, rumus yang bergantung padanya dihitung ulang di latar belakang. Lihat [Ketik dalam sel](edit-cells).

## Menulis rumus

Ketik **=** dan beberapa huruf pertama dari sebuah fungsi, maka daftar pendek akan menampilkan fungsi dan [nama](names) yang diawali huruf tersebut, dengan satu kalimat keterangan untuk masing-masing. Tekan **Tab** untuk memilih yang ditandai. Gunakan tombol panah **Bawah** dan **Atas** untuk menandai yang lain, lalu tekan **Tab** atau **Enter**. **Esc** menutup daftar. Setelah Anda berada di dalam kurung fungsi, argumennya ditampilkan di bawah sel, dengan argumen yang sedang Anda ketik ditebalkan. Daftar dan argumen ini sama dengan yang ditampilkan **Pilih Fungsi**. Argumen dipisahkan dengan koma, atau dengan titik koma jika bahasa Anda menggunakan koma sebagai tanda desimal.

## Menunjuk sel

Alih-alih mengetik alamat sel, Anda dapat menunjuknya. Saat mengetik rumus, letakkan kursor di tempat sel dapat ditulis (tepat setelah **=**, operator seperti **+**, tanda kurung buka, atau koma), lalu klik sel. Alamatnya akan ditulis untuk Anda. Seret di atas sel untuk menulis blok seperti B2:D5. Klik lagi sebelum Anda mengetik hal lain, dan sel baru menggantikan sel yang baru saja ditulis. Tahan **Ctrl** lalu klik untuk menambahkan alamat lain setelah koma. Klik huruf kolom untuk menulis seluruh kolom seperti A:A, atau nomor baris untuk menulis seluruh baris seperti 3:3, dan seret sepanjang huruf atau nomor untuk B:D atau 2:5.

Tombol panah juga berfungsi. Tekan panah dan penunjuk akan bergerak menjauh dari sel tempat Anda mengetik, dan alamatnya ditulis; tekan panah lagi untuk bergerak lebih jauh, atau tahan **Shift** untuk memperluas penunjuk ke sebuah blok. Bilah status menampilkan **Tunjuk** ketika tombol panah sedang menunjuk, **Enter** saat Anda mengetik, dan **Edit** ketika tombol panah menggerakkan kursor dalam teks. Tekan **F2** untuk beralih antara menunjuk dan mengedit teks.

Untuk menunjuk sel pada lembar lain, klik tab lembar tersebut selagi kursor berada di tempat sel dapat ditulis, lalu klik selnya. Nama lembar ditulis bersama alamat, dengan tanda kutip jika diperlukan, misalnya 'Lembar Saya'!B3. **Enter** membawa Anda kembali ke lembar tempat Anda memulai dan menempatkan rumus di selnya. **Esc** membatalkan semuanya, termasuk perpindahan lembar.

Anda juga dapat menyeret batas blok yang diberi garis tepi pada lembar untuk memindahkan alamat tersebut, atau menyeret kotak sudutnya untuk memperluas blok; rumus berubah saat Anda menyeret.

Tekan **F4** dengan kursor berada di dalam atau tepat setelah sebuah alamat untuk mengunci sebagian alamat dengan tanda dolar: A1 menjadi $A$1, lalu A$1, lalu $A1, lalu kembali ke A1.

Setiap alamat dalam rumus diberi warna pada teksnya, dan sel yang dirujuknya diberi garis tepi pada lembar dengan warna yang sama, sehingga Anda dapat melihat sekilas apa yang dibaca rumus. Klik setelah alamat utuh, seperti =A1, mengakhiri entri dan memilih sel yang Anda klik, sebagaimana pada teks lainnya.

## Tabel data

Tabel data (tabel what-if) menampilkan hasil sebuah rumus untuk banyak nilai dari satu atau dua sel. Saat Anda membuka buku kerja yang memilikinya, JeffCrunch menghitung selnya: untuk tabel dengan dua masukan, rumus berada di sel pojok kiri atas tabel, nilai yang dicoba ke samping ditempatkan di sel masukan baris, dan nilai ke bawah ditempatkan di sel masukan kolom. Untuk tabel dengan satu masukan, nilai ditempatkan ke bawah pada kolom di sebelah kirinya (dan rumusnya berada di baris di atasnya), atau ke samping pada baris di atasnya (dan rumusnya berada di kolom di sebelah kirinya). Setiap sel tabel adalah hasil rumus, dan setiap rumus yang bergantung pada sel masukan, dengan masukan diatur ke nilai tersebut; tidak ada bagian lain buku kerja yang diubah. Dalam mode **Otomatis**, tabel dihitung bersama rumus, dan rumus yang membaca sel-sel tabel akan mengikutinya. Membuat tabel data baru belum tersedia. Tabel yang tidak dapat dihitung (rumus yang bergantung pada masukan memakai fungsi yang tidak dimiliki JeffCrunch, atau berubah dengan sendirinya, seperti RAND atau NOW) tetap menyimpan nilai yang ada di berkasnya.

## Menghitung secara manual

Klik **Mode Perhitungan** pada tab **Rumus** untuk memilih cara buku kerja menghitung rumusnya:

- **Otomatis** menghitungnya saat Anda mengubah sel.
- **Otomatis, Tabel Manual** melakukan hal yang sama, tetapi membiarkan sel-sel tabel data seperti apa adanya hingga Anda menekan **F9**.
- **Manual** tidak menghitung apa pun, termasuk tabel data, hingga Anda memintanya. Bilah status menampilkan **Tidak mutakhir** setelah ada perubahan. Tekan **F9** (**Hitung Ulang Semua**) untuk menghitung semuanya, atau **Shift+F9** (**Hitung Ulang Lembar Ini**) untuk menghitung rumus pada lembar yang sedang Anda buka, beserta hal lain yang menunggu. Buku kerja yang diatur ke Manual dibuka dengan hasil yang tersimpan dalam berkasnya. **Hitung Sebelum Menyimpan**, yang juga berada di menu **Mode Perhitungan**, dicentang sejak awal: saat Anda menyimpan buku kerja Manual yang tidak mutakhir, rumus dihitung terlebih dahulu, sehingga berkas memuat angka yang mutakhir. Matikan opsi ini untuk menyimpan angka apa adanya.

Pilihan ini disimpan bersama buku kerja, dan merupakan satu langkah yang dapat Anda urungkan.

## Rumus yang membaca dirinya sendiri

Rumus yang membaca hasilnya sendiri, secara langsung atau melalui sel lain, tidak dapat dihitung dengan cara biasa. JeffCrunch memberi tahu Anda sekali, menyebutkan selnya, dan memberi garis tepi pada sel-sel tersebut di lembar; sel-sel itu tetap menyimpan nilai yang dimilikinya. Klik **Referensi Sirkular** pada tab **Rumus** untuk melihat daftarnya, lalu klik ganda sebuah sel di dalamnya untuk pergi ke sana.

Beberapa model memerlukan lingkaran seperti itu dengan sengaja. Pilih **Iterasi Rumus Sirkular** pada menu **Mode Perhitungan**, dan rumus akan dihitung berulang kali, masing-masing membaca nilai terbaru dari yang lain, hingga tidak ada nilai yang berubah lebih dari jumlah yang Anda tentukan atau jumlah putaran yang Anda izinkan habis.

## Melihat apa yang dibaca sebuah rumus

Pilih sebuah sel lalu klik **Tampilkan Sumber Sel Ini** untuk menggambar panah dari sel-sel yang dibaca rumusnya. Klik lagi untuk melangkah satu tingkat lebih jauh ke belakang. **Tampilkan Penggunaan Sel Ini** menggambar panah ke rumus yang membaca sel tersebut, dan setiap klik melangkah satu tingkat lebih jauh. Sebuah halaman kecil mewakili sel pada lembar lain. Paling banyak 2.000 panah digambar, dan bilah status memberi tahu jika ada lebih banyak. **Hapus Panah** menghapusnya, dan perubahan apa pun pada buku kerja juga menghapusnya.

Ketika sebuah sel menampilkan kesalahan seperti #DIV/0!, pilih sel tersebut lalu klik **Telusuri Kesalahan**. Panah digambar ke sel-sel yang dibacanya dan juga menampilkan kesalahan. Klik lagi untuk melangkah satu tingkat lebih jauh ke belakang. Jika tidak ada sel yang dibacanya menampilkan kesalahan, bilah status menyatakan bahwa sel ini yang menyebabkan kesalahan, dan itulah yang perlu diperbaiki.

Untuk menelusuri setiap kesalahan pada lembar, klik **Kesalahan Berikutnya** atau **Kesalahan Sebelumnya**. Sel yang menampilkan kesalahan dikunjungi baris demi baris, dan setelah yang terakhir, pencarian mulai lagi dari yang pertama. Setiap sel dipilih, dan panah digambar sampai ke sel yang menjadi penyebab kesalahan. Bilah status menyatakan kesalahan yang sedang Anda lihat dan berapa jumlahnya.

## Pemeriksaan Kesalahan

Pilih **Pemeriksaan Kesalahan** pada grup **Periksa** di tab Rumus untuk memeriksa lembar dari sel-sel yang mungkin keliru. Ada dua hal yang diperiksa: rumus yang menghasilkan kesalahan (seperti #DIV/0!), dan angka yang disimpan sebagai teks, yang mungkin dilewati oleh rumus.

Kotak dialog menampilkan satu sel setiap kali, berpindah ke sel tersebut pada lembar, dan menjelaskan dalam satu kalimat apa masalahnya. **Berikutnya** dan **Sebelumnya** berpindah di antara sel-sel tersebut. Segitiga hijau kecil di pojok kiri atas sel menandainya pada lembar. **Abaikan Kesalahan** menghapus sel dari daftar, menghilangkan segitiganya, dan disimpan dalam berkas. Satu langkah Urungkan akan mengembalikannya. Jika tidak ada yang salah, Anda akan diberi tahu.

## Evaluasi Rumus

Gunakan fitur ini untuk melihat bagaimana sebuah rumus mencapai nilainya. Pilih sel yang berisi rumus lalu pilih **Evaluasi Rumus**. Rumus ditampilkan dengan bagian berikutnya bergaris bawah. Tekan **Evaluasi** untuk menghitung bagian tersebut; nilainya menggantikan bagian itu. Bagian-bagian diproses dengan urutan: setiap sel tunggal dari kiri, lalu setiap fungsi beserta kurungnya, dimulai dari yang paling dalam, lalu sisanya. **Mulai Ulang** mengulang dari awal. Jika suatu bagian tidak dapat dihitung, kotak dialog menjelaskan alasannya dan membiarkan rumus seperti semula.

## Jendela Pantau

**Jendela Pantau** adalah jendela kecil yang tetap terlihat saat Anda bekerja. Jendela ini mencantumkan sel-sel yang ingin Anda awasi, dengan lembar, sel, nilai, dan rumusnya, bahkan ketika sel-sel tersebut berada di lembar lain. Pilih sel-sel lalu tekan **Tambah Pantauan**. Pilih baris dalam daftar lalu tekan **Hapus Pantauan**. Klik ganda sebuah baris untuk pergi ke sel. Nilainya berubah ketika lembar dihitung ulang. Daftar disimpan selama buku kerja terbuka.

## Rumus yang mengisi sel di sampingnya

Ketika sebuah rumus menghasilkan blok nilai, blok tersebut mengisi sel di bawah dan di samping sel tempat Anda mengetiknya. Ini disebut tumpahan. Coba =SEQUENCE(3) di A1: A1 menampilkan 1, dan A2 serta A3 menampilkan 2 dan 3. Sebuah rumus tumpah ketika ia memanggil fungsi yang menghasilkan blok (SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, TRANSPOSE, XLOOKUP, atau MMULT), menulis blok dalam kurung kurawal, memakai rentang tumpahan (di bawah), atau melakukan operasi aritmetika pada sebuah rentang, seperti =A1:A3*2.

- **Sel yang diisinya tidak memiliki rumus sendiri.** Sel-sel tersebut menampilkan nilai. Ketika Anda memilih salah satunya, bilah rumus menampilkan rumus dari sel yang mengisinya, dengan warna abu-abu. Menekan Enter pada sel itu apa adanya tidak mengubah apa pun. Untuk mengubah blok, ubah sel pertama itu.
- **Blok mengikuti hasil.** Jika rumus menghasilkan lebih banyak atau lebih sedikit nilai setelah sesuatu yang dibacanya berubah, blok akan membesar atau mengecil, dan rumus yang membaca sel-sel tersebut dihitung ulang.
- **Sel yang menghalangi menghasilkan #SPILL!.** Jika ada sel tempat blok akan ditempatkan yang berisi nilai atau rumus, sel pertama menampilkan **#SPILL!** dan tidak ada yang diisi. Kosongkan sel yang menghalangi dan blok akan langsung terisi. Mengetik di sel yang diisi oleh blok juga memberikan hasil yang sama: sel pertama menampilkan #SPILL!.
- **Blok memiliki batas ukuran.** Blok yang lebih besar dari lembar, atau yang berisi lebih dari 200.000 sel, menghasilkan #SPILL!.
- **A1# adalah seluruh blok.** Tulis sel yang berisi rumus dan tanda **#**, seperti =SUM(A1#), untuk menggunakan seluruh blok sebagaimana adanya saat ini, dari lembar ini atau lembar lain (=SUM(Sheet1!A1#)). Jika A1 tidak tumpah, A1# menghasilkan #REF!.

Tumpahan disimpan bersama berkas sebagai array dinamis, sehingga program asal berkas juga menampilkannya sebagai tumpahan. Menyimpan ke bentuk .xls atau .ods yang lebih lama mempertahankan nilainya, dan rumusnya sebagai rumus array di atas blok, di tempat bentuk lama memiliki fungsi yang dipakai. Periksa Kompatibilitas akan menyebutkan rumus-rumus ini.

## Memberi nama pada nilai di dalam rumus

**LET** memberi nama pada nilai di tengah rumus, sehingga Anda menulisnya sekali dan menggunakannya kembali. Pasangan nama dan nilai ditulis lebih dulu, dan perhitungannya di akhir. Dalam =LET(harga, B2, pajak, harga*0,08, harga+pajak), nama **harga** mewakili nilai di B2, **pajak** dihitung darinya, dan bagian terakhir menjumlahkan keduanya. Sebuah nama dapat memakai nama-nama sebelumnya, dan satu LET dapat memuat hingga 126 pasangan. Sebuah nama hanya berlaku di dalam LET-nya sendiri, dan LET di dalam LET lain boleh memakai nama yang sama lagi. Sebuah nama juga dapat mewakili blok sel, seperti =LET(baris, B2:B9, SUM(baris)/COUNT(baris)).

## Bertanya tentang sel atau buku kerja

**CELL** memberikan informasi tentang sebuah sel. Argumen pertamanya menentukan jenisnya: "address", "row", "col", "contents", "type", "format" (cara angka ditampilkan, seperti "F2" untuk dua desimal, "P0" untuk persen, atau "D1" untuk tanggal), "color" (1 jika angka negatif ditampilkan berwarna), "parentheses" (1 jika angka positif ditampilkan dalam kurung), "prefix" (tanda teks yang rata kiri, kanan, atau tengah), "protect" (1 jika sel dikunci), "width" (lebar kolom dalam karakter), dan "filename" (folder, berkas, dan lembar, seperti /home/me/[Anggaran.xlsx]Sheet1; kosong sampai buku kerja disimpan). **INFO** memberikan informasi tentang program dan buku kerja: "directory", "numfile" (berapa banyak lembar yang tampak), "origin", "osversion", "recalc" (Otomatis atau Manual), "release", dan "system".

## Mengambil nilai dari XML

**FILTERXML** menerima teks yang berisi XML dan sebuah XPath, yaitu cara singkat untuk menyatakan bagian mana yang Anda inginkan, lalu memberikan nilai yang ditemukan. =FILTERXML(A1, "//item/price") memberikan setiap harga dalam teks di A1, satu di bawah yang lain. Gunakan "/shop/item[2]/name" untuk item kedua, "//item[@id='b']/name" untuk item yang id-nya b, atau "count(//item)" untuk mengetahui jumlahnya. Jika teks bukan XML yang valid, atau jalurnya bukan jalur yang benar, hasilnya #VALUE!. Jika tidak ada yang cocok, hasilnya #N/A. XML yang memiliki tipe dokumen ditolak.
