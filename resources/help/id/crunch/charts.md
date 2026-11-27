# Bagan, sparkline, gambar, dan bentuk
<!-- keywords: objek tersemat, sisipkan objek, objek dari rentang, objek dari bagan, objek dari halaman, tempel sebagai objek, daftar objek, edit objek, tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, lingkaran, area, sebar, kombinasi, radar, radar terisi, bagan laba-laba, legenda, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, teks alternatif, logaritmik, skala log, simpan sebagai gambar, templat, crtx, teks judul, kunci legenda, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, oval, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area yang dipotong, kode batang, kode QR, tautkan bentuk ke sel, bentuk menampilkan sel, kotak teks menampilkan sel, nilai sel dalam bentuk -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ketika angka diubah, bagan ikut berubah. Gambar dan bentuk ditempatkan di atas sel dengan cara yang sama, tetapi keduanya tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Lingkaran**, **Area**, **Sebar**, **Kombinasi**, **Radar**, atau **Saham**. Tanda panah kecil di sebelah jenis memuat variasinya, seperti kolom bertumpuk, garis dengan penanda, bagan sebar berupa titik, titik tersambung, atau garis halus, atau bagan radar yang polos, bertanda, atau terisi. Bagan radar menggambar setiap label sebagai jari-jari dari pusat dan setiap seri sebagai bentuk tertutup yang melalui angkanya pada jari-jari, sehingga beberapa ukuran dari satu hal dapat dibandingkan berdampingan.
   Bagan **saham** menampilkan harga: sebuah garis untuk setiap kategori dari harga tertinggi hingga terendah, dengan titik kecil untuk harga penutupan, atau, jika ada harga pembukaan, sebuah batang di antara pembukaan dan penutupan (terang untuk kenaikan, gelap untuk penurunan). Bagan dibuat dari kolom Anda persis seperti urutannya, dari kiri ke kanan, dan tidak ada pemeriksaan apakah angkanya sesuai dengan posisinya, jadi urutkan sebagai: tertinggi, terendah, dan penutupan; pembukaan, tertinggi, terendah, dan penutupan; volume, tertinggi, terendah, dan penutupan; atau volume, pembukaan, tertinggi, terendah, dan penutupan. Jika kolom berada dalam urutan lain, bagan tetap digambar, tetapi tandanya memiliki arti yang berbeda (titik penutupan berada pada kolom harga ketiga, dan kotak membentang dari kolom harga pertama hingga terakhir). Dengan volume, volume dibaca pada sumbu kiri dan harga dibaca pada sumbu kedua di sebelah kanan. Jika jumlah kolom tidak sesuai dengan jenisnya, JeffOffice memberi tahu berapa kolom yang dibutuhkan dan tidak membuat bagan.
3. Untuk melihat setiap jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Tidak yakin jenis mana yang harus dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel dan mendaftar beberapa jenis yang cocok, yang terbaik di urutan pertama. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan lingkaran. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi sesuatu.

Seri berjalan ke bawah kolom jika pilihan memiliki baris setidaknya sebanyak kolom, dan berjalan ke samping sepanjang baris jika memiliki lebih banyak kolom daripada baris. Lingkaran memuat satu seri. Bagan sebar mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi Timur dan Barat, dan sisanya adalah angka. Bagan memiliki dua seri, Timur dan Barat, serta tiga kelompok kolom, Q1 hingga Q3.

## Sembunyikan seri untuk sementara

Pilih sebuah bagan. Tiga tombol kecil muncul di sebelahnya. Yang bergambar corong adalah **Filter Bagan**. Tombol ini mendaftar seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang lagi untuk memunculkannya kembali di tempat yang sama. **Urungkan** juga berfungsi. Bagan selalu mempertahankan setidaknya satu seri. Seri yang Anda keluarkan tetap ada dalam berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang lagi untuk memunculkannya kembali. (Bagan donat menampilkan setiap seri saat dibuka.)

