# Alat Data
<!-- keywords: prakiraan lembar, prakiraan, nilai masa depan, interval kepercayaan, musiman, tren, pemulusan, formulir, formulir data, entri data, rekaman, rekaman baru, temukan berikutnya, temukan sebelumnya, kriteria, pengelola skenario, skenario, analisis what-if, apa jika, ringkasan skenario, data, tab data, teks ke kolom, pisah, pemisah, koma, tab, lebar tetap, hapus duplikat, goal seek, pencarian target, subtotal, total, filter lanjutan, kondisi, validasi data, lingkari data tidak valid, lingkaran merah, hapus lingkaran validasi, konsolidasi, gabungkan rentang, izinkan, daftar, drop-down, pesan masukan, peringatan galat, baris berulang, unik, berkas teks, csv, dari teks, kolom, pembatas, tanda kutip, edit tautan, perbarui nilai, ubah sumber, tautan, eksternal, sumber, putus tautan, periksa status, buka sumber -->

Tab **Data** berisi alat yang bekerja pada daftar nilai: mengambil teks dari berkas, mengurutkan daftar, memecah teks menjadi kolom, dan menghapus baris yang berulang.

## Dari Teks/CSV

**Dari Teks/CSV** membuka berkas teks (.csv, .tsv, atau .txt). Perintah ini sama dengan **Buka Berkas Teks** pada tab **Berkas**. JeffCrunch menanyakan cara berkas dipisah menjadi kolom dan jenis teks yang dikandungnya, lalu menampilkannya sebagai buku kerja baru.

## Dari Berkas

**Dari Berkas** pada grup **Dapatkan Data** memuat baris dari berkas JSON, berkas XML, atau buku kerja lain sebagai tabel. Hanya berkas di komputer ini yang dapat dimuat. Alamat web atau berkas di komputer lain ditolak.

1. Klik **Dari Berkas**, lalu **Telusuri** dan pilih berkasnya. Baris pertama langsung ditampilkan, beserta jenis yang ditemukan JeffCrunch untuk setiap kolom dari baris-baris pertama itu: angka, tanggal, benar atau salah, atau teks. Seluruh berkas diperiksa saat dimuat, sehingga sebuah kolom bisa berakhir sebagai teks. Tidak ada yang berubah di lembar sampai Anda mengklik **Muat**.
2. Jika berkas berisi lebih dari satu daftar, pilih daftar yang akan dimuat. Pada berkas JSON, itu berupa daftar item dengan nilai bernama, misalnya `$.items`. Pada berkas XML, itu berupa item yang berulang, misalnya `/feed/entry`. Pada buku kerja, itu berupa lembar, tabel, atau rentang bernama.
3. Item JSON di dalam item lain menjadi kolom bernama `luar.dalam`, sampai empat tingkat. Daftar nilai sederhana digabungkan menjadi satu sel (10.000 nilai pertama), atau dibagi ke beberapa kolom (16 pertama), sesuai pilihan Anda. Pada XML, atribut dan item anak menjadi kolom. Dua nilai berbeda tidak pernah berbagi kolom: jika dua nama akan sama, nama kedua diberi angka tambahan atau, pada XML, awalan, dan sebuah catatan menyebutkannya. Nama yang berulang di dalam satu item JSON menyimpan nilai terakhirnya, dan sebuah catatan menyebutkannya. Angka yang terlalu besar atau terlalu kecil untuk sel disimpan sebagai teks sebagaimana tertulis, dan karakter yang tidak dapat dibaca ditampilkan sebagai tanda pengganti; keduanya dicatat.
4. Centang **Muat setiap kolom sebagai teks** untuk menyimpan setiap nilai persis seperti tertulis. Kolom angka dengan awalan nol, seperti 007, selalu disimpan sebagai teks.
5. Ketik sel tempat tabel akan diletakkan, lalu klik **Muat**. Jika tabel akan menimpa sel yang berisi sesuatu, JeffCrunch meminta konfirmasi sebelum menggantinya. Jika tidak ada ruang, atau lembar dilindungi, Anda diberi tahu dan tidak ada yang berubah.

