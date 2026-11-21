# Bagan, sparkline, gambar, dan bentuk

<!-- keywords: objek tersemat, sisipkan objek, objek dari rentang, objek dari bagan, objek dari halaman, tempel sebagai objek, daftar objek, edit objek, tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, rekomendasi bagan, saran bagan, bagan, grafik, kolom, batang, garis, pai, area, sebar, kombinasi, legenda, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, garis tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, teks alternatif, logaritmik, skala logaritma, simpan sebagai gambar, templat, crtx, teks judul, kunci legenda, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, oval, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran file, ppi, hapus area yang dipotong, kode batang, kode QR, tautkan bentuk ke sel, bentuk menampilkan sel, kotak teks menampilkan sel, nilai sel dalam bentuk -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ketika angka diubah, bagan ikut berubah. Gambar dan bentuk juga berada di atas sel, tetapi tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Pai**, **Area**, **Sebar**, atau **Kombinasi**. Tanda panah kecil di samping suatu jenis memuat variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebar berupa titik, titik yang disambung, atau garis halus.
3. Untuk melihat semua jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar Baris dan Kolom** membalik arah seri.

Belum yakin jenis mana yang dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel dan menampilkan beberapa jenis yang cocok, dari yang paling sesuai. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan pai. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri disusun menurun pada kolom jika pilihan memiliki jumlah baris setidaknya sama dengan jumlah kolom, dan mendatar pada baris jika pilihan memiliki lebih banyak kolom daripada baris. Pai hanya memiliki satu seri. Bagan sebar mengambil nilai mendatarnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya berupa angka. Bagan memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 sampai Q3.

## Sembunyikan seri untuk sementara

Pilih sebuah bagan. Tiga tombol kecil muncul di sebelahnya. Tombol dengan corong adalah **Filter Bagan**. Tombol ini menampilkan seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang kembali untuk memunculkannya lagi di posisi yang sama. **Urungkan** juga berfungsi. Bagan selalu memiliki setidaknya satu seri. Seri yang Anda keluarkan tetap ada di berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang kembali untuk memunculkannya. (Bagan donat menampilkan semua seri saat dibuka.)

## Memindahkan dan mengubah ukuran

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkannya, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar sisi objek rata dengan tepi sel.
- **Tarik ke Kisi** dan **Tarik ke Bentuk** ada di tab **Format Bentuk**, dan disimpan untuk Anda di setiap buku kerja. Jika **Tarik ke Kisi** aktif, tepi yang Anda seret akan menuju tepi sel terdekat. Jika **Tarik ke Bentuk** aktif, tepi akan menuju tepi objek lain jika jaraknya kurang dari 6 piksel. Hanya 2.000 objek lain pertama pada lembar yang dihitung untuk **Tarik ke Bentuk**. Jika keduanya aktif, tepi sel didahulukan, karena objek terikat pada sel, dan tepi objek lain tidak diperiksa. Perubahan langsung berlaku di setiap jendela yang terbuka.
- **Alt** membalik penarikan yang aktif untuk satu seretan: jika **Tarik ke Kisi** atau **Tarik ke Bentuk** aktif, menahan **Alt** akan mematikan penarikan untuk seretan itu (tidak pernah beralih ke jenis lainnya), dan jika keduanya nonaktif, **Alt** membuat tepi menuju sel, seperti di atas.
- Objek yang diputar ditarik menurut garis luar yang terlihat (kotak terkecil yang memuatnya), bukan menurut bingkai sebelum diputar, begitu pula objek yang menjadi tujuannya.
- Tahan **Shift** saat menyeret sudut untuk menjaga proporsi.
- Tombol panah menggeser objek beberapa piksel sekali. **Ctrl** dengan panah menggeser satu piksel, **Shift** dengan panah membuat objek lebih besar atau lebih kecil, dan **Alt** dengan panah menuju tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat diurungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), mengetik ukurannya, dan memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berubah ukuran bersama sel, bergerak bersama sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman cetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi bagan

