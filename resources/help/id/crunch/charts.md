# Bagan, sparkline, gambar, dan bentuk

<!-- keywords: tombol bidang, tombol bidang bagan pivot, kelompokkan, pisahkan kelompok, rata, rata kiri, distribusikan, pilih objek, beberapa bentuk, pilih beberapa, kotak seret, filter bagan, sembunyikan seri, bagan yang direkomendasikan, saran bagan, bagan, grafik, kolom, batang, garis, pai, area, sebar, kombinasi, keterangan, judul, garis kisi, label data, data bagan, jenis bagan, tampilan, tata letak, pindahkan, ubah ukuran, gambar, bentuk, objek, seri, garis tren, persamaan, R-kuadrat, sumbu kedua, sparkline, sparkline, sumbu tanggal, menang/kalah, bagan mini, penanda, nama, teks alternatif, teks alternatif, logaritmik, skala log, simpan sebagai gambar, templat, crtx, teks judul, kunci keterangan, panel pilihan, isian bentuk, garis tepi bentuk, putar, balik, potong, edit teks, bingkai gambar, sisipkan gambar, ikon, wmf, emf, gaya bentuk, gaya, sisipkan bentuk, kotak teks, bentuk, panah, oval, persegi panjang, bintang, diagram, diagram, gambar daring, gambar gratis, openverse, wikimedia commons, lisensi, kredit, seni teks, wordart, lengkung, kompres gambar, perkecil gambar, ukuran berkas, ppi, hapus area yang dipotong, kode batang, kode QR -->

Bagan adalah gambar dari angka yang tetap terhubung dengannya: jika Anda mengubah angka, bagan ikut berubah. Gambar dan bentuk ditempatkan di atas sel dengan cara yang sama, tetapi keduanya tidak terhubung dengan angka apa pun.

## Membuat bagan

1. Pilih angka yang ingin Anda jadikan bagan. Jika baris pertama berisi nama dan kolom pertama berisi label, pilih juga keduanya. JeffCrunch memeriksa baris pertama dan kolom pertama, lalu menggunakannya sebagai nama dan label jika berupa kata (atau tanggal, untuk label).
2. Pada tab **Sisipkan**, di grup **Bagan**, pilih jenis: **Kolom**, **Batang**, **Garis**, **Pai**, **Area**, **Sebar**, atau **Kombinasi**. Tanda panah kecil di sebelah jenis memuat variasinya, seperti kolom bertumpuk, garis dengan penanda, atau bagan sebar berupa titik, titik tersambung, atau garis halus.
3. Untuk melihat semua jenis terlebih dahulu, pilih **Bagan Lainnya**. Daftar di sebelah kiri menampilkan setiap jenis yang digambar dari sel Anda. **Tukar baris dan kolom** membalik arah seri.

Belum yakin jenis mana yang dipilih? Pilih sel Anda lalu pilih **Bagan yang Direkomendasikan** pada tab **Sisipkan**. JeffCrunch memeriksa sel-sel tersebut dan mendaftarkan beberapa jenis yang cocok, dengan yang paling sesuai di urutan pertama. Misalnya, tanggal di sisi kiri menyarankan garis, dan satu kolom berisi beberapa angka positif dengan label menyarankan pai. Setiap jenis digambar dari sel Anda. Pilih salah satu lalu pilih **OK**.

Jika Anda hanya memilih satu sel, JeffCrunch membuat bagan dari blok sel di sekitarnya yang berisi data.

Seri disusun menurut kolom jika pilihan memiliki baris setidaknya sama banyak dengan kolom, dan menurut baris jika pilihan memiliki lebih banyak kolom daripada baris. Pai hanya memiliki satu seri. Bagan sebar mengambil nilai horizontalnya dari kolom pertama.

Contoh: Anda memilih A1:C4, di mana A2:A4 berisi Q1, Q2, dan Q3, B1 dan C1 berisi East dan West, dan sisanya angka. Bagan memiliki dua seri, East dan West, serta tiga kelompok kolom, Q1 sampai Q3.

## Sembunyikan seri untuk sementara

