# Bagan, sparkline, gambar, dan bentuk
<!-- keywords: tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan rekomendasi, saran bagan, bagan, grafik, kolom, batang, garis, lingkaran, area, sebar, kombinasi, keterangan, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, teks alternatif, skala logaritma, log, simpan sebagai gambar, templat, crtx, teks judul, kunci keterangan, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, lingkaran, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area potongan, kode batang, kode QR -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: jika angka diubah, bagan ikut berubah. Gambar dan bentuk diletakkan di atas sel dengan cara yang sama, tetapi keduanya tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda jadikan bagan. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Lingkaran**, **Area**, **Sebar**, atau **Kombinasi**. Panah kecil di samping suatu jenis berisi variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebar berupa titik, titik yang disambung, atau garis halus.
3. Untuk melihat setiap jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Tidak yakin harus memilih jenis yang mana? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel dan mendaftarkan beberapa jenis yang cocok, dari yang paling sesuai. Misalnya, tanggal di sisi bawah menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan lingkaran. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri disusun menurut kolom jika pilihan memiliki baris setidaknya sama banyak dengan kolom, dan menurut baris jika kolomnya lebih banyak daripada baris. Lingkaran hanya memiliki satu seri. Bagan sebar mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, dengan A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya berupa angka. Bagan memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 sampai Q3.

## Sembunyikan seri untuk sementara

Pilih bagan. Tiga tombol kecil muncul di sampingnya. Tombol berbentuk corong adalah **Filter Bagan**. Tombol ini menampilkan seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri dari bagan. Centang kembali untuk memunculkannya di posisi yang sama. **Urungkan** juga berfungsi. Bagan selalu menyimpan setidaknya satu seri. Seri yang Anda keluarkan tetap ada di berkas saat Anda menyimpan, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang kembali untuk memunculkannya. (Bagan donat menampilkan semua seri saat dibuka.)

## Memindahkan dan mengubah ukurannya

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkannya, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** saat menyeret agar tepi objek sejajar dengan tepi sel.
- Tahan **Shift** saat menyeret sudut untuk mempertahankan proporsi.
- Tombol panah menggeser objek beberapa piksel sekali jalan. **Ctrl** dengan panah menggesernya satu piksel, **Shift** dengan panah membuatnya lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat diurungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek di lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan objek kepada orang yang tidak dapat melihatnya), untuk mengetik ukurannya, dan untuk memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berubah ukuran bersama sel, bergerak bersama sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman cetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi bagan

