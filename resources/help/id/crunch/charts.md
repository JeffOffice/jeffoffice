# Bagan, sparkline, gambar, dan bentuk
<!-- keywords: tombol bidang, tombol bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, pie, lingkaran, area, sebar, kombinasi, keterangan, legenda, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sumbu tanggal, menang/kalah, bagan kecil, penanda, nama, teks alternatif, logaritmik, skala logaritma, simpan sebagai gambar, templat, crtx, teks judul, kunci legenda, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, lingkaran, persegi panjang, bintang, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area potong, kode batang, kode QR -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: ketika sebuah angka diubah, bagan ikut berubah. Gambar dan bentuk juga ditempatkan di atas sel, tetapi keduanya tidak terhubung dengan angka mana pun.

## Membuat bagan

1. Pilih angka yang ingin Anda buat bagannya. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenisnya: **Kolom**, **Batang**, **Garis**, **Pie**, **Area**, **Sebar**, atau **Kombinasi**. Tanda panah kecil di samping sebuah jenis menampilkan variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebar berupa titik, titik tersambung, atau garis halus.
3. Untuk melihat semua jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Belum yakin jenis mana yang sebaiknya dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel-sel tersebut dan menampilkan beberapa jenis yang sesuai, dengan yang paling cocok di urutan teratas. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan pie. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri disusun ke bawah pada kolom jika pilihan memiliki baris setidaknya sebanyak kolomnya, dan disusun ke samping pada baris jika pilihan memiliki lebih banyak kolom daripada baris. Pie hanya memiliki satu seri. Bagan sebar mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya berupa angka. Bagan tersebut memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 sampai Q3.

## Menyembunyikan seri untuk sementara

Pilih sebuah bagan. Tiga tombol kecil muncul di sampingnya. Tombol yang berbentuk corong adalah **Filter Bagan**. Tombol ini menampilkan daftar seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri itu dari bagan. Centang kembali untuk memunculkannya lagi di posisi yang sama. **Urungkan** juga berfungsi. Sebuah bagan selalu menyimpan setidaknya satu seri. Seri yang Anda keluarkan tetap ada di berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang kembali untuk memunculkannya. (Bagan donat menampilkan semua seri saat dibuka.)