Pilih bagan. Tiga tombol kecil muncul di sebelahnya. Tombol dengan corong adalah **Filter Bagan**. Tombol ini menampilkan daftar seri bagan, masing-masing dengan kotak centang. Hapus centang untuk mengeluarkan seri dari bagan. Centang lagi untuk memunculkannya kembali di posisi yang sama. **Urungkan** juga berfungsi. Bagan selalu menyimpan setidaknya satu seri. Seri yang Anda keluarkan tetap ada dalam berkas saat Anda menyimpannya, dan tetap tersembunyi saat berkas dibuka, sehingga tidak ada yang hilang. Centang lagi untuk memunculkannya kembali. (Bagan donat menampilkan semua seri saat dibuka.)

## Memindahkan dan mengubah ukuran

Klik bagan, gambar, atau bentuk untuk memilihnya. Seret untuk memindahkan, atau seret salah satu dari delapan kotak di tepinya untuk mengubah ukurannya.

- Tahan **Alt** sambil menyeret agar tepi objek rata dengan tepi sel.
- Tahan **Shift** sambil menyeret sudut untuk menjaga proporsinya.
- Tombol panah memindahkan objek beberapa piksel setiap kali. **Ctrl** dengan panah memindahkannya satu piksel, **Shift** dengan panah membuatnya lebih besar atau lebih kecil, dan **Alt** dengan panah berpindah ke tepi sel berikutnya.
- **Esc** saat menyeret mengembalikan objek ke posisi semula.

Setiap pemindahan atau perubahan ukuran adalah satu langkah yang dapat Anda urungkan. Klik kanan objek lalu pilih **Properti** untuk mengetik **Nama** (setiap objek pada lembar harus memiliki nama sendiri) dan **Teks alternatif** (kalimat singkat yang menjelaskan kepada orang yang tidak dapat melihat objek tersebut apa yang ditampilkannya), untuk mengetik ukurannya, dan untuk memilih apa yang dilakukan objek saat Anda menyisipkan, menghapus, atau mengubah ukuran sel: bergerak dan berubah ukuran mengikuti sel, bergerak mengikuti sel tetapi mempertahankan ukurannya, atau tetap di tempatnya. Pilihan ini disimpan dalam berkas. **Cetak objek ini**, dalam kotak yang sama, mematikan objek pada halaman cetak (objek tetap tampil di layar); berkas juga menyimpan pilihan ini.

## Mengubah isi bagan

Saat bagan dipilih, dua tombol muncul di sebelahnya dan tab **Bagan** muncul pada pita.

