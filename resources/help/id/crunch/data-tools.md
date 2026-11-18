# Alat Data
<!-- keywords: prakiraan lembar, prakiraan, nilai masa depan, interval kepercayaan, musiman, tren, pemulusan, formulir, formulir data, entri data, rekaman, rekaman baru, temukan berikutnya, temukan sebelumnya, kriteria, pengelola skenario, skenario, analisis what-if, apa jika, ringkasan skenario, data, tab data, teks ke kolom, pisahkan, pemisah, koma, tab, lebar tetap, hapus duplikat, goal seek, pencarian target, subtotal, total, filter lanjutan, kondisi, validasi data, lingkari data tidak valid, lingkaran merah, hapus lingkaran validasi, konsolidasi, gabungkan rentang, izinkan, daftar, daftar turun, pesan masukan, peringatan galat, baris berulang, unik, berkas teks, csv, dari teks, kolom, pembatas, tanda kutip, edit tautan, perbarui nilai, ubah sumber, tautan, eksternal, sumber, putus tautan, periksa status, buka sumber -->

Tab **Data** berisi alat yang bekerja pada daftar nilai: mengambil teks dari berkas, mengurutkan daftar, memisahkan teks menjadi kolom, dan menghapus baris yang berulang.

## Dari Teks/CSV

**Dari Teks/CSV** membuka berkas teks (.csv, .tsv, atau .txt). Perintah ini sama dengan **Buka Berkas Teks** pada tab Berkas. JeffCrunch menanyakan bagaimana berkas dibagi menjadi kolom dan jenis teks apa yang dikandungnya, lalu menampilkannya sebagai buku kerja baru.

## Edit Tautan

Jika buku kerja memiliki rumus yang membaca buku kerja lain (misalnya ='[Harga.xlsx]Sheet1'!A1), **Edit Tautan** pada grup **Dapatkan Data** menampilkan daftar buku kerja lain tersebut, yang disebut sumber. Setiap baris menunjukkan letak sumber dan statusnya. Sampai buku kerja dibuka kembali bersama sumbernya, buku kerja menampilkan angka yang disimpan berkasnya dari sumber tersebut.

- **Periksa Status** mencari setiap sumber di komputer ini, lalu menyatakan **Ditemukan** atau menyatakan bahwa sumber tidak ditemukan. Sumber di web tidak diambil, sehingga statusnya menyatakan belum diperiksa.
- **Perbarui Nilai** membaca sumber yang dipilih dari komputer ini dan memberi setiap sel yang hanya merujuk ke salah satu selnya nilai yang dimiliki sel tersebut sekarang. Sel yang melakukan lebih banyak hal dengan nilai itu, seperti `=[1]Harga!A1*2`, tetap menyimpan angka yang disimpan berkasnya, dan pesan di bawah daftar menyatakan berapa sel yang diperbarui dan berapa yang tidak. Perbarui Nilai adalah satu langkah, sehingga **Urungkan** mengembalikan nilai lama. Nilai baru juga ditulis ke bagian tautan itu di dalam berkas saat Anda menyimpan, sehingga buku kerja menampilkannya lagi saat dibuka berikutnya, bahkan ketika sumbernya tidak ada.
- **Ubah Sumber** memungkinkan Anda memilih berkas lain untuk tautan yang dipilih, misalnya ketika buku kerja dipindahkan. Rumusnya tetap sama. Lokasi baru ditulis ke berkas saat Anda menyimpan, dan **Urungkan** membatalkan perubahan tersebut.
- **Buka Sumber** membuka sumber yang dipilih di jendela baru, jika sumber itu dapat ditemukan.
- **Putus Tautan** mengubah setiap sel yang membaca sumber yang dipilih menjadi nilai yang tampil sekarang, dan nama yang mewakili suatu lokasi di dalamnya menjadi **#REF!**. JeffCrunch menanyakan terlebih dahulu. Putus Tautan adalah satu langkah, sehingga **Urungkan** mengembalikan rumus, nama, dan tautan, sampai Anda menyimpan buku kerja. Jika tidak ada tautan yang tersisa, berkas disimpan tanpa bagian yang menyimpannya.

## Formulir