Tabel adalah satu langkah **Urungkan**. Tabel mengingat berkas dan pilihan Anda. Untuk membaca ulang berkas, klik sel di dalam tabel lalu klik **Segarkan** pada tab **Data**. **Segarkan Semua** adalah tombol besar di sebelahnya: tombol ini membaca setiap tabel yang dibuat dengan Dapatkan Data, lalu menyegarkan setiap tabel pivot, semuanya sebagai satu langkah Urungkan. Tabel bertambah atau berkurang mengikuti baris berkas. Kolom yang Anda tambahkan di samping tabel tetap ada: rumus yang Anda salin ke bawah akan diisikan ke baris baru, dan baris yang sudah tidak ada di berkas ikut dihapus. Jika berkas hilang atau tidak dapat dibaca, Anda diberi tahu dan tidak ada yang berubah. Jika kolom berkas berubah, Anda diberi tahu kolom mana yang hilang atau baru, dan tidak ada yang berubah. Buat ulang tabel dengan Dapatkan Data, atau kembalikan kolomnya ke dalam berkas.

Tautan ke berkas disimpan di dalam tabel, dalam buku kerja `.xlsx` yang tersimpan, pada bagian yang diabaikan program lain. Tautan itu memuat jalur lengkap berkas di komputer ini, sehingga siapa pun yang menerima buku kerja dapat membaca jalur tersebut. Untuk menghapusnya sebelum Anda membagikan buku kerja, gunakan **Periksa**, lalu **Tautan ke berkas data**, lalu **Hapus**. Salinan yang disimpan sebagai jenis berkas lain tidak menyimpan tautan ini, dan **Periksa Kompatibilitas** akan memberi tahu Anda. **Periksa** mencantumkan tautan di bawah **Tautan ke berkas data** dan dapat menghapusnya. Tabel dan selnya tetap ada.

Berkas dibaca dengan batas: berkas hingga 64 MB, 200.000 baris, 256 kolom, dan 32.767 karakter per sel. XML yang memuat deklarasi jenis dokumen atau entitas ditolak. Setiap bagian yang terpotong oleh batas akan diberitahukan.

## Edit Tautan

Jika buku kerja memiliki rumus yang membaca buku kerja lain (seperti ='[Harga.xlsx]Sheet1'!A1), **Edit Tautan** pada grup **Dapatkan Data** mencantumkan buku kerja lain tersebut, yang disebut sumber. Setiap baris menunjukkan lokasi sumber dan statusnya. Sampai buku kerja dibuka kembali bersama sumbernya, buku kerja menampilkan angka yang disimpan dalam berkasnya dari sumber tersebut.

- **Periksa Status** mencari setiap sumber di komputer ini, lalu menyatakan **Ditemukan**, atau menyatakan bahwa sumber tidak ditemukan. Sumber di web tidak diambil, sehingga statusnya menyatakan belum diperiksa.
- **Perbarui Nilai** membaca sumber yang dipilih dari komputer ini, dan memberi setiap sel yang hanya merupakan referensi ke salah satu selnya nilai yang dimiliki sel tersebut sekarang. Sel yang melakukan lebih dari sekadar menyalin nilai, seperti `=[1]Harga!A1*2`, tetap menyimpan angka yang disimpan berkasnya, dan pesan di bawah daftar menyebutkan berapa sel yang diperbarui dan berapa yang tidak. Perbarui Nilai adalah satu langkah, sehingga **Urungkan** mengembalikan nilai lama. Nilai baru juga ditulis ke bagian tautan di dalam berkas saat Anda menyimpan, sehingga buku kerja menampilkannya lagi saat dibuka berikutnya, bahkan ketika sumbernya tidak ada.
- **Ubah Sumber** memungkinkan Anda memilih berkas lain untuk tautan yang dipilih, misalnya ketika buku kerja dipindahkan. Rumus tetap seperti semula. Lokasi baru ditulis ke berkas saat Anda menyimpan, dan **Urungkan** membatalkan perubahan ini.
- **Buka Sumber** membuka sumber yang dipilih di jendela baru, jika sumber itu dapat ditemukan.
- **Putus Tautan** mengubah setiap sel yang membaca sumber yang dipilih menjadi nilai yang ditampilkannya sekarang, dan nama yang mewakili suatu lokasi di dalamnya menjadi **#REF!**. JeffCrunch menanyakan konfirmasi terlebih dahulu. Putus Tautan adalah satu langkah, sehingga **Urungkan** mengembalikan rumus, nama, dan tautan, sampai Anda menyimpan buku kerja. Jika tidak ada tautan yang tersisa, berkas disimpan tanpa bagian yang menyimpannya.