- Tombol **+** (dan **Bagian Bagan** pada tab) menyalakan atau mematikan judul bagan, sumbu, judul sumbu, garis kisi, keterangan, dan label data, serta menentukan tempat keterangan dan label.
- Tombol kuas (dan **Warna** serta **Tampilan** pada tab) memberi bagan salah satu set warna dan salah satu tampilan dari JeffCrunch. **Tata Letak** mengatur beberapa elemen sekaligus.
- **Data Bagan** menampilkan seri bagan. Anda dapat menambah, mengedit, menghapus, dan mengurutkan ulang seri, mengubah sel yang dibaca setiap seri, menetapkan sel yang berisi label, menukar baris dan kolom, serta memilih cara sel kosong dan baris atau kolom tersembunyi ditampilkan dalam bagan.
- **Tombol Bidang** (pada menu **+**, hanya untuk bagan pivot) menampilkan tombol kecil dengan nama bidang dari tabel pivot yang diikuti bagan: filter laporan di bawah judul, serta baris, nilai, dan kolom di sepanjang bagian bawah. Nyalakan atau matikan setiap jenis, atau pilih **Tampilkan Semua** atau **Sembunyikan Semua**. Bagan pivot baru menampilkannya.
- **Bilah Kesalahan** (pada menu **+**, untuk bagan kolom, batang, garis, area, atau sebar) menggambar garis kecil di atas dan di bawah setiap titik untuk menunjukkan seberapa jauh nilai sebenarnya bisa meleset. Pilih **Kesalahan Standar**, **Persentase (5%)**, **Deviasi Standar**, atau **Nilai Tetap (1)**. Pilih **Kustom** untuk memakai jumlah Anda sendiri: ketik sel yang berisi jumlah di atas setiap titik (misalnya `=Sheet1!$D$2:$D$4`), lalu sel untuk jumlah di bawahnya. Kosongkan salah satunya untuk menggambar bilah hanya di satu sisi. Bilah berubah ketika sel-sel tersebut berubah. Berkas yang sudah memiliki jumlah sendiri untuk setiap titik tetap menyimpannya.
- **Garis** (pada menu **+**, untuk bagan garis) memiliki **Garis Jatuh**, yang membentang dari titik tertinggi setiap kategori ke sumbu, **Garis Tinggi-Rendah**, yang menghubungkan titik tertinggi dan terendah setiap kategori, dan **Batang Naik/Turun**, yaitu kotak di antara seri pertama dan terakhir setiap kategori. Batang naik menunjukkan bahwa seri terakhir lebih tinggi. Garis Tinggi-Rendah dan Batang Naik/Turun memerlukan dua seri atau lebih. Bagan area memiliki Garis Jatuh.
- **Garis Tren** (pada menu **+**) menggambar garis yang dicocokkan dengan angka setiap seri, sehingga Anda dapat melihat arah angka tersebut. Pilih **Linear** untuk garis lurus, **Eksponensial** untuk angka yang bertambah dengan persentase tetap, **Logaritmik** untuk angka yang naik cepat lalu mendatar, **Polinomial** untuk kurva dengan lengkungan, **Pangkat**, atau **Rata-Rata Bergerak**, yang menggambar rata-rata dari beberapa angka terakhir. **Tampilkan Persamaan** menuliskan rumus garis pada bagan, dan **Tampilkan R-kuadrat** menuliskan seberapa baik garis itu cocok (1 berarti sangat cocok; 0 berarti tidak cocok sama sekali). Garis tren mendapat entri sendiri di keterangan.
- Ketika sebuah seri digambar terhadap sumbu vertikal kedua, menu **Sumbu** dan **Judul Sumbu** memiliki entri **Horizontal Kedua** dan **Vertikal Kedua** untuk seri tersebut. Menu **Keterangan** memiliki **Di Atas Bagan**, yang meletakkan keterangan di atas area plot, bukan di sampingnya.
- **Label Data** juga memiliki **Label Berisi** (nilai, nama kategori, nama seri, persentase, dan kunci keterangan, yaitu kotak kecil dengan warna seri, dalam kombinasi apa pun), **Pemisah** (apa yang berada di antaranya: koma, titik koma, titik, baris baru, atau spasi), dan **Format Angka…** (kode format seperti 0.0 untuk angka dalam label; kosongkan untuk menampilkan angka apa adanya). **Nilai Dari Sel…** menampilkan kata-kata dari beberapa sel sebagai label, satu sel untuk setiap titik. Ketik sel-selnya, misalnya =Sheet1!$D$2:$D$4. Label berubah ketika sel-selnya berubah. Kosongkan untuk menghapus label. Matikan semua isi dan label akan hilang.
- **Format Sumbu Horizontal…** dan **Format Sumbu Vertikal…** pada tab **Bagan** membuka kotak untuk satu sumbu. Kotak ini mengatur nilai terendah dan tertinggi serta satuan utama dan minor (masing-masing otomatis atau angka yang Anda ketik), apakah nilai ditampilkan dengan urutan terbalik, satuan tampilan (label menampilkan angka yang dibagi, misalnya dengan 1.000 untuk ribuan, sehingga 30.000 terbaca sebagai 30; centang **Tampilkan nama satuan pada bagan** untuk mencetak nama, misalnya Ribuan, bersama judul sumbu), apakah skalanya logaritmik (setiap tanda adalah basis dikali tanda sebelumnya, misalnya 1, 10, 100, dan 1000, yang cocok untuk angka yang berbeda jauh besarnya dan semuanya harus lebih dari 0), di mana sumbu lain memotongnya, format angka labelnya, tanda sumbunya, letak labelnya, dan seberapa jauh label diputar. Hanya sumbu angka yang memiliki batas dan satuan.
- **Pindahkan Bagan…** pada tab **Bagan** menanyakan di mana bagan sebaiknya ditempatkan. Pilih **Lembar baru** dan ketik nama, maka bagan mendapat lembar bagan sendiri: lembar yang hanya berisi bagan, tanpa sel. Pilih **Objek di** dan pilih lembar kerja, maka bagan akan ditempatkan di lembar kerja tersebut. Bagan pada lembar bagan hanya dapat dipindahkan ke lembar kerja, dan lembar bagan yang ditinggalkannya akan dihapus. Bagan tetap membaca sel yang sama seperti sebelumnya. Pada lembar kerja, bagan mempertahankan ukuran dan posisinya di lembar, bahkan ketika baris dan kolomnya lebih tinggi atau lebih lebar, dan ikut bergerak serta berubah ukuran mengikuti sel di sana seperti sebelumnya. Lembar yang disembunyikan tidak ditawarkan. Ini hanya satu **Urungkan**, bahkan setelah Anda menyimpan, kecuali jika lembar dihapus olehnya. Berkas dalam bentuk `.ods` tidak dapat menyimpan lembar bagan, jadi simpan lembar bagan dalam bentuk `.xlsx`.
- **Judul Bagan** dan **Judul Sumbu** (pada menu **+**) juga memiliki **Teks Judul…**. Ketik kata-kata yang Anda inginkan. Atau ketik tanda sama dengan dan sebuah sel, seperti **=Sheet1!A1**, maka judul menampilkan isi sel tersebut. Ketika sel berubah, judul ikut berubah. Berkas menyimpan tautan ini.
- **Simpan sebagai Gambar…** pada tab **Bagan** (dan pada menu klik kanan) menyimpan bagan yang dipilih sebagai berkas gambar **PNG** atau **JPEG**, dua kali lebih besar daripada di lembar agar tetap tajam.
- **Simpan sebagai Templat…** dan **Terapkan Templat…** (pada tab **Bagan**, di grup **Jenis**) menyimpan tampilan bagan dalam berkas **.crtx** dan memberikannya kepada bagan lain. Tampilan mencakup jenis, warna, pengaturan judul dan keterangan, sumbu, serta font. Bagan yang menerima templat tetap memakai angkanya sendiri dan sel yang dibacanya. Satu **Urungkan** menghapus tampilan tersebut kembali.
- **Jenis Bagan** mengubah jenis untuk seluruh bagan, atau untuk setiap seri: kolom dengan garis di atasnya, atau seri terhadap sumbu vertikal kedua.

