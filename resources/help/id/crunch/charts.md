# Bagan, sparkline, gambar, dan bentuk

<!-- keywords: objek tersemat, sisipkan objek, objek dari rentang, objek dari bagan, objek dari halaman, tempel sebagai objek, daftar objek, edit objek, tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, lingkaran, area, sebaran, kombinasi, radar, radar terisi, bagan laba-RADAR, legenda, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, teks alternatif, logaritmik, skala log, simpan sebagai gambar, templat, crtx, teks judul, kunci legenda, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, oval, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area yang dipotong, kode batang, kode QR, tautkan bentuk ke sel, bentuk menampilkan sel, kotak teks menampilkan sel, nilai sel dalam bentuk -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ubah sebuah angka dan bagan ikut berubah. Gambar dan bentuk juga berada di atas sel, tetapi tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama lalu menggunakannya sebagai nama dan label, jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Lingkaran**, **Area**, **Sebaran**, **Kombinasi**, atau **Radar**. Tanda panah kecil di samping sebuah jenis berisi variasinya, seperti kolom bertumpuk, garis dengan penanda, bagan sebaran berupa titik, titik tersambung, atau garis halus, atau bagan radar yang polos, dengan penanda, atau terisi. Bagan radar menggambar setiap label sebagai jari-jari yang keluar dari pusat dan setiap seri sebagai bentuk tertutup yang melalui angkanya pada jari-jari tersebut, sehingga beberapa ukuran dari satu hal dapat terlihat berdampingan.
3. Untuk melihat setiap jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Belum yakin jenis mana yang dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel dan mendaftarkan beberapa jenis yang cocok, dimulai dari yang terbaik. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan lingkaran. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi sesuatu.

Seri berjalan ke bawah kolom jika pilihan memiliki jumlah baris setidaknya sama dengan jumlah kolom, dan ke samping baris jika kolomnya lebih banyak daripada barisnya. Lingkaran memuat satu seri. Bagan sebaran mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya angka. Bagan memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 sampai Q3.

## Sembunyikan sebuah seri untuk sementara

Pilih sebuah bagan. Tiga tombol kecil muncul di sebelahnya. Yang berbentuk corong adalah **Filter Bagan**. Tombol ini menampilkan daftar seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang lagi untuk memunculkannya kembali di tempat yang sama. **Urungkan** juga berfungsi. Sebuah bagan selalu menyimpan setidaknya satu seri. Seri yang Anda keluarkan tetap ada dalam berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang lagi untuk memunculkannya. (Bagan donat menampilkan setiap seri saat dibuka.)

## Memindahkan dan mengubah ukurannya

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkannya, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar sebuah garis tepi sejajar dengan tepi sel.
- **Tarik ke Kisi** dan **Tarik ke Bentuk** terdapat pada tab **Format Bentuk**, dan disimpan untuk Anda, di setiap buku kerja. Dengan **Tarik ke Kisi** aktif, tepi yang Anda seret berpindah ke tepi sel terdekat. Dengan **Tarik ke Bentuk** aktif, tepi berpindah ke tepi objek lain jika jaraknya kurang dari 6 piksel. Hanya 2.000 objek lain pertama pada sebuah lembar yang dihitung untuk **Tarik ke Bentuk**. Jika keduanya aktif, tepi sel didahulukan, karena objek terjangkar pada sel, dan tepi objek lain tidak diperiksa. Perubahan langsung berlaku di setiap jendela yang terbuka.
- **Alt** membalik penarikan yang aktif untuk satu seretan: dengan **Tarik ke Kisi** atau **Tarik ke Bentuk** aktif, menahan **Alt** mematikan penarikan untuk seretan itu (tidak pernah berganti ke jenis lainnya), dan jika keduanya nonaktif, **Alt** membuat tepi menuju sel, seperti di atas.
- Objek yang diputar ditarik berdasarkan garis tepi yang terlihat (kotak terkecil yang memuatnya), bukan bingkai sebelum diputar, begitu pula objek yang ditempelinya.
- Tahan **Shift** saat menyeret sudut untuk menjaga proporsinya.
- Tombol panah menggeser objek beberapa piksel sekaligus. **Ctrl** dengan panah menggeser satu piksel, **Shift** dengan panah membuatnya lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat Anda urungkan. Klik kanan objek dan pilih **Properti** untuk mengetik **Nama** (setiap objek pada sebuah lembar memerlukan nama sendiri) dan **Teks alternatif** (sebuah kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek apa yang ditampilkannya), untuk mengetik ukurannya, dan untuk memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berukuran mengikuti sel, bergerak mengikuti sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, pada kotak yang sama, mematikan objek pada halaman cetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah apa yang ditampilkan bagan