Saat bagan dipilih, dua tombol muncul di sampingnya dan tab **Bagan** muncul di pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, keterangan, dan label data, serta menentukan letak keterangan dan label.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan dari JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca masing-masing seri, mengatur sel berisi label, menukar baris dan kolom, serta memilih cara bagan memperlakukan sel kosong dan baris atau kolom tersembunyi.
- **Tombol Bidang** (pada menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang dari tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (pada menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Deviasi Standar**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawahnya. Biarkan satu kosong untuk menggambar bilah hanya di satu sisi. Bilah berubah ketika sel tersebut berubah. Berkas yang sudah memiliki jumlahnya sendiri untuk setiap titik tetap mempertahankannya.
- **Garis** (pada menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Batang Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Batang naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Batang Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Garis Tren** (pada menu **+**) menggambar garis yang disesuaikan dengan angka setiap seri, sehingga Anda dapat melihat arah angka-angka tersebut. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase yang tetap, **Logaritmik** untuk angka yang naik cepat lalu melandai, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menulis rumus garis pada bagan, dan **Tampilkan R-kuadrat** menulis seberapa baik garis itu cocok (1 berarti cocok sempurna; 0 berarti sama sekali tidak cocok). Garis tren mendapat entri tersendiri di keterangan.
- Bila sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Keterangan** memiliki **Di Atas Bagan**, yang meletakkan keterangan di atas area plot, bukan di sampingnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci keterangan, yaitu kotak kecil berwarna sesuai seri, dalam kombinasi apa pun), **Pemisah** (yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0,0 untuk angka dalam label; biarkan kosong untuk menampilkan angka apa adanya). **Nilai Dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel berubah. Biarkan kosong untuk menghapusnya. Matikan semua isi dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Kotak ini mengatur nilai terendah dan tertinggi serta satuan utama dan minor (masing-masing otomatis atau angka yang Anda ketik), apakah nilai ditampilkan dalam urutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dibagi 1.000 untuk ribuan, sehingga 30.000 terbaca 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, misalnya Ribuan, bersama judul sumbu), apakah skala logaritmik (setiap tanda adalah basis dikali tanda sebelumnya, misalnya 1, 10, 100, dan 1000, yang cocok untuk angka yang berbeda jauh besarnya dan semuanya harus lebih dari 0), di mana sumbu lain memotong sumbu ini, format angka labelnya, tanda sumbunya, letak labelnya, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan lokasi tujuan bagan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** lalu pilih lembar kerja, dan bagan dipindahkan ke lembar kerja tersebut. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkannya akan dihapus. Bagan tetap membaca sel yang sama seperti sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan posisinya di lembar, meskipun baris dan kolomnya lebih tinggi atau lebih lebar, serta bergerak dan berubah ukuran bersama sel di sana seperti sebelumnya. Lembar yang tersembunyi tidak ditawarkan. Ini adalah satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika lembar dihapus olehnya. Berkas berformat `.ods` tidak dapat menyimpan lembar bagan, jadi simpan lembar bagan dalam format `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (pada menu **+**) juga memiliki **Teks Judul…**. Ketik kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, misalnya **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Ketika sel berubah, judul ikut berubah. Berkas menyimpan tautan ini.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya ke bagan lain. Tampilan mencakup jenis, warna, pengaturan judul dan keterangan, sumbu, serta font. Bagan yang menerimanya tetap mempertahankan angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus kembali tampilan tersebut.
- **Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di antara tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan mini yang muat di dalam satu sel. Sparkline menampilkan tren dari satu baris atau kolom angka tepat di sebelahnya.

1. Pilih angka. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya berukuran sama, sehingga cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline diletakkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca semua data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok dan berbagi satu tampilan. Klik sel yang berisi sparkline, dan tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (biarkan celah, anggap nol, atau sambungkan titik di antaranya) dan apakah sel di baris atau kolom tersembunyi ikut dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang dan **Ketebalan** (seberapa tebal garis, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis pada nol, membaca angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala yang sama, sehingga dapat dibandingkan. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** mengatur jarak titik berdasarkan tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal lebih baru akan berada lebih jauh ke kanan. Biarkan kosong untuk mengatur jarak titik kembali merata.
- **Kelompokkan** menempatkan sparkline yang dipilih dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat diurungkan. Sparkline disimpan dalam berkas dengan format yang dibaca program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka jendela pemilihan berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG) dan gambar akan muncul di lembar pada sel yang dipilih. Gambar yang lebih lebar dari 480 titik diperkecil agar muat, dan bentuknya tetap dipertahankan. Buku kerja menyimpan gambar itu sendiri. Gambar berjenis yang tidak dapat disimpan buku kerja apa adanya (misalnya SVG) disimpan sebagai PNG, yaitu gambar biasa yang terdiri dari titik. Berkas yang bukan gambar, atau yang berukuran lebih dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi kumpulan gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau lebih, dan masing-masing akan muncul di lembar sebagai gambar selebar dan setinggi 48 poin. Ikon digambar dengan warna abu-abu tua dalam ukuran yang tetap tajam saat diperbesar. Setiap ikon adalah satu langkah yang dapat diurungkan.

**Kode Batang** membuka menu singkat dengan dua pilihan. **Kode Batang** membuka jendela untuk membuat satu kode batang (**Kode Batang dari Sel**, di bawah, membuat banyak): ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa pun, seperti alamat web. Pilih **Jenis**, ketik teks atau angka, dan jendela akan menampilkan kode batang saat Anda mengetik. Jika kode tidak dapat dibuat, jendela menjelaskan kesalahannya dengan teks merah. Klik **Sisipkan**, dan kode batang akan muncul di lembar sebagai gambar, yang dapat diurungkan dalam satu langkah. Pindahkan dan ubah ukurannya seperti gambar lainnya. Untuk mengubahnya, pilih kode batang lalu klik **Kode Batang** lagi, ubah teksnya, dan klik **Perbarui**; kode baru menggantikan posisi dan lebar kode lama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam sehingga semua program dapat menampilkannya, dan JeffCrunch menyimpan pengaturannya dalam berkas, sehingga kode batang tetap dapat diedit saat Anda membuka buku kerja lagi di JeffCrunch.

**Kode Batang > Kode Batang dari Sel** (pada menu **Kode Batang** yang sama) membuat kode batang untuk setiap sel berisi pada sel yang dipilih, untuk lembar label. Pilih sel, klik perintah itu, lalu pilih jenis dan tampilan pada jendela yang sama. Apa pun yang Anda ketik di sana hanyalah contoh, karena teks setiap sel akan menjadi kode batangnya sendiri. Kode batang ditempatkan dalam satu kolom tepat di sebelah kanan sel yang dipilih, satu di bawah yang lain, sebagai satu langkah yang dapat diurungkan. Sel yang teksnya tidak dapat dimuat kode (misalnya huruf dalam EAN-13) akan disebutkan dalam pesan dan tidak mendapat kode batang. Paling banyak 500 sel diproses sekaligus; pilih lebih sedikit jika ada pesan bahwa terlalu banyak. Setiap kode batang adalah gambar biasa yang dibuat dari teks sel saat Anda mengklik: jika Anda mengubah sel nanti, kode batang tidak mengikutinya, jadi pilih sel dan buat ulang.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, lingkaran, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih salah satu dan bentuk itu muncul di lembar dengan isian biru muda dan tepi biru yang lebih gelap. **Kotak Teks**, pada grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi.

Dengan bentuk terpilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, misalnya biru muda dengan tepi biru tua. Dua pilihan terakhir hanya menyimpan garis tepi atau hanya isian. Satu klik adalah satu langkah yang dapat diurungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis di internet

**Gambar Daring**, pada grup **Ilustrasi** tab **Sisipkan**, mencari gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Panel terbuka di sisi kanan.
2. Pilih pustaka, ketik apa yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik gambar untuk melihat siapa pembuatnya dan apa lisensinya. **Lisensi** menjelaskan bagaimana Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik ganda gambar).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya saat Anda mengklik **Cari** atau menekan Enter. Kemudian JeffCrunch meminta pustaka tersebut mengirim pratinjau kecil dari hasil pencarian, dan gambar yang Anda sisipkan. Seperti halaman web lainnya, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch yang meminta. Tidak ada isi buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar yang berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya dimasukkan sebagai satu langkah, sehingga **Urungkan** menarik keduanya kembali. Pastikan lisensinya sesuai dengan cara Anda akan menggunakan gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf dengan warna, garis tepi, bayangan, atau bentuk melengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik satu gaya. Kata "Teks Anda di Sini" muncul di lembar dengan gaya tersebut.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkung. "Biasa" membuat kata tetap lurus.

Seni Teks adalah bentuk dalam gambar lembar, dengan lengkungan, isian, dan garis tepinya, sehingga tampilannya tetap sama saat Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya lagi, di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat diurungkan. Buku kerja yang disimpan sebagai berkas .ods juga mempertahankan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungannya, dan kembali sebagai Seni Teks saat Anda membuka berkas .ods lagi. Beberapa lengkungan yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, dan bentuk bawah dan atas dari Menggembung dan Mengempis) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada yang tampak di lembar, dan membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** pada grup **Sesuaikan** tab **Format Gambar**. Pilih tempat buku kerja akan digunakan: **Cetak komersial (300 ppi)**, **Cetak desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar di lembar, atau hapus centangnya untuk hanya memperkecil gambar yang dipilih. **Hapus area potongan gambar** juga menghapus bagian tersembunyi yang telah Anda potong, sehingga potongannya hilang. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti semula. Urungkan (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya berisi gambar yang lebih kecil. Gambar pada lembar lain tidak berubah: buka setiap lembar dan lakukan lagi. Gambar berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah ini dapat diurungkan dalam satu kali. Objek baru akan dipilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau ke belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat diurungkan, dan objek tetap mempertahankan nama, teks alternatif, tempat, serta urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garis) dan garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, lebar 4 tinggi 3, lebar 3 tinggi 4, 16 banding 9, atau 9 banding 16. Pemotongan dilakukan secara merata dari tengah bagian yang tampak, dan gambar dibesarkan sebanyak mungkin dari sisa potongan. **Tampilkan Seluruh Gambar** mengembalikan bagian yang terpotong. Berkas gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih cara letaknya secara mendatar dan vertikal, ukurannya, tebal atau biasa, dan apakah teks mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, perintah ini meminta kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek di lembar menurut nama, dengan objek paling depan di atas. Klik nama untuk memilih objek tersebut, yang berguna jika objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek saat sel dipindahkan atau diperbesar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang menggunakan sesuatu selain gambar dari buku kerja, misalnya tautan ke suatu berkas, tidak dapat disalin, dan JeffCrunch akan memberi tahu alasannya.

Salin bagan dan tempel ke buku kerja yang sama, di lembar mana pun: salinan membaca sel yang sama dengan bagan pertama. Tempel ke buku kerja lain dan bagan tetap menyimpan angka yang ditampilkannya, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukanlah sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkan, dan meratakan

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi dengan Shift atau Ctrl untuk membatalkannya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** dan menyeret kotak di atas sel kosong: setiap objek yang berada sepenuhnya di dalam kotak akan dipilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah ini pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu bergerak dan berubah ukuran sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memisahkan kelompok yang dipilih. Setiap bagian kembali ke posisinya di lembar.
- **Rata** meratakan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek diratakan ke tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** membagi tiga objek atau lebih secara merata di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat diurungkan.

## Memberi nama

Dengan objek dipilih, kotak nama menampilkan namanya. Ketik nama baru di sana lalu tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** saat objek dipilih. Urungkan mengembalikannya.

## Apa yang disimpan

Semua hal di atas disimpan dalam berkas dengan format yang dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini dapat dibuka di sana dengan cara yang sama. Bagan yang berasal dari program lain dengan sesuatu yang tidak dapat dipertahankan JeffCrunch saat menulis ulang bagan (bilah kesalahan, bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen serta datanya tidak diubah di sini.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, misalnya sebuah proses, yang dibuat oleh program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sampingnya. Klik untuk memilih diagram, dan seret untuk memindahkannya. Saat Anda menyimpan berkas, diagram ditulis kembali persis seperti semula, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap mempertahankan bagannya: kolom, batang, garis, area, lingkaran, donat, dan bagan sebar, masing-masing dengan seri dan sel yang dibacanya (di lembar ini atau lembar lain), kategori, judul, keterangan, sumbu (judul, nilai terendah dan tertinggi, format angka), label data, warna, serta letaknya. Berkas .ods yang dibuat program lain dibuka beserta bagannya, digambar dari sel yang dibacanya. Apa pun yang tidak memiliki padanannya dalam bagan .ods akan dihilangkan dan disebutkan namanya saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, dan tampilan khusus untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