Referensi ke sel ditulis seperti **Sheet1!$B$2:$B$9**, dengan nama lembar di antara tanda kutip tunggal jika mengandung spasi.

## Sparkline

Sparkline adalah bagan mini yang muat di dalam satu sel. Sparkline menampilkan tren dari satu baris atau kolom angka tepat di sebelahnya.

1. Pilih angka. Untuk satu sparkline per baris, pilih beberapa baris angka.
2. Pada tab **Sisipkan**, di grup **Sparkline**, pilih **Garis**, **Kolom**, atau **Menang/Kalah**. **Garis** menghubungkan angka dengan garis. **Kolom** menggambar batang untuk setiap angka. **Menang/Kalah** menggambar batang ke atas untuk setiap angka di atas nol dan batang ke bawah untuk setiap angka di bawahnya, semuanya berukuran sama, yang cocok untuk hasil yang hanya baik atau buruk.
3. Pada kotak **Buat Sparkline**, periksa **Rentang data** (sel yang dibaca sparkline) dan ketik **Rentang lokasi** (sel tempat sparkline ditempatkan). Pilih satu sel lokasi untuk setiap baris data, atau untuk setiap kolom data. Jika Anda memilih satu sel, sparkline membaca semua data.

Sparkline yang Anda buat dalam satu langkah membentuk satu kelompok dan berbagi satu tampilan. Klik sel yang berisi sparkline, dan tab **Sparkline** muncul.

- **Edit Data** mengubah data dan lokasi seluruh kelompok, atau data dari sparkline yang Anda klik. Di sini juga ditentukan apa yang dilakukan terhadap sel kosong (biarkan celah, anggap sebagai nol, atau hubungkan titik melintasinya) dan apakah sel di baris dan kolom tersembunyi dibaca.
- **Jenis** beralih antara garis, kolom, dan menang/kalah.
- **Tampilkan** menandai titik: **Titik Tertinggi**, **Titik Terendah**, **Titik Negatif**, **Titik Pertama**, **Titik Terakhir**, dan, untuk garis, **Penanda** pada setiap titik.
- **Gaya** memberi kelompok sekumpulan warna. **Warna Sparkline** mengatur warna garis atau batang serta **Ketebalan** (seberapa tebal garisnya, dalam poin). **Warna Penanda** mengatur warna setiap jenis titik yang ditandai.
- **Sumbu** menampilkan garis di nol, memplot angka dari kanan ke kiri, dan mengatur nilai terendah dan tertinggi pada sumbu vertikal. **Otomatis untuk Setiap Sparkline** menyesuaikan setiap sparkline dengan angkanya sendiri. **Sama untuk Semua Sparkline** memberi seluruh kelompok satu skala, sehingga Anda dapat membandingkannya. **Nilai Kustom** memungkinkan Anda mengetik angka. **Sumbu Tanggal…** mengatur jarak titik berdasarkan tanggal: ketik sel yang berisi tanggal setiap titik, dan titik dengan tanggal lebih baru akan berada lebih jauh ke kanan. Kosongkan untuk mengatur ulang jarak titik secara merata.
- **Kelompokkan** menempatkan sparkline yang dipilih dalam satu kelompok. **Pisahkan Kelompok** memberi setiap sparkline tampilannya sendiri. **Hapus** menghapus sparkline yang dipilih, atau seluruh kelompoknya.

