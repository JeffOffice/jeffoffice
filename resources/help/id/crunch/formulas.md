# Cara rumus ditampilkan dan dihitung
<!-- keywords: tumpah, tumpahan, #SPILL!, larik dinamis, rentang tumpahan, A1#, kesalahan berikutnya, kesalahan sebelumnya, telusuri kesalahan, lacak kesalahan, hitung sebelum menyimpan, rumus, rumus-rumus, hitung, perhitungan, hitung ulang, fungsi, fungsi-fungsi, nilai, hasil, tersimpan, F9, sirkular, tampilkan rumus, bilah rumus, kesalahan, titik, menunjuk, klik sel, referensi, F4, pemeriksaan kesalahan, abaikan kesalahan, evaluasi rumus, langkah, jendela pantau, pantau, LET, beri nama nilai, fungsi sel, fungsi info, filterxml, xml, xpath, presisi sesuai tampilan, atur presisi sesuai tampilan, bulatkan, pembulatan, desimal, digit-->

Rumus adalah sel yang menghitung nilainya dari sel-sel lain, misalnya =SUM(B3:B8). Sel menampilkan nilainya, dan **bilah rumus** menampilkan rumusnya.

## Apa yang dilakukan JeffCrunch terhadap rumus

Saat Anda membuka buku kerja, berkas menyimpan nilai terakhir setiap rumus, sehingga lembar kerja dapat langsung ditampilkan. Kemudian JeffCrunch menghitung ulang setiap rumus, sedikit demi sedikit, agar jendela tetap cepat. Bilah status menampilkan **Menghitung** dengan persen selama proses ini berlangsung. Setelah selesai, sel-sel berisi hasil perhitungan JeffCrunch sendiri. Untuk menghitung ulang semua, klik **Hitung Ulang Semua** pada tab **Rumus**, atau tekan **F9**.

## Saat JeffCrunch tidak dapat menghitung rumus

JeffCrunch memiliki lebih dari 400 fungsi, dan jumlahnya terus bertambah. Rumus yang memakai fungsi yang belum dimiliki JeffCrunch tetap menggunakan nilai yang disimpan berkas untuk rumus tersebut. Demikian pula rumus yang merujuk ke buku kerja lain. Sel-sel yang bergantung pada rumus tersebut dihitung dari nilai yang tersimpan.

## Fungsi yang memerlukan beberapa catatan

- **Obligasi dan pinjaman.** PRICE, YIELD, DURATION, ODDFPRICE, dan fungsi obligasi lainnya menghitung hari sesuai **basis** yang Anda berikan (0 berarti 30 hari per bulan dalam setahun 360 hari, 1 berarti hari sebenarnya, dan seterusnya). YIELD, ODDFYIELD, ODDLYIELD, dan RATE menemukan hasilnya dengan mencoba nilai sampai harga atau pembayaran sesuai. Perhitungan berhenti saat hasilnya benar hingga sekitar 15 digit, sehingga digit terakhir dapat berbeda dari program lain.
- **Distribusi dan uji.** Fungsi yang berakhiran .DIST, .INV, .TEST, serta nama lama seperti NORMDIST dan TDIST, memberikan hasil yang sama. Nama lama tetap dipertahankan agar berkas lama tetap berfungsi. Fungsi *.INV juga dihitung dengan mencoba nilai.
- **Daftar yang mengisi beberapa sel.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE, dan LINEST menghasilkan blok nilai. Ketik salah satunya di satu sel dan blok itu akan tumpah ke sel di bawah dan di sampingnya (lihat **Rumus yang mengisi sel di sampingnya**), atau masukkan sebagai rumus larik (**Ctrl+Shift+Enter**) pada sel-sel yang harus diisi.
- **Teks dalam aksara lain.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB, dan REPLACEB menghitung setiap karakter sebagai satu, seperti pada komputer yang tidak diatur untuk teks dua-byte. PHONETIC memberikan teks sel itu sendiri, karena JeffCrunch tidak menyimpan cara baca teks Jepang.
- **CONVERT.** Satuan ditulis sebagai teks, dengan huruf besar dan kecil seperti yang tertera dalam daftar fungsi: "kg", "lbm", "mi", "F". Awalan metrik dapat diletakkan di depan satuan yang menerimanya, seperti "km" atau "mL".

## MOD: sisa bagi