**Formulir** (pada grup **Alat Data**, dan di kotak pencarian) menampilkan satu baris daftar setiap kali, sebagai formulir kecil. Daftar adalah tabel, atau blok sel dengan baris nama di atas dan baris data di bawah setiap nama. Klik sel mana pun dari daftar terlebih dahulu.

- Setiap kotak adalah satu kolom, diberi label sesuai nama di baris atas. Ketik di dalam kotak untuk mengubah nilai. Kotak yang berwarna abu-abu berisi rumus: formulir menampilkan hasilnya, dan Anda mengubah rumusnya di lembar.
- **Temukan Berikutnya** dan **Temukan Sebelumnya** berpindah ke baris berikutnya dan baris sebelumnya. Berpindah ke baris lain, atau menekan **Enter**, menulis apa yang Anda ketik pada baris ini.
- **Baru** menampilkan formulir kosong. Isi, lalu tekan **Baru** lagi, **Temukan Berikutnya**, atau **Tutup**. Baris ditambahkan di bawah baris terakhir daftar. Tabel bertambah satu baris. Jika sel di bawah daftar tidak kosong, formulir memberi tahu Anda dan tidak menambahkan apa pun.
- **Hapus** menghapus baris yang sedang Anda lihat, setelah meminta konfirmasi. Baris di bawahnya naik. Tabel selalu menyisakan setidaknya satu baris data. Baris yang disembunyikan, atau yang disembunyikan filter, tidak ditampilkan di formulir, tidak dihitung, dan tidak pernah dihapus olehnya. Jika daftar terlalu panjang untuk ditelusuri seluruhnya, formulir memberi tahu bahwa hanya baris pertama yang ditampilkan.
- **Pulihkan** membatalkan apa yang Anda ketik pada baris ini, sebelum Anda meninggalkannya.
- **Kriteria** memungkinkan Anda mencari baris. Ketik kondisi di satu atau beberapa kotak, lalu tekan **Temukan Berikutnya** atau **Temukan Sebelumnya**. Kondisi adalah sebuah nilai, seperti **Ani** (teks cocok dengan awal kata, dan **?** serta **\*** mewakili huruf), atau nilai setelah tanda, seperti **>100**, **<=5**, **<>0**, atau **=Oslo**. Sebuah baris harus memenuhi setiap kondisi yang Anda ketik. Tekan **Formulir** untuk kembali, atau **Hapus** untuk mulai lagi. Selama kriteria diatur, **Temukan Berikutnya** dan **Temukan Sebelumnya** melewati baris yang tidak memenuhinya.
- **Tutup** menulis baris dan menutup formulir.

Setiap baris yang Anda ubah, tambahkan, atau hapus adalah satu langkah, sehingga **Urungkan** di lembar membatalkannya sekaligus.

## Urutkan dan filter

Grup **Urutkan & Filter** memiliki perintah yang sama dengan menu **Urutkan dan Pilih Baris** pada tab Beranda. Lihat [Urutkan dan filter](sort-filter).

## Teks ke Kolom

Gunakan ini ketika satu kolom berisi beberapa potongan teks yang ingin Anda tempatkan di kolom terpisah, seperti "Smith, Ann, 31".

1. Pilih sel dari **satu** kolom.
2. Pilih **Teks ke Kolom** pada tab **Data**.
3. Pilih cara memisahkan teks:
   - **Pisahkan pada pemisah**: centang tanda yang berada di antara potongan: **Tab**, **Titik koma**, **Koma**, **Spasi**, atau **Lainnya**, tempat Anda mengetik tanda sendiri. **Perlakukan pemisah berurutan sebagai satu** mencegah dua koma berurutan membuat potongan kosong. **Teks dalam tanda kutip** menentukan tanda yang membungkus potongan yang tidak boleh dipisah, seperti "Lee, Bo". Di dalam tanda kutip, dua tanda berurutan berarti satu.
   - **Pisahkan pada posisi tetap**: ketik posisi tempat kolom baru dimulai, dengan menghitung karakter dari 1. Misalnya, 5, 12 memulai kolom baru pada karakter kelima dan kedua belas.