Sparkline mengikuti selnya: jika Anda menyisipkan baris di atas data, sparkline tetap membaca angka yang sama. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Sparkline disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain.

## Menyisipkan gambar, bentuk, atau kotak teks

Pada tab **Sisipkan**, di grup **Ilustrasi**, **Gambar** membuka jendela pemilihan berkas. Pilih berkas gambar (PNG, JPEG, GIF, BMP, TIFF, atau SVG) dan gambar akan muncul di lembar pada sel yang dipilih. Gambar yang lebarnya lebih dari 480 titik diperkecil agar muat, dan tetap mempertahankan bentuknya. Buku kerja menyimpan gambar itu sendiri. Gambar dengan jenis yang tidak dapat disimpan buku kerja apa adanya (misalnya SVG) disimpan sebagai PNG, yaitu gambar biasa yang terbentuk dari titik. Berkas yang bukan gambar, atau yang lebih besar dari 16 MB, ditolak dengan pesan. Berkas WMF atau EMF (jenis gambar yang dibuat program lama dari garis dan bentuk) juga digambar dan disimpan sebagai PNG.

**Ikon** menampilkan jendela berisi gambar garis kecil yang sama seperti yang dimiliki JeffPub dan JeffDeck, dengan kotak untuk mencarinya berdasarkan nama. Pilih satu atau lebih, dan setiap ikon akan muncul di lembar sebagai gambar selebar dan setinggi 48 poin. Ikon digambar dengan warna abu-abu tua dalam ukuran yang tetap tajam saat dibesarkan. Setiap ikon adalah satu langkah yang dapat Anda urungkan.

**Kode Batang** membuka jendela untuk membuat kode batang: ISBN buku (dengan atau tanpa harga), EAN-13, UPC-A, EAN-8, Code 128, Code 39, atau kode QR, yang dapat memuat teks apa saja, seperti alamat web. Pilih **Jenis**, ketik teks atau angka, dan jendela akan menampilkan kode batang saat Anda mengetik. Jika kode batang tidak dapat dibuat, jendela menjelaskan kesalahannya dengan teks merah. Klik **Sisipkan**, dan kode batang akan muncul di lembar sebagai gambar, satu langkah yang dapat Anda urungkan. Pindahkan dan ubah ukurannya seperti gambar biasa. Untuk mengubahnya, pilih kode batang lalu klik **Kode Batang** lagi, ubah teksnya, dan klik **Perbarui**; kode batang baru menggantikan yang lama dengan lebar yang sama. Saat Anda menyimpan, kode batang disimpan sebagai gambar dengan SVG yang tetap tajam sehingga setiap program dapat menampilkannya, dan JeffCrunch menyimpan pengaturannya dalam berkas, sehingga kode batang tetap dapat diedit ketika Anda membuka kembali buku kerja di JeffCrunch.

**Bentuk** membuka daftar: persegi panjang, persegi panjang sudut membulat, oval, segitiga, belah ketupat, panah, bintang, hati, awan, balon ucapan, dan garis. Pilih salah satu dan bentuk tersebut muncul di lembar dengan warna biru muda dan tepi biru yang lebih gelap. **Kotak Teks**, dalam grup **Teks**, menyisipkan kotak kosong tanpa isian dan tanpa tepi.

Dengan bentuk terpilih, **Gaya** pada tab **Format Bentuk** memberinya tampilan siap pakai: isian dan garis tepi yang serasi, seperti biru muda dengan tepi biru tua. Dua gaya terakhir hanya menyimpan garis tepinya atau hanya isiannya. Satu klik adalah satu langkah yang dapat Anda urungkan. **Isian Bentuk** dan **Garis Tepi Bentuk** mengubah masing-masing bagian secara terpisah.