## Formulir

**Formulir** (pada grup **Alat Data**, dan di kotak pencarian) menampilkan satu baris daftar dalam satu waktu, seperti formulir kecil. Daftar adalah tabel, atau blok sel dengan baris nama di atas dan baris data di bawah setiap nama. Klik sel mana pun di dalam daftar terlebih dahulu.

- Setiap kotak adalah satu kolom, diberi label dengan nama di baris teratas. Ketik di dalam kotak untuk mengubah nilai. Kotak yang diberi warna abu-abu berisi rumus: formulir menampilkan hasilnya, dan Anda mengubah rumusnya di lembar.
- **Temukan Berikutnya** dan **Temukan Sebelumnya** pindah ke baris berikutnya dan baris sebelumnya. Pindah ke baris lain, atau menekan **Enter**, menuliskan apa yang Anda ketik di baris ini.
- **Baru** menampilkan formulir kosong. Isi, lalu tekan **Baru** lagi, **Temukan Berikutnya**, atau **Tutup**. Baris ditambahkan di bawah baris terakhir daftar. Tabel bertambah satu baris. Jika sel di bawah daftar tidak kosong, formulir memberi tahu Anda dan tidak menambahkan apa pun.
- **Hapus** menghapus baris yang sedang Anda lihat, setelah meminta konfirmasi. Baris di bawahnya naik. Tabel selalu menyimpan setidaknya satu baris data. Baris yang disembunyikan, atau yang disembunyikan oleh filter, tidak ditampilkan di formulir, tidak dihitung, dan tidak pernah dihapus olehnya. Jika daftar terlalu panjang untuk ditelusuri seluruhnya, formulir memberi tahu bahwa hanya baris pertama yang ditampilkan.
- **Pulihkan** membatalkan apa yang Anda ketik di baris ini, sebelum Anda meninggalkannya.
- **Kriteria** memungkinkan Anda mencari baris. Ketik kondisi di satu atau beberapa kotak, lalu tekan **Temukan Berikutnya** atau **Temukan Sebelumnya**. Kondisi berupa nilai, seperti **Ann** (teks yang cocok dengan awal kata, dan **?** serta ***** mewakili huruf), atau nilai setelah tanda: **>100**, **<=5**, **<>0**, **=Oslo**. Sebuah baris harus memenuhi setiap kondisi yang Anda ketik. Tekan **Formulir** untuk kembali, atau **Bersihkan** untuk mulai lagi. Selama kriteria aktif, **Temukan Berikutnya** dan **Temukan Sebelumnya** melewati baris yang tidak memenuhinya.
- **Tutup** menuliskan baris dan menutup formulir.

Setiap baris yang Anda ubah, tambahkan, atau hapus adalah satu langkah, sehingga **Urungkan** di lembar membatalkannya sekaligus.

## Urutkan dan filter

Grup **Urutkan & Filter** memiliki perintah yang sama dengan menu **Urutkan dan Pilih Baris** pada tab **Beranda**. Lihat [Urutkan dan filter](sort-filter).

## Teks ke Kolom

