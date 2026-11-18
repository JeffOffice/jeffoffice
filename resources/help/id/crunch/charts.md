# Bagan, sparkline, gambar, dan bentuk

<!-- keywords: tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, pai, area, sebaran, kombinasi, keterangan, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan kecil, penanda, nama, teks alternatif, alternative text, logaritmik, skala logaritma, simpan sebagai gambar, templat, crtx, teks judul, kunci keterangan, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, lingkaran, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area yang dipotong, kode batang, kode QR, tautkan bentuk ke sel, bentuk menampilkan sel, kotak teks menampilkan sel, nilai sel dalam bentuk -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ketika angka berubah, bagan ikut berubah. Gambar dan bentuk berada di atas sel dengan cara yang sama, tetapi tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Pai**, **Area**, **Sebaran**, atau **Kombinasi**. Tanda panah kecil di sebelah jenis memuat variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebaran berupa titik, titik tersambung, atau garis halus.
3. Untuk melihat semua jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Belum yakin jenis mana yang harus dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel-sel tersebut dan menampilkan beberapa jenis yang sesuai, dengan yang terbaik di urutan pertama. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan pai. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri berjalan ke bawah dalam kolom jika pilihan memiliki baris setidaknya sebanyak kolomnya, dan berjalan ke samping dalam baris jika kolomnya lebih banyak daripada barisnya. Pai hanya memiliki satu seri. Bagan sebaran mengambil nilai horizontal dari kolom pertama.

Contoh: Anda memilih A1:C4, di mana A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya berupa angka. Bagan memiliki dua seri, East dan West, dan tiga kelompok kolom, Q1 sampai Q3.

## Menyembunyikan seri untuk sementara

Pilih bagan. Tiga tombol kecil muncul di sebelahnya. Tombol dengan corong adalah **Filter Bagan**. Tombol ini menampilkan seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang lagi untuk memunculkannya kembali di posisi yang sama. **Urungkan** juga berfungsi. Bagan selalu menyimpan setidaknya satu seri. Seri yang Anda keluarkan tetap ada dalam berkas saat Anda menyimpannya, dan tetap tersembunyi ketika berkas dibuka, sehingga tidak ada data yang hilang. Centang lagi untuk memunculkannya. (Bagan donat menampilkan semua seri saat dibuka.)

## Memindahkan dan mengubah ukuran

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkannya, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar tepi objek sejajar dengan tepi sel.
- Tahan **Shift** saat menyeret sudut untuk mempertahankan proporsi.
- Tombol panah menggeser objek beberapa piksel setiap kali. **Ctrl** dengan panah memindahkan satu piksel, **Shift** dengan panah membuat objek lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat Anda urungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), untuk mengetik ukurannya, dan untuk memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: ikut berpindah dan berubah ukuran bersama sel, ikut berpindah tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman cetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi bagan