Saat sebuah bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul pada pita.

- Tombol **+** (dan **Elemen Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, legenda, dan label data, serta menentukan letak legenda dan label.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulangnya, mengubah sel yang dibaca masing-masing, mengatur sel dengan label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi dibuat bagannya.
- **Tombol Bidang** (pada menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (pada menu **+**, untuk bagan kolom, batang, garis, area, atau sebaran) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Galat Standar**, **Persentase (5%)**, **Simpangan Baku**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai nilai Anda sendiri: ketik sel yang berisi nilai di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk nilai di bawahnya. Kosongkan salah satu untuk menggambar bilah hanya di satu sisi. Bilah berubah saat sel tersebut berubah. Berkas yang sudah memiliki nilai sendiri untuk setiap titik tetap menyimpannya.
- **Garis** (pada menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Batang Naik/Turun**, berupa kotak di antara seri pertama dan terakhir setiap kategori. Batang naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Batang Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Tabel Data** (pada menu **+**, untuk bagan kolom, garis, atau area) menempatkan kisi angka di bawah bagan, dengan satu kolom untuk setiap kategori dan satu baris untuk setiap seri. Daftarnya menyalakan dan mematikan tabel, serta mengatur garis antarbaris, garis antarkolom, garis tepi, dan kunci legenda (warna setiap seri di sebelah namanya).
- **Garis Tren** (pada menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, agar Anda melihat ke mana arahnya. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase yang tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lekukan, **Pangkat**, atau **Rata-rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menulis rumus garis pada bagan, dan **Tampilkan R-kuadrat** menulis seberapa baik garis itu cocok (1 berarti sangat cocok; 0 berarti tidak cocok sama sekali). Garis tren memiliki entri sendiri di legenda.
- Saat sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Legenda** memiliki **Di Atas Bagan**, yang meletakkan legenda di atas area plot, bukan di sebelahnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci legenda, yaitu kotak kecil berwarna seri, dalam kombinasi apa pun), **Pemisah** (yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0,0 untuk angka pada label; kosongkan untuk menampilkan angka apa adanya). **Nilai dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel, misalnya =Sheet1!$D$2:$D$4. Label berubah saat sel berubah. Kosongkan untuk menghapusnya. Matikan semua isi label dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Kotak ini mengatur nilai terendah dan tertinggi serta satuan utama dan kecil (masing-masing otomatis atau angka yang Anda ketik), apakah nilai berjalan dalam urutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dengan 1.000 untuk ribuan, sehingga 30.000 terbaca 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skala logaritmik (setiap tanda adalah basis dikali tanda sebelumnya, misalnya 1, 10, 100, dan 1000, yang cocok untuk angka yang sangat berbeda ukurannya dan semuanya harus lebih dari 0), tempat sumbu lain memotong sumbu ini, format angka labelnya, tanda sumbunya, letak labelnya, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Format Bagian Bagan…** pada tab **Bagan** (dan pada menu klik kanan) mengatur isian dan garis halaman bagan, area plot (kotak tempat tanda berada), atau satu seri. Untuk garis, pengaturan ini juga menentukan apakah garis dibuat halus dan tanda apa yang berada di setiap titik. Menu klik kanan bagan juga memiliki **Tukar Baris dan Kolom** dan kedua perintah sumbu.
- **Pilihan Saat Ini** menampilkan bagian-bagian bagan: area bagan, area plot, dan setiap seri. Pilih salah satu, lalu **Format Pilihan** membuka jendela format pada bagian tersebut. **Atur Ulang agar Sesuai Gaya** menghapus pengaturan yang dibuat pada bagian yang dipilih dan mengembalikan apa yang diberikan gaya bagan. Untuk area bagan, itu berarti isiannya, garisnya, dan warna teks yang diberikan oleh sebuah tampilan (teks terang pada tampilan gelap kembali ke warna teks tema). Untuk area plot, itu berarti isian dan garisnya. Untuk sebuah seri, itu berarti warnanya (yang diberikan oleh pilihan Warna atau gaya bagan), garis tepinya, tandanya seperti yang dimiliki jenis bagan tersebut, dan warna titik-titiknya; garis halus tetap ada jika jenis bagannya halus. Perintah ini tidak mengubah judul, legenda, label, sumbu, garis kisi, atau data, dan tidak berpengaruh pada bagian yang sudah sesuai. Perintah ini adalah satu langkah Urungkan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan di mana bagan sebaiknya diletakkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan dipindahkan ke lembar kerja itu. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkannya akan dihapus. Bagan tetap membaca sel yang dibacanya sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan posisinya di lembar, meskipun baris dan kolomnya lebih tinggi atau lebih lebar, dan ikut bergerak serta berubah ukuran bersama sel di sana seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini adalah satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika sebuah lembar dihapus olehnya. Berkas berbentuk `.ods` tidak dapat menampung lembar bagan, jadi simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (pada menu **+**) juga memiliki **Teks Judul…**. Ketik kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Saat sel berubah, judul ikut berubah. Berkas menyimpan tautannya.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada ukurannya di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan sebuah bagan dalam berkas **.crtx** dan memberikannya kepada bagan lain. Tampilan itu mencakup jenis, warna, pengaturan judul dan legenda, sumbu, serta font. Bagan yang menerimanya tetap memakai angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus tampilan tersebut.
- **Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau sebuah seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di antara tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan kecil yang muat di dalam satu sel. Sparkline menampilkan tren sebuah baris atau kolom angka tepat di sebelah angka tersebut.

1. Pilih angkanya. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya berukuran sama, yang cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca semua data.

Sparkline yang Anda buat dalam satu langkah membentuk sebuah grup dan memakai satu tampilan bersama. Klik sel dengan sparkline, maka tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh grup, atau data sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (membiarkan celah, menganggapnya nol, atau menghubungkan titik melewatinya) dan apakah sel di baris atau kolom tersembunyi ikut dibaca.
- **Jenis** beralih di antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi grup satu set warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garis, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis pada nol, menggambar angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh grup satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** menjarakkan titik sesuai tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal lebih baru akan berada lebih jauh. Kosongkan untuk menjarakkan titik secara merata lagi.
- **Kelompokkan** memasukkan sparkline yang dipilih ke dalam satu grup. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh grupnya.

Sparkline mengikuti selnya: sisipkan baris di atas data, dan sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan cara yang dibaca oleh program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka jendela pemilih berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG), dan gambar akan muncul di lembar pada sel yang dipilih. Gambar yang lebarnya lebih dari 480 titik diperkecil agar muat, dan bentuknya tetap sama. Buku kerja menyimpan gambar itu sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (misalnya SVG) disimpan sebagai PNG, yaitu gambar biasa yang tersusun dari titik. Berkas yang bukan gambar, atau yang lebih besar dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat sebagian program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi kumpulan gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau beberapa, dan masing-masing muncul di lembar sebagai gambar selebar dan setinggi 48 poin. Gambar digambar dengan warna abu-abu tua dalam ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka menu pendek dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angkanya, dan jendela menampilkan kode batang saat Anda mengetik. Jika kode tidak dapat dibuat, jendela memberi tahu dengan tulisan merah apa yang salah. Klik **Sisipkan**, dan kode batang akan muncul di lembar sebagai gambar, yang dapat Anda urungkan dalam satu langkah. Pindahkan dan ubah ukurannya seperti gambar lainnya. Untuk mengubahnya, pilih kode batang itu lalu klik **Kode Batang** lagi, ubah teksnya, dan klik **Perbarui**; kode batang baru menggantikan tempat dan lebar yang lama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam, sehingga setiap program dapat menampilkannya. JeffCrunch juga menyimpan pengaturannya dalam berkas, sehingga kode batang tetap dapat diedit saat Anda membuka buku kerja lagi di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel berisi pada sel yang dipilih, misalnya untuk lembar label. Pilih sel, klik perintah itu, lalu pilih jenis dan tampilannya pada jendela yang sama. Apa yang Anda ketik di sana hanya contoh, karena teks setiap sel itulah yang menjadi kode batangnya. Kode batang diletakkan dalam sebuah kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat Anda urungkan. Sel yang teksnya tidak dapat ditampung kode (misalnya huruf pada EAN-13) disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel dikerjakan sekaligus; pilih lebih sedikit jika ada pesan bahwa jumlahnya terlalu banyak. Setiap kode batang mengingat sel asalnya dan mengikutinya: saat Anda mengubah sel itu, kode batang dibuat ulang dari teks yang baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan ikut berpindah bersama sel saat Anda menyisipkan atau menghapus baris atau kolom. Jika Anda menghapus sel, kode batang tetap memakai teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel pada saat itu, sehingga program lain juga menampilkan nilai terkini.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, oval, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih salah satu dan bentuk itu muncul di lembar dengan warna biru muda dan tepi yang lebih gelap. **Kotak Teks**, di grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi. Klik **Gambar Kotak Teks** (tombol utama, atau pilihan pertama pada menunya), lalu seret di lembar untuk menggambar kotak. Seretan membuat kotak sebesar yang Anda seret, tetapi tidak pernah kurang dari 18 poin (0,64 cm) pada setiap sisinya, dan klik kurang dari 4 poin menghasilkan ukuran biasa. Tahan **Shift** untuk membuat kotak tetap persegi. Penunjuk berbentuk silang sampai Anda melepas tombol; **Esc**, atau perintah lain, menghentikan alat ini. Kotak itu adalah satu langkah yang dapat Anda urungkan.

Untuk memutar teks dalam kotak teks, pilih kotak itu lalu klik **Arah Teks** pada tab **Format Bentuk**: **Horizontal**, **Putar semua teks 90°**, atau **Putar semua teks 270°**. Teks diketik sudah dalam posisi diputar, seperti yang ditampilkan, dan buku kerja yang disimpan sebagai berkas .xlsx atau .ods mempertahankan arah tersebut.

Dengan sebuah bentuk dipilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru yang lebih gelap. Dua pilihan terakhir hanya mempertahankan garis tepinya atau hanya isiannya. Satu klik adalah satu langkah yang dapat Anda urungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis secara daring

**Gambar Daring**, pada grup **Ilustrasi** tab **Sisipkan**, menemukan gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik sebuah gambar untuk melihat siapa pembuatnya dan apa lisensinya. **Lisensi** menyatakan bagaimana Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik dua kali gambarnya).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya saat Anda mengklik **Cari** atau menekan Enter. Setelah itu JeffCrunch meminta pustaka untuk pratinjau kecil dari hasil yang ditemukan, serta satu gambar yang Anda sisipkan. Seperti halaman web mana pun, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch sedang memintanya. Tidak ada yang dari buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar, berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya disisipkan sebagai satu langkah, sehingga **Urungkan** menarik keduanya kembali. Pastikan lisensinya sesuai dengan cara Anda akan memakai gambar itu.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf berwarna, dengan garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Kata "Teks Anda di Sini" muncul di lembar dengan gaya itu.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkung. "Polos" membuatnya lurus kembali.

Seni Teks adalah bentuk dalam gambar lembar, dengan lengkung, isian, dan garis tepinya, sehingga tampilannya tetap sama saat Anda menyimpan buku kerja sebagai .xlsx dan membukanya lagi, di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Buku kerja yang disimpan sebagai .ods tetap menyimpan gambar, bentuk, kotak teks, grup, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungnya, lalu kembali sebagai Seni Teks saat berkas .ods dibuka lagi. Beberapa lengkung yang tidak punya bentuk padanan di OpenDocument (Gelombang 2, Gelombang 4, dan bentuk bawah serta atas dari Mengembung dan Mengempis) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada yang tampil di lembar, dan membuat berkas menjadi besar. Klik sebuah gambar, lalu klik **Kompres Gambar…** pada grup **Sesuaikan** tab **Format Gambar**. Pilih tempat buku kerja akan dipakai: **Cetak komersial (300 ppi)**, **Cetak desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar pada lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghilangkan bagian tersembunyi yang Anda potong, sehingga pemotongannya hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti semula. Urungkan (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya memuat gambar yang lebih kecil. Gambar pada lembar lain tidak berubah: buka setiap lembar dan ulangi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah adalah satu tindakan yang dapat Anda urungkan. Objek baru yang dihasilkan langsung terpilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau ke belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Menampilkan teks sel dalam bentuk atau kotak teks

Sebuah bentuk atau kotak teks dapat menampilkan isi sebuah sel dan mengikutinya. Pilih bentuk, klik **bilah rumus**, ketik tanda sama dengan dan sebuah sel, lalu tekan **Enter**.

- `=B2` menautkan bentuk ke B2 pada lembar ini. `=Sheet2!B3` menautkannya ke sel pada lembar lain. `=Total` menautkannya ke nama yang mewakili satu sel.
- Bentuk kemudian menampilkan teks sel seperti yang ditampilkan sel, dengan format angkanya. Saat Anda mengubah sel, formatnya, atau rumus yang bergantung padanya, bentuk ikut berubah. Teks yang lebih panjang dari yang dapat ditampung sel (32.767 karakter) akan dipotong. Semua bentuk tertaut dalam sebuah buku kerja bersama-sama menampilkan paling banyak 4 MB teks: lebih dari itu, sebuah bentuk menampilkan teksnya yang dipotong.
- Dengan bentuk dipilih, bilah rumus menampilkan tautannya, seperti `=Sheet1!$B$2`. Kosongkan bilah rumus lalu tekan **Enter** untuk menghapus tautan. Bentuk kemudian mempertahankan kata-kata yang ditampilkannya.
- Mengetik kata-kata di dalam bentuk juga menghapus tautan.
- Tautan hanya untuk satu sel. Rentang seperti `=A1:B2`, daftar, perhitungan, atau nama yang bukan sel akan ditolak dengan pesan, dan bentuk tetap mempertahankan tautannya.
- Jika baris atau kolom sel dihapus, atau lembarnya dihapus, bentuk menampilkan `#REF!` dan tetap menyimpan tautannya, seperti rumus. Urungkan mengembalikan sel tersebut.
- Setiap tautan, dan setiap kali tautan dihapus, adalah satu langkah yang dapat Anda urungkan.

Tautan disimpan dalam berkas .xlsx bersama teks yang ditampilkan sel, sehingga program yang tidak mengikuti tautan tetap menampilkan kata-katanya, dan JeffCrunch membuka berkas dengan tautan yang berfungsi. OpenDocument tidak memiliki cara untuk menautkan kotak teks ke sel, sehingga buku kerja yang disimpan sebagai .ods menyimpan kata-kata yang ditampilkan bentuk pada saat itu, bukan tautannya.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih sebuah gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek tetap mempertahankan namanya, teks alternatifnya, tempatnya, serta urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garis) dan garis putus-putusnya (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, lebar 4 tinggi 3, lebar 3 tinggi 4, 16 kali 9, atau 9 kali 16. Pemotongan dilakukan merata dari tengah bagian yang terlihat, dan gambar diperbesar sebesar sisa yang tertinggal. **Tampilkan Seluruh Gambar** mengembalikan bagian yang dipotong. **Isi** mempertahankan bingkai dan memotong gambar agar menutupi seluruhnya. **Muat** menampilkan seluruh gambar, dan bingkai mengambil bentuk gambar di dalam bingkai lama. **Potong ke Bentuk** (galeri di samping **Potong**) memotong gambar menjadi bentuk seperti oval, bintang, atau persegi panjang sudut membulat; bentuk itu disimpan bersama gambar. File gambar tidak berubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** berlaku untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih posisinya secara mendatar maupun vertikal, ukurannya, tebal atau biasa, dan apakah kata dibungkus mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, perintah ini meminta kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek pada lembar menurut nama, dengan yang paling depan di atas. Klik sebuah nama untuk memilih objek itu, yang berguna jika objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek saat sel dipindahkan atau diperbesar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang memakai sesuatu selain gambar dari buku kerja, seperti tautan ke berkas, tidak dapat disalin, dan JeffCrunch memberi tahu alasannya.

Salin sebuah bagan dan tempelkan ke buku kerja yang sama, pada lembar mana pun: salinan itu membaca sel yang sama dengan bagan pertama. Tempelkan ke buku kerja lain dan bagan itu tetap menampilkan angka yang ditampilkannya, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkan, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi dengan Shift atau Ctrl untuk melepasnya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** lalu menyeret kotak di atas sel kosong: setiap objek yang seluruhnya berada di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah berikut pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu bergerak dan berubah ukuran sebagai satu kesatuan, dan berkas menyimpannya sebagai grup. Bagan tidak dapat dimasukkan ke dalam grup.
- **Pisahkan Kelompok** memisahkan grup yang dipilih. Setiap bagiannya kembali ke tempatnya semula pada lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek disejajarkan dengan tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi tiga objek atau lebih secara merata di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan sebuah objek dipilih, kotak nama menampilkan namanya. Ketik nama baru di sana lalu tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** saat objek dipilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan cara yang dibaca oleh program lembar kerja lain, sehingga bagan yang Anda buat di sini terbuka dengan cara yang sama di sana. Bagan yang berasal dari program lain dan memuat sesuatu yang tidak dapat dipertahankan JeffCrunch saat menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi unsur serta datanya tidak diubah di sini.

## Menyisipkan objek

Sebuah **objek** menyimpan berkasnya sendiri di dalam buku kerja, sehingga aplikasi yang membuatnya dapat membukanya lagi. Objek menampilkan gambar dirinya sendiri, tetapi bukan gambar biasa: ia tetap menjadi objek saat Anda memindahkannya, mengubah ukurannya, menyalinnya, atau menyimpan buku kerja.

1. Klik **Sisipkan**, lalu **Objek**, dan pilih **Dari Rentang JeffCrunch**, **Dari Bagan JeffCrunch**, atau **Dari Halaman JeffWrite**.
2. Pilih berkasnya, dan rentang atau nomor halaman jika kotak memintanya. Jika Anda sudah menyalinnya di aplikasi lain, kotak akan menawarkan untuk memakainya.
3. Objek disisipkan pada sel yang dipilih. Seret untuk memindahkannya, dan seret salah satu pegangan untuk mengubah ukurannya. Rentang, bagan, atau halaman yang ukurannya Anda ubah akan digambar ulang pada ukuran barunya, sehingga tetap tajam. Tekan **Delete** untuk menghapusnya. Setiap langkah adalah satu tindakan Urungkan.

Untuk menyisipkan apa yang Anda salin di aplikasi lain, klik panah di bawah **Tempel** dan pilih **Sebagai Objek**.

**Klik dua kali** objek untuk membukanya di aplikasi yang membuatnya. Simpan di sana, dan objek di sini menampilkan perubahannya sebagai satu langkah Urungkan. Sel yang sedang Anda ketik tetap apa adanya. Salinan yang Anda tempel adalah objek tersendiri: mengubah salah satunya tidak mengubah yang lain. Hal yang sama berlaku untuk lembar yang Anda salin: objek di dalamnya ikut disalin, dan salinan lembar itu menyimpan objek-objek tersebut.

**Berkas**, lalu **Info**, lalu **Objek** mendaftar setiap objek dalam buku kerja. Pilih salah satu untuk membukanya, mengubahnya menjadi gambar biasa, atau menghapusnya.

Objek yang dibuat di program lain tampil sebagai gambar yang disimpan berkasnya, apa pun jenis gambar itu. Anda dapat memilihnya, memindahkannya, mengubah ukurannya, merapikannya dengan objek lain, dan menghapusnya; objek itu bergerak bersama sel seperti objek Anda sendiri. Anda tidak dapat mengeditnya di sini, memasukkannya ke dalam grup, atau mengubah urutannya di antara objek. Objek disimpan persis seperti berkas aslinya, dengan hanya posisinya yang berubah jika Anda memindahkannya. Objek yang disimpan dalam berkas .ods diperlakukan dengan cara yang sama. Saat disimpan sebagai berkas .xlsx atau .ods, objek Anda berada dalam bentuk standar yang dibaca program lain. Saat berkas .ods ditulis untuk objek yang gambarnya tidak dapat digambar, objek itu disimpan tanpa gambar.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, misalnya sebuah proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sebelahnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Saat Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam grup. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, lingkaran, donat, bagan sebaran, dan bagan radar, masing-masing dengan seri serta sel yang dibacanya (pada lembar ini atau lembar lain), kategori, judul, legenda, sumbu (judul, nilai terkecil dan terbesar, format angka), label data, warna, dan tempatnya. Berkas .ods yang dibuat program lain dibuka beserta bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat dalam bagan .ods akan dihilangkan dan disebutkan namanya saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, dan tampilan khusus untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