Gunakan ini ketika satu kolom berisi beberapa potongan teks yang ingin Anda pisahkan ke kolom terpisah, seperti "Smith, Ann, 31".

1. Pilih sel-sel dari **satu** kolom.
2. Pilih **Teks ke Kolom** pada tab **Data**.
3. Pilih cara memecah teks:
   - **Pisahkan pada pemisah**: centang tanda yang berada di antara potongan: **Tab**, **Titik koma**, **Koma**, **Spasi**, atau **Lainnya**, tempat Anda mengetik tanda sendiri. **Perlakukan pemisah berurutan sebagai satu** mencegah dua koma berurutan membuat potongan kosong. **Teks dalam tanda kutip** menyebutkan tanda yang membungkus potongan yang tidak boleh dipecah, seperti "Lee, Bo". Di dalam tanda kutip, dua tanda berurutan mewakili satu tanda.
   - **Pisahkan pada posisi tetap**: ketik posisi tempat kolom baru dimulai, dengan menghitung karakter dari 1. Misalnya, 5, 12 memulai kolom baru pada karakter kelima dan kedua belas.
4. Pilih jenis setiap potongan: **Umum** membaca angka dan tanggal seperti yang Anda ketik, dan **Teks** menyimpan setiap potongan apa adanya, sehingga "007" tetap "007".
   Untuk memperlakukan satu potongan secara berbeda, ketik nomornya di **Potongan** dan pilih cara membacanya: **Sama seperti di atas**, **Seperti yang diketik**, **Teks**, tanggal dengan urutan **bulan, hari, tahun**, **hari, bulan, tahun**, atau **tahun, bulan, hari**, atau **Abaikan**, yang menempatkan potongan itu di mana pun. Potongan pertama adalah 1. Potongan tanggal dapat memiliki nama bulan (**5 Mar 2024**, **March 5, 2024**) dalam bahasa Inggris atau bahasa Anda, dapat dipisahkan dengan /, -, titik, koma, atau spasi, dan dapat diikuti waktu (**10:30**, **10:30:45**, atau **10:30 PM**). Potongan yang bukan tanggal dalam urutan yang Anda pilih (bulan 13, hari yang tidak ada, tahun yang bukan dua atau empat digit, atau waktu yang tidak masuk akal) disimpan sebagai teks, sebagaimana diketik; potongan itu tidak pernah dibaca dengan urutan lain.
5. Di **Letakkan potongan mulai dari**, ketik sel pertama untuk potongan. Awalnya sel pertama yang Anda pilih.
6. Kotak di bawah pilihan menampilkan baris pertama yang sudah dipecah sesuai pilihan Anda. Tekan **OK**.

Potongan ditempatkan di sel-sel di kanan sel pertama, dan menggantikan isi yang ada di sana. Teks ke Kolom adalah satu langkah yang dapat Anda urungkan.

## Hapus Duplikat

Gunakan ini untuk menyimpan hanya yang pertama dari setiap kelompok baris yang berulang.

1. Pilih sel-sel daftar, atau satu sel di dalamnya. JeffCrunch mengambil seluruh blok data di sekitar satu sel.
2. Pilih **Hapus Duplikat**.
3. Centang **Data saya memiliki baris header** jika baris pertama berisi nama. Baris itu tetap ada dan tidak dibandingkan.
4. Centang kolom yang harus sama agar dua baris dihitung sebagai berulang. Dua baris berulang hanya jika setiap kolom yang dicentang sama. Huruf besar dan kecil dianggap sama, dan rumus dihitung sebagai nilai yang dihasilkannya.
5. Tekan **OK**. Kotak dialog memberi tahu berapa baris yang dihapus dan berapa yang tersisa.

Baris di bawah baris yang dihapus naik di dalam sel-sel yang dipilih, beserta format dan rumusnya. Sel-sel di bawah pilihan tidak bergerak. Baris kosong yang tertinggal di bagian bawah pilihan dikosongkan. Hapus Duplikat adalah satu langkah yang dapat Anda urungkan.