**MOD(n, d)** memberikan sisa dari **n** setelah sebanyak mungkin bilangan bulat **d** dikurangkan: hasilnya n - d * INT(n / d). Contoh: **=MOD(10, 3)** menghasilkan 1, dan **=MOD(-10, 3)** menghasilkan 2, karena INT(-10 / 3) adalah -4 dan -10 - 3 * -4 adalah 2. Hasilnya memiliki tanda dari **d**. Jika **d** bernilai 0, hasilnya #DIV/0!.

## Tampilkan rumus

Klik **Tampilkan Rumus** pada tab **Rumus**, atau tekan **Ctrl+`** (di Mac, tombol Control, bukan tombol Command), untuk melihat rumus di setiap sel sebagai ganti nilainya. Kolom yang berisi rumus dibuat lebih lebar, hingga batas tertentu, agar rumus tampil utuh; lebarnya kembali normal saat Anda mematikan fitur ini, dan lebar kolom dalam berkas Anda tidak berubah.

## Mengubah rumus

Untuk mengubah rumus, pilih selnya, tekan **F2**, edit, dan tekan **Enter**. Anda juga dapat mengeditnya di bilah rumus. Saat Anda mengubah sebuah sel, rumus yang bergantung padanya dihitung ulang di latar belakang. Lihat [Ketik di sel](edit-cells).

## Menulis rumus

Ketik **=** dan huruf awal sebuah fungsi, lalu daftar singkat akan menampilkan fungsi dan [nama](names) yang diawali huruf tersebut, dengan satu kalimat keterangan untuk setiap item. Tekan **Tab** untuk memilih item yang ditandai. Gunakan panah **Bawah** dan **Atas** untuk menandai item lain, lalu **Tab** atau **Enter**. **Esc** menutup daftar. Setelah Anda berada di dalam tanda kurung fungsi, argumennya ditampilkan di bawah sel, dengan argumen yang sedang Anda ketik ditebalkan. Daftar dan argumen ini sama dengan yang ditampilkan **Pilih Fungsi**. Argumen dipisahkan dengan koma, atau dengan titik koma jika bahasa Anda menggunakan koma sebagai tanda desimal.

## Nama fungsi dalam bahasa Anda

Saat JeffCrunch ditampilkan dalam bahasa Jerman, Prancis, Spanyol, Italia, Belanda, Portugis, atau Rusia, Anda dapat mengetik rumus dengan nama fungsi dalam bahasa tersebut, misalnya `=SUMME(A1;A2)` dalam bahasa Jerman atau `=SOMME(A1;A2)` dalam bahasa Prancis. Nama bahasa Inggris juga berfungsi, sehingga rumus yang Anda salin dari web tidak perlu diubah. Bilah rumus, sel, daftar fungsi, dan **Pilih Fungsi** menampilkan nama dalam bahasa Anda.

Buku kerja tidak berubah saat Anda berganti bahasa. Berkas selalu menyimpan nama bahasa Inggris, sehingga buku kerja yang dibuat dalam satu bahasa dapat dibuka dan dibaca dengan benar dalam bahasa lain, dan di program lain mana pun. Nama lokal tersedia dalam bahasa Jerman, Prancis, Spanyol, Italia, Belanda, Portugis (Brasil dan Portugal), dan Rusia; nama bahasa Inggris selalu berfungsi di setiap bahasa. Program bahasa Ukraina dan Indonesia tetap menggunakan nama bahasa Inggris untuk hampir semua fungsi. Jika sebuah bahasa memiliki dua nama umum untuk satu fungsi, keduanya dikenali dan nama yang lebih umum yang ditampilkan. Fungsi yang tidak memiliki nama lazim dalam bahasa Anda tetap menggunakan nama bahasa Inggrisnya. TRUE dan FALSE juga mengikuti bahasa Anda. Nilai galat dalam bahasa Anda, seperti `#NV` dalam bahasa Jerman, dikenali saat Anda mengetik rumus dan ditampilkan di bilah rumus, tetapi sel yang berisi galat selalu menampilkan bentuk baku, seperti `#N/A`.

Jika pengaturan regional komputer Anda menggunakan koma sebagai tanda desimal, argumen fungsi dipisahkan dengan titik koma. Larik dalam kurung kurawal dengan lebih dari satu baris ditampilkan dengan titik koma di antara nilai dalam satu baris dan garis miring terbalik di antara baris, misalnya `{1;2\3;4}`, dan Anda dapat mengetiknya dengan cara tersebut. Larik yang diketik tanpa garis miring terbalik dibaca seperti cara berkas menulisnya: koma di antara nilai dalam satu baris, titik koma di antara baris, dan titik sebagai tanda desimal.

