# Alat data

<!-- keywords: ramalan, lembar ramalan, prakiraan, nilai masa depan, interval kepercayaan, musiman, tren, pemulusan, formulir, form data, entri data, rekaman, rekaman baru, temukan berikutnya, temukan sebelumnya, kriteria, pengelola skenario, skenario, what-if, jika-maka, ringkasan skenario, data, tab data, teks ke kolom, pisahkan, pemisah, koma, tab, lebar tetap, hapus duplikat, goal seek, cari nilai target, subtotal, total, filter lanjutan, kondisi, validasi data, lingkari data tidak valid, lingkaran merah, hapus lingkaran validasi, konsolidasi, gabungkan rentang, izinkan, daftar, daftar tarik-turun, pesan masukan, peringatan galat, baris berulang, unik, berkas teks, csv, dari teks, kolom, pembatas, tanda kutip, edit tautan, perbarui nilai, ubah sumber, tautan, eksternal, sumber, putus tautan, periksa status, buka sumber -->

Tab **Data** berisi alat yang bekerja pada daftar nilai: mengambil teks dari berkas, mengurutkan daftar, memecah teks menjadi kolom, dan menghapus baris yang berulang.

## Dari Teks/CSV

**Dari Teks/CSV** membuka berkas teks (.csv, .tsv, atau .txt). Perintah ini sama dengan **Buka Berkas Teks** pada tab Berkas. JeffCrunch menanyakan cara berkas dibagi menjadi kolom dan isi setiap kolom, lalu menampilkannya sebagai buku kerja baru. Jendela yang sama juga muncul saat Anda membuka berkas teks atau menggunakan **Tempel Impor Teks**.

Jendela menebak isi setiap kolom dari 200 baris pertama berkas, dan menampilkan tebakan itu di kotak di atas kolom. Ubah kolom mana pun sebelum Anda membuka berkas. Jenis yang Anda ubah tidak akan diganti oleh tebakan berikutnya, meskipun Anda kemudian mengubah pemisahnya. Tebakan hanya melakukan tiga hal pada kolom: membiarkan kolom **Seperti yang diketik**, yaitu cara berkas dibaca jika tidak ada yang ditebak; membuat kolom **Teks** untuk melindunginya; atau membuat tanggal dalam kolom menurut satu urutan yang dibuktikan oleh nilainya. Aturannya sebagai berikut:

- Kolom tetap **Seperti yang diketik** jika Seperti yang diketik sudah membaca nilainya dengan benar: angka, benar dan salah, waktu, tanggal yang ditulis dengan nama bulan, angka bertanda seperti -5 dan +3, kata, dan nomor versi seperti 1.13.2. Mengubahnya hanya akan memperburuk keadaan.
- Kolom adalah **tanggal** hanya jika setiap nilainya memiliki tahun empat digit (atau berupa tanggal lengkap seperti 2024-05-13) dan terbaca sebagai tanggal dalam satu urutan saja (bulan, hari, tahun; hari, bulan, tahun; atau tahun, bulan, hari). Tanggalnya ditampilkan seperti tanggal yang Anda ketik. Jika lebih dari satu urutan cocok dengan setiap nilai, seperti 05/06/2024, kolom tetap **Teks**, jendela mencantumkannya, dan Anda memilih urutannya dari kotak kolom tersebut.
- Nilai dengan tiga angka dan tanpa tahun empat digit, seperti 1.13.2 atau 13.05.24, tidak pernah dijadikan tanggal oleh tebakan. Jika Seperti yang diketik akan membiarkannya sebagai teks, kolom tetap Seperti yang diketik; jika Seperti yang diketik akan mengubahnya menjadi tanggal, kolom menjadi **Teks**. Dalam kedua kasus, jendela mencantumkan kolom yang bisa berupa tanggal dengan tahun dua digit, sehingga Anda bisa memilih urutannya.
- Kolom adalah **Teks** jika sebuah nilai memiliki angka nol di depan digit lain atau memiliki lebih dari 15 digit, jika tanda plus muncul sebelum tujuh digit atau lebih (nomor telepon), atau jika notasi eksponen seperti 1E5 berada di antara nilai lain.
- Nilai yang diawali tanda sama dengan diserahkan ke kotak **Baca rumus**. Jika kotak dicentang, kolom tetap Seperti yang diketik agar nilai dibaca sebagai rumus. Jika tidak dicentang, kolom menjadi **Teks**.
- Centang **Baris pertama berisi nama kolom**, dan baris pertama tidak dipakai untuk tebakan. Jendela mencentangnya jika baris pertama berupa teks di atas angka atau tanggal.
- Kolom setelah 100 kolom pertama dimasukkan sebagai teks.