## Mencari gambar gratis secara daring

**Gambar Daring**, dalam grup **Ilustrasi** pada tab **Sisipkan**, mencari gambar gratis. Gambar berasal dari dua pustaka terbuka: Openverse, yang mencari di banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Klik **Gambar Daring…**. Panel terbuka di sebelah kanan.
2. Pilih pustaka, ketik apa yang Anda cari, lalu klik **Cari** (atau tekan Enter).
3. Klik gambar untuk melihat siapa pembuatnya dan apa lisensinya. **Lisensi** menyatakan bagaimana Anda boleh menggunakan gambar. Klik nama lisensi untuk membacanya.
4. Klik **Sisipkan** (atau klik ganda gambar).

**Apa yang dikirim JeffCrunch.** Kata pencarian Anda dikirim ke pustaka yang Anda pilih, dan hanya ketika Anda mengklik **Cari** atau menekan Enter. Kemudian JeffCrunch meminta pustaka tersebut mengirim pratinjau kecil dari hasil pencarian, dan satu gambar yang Anda sisipkan. Seperti halaman web mana pun, pustaka dapat melihat alamat internet komputer Anda, dan diberi tahu bahwa JeffCrunch sedang memintanya. Tidak ada yang dari buku kerja Anda yang dikirim.

Gambar muncul di lembar seperti gambar dari berkas, dan buku kerja menyimpannya. Judulnya disimpan sebagai **teks alternatif**. Sebagian besar lisensi meminta Anda mencantumkan nama pembuatnya. Jika **Tambahkan kredit di bawah gambar** dicentang, JeffCrunch menambahkan kotak teks kecil di bawah gambar yang berisi judul, pembuat, dan lisensinya. Anda dapat memindahkan, mengubah, atau menghapusnya. Gambar dan kreditnya dimasukkan sebagai satu langkah, sehingga **Urungkan** membatalkan keduanya. Periksa bahwa lisensinya sesuai dengan cara Anda akan menggunakan gambar tersebut.

## Menambahkan Seni Teks

**Seni Teks** adalah kata-kata berhias: huruf berwarna, dengan garis tepi, bayangan, atau bentuk lengkung.

1. Klik **Sisipkan > Teks > Seni Teks**. Galeri gaya terbuka.
2. Klik sebuah gaya. Kata "Teks Anda di Sini" muncul di lembar dengan gaya tersebut.
3. Untuk mengubah kata-katanya, pilih Seni Teks lalu klik **Edit Teks…** pada tab **Format Bentuk**. Sebuah kotak meminta kata-kata, font, ukuran, tebal, dan miring. Ukuran yang lebih besar membuat Seni Teks lebih besar.
4. Pada tab **Format Bentuk**, **Gaya Seni Teks** memberi huruf gaya lain, dan **Lengkung Teks** membengkokkan kata menjadi bentuk, seperti gelombang atau lengkungan. "Biasa" membuat kata tetap lurus.

Seni Teks adalah bentuk dalam gambar lembar, dengan lengkung, isian, dan garis tepinya, sehingga tampilannya tetap sama ketika Anda menyimpan buku kerja sebagai berkas .xlsx dan membukanya kembali, baik di JeffCrunch maupun di program lain. Setiap perubahan adalah satu langkah yang dapat Anda urungkan. Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan gambar, bentuk, kotak teks, kelompok, garis, dan Seni Teks. Seni Teks disimpan sebagai bentuk fontwork yang dimiliki OpenDocument, dengan kata-kata, font, isian, garis tepi, dan lengkungnya, dan kembali sebagai Seni Teks ketika Anda membuka berkas .ods lagi. Beberapa lengkung yang tidak memiliki bentuk di OpenDocument (Gelombang 2, Gelombang 4, serta bentuk bawah dan atas dari Membesar dan Mengecil) disimpan sebagai kata lurus dengan huruf dan gaya yang sama. Bagan juga disimpan dalam berkas .ods: lihat di bawah.

## Memperkecil gambar