4. Pilih jenis setiap potongan: **Umum** membaca angka dan tanggal seperti yang Anda ketik, dan **Teks** menyimpan setiap potongan apa adanya, sehingga "007" tetap "007".
5. Pada **Tempatkan potongan mulai dari**, ketik sel pertama untuk potongan. Awalnya sel pertama yang Anda pilih.
6. Kotak di bawah pilihan menampilkan beberapa baris pertama yang sudah dipisah sesuai pilihan Anda. Tekan **OK**.

Potongan ditempatkan di sel sebelah kanan sel pertama, dan menggantikan isi sel tersebut. Teks ke Kolom adalah satu langkah yang dapat Anda urungkan.

## Hapus Duplikat

Gunakan ini untuk menyimpan hanya baris pertama dari setiap kelompok baris yang berulang.

1. Pilih sel-sel daftar, atau satu sel di dalamnya. JeffCrunch mengambil seluruh blok data di sekitar satu sel.
2. Pilih **Hapus Duplikat**.
3. Centang **Data saya memiliki baris header** jika baris pertama berisi nama. Baris itu tetap ada dan tidak dibandingkan.
4. Centang kolom yang harus sama agar dua baris dianggap berulang. Dua baris berulang hanya jika setiap kolom yang dicentang sama. Huruf besar dan huruf kecil dianggap sama, dan rumus dihitung sebagai nilai yang dihasilkannya.
5. Tekan **OK**. Kotak dialog memberi tahu berapa baris yang dihapus dan berapa yang tersisa.

Baris di bawah baris yang dihapus naik di dalam sel yang dipilih, beserta format dan rumusnya. Sel di bawah pilihan tidak bergerak. Baris kosong yang tertinggal di bagian bawah pilihan dikosongkan. Hapus Duplikat adalah satu langkah yang dapat Anda urungkan.

## Goal Seek

Gunakan ini ketika Anda mengetahui jawaban yang diinginkan tetapi tidak mengetahui angka yang menghasilkannya. Misalnya, sel pembayaran berisi rumus yang memakai suku bunga, dan Anda ingin pembayarannya 500. Goal Seek mencoba angka di sel suku bunga hingga pembayarannya 500.

1. Pilih **Goal Seek** pada tab **Data**.
2. Pada **Sel dengan rumus**, ketik sel yang berisi rumus, misalnya B3. Awalnya sel yang Anda pilih.
3. Pada **Nilai yang Anda inginkan**, ketik jawaban yang ingin dihasilkan rumus.
4. Pada **Sel yang diubah**, ketik sel yang boleh diubah Goal Seek, misalnya B1. Sel itu harus berisi angka, bukan rumus.
5. Tekan **OK**.

Jika Goal Seek menemukan angka, angka itu tetap berada di sel yang diubah, dan bilah status menyatakan angkanya. Jika tidak dapat, tidak ada yang berubah dan Anda diberi tahu. Coba angka awal yang berbeda di sel yang diubah, dan periksa bahwa rumus benar-benar memakai sel tersebut. Goal Seek adalah satu langkah yang dapat Anda urungkan.

## Pengelola Skenario

Skenario adalah kumpulan nilai bernama untuk beberapa sel. Gunakan skenario untuk mencoba "apa jika" pada satu lembar. Misalnya, simpan skenario "Penjualan rendah" dan skenario "Penjualan tinggi" untuk sel masukan yang sama, lalu tampilkan yang ingin Anda lihat.

1. Pilih **Pengelola Skenario** pada tab **Data**.
2. Tekan **Tambah**. Ketik nama, misalnya Penjualan tinggi. Pada **Sel yang diubah**, ketik sel yang diatur skenario, misalnya B1:B3 atau D5. Skenario dapat mengubah hingga 32 sel. Sel yang Anda pilih sudah diisi untuk Anda. Setiap sel dimulai dengan nilai yang dimilikinya sekarang. Ketik nilai yang harus digunakan skenario ini di tabel, lalu tekan **OK**.
3. Pilih skenario dalam daftar dan tekan **Tampilkan** untuk memasukkan nilainya ke sel. Mengklik dua kali nama juga melakukan hal yang sama. Tekan **Edit** untuk mengubah nama, komentar, atau nilainya. Tekan **Hapus** untuk menghapusnya.