Hanya 200 baris pertama yang diperiksa, sehingga nilai yang lebih ke bawah bisa jadi tidak sesuai dengan tebakan. Pada kolom tanggal, nilai yang lebih ke bawah yang bukan tanggal dalam urutan yang dipilih akan dimasukkan sebagai teks, tidak pernah sebagai tanggal yang salah. Pada kolom Teks, setiap nilai dimasukkan persis seperti yang tertulis. Pada kolom Seperti yang diketik, nilai yang lebih ke bawah dibaca seperti cara Seperti yang diketik membacanya di tempat lain.

**Tempel Impor Teks** memasukkan teks seolah-olah Anda mengetiknya, dengan tanda desimal dan tanda ribuan Anda sendiri. Karena itu jendelanya tidak memiliki kotak untuk tanda-tanda tersebut, dan tebakan membaca teks dengan cara yang sama seperti saat ditempel. Jendela untuk berkas memilikinya.

## Dari Berkas

**Dari Berkas** pada grup **Dapatkan Data** memuat baris dari berkas JSON, berkas XML, atau buku kerja lain sebagai tabel. Hanya berkas di komputer ini yang dapat dimuat. Alamat web atau berkas di komputer lain ditolak.

1. Klik **Dari Berkas**, lalu **Telusuri**, dan pilih berkas. Baris pertama langsung ditampilkan, dengan jenis yang ditemukan JeffCrunch untuk setiap kolom dari baris-baris awal itu: angka, tanggal, benar atau salah, atau teks. Seluruh berkas diperiksa saat dimuat, sehingga sebuah kolom bisa berakhir sebagai teks. Tidak ada yang berubah di lembar sampai Anda mengklik **Muat**.
2. Jika berkas berisi lebih dari satu daftar, pilih daftar yang akan dimuat. Dalam berkas JSON, daftar berupa item dengan nilai bernama, seperti `$.items`. Dalam berkas XML, berupa item yang berulang, seperti `/feed/entry`. Dalam buku kerja, berupa lembar, tabel, atau rentang bernama.
3. Item JSON di dalam item menjadi kolom bernama `luar.dalam`, sampai empat tingkat. Daftar nilai sederhana digabungkan menjadi satu sel (10.000 nilai pertama), atau dibagi ke beberapa kolom (16 pertama), sesuai pilihan Anda. Dalam XML, atribut dan item anak menjadi kolom. Dua nilai yang berbeda tidak pernah berbagi kolom: jika dua nama akan sama, nama kedua diberi angka tambahan atau, dalam XML, awalan namanya, dan catatan menyebutkannya. Nama yang berulang dalam satu item JSON menyimpan nilai terakhirnya, dan catatan menyebutkannya. Angka yang terlalu besar atau terlalu kecil untuk sel disimpan sebagai teks, seperti tertulis, dan karakter yang tidak bisa dibaca ditampilkan sebagai tanda pengganti; keduanya dicatat.
4. Centang **Muat setiap kolom sebagai teks** untuk menyimpan setiap nilai seperti tertulisnya. Kolom angka dengan angka nol di depan, seperti 007, selalu disimpan sebagai teks.
5. Ketik sel tempat tabel akan diletakkan, lalu klik **Muat**. Jika tabel akan menutupi sel yang berisi sesuatu, JeffCrunch bertanya sebelum menggantinya. Jika tidak ada ruang, atau lembar dilindungi, Anda diberi tahu dan tidak ada yang berubah.

Tabel adalah satu langkah **Urungkan**. Tabel mengingat berkas dan pilihan Anda. Untuk membaca berkas lagi, klik sel di dalam tabel, lalu klik **Segarkan** pada tab Data. **Segarkan Semua** adalah tombol besar di sebelahnya: ia membaca setiap tabel yang dibuat dengan Dapatkan Data, lalu menyegarkan setiap tabel pivot, semuanya sebagai satu langkah Urungkan. Tabel bertambah atau berkurang mengikuti baris berkasnya. Kolom yang Anda tambahkan di sebelah tabel tetap ada: rumus yang Anda salin ke bawah akan diisi ke baris baru, dan baris yang sudah tidak ada di berkas juga dihapus darinya. Jika berkas hilang atau tidak bisa dibaca, Anda diberi tahu dan tidak ada yang berubah. Jika kolom-kolom berkas berubah, Anda diberi tahu kolom mana yang hilang atau baru, dan tidak ada yang berubah. Buat tabel lagi dengan Dapatkan Data, atau kembalikan kolomnya ke dalam berkas.