Saat bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul pada pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, keterangan, dan label data, serta menentukan di mana keterangan dan label diletakkan.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca masing-masing seri, menentukan sel dengan label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi dibuat bagannya.
- **Tombol Bidang** (dalam menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (dalam menu **+**, untuk bagan kolom, batang, garis, area, atau sebaran) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Deviasi Standar**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawahnya. Kosongkan salah satu untuk menggambar bilah hanya di satu sisi. Bilah berubah ketika sel tersebut berubah. Berkas yang sudah memiliki nilai sendiri untuk setiap titik tetap mempertahankannya.
- **Garis** (dalam menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke bawah hingga sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Bilah Naik/Turun**, berupa kotak di antara seri pertama dan terakhir pada setiap kategori. Bilah naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Bilah Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Tabel Data** (dalam menu **+**, untuk bagan kolom, garis, atau area) menampilkan kisi angka di bawah bagan, dengan kolom untuk setiap kategori dan baris untuk setiap seri. Daftarnya menyalakan dan mematikan tabel, serta mengatur garis antarbaris, garis antarkolom, garis tepi, dan kunci keterangan (warna setiap seri di sebelah namanya).
- **Garis Tren** (dalam menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat ke mana angka-angka itu menuju. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menuliskan rumus garis pada bagan, dan **Tampilkan R-kuadrat** menuliskan seberapa baik garis itu cocok (1 berarti sangat cocok; 0 berarti tidak cocok sama sekali). Garis tren mendapat entri sendiri pada keterangan.
- Saat sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Keterangan** memiliki **Di Atas Bagan**, yang meletakkan keterangan di atas area plot, bukan di sebelahnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci keterangan, yaitu kotak kecil berwarna seri, dalam kombinasi apa pun), **Pemisah** (apa yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0,0 untuk angka pada label; kosongkan untuk menampilkan angka apa adanya). **Nilai dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel-selnya, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel-sel tersebut berubah. Kosongkan untuk menghapus label. Matikan semua isi label dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Kotak ini mengatur nilai terendah dan tertinggi serta satuan utama dan kecil (masing-masing otomatis atau angka yang Anda ketik), apakah nilai berjalan dalam urutan terbalik, satuan tampilan (label menampilkan angka yang sudah dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 terbaca sebagai 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skala logaritmik (setiap tanda adalah basis dikali tanda sebelumnya, misalnya 1, 10, 100, dan 1000, yang cocok untuk angka yang sangat berbeda ukurannya dan semuanya harus lebih dari 0), di mana sumbu lain memotongnya, format angka labelnya, tanda skalanya, letak labelnya, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan di mana bagan akan diletakkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan dipindahkan ke lembar tersebut. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkannya akan dihapus. Bagan tetap membaca sel yang sama seperti sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan posisinya di lembar, meskipun baris dan kolomnya lebih tinggi atau lebih lebar, dan ikut berpindah serta berubah ukuran bersama sel di sana seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini merupakan satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika sebuah lembar dihapus olehnya. Berkas dengan bentuk `.ods` tidak dapat menyimpan lembar bagan, jadi simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (dalam menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Ketika sel berubah, judul ikut berubah. Berkas menyimpan tautan tersebut.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, dalam grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya ke bagan lain. Tampilan itu mencakup jenis, warna, pengaturan judul dan keterangan, sumbu, dan font. Bagan yang menerimanya tetap memakai angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus tampilan tersebut.
- **Ubah Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar diapit tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan kecil yang muat di dalam satu sel. Sparkline menampilkan tren dari satu baris atau kolom angka tepat di sebelahnya.

1. Pilih angka. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya berukuran sama, yang cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca semua data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok dan memakai satu tampilan bersama. Klik sel yang berisi sparkline, dan tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (membiarkan celah, menganggapnya nol, atau menyambung titik melewatinya) dan apakah sel di baris dan kolom tersembunyi dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garis, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, memplot angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** mengatur jarak titik berdasarkan tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal yang lebih akhir berada lebih jauh ke kanan. Biarkan kosong untuk mengatur jarak titik kembali secara merata.
- **Kelompokkan** memasukkan sparkline yang dipilih ke dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka pemilih berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG), dan gambar akan muncul di lembar pada sel yang dipilih. Gambar yang lebarnya lebih dari 480 titik diperkecil agar muat, dan tetap mempertahankan bentuknya. Buku kerja menyimpan gambar itu sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (misalnya SVG) disimpan sebagai PNG, yaitu gambar biasa yang terdiri dari titik. Berkas yang bukan gambar, atau yang lebih besar dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat sebagian program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi kumpulan gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau lebih, dan masing-masing muncul di lembar sebagai gambar selebar dan setinggi 48 poin. Ikon digambar dengan warna abu-abu tua pada ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak sekaligus): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angkanya, dan jendela menampilkan kode batang saat Anda mengetik. Jika kode tidak dapat dibuat, jendela menjelaskan dengan tulisan merah apa yang salah. Klik **Sisipkan**, dan kode batang muncul di lembar sebagai gambar; ini satu langkah yang dapat Anda urungkan. Pindahkan dan ubah ukurannya seperti gambar biasa. Untuk mengubahnya, pilih kode batang itu dan klik **Kode Batang** lagi, ubah teksnya, lalu klik **Perbarui**; kode baru menggantikan tempat dan lebar kode lama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam, sehingga setiap program dapat menampilkannya. JeffCrunch juga menyimpan pengaturannya dalam berkas, jadi kode batang tetap dapat Anda edit saat buku kerja dibuka lagi di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel terisi di antara sel yang dipilih, misalnya untuk lembar label. Pilih sel, klik perintah itu, lalu pilih jenis dan tampilannya di jendela yang sama. Teks yang Anda ketik di sana hanya contoh, karena teks setiap sel itulah yang menjadi kode batangnya. Kode batang diletakkan dalam satu kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat Anda urungkan. Sel yang teksnya tidak dapat dimuat kode (misalnya huruf dalam EAN-13) disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel dikerjakan sekaligus; pilih lebih sedikit jika ada pesan bahwa terlalu banyak. Setiap kode batang mengingat sel asalnya dan mengikutinya: ketika Anda mengubah sel itu, kode batang dibuat ulang dari teks yang baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan ini ikut berpindah bersama sel ketika Anda menyisipkan atau menghapus baris atau kolom. Jika Anda menghapus sel, kode batang mempertahankan teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel saat itu, sehingga program lain juga menampilkan nilai terkini.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, lingkaran, segitiga, berlian, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih satu, dan bentuk itu muncul di lembar dengan warna biru muda dan tepi yang lebih gelap. **Kotak Teks**, dalam grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi.

Dengan bentuk dipilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru tua. Dua gaya terakhir hanya menyimpan garis tepi atau hanya isian. Satu klik adalah satu langkah yang dapat Anda urungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis di internet

**Gambar Daring**, pada grup **Ilustrasi** tab **Sisipkan**, mencari gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik apa yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik gambar untuk melihat siapa pembuatnya dan apa **lisensi**-nya. Lisensi menjelaskan bagaimana Anda boleh memakai gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik dua kali gambar).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya ketika Anda mengklik **Cari** atau menekan Enter. Setelah itu JeffCrunch meminta pustaka tersebut mengirim pratinjau kecil dari hasil pencarian, dan gambar yang Anda sisipkan. Seperti halaman web lainnya, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang meminta. Tidak ada isi buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar, berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya dimasukkan sebagai satu langkah, sehingga **Urungkan** membatalkan keduanya. Pastikan lisensinya sesuai dengan cara Anda akan memakai gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf dengan warna, garis tepi, bayangan, atau bentuk lengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Tulisan "Teks Anda di Sini" muncul di lembar dengan gaya tersebut.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkung. "Datar" membuatnya lurus kembali.

Seni Teks adalah bentuk dalam gambar lembar, dengan lengkung, isian, dan garis tepinya, sehingga tampilannya tetap ketika Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya lagi, di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Buku kerja yang disimpan sebagai berkas .ods juga mempertahankan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungnya, dan kembali sebagai Seni Teks saat berkas .ods dibuka lagi. Beberapa lengkung yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, dan bentuk bawah serta atas dari Mengembung dan Mengempis) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada yang tampak di lembar, dan membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** dalam grup **Sesuaikan** pada tab **Format Gambar**. Pilih tempat buku kerja akan dipakai: **Cetak Komersial (300 ppi)**, **Cetak Desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghilangkan bagian tersembunyi hasil potongan, dan potongan itu hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti semula. Ketik Ctrl+Z untuk mengembalikan gambar (Undo), dan buku kerja yang tersimpan hanya berisi gambar yang lebih kecil. Gambar pada lembar lain tidak berubah: buka setiap lembar dan lakukan lagi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap perintah adalah satu langkah yang dapat Anda urungkan. Objek baru dipilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Menampilkan teks sel dalam bentuk atau kotak teks