Gambar dari kamera bisa jauh lebih besar daripada yang tampak di lembar, sehingga membuat berkas menjadi besar. Klik gambar, lalu klik **Kompres Gambar…** pada grup **Sesuaikan** di tab **Format Gambar**. Pilih tempat buku kerja akan digunakan: **Cetak komersial (300 ppi)**, **Cetak desktop (220 ppi)**, atau **Web (96 ppi)**. Biarkan **Terapkan ke semua gambar di lembar ini** tercentang untuk memperkecil setiap gambar pada lembar, atau hapus centangnya untuk hanya memperkecil gambar yang dipilih. **Hapus area gambar yang dipotong** juga menghilangkan bagian tersembunyi yang telah Anda potong, sehingga potongan tersebut tidak ada lagi. Gambar hanya diganti jika versi barunya lebih kecil. Ukuran, tempat, nama, dan teks alternatifnya tetap seperti sebelumnya. Urungkan (Ctrl+Z) mengembalikan gambar, dan buku kerja yang disimpan hanya memuat gambar yang lebih kecil. Gambar pada lembar lain tidak diubah: buka setiap lembar dan lakukan lagi. Gambar yang berupa gambar vektor (SVG, EMF, atau WMF), atau animasi, dibiarkan apa adanya.

Setiap langkah adalah satu tindakan yang dapat Anda urungkan. Objek baru akan terpilih, sehingga Anda dapat menyeretnya, mengubah ukurannya, memindahkannya ke depan atau ke belakang, memberinya nama dan teks alternatif di **Properti**, atau menekan **Delete**.

## Mengubah tampilan gambar, bentuk, atau kotak teks

Pilih gambar dan tab **Format Gambar** muncul. Pilih bentuk, garis, atau kotak teks dan tab **Format Bentuk** muncul. Setiap perubahan adalah satu langkah yang dapat Anda urungkan, dan objek tetap memiliki nama, teks alternatif, tempat, dan urutannya di antara objek lain.

- **Isian Bentuk** dan **Garis Tepi Bentuk** memilih warna, atau tanpa warna. **Gaya Garis** mengatur ketebalan (seberapa tebal garisnya) dan garis putus-putus (padat, putus-putus, atau titik-titik). Gambar memiliki **Bingkai Gambar**, yaitu garis tepinya.
- **Putar** memutar objek seperempat putaran ke kanan atau ke kiri, atau membaliknya dari sisi ke sisi atau dari atas ke bawah.
- **Potong**, untuk gambar, memotongnya menjadi bentuk jendela: persegi, lebar 4 kali tinggi 3, tinggi 4 kali lebar 3, 16 kali 9, atau 9 kali 16. Pemotongan dilakukan merata dari tengah bagian yang terlihat, dan gambar diperbesar sebesar sisa yang tersisa. **Tampilkan Seluruh Gambar** mengembalikan bagian yang dipotong. File gambar tidak diubah: bagian yang dipotong hanya disembunyikan.
- **Edit Teks** untuk bentuk dan kotak teks. Ketik kata-katanya, lalu pilih cara kata-kata tersebut diletakkan secara mendatar dan vertikal, ukuran, tebal atau biasa, serta apakah kata-kata dibungkus mengikuti bentuk atau tetap dalam satu baris. Untuk Seni Teks, yang diminta adalah kata-kata, font, ukuran, tebal, dan miring.
- **Panel Pilihan** membuka daftar setiap objek pada lembar berdasarkan nama, dengan objek paling depan di urutan pertama. Klik sebuah nama untuk memilih objek tersebut, yang berguna ketika objek saling menutupi. **Maju Satu Tingkat** dan **Mundur Satu Tingkat** memindahkannya dalam daftar.
- **Properti** mengatur ukuran, nama, teks alternatif, dan apa yang dilakukan objek ketika sel dipindahkan atau dibuat lebih besar.

## Memotong, menyalin, dan menempel objek

Gambar atau bentuk disalin dan ditempel dengan cara yang sama seperti bagan. Bentuk yang menggunakan sesuatu selain gambar dari buku kerja, seperti tautan ke berkas, tidak dapat disalin, dan JeffCrunch memberi tahu Anda alasannya.

Salin bagan dan tempel ke buku kerja yang sama, pada lembar mana pun: salinan itu membaca sel yang sama dengan bagan pertama. Tempelkan ke buku kerja lain dan bagan tetap menampilkan angka yang ditampilkannya, tetapi tidak lagi membaca sel, karena sel yang akan dibacanya di sana bukan sel tempat bagan itu dibuat.

## Memilih beberapa objek, mengelompokkan, dan merapikan