Tautan ke berkas disimpan dalam tabel, di dalam buku kerja `.xlsx` yang tersimpan, pada bagian yang diabaikan program lain. Tautan memuat jalur lengkap berkas di komputer ini, sehingga siapa pun yang menerima buku kerja dapat membaca jalur tersebut. Untuk menghapusnya sebelum Anda membagikan buku kerja, gunakan **Periksa**, lalu **Tautan ke berkas data**, lalu **Hapus**. Salinan yang disimpan sebagai jenis berkas lain tidak menyimpannya, dan **Periksa Kompatibilitas** akan memberi tahu hal itu. **Periksa** mencantumkan tautan di bawah **Tautan ke berkas data** dan dapat menghapusnya. Tabel dan selnya tetap ada.

Berkas dibaca dengan batas: berkas hingga 64 MB, 200.000 baris, 256 kolom, dan 32.767 karakter dalam satu sel. XML yang memuat deklarasi tipe dokumen atau entitas ditolak. Apa pun yang terpotong oleh batas akan diberitahukan.

## Edit Tautan

Jika buku kerja memiliki rumus yang membaca buku kerja lain (seperti ='[Harga.xlsx]Sheet1'!A1), **Edit Tautan** pada grup **Dapatkan Data** mencantumkan buku kerja lain tersebut, yang disebut sumber. Setiap baris menunjukkan lokasi sumber dan statusnya. Sampai buku kerja dibuka kembali bersama sumbernya, buku kerja menampilkan angka yang disimpan berkasnya dari sumber tersebut.

- **Periksa Status** mencari setiap sumber di komputer ini dan menyatakan **Ditemukan**, atau menyatakan bahwa sumber tidak ditemukan. Sumber di web tidak diambil, sehingga statusnya menyatakan belum diperiksa.
- **Perbarui Nilai** membaca sumber yang dipilih dari komputer ini dan memberi setiap sel yang hanya merujuk ke salah satu selnya nilai yang dimiliki sel itu sekarang. Sel yang melakukan lebih dari sekadar menyalin nilai, seperti `=[1]Harga!A1*2`, tetap memakai angka yang disimpan berkasnya, dan pesan di bawah daftar menyebutkan berapa sel yang diperbarui dan berapa yang tidak. Perbarui Nilai adalah satu langkah, sehingga **Urungkan** mengembalikan nilai lama. Nilai baru juga ditulis ke bagian tautan di dalam berkas saat Anda menyimpan, sehingga buku kerja menampilkannya saat dibuka lagi, meskipun sumbernya tidak ada.
- **Ubah Sumber** memungkinkan Anda memilih berkas lain untuk tautan yang dipilih, misalnya saat buku kerja dipindahkan. Rumusnya tetap seperti semula. Lokasi baru ditulis ke berkas saat Anda menyimpan, dan **Urungkan** membatalkan perubahan itu.
- **Buka Sumber** membuka sumber yang dipilih di jendela baru, jika sumbernya dapat ditemukan.
- **Putus Tautan** mengubah setiap sel yang membaca sumber yang dipilih menjadi nilai yang ditampilkannya sekarang, dan nama yang mewakili lokasi di dalamnya menjadi **#REF!**. JeffCrunch bertanya terlebih dahulu. Putus Tautan adalah satu langkah, sehingga **Urungkan** mengembalikan rumus, nama, dan tautan, sampai Anda menyimpan buku kerja. Jika tidak ada tautan yang tersisa, berkas disimpan tanpa bagian yang memuatnya.

## Formulir

**Formulir** (pada grup **Alat Data**, dan di kotak pencarian) menampilkan satu baris daftar setiap kali, sebagai formulir kecil. Daftar adalah tabel, atau blok sel dengan baris nama di atas dan baris data di bawah setiap nama. Klik sel mana pun di dalam daftar terlebih dahulu.