Bentuk atau kotak teks dapat menampilkan isi sebuah sel, dan mengikutinya. Pilih bentuk, klik **bilah rumus**, ketik tanda sama dengan dan sebuah sel, lalu tekan **Enter**.

- `=B2` menautkan bentuk ke B2 pada lembar ini. `=Sheet2!B3` menautkannya ke sel di lembar lain. `=Total` menautkannya ke nama yang mewakili satu sel.
- Bentuk kemudian menampilkan teks sel seperti yang ditampilkan sel tersebut, dengan format angkanya. Ketika Anda mengubah sel, formatnya, atau rumus yang bergantung padanya, bentuk ikut berubah. Teks yang lebih panjang dari yang dapat ditampung sel (32.767 karakter) dipotong. Semua bentuk yang tertaut dalam satu buku kerja bersama-sama menampilkan paling banyak 4 MB teks: lebih dari itu, sebuah bentuk menampilkan teksnya yang dipotong.
- Dengan bentuk dipilih, bilah rumus menampilkan tautannya, seperti `=Sheet1!$B$2`. Kosongkan bilah rumus dan tekan **Enter** untuk menghapus tautan. Bentuk kemudian mempertahankan kata-kata yang ditampilkannya.
- Mengetik kata-kata di dalam bentuk juga menghapus tautan.
- Tautan hanya berlaku untuk satu sel. Rentang seperti `=A1:B2`, daftar, perhitungan, atau nama yang bukan sel ditolak dengan pesan, dan bentuk mempertahankan tautannya.
- Jika baris atau kolom sel dihapus, atau lembarnya dihapus, bentuk menampilkan `#REF!` dan tetap mempertahankan tautannya, seperti sebuah rumus. Urungkan untuk mengembalikan sel.
- Setiap tautan, dan setiap penghapusan tautan, adalah satu langkah yang dapat Anda urungkan.