## Memindahkan dan mengubah ukurannya

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkannya, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar garis tepi sejajar dengan tepi sel.
- **Tarik ke Kisi** dan **Tarik ke Bentuk** ada pada tab **Format Bentuk**, dan disimpan untuk Anda, untuk setiap buku kerja. Saat **Tarik ke Kisi** aktif, tepi yang Anda seret berpindah ke tepi sel terdekat. Saat **Tarik ke Bentuk** aktif, tepi berpindah ke tepi objek lain jika berjarak tidak lebih dari 6 piksel. Hanya 2.000 objek lain pertama pada lembar yang dihitung untuk **Tarik ke Bentuk**. Jika keduanya aktif, tepi sel didahulukan, karena objek ditambatkan ke sel, dan tepi objek lain tidak diperiksa. Perubahan langsung berlaku di setiap jendela yang terbuka.
- **Alt** membalik penarikan yang aktif untuk satu seretan: jika **Tarik ke Kisi** atau **Tarik ke Bentuk** aktif, menahan **Alt** mematikan penarikan untuk seretan itu (tidak pernah beralih ke jenis lainnya), dan jika keduanya nonaktif, **Alt** membuat tepi berpindah ke sel, seperti di atas.
- Objek yang diputar ditarik menurut garis luar yang Anda lihat (kotak terkecil yang memuatnya), bukan menurut bingkainya sebelum diputar, begitu pula objek yang ditariknya.
- Tahan **Shift** saat menyeret sudut untuk menjaga proporsi.
- Tombol panah menggeser objek beberapa piksel setiap kali. **Ctrl** dengan panah menggeser satu piksel, **Shift** dengan panah membuatnya lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat diurungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar memerlukan nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), untuk mengetik ukurannya, dan untuk memilih apa yang terjadi saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berubah ukuran bersama sel, bergerak bersama sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan itu disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman cetakan (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah tampilan bagan

Saat sebuah bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul di pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul, sumbu, judul sumbu, garis kisi, legenda, dan label data bagan, serta menentukan di mana legenda dan label diletakkan.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** mendaftar seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca masing-masing seri, mengatur sel dengan label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi dibuat bagannya.
- **Tombol Bidang** (di menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang dari tabel pivot yang diikuti bagan: filter laporan di bawah judul, dan baris, nilai, serta kolom di sepanjang bagian bawah. Aktifkan atau nonaktifkan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (di menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Simpangan Standar**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawahnya. Biarkan salah satu kosong untuk menggambar bilah hanya di satu sisi. Bilah berubah saat sel tersebut berubah. Berkas yang memiliki jumlah sendiri untuk setiap titik tetap menyimpannya.
- **Garis** (di menu **+**, untuk bagan garis) memiliki **Garis Turun**, yang membentang dari titik tertinggi setiap kategori ke bawah hingga sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Batang Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Batang naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Batang Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Turun.
- **Tabel Data** (di menu **+**, untuk bagan kolom, garis, atau area) menampilkan kisi angka di bawah bagan, dengan kolom untuk setiap kategori dan baris untuk setiap seri. Daftarnya menyalakan atau mematikan tabel, serta mengatur garis di antara baris, garis di antara kolom, garis tepi, dan kunci legenda (warna setiap seri di sebelah namanya).
- **Garis Tren** (di menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat ke mana arahnya. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase yang tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menulis rumus garis pada bagan, dan **Tampilkan R-kuadrat** menulis seberapa cocok garis itu (1 berarti sangat cocok; 0 berarti tidak cocok sama sekali). Garis tren mendapat entri sendiri di legenda.
- Saat sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Legenda** memiliki **Di Atas Bagan**, yang meletakkan legenda di atas area plot, bukan di sebelahnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci legenda, yaitu kotak kecil berwarna seri, dalam kombinasi apa pun), **Pemisah** (pemisah di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0,0 untuk angka dalam label; kosongkan untuk menampilkan angka apa adanya). **Nilai dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel-selnya, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel berubah. Kosongkan untuk menghapusnya. Matikan semua isi label dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Kotak ini mengatur nilai terendah dan tertinggi serta satuan utama dan kecil (masing-masing otomatis atau angka yang Anda ketik), apakah nilai berurutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 terbaca sebagai 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skala logaritmik (setiap tanda adalah basis dikali tanda sebelumnya, seperti 1, 10, 100, dan 1000, yang cocok untuk angka yang sangat berbeda ukurannya dan semuanya harus lebih dari 0), di mana sumbu lain memotongnya, format angka labelnya, tanda ticknya, letak labelnya, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Format Bagian Bagan…** pada tab **Bagan** (dan pada menu klik kanan) mengatur isian dan garis tepi halaman bagan, area plot (kotak tempat tanda berdiri), atau satu seri. Untuk garis, juga diatur apakah garis halus dan tanda apa yang berdiri di setiap titik. Menu klik kanan bagan juga memiliki **Tukar Baris dan Kolom** dan dua perintah sumbu.
- **Pilihan Saat Ini** mendaftar bagian-bagian bagan: area bagan, area plot, dan setiap seri. Pilih salah satu, lalu **Format Pilihan** membuka jendela format pada bagian tersebut. **Atur Ulang agar Sesuai Gaya** menghapus apa yang diatur pada bagian yang dipilih dan mengembalikan apa yang diberikan gaya bagan kepadanya. Untuk area bagan, itu adalah isian, garisnya, dan warna teks yang diatur sebuah tampilan (teks terang dari tampilan gelap kembali ke warna teks tema). Untuk area plot, itu adalah isian dan garisnya. Untuk seri, itu adalah warnanya (yang diberikan pilihan Warna atau gaya bagan), garis tepinya, penandanya sesuai jenis bagan, dan warna titik-titiknya; garis halus tetap ada jika jenis bagannya halus. Perintah ini tidak mengubah judul, legenda, label, sumbu, garis kisi, atau data, dan tidak mengubah bagian yang sudah sesuai. Ini adalah satu langkah Urungkan.
- **Pindahkan Bagan…** pada tab **Bagan** bertanya ke mana bagan harus dipindahkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapatkan lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan dipindahkan ke lembar kerja itu. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkan akan dihapus. Bagan tetap membaca sel yang dibacanya. Pada lembar kerja, bagan mempertahankan ukuran dan tempatnya pada lembar, bahkan jika baris dan kolomnya lebih tinggi atau lebih lebar, dan ikut bergerak serta berubah ukuran bersama sel di sana seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini adalah satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika sebuah lembar dihapus olehnya. Berkas dengan bentuk `.ods` tidak dapat memuat lembar bagan, jadi simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (di menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, dan judul menampilkan isi sel tersebut. Saat sel berubah, judul ikut berubah. Berkas menyimpan tautannya.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya ke bagan lain. Tampilan itu meliputi jenis, warna, pengaturan judul dan legenda, sumbu, dan font. Bagan yang menerimanya tetap memakai angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus tampilan itu lagi.
- **Ubah Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di antara tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan mini yang muat di dalam satu sel. Sparkline menunjukkan tren dari satu baris atau satu kolom angka tepat di sebelahnya.

1. Pilih angkanya. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya berukuran sama, yang cocok untuk hasil yang hanya baik atau buruk.
3. Di kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca semua data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok, dan berbagi satu tampilan. Klik sel dengan sparkline, dan tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data sparkline yang Anda klik. Juga diatur apa yang dilakukan pada sel kosong (membiarkan celah, menganggapnya nol, atau menyambungkan titik di seberangnya) dan apakah sel di baris atau kolom tersembunyi dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garisnya, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, memplot angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** menjarakkan titik menurut tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal yang lebih akhir berada lebih jauh. Kosongkan untuk menjarakkan titik secara merata lagi.
- **Kelompokkan** menempatkan sparkline yang dipilih dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: sisipkan baris di atas data, dan sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan cara yang dibaca oleh program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka pemilih berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG), maka gambar muncul di lembar pada sel yang dipilih. Gambar yang lebih lebar dari 480 titik diperkecil agar muat, dan tetap mempertahankan bentuknya. Buku kerja menyimpan gambar itu sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (SVG, misalnya) disimpan sebagai PNG, yaitu gambar biasa yang terdiri dari titik. Berkas yang bukan gambar, atau yang lebih besar dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi gambar garis kecil yang juga dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau beberapa, dan masing-masing muncul di lembar sebagai gambar, lebar dan tinggi 48 poin. Ikon digambar dengan warna abu-abu tua pada ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angkanya, dan jendela menampilkan kode batang saat Anda mengetik serta memberi tahu dengan tulisan merah jika ada yang salah sehingga kode tidak bisa dibuat. Klik **Sisipkan**, dan kode batang muncul di lembar sebagai gambar, satu langkah yang dapat Anda urungkan. Pindahkan dan ubah ukurannya seperti gambar mana pun. Untuk mengubahnya, pilih kode batang itu dan klik **Kode Batang** lagi, ubah teksnya, lalu klik **Perbarui**; kode batang yang baru menggantikan tempat dan lebar yang lama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam, sehingga setiap program menampilkannya, dan JeffCrunch menyimpan pengaturannya dalam berkas, jadi kode batang itu tetap dapat diedit saat Anda membuka kembali buku kerja di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel yang berisi pada sel yang dipilih, untuk lembar label. Pilih sel, klik perintah itu, lalu pilih jenis dan tampilannya di jendela yang sama. Apa pun yang Anda ketik di sana hanya contoh, karena teks setiap sel sendiri menjadi kode batangnya. Kode batang diletakkan dalam sebuah kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat diurungkan. Sel yang teksnya tidak dapat ditampung kode (misalnya huruf pada EAN-13) disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel diproses sekaligus; pilih lebih sedikit jika ada pesan bahwa jumlahnya terlalu banyak. Setiap kode batang mengingat sel asalnya dan mengikutinya: saat Anda mengubah sel itu, kode batang dibuat ulang dari teks yang baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan ikut berpindah bersama sel saat Anda menyisipkan atau menghapus baris atau kolom. Jika Anda menghapus sel, kode batang tetap menyimpan teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel sebagaimana adanya saat itu, sehingga program lain juga menampilkan nilai terkini.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, oval, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih satu dan bentuk itu muncul di lembar dengan warna biru muda dan tepi biru tua. **Kotak Teks**, di grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi. Klik **Gambar Kotak Teks** (tombol utama, atau pilihan pertama di menunya), lalu seret di lembar untuk menggambar kotaknya. Seretan membuat kotak dengan ukuran yang Anda seret, tetapi tidak pernah kurang dari 18 poin (0,6 cm) pada setiap sisi, dan klik kurang dari 4 poin memberi ukuran biasa. Tahan **Shift** untuk menjaga kotak tetap persegi. Penunjuk berbentuk silang sampai Anda melepas tombol; **Esc**, atau perintah lain, menonaktifkan alat ini. Kotak itu adalah satu langkah yang dapat diurungkan.

Untuk memutar teks dalam kotak teks, pilih kotak itu lalu klik **Arah Teks** pada tab **Format Bentuk**: **Horizontal**, **Putar semua teks 90°**, atau **Putar semua teks 270°**. Teks diketik dalam keadaan diputar, sebagaimana ditampilkan, dan buku kerja yang disimpan sebagai berkas .xlsx atau .ods mempertahankan arahnya.

Saat sebuah bentuk dipilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru tua. Dua pilihan terakhir hanya mempertahankan garis tepinya atau hanya isiannya. Satu klik adalah satu langkah yang dapat Anda urungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis di internet

**Gambar Daring**, di grup **Ilustrasi** pada tab **Sisipkan**, mencari gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik apa yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik gambar untuk melihat siapa pembuatnya dan apa lisensinya. **Lisensi** menyatakan bagaimana Anda boleh memakai gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik dua kali gambarnya).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya saat Anda mengklik **Cari** atau menekan Enter. Setelah itu JeffCrunch meminta pratinjau kecil dari hasil yang ditemukan, dan satu gambar yang Anda sisipkan. Seperti halaman web mana pun, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang meminta. Tidak ada yang dari buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Saat **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar, berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya dimasukkan sebagai satu langkah, sehingga **Urungkan** menarik keduanya kembali. Pastikan lisensinya sesuai dengan cara Anda akan memakai gambar itu.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf dengan warna, garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Tulisan "Teks Anda di Sini" muncul di lembar dengan gaya itu.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-katanya, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkung. "Biasa" membuatnya lurus kembali.

Seni Teks adalah bentuk dalam gambar lembar, dengan lengkung, isian, dan garis tepinya, sehingga tampilannya tetap sama saat Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya kembali, di JeffCrunch atau di program lain. Setiap perubahan adalah satu langkah yang dapat diurungkan. Buku kerja yang disimpan sebagai berkas .ods juga mempertahankan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungnya, dan kembali sebagai Seni Teks saat berkas .ods dibuka lagi. Beberapa lengkung yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, dan bentuk bawah serta atas dari Menggelembung dan Mengempis) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods; lihat di bawah.

## Memperkecil gambar

Gambar dari kamera dapat jauh lebih besar daripada yang tampak di lembar, dan membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** di grup **Sesuaikan** pada tab **Format Gambar**. Pilih tempat buku kerja akan dipakai: **Cetak komersial (300 ppi)**, **Cetak desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar pada lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghapus bagian tersembunyi yang Anda potong, sehingga potongannya hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti semula. Urungkan (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya memuat gambar yang lebih kecil. Gambar pada lembar lain tidak diubah: buka setiap lembar dan lakukan lagi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah adalah satu langkah yang dapat Anda urungkan. Objek baru terpilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Menampilkan teks sel dalam bentuk atau kotak teks

Bentuk atau kotak teks dapat menampilkan isi sebuah sel, dan mengikutinya. Pilih bentuknya, klik **bilah rumus**, ketik tanda sama dengan dan sebuah sel, lalu tekan **Enter**.

- `=B2` menautkan bentuk ke B2 pada lembar ini. `=Sheet2!B3` menautkannya ke sel pada lembar lain. `=Total` menautkannya ke nama yang mewakili satu sel.
- Bentuk kemudian menampilkan teks sel sebagaimana sel menampilkannya, dengan format angkanya. Saat Anda mengubah sel, formatnya, atau rumus yang bergantung padanya, bentuk ikut berubah. Teks yang lebih panjang dari yang dapat ditampung sel (32.767 karakter) dipotong. Semua bentuk tertaut dalam sebuah buku kerja bersama-sama menampilkan paling banyak 4 MB teks: lebih dari itu, sebuah bentuk menampilkan teksnya yang dipotong.
- Saat bentuk dipilih, bilah rumus menampilkan tautannya, seperti `=Sheet1!$B$2`. Kosongkan bilah rumus lalu tekan **Enter** untuk menghapus tautan. Bentuk kemudian mempertahankan kata-kata yang ditampilkannya.
- Mengetik kata-kata di dalam bentuk juga menghapus tautan.
- Tautan hanya untuk satu sel. Rentang seperti `=A1:B2`, daftar, perhitungan, atau nama yang bukan sel ditolak dengan pesan, dan bentuk tetap mempertahankan tautannya.
- Jika baris atau kolom sel dihapus, atau lembarnya, bentuk menampilkan `#REF!` dan tetap mempertahankan tautan, seperti rumus. Urungkan mengembalikan sel.
- Setiap tautan, dan setiap kali Anda menghapus tautan, adalah satu langkah yang dapat diurungkan.

Tautan disimpan dalam berkas .xlsx bersama teks yang ditampilkan sel, sehingga program yang tidak mengikuti tautan tetap menampilkan kata-katanya, dan JeffCrunch membuka berkas dengan tautan yang berfungsi. OpenDocument tidak memiliki cara untuk menautkan kotak teks ke sel, jadi buku kerja yang disimpan sebagai berkas .ods mempertahankan kata-kata yang ditampilkan bentuk pada saat itu, bukan tautannya.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek mempertahankan namanya, teks alternatifnya, tempatnya, serta urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garisnya) dan garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, 4 lebar kali 3 tinggi, 3 lebar kali 4 tinggi, 16 kali 9, atau 9 kali 16. Pemotongan dilakukan secara merata dari tengah bagian yang tampak, dan gambar dibuat sebesar mungkin dari sisa yang ada. **Tampilkan Seluruh Gambar** mengembalikan bagian yang dipotong. **Isi** mempertahankan bingkai dan memotong gambar agar menutupi seluruhnya. **Muat** menampilkan seluruh gambar, dan bingkai mengambil bentuk gambar di dalam bingkai lama. **Potong ke Bentuk** (galeri di sebelah **Potong**) memotong gambar menjadi bentuk seperti oval, bintang, atau persegi panjang sudut membulat; bentuk itu disimpan bersama gambar. Berkas gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** berlaku untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih bagaimana kata-kata itu diletakkan secara mendatar dan tegak, ukurannya, tebal atau biasa, dan apakah kata-kata membungkus mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, kotak ini meminta kata-katanya, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek pada lembar menurut namanya, dengan yang paling depan di atas. Klik sebuah nama untuk memilih objek itu, yang membantu saat objek saling menutupi. **Bawa ke Depan** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek saat sel dipindahkan atau dibuat lebih besar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang memakai sesuatu selain gambar dari buku kerja, seperti tautan ke berkas, tidak dapat disalin, dan JeffCrunch memberi tahu alasannya.

Salin bagan dan tempelkan ke buku kerja yang sama, di lembar mana pun: salinannya membaca sel yang sama dengan yang pertama. Tempelkan ke buku kerja lain dan bagan tetap memuat angka yang ditampilkannya, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkannya, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik lagi objek yang sudah dipilih dengan Shift atau Ctrl untuk melepasnya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** dan menyeret kotak di atas sel kosong: setiap objek yang seluruhnya berada di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Dengan dua objek atau lebih terpilih, tab **Format Bentuk** memiliki pilihan berikut pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu bergerak dan berubah ukuran sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memisahkan kelompok yang dipilih. Setiap bagian kembali ke tempatnya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek dirapikan dengan tepi atau tengah dari seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi tiga objek atau lebih secara merata di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan sebuah objek terpilih, kotak nama menampilkan namanya. Ketik nama baru di sana dan tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus objek

Tekan **Delete** dengan objek terpilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan bentuk yang dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini terbuka di sana dengan cara yang sama. Bagan yang berasal dari program lain dengan sesuatu yang tidak dapat dipertahankan JeffCrunch saat menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen dan datanya tidak diubah di sini.

## Menyisipkan objek

Sebuah **objek** menyimpan berkasnya sendiri di dalam buku kerja, sehingga aplikasi yang membuatnya dapat membukanya kembali. Objek menampilkan gambar dari dirinya sendiri, tetapi bukan gambar biasa: ia tetap menjadi objek saat Anda memindahkannya, mengubah ukurannya, menyalinnya, atau menyimpan buku kerja.

1. Klik **Sisipkan**, lalu **Objek**, dan pilih **Dari Rentang JeffCrunch**, **Dari Bagan JeffCrunch**, atau **Dari Halaman JeffWrite**.
2. Pilih berkasnya, dan rentang atau nomor halaman jika kotak memintanya. Jika Anda sudah menyalinnya di aplikasi lain, kotak menawarkan untuk memakainya.
3. Objek dimasukkan pada sel yang dipilih. Seret untuk memindahkannya, dan seret pegangan untuk mengubah ukurannya. Rentang, bagan, atau halaman yang ukurannya Anda ubah digambar ulang pada ukuran barunya, sehingga tetap tajam. Tekan **Delete** untuk menghapusnya. Setiap langkah adalah satu langkah Urungkan.

Untuk menyisipkan apa yang Anda salin di aplikasi lain, klik panah di bawah **Tempel** dan pilih **Sebagai Objek**.

**Klik dua kali** objek untuk membukanya di aplikasi yang membuatnya. Simpan di sana, dan objek di sini menampilkan perubahannya sebagai satu langkah Urungkan. Sel yang sedang Anda ketik tetap apa adanya. Salinan yang Anda tempel adalah objek tersendiri: mengubah salah satunya tidak mengubah yang lain. Hal yang sama berlaku untuk lembar yang Anda salin: objeknya adalah salinan, dan salinan lembar tetap memilikinya.

**Berkas**, lalu **Info**, lalu **Objek** mendaftar setiap objek dalam buku kerja. Pilih satu untuk membukanya, mengubahnya menjadi gambar biasa, atau menghapusnya.

Objek yang dibuat di program lain tampil sebagai gambar yang disimpan berkasnya, apa pun jenis gambarnya. Anda dapat memilihnya, memindahkannya, mengubah ukurannya, merapikannya dengan objek lain, dan menghapusnya, dan objek itu bergerak bersama sel seperti objek milik Anda sendiri. Anda tidak dapat mengeditnya di sini, memasukkannya ke dalam kelompok, atau mengubah urutannya di antara objek. Objek disimpan persis seperti berkas aslinya, hanya tempatnya yang berubah jika Anda memindahkannya. Objek yang disimpan dalam berkas .ods juga bekerja dengan cara yang sama. Saat disimpan sebagai berkas .xlsx atau .ods, objek Anda berada dalam bentuk standar yang dibaca program lain. Saat berkas .ods ditulis untuk objek yang gambarnya tidak dapat digambar, objek tersebut disimpan tanpa gambar.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, seperti proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sebelahnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Saat Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar untuk sebuah diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods mempertahankan bagannya: kolom, batang, garis, area, lingkaran, donat, bagan sebar, bagan radar, dan bagan saham high-low-close atau open-high-low-close, masing-masing dengan serinya dan sel yang dibacanya (di lembar ini atau lembar lain), kategori, judul, legenda, sumbu (judul, nilai terkecil dan terbesar, format angka), label data, warna, dan tempatnya. Berkas .ods yang dibuat program lain dibuka beserta bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat dalam bagan .ods akan dihilangkan dan disebutkan namanya saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua (sehingga bagan saham dengan volume disimpan dengan volumenya pada sumbu pertama, dan terbuka sebagai kolom), serta tampilan tambahan pada satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch dibuka sebagai kolom dari nilainya.