Untuk hanya mengetik nama bahasa Inggris, klik **Berkas > Opsi > Rumus** lalu centang **Gunakan nama fungsi bahasa Inggris**.

## Menunjuk sel

Alih-alih mengetik alamat sel, Anda dapat menunjuknya. Saat mengetik rumus, letakkan kursor di tempat sel dapat ditulis (tepat setelah **=**, operator seperti **+**, tanda kurung buka, atau koma), lalu klik sel. Alamatnya akan ditulis untuk Anda. Seret di atas sel untuk menulis blok seperti B2:D5. Klik lagi sebelum Anda mengetik hal lain, dan sel baru menggantikan sel yang baru saja ditulis. Tahan **Ctrl** lalu klik untuk menambahkan alamat lain setelah koma. Klik huruf kolom untuk menulis seluruh kolom seperti A:A, atau nomor baris untuk menulis seluruh baris seperti 3:3, dan seret di sepanjang huruf atau nomor untuk B:D atau 2:5.

Tombol panah juga dapat digunakan. Tekan panah dan penunjuk akan bergerak menjauh dari sel tempat Anda mengetik, dan alamatnya ditulis; tekan panah lagi untuk berpindah lebih jauh, atau tahan **Shift** untuk membentangkan penunjuk di atas sebuah blok. Bilah status menampilkan **Tunjuk** saat tombol panah sedang menunjuk, **Enter** saat Anda mengetik, dan **Edit** saat tombol panah menggerakkan kursor di dalam teks. Tekan **F2** untuk beralih antara menunjuk dan mengedit teks.

Untuk menunjuk sel pada lembar lain, klik tab lembar tersebut saat kursor berada di tempat sel dapat ditulis, lalu klik selnya. Nama lembar ikut ditulis, diapit tanda kutip bila diperlukan, seperti 'Lembar Saya'!B3. **Enter** membawa Anda kembali ke lembar tempat Anda memulai dan memasukkan rumus ke selnya. **Esc** membatalkan semuanya, termasuk perpindahan lembar.

Anda juga dapat menyeret tepi blok yang diberi garis tepi pada lembar untuk memindahkan alamat tersebut, atau menyeret kotak sudutnya untuk memperluasnya; rumus akan berubah saat Anda menyeret.

Tekan **F4** saat kursor berada di dalam atau tepat setelah sebuah alamat untuk mengunci bagiannya dengan tanda dolar: A1 menjadi $A$1, lalu A$1, lalu $A1, lalu kembali ke A1.

Setiap alamat dalam rumus diberi warna pada teksnya, dan sel yang dirujuknya diberi garis tepi di lembar dengan warna yang sama, sehingga Anda dapat melihat sekilas apa yang dibaca rumus tersebut. Klik setelah alamat lengkap, seperti =A1, mengakhiri entri dan memilih sel yang Anda klik, sama seperti untuk teks lainnya.

## Tabel data

Tabel data (tabel what-if) menunjukkan hasil sebuah rumus untuk banyak nilai dari satu atau dua sel. Saat Anda membuka buku kerja yang memilikinya, JeffCrunch menghitung selnya: untuk tabel dengan dua masukan, rumusnya berada di sel pojok kiri atas tabel, nilai yang akan dicoba secara mendatar dimasukkan ke sel masukan baris, dan nilai secara menurun dimasukkan ke sel masukan kolom. Untuk tabel dengan satu masukan, nilai ditempatkan menurun di kolom sebelah kiri (dan rumusnya berada di baris di atasnya) atau mendatar di baris di atasnya (dan rumusnya berada di kolom sebelah kirinya). Setiap sel tabel adalah hasil rumus, dan setiap rumus yang bergantung pada sel masukan, dengan masukan diatur ke nilai tersebut; tidak ada yang lain dalam buku kerja yang diubah. Dalam mode **Otomatis**, tabel dihitung bersama rumus, dan rumus yang membaca sel-sel tabel mengikutinya. Membuat tabel data baru belum tersedia. Tabel yang tidak dapat dihitung (rumus yang bergantung pada masukan memakai fungsi yang belum dimiliki JeffCrunch, atau berubah dengan sendirinya, seperti RAND atau NOW) tetap menggunakan nilai yang tersimpan dalam berkasnya.

## Menghitung secara manual

Klik **Mode Perhitungan** pada tab **Rumus** untuk memilih cara buku kerja menghitung rumusnya:

- **Otomatis** menghitungnya saat Anda mengubah sel.
- **Otomatis, Tabel Secara Manual** melakukan hal yang sama, tetapi membiarkan sel tabel data tetap seperti semula hingga Anda menekan **F9**.
- **Manual** tidak menghitung apa pun, termasuk tabel data, sampai Anda memintanya. Bilah status menampilkan **Belum diperbarui** setelah ada perubahan. Tekan **F9** (**Hitung Ulang Semua**) untuk menghitung semuanya, atau **Shift+F9** (**Hitung Ulang Lembar Ini**) untuk menghitung rumus pada lembar yang sedang Anda buka, beserta apa pun yang menunggu. Buku kerja yang diatur ke Manual dibuka dengan hasil yang tersimpan dalam berkasnya. **Hitung Sebelum Menyimpan**, juga pada menu **Mode Perhitungan**, dicentang sejak awal: saat Anda menyimpan buku kerja Manual yang belum diperbarui, rumus dihitung terlebih dahulu sehingga berkas berisi angka terbaru. Matikan opsi ini untuk menyimpan angka apa adanya.

Pilihan ini disimpan bersama buku kerja, dan merupakan satu langkah yang dapat diurungkan.

## Rumus yang membaca dirinya sendiri

Rumus yang membaca hasilnya sendiri, secara langsung atau melalui sel lain, tidak dapat dihitung dengan cara biasa. JeffCrunch memberi tahu Anda sekali, menyebutkan sel-selnya, dan memberi garis tepi pada sel tersebut di lembar; sel itu tetap menyimpan nilai yang dimilikinya. Klik **Referensi Sirkular** pada tab **Rumus** untuk melihat daftarnya, dan klik ganda sel dalam daftar untuk menuju ke sana.

Beberapa model memang memerlukan lingkaran seperti itu. Pilih **Iterasi Rumus Sirkular** pada menu **Mode Perhitungan**, dan rumus akan dihitung berulang kali, dengan setiap rumus membaca nilai terbaru dari yang lain, hingga tidak ada nilai yang berubah lebih dari jumlah yang Anda tentukan atau jumlah putaran yang Anda izinkan sudah terpakai.

## Pertahankan angka sebagaimana tampilan sel

Sebuah sel dapat menampilkan lebih sedikit digit daripada yang disimpannya. Jika sebuah sel menyimpan 2,675 dan menampilkan 2,68, rumus yang membacanya tetap menggunakan 2,675. Terkadang Anda ingin rumus menggunakan apa yang Anda lihat. Dalam kasus itu, pilih **Atur Presisi sesuai Tampilan** pada menu **Opsi Perhitungan** di tab **Rumus**.

JeffCrunch memberi peringatan terlebih dahulu, karena digit yang tersembunyi akan hilang. Pilih **Batal** dan tidak ada yang berubah. Pilih **OK** dan setiap angka dalam buku kerja dibulatkan sesuai yang ditampilkan selnya. Sel yang menampilkan dua desimal tetap memiliki dua desimal. Sel yang menampilkan persen tetap berupa persen. Sel berisi tanggal tetap menyimpan harinya, dan sel berisi waktu mempertahankan bagian waktu terkecil yang ditampilkan, seperti menit. Sel yang menampilkan **Umum** mempertahankan semua digitnya. Buku kerja yang besar menampilkan catatan dengan tombol **Batal** selama proses ini berlangsung. Klik Batal saat catatan masih tampil, dan tidak ada yang diubah.

Setelah itu, setiap angka yang Anda ketik atau tempel, dan setiap hasil rumus, dibulatkan dengan cara yang sama. Rumus itu sendiri tidak berubah. Pilihan ini disimpan bersama buku kerja.

Pembulatan adalah satu langkah yang dapat diurungkan, dan **Urungkan** mengembalikan angka-angkanya. Memilih **Atur Presisi sesuai Tampilan** lagi akan mematikannya. Itu hanya menghentikan pembulatan mulai saat itu. Angka yang sudah dibulatkan tetap dibulatkan.

## Melihat apa yang dibaca sebuah rumus

Pilih sel lalu klik **Tampilkan Sumber Sel Ini** untuk menggambar panah dari sel-sel yang dibaca rumusnya. Klik lagi untuk melangkah satu tingkat lebih jauh ke belakang. **Tampilkan Penggunaan Sel Ini** menggambar panah ke rumus-rumus yang membaca sel tersebut, dan setiap klik melangkah satu tingkat lagi. Sebuah halaman kecil mewakili sel pada lembar lain. Paling banyak 2.000 panah digambar, dan bilah status memberi tahu jika ada lebih banyak. **Hapus Panah** menghapusnya, dan perubahan apa pun pada buku kerja juga menghapusnya.