- Setiap kotak adalah satu kolom, diberi label dengan nama di baris teratas. Ketik di dalam kotak untuk mengubah nilai. Kotak yang redup berisi rumus: formulir menampilkan hasilnya, dan Anda mengubah rumusnya di lembar.
- **Temukan Berikutnya** dan **Temukan Sebelumnya** pindah ke baris berikutnya dan sebelumnya. Berpindah ke baris lain, atau menekan **Enter**, menulis apa yang Anda ketik di baris ini.
- **Baru** menampilkan formulir kosong. Isi, lalu tekan **Baru** lagi, **Temukan Berikutnya**, atau **Tutup**. Baris ditambahkan di bawah baris terakhir daftar. Tabel bertambah satu baris. Jika sel di bawah daftar tidak kosong, formulir memberi tahu Anda dan tidak menambahkan apa pun.
- **Hapus** menghapus baris yang sedang Anda lihat, setelah bertanya terlebih dahulu. Baris di bawahnya naik ke atas. Tabel selalu menyimpan setidaknya satu baris data. Baris yang disembunyikan, atau yang disembunyikan filter, tidak ditampilkan di formulir, tidak dihitung, dan tidak pernah dihapus olehnya. Jika daftar terlalu panjang untuk ditelusuri seluruhnya, formulir memberi tahu bahwa hanya baris pertama yang ditampilkan.
- **Pulihkan** mengembalikan apa yang Anda ketik di baris ini, sebelum Anda meninggalkannya.
- **Kriteria** memungkinkan Anda mencari baris. Ketik kondisi di satu atau beberapa kotak, lalu tekan **Temukan Berikutnya** atau **Temukan Sebelumnya**. Kondisi berupa nilai, seperti **Ani** (teks cocok dengan awal kata, dan **?** serta ***** mewakili huruf), atau nilai setelah tanda: **>100**, **<=5**, **<>0**, **=Oslo**. Sebuah baris harus memenuhi setiap kondisi yang Anda ketik. Tekan **Formulir** untuk kembali, atau **Hapus** untuk memulai lagi. Selama kriteria diatur, **Temukan Berikutnya** dan **Temukan Sebelumnya** melewati baris yang tidak memenuhinya.
- **Tutup** menulis baris dan menutup formulir.

Setiap baris yang Anda ubah, tambahkan, atau hapus adalah satu langkah, sehingga **Urungkan** di lembar membatalkannya sekaligus.

## Urutkan dan filter

Grup **Urutkan & Filter** memiliki perintah yang sama dengan menu **Urutkan dan Pilih Baris** pada tab Beranda. Lihat [Urutkan dan filter](sort-filter).

## Teks ke Kolom

Gunakan ini saat satu kolom berisi beberapa potongan teks yang ingin Anda pisahkan ke kolom terpisah, seperti "Smith, Ann, 31".

1. Pilih sel-sel dari **satu** kolom.
2. Pilih **Teks ke Kolom** pada tab **Data**.
3. Pilih cara memecah teks:
   - **Pisahkan pada pemisah**: centang tanda yang berada di antara potongan: **Tab**, **Titik koma**, **Koma**, **Spasi**, atau **Lainnya**, tempat Anda mengetik tanda sendiri. **Perlakukan pemisah berurutan sebagai satu** mencegah dua koma berurutan membuat potongan kosong. **Teks dalam tanda kutip** menentukan tanda yang membungkus potongan yang tidak boleh dipecah, seperti "Lee, Bo". Di dalam tanda kutip, dua tanda berurutan berarti satu tanda.
   - **Pisahkan pada posisi tetap**: ketik posisi tempat kolom baru dimulai, dengan menghitung karakter dari 1. Misalnya, 5, 12 memulai kolom baru pada karakter kelima dan kedua belas.
4. Pilih jenis setiap potongan: **Umum** membaca angka dan tanggal seperti yang Anda ketik, dan **Teks** menyimpan setiap potongan apa adanya, sehingga "007" tetap "007".
   Untuk memperlakukan satu potongan secara berbeda, ketik nomornya di **Potongan** dan pilih cara membacanya: **Sama seperti di atas**, **Seperti yang diketik**, **Teks**, tanggal yang ditulis sebagai **bulan, hari, tahun**, **hari, bulan, tahun**, atau **tahun, bulan, hari**, atau **Jangan sertakan**, yang tidak meletakkan potongan itu di mana pun. Potongan pertama bernomor 1. Potongan tanggal boleh memiliki bulan berupa nama (5 Mar 2024, 5 Maret 2024) dalam bahasa Inggris atau bahasa Anda, boleh dipisahkan oleh /, -, titik, koma, atau spasi, dan boleh diikuti waktu (10:30, 10:30:45, atau 10:30 PM). Potongan yang bukan tanggal dalam urutan yang Anda pilih (bulan 13, hari yang tidak ada, tahun yang bukan dua atau empat digit, atau waktu yang tidak masuk akal) disimpan sebagai teks, sebagaimana diketik; potongan itu tidak pernah dibaca dengan urutan lain.