Tampilkan, Tambah, Edit, dan Hapus masing-masing adalah satu langkah yang dapat Anda urungkan.

**Ringkasan** menempatkan semua skenario berdampingan pada lembar baru bernama Ringkasan Skenario. Ringkasan meminta sel hasil, yaitu sel yang ingin Anda bandingkan, misalnya total di B10. Untuk setiap skenario, ringkasan menampilkan nilai sel yang berubah dan hasil sel hasil, yang dihitung dengan nilai skenario tersebut. Kolom bernama Nilai Saat Ini menampilkan kondisi sel ketika ringkasan dibuat. Sel Anda sendiri dibiarkan seperti semula.

Skenario disimpan di dalam buku kerja. Buku kerja yang dibuat program lain tetap menyimpan skenarionya, dan Anda dapat melihatnya di sini. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tidak menyimpannya, dan penyimpanan akan memberi tahu Anda.

## Subtotal

Gunakan ini untuk menempatkan baris total di bawah setiap kelompok baris yang memiliki entri sama pada satu kolom, dan total keseluruhan di bagian bawah. Daftar memerlukan baris header, dan sebaiknya diurutkan menurut kolom yang dikelompokkan, agar entri yang sama berdampingan.

1. Pilih sel di dalam daftar.
2. Pilih **Subtotal** pada grup **Garis Besar** tab **Data**.
3. Pada **Pada setiap perubahan di**, pilih kolom yang mengelompokkan baris, misalnya Wilayah.
4. Pada **Gunakan fungsi**, pilih yang akan dihitung: **Jumlah**, **Hitung**, **Rata-rata**, **Maks**, **Min**, dan lainnya.
5. Pada **Tambahkan subtotal ke**, centang setiap kolom yang akan dijumlahkan.
6. Biarkan **Ganti subtotal saat ini** tercentang untuk mengganti subtotal yang sudah ada. Tekan **OK**.

Setiap baris total berbunyi "Total Timur" (atau entri yang diwakilinya) dan berisi rumus SUBTOTAL, sehingga melewati baris total lain. Baris setiap kelompok dikelompokkan, sehingga Anda dapat memakai tombol 1, 2, dan 3 di samping baris untuk hanya menampilkan total. Untuk menghapus total, buka **Subtotal** lagi dan tekan **Hapus Semua**. Menambahkan atau menghapus subtotal adalah satu langkah yang dapat Anda urungkan.

## Filter Lanjutan

Tombol Filter memilih baris berdasarkan satu kolom setiap kali. **Lanjutan** (pada grup **Urutkan & Filter**) memungkinkan Anda menulis kondisi di lembar, sehingga dapat sepanjang yang Anda perlukan.

1. Di sebelah daftar, salin judul kolom yang ingin diuji ke satu baris sel kosong.
2. Di bawah setiap judul, tulis kondisi. Kondisi pada **baris yang sama** harus semuanya benar (ini berarti "dan"). Kondisi pada **baris yang berbeda** adalah alternatif: sebuah baris daftar cukup memenuhi salah satu baris tersebut (ini berarti "atau").
3. Pilih **Lanjutan**. Pastikan **Rentang daftar** berisi daftar beserta baris header-nya, dan ketik sel judul serta kondisi Anda pada **Rentang kondisi**.
4. Pilih **Sembunyikan baris lain, dalam daftar**, atau **Salin baris ke tempat lain** lalu ketik sel pertama tempat itu, misalnya H1. Centang **Abaikan baris yang menduplikasi baris sebelumnya** untuk menyalin hanya satu dari setiap baris.
5. Tekan **OK**.

Cara menulis kondisi:

- Sebuah kata, seperti **An**, memilih teks yang diawali dengan kata tersebut. Huruf besar dan huruf kecil dianggap sama. **?** mewakili satu karakter apa pun dan **\*** mewakili rangkaian karakter apa pun.
- Sebuah angka memilih sel yang sama dengannya.
- **=Ann** memilih teks yang persis Ann. **=** saja memilih sel kosong.
- **>**, **>=**, **<**, **<=**, dan **<>** (tidak sama dengan) di depan angka atau kata membandingkan dengannya, misalnya **>25**. **<>** saja memilih sel yang tidak kosong.