## Goal Seek

Gunakan ini ketika Anda tahu hasil yang diinginkan tetapi tidak tahu angka yang menghasilkannya. Misalnya, sel pembayaran berisi rumus yang memakai suku bunga, dan Anda ingin pembayarannya 500. Goal Seek mencoba angka di sel suku bunga sampai pembayarannya 500.

1. Pilih **Goal Seek** pada tab **Data**.
2. Di **Sel dengan rumus**, ketik sel yang berisi rumus, misalnya B3. Awalnya sel yang Anda pilih.
3. Di **Nilai yang Anda inginkan**, ketik hasil yang Anda inginkan dari rumus.
4. Di **Sel yang akan diubah**, ketik sel yang boleh diubah Goal Seek, misalnya B1. Sel ini harus berisi angka, bukan rumus.
5. Tekan **OK**.

Jika Goal Seek menemukan angka, angka itu tetap berada di sel yang diubah, dan bilah status menyatakan angkanya. Jika tidak menemukan, tidak ada yang berubah dan Anda diberi tahu. Coba angka awal yang berbeda di sel yang diubah, dan pastikan rumus benar-benar memakai sel itu. Goal Seek adalah satu langkah yang dapat Anda urungkan.

## Pengelola Skenario

Skenario adalah kumpulan nilai bernama untuk beberapa sel. Gunakan skenario untuk mencoba "apa jika" pada satu lembar. Misalnya, simpan skenario "Penjualan rendah" dan skenario "Penjualan tinggi" untuk sel masukan yang sama, lalu tampilkan yang ingin Anda lihat.

1. Pilih **Pengelola Skenario** pada tab **Data**.
2. Tekan **Tambah**. Ketik nama, misalnya Penjualan tinggi. Di **Sel yang diubah**, ketik sel-sel yang diatur skenario, misalnya B1:B3 atau D5. Skenario dapat mengubah hingga 32 sel. Sel yang Anda pilih sudah diisi untuk Anda. Setiap sel mulai dengan nilai yang dimilikinya sekarang. Ketik nilai yang harus dipakai skenario ini di tabel, lalu tekan **OK**.
3. Pilih skenario di daftar dan tekan **Tampilkan** untuk memasukkan nilainya ke sel. Mengklik dua kali nama juga melakukan hal yang sama. Tekan **Edit** untuk mengubah namanya, komentarnya, atau nilainya. Tekan **Hapus** untuk menghilangkannya.

Tampilkan, Tambah, Edit, dan Hapus masing-masing adalah satu langkah yang dapat Anda urungkan.

**Ringkasan** menempatkan semua skenario berdampingan pada lembar baru bernama Ringkasan Skenario. Perintah ini meminta sel hasil, yaitu sel yang ingin Anda bandingkan, misalnya total di B10. Untuk setiap skenario, ringkasan menampilkan nilai sel yang diubah dan hasil sel hasil, yang dihitung dengan nilai skenario tersebut. Kolom bernama Nilai Saat Ini menampilkan keadaan sel ketika ringkasan dibuat. Sel Anda sendiri dibiarkan seperti semula.

Skenario disimpan di dalam buku kerja. Buku kerja yang dibuat program lain tetap menyimpan skenarionya, dan Anda dapat melihatnya di sini. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tidak menyimpannya, dan penyimpanan akan memberi tahu Anda.

## Subtotal

Gunakan ini untuk menempatkan baris total di bawah setiap kelompok baris yang memiliki entri sama pada satu kolom, serta total keseluruhan di bagian bawah. Daftar memerlukan baris header, dan sebaiknya diurutkan menurut kolom yang dikelompokkan, agar entri yang sama berdampingan.