5. Pada **Letakkan potongan dari**, ketik sel pertama untuk potongan-potongan tersebut. Awalnya adalah sel pertama yang Anda pilih.
6. Kotak di bawah pilihan menampilkan baris-baris pertama yang sudah dipecah sesuai pilihan Anda. Tekan **OK**.

Potongan-potongan diletakkan di sel sebelah kanan sel pertama, dan menggantikan isi sel-sel tersebut. Teks ke Kolom adalah satu langkah yang dapat Anda urungkan.

## Hapus Duplikat

Gunakan ini untuk hanya menyimpan baris pertama dari setiap kelompok baris yang berulang.

1. Pilih sel-sel daftar, atau satu sel di dalamnya. JeffCrunch mengambil seluruh blok data di sekitar satu sel.
2. Pilih **Hapus Duplikat**.
3. Centang **Data saya memiliki header** jika baris pertama berisi nama. Baris itu tetap, dan tidak dibandingkan.
4. Centang kolom yang harus sama agar dua baris dihitung sebagai berulang. Dua baris berulang hanya jika setiap kolom yang dicentang sama. Huruf besar dan kecil dihitung sama, dan rumus dihitung sebagai nilai yang dihasilkannya.
5. Tekan **OK**. Kotak dialog memberi tahu berapa baris yang dihapus dan berapa yang tersisa.

Baris di bawah baris yang dihapus naik ke atas di dalam sel yang dipilih, beserta format dan rumusnya. Sel di bawah pilihan tidak bergerak. Baris kosong yang tersisa di bawah pilihan dikosongkan. Hapus Duplikat adalah satu langkah yang dapat Anda urungkan.

## Cari Nilai Target

Gunakan ini saat Anda tahu jawaban yang diinginkan tetapi tidak tahu angka yang menghasilkannya. Misalnya, sel pembayaran berisi rumus yang memakai suku bunga, dan Anda ingin pembayarannya 500. Cari Nilai Target mencoba angka di sel suku bunga sampai pembayarannya 500.

1. Pilih **Cari Nilai Target** pada tab **Data**.
2. Pada **Sel dengan rumus**, ketik sel yang berisi rumus, seperti B3. Awalnya adalah sel yang Anda pilih.
3. Pada **Nilai yang diinginkan**, ketik jawaban yang Anda inginkan dari rumus.
4. Pada **Sel yang diubah**, ketik sel yang boleh diubah Cari Nilai Target, seperti B1. Sel itu harus berisi angka, bukan rumus.
5. Tekan **OK**.

Jika Cari Nilai Target menemukan angka, angka itu tetap berada di sel yang diubah, dan bilah status menyebutkan angkanya. Jika tidak menemukan, tidak ada yang berubah dan Anda diberi tahu. Coba angka awal yang berbeda di sel yang diubah, dan periksa apakah rumusnya memang memakai sel itu. Cari Nilai Target adalah satu langkah yang dapat Anda urungkan.

## Pengelola Skenario

Skenario adalah kumpulan nilai bernama untuk beberapa sel. Gunakan skenario untuk mencoba "jika-maka" pada satu lembar. Misalnya, simpan skenario "Penjualan rendah" dan skenario "Penjualan tinggi" untuk sel masukan yang sama, lalu tampilkan yang ingin Anda lihat.

1. Pilih **Pengelola Skenario** pada tab **Data**.
2. Tekan **Tambah**. Ketik nama, seperti Penjualan tinggi. Pada **Sel yang diubah**, ketik sel-sel yang diatur skenario, seperti B1:B3 atau D5. Sebuah skenario dapat mengubah hingga 32 sel. Sel yang Anda pilih sudah diisi untuk Anda. Setiap sel dimulai dengan nilai yang dimilikinya sekarang. Ketik nilai yang harus dipakai skenario ini di tabel, lalu tekan **OK**.
3. Pilih skenario di daftar dan tekan **Tampilkan** untuk memasukkan nilainya ke sel. Mengklik dua kali nama juga melakukan hal yang sama. Tekan **Ubah** untuk mengganti namanya, komentarnya, atau nilainya. Tekan **Hapus** untuk menghapusnya.