## Memindahkan dan mengubah ukuran

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkan, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar sebuah tepi sejajar dengan tepi sel.
- Tahan **Shift** saat menyeret sudut untuk menjaga proporsinya.
- Tombol panah memindahkannya beberapa piksel setiap kali. **Ctrl** dengan panah memindahkannya satu piksel, **Shift** dengan panah membuatnya lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikannya ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat Anda urungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), untuk mengetik ukurannya, dan untuk memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: ikut dipindahkan dan diubah ukurannya bersama sel, ikut dipindahkan tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, di kotak yang sama, mematikan objek pada halaman cetakan (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi bagan

Saat sebuah bagan dipilih, dua tombol muncul di sampingnya dan tab **Bagan** muncul di pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, keterangan, dan label data, serta menentukan letak keterangan dan label.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu kumpulan warna dan salah satu tampilan dari JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan daftar seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca setiap seri, menentukan sel dengan label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi ditampilkan dalam bagan.
- **Tombol Bidang** (pada menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru langsung menampilkannya.
- **Bilah Kesalahan** (pada menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya dapat meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Deviasi Standar**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawah. Kosongkan salah satunya untuk menggambar bilah hanya di satu sisi. Bilah berubah ketika sel-sel tersebut berubah. Berkas yang sudah memiliki jumlahnya sendiri untuk setiap titik akan tetap mempertahankannya.
- **Garis** (pada menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke bawah sampai sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, serta **Bilah Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Bilah naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Bilah Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Garis Tren** (pada menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat arah perkembangannya. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase yang tetap, **Logaritmik** untuk angka yang naik dengan cepat lalu mendatar, **Polinomial** untuk kurva dengan lekukan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menuliskan rumus garis pada bagan, dan **Tampilkan R-kuadrat** menuliskan seberapa baik garis itu cocok (1 berarti sangat cocok; 0 berarti tidak cocok sama sekali). Garis tren mendapat entri sendiri di keterangan.
- Ketika sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Keterangan** memiliki **Di Atas Bagan**, yang meletakkan keterangan di atas area plot, bukan di sampingnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci legenda, yaitu kotak kecil berwarna sesuai seri, dalam kombinasi apa pun), **Pemisah** (apa yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0.0 untuk angka pada label; kosongkan untuk menampilkan angka apa adanya). **Nilai Dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel-selnya, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel-selnya berubah. Kosongkan untuk menghapusnya. Matikan semua isi label dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Di sana Anda dapat mengatur nilai terendah dan tertinggi serta satuan utama dan kecil (masing-masing otomatis atau angka yang Anda ketik), apakah nilai ditampilkan dengan urutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 tampil sebagai 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, seperti Ribuan, bersama judul sumbu), apakah skalanya logaritmik (setiap tanda merupakan basis dikali tanda sebelumnya, seperti 1, 10, 100, dan 1000, yang cocok untuk angka yang ukurannya sangat berbeda dan semuanya harus lebih dari 0), di mana sumbu lain memotongnya, format angka labelnya, tanda pembagi, letak labelnya, serta seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan tempat bagan harus diletakkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagannya sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan dipindahkan ke lembar kerja tersebut. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkannya akan dihapus. Bagan tetap membaca sel yang dibacanya sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan tempatnya di lembar, meskipun baris dan kolomnya lebih tinggi atau lebih lebar, dan ikut dipindahkan serta diubah ukurannya bersama sel di sana seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini adalah satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika sebuah lembar dihapus olehnya. Berkas berformat `.ods` tidak dapat menyimpan lembar bagan, jadi simpan lembar bagan dalam format `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (pada menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Ketika sel berubah, judul ikut berubah. Berkas menyimpan tautannya.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada ukurannya di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya kepada bagan lain. Tampilan tersebut meliputi jenis, warna, pengaturan judul dan keterangan, sumbu, serta font. Bagan yang menerimanya tetap memakai angka dan sel yang dibacanya sendiri. Satu **Urungkan** menghapus kembali tampilan tersebut.
- **Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di antara tanda kutip tunggal jika namanya mengandung spasi.

## Sparkline

Sparkline adalah bagan kecil yang muat di dalam satu sel. Sparkline menampilkan kecenderungan sebuah baris atau kolom angka tepat di sebelah angka tersebut.

1. Pilih angka-angkanya. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka-angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya berukuran sama, yang cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca seluruh data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok, dan mereka berbagi satu tampilan yang sama. Klik sel yang berisi sparkline, dan tab **Sparkline** akan muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (biarkan celah, anggap sebagai nol, atau hubungkan titik melewatinya) dan apakah sel di baris dan kolom tersembunyi ikut dibaca.
- **Jenis** beralih di antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garisnya, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, menggambar angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** mengatur jarak titik berdasarkan tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal yang lebih akhir akan berada lebih jauh ke kanan. Kosongkan untuk mengatur ulang jarak titik secara merata.
- **Kelompokkan** memasukkan sparkline yang dipilih ke dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka pemilih berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG) dan gambar itu akan muncul di lembar pada sel yang dipilih. Gambar yang lebarnya lebih dari 480 titik diperkecil agar muat, dan tetap mempertahankan bentuknya. Buku kerja menyimpan gambarnya sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (misalnya SVG) disimpan sebagai PNG, yaitu gambar biasa yang terdiri dari titik. Berkas yang bukan gambar, atau yang lebih besar dari 16 MB, akan ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat beberapa program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi kumpulan gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau beberapa ikon, dan masing-masing akan muncul di lembar sebagai gambar selebar dan setinggi 48 poin. Ikon digambar dengan warna abu-abu gelap dalam ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak sekaligus): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angka, dan jendela akan menampilkan kode batang saat Anda mengetik, serta menjelaskan dengan warna merah jika ada yang salah dan kode tidak dapat dibuat. Klik **Sisipkan**, dan kode batang akan muncul di lembar sebagai gambar, yang dapat Anda urungkan sebagai satu langkah. Pindahkan dan ubah ukurannya seperti gambar lainnya. Untuk mengubahnya, pilih kode batang itu dan klik **Kode Batang** lagi, ubah teksnya, lalu klik **Perbarui**; kode batang baru menggantikan posisi dan lebar yang lama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam, sehingga setiap program dapat menampilkannya. JeffCrunch juga menyimpan pengaturannya dalam berkas, sehingga kode batang tetap dapat diedit saat Anda membuka kembali buku kerja di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel yang berisi data dari sel-sel yang dipilih, misalnya untuk lembar label. Pilih sel-selnya, klik perintah itu, lalu pilih jenis dan tampilannya di jendela yang sama. Apa pun yang Anda ketik di sana hanyalah contoh, karena teks setiap sel akan menjadi kode batangnya sendiri. Kode batang diletakkan dalam satu kolom tepat di kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat Anda urungkan. Sel yang teksnya tidak dapat dimuat kode (misalnya huruf pada EAN-13) akan disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel diproses sekaligus; pilih lebih sedikit jika ada pesan yang menyatakan terlalu banyak. Setiap kode batang mengingat sel asalnya dan mengikutinya: ketika Anda mengubah sel itu, kode batang dibuat ulang dari teks yang baru, di tempat dan ukuran yang sama, sebagai langkah tersendiri yang dapat Anda urungkan. Tautan itu ikut berpindah bersama sel ketika Anda menyisipkan atau menghapus baris atau kolom. Jika Anda menghapus sel, kode batang tetap menyimpan teks terakhirnya. Saat Anda menyimpan, gambar dalam berkas dibuat dari teks sel pada saat itu, sehingga program lain juga menampilkan nilai terkini.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, lingkaran, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih salah satu dan bentuk itu akan muncul di lembar dengan warna biru muda dan tepi yang lebih gelap. **Kotak Teks**, pada grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi.

Saat sebuah bentuk dipilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, misalnya biru muda dengan tepi biru tua. Dua pilihan terakhir hanya menyimpan garis tepi atau hanya isian. Satu klik adalah satu langkah yang dapat Anda urungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis secara daring

**Gambar Daring**, dalam grup **Ilustrasi** pada tab **Sisipkan**, menemukan gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Sebuah panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik sebuah gambar untuk melihat siapa pembuatnya dan lisensinya. **Lisensi** menjelaskan bagaimana Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik ganda gambar).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya ketika Anda mengklik **Cari** atau menekan Enter. JeffCrunch kemudian meminta pustaka tersebut untuk pratinjau kecil dari hasil yang ditemukan, serta gambar tunggal yang Anda sisipkan. Seperti halaman web lainnya, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang meminta. Tidak ada isi buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda menyebutkan nama pembuatnya. Ketika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar yang berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya dimasukkan sebagai satu langkah, sehingga **Urungkan** membatalkan keduanya. Pastikan lisensinya sesuai dengan cara Anda akan menggunakan gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf berwarna, dengan garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Kata "Your Text Here" muncul di lembar dengan gaya tersebut.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak akan meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata-kata menjadi bentuk, seperti gelombang atau lengkung. "Biasa" membuatnya lurus kembali.

Seni Teks adalah bentuk dalam gambar lembar, lengkap dengan lengkung, isian, dan garis tepinya, sehingga tampilannya tetap sama saat Anda menyimpan buku kerja sebagai berkas .xlsx lalu membukanya kembali, baik di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungnya, dan kembali menjadi Seni Teks saat Anda membuka berkas .ods lagi. Beberapa lengkung yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, serta bentuk bawah dan atas dari Membesar dan Mengecil) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera dapat jauh lebih besar daripada yang ditampilkan di lembar, dan membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** pada grup **Sesuaikan** di tab **Format Gambar**. Pilih tempat buku kerja akan digunakan: **Cetak komersial (300 ppi)**, **Cetak Desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk memperkecil hanya gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghapus bagian tersembunyi yang telah Anda potong, sehingga potongan itu hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap sama. Urungkan (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya berisi gambar yang lebih kecil. Gambar pada lembar lain tidak berubah: buka setiap lembar dan ulangi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah ini dapat Anda urungkan. Objek baru akan dipilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya maju atau mundur, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** akan muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** akan muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek tetap mempertahankan nama, teks alternatif, tempat, serta urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tidak sama sekali. **Gaya Garis** mengatur ketebalan (seberapa tebal garisnya) dan garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, lebar 4 tinggi 3, lebar 3 tinggi 4, 16 banding 9, atau 9 banding 16. Pemotongan dilakukan secara merata dari tengah bagian yang tampil, dan gambar diperbesar sebesar sisa yang tertinggal. **Tampilkan Seluruh Gambar** mengembalikan bagian yang terpotong. Berkas gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih cara letaknya secara horizontal dan vertikal, ukurannya, tebal atau biasa, serta apakah kata-kata mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, perintah ini meminta kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek di lembar berdasarkan nama, dengan objek paling depan di urutan pertama. Klik sebuah nama untuk memilih objek itu, yang berguna saat objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, serta apa yang dilakukan objek ketika sel dipindahkan atau dibuat lebih besar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang memakai sesuatu selain gambar dari buku kerja, seperti tautan ke berkas, tidak dapat disalin, dan JeffCrunch akan memberi tahu Anda alasannya.

Salin bagan dan tempelkan ke buku kerja yang sama, pada lembar mana pun: salinan itu membaca sel yang sama dengan bagan pertama. Jika ditempelkan ke buku kerja lain, bagan tetap menampilkan angka yang dulu ditampilkannya tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkannya, dan merapikannya

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi sambil menahan Shift atau Ctrl untuk membatalkan pilihannya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** lalu menyeret kotak di atas sel kosong: setiap objek yang seluruhnya berada di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah berikut pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu dipindahkan dan diubah ukurannya sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memecah kelompok yang dipilih. Setiap bagian kembali ke tempatnya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah**, dan **Rata Bawah**. Objek diratakan dengan tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi rata tiga objek atau lebih di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan objek yang dipilih, kotak nama menampilkan nama objek tersebut. Ketik nama baru di sana dan tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus objek

Tekan **Delete** dengan objek terpilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini dapat dibuka di sana dengan cara yang sama. Bagan yang berasal dari program lain dan memiliki sesuatu yang tidak dapat dipertahankan JeffCrunch saat menulis ulang bagan (bilah kesalahan, atau bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen dan datanya tidak diubah di sini.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, seperti sebuah proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sampingnya. Klik diagram untuk memilihnya, dan seret untuk memindahkannya. Saat Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, pie, donat, dan bagan sebar, masing-masing dengan seri dan sel yang dibacanya (pada lembar ini atau lembar lain), kategori, judul, keterangan, sumbu (judul, nilai terendah dan tertinggi, format angka), label data, warna, serta tempatnya. Berkas .ods yang dibuat program lain dibuka beserta bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat dalam bagan .ods akan dihilangkan dan disebutkan namanya saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, dan tampilan khusus untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