Saat sebuah bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul di pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul, sumbu, judul sumbu, garis kisi, legenda, dan label data bagan, serta menentukan posisi legenda dan label.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca setiap seri, menentukan sel berisi label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi dibuat bagannya.
- **Tombol Bidang** (pada menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (pada menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Simpangan Baku**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai nilai Anda sendiri: ketik sel yang berisi nilai di atas setiap titik (seperti `=Sheet1!$D$2:$D$4`), lalu sel untuk nilai di bawahnya. Kosongkan salah satunya untuk menggambar bilah hanya di satu sisi. Bilah berubah ketika sel tersebut berubah. Berkas yang sudah memiliki nilainya sendiri untuk setiap titik tetap menyimpannya.
- **Garis** (pada menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Bilah Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Bilah naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Bilah Naik/Turun membutuhkan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Tabel Data** (pada menu **+**, untuk bagan kolom, garis, atau area) menampilkan kisi angka di bawah bagan, dengan satu kolom untuk setiap kategori dan satu baris untuk setiap seri. Daftarnya menyalakan atau mematikan tabel, serta mengatur garis di antara baris, garis di antara kolom, garis tepi, dan kunci legenda (warna setiap seri di sebelah namanya).
- **Garis Tren** (pada menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat ke mana angka-angka itu menuju. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menulis rumus garis pada bagan, dan **Tampilkan R-kuadrat** menulis seberapa baik garis itu cocok (1 berarti cocok sempurna; 0 berarti tidak cocok sama sekali). Garis tren mendapat entri sendiri di legenda.
- Jika sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Legenda** memiliki **Di Atas Bagan**, yang meletakkan legenda di atas area plot, bukan di sampingnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci legenda, yaitu kotak kecil berwarna seri, dalam kombinasi apa pun), **Pemisah** (yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0,0 untuk angka pada label; kosongkan untuk menampilkan angka apa adanya). **Nilai dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel berubah. Kosongkan untuk menghapusnya. Jika semua isi label dimatikan, label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Di sana Anda dapat mengatur nilai terendah dan tertinggi serta satuan utama dan kecil (masing-masing otomatis atau angka yang Anda ketik), apakah nilai berurutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 terbaca 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skalanya logaritmik (setiap tanda merupakan basis dikali tanda sebelumnya, seperti 1, 10, 100, dan 1000, yang cocok untuk angka yang sangat berbeda ukurannya dan semuanya harus lebih dari 0), tempat sumbu lain memotong sumbu ini, format angka labelnya, tanda skalanya, tempat labelnya berada, dan seberapa jauh labelnya diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Format Bagian Bagan…** pada tab **Bagan** (dan pada menu klik kanan) mengatur isian dan garis tepi area bagan, area plot (kotak tempat tanda-tanda berdiri), atau satu seri. Untuk garis, menu ini juga mengatur apakah garis halus dan tanda apa yang berdiri di setiap titik. Menu klik kanan bagan juga memiliki **Tukar Baris dan Kolom** serta dua perintah sumbu.
- **Pilihan Saat Ini** menampilkan bagian-bagian bagan: area bagan, area plot, dan setiap seri. Pilih salah satu, lalu **Format Pilihan** membuka jendela format pada bagian tersebut. **Atur Ulang agar Sesuai Gaya** menghapus pengaturan yang dibuat pada bagian yang dipilih dan mengembalikan apa yang diberikan gaya bagan. Untuk area bagan, yang dikembalikan adalah isian, garis, dan warna teks yang ditetapkan oleh tampilan (teks terang pada tampilan gelap kembali ke warna teks tema). Untuk area plot, yang dikembalikan adalah isian dan garis. Untuk seri, yang dikembalikan adalah warnanya (yang diberikan oleh pilihan Warna atau gaya bagan), garis tepinya, penandanya sesuai jenis bagan, dan warna titik-titiknya; garis halus tetap halus jika jenis bagannya halus. Perintah ini tidak mengubah judul, legenda, label, sumbu, garis kisi, atau data, dan tidak berpengaruh pada bagian yang sudah sesuai. Ini adalah satu langkah Urungkan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan tempat bagan akan dipindahkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** lalu pilih lembar kerja, maka bagan dipindahkan ke lembar kerja itu. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkan akan dihapus. Bagan tetap membaca sel yang sama seperti sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan tempatnya di lembar, meskipun baris dan kolom di sana lebih tinggi atau lebih lebar, dan ikut bergerak serta berubah ukuran bersama sel seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini adalah satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika sebuah lembar dihapus olehnya. Berkas dalam bentuk `.ods` tidak dapat menyimpan lembar bagan, jadi simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (pada menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Ketika sel berubah, judul pun ikut berubah. Berkas menyimpan tautannya.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dengan ukuran dua kali lipat dari ukuran di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya kepada bagan lain. Tampilan itu meliputi jenis, warna, pengaturan judul dan legenda, sumbu, dan fon. Bagan yang menerima templat tetap memakai angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus tampilan tersebut kembali.
- **Ubah Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri yang digambar terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di dalam tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan mini yang muat di dalam satu sel. Sparkline menampilkan tren satu baris atau satu kolom angka tepat di sebelahnya.

1. Pilih angkanya. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, dengan ukuran yang sama, cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline ditempatkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca seluruh data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok dan memiliki tampilan yang sama. Klik sel berisi sparkline, dan tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (membiarkan celah, menganggapnya nol, atau menghubungkan titik di seberangnya) dan apakah sel di baris dan kolom tersembunyi ikut dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garisnya, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, memplot angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Skala Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** memberi jarak titik sesuai tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal lebih akhir akan berada lebih jauh ke kanan. Kosongkan untuk meratakan jarak titik kembali.
- **Kelompokkan** memasukkan sparkline yang dipilih ke dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat diurungkan. Sparkline disimpan dalam berkas dalam bentuk yang dibaca program lembar kerja lain.

## Memasukkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka jendela pemilihan berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG), dan gambar akan muncul di lembar pada sel yang dipilih. Gambar yang lebih lebar dari 480 titik diperkecil agar pas, dan bentuknya tetap. Buku kerja menyimpan gambarnya sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (SVG, misalnya) disimpan sebagai PNG, yaitu gambar biasa yang terdiri dari titik. Berkas yang bukan gambar, atau yang lebih besar dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat sebagian program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi kumpulan gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau beberapa ikon, dan masing-masing akan muncul di lembar sebagai gambar selebar dan setinggi 48 poin. Ikon digambar dengan warna abu-abu tua pada ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat diurungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak sekaligus): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angkanya, dan jendela menampilkan kode batang saat Anda mengetik. Jika kode batang tidak dapat dibuat, jendela akan menjelaskan kesalahannya dengan tulisan merah. Klik **Sisipkan**, dan kode batang akan muncul di lembar sebagai gambar, sebagai satu langkah yang dapat diurungkan. Pindahkan dan ubah ukurannya seperti gambar lainnya. Untuk mengubahnya, pilih kode batang itu lalu klik **Kode Batang** lagi, ubah teksnya, dan klik **Perbarui**; kode batang baru menggantikan tempat dan lebar yang lama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam, sehingga setiap program dapat menampilkannya. JeffCrunch juga menyimpan pengaturannya dalam berkas, sehingga kode batang itu tetap dapat diedit saat Anda membuka kembali buku kerja di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel yang berisi data di antara sel yang dipilih, misalnya untuk lembar label. Pilih sel, klik perintah ini, lalu pilih jenis dan tampilannya di jendela yang sama. Apa pun yang Anda ketik di sana hanya contoh, karena teks setiap sel itulah yang menjadi kode batangnya. Kode batang diletakkan dalam satu kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat diurungkan. Sel yang teksnya tidak dapat dimuat oleh kode (misalnya huruf di dalam EAN-13) disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel diproses sekaligus; pilih lebih sedikit jika pesan menyebutkan terlalu banyak. Setiap kode batang mengingat sel asalnya dan mengikutinya: ketika sel itu berubah, kode batang dibuat ulang dari teks baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan ikut berpindah bersama sel saat Anda menyisipkan atau menghapus baris atau kolom. Jika sel dihapus, kode batang tetap menyimpan teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel pada saat itu, sehingga program lain juga menampilkan nilai terkini.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, oval, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih salah satu, dan bentuk itu akan muncul di lembar dengan warna biru muda dan tepi yang lebih gelap. **Kotak Teks**, pada grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi. Klik **Gambar Kotak Teks** (tombol utama, atau pilihan pertama pada menunya), lalu seret di lembar untuk menggambar kotak. Seretan membuat kotak dengan ukuran yang Anda seret, tetapi tidak pernah kurang dari 18 poin (sekitar 0,6 cm) pada setiap sisi; klik kurang dari 4 poin menghasilkan ukuran biasa. Tahan **Shift** untuk menjaga kotak tetap persegi. Penunjuk berbentuk silang sampai Anda melepas tombol; **Esc**, atau perintah lain, menonaktifkan alat ini. Kotak adalah satu langkah yang dapat diurungkan.

Untuk memutar tulisan dalam kotak teks, pilih kotak itu lalu klik **Arah Teks** pada tab **Format Bentuk**: **Horizontal**, **Putar semua teks 90°**, atau **Putar semua teks 270°**. Tulisan diketik dalam keadaan diputar, sesuai dengan tampilannya, dan buku kerja yang disimpan dalam bentuk .xlsx atau .ods mempertahankan arah tersebut.

Dengan bentuk yang dipilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru tua. Dua pilihan terakhir hanya menyimpan garis tepi atau hanya isiannya. Satu klik adalah satu langkah yang dapat diurungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis di internet

**Gambar Daring**, pada grup **Ilustrasi** tab **Sisipkan**, mencari gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Sebuah panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik sebuah gambar untuk melihat siapa pembuatnya dan apa **lisensinya**. **Lisensi** menjelaskan bagaimana Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik ganda gambarnya).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya ketika Anda mengklik **Cari** atau menekan Enter. Kemudian JeffCrunch meminta pustaka tersebut untuk pratinjau kecil dari hasil yang ditemukan, serta satu gambar yang Anda sisipkan. Seperti halaman web lainnya, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang meminta. Tidak ada isi buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar, berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya dimasukkan sebagai satu langkah, sehingga **Urungkan** membatalkan keduanya. Periksa bahwa lisensinya sesuai dengan cara Anda akan menggunakan gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf berwarna, dengan garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Tulisan "Teks Anda di Sini" muncul di lembar dengan gaya tersebut.
3. Untuk mengubah tulisan, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, fon, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata-kata menjadi bentuk, seperti gelombang atau lengkung. "Biasa" membuatnya lurus kembali.

Seni Teks adalah bentuk dalam gambar lembar, lengkapi dengan lengkungan, isian, dan garis tepinya, sehingga tampilannya tetap ketika Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya kembali, baik di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat diurungkan. Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan tulisan, fon, isian, garis tepi, dan lengkungannya, dan kembali menjadi Seni Teks saat berkas .ods dibuka lagi. Beberapa lengkungan yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, serta bentuk bawah dan atas dari Membesar dan Mengecil) disimpan sebagai tulisan lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada yang tampil di lembar, dan membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** pada grup **Sesuaikan** di tab **Format Gambar**. Pilih tempat buku kerja akan digunakan: **Cetak Komersial (300 ppi)**, **Cetak Desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghilangkan bagian tersembunyi yang telah Anda potong, dan potongan itu hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti semula. **Urungkan** (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya berisi gambar yang lebih kecil. Gambar pada lembar lain tidak diubah: buka setiap lembar dan lakukan lagi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap perubahan adalah satu langkah yang dapat diurungkan. Objek baru ikut dipilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau ke belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Menampilkan teks sel di dalam bentuk atau kotak teks

Bentuk atau kotak teks dapat menampilkan isi sel dan mengikutinya. Pilih bentuknya, klik **bilah rumus**, ketik tanda sama dengan dan sebuah sel, lalu tekan **Enter**.

- `=B2` menautkan bentuk ke B2 pada lembar ini. `=Sheet2!B3` menautkannya ke sel di lembar lain. `=Total` menautkannya ke nama yang mewakili satu sel.
- Bentuk kemudian menampilkan teks sel sebagaimana sel menampilkannya, dengan format angkanya. Ketika Anda mengubah sel, formatnya, atau rumus yang bergantung padanya, bentuk ikut berubah. Teks yang lebih panjang dari yang dapat ditampung sel (32.767 karakter) akan dipotong. Semua bentuk tertaut dalam satu buku kerja secara bersama-sama menampilkan paling banyak 4 MB teks: jika melebihi itu, sebuah bentuk menampilkan teksnya yang dipotong.
- Dengan bentuk yang dipilih, bilah rumus menampilkan tautannya, seperti `=Sheet1!$B$2`. Kosongkan bilah rumus lalu tekan **Enter** untuk menghapus tautan. Bentuk tetap menampilkan kata-kata yang ada saat itu.
- Mengetik kata-kata di dalam bentuk juga menghapus tautan.
- Tautan hanya untuk satu sel. Rentang seperti `=A1:B2`, daftar, perhitungan, atau nama yang bukan sel akan ditolak dengan pesan, dan bentuk tetap mempertahankan tautannya.
- Jika baris atau kolom sel dihapus, atau lembarnya dihapus, bentuk menampilkan `#REF!` dan tetap menyimpan tautannya, seperti rumus. **Urungkan** mengembalikan sel tersebut.
- Setiap tautan, dan setiap penghapusan tautan, adalah satu langkah yang dapat diurungkan.

Tautan disimpan dalam berkas .xlsx bersama teks yang ditampilkan sel, sehingga program yang tidak mengikuti tautan tetap menampilkan kata-katanya, dan JeffCrunch membuka berkas dengan tautan yang berfungsi. OpenDocument tidak memiliki cara untuk menautkan kotak teks ke sel, sehingga buku kerja yang disimpan sebagai berkas .ods menyimpan kata-kata yang ditampilkan bentuk pada saat itu, bukan tautannya.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat diurungkan, dan objek tetap menyimpan namanya, teks alternatifnya, tempatnya, dan urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garisnya) dan jenis putus-putusnya (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, 4 lebar kali 3 tinggi, 3 lebar kali 4 tinggi, 16 kali 9, atau 9 kali 16. Pemotongan dilakukan secara merata dari tengah bagian yang terlihat, dan gambar diperbesar sebesar sisa yang ada. **Tampilkan Seluruh Gambar** mengembalikan bagian yang dipotong. **Isi** mempertahankan bingkai dan memotong gambar agar menutupi seluruhnya. **Pas** menampilkan seluruh gambar, dan bingkai mengambil bentuk gambar di dalam bingkai lama. **Potong ke Bentuk** (galeri di sebelah **Potong**) memotong gambar menjadi bentuk seperti oval, bintang, atau persegi panjang sudut membulat; bentuk itu disimpan bersama gambar. Berkas gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** khusus untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih posisinya ke samping dan ke atas-bawah, ukurannya, tebal atau biasa, dan apakah kata-kata dibungkus mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, kotak ini meminta kata-kata, fon, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek di lembar menurut namanya, dengan objek paling depan di atas. Klik sebuah nama untuk memilih objek itu, yang berguna ketika objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek ketika sel dipindahkan atau dibuat lebih besar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang menggunakan sesuatu selain gambar dari buku kerja, seperti tautan ke berkas, tidak dapat disalin, dan JeffCrunch memberi tahu alasannya.

Salin bagan dan tempel ke buku kerja yang sama, pada lembar mana pun: salinan itu membaca sel yang sama dengan bagan pertama. Tempel ke buku kerja lain dan bagan tetap menyimpan angka yang ditampilkannya, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkan, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik kembali objek yang sudah dipilih sambil menahan Shift atau Ctrl untuk membatalkan pilihannya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** dan menyeret kotak di atas sel kosong: setiap objek yang sepenuhnya berada di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah ini pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu bergerak dan berubah ukuran sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memisahkan kelompok yang dipilih. Setiap bagiannya kembali ke tempatnya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek diratakan terhadap tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** mengatur tiga objek atau lebih agar berjarak sama di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat diurungkan.

## Memberi nama

Dengan objek yang dipilih, kotak nama menampilkan namanya. Ketik nama baru di sana dan tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** saat objek dipilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dalam bentuk yang dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini dapat dibuka di sana dengan cara yang sama. Bagan yang berasal dari program lain dan memiliki unsur yang tidak dapat disimpan JeffCrunch saat menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi unsur serta datanya tidak diubah di sini.

## Memasukkan objek

Sebuah **objek** menyimpan berkasnya sendiri di dalam buku kerja, sehingga aplikasi yang membuatnya dapat membukanya kembali. Objek menampilkan gambar dirinya sendiri, tetapi objek itu bukan gambar: ia tetap menjadi objek ketika Anda memindahkannya, mengubah ukurannya, menyalinnya, atau menyimpan buku kerja.

1. Klik **Sisipkan**, lalu **Objek**, dan pilih **Dari Rentang JeffCrunch**, **Dari Bagan JeffCrunch**, atau **Dari Halaman JeffWrite**.
2. Pilih berkasnya, dan rentang atau nomor halaman jika kotak meminta. Jika Anda sudah menyalinnya di aplikasi lain, kotak menawarkan untuk memakainya.
3. Objek dimasukkan pada sel yang dipilih. Seret untuk memindahkannya, dan seret pegangan untuk mengubah ukurannya. Rentang, bagan, atau halaman yang ukurannya diubah akan digambar ulang pada ukuran barunya, sehingga tetap tajam. Tekan **Delete** untuk menghapusnya. Setiap tindakan adalah satu langkah Urungkan.

Untuk memasukkan apa yang Anda salin di aplikasi lain, klik panah di bawah **Tempel** dan pilih **Sebagai Objek**.

**Klik ganda** sebuah objek untuk membukanya di aplikasi yang membuatnya. Simpan di sana, dan objek di sini menampilkan perubahannya sebagai satu langkah Urungkan. Sel yang sedang Anda ketik dibiarkan apa adanya. Salinan yang Anda tempel adalah objek tersendiri: mengubah salah satunya tidak mengubah yang lain. Hal yang sama berlaku untuk lembar yang Anda salin: objeknya juga disalin, dan salinan lembar tetap memilikinya.

**Berkas**, lalu **Info**, lalu **Objek** menampilkan setiap objek dalam buku kerja. Pilih satu untuk membukanya, mengubahnya menjadi gambar biasa, atau menghapusnya.

Objek yang dibuat di program lain tampil sebagai gambar yang disimpan berkasnya, apa pun jenis gambarnya. Anda dapat memilihnya, memindahkannya, mengubah ukurannya, merapikannya dengan objek lain, dan menghapusnya, dan objek itu bergerak bersama sel seperti objek Anda sendiri. Anda tidak dapat mengeditnya di sini, memasukkannya ke dalam kelompok, atau mengubah urutannya di antara objek. Objek disimpan persis seperti berkas aslinya, hanya tempatnya yang berubah jika Anda memindahkannya. Objek yang disimpan di berkas .ods bekerja dengan cara yang sama. Saat disimpan sebagai berkas .xlsx atau .ods, objek Anda berada dalam bentuk standar yang dibaca program lain. Ketika berkas .ods ditulis untuk objek yang gambarnya tidak dapat digambar, objek tetap disimpan tanpa gambar.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, seperti sebuah proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sebelahnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Ketika Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan di berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, pai, donat, dan bagan sebar, masing-masing dengan seri dan sel yang dibacanya (pada lembar ini atau lembar lain), kategori, judul, legenda, sumbu (judul, nilai terendah dan tertinggi, format angka), label data, warna, dan tempatnya. Berkas .ods yang dibuat program lain dibuka beserta bagannya, digambar dari sel yang dibacanya. Apa pun yang tidak memiliki tempat di bagan .ods, yaitu garis tren, bilah kesalahan, sumbu kedua, dan tampilan tambahan untuk satu titik, akan dihilangkan dan disebutkan saat Anda menyimpan. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