Tampilkan, Tambah, Ubah, dan Hapus masing-masing adalah satu langkah yang dapat Anda urungkan.

**Ringkasan** menempatkan semua skenario berdampingan pada lembar baru bernama Ringkasan Skenario. Perintah ini meminta sel hasil, yaitu sel yang ingin Anda bandingkan, seperti total di B10. Untuk setiap skenario, ringkasan menampilkan nilai sel-sel yang berubah dan hasil sel-sel hasil, dihitung dengan nilai skenario tersebut. Kolom bernama Nilai Saat Ini menampilkan keadaan sel saat Anda membuat ringkasan. Sel-sel Anda sendiri tetap seperti semula.

Skenario disimpan di dalam buku kerja. Buku kerja yang dibuat program lain tetap menyimpan skenarionya, dan Anda dapat melihatnya di sini. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tidak memuatnya, dan penyimpanan akan memberi tahu Anda.

## Subtotal

Gunakan ini untuk meletakkan baris total di bawah setiap kelompok baris yang memiliki entri yang sama di satu kolom, serta total keseluruhan di bagian bawah. Daftar memerlukan baris judul, dan sebaiknya diurutkan menurut kolom yang dikelompokkan, agar entri yang sama berdampingan.

1. Pilih satu sel di dalam daftar.
2. Pilih **Subtotal** pada grup **Garis Besar** di tab **Data**.
3. Pada **Pada setiap perubahan di**, pilih kolom yang mengelompokkan baris, seperti Wilayah.
4. Pada **Gunakan fungsi**, pilih yang akan dihitung: **Jumlah**, **Hitung**, **Rata-rata**, **Maks**, **Min**, dan lainnya.
5. Pada **Tambahkan subtotal ke**, centang setiap kolom yang akan ditotal.
6. Biarkan **Ganti subtotal saat ini** tercentang untuk menggantikan subtotal yang sudah ada.
7. **Ringkasan di bawah data** mengikuti pengaturan garis besar lembar. Biarkan tercentang untuk meletakkan setiap baris total di bawah kelompoknya, atau hapus centangnya untuk meletakkan setiap baris total di atas kelompoknya, dengan total keseluruhan di bagian atas. Tekan **OK**.

Setiap baris total bertuliskan "Total Timur" (atau entri yang dimaksud) dan berisi rumus SUBTOTAL, sehingga melewati baris total lainnya. Baris dalam setiap kelompok dikelompokkan, sehingga Anda dapat memakai tombol 1, 2, dan 3 di sebelah baris untuk hanya menampilkan total. Untuk menghapus total, buka **Subtotal** lagi dan tekan **Hapus Semua**. Jika total dipindahkan ke sisi lain lembar yang sudah memiliki kelompoknya sendiri, bilah status memberi tahu bahwa kelompok-kelompok itu kini memiliki baris ringkasan di sisi tersebut, dan Hapus Semua mengembalikan sisi itu seperti sebelum subtotal pertama, kecuali jika Anda sudah mengubahnya sendiri. Saat total berada di atas, tombol garis besar berada di baris total, sehingga tombol plus atau minus di sebelah "Total Timur" menampilkan atau menyembunyikan baris Timur di bawahnya. Jika Anda mematikan **Ganti subtotal saat ini**, total baru harus berada di sisi yang sama dengan total yang sudah ada. Jika tidak, tidak ada yang berubah dan pesan akan memberi tahu Anda. Menambah atau menghapus subtotal adalah satu langkah yang dapat Anda urungkan.

## Filter Lanjutan

Tombol Filter memilih baris berdasarkan satu kolom sekaligus. **Lanjutan** (pada grup **Urutkan & Filter**) memungkinkan Anda menulis kondisi di lembar, sehingga kondisinya bisa sepanjang yang Anda perlukan.

1. Di sebelah daftar, salin judul kolom yang ingin Anda uji ke satu baris sel kosong.
2. Di bawah setiap judul, tulis kondisi. Kondisi di **baris yang sama** harus semuanya benar (ini "dan"). Kondisi di **baris yang berbeda** adalah alternatif: sebuah baris daftar cukup memenuhi salah satu baris tersebut (ini "atau").
3. Pilih **Lanjutan**. Pastikan **Rentang daftar** adalah daftar beserta baris judulnya, dan ketik sel-sel judul dan kondisi Anda pada **Rentang kondisi**.
4. Pilih **Sembunyikan baris lain, dalam daftar**, atau **Salin baris ke tempat lain** lalu ketik sel pertama tempat itu, seperti H1. Centang **Abaikan baris yang menduplikasi baris sebelumnya** untuk menyalin hanya satu dari setiap baris.
5. Tekan **OK**.

