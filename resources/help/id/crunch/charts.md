# Bagan, sparkline, gambar, dan bentuk

<!-- keywords: tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, lingkaran, area, sebar, kombinasi, keterangan, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, alt text, logaritmik, skala logaritma, simpan sebagai gambar, templat, crtx, teks judul, kunci keterangan, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, oval, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area yang dipotong, kode batang, kode QR -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ketika angka diubah, bagan ikut berubah. Gambar dan bentuk ditempatkan di atas sel dengan cara yang sama, tetapi keduanya tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Lingkaran**, **Area**, **Sebar**, atau **Kombinasi**. Tanda panah kecil di samping suatu jenis berisi variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebar dengan titik, titik yang tersambung, atau garis halus.
3. Untuk melihat semua jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Belum yakin harus memilih jenis yang mana? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel-sel tersebut dan menampilkan beberapa jenis yang sesuai, dengan yang paling cocok di urutan teratas. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan lingkaran. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri disusun menurun pada kolom jika pilihan memiliki baris setidaknya sebanyak kolomnya, dan mendatar pada baris jika pilihan memiliki lebih banyak kolom daripada baris. Lingkaran hanya memiliki satu seri. Bagan sebar mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya berupa angka. Bagan tersebut memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 hingga Q3.

## Menyembunyikan seri untuk sementara

Pilih sebuah bagan. Tiga tombol kecil muncul di sebelahnya. Tombol dengan gambar corong adalah **Filter Bagan**. Tombol ini menampilkan daftar seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang kembali untuk memunculkannya lagi di posisi yang sama. **Urungkan** juga berfungsi. Bagan selalu mempertahankan setidaknya satu seri. Seri yang Anda keluarkan tetap tersimpan dalam berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang kembali untuk memunculkannya. (Bagan donat menampilkan semua seri saat dibuka.)

## Memindahkan dan mengubah ukurannya

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkannya, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar sisi objek sejajar dengan tepi sel.
- Tahan **Shift** saat menyeret sudut untuk mempertahankan proporsi.
- Tombol panah menggeser objek beberapa piksel sekali tekan. **Ctrl** dengan panah menggeser satu piksel, **Shift** dengan panah membuat objek lebih besar atau lebih kecil, dan **Alt** dengan panah menuju tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap perpindahan atau perubahan ukuran adalah satu langkah yang dapat Anda urungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), untuk mengetik ukurannya, dan untuk memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berubah ukuran mengikuti sel, bergerak mengikuti sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman cetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi yang ditampilkan bagan