1. Pilih sel di dalam daftar.
2. Pilih **Subtotal** pada grup **Garis Besar** di tab **Data**.
3. Di **Pada setiap perubahan di**, pilih kolom yang mengelompokkan baris, misalnya Wilayah.
4. Di **Gunakan fungsi**, pilih yang akan dihitung: **Sum**, **Count**, **Average**, **Max**, **Min**, dan lainnya.
5. Di **Tambahkan subtotal ke**, centang setiap kolom yang akan dijumlahkan.
6. Biarkan **Ganti subtotal saat ini** tercentang untuk mengganti subtotal yang sudah ada.
7. **Ringkasan di bawah data** mengikuti pengaturan garis besar lembar. Biarkan tercentang untuk menempatkan setiap baris total di bawah kelompoknya, atau hapus centangnya untuk menempatkan setiap baris total di atas kelompoknya, dengan total keseluruhan di bagian atas. Tekan **OK**.

Setiap baris total bertuliskan "Total Wilayah Timur" (atau entri yang dimaksud) dan berisi rumus SUBTOTAL, sehingga tidak ikut menghitung baris total lainnya. Baris setiap kelompok dikelompokkan, sehingga Anda dapat memakai tombol 1, 2, dan 3 di samping baris untuk hanya menampilkan total. Untuk menghapus total, buka **Subtotal** lagi lalu tekan **Hapus Semua**. Jika total dipindahkan ke sisi lain lembar yang sudah memiliki kelompoknya sendiri, bilah status memberi tahu bahwa kelompok itu sekarang memiliki baris ringkasan di sisi tersebut, dan Hapus Semua mengembalikan sisinya seperti sebelum subtotal pertama, kecuali jika Anda mengubahnya sendiri sejak itu. Ketika total berada di atas, tombol garis besar berada di baris total, sehingga tombol plus atau minus di samping "Total Wilayah Timur" menampilkan atau menyembunyikan baris Timur di bawahnya. Jika Anda mematikan **Ganti subtotal saat ini**, subtotal baru harus berada di sisi yang sama dengan yang sudah ada. Jika tidak, tidak ada yang diubah, dan pesan akan menyatakannya. Menambah atau menghapus subtotal adalah satu langkah yang dapat Anda urungkan.

## Filter Lanjutan

Tombol Filter memilih baris berdasarkan satu kolom dalam satu waktu. **Lanjutan** (pada grup **Urutkan & Filter**) memungkinkan Anda menulis kondisi di lembar, sehingga kondisinya dapat sepanjang yang Anda perlukan.

1. Di samping daftar, salin judul kolom yang ingin diuji ke dalam satu baris sel kosong.
2. Di bawah setiap judul, tulis kondisi. Kondisi di **baris yang sama** harus semuanya benar (ini "dan"). Kondisi di **baris yang berbeda** adalah alternatif: sebuah baris daftar cukup memenuhi salah satu baris tersebut (ini "atau").
3. Pilih **Lanjutan**. Pastikan **Rentang daftar** adalah daftar beserta baris headernya, dan ketik sel judul serta kondisi Anda di **Rentang kondisi**.
4. Pilih **Sembunyikan baris lain, dalam daftar**, atau **Salin baris ke tempat lain** lalu ketik sel pertama tempat tujuan, misalnya H1. Centang **Abaikan baris yang menduplikasi baris sebelumnya** untuk menyalin hanya satu dari setiap baris.
5. Tekan **OK**.

Cara menulis kondisi:

- Sebuah kata, misalnya **An**, memilih teks yang diawali dengannya. Huruf besar dan kecil dianggap sama. **?** mewakili satu karakter apa pun dan ***** mewakili rangkaian karakter apa pun.
- Sebuah angka memilih sel yang sama dengannya.
- **=Ann** memilih teks yang persis Ann. **=** saja memilih sel kosong.
- **>**, **>=**, **<**, **<=**, dan **<>** (tidak sama dengan) sebelum angka atau kata membandingkan dengannya, misalnya **>25**. **<>** saja memilih sel yang tidak kosong.

Baris kondisi yang kosong memilih setiap baris. Untuk menampilkan kembali baris yang tersembunyi, pilih **Bersihkan**. Salinan dibuat seketika dan tidak berubah ketika daftar berubah. Filter Lanjutan adalah satu langkah yang dapat Anda urungkan.