Tautan disimpan dalam berkas .xlsx bersama teks yang ditampilkan sel, sehingga program yang tidak mengikuti tautan tetap menampilkan kata-katanya, dan JeffCrunch membuka berkas dengan tautan yang berfungsi. OpenDocument tidak memiliki cara untuk menautkan kotak teks ke sel, sehingga buku kerja yang disimpan sebagai berkas .ods menyimpan kata-kata yang ditampilkan bentuk pada saat itu, bukan tautannya.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek tetap mempertahankan namanya, teks alternatifnya, tempatnya, dan urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garis) dan garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bingkai jendela: persegi, 4 lebar kali 3 tinggi, 3 lebar kali 4 tinggi, 16 kali 9, atau 9 kali 16. Potongan dibuat merata dari tengah bagian yang tampak, dan gambar dibesarkan sebesar sisa yang tersedia. **Tampilkan Seluruh Gambar** mengembalikan bagian yang dipotong. **Isi** mempertahankan bingkai dan memotong gambar sehingga menutupi seluruhnya. **Muat** menampilkan seluruh gambar, dan bingkai mengambil bentuk gambar di dalam bingkai lama. **Potong ke Bentuk** (galeri di sebelah **Potong**) memotong gambar menjadi bentuk seperti lingkaran, bintang, atau persegi panjang sudut membulat; bentuk itu disimpan bersama gambar. Berkas gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** berlaku untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih cara kata-kata itu diletakkan secara mendatar dan tegak, ukuran, tebal atau biasa, serta apakah kata-kata membungkus ke dalam bentuk atau tetap dalam satu baris. Untuk Seni Teks, kotak ini meminta kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek di lembar menurut nama, dengan objek paling depan di atas. Klik sebuah nama untuk memilih objek itu, yang berguna ketika objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek ketika sel dipindahkan atau dibesarkan.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang memakai sesuatu selain gambar dari buku kerja, misalnya tautan ke berkas, tidak dapat disalin, dan JeffCrunch menjelaskan alasannya.

Salin bagan dan tempelkan ke buku kerja yang sama, pada lembar mana pun: salinan membaca sel yang sama dengan bagan pertama. Tempelkan ke buku kerja lain, dan bagan tetap menampilkan angka yang ia tampilkan, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkan, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi dengan Shift atau Ctrl untuk membatalkan pilihannya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** dan menyeret kotak di atas sel kosong: setiap objek yang sepenuhnya berada di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih sekaligus.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah ini dalam grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu berpindah dan berubah ukuran sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memecah kelompok yang dipilih. Setiap bagian kembali ke tempatnya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek diratakan terhadap tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi tiga objek atau lebih secara merata di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan objek dipilih, kotak nama menampilkan namanya. Ketik nama baru di sana lalu tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** dengan objek dipilih. Urungkan untuk mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini terbuka dengan cara yang sama di sana. Bagan yang berasal dari program lain dengan sesuatu yang tidak dapat dipertahankan JeffCrunch ketika menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen dan datanya tidak diubah di sini.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, misalnya sebuah proses, yang dibuat program lain. JeffCrunch menampilkan diagram dari gambarnya yang disimpan berkas di sebelahnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Saat Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, pai, donat, dan bagan sebaran, masing-masing dengan seri dan sel yang dibacanya (pada lembar ini atau lembar lain), kategori, judul, keterangan, sumbu (judul, nilai terendah dan tertinggi, format angka), label data, warna, dan tempatnya. Berkas .ods yang dibuat program lain dibuka dengan bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat dalam bagan .ods akan dihilangkan dan disebutkan saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, dan tampilan tambahan untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