Baris kondisi yang kosong memilih setiap baris. Untuk menampilkan kembali baris yang disembunyikan, pilih **Hapus**. Salinan dibuat seketika dan tidak berubah ketika daftar berubah. Filter Lanjutan adalah satu langkah yang dapat Anda urungkan.

## Validasi Data

Gunakan ini untuk menentukan apa yang boleh diketik pada beberapa sel, sehingga entri yang salah dihentikan atau dipertanyakan.

1. Pilih sel-selnya.
2. Pilih **Validasi Data** pada grup **Alat Data** tab **Data**.
3. Pada tab **Pengaturan**, pilih yang boleh **Diizinkan**: **Nilai apa pun** (tanpa aturan), **Bilangan bulat**, **Desimal**, **Daftar**, **Tanggal**, **Waktu**, **Panjang teks**, atau **Kustom**.
4. Untuk angka, tanggal, waktu, atau panjang teks, pilih **Data** (**di antara**, **tidak di antara**, **sama dengan**, dan seterusnya) lalu ketik batasnya. Batas dapat berupa nilai atau rumus yang diawali =.
5. Untuk **Daftar**, ketik pilihan dengan koma di antaranya, misalnya Merah,Hijau,Biru, atau ketik rentang sel setelah =, misalnya =$A$1:$A$5. Sel kemudian menampilkan panah dengan pilihannya. Matikan **Daftar turun dalam sel** untuk menyembunyikan panah.
6. Untuk **Kustom**, ketik rumus yang diawali =. Entri diizinkan ketika rumus bernilai benar.
7. **Abaikan kosong** membiarkan sel kosong lolos.

Pada tab **Pesan Masukan**, centang kotaknya lalu tulis judul dan pesan. Keduanya tampil di sebelah sel setiap kali sel itu dipilih.

Pada tab **Peringatan Galat**, pilih apa yang terjadi pada entri yang tidak diizinkan. **Berhenti** menolaknya. **Peringatan** menanyakan apakah entri tetap disimpan. **Informasi** memberi tahu pengguna dan tetap menyimpannya. Tulis judul dan pesan untuk peringatan tersebut.

**Lingkari Data Tidak Valid** (di sebelah **Validasi Data**) memberi lingkaran oval merah pada setiap sel di lembar yang nilainya tidak diizinkan aturannya, misalnya angka yang sudah ada sebelum aturan dibuat, atau hasil rumus. Sebuah pesan memberi tahu Anda jika semua sel sudah benar. Sel yang Anda perbaiki kehilangan ovalnya. **Hapus Lingkaran Validasi** menghilangkan semua oval.

**Hapus Semua** menghapus aturan dari sel-sel yang dipilih. Aturan baru menggantikan aturan apa pun yang dimiliki sel-sel yang dipilih; sel lain tetap menyimpan aturannya. Mengubah aturan adalah satu langkah yang dapat Anda urungkan, dan aturan disimpan dalam berkas.

## Konsolidasi

Gunakan ini untuk menggabungkan rentang yang serupa, seperti tabel yang sama dari beberapa lembar, menjadi satu tabel.

1. Pilih **Konsolidasi** pada grup **Alat Data** tab **Data**.
2. Pilih **Fungsi** untuk digabungkan, seperti **Jumlah** atau **Rata-rata**.
3. Ketik rentang pada kotak dan tekan **Tambah**. Gunakan **Sheet2!A1:C10** untuk rentang pada lembar lain. Tambahkan setiap rentang yang ingin digabungkan. Pilih rentang dalam daftar dan tekan **Hapus** untuk mengeluarkannya.
4. Jika rentang memiliki nama pada baris pertamanya, centang **Baris atas berisi nama kolom**. Jika memiliki nama pada kolom pertamanya, centang **Kolom kiri berisi nama baris**.
5. Pada **Letakkan hasil di**, ketik sel pertama untuk hasilnya.
6. Centang **Buat tautan ke data sumber** jika setiap hasil harus berupa rumus yang membaca sel sumbernya, sehingga berubah ketika sel sumber berubah. Tanpa ini, hasilnya berupa angka biasa.
7. Tekan **OK**.