## Validasi Data

Gunakan ini untuk menentukan apa yang boleh diketik di sebagian sel, sehingga entri yang salah dihentikan atau dipertanyakan.

1. Pilih sel-selnya.
2. Pilih **Validasi Data** di grup **Alat Data** pada tab **Data**.
3. Pada tab **Pengaturan**, pilih yang akan **Diizinkan**: **Nilai apa saja** (tanpa aturan), **Bilangan bulat**, **Desimal**, **Daftar**, **Tanggal**, **Waktu**, **Panjang teks**, atau **Kustom**.
4. Untuk angka, tanggal, waktu, atau panjang teks, pilih **Data** (**di antara**, **tidak di antara**, **sama dengan**, dan seterusnya) lalu ketik batasnya. Batas dapat berupa nilai atau rumus yang diawali =.
5. Untuk **Daftar**, ketik pilihan dengan koma di antaranya, misalnya Merah,Hijau,Biru, atau ketik rentang sel setelah =, misalnya =$A$1:$A$5. Sel kemudian menampilkan panah dengan pilihannya. Matikan **Drop-down dalam sel** untuk menyembunyikan panah.
6. Untuk **Kustom**, ketik rumus yang diawali =. Entri diizinkan ketika rumus benar.
7. **Abaikan kosong** membiarkan sel kosong lolos.

Pada tab **Pesan Masukan**, centang kotaknya lalu tulis judul dan pesan. Keduanya muncul di samping sel setiap kali sel dipilih.

Pada tab **Peringatan Galat**, pilih apa yang terjadi pada entri yang tidak diizinkan. **Stop** menolaknya. **Peringatan** menanyakan apakah entri tetap disimpan. **Informasi** memberi tahu pengguna dan tetap menyimpannya. Tulis judul dan pesan untuk peringatan.

**Lingkari Data Tidak Valid** (di samping **Validasi Data**) memberi lingkaran oval merah pada setiap sel di lembar yang nilainya tidak diizinkan aturannya, misalnya angka yang sudah ada sebelum aturan dibuat, atau hasil rumus. Sebuah pesan memberi tahu Anda jika semua sel sudah benar. Sel yang Anda perbaiki kehilangan lingkarannya. **Hapus Lingkaran Validasi** menghilangkan semua lingkaran.

**Hapus Semua** menghapus aturan dari sel yang dipilih. Aturan baru menggantikan aturan apa pun yang dimiliki sel yang dipilih; sel lain tetap memiliki aturannya. Mengubah aturan adalah satu langkah yang dapat Anda urungkan, dan aturan disimpan di dalam berkas.

## Konsolidasi

Gunakan ini untuk menggabungkan rentang yang serupa, seperti tabel yang sama dari beberapa lembar, menjadi satu tabel.

1. Pilih **Konsolidasi** di grup **Alat Data** pada tab **Data**.
2. Pilih **Fungsi** untuk digabungkan, seperti **Sum** atau **Average**.
3. Ketik rentang di kotak dan tekan **Tambah**. Gunakan **Sheet2!A1:C10** untuk rentang di lembar lain. Tambahkan setiap rentang yang ingin digabungkan. Pilih rentang di daftar dan tekan **Hapus** untuk mengeluarkannya.
4. Jika rentang memiliki nama di baris pertamanya, centang **Baris teratas berisi nama kolom**. Jika memiliki nama di kolom pertamanya, centang **Kolom kiri berisi nama baris**.
5. Di **Letakkan hasil di**, ketik sel pertama untuk hasil.
6. Centang **Buat tautan ke data sumber** jika setiap hasil harus berupa rumus yang membaca sel sumbernya, sehingga berubah ketika sel sumber berubah. Tanpa itu, hasilnya berupa angka biasa.
7. Tekan **OK**.

