# Bagan, sparkline, gambar, dan bentuk

<!-- keywords: tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, pai, area, sebar, kombinasi, legenda, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, teks alternatif, logaritmik, skala logaritma, simpan sebagai gambar, templat, crtx, teks judul, kunci legenda, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, lingkaran, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area yang dipotong, kode batang, kode QR -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ketika angka berubah, bagan ikut berubah. Gambar dan bentuk ditempatkan di atas sel dengan cara yang sama, tetapi keduanya tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Pai**, **Area**, **Sebar**, atau **Kombinasi**. Tanda panah kecil di sebelah jenis memuat variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebar berupa titik, titik tersambung, atau garis halus.
3. Untuk melihat setiap jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Belum yakin jenis mana yang harus dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel dan menampilkan beberapa jenis yang cocok, dengan yang terbaik di urutan pertama. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan pai. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri disusun menurut kolom jika pilihan memiliki baris setidaknya sebanyak kolom, dan menurut baris jika pilihan memiliki lebih banyak kolom daripada baris. Pai hanya memuat satu seri. Bagan sebar mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya berupa angka. Bagan memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 sampai Q3.

## Menyembunyikan seri untuk sementara

Pilih sebuah bagan. Tiga tombol kecil muncul di sebelahnya. Tombol dengan gambar corong adalah **Filter Bagan**. Tombol ini menampilkan seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang lagi untuk memunculkannya kembali di tempat yang sama. **Urungkan** juga berfungsi. Bagan selalu menyimpan setidaknya satu seri. Seri yang Anda keluarkan tetap ada dalam berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang lagi untuk memunculkannya kembali. (Bagan donat menampilkan setiap seri saat dibuka.)

## Memindahkan dan mengubah ukuran

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkan, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar tepi objek sejajar dengan tepi sel.
- Tahan **Shift** saat menyeret sudut agar proporsinya tetap.
- Tombol panah memindahkan objek beberapa piksel setiap kali. **Ctrl** dengan panah memindahkan satu piksel, **Shift** dengan panah membuat objek lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat Anda urungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), mengetik ukurannya, dan memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berubah ukuran bersama sel, bergerak bersama sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman yang dicetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi bagan

Saat bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul pada pita.