Klik satu objek, lalu tahan Shift atau Ctrl dan klik objek lain untuk memilihnya juga. Klik objek yang sudah dipilih lagi dengan Shift atau Ctrl untuk membatalkan pilihannya. Objek yang dipilih masing-masing mendapat bingkai putus-putus. Anda juga dapat memilih **Pilih Objek** pada tab **Format Bentuk** lalu menyeret kotak di atas sel kosong: setiap objek yang sepenuhnya berada di dalam kotak akan terpilih. Tekan Esc, atau pilih **Pilih Objek** lagi, untuk kembali memilih sel. Tekan Delete untuk menghapus semua objek yang dipilih dalam satu langkah.

Jika dua objek atau lebih dipilih, tab **Format Bentuk** memiliki perintah berikut pada grup **Atur**:

- **Kelompokkan** menggabungkan bentuk, gambar, dan garis yang dipilih menjadi satu objek. Objek itu dipindahkan dan diubah ukurannya sebagai satu kesatuan, dan berkas menyimpannya sebagai kelompok. Bagan tidak dapat dimasukkan ke dalam kelompok.
- **Pisahkan Kelompok** memisahkan kelompok yang dipilih. Setiap bagiannya kembali ke posisinya di lembar.
- **Rata** merapikan objek yang dipilih: **Rata Kiri**, **Rata Tengah**, **Rata Kanan**, **Rata Atas**, **Rata Tengah Vertikal**, dan **Rata Bawah**. Objek diratakan terhadap tepi atau tengah seluruh kumpulan. **Distribusikan Horizontal** dan **Distribusikan Vertikal** mengatur tiga objek atau lebih agar berjarak sama di antara dua objek di ujungnya.

Kelompokkan, Pisahkan Kelompok, dan setiap pilihan Rata adalah satu langkah yang dapat Anda urungkan.

## Memberi nama

Dengan objek terpilih, kotak nama menampilkan nama objek tersebut. Ketik nama baru di sana dan tekan **Enter**. Urungkan mengembalikan nama lama, dan nama baru disimpan dalam berkas.

## Menghapus

Tekan **Delete** dengan objek terpilih. Urungkan mengembalikannya.

## Yang disimpan

Semua yang dijelaskan di atas disimpan dalam berkas dengan format yang dapat dibaca program lembar kerja lain, sehingga bagan yang Anda buat di sini akan terbuka dengan cara yang sama di sana. Bagan yang berasal dari program lain dengan sesuatu yang tidak dapat dipertahankan JeffCrunch ketika menulis ulang bagan (bilah kesalahan, atau bagan tiga dimensi) tetap dapat dipindahkan, diubah ukurannya, dan dihapus, tetapi elemen dan datanya tidak diubah di sini.

## Diagram dari program lain

**Diagram** adalah gambar kotak dan panah, seperti sebuah proses, yang dibuat program lain. JeffCrunch menampilkan diagram dari gambar yang disimpan berkas di sebelahnya. Klik untuk memilihnya, dan seret untuk memindahkannya. Ketika Anda menyimpan berkas, diagram ditulis kembali apa adanya, sehingga program yang membuatnya tetap dapat mengubahnya.

JeffCrunch belum dapat mengubah isi diagram, dan diagram tidak dapat disalin atau dimasukkan ke dalam kelompok. Jika berkas tidak menyimpan gambar diagram, JeffCrunch menampilkan kotak abu-abu polos yang menyatakan bahwa itu adalah diagram.

## Bagan dalam berkas .ods

Buku kerja yang disimpan sebagai berkas .ods tetap menyimpan bagannya: kolom, batang, garis, area, pai, donat, dan bagan sebar, masing-masing dengan seri dan sel yang dibacanya (pada lembar ini atau lembar lain), kategori, judul, keterangan, sumbu (judul, nilai terkecil dan terbesar, format angka), label data, warna, dan tempatnya. Berkas .ods yang dibuat program lain terbuka dengan bagannya, digambar dari sel yang dibacanya. Yang tidak memiliki tempat dalam bagan .ods akan dihilangkan dan disebutkan namanya saat Anda menyimpan: garis tren, bilah kesalahan, sumbu kedua, serta tampilan khusus untuk satu titik. Bagan dengan jenis yang tidak digambar JeffCrunch (bagan saham dan radar) dibuka sebagai kolom dari nilainya.