Saat sebuah sel menampilkan galat seperti #DIV/0!, pilih sel tersebut lalu klik **Telusuri Kesalahan**. Panah akan digambar ke sel-sel yang dibacanya dan juga menampilkan galat. Klik lagi untuk melangkah satu tingkat lebih jauh ke belakang. Jika tidak ada sel yang dibacanya menampilkan galat, bilah status menyatakan bahwa sel ini yang menyebabkan galat, dan itulah sel yang perlu diperbaiki.

Untuk memeriksa setiap galat pada lembar, klik **Kesalahan Berikutnya** atau **Kesalahan Sebelumnya**. Sel yang menampilkan galat dikunjungi baris demi baris, dan setelah yang terakhir, pencarian dimulai lagi dari yang pertama. Setiap sel dipilih, dan panah digambar sampai ke sel yang menjadi penyebab galat. Bilah status menyatakan galat yang sedang Anda lihat, dan berapa jumlah galat seluruhnya.

## Pemeriksaan Kesalahan

Pilih **Pemeriksaan Kesalahan** pada grup **Periksa** di tab Rumus untuk mencari sel yang mungkin salah pada lembar. Fitur ini mencari dua hal: rumus yang menghasilkan galat (seperti #DIV/0!), dan angka yang disimpan sebagai teks, yang mungkin dilewati rumus.

Kotak dialog menampilkan satu sel setiap kali, membawa Anda ke sel tersebut di lembar, dan menjelaskan dengan satu kalimat apa yang bermasalah. **Berikutnya** dan **Sebelumnya** berpindah di antara sel. Segitiga hijau kecil di sudut kiri atas sel menandainya di lembar. **Abaikan Kesalahan** menghapus sel dari daftar, menghilangkan segitiganya, dan disimpan dalam berkas. Satu langkah Urungkan akan mengembalikannya. Jika tidak ada yang salah, Anda akan diberi tahu.

## Evaluasi Rumus

Gunakan fitur ini untuk melihat bagaimana sebuah rumus menghasilkan nilainya. Pilih sel yang berisi rumus lalu pilih **Evaluasi Rumus**. Rumus ditampilkan dengan bagian berikutnya bergaris bawah. Tekan **Evaluasi** untuk menghitung bagian tersebut; nilainya menggantikan bagian itu. Bagian-bagian dihitung dengan urutan: setiap sel tunggal dari kiri, lalu setiap fungsi beserta tanda kurungnya, dimulai dari yang paling dalam, kemudian sisanya. **Mulai Ulang** memulai dari awal. Jika suatu bagian tidak dapat dihitung, kotak dialog menjelaskan alasannya dan membiarkan rumus seperti semula.

## Jendela Pantau

**Jendela Pantau** adalah jendela kecil yang tetap terlihat saat Anda bekerja. Jendela ini mencantumkan sel yang ingin Anda awasi, dengan lembar, sel, nilai, dan rumusnya, bahkan jika sel tersebut berada di lembar lain. Pilih sel lalu tekan **Tambah Pantau**. Pilih baris dalam daftar lalu tekan **Hapus Pantau**. Klik ganda sebuah baris untuk menuju ke sel tersebut. Nilai akan berubah saat lembar dihitung ulang. Daftar disimpan selama buku kerja terbuka.

## Rumus yang mengisi sel di sampingnya

Saat sebuah rumus menghasilkan blok nilai, blok itu mengisi sel di bawah dan di samping sel tempat Anda mengetiknya. Ini disebut tumpahan. Coba =SEQUENCE(3) di A1: A1 menampilkan 1, dan A2 serta A3 menampilkan 2 dan 3. Sebuah rumus tumpah saat ia memanggil fungsi yang menghasilkan blok (SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, TRANSPOSE, XLOOKUP, atau MMULT), menulis blok dalam kurung kurawal, memakai rentang tumpahan (lihat di bawah), atau melakukan operasi aritmetika pada sebuah rentang, seperti =A1:A3*2.

- **Sel-sel yang diisinya tidak memiliki rumus sendiri.** Sel-sel itu hanya menampilkan nilai. Saat Anda memilih salah satunya, bilah rumus menampilkan rumus dari sel pertama yang mengisinya, dengan warna abu-abu. Menekan Enter pada sel itu tanpa perubahan tidak mengubah apa pun. Untuk mengubah blok, ubah sel pertama itu.
- **Blok mengikuti hasilnya.** Jika rumus menghasilkan lebih banyak atau lebih sedikit nilai setelah sesuatu yang dibacanya berubah, blok akan membesar atau menyusut, dan rumus yang membaca sel-sel tersebut dihitung ulang.
- **Sel yang menghalangi menyebabkan #SPILL!.** Jika ada sel tempat blok akan dibuat yang sudah berisi nilai atau rumus, sel pertama menampilkan **#SPILL!** dan tidak ada yang diisi. Kosongkan sel yang menghalangi, dan blok akan langsung terisi. Mengetik di sel yang diisi blok juga menyebabkan hal yang sama: sel pertama menampilkan #SPILL!.
- **Blok memiliki batas ukuran.** Blok yang lebih besar dari lembar, atau lebih dari 200.000 sel, menghasilkan #SPILL!.
- **A1# berarti seluruh blok.** Tulis sel yang berisi rumus dan tanda **#**, seperti =SUM(A1#), untuk memakai seluruh blok sebagaimana adanya saat ini, dari lembar ini atau lembar lain (=SUM(Sheet1!A1#)). Jika A1 tidak tumpah, A1# menghasilkan #REF!.

Tumpahan disimpan bersama berkas sebagai larik dinamis, sehingga program yang membuka berkas tersebut juga menampilkannya sebagai tumpahan. Menyimpan ke bentuk .xls atau .ods yang lebih lama mempertahankan nilainya, serta rumusnya sebagai rumus larik pada blok tempat bentuk lama memiliki fungsi yang dipakai. Periksa Kompatibilitas akan menyebutkan rumus-rumus ini.

## Beri nama pada nilai di dalam rumus

**LET** memberi nama pada nilai di tengah rumus, sehingga Anda cukup menuliskannya sekali dan dapat memakainya kembali. Pasangan nama dan nilai ditulis lebih dahulu, dan perhitungannya di akhir. Dalam =LET(harga, B2, pajak, harga*0,08, harga+pajak), nama **harga** mewakili nilai di B2, **pajak** dihitung darinya, dan bagian terakhir menjumlahkan keduanya. Sebuah nama dapat memakai nama-nama yang ada sebelumnya, dan satu LET dapat memuat hingga 126 pasangan. Sebuah nama hanya berfungsi di dalam LET-nya sendiri, dan LET di dalam LET lain boleh memakai nama yang sama lagi. Sebuah nama juga dapat mewakili blok sel, seperti =LET(baris, B2:B9, SUM(baris)/COUNT(baris)).

## Tanyakan tentang sel atau buku kerja

**CELL** memberikan informasi tentang sebuah sel. Argumen pertamanya menentukan jenis informasi: "address", "row", "col", "contents", "type", "format" (cara angka ditampilkan, seperti "F2" untuk dua desimal, "P0" untuk persen, atau "D1" untuk tanggal), "color" (1 jika angka negatif ditampilkan berwarna), "parentheses" (1 jika angka positif ditampilkan dalam tanda kurung), "prefix" (tanda teks yang rata kiri, kanan, atau tengah), "protect" (1 jika sel dikunci), "width" (lebar kolom dalam karakter), dan "filename" (folder, berkas, dan lembar, seperti /home/saya/[Anggaran.xlsx]Sheet1; kosong sampai buku kerja disimpan). **INFO** memberikan informasi tentang program dan buku kerja: "directory", "numfile" (berapa banyak lembar yang tampak), "origin", "osversion", "recalc" (Otomatis atau Manual), "release", dan "system".

## Mengambil nilai dari XML

**FILTERXML** menerima teks yang berisi XML dan sebuah XPath, yaitu cara singkat untuk menyatakan bagian mana yang Anda inginkan, lalu memberikan nilai yang ditemukannya. =FILTERXML(A1, "//item/price") memberikan setiap harga dalam teks di A1, satu di bawah yang lain. Gunakan "/shop/item[2]/name" untuk item kedua, "//item[@id='b']/name" untuk item yang id-nya b, atau "count(//item)" untuk mengetahui jumlahnya. Jika teks bukan XML yang valid, atau jalurnya tidak valid, hasilnya #VALUE!. Jika tidak ada yang cocok, hasilnya #N/A. XML yang memiliki tipe dokumen ditolak.