Jika tidak ada nama yang dicentang, sel di tempat yang sama pada setiap rentang digabungkan (sel pertama dari setiap rentang, lalu sel kedua, dan seterusnya). Jika nama dicentang, sel dengan nama yang sama digabungkan, di mana pun letaknya di setiap rentang, dan nama yang hanya dimiliki sebagian rentang tetap mendapatkan baris atau kolom. Konsolidasi adalah satu langkah yang dapat Anda urungkan.

## Lembar Prakiraan

Lembar Prakiraan membuat lembar baru dengan tabel dan bagan nilai masa depan, yang dihitung dari garis waktu dan serangkaian nilai. Lembar ini menemukan tren dan musim berulang dari nilai, lalu menambahkan pita tempat nilai masa depan kemungkinan besar berada.

1. Pilih garis waktu dan nilainya, dua kolom berdampingan (garis waktu di sebelah kiri). Garis waktu berupa tanggal atau angka yang berjarak sama, misalnya setiap bulan atau setiap 5 satuan. Baris nama pertama tidak ikut.
2. Pilih **Lembar Prakiraan** pada grup **Prakiraan** tab **Data**.
3. Sebuah gambar menampilkan prakiraan, dan berubah ketika Anda mengubah pilihan. Pilih **Bagan garis** atau **Bagan kolom**.
4. Atur pilihannya:
   - **Rentang garis waktu** dan **Rentang nilai** adalah sel yang dibaca. Tulis sel lain di sini jika pilihan awal salah.
   - **Awal prakiraan** dibiarkan kosong untuk mulai setelah titik terakhir. Tanggal atau angka sebelum titik terakhir memulai prakiraan di sana dan hanya memakai titik sebelumnya, sehingga Anda dapat membandingkan prakiraan dengan yang sebenarnya terjadi.
   - **Akhir prakiraan** adalah waktu terakhir yang diprakirakan. Paling banyak 10.000 titik diprakirakan.
   - **Interval kepercayaan** menunjukkan seberapa yakin pita tersebut, mulai dari 95%.
   - **Musiman** ditemukan dengan memeriksa nilai, atau Anda dapat mengatur panjang musim dalam titik, atau memilih tidak ada. Nilai harus memuat dua musim penuh.
   - **Isi titik yang hilang dengan** menentukan nilai apa yang didapat waktu yang tidak memiliki nilai: garis lurus di antara tetangganya, atau nol.
   - **Gabungkan duplikat dengan** menentukan cara nilai pada waktu yang sama digabungkan: Rata-rata, Hitung, CountA, Maks, Median, Min, atau Jumlah.
   - **Sertakan statistik prakiraan** menambahkan tabel Alpha, Beta, Gamma, MASE, SMAPE, MAE, dan RMSE, yang menunjukkan seberapa baik prakiraan cocok dengan nilai.
5. Jika ada yang salah, kotak dialog menjelaskannya dengan kata-kata sederhana (langkah yang tidak sama, titik terlalu sedikit, teks pada nilai), dan **Buat** menunggu.
6. Tekan **Buat**.

Lembar baru ditempatkan sebelum lembar yang sedang Anda buka. Tabelnya memiliki kolom Garis Waktu, Nilai, Prakiraan, Batas Kepercayaan Bawah, dan Batas Kepercayaan Atas. Kolom Prakiraan dan batasnya berupa rumus yang memakai `FORECAST.ETS` dan `FORECAST.ETS.CONFINT` pada kolom Garis Waktu dan Nilai tabel itu sendiri, sehingga dihitung ulang ketika Anda mengubah nilai. Bagan menampilkan nilai, prakiraan, dan pita. Lembar Prakiraan adalah satu langkah yang dapat Anda urungkan.

Angkanya dapat berbeda beberapa persen dari program lembar kerja lain, yang memakai cara sendiri untuk mencocokkan pemulusan, dan prakiraan yang sudah ada dalam berkas tetap menyimpan nilai yang disimpan berkas tersebut sampai data yang dibacanya diubah.

Berkas dengan lebih dari 1.000.000 titik tidak digunakan.