- Tombol **+** (dan **Elemen Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, legenda, dan label data, serta menentukan tempat legenda dan label. 
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna JeffCrunch dan salah satu tampilannya. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca masing-masing seri, menentukan sel dengan label, menukar baris dan kolom, dan memilih cara sel kosong serta baris dan kolom tersembunyi dibagankan.
- **Tombol Bidang** (dalam menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (dalam menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Deviasi Standar**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawahnya. Kosongkan salah satu untuk menggambar bilah hanya di satu sisi. Bilah berubah ketika sel-sel tersebut berubah. Berkas yang sudah memiliki jumlah sendiri untuk setiap titik tetap mempertahankannya.
- **Garis** (dalam menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke bawah hingga sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Batang Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Batang naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Batang Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Garis Tren** (dalam menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat arah angka tersebut. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menuliskan rumus garis pada bagan, dan **Tampilkan R-kuadrat** menuliskan seberapa baik garis itu cocok (1 berarti sangat cocok; 0 berarti sama sekali tidak cocok). Garis tren mendapat entri sendiri di legenda.
- Ketika sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Legenda** memiliki **Di Atas Bagan**, yang menempatkan legenda di atas area plot, bukan di sampingnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci legenda, yaitu kotak kecil berwarna seri, dalam kombinasi apa pun), **Pemisah** (apa yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0,0 untuk angka pada label; kosongkan untuk menampilkan angka apa adanya). **Nilai Dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel-selnya, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel-sel tersebut berubah. Kosongkan untuk menghapusnya. Matikan semua isi dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Kotak ini mengatur nilai terendah dan tertinggi serta satuan utama dan minor (masing-masing otomatis atau angka yang Anda ketik), apakah nilai ditampilkan dalam urutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 terbaca sebagai 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skalanya logaritmik (setiap tanda adalah basis dikali tanda sebelumnya, seperti 1, 10, 100, dan 1000, yang cocok untuk angka yang berbeda jauh besarnya dan semuanya harus lebih dari 0), di mana sumbu lain memotong sumbu ini, format angka labelnya, tanda skala, posisi labelnya, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan tempat bagan harus diletakkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan akan dipindahkan ke lembar kerja tersebut. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkan akan dihapus. Bagan tetap membaca sel yang dibacanya sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan posisinya di lembar, meskipun baris dan kolom di sana lebih tinggi atau lebih lebar, dan ikut berpindah serta berubah ukuran bersama sel seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini adalah satu langkah **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika lembar telah dihapus olehnya. Berkas dalam bentuk `.ods` tidak dapat menyimpan lembar bagan, jadi simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (dalam menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Ketika isi sel berubah, judul ikut berubah. Berkas menyimpan tautan tersebut.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, dalam grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya ke bagan lain. Tampilan tersebut meliputi jenis, warna, pengaturan judul dan legenda, sumbu, dan font. Bagan yang menerima templat tetap mempertahankan angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus tampilan tersebut kembali.
- **Ubah Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di dalam tanda kutip tunggal jika namanya mengandung spasi.

## Sparkline

Sparkline adalah bagan mini yang muat di dalam satu sel. Sparkline menampilkan tren dari satu baris atau kolom angka tepat di sebelahnya.

1. Pilih angka-angkanya. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya dengan ukuran sama, cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca seluruh data.

Sparkline yang Anda buat dalam satu langkah membentuk sebuah kelompok, dan berbagi satu tampilan. Klik sel yang memiliki sparkline, dan tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data dari sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (biarkan celah, anggap nol, atau sambungkan titik melewatinya) dan apakah sel pada baris dan kolom tersembunyi dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garis, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, menggambar angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** mengatur jarak titik berdasarkan tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal lebih baru akan berada lebih jauh ke kanan. Kosongkan untuk mengatur ulang jarak titik secara merata.
- **Kelompok** menempatkan sparkline yang dipilih dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka jendela pemilihan berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG), dan gambar akan muncul di lembar pada sel yang dipilih. Gambar yang lebarnya lebih dari 480 titik diperkecil agar pas, dan bentuknya tetap sama. Buku kerja menyimpan gambar itu sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (misalnya SVG) disimpan sebagai PNG, yaitu gambar biasa yang tersusun dari titik. Berkas yang bukan gambar, atau berukuran lebih dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat sebagian program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela yang sama berisi gambar garis kecil seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau beberapa ikon, dan masing-masing akan muncul di lembar sebagai gambar, lebar dan tingginya 48 poin. Ikon digambar dengan warna abu-abu tua pada ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak kode): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angka, dan jendela menampilkan kode batang saat Anda mengetik. Jika kode batang tidak dapat dibuat, jendela menjelaskan kesalahannya dengan teks merah. Klik **Sisipkan**, dan kode batang akan muncul di lembar sebagai gambar, yang dapat Anda urungkan dalam satu langkah. Pindahkan dan ubah ukurannya seperti gambar lain. Untuk mengubahnya, pilih kode batang lalu klik **Kode Batang** lagi, ubah teksnya, dan klik **Perbarui**; kode batang baru menggantikan yang lama dengan lebar yang sama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam, sehingga semua program dapat menampilkannya, dan JeffCrunch menyimpan pengaturannya dalam berkas, sehingga Anda masih dapat mengeditnya saat membuka buku kerja di JeffCrunch lagi.

**Kode Batang > Kode Batang dari Sel** (dalam menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel berisi pada sel yang dipilih, untuk lembar label. Pilih sel, klik perintah itu, lalu pilih jenis dan tampilannya di jendela yang sama. Teks yang Anda ketik di sana hanya contoh, karena teks setiap sel akan menjadi kode batangnya sendiri. Kode batang ditempatkan dalam satu kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat Anda urungkan. Sel yang teksnya tidak dapat dimuat kode (misalnya huruf pada EAN-13) disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel diproses sekaligus; pilih lebih sedikit jika ada pesan bahwa terlalu banyak sel. Setiap kode batang mengingat sel asalnya dan mengikutinya: ketika Anda mengubah sel itu, kode batang dibuat ulang dari teks yang baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan ikut berpindah bersama sel ketika Anda menyisipkan atau menghapus baris atau kolom. Jika Anda menghapus sel, kode batang tetap menyimpan teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel sebagaimana adanya saat itu, sehingga program lain juga menampilkan nilai terkini.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, lingkaran, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih salah satu, dan bentuk itu muncul di lembar dengan warna biru muda dan tepi biru yang lebih gelap. **Kotak Teks**, dalam grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi.

Dengan bentuk terpilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru tua. Dua pilihan terakhir hanya mempertahankan garis tepi atau hanya isian. Satu klik adalah satu langkah yang dapat Anda urungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah setiap bagian secara terpisah.

## Mencari gambar gratis di internet

**Gambar Daring**, dalam grup **Ilustrasi** pada tab **Sisipkan**, menemukan gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik yang ingin Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik gambar untuk melihat siapa pembuatnya dan apa **lisensi**-nya. Lisensi menjelaskan cara Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik ganda gambar).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya ketika Anda mengklik **Cari** atau menekan Enter. JeffCrunch kemudian meminta pustaka tersebut mengirimkan pratinjau kecil dari hasil pencarian, dan satu gambar yang Anda sisipkan. Seperti halaman web lainnya, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang meminta. Tidak ada yang dari buku kerja Anda yang dikirim.

Gambar akan muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar, berisi judul, nama pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya disisipkan dalam satu langkah, sehingga **Urungkan** menarik keduanya kembali. Pastikan lisensinya sesuai dengan cara Anda akan menggunakan gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf berwarna, dengan garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Kata "Teks Anda di Sini" muncul di lembar dengan gaya tersebut.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkung. "Biasa" membuat kata tetap lurus.

Seni Teks adalah bentuk dalam gambar lembar, dengan lengkung, isian, dan garis tepinya, sehingga tampilannya tetap sama ketika Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya kembali, di JeffCrunch maupun program lain. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungnya, lalu kembali sebagai Seni Teks ketika Anda membuka berkas .ods lagi. Beberapa lengkung yang tidak memiliki padanan bentuk di OpenDocument (Gelombang 2, Gelombang 4, serta bentuk bawah dan atas dari Membesar dan Mengecil) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods; lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada tampilannya di lembar, dan membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** dalam grup **Sesuaikan** pada tab **Format Gambar**. Pilih tempat buku kerja akan digunakan: **Cetak komersial (300 ppi)**, **Cetak desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghapus bagian tersembunyi yang telah Anda potong, sehingga potongannya hilang. Gambar hanya diganti jika versi baru lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti sebelumnya. Urungkan (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya berisi gambar yang lebih kecil. Gambar pada lembar lain tidak diubah: buka setiap lembar dan lakukan lagi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah adalah satu tindakan yang dapat Anda urungkan. Objek baru akan dipilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** akan muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** akan muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek tetap mempertahankan nama, teks alternatif, tempat, dan urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tidak ada. **Gaya Garis** mengatur ketebalan (seberapa tebal garisnya) dan garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau kiri, atau membalik objek secara horizontal atau vertikal.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, lebar 4 kali tinggi 3, tinggi 4 kali lebar 3, 16 kali 9, atau 9 kali 16. Pemotongan dilakukan merata dari tengah bagian yang terlihat, dan gambar dibuat sebesar mungkin dari sisa yang ada. **Tampilkan Seluruh Gambar** mengembalikan bagian yang dipotong. File gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih cara kata diletakkan secara horizontal dan vertikal, ukuran, tebal atau biasa, serta apakah kata dibungkus mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, perintah ini meminta kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar semua objek pada lembar berdasarkan nama, dengan objek paling depan di urutan pertama. Klik sebuah nama untuk memilih objek tersebut, yang berguna ketika objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek ketika sel dipindahkan atau diperbesar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang menggunakan sesuatu selain gambar dari buku kerja, misalnya tautan ke berkas, tidak dapat disalin, dan JeffCrunch memberi tahu alasannya.

Salin bagan lalu tempel di buku kerja yang sama, pada lembar mana pun: salinan itu membaca sel yang sama dengan bagan pertama. Jika ditempel di buku kerja lain, bagan tetap menyimpan angka yang ditampilkannya tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel asalnya.

## Memilih beberapa objek, mengelompokkan, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi sambil menahan Shift atau Ctrl untuk membatalkan pilihannya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** lalu menyeret kotak di atas sel kosong: setiap objek yang berada sepenuhnya di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah berikut dalam grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu bergerak dan berubah ukuran sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memisahkan kelompok yang dipilih. Setiap bagian kembali ke tempatnya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek akan rata dengan tepi atau tengah keseluruhan kelompok. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi tiga objek atau lebih secara merata di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan objek terpilih, kotak nama menampilkan namanya. Ketik nama baru di sana lalu tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** dengan objek terpilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini dapat dibuka di sana dengan cara yang sama. Bagan yang berasal dari program lain dengan sesuatu yang tidak dapat dipertahankan JeffCrunch saat menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen dan datanya tidak diubah di sini.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, misalnya sebuah proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sampingnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Ketika Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, pai, donat, dan bagan sebar, masing-masing dengan seri dan sel yang dibacanya (di lembar ini atau lembar lain), kategori, judul, legenda, sumbu (judul, nilai terkecil dan terbesar, format angka), label data, warna, dan tempatnya. Berkas .ods yang dibuat program lain dibuka dengan bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat dalam bagan .ods akan dihilangkan dan disebutkan namanya saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, dan tampilan tambahan untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