Saat sebuah bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul pada pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, keterangan, dan label data, serta menentukan tempat keterangan dan label data.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan dari JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan daftar seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca masing-masing seri, mengatur sel yang berisi label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi digambarkan dalam bagan.
- **Tombol Bidang** (di menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (di menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Standard Error**, **Percentage (5%)**, **Standard Deviation**, atau **Fixed Value (1)**. Pilih **Custom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawahnya. Biarkan salah satunya kosong untuk menggambar bilah hanya pada satu sisi. Bilah berubah saat sel tersebut berubah. Berkas yang sudah memiliki jumlah sendiri untuk setiap titik akan mempertahankannya.
- **Garis** (di menu **+**, untuk bagan garis) berisi **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke bawah hingga sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Batang Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Batang naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Batang Naik/Turun membutuhkan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Tabel Data** (di menu **+**, untuk bagan kolom, garis, atau area) menampilkan kisi angka di bawah bagan, dengan satu kolom untuk setiap kategori dan satu baris untuk setiap seri. Daftarnya menyalakan dan mematikan tabel, serta mengatur garis di antara baris, garis di antara kolom, tepi, dan kunci keterangan (warna setiap seri di samping namanya).
- **Garis Tren** (di menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat ke mana arahnya. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menulis rumus garis pada bagan, dan **Tampilkan R-kuadrat** menulis seberapa baik garis tersebut cocok (1 berarti sangat cocok; 0 berarti tidak cocok sama sekali). Garis tren mendapat entri sendiri pada keterangan.
- Ketika sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Keterangan** memiliki **Di Atas Bagan**, yang meletakkan keterangan di atas area plot, bukan di sampingnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci keterangan, yaitu kotak kecil berwarna seri, dalam kombinasi apa pun), **Pemisah** (apa yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0.0 untuk angka pada label; biarkan kosong untuk menampilkan angka apa adanya). **Nilai dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel-selnya, misalnya =Sheet1!$D$2:$D$4. Label berubah saat sel berubah. Biarkan kosong untuk menghapusnya. Matikan semua isi dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Di sana Anda dapat mengatur nilai terendah dan tertinggi serta satuan utama dan minor (masing-masing otomatis atau angka yang Anda ketik), apakah nilai berjalan dalam urutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 terbaca sebagai 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skalanya logaritmik (setiap tanda merupakan basis dikali tanda sebelumnya, misalnya 1, 10, 100, dan 1000, yang cocok untuk angka yang berbeda jauh ukurannya dan semuanya harus lebih dari 0), tempat sumbu lain memotong sumbu ini, format angka label, tanda skala, posisi label, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan tempat bagan harus diletakkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagannya sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan akan diletakkan di lembar kerja tersebut. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkannya akan dihapus. Bagan tetap membaca sel yang sebelumnya dibacanya. Pada lembar kerja, bagan mempertahankan ukuran dan posisinya pada lembar, bahkan di tempat baris dan kolomnya lebih tinggi atau lebih lebar, serta ikut bergerak dan berubah ukuran bersama sel di sana seperti sebelumnya. Lembar yang tersembunyi tidak ditampilkan dalam daftar. Ini adalah satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika lembar dihapus olehnya. Berkas dengan bentuk `.ods` tidak dapat memuat lembar bagan, sehingga simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (di menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul akan menampilkan isi sel tersebut. Saat sel berubah, judul ikut berubah. Berkas menyimpan tautannya.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada ukurannya pada lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya ke bagan lain. Tampilan tersebut meliputi jenis, warna, pengaturan judul dan keterangan, sumbu, dan font. Bagan yang menerima templat tetap memakai angka dan sel yang dibacanya sendiri. Satu **Urungkan** menghapus tampilan tersebut kembali.
- **Ubah Jenis Bagan** mengubah jenis untuk seluruh bagan atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di antara tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan mini yang muat di dalam satu sel. Sparkline menampilkan tren satu baris atau satu kolom angka tepat di sebelahnya.

1. Pilih angka-angkanya. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya dengan ukuran yang sama, yang cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca semua data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok dan berbagi satu tampilan yang sama. Klik sel yang berisi sparkline, maka tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data sparkline yang Anda klik. Di sini juga diatur apa yang dilakukan terhadap sel kosong (biarkan celah, anggap sebagai nol, atau hubungkan titik melintasinya) dan apakah sel pada baris dan kolom tersembunyi ikut dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang serta **Ketebalan** (seberapa tebal garis, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, memplot angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** mengatur jarak titik berdasarkan tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal lebih baru akan berada lebih jauh ke kanan. Biarkan kosong untuk mengatur ulang jarak titik secara merata.
- **Kelompokkan** menempatkan sparkline yang dipilih dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan cara yang dapat dibaca oleh program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka pemilih berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG) dan gambar akan muncul pada lembar di sel yang dipilih. Gambar yang lebih lebar dari 480 titik diperkecil agar muat, dan tetap mempertahankan bentuknya. Buku kerja menyimpan gambar itu sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (SVG, misalnya) disimpan sebagai PNG, yaitu gambar biasa yang terdiri dari titik. Berkas yang bukan gambar, atau berukuran lebih dari 16 MB, akan ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat sebagian program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi kumpulan gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau lebih, dan masing-masing akan muncul pada lembar sebagai gambar, lebarnya dan tingginya 48 poin. Gambar digambar dengan warna abu-abu gelap pada ukuran yang tetap tajam saat Anda perbesar. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, misalnya alamat web. Pilih **Jenis**, ketik teks atau angka, dan jendela akan menampilkan kode batang saat Anda mengetik. Jika tidak dapat dibuat, jendela menjelaskan kesalahannya dengan teks merah. Klik **Sisipkan**, dan kode batang akan muncul pada lembar sebagai gambar, yang dapat diurungkan dalam satu langkah. Pindahkan dan ubah ukurannya seperti gambar lainnya. Untuk mengubahnya, pilih kode batang lalu klik **Kode Batang** lagi, ubah teksnya, dan klik **Perbarui**; kode batang baru menggantikan yang lama dengan lebar yang sama. Saat Anda menyimpan, kode batang disimpan sebagai gambar SVG yang tetap tajam, sehingga semua program dapat menampilkannya. JeffCrunch juga menyimpan pengaturannya dalam berkas, sehingga kode batang tetap dapat Anda edit saat buku kerja dibuka kembali di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel yang berisi data dari sel yang dipilih, misalnya untuk lembar label. Pilih sel-sel, klik perintah tersebut, lalu pilih jenis dan tampilan pada jendela yang sama. Apa yang Anda ketik di sana hanya contoh, karena teks setiap sel menjadi kode batangnya sendiri. Kode batang disusun dalam satu kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat Anda urungkan. Sel yang teksnya tidak dapat dimuat kode (misalnya huruf pada EAN-13) disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel diproses sekaligus; pilih lebih sedikit jika pesan menyatakan terlalu banyak. Setiap kode batang mengingat sel asalnya dan mengikutinya: ketika sel itu berubah, kode batang dibuat ulang dari teks yang baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan ikut berpindah bersama sel saat Anda menyisipkan atau menghapus baris atau kolom. Jika sel dihapus, kode batang mempertahankan teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel pada saat itu, sehingga program lain juga menampilkan nilai terbaru.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, oval, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih satu dan bentuk akan muncul pada lembar dengan warna biru muda dan tepi yang lebih gelap. **Kotak Teks**, pada grup **Teks**, memasukkan kotak kosong tanpa isian dan tanpa tepi.

Dengan bentuk terpilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru tua. Dua pilihan terakhir hanya menyimpan garis tepi atau hanya isiannya. Satu klik adalah satu langkah yang dapat diurungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis di internet

**Gambar Daring**, pada grup **Ilustrasi** tab **Sisipkan**, mencari gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Sebuah panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik apa yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik sebuah gambar untuk melihat siapa pembuatnya dan apa **lisensinya**. **Lisensi** menjelaskan bagaimana Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik ganda gambarnya).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya ketika Anda mengklik **Cari** atau menekan Enter. Setelah itu JeffCrunch meminta pustaka untuk pratinjau kecil dari hasil yang ditemukan, dan satu gambar yang Anda sisipkan. Seperti halaman web lainnya, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang memintanya. Tidak ada isi buku kerja Anda yang dikirim.

Gambar masuk ke lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar, berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya disisipkan dalam satu langkah, sehingga **Urungkan** menarik keduanya kembali. Periksa bahwa lisensinya sesuai dengan cara Anda akan menggunakan gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata hias: huruf dengan warna, garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Kata "Teks Anda di Sini" muncul pada lembar dengan gaya tersebut.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkung. "Biasa" membuatnya lurus kembali.

Seni Teks adalah bentuk dalam gambar lembar, lengkap dengan lengkungan, isian, dan garis tepinya, sehingga tampilannya tetap sama saat Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya kembali, di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungannya, dan kembali sebagai Seni Teks saat berkas .ods dibuka lagi. Beberapa lengkungan yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, dan bentuk bawah serta atas dari Membesar dan Mengecil) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada yang ditampilkan pada lembar, dan membuat berkas menjadi besar. Klik sebuah gambar, lalu klik **Kompres Gambar…** pada grup **Sesuaikan** tab **Format Gambar**. Pilih tempat buku kerja akan dipakai: **Cetak Komersial (300 ppi)**, **Cetak Desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar pada lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghapus bagian tersembunyi yang telah Anda potong, sehingga potongan tersebut hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, posisi, nama, dan teks alternatifnya tetap seperti semula. **Urungkan** (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya memuat gambar yang lebih kecil. Gambar pada lembar lain tidak berubah: buka setiap lembar dan lakukan lagi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah adalah satu tindakan yang dapat Anda urungkan. Objek baru dipilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau ke belakang, memberinya nama dan teks alternatif pada **Properti**, atau menekan **Delete**.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih sebuah gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek mempertahankan nama, teks alternatif, posisi, serta urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garis) dan jenis garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, lebar 4 tinggi 3, lebar 3 tinggi 4, 16 banding 9, atau 9 banding 16. Pemotongan dilakukan merata dari tengah bagian yang terlihat, dan gambar diperbesar sebesar sisa yang tersisa. **Tampilkan Seluruh Gambar** mengembalikan bagian yang terpotong. **Isi** mempertahankan bingkai dan memotong gambar sehingga menutupi seluruhnya. **Sesuaikan** menampilkan seluruh gambar, dan bingkai mengambil bentuk gambar di dalam bingkai lama. **Potong ke Bentuk** (galeri di sebelah **Potong**) memotong gambar menjadi bentuk seperti oval, bintang, atau persegi panjang sudut membulat; bentuk tersebut disimpan bersama gambar. Berkas gambar tidak berubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih cara letaknya secara mendatar dan vertikal, ukurannya, tebal atau biasa, serta apakah kata dibungkus mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, perintah ini meminta kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek pada lembar berdasarkan nama, dengan yang paling depan di atas. Klik sebuah nama untuk memilih objek tersebut, yang membantu saat objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek saat sel dipindahkan atau dibuat lebih besar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang memakai sesuatu selain gambar dari buku kerja, misalnya tautan ke berkas, tidak dapat disalin, dan JeffCrunch memberi tahu alasannya.

Salin sebuah bagan dan tempelkan ke buku kerja yang sama, di lembar mana pun: salinannya membaca sel yang sama dengan yang pertama. Tempelkan ke buku kerja lain dan bagan tersebut tetap menampilkan angka yang sebelumnya tampil, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkan, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi sambil menahan Shift atau Ctrl untuk membatalkan pilihannya. Setiap objek yang dipilih mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** lalu menyeret kotak di atas sel kosong: setiap objek yang seluruhnya berada di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah berikut pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu dipindahkan dan diubah ukurannya sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memisahkan kelompok yang dipilih. Setiap bagiannya kembali ke posisinya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek diratakan terhadap tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi tiga objek atau lebih secara merata di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan objek terpilih, kotak nama menampilkan namanya. Ketik nama baru di sana lalu tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** saat objek terpilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan cara yang dapat dibaca oleh program lembar kerja lain, sehingga bagan yang Anda buat di sini dapat dibuka di sana dengan cara yang sama. Bagan yang berasal dari program lain dan memiliki sesuatu yang tidak dapat dipertahankan JeffCrunch saat menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen serta datanya tidak diubah di sini.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, seperti sebuah proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sebelahnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Saat Anda menyimpan berkas, diagram ditulis kembali apa adanya, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, lingkaran, donat, dan bagan sebar, masing-masing dengan seri dan sel yang dibacanya (pada lembar ini atau lembar lain), kategori, judul, keterangan, sumbu (judul, nilai terkecil dan terbesar, format angka), label data, warna, serta tempatnya. Berkas .ods buatan program lain dibuka beserta bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat di bagan .ods akan dihilangkan dan disebutkan saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, dan tampilan tambahan untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