Cara menulis kondisi:

- Sebuah kata, seperti **An**, memilih teks yang diawali dengannya. Huruf besar dan kecil dihitung sama. **?** mewakili satu karakter apa pun dan ***** mewakili rangkaian karakter apa pun.
- Sebuah angka memilih sel yang sama dengannya.
- **=Ani** memilih teks yang persis Ani. **=** saja memilih sel kosong.
- **>**, **>=**, **<**, **<=**, dan **<>** (tidak sama) sebelum angka atau kata membandingkan dengannya, seperti **>25**. **<>** saja memilih sel yang tidak kosong.

Baris kondisi yang kosong memilih setiap baris. Untuk menampilkan kembali baris yang disembunyikan, pilih **Hapus**. Salinan dibuat seketika dan tidak berubah ketika daftar berubah. Filter Lanjutan adalah satu langkah yang dapat Anda urungkan.

## Validasi Data

Gunakan ini untuk menentukan apa yang boleh diketik di beberapa sel, sehingga entri yang salah dihentikan atau dipertanyakan.

1. Pilih sel-selnya.
2. Pilih **Validasi Data** pada grup **Alat Data** di tab **Data**.
3. Pada tab **Pengaturan**, pilih yang **Diizinkan**: **Nilai apa saja** (tanpa aturan), **Bilangan bulat**, **Desimal**, **Daftar**, **Tanggal**, **Waktu**, **Panjang teks**, atau **Kustom**.
4. Untuk angka, tanggal, waktu, atau panjang teks, pilih **Data** (**di antara**, **tidak di antara**, **sama dengan**, dan seterusnya), lalu ketik batasnya. Batas dapat berupa nilai atau rumus yang diawali =.
5. Untuk **Daftar**, ketik pilihan dengan koma di antaranya, seperti Merah,Hijau,Biru, atau ketik rentang sel setelah =, seperti =$A$1:$A$5. Sel kemudian menampilkan panah berisi pilihan. Matikan **Daftar tarik-turun dalam sel** untuk menyembunyikan panah.
6. Untuk **Kustom**, ketik rumus yang diawali =. Entri diizinkan jika rumus itu benar.
7. **Abaikan kosong** membiarkan sel kosong lolos.

Pada tab **Pesan Masukan**, centang kotaknya lalu tulis judul dan pesan. Keduanya tampil di sebelah sel setiap kali sel itu dipilih.

Pada tab **Peringatan Galat**, pilih apa yang terjadi pada entri yang tidak diizinkan. **Berhenti** menolaknya. **Peringatan** menanyakan apakah entri tetap disimpan. **Informasi** memberi tahu orang tersebut dan tetap menyimpannya. Tulis judul dan pesan untuk peringatan itu.

**Lingkari Data Tidak Valid** (di sebelah **Validasi Data**) memberi lingkaran merah pada setiap sel di lembar yang nilainya tidak diizinkan oleh aturannya, seperti angka yang sudah ada sebelum aturan dibuat, atau hasil rumus. Sebuah pesan memberi tahu Anda jika semua sel sudah benar. Sel yang sudah Anda perbaiki kehilangan lingkarannya. **Hapus Lingkaran Validasi** menghapus semua lingkaran.

**Hapus Semua** menghapus aturan dari sel yang dipilih. Aturan baru menggantikan aturan apa pun yang dimiliki sel yang dipilih; sel lain tetap memiliki aturannya. Mengubah aturan adalah satu langkah yang dapat Anda urungkan, dan aturan disimpan dalam berkas.

## Konsolidasi

Gunakan ini untuk menggabungkan rentang yang serupa, seperti tabel yang sama dari beberapa lembar, menjadi satu tabel.

1. Pilih **Konsolidasi** pada grup **Alat Data** di tab **Data**.
2. Pilih **Fungsi** untuk digabungkan, seperti **Jumlah** atau **Rata-rata**.
3. Ketik rentang di kotak lalu tekan **Tambah**. Gunakan **Sheet2!A1:C10** untuk rentang di lembar lain. Tambahkan setiap rentang yang ingin digabungkan. Pilih rentang di daftar dan tekan **Hapus** untuk mengeluarkannya.
4. Jika rentang memiliki nama di baris pertamanya, centang **Baris teratas berisi nama kolom**. Jika memiliki nama di kolom pertamanya, centang **Kolom kiri berisi nama baris**.
5. Pada **Letakkan hasil di**, ketik sel pertama untuk hasilnya.
6. Centang **Buat tautan ke data sumber** jika setiap hasil harus berupa rumus yang membaca sel sumbernya, sehingga ikut berubah saat sel sumbernya berubah. Tanpa centang ini, hasilnya berupa angka biasa.
7. Tekan **OK**.