Jika tidak ada nama yang dicentang, sel pada posisi yang sama di setiap rentang digabungkan (sel pertama dari setiap rentang, lalu sel kedua, dan seterusnya). Jika nama dicentang, sel dengan nama yang sama digabungkan, di mana pun letaknya dalam setiap rentang, dan nama yang hanya dimiliki sebagian rentang tetap mendapat baris atau kolom. Konsolidasi adalah satu langkah yang dapat Anda urungkan.

## Lembar Prakiraan

Lembar Prakiraan membuat lembar baru dengan tabel dan bagan nilai masa depan, dihitung dari sebuah garis waktu dan serangkaian nilai. Perintah ini menemukan tren dan musim yang berulang pada nilai, lalu menambahkan pita tempat nilai masa depan kemungkinan besar berada.

1. Pilih garis waktu dan nilai, dua kolom yang berdampingan (garis waktu di depan). Garis waktu berupa tanggal atau angka dengan jarak yang sama, misalnya setiap bulan atau setiap 5 satuan. Baris nama pertama tidak ikut.
2. Pilih **Lembar Prakiraan** pada grup **Prakiraan** di tab **Data**.
3. Sebuah gambar menampilkan prakiraan, dan berubah ketika Anda mengubah pilihan. Pilih **Bagan garis** atau **Bagan kolom**.
4. Atur pilihan:
   - **Rentang garis waktu** dan **Rentang nilai** adalah sel yang dibaca. Tulis sel lain di sini jika pilihan awalnya salah.
   - **Awal prakiraan** dibiarkan kosong untuk mulai setelah titik terakhir. Tanggal atau angka sebelum titik terakhir memulai prakiraan di sana dan hanya memakai titik sebelumnya, sehingga Anda dapat membandingkan prakiraan dengan yang benar-benar terjadi.
   - **Akhir prakiraan** adalah waktu terakhir yang diprakirakan. Paling banyak 10.000 titik yang diprakirakan.
   - **Interval kepercayaan** menunjukkan seberapa yakin pita tersebut, mulai dari 95%.
   - **Musiman** ditemukan dengan memeriksa nilai, atau Anda dapat mengatur panjang musim dalam titik, atau memilih tidak ada. Nilai harus memuat dua musim penuh.
   - **Isi titik yang hilang dengan** menentukan apa yang didapat waktu tanpa nilai: garis lurus di antara tetangganya, atau nol.
   - **Gabungkan duplikat dengan** menentukan cara nilai pada waktu yang sama digabungkan: Average, Count, CountA, Max, Median, Min, atau Sum.
   - **Sertakan statistik prakiraan** menambahkan tabel Alpha, Beta, Gamma, MASE, SMAPE, MAE, dan RMSE, yang menunjukkan seberapa baik prakiraan cocok dengan nilai.
5. Jika ada yang salah, kotak dialog menjelaskannya dengan kata-kata sederhana (langkah yang tidak sama, terlalu sedikit titik, teks dalam nilai), dan **Buat** menunggu.
6. Tekan **Buat**.

Lembar baru ditempatkan sebelum lembar yang sedang Anda buka. Tabelnya memiliki kolom Garis Waktu, Nilai, Prakiraan, Batas Kepercayaan Bawah, dan Batas Kepercayaan Atas. Kolom Prakiraan dan batasnya adalah rumus yang memakai `FORECAST.ETS` dan `FORECAST.ETS.CONFINT` pada kolom Garis Waktu dan Nilai tabel itu sendiri, sehingga dihitung ulang ketika Anda mengubah nilai. Bagan menampilkan nilai, prakiraan, dan pita. Lembar Prakiraan adalah satu langkah yang dapat Anda urungkan.

Angkanya dapat berbeda beberapa persen dari program lembar kerja lain, yang memakai cara sendiri untuk menyesuaikan pemulusan, dan prakiraan yang sudah ada di dalam berkas tetap memakai nilai yang disimpan berkas tersebut sampai data yang dibacanya diubah.

Berkas dengan lebih dari 1.000.000 titik tidak digunakan.