Jika tidak ada nama yang dicentang, sel di tempat yang sama pada setiap rentang digabungkan (sel pertama dari setiap rentang, lalu sel kedua, dan seterusnya). Jika nama dicentang, sel dengan nama yang sama digabungkan, di mana pun letaknya dalam setiap rentang, dan nama yang hanya dimiliki sebagian rentang tetap mendapat baris atau kolom. Konsolidasi adalah satu langkah yang dapat Anda urungkan.

## Lembar Ramalan

Lembar Ramalan membuat lembar baru berisi tabel dan bagan nilai masa depan, dihitung dari garis waktu dan deret nilai. Perintah ini menemukan tren dan musim yang berulang pada nilai, lalu menambahkan pita tempat nilai masa depan kemungkinan besar berada.

1. Pilih garis waktu dan nilai, dua kolom berdampingan (garis waktu di depan). Garis waktu berupa tanggal atau angka dengan jarak yang sama, seperti setiap bulan atau setiap 5 satuan. Baris pertama berisi nama tidak diikutsertakan.
2. Pilih **Lembar Ramalan** pada grup **Ramalan** di tab **Data**.
3. Sebuah gambar menampilkan ramalan, dan berubah saat Anda mengubah pilihan. Pilih **Bagan garis** atau **Bagan kolom**.
4. Atur pilihannya:
   - **Rentang garis waktu** dan **Rentang nilai** adalah sel yang dibaca. Tulis sel lain di sini jika pilihan awal salah.
   - **Awal ramalan** dikosongkan untuk mulai setelah titik terakhir. Tanggal atau angka sebelum titik terakhir memulai di sana dan hanya memakai titik-titik sebelumnya, sehingga Anda dapat membandingkan ramalan dengan yang sebenarnya terjadi.
   - **Akhir ramalan** adalah waktu terakhir yang diramalkan. Paling banyak 10.000 titik yang diramalkan.
   - **Interval kepercayaan** adalah seberapa yakin pita tersebut, mulai dari 95%.
   - **Musiman** ditemukan dengan memeriksa nilai, atau Anda dapat mengatur panjang musim dalam titik, atau memilih tidak ada. Nilai harus memuat dua musim penuh.
   - **Isi titik yang hilang dengan** menentukan nilai untuk waktu yang tidak memiliki nilai: garis lurus di antara tetangganya, atau nol.
   - **Gabungkan duplikat dengan** menentukan cara nilai pada waktu yang sama digabungkan: Rata-rata, Hitung, CountA, Maks, Median, Min, atau Jumlah.
   - **Sertakan statistik prakiraan** menambahkan tabel Alpha, Beta, Gamma, MASE, SMAPE, MAE, dan RMSE, yang menunjukkan seberapa baik ramalan cocok dengan nilai.
5. Jika ada yang salah, kotak dialog menjelaskannya dengan kata-kata biasa (langkah yang tidak sama, terlalu sedikit titik, teks pada nilai), dan **Buat** menunggu.
6. Tekan **Buat**.

Lembar baru diletakkan sebelum lembar yang sedang Anda buka. Tabelnya memiliki kolom Garis Waktu, Nilai, Ramalan, Batas Kepercayaan Bawah, dan Batas Kepercayaan Atas. Kolom Ramalan dan batasnya adalah rumus yang memakai `FORECAST.ETS` dan `FORECAST.ETS.CONFINT` pada kolom Garis Waktu dan Nilai milik tabel itu, sehingga dihitung ulang saat Anda mengubah nilai. Bagan menampilkan nilai, ramalan, dan pita. Lembar Ramalan adalah satu langkah yang dapat Anda urungkan.

Angkanya dapat berbeda beberapa persen dari program lembar kerja lain, yang memakai cara sendiri untuk menyesuaikan pemulusan, dan ramalan yang sudah ada di berkas tetap menyimpan nilai yang disimpan berkas sampai data yang dibacanya diubah.

Berkas dengan lebih dari 1.000.000 titik tidak digunakan.
