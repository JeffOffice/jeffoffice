# Alat data

<!-- keywords: pengelola skenario, skenario, apa jika, bagaimana jika, ringkasan skenario, data, tab data, teks ke kolom, pisah, pemisah, koma, tab, lebar tetap, hapus duplikat, goal seek, pencarian target, subtotal, total, filter lanjutan, kondisi, validasi data, lingkari data tidak valid, lingkaran merah, hapus lingkaran validasi, konsolidasi, gabung rentang, izinkan, daftar, daftar turun, pesan masukan, peringatan galat, baris berulang, unik, berkas teks, csv, dari teks, kolom, pembatas, tanda kutip, edit tautan, perbarui nilai, ubah sumber, tautan, eksternal, sumber, putus tautan, periksa status, buka sumber -->

Tab **Data** berisi alat yang bekerja pada daftar nilai: mengambil teks dari berkas, mengurutkan daftar, memisahkan teks menjadi kolom, dan menghapus baris yang berulang.

## Dari Teks/CSV

**Dari Teks/CSV** membuka berkas teks (.csv, .tsv, atau .txt). Perintah ini sama dengan **Buka Berkas Teks** pada tab Berkas. JeffCrunch menanyakan cara berkas dibagi menjadi kolom dan jenis teks yang dikandungnya, lalu menampilkannya sebagai buku kerja baru.

## Edit Tautan

Jika sebuah buku kerja memiliki rumus yang membaca buku kerja lain (seperti ='[Harga.xlsx]Sheet1'!A1), **Edit Tautan** pada grup **Dapatkan Data** menampilkan daftar buku kerja lain tersebut, yang disebut sumber. Setiap baris menunjukkan lokasi sumber dan statusnya. Sampai buku kerja dibuka kembali bersama sumbernya, buku kerja menampilkan angka yang disimpan berkasnya dari sumber tersebut.

- **Periksa Status** mencari setiap sumber di komputer ini dan menyatakan **Ditemukan**, atau menyatakan bahwa sumber tidak ditemukan. Sumber di web tidak diambil, sehingga statusnya menyatakan belum diperiksa.
- **Perbarui Nilai** membaca sumber yang dipilih dari komputer ini dan memberi setiap sel yang hanya berupa referensi ke salah satu selnya, nilai yang kini dimiliki sel tersebut. Sel yang melakukan lebih dari sekadar mengambil nilai, seperti `=[1]Harga!A1*2`, tetap menyimpan angka yang disimpan berkasnya, dan pesan di bawah daftar menyatakan berapa sel yang diperbarui dan berapa yang tidak. Perbarui Nilai adalah satu langkah, sehingga **Urungkan** mengembalikan nilai lama. Nilai baru juga ditulis ke bagian tautan itu dalam berkas saat Anda menyimpan, sehingga buku kerja menampilkannya saat dibuka lagi, meskipun sumbernya tidak ada.
- **Ubah Sumber** memungkinkan Anda memilih berkas lain untuk tautan yang dipilih, misalnya ketika buku kerja dipindahkan. Rumus tetap seperti semula. Lokasi baru ditulis ke berkas saat Anda menyimpan, dan **Urungkan** membatalkan perubahan tersebut.
- **Buka Sumber** membuka sumber yang dipilih di jendela baru, jika sumber itu dapat ditemukan.
- **Putus Tautan** mengubah setiap sel yang membaca sumber yang dipilih menjadi nilai yang tampil saat ini, dan nama yang mewakili suatu lokasi di dalamnya menjadi **#REF!**. JeffCrunch menanyakan terlebih dahulu. Putus Tautan adalah satu langkah, sehingga **Urungkan** mengembalikan rumus, nama, dan tautan, hingga Anda menyimpan buku kerja. Jika tidak ada tautan yang tersisa, berkas disimpan tanpa bagian yang memuatnya.

## Urutkan dan filter

Grup **Urutkan & Filter** memiliki perintah yang sama dengan menu **Urutkan dan Pilih Baris** pada tab Beranda. Lihat [Urutkan dan filter](sort-filter).

## Teks ke Kolom

Gunakan ini ketika satu kolom berisi beberapa potongan teks yang ingin Anda letakkan di kolom terpisah, seperti "Smith, Ann, 31".

1. Pilih sel dari **satu** kolom.
2. Pilih **Teks ke Kolom** pada tab **Data**.
3. Pilih cara memisahkan teks:
   - **Pisahkan pada pemisah**: centang tanda yang berada di antara potongan teks: **Tab**, **Titik koma**, **Koma**, **Spasi**, atau **Lainnya**, tempat Anda mengetik tanda sendiri. **Perlakukan pemisah berurutan sebagai satu** mencegah dua koma berurutan membuat potongan kosong. **Teks dalam tanda kutip** menentukan tanda yang membungkus potongan yang tidak boleh dipisah, seperti "Lee, Bo". Di dalam tanda kutip, dua tanda berurutan dianggap sebagai satu.
   - **Pisahkan pada posisi tetap**: ketik posisi tempat kolom baru dimulai, dengan menghitung karakter dari 1. Misalnya, 5, 12 memulai kolom baru pada karakter kelima dan kedua belas.
4. Pilih jenis setiap potongan: **Umum** membaca angka dan tanggal seperti yang Anda ketik, dan **Teks** mempertahankan setiap potongan apa adanya, sehingga "007" tetap "007".
5. Pada **Letakkan potongan dari**, ketik sel pertama untuk potongan-potongan tersebut. Awalnya sel pertama yang Anda pilih.
6. Kotak di bawah pilihan menampilkan beberapa baris pertama yang sudah dipisah sesuai pilihan Anda. Tekan **OK**.

Potongan-potongan itu masuk ke sel di sebelah kanan sel pertama, dan menggantikan isi sel tersebut. Teks ke Kolom adalah satu langkah yang dapat diurungkan.

## Hapus Duplikat

Gunakan ini untuk hanya mempertahankan baris pertama dari setiap kelompok baris yang berulang.

1. Pilih sel-sel daftar, atau satu sel di dalamnya. JeffCrunch mengambil seluruh blok data di sekitar satu sel.
2. Pilih **Hapus Duplikat**.
3. Centang **Data saya memiliki baris header** jika baris pertama berisi nama. Baris itu tetap ada dan tidak dibandingkan.
4. Centang kolom yang harus sama agar dua baris dianggap berulang. Dua baris berulang hanya jika setiap kolom yang dicentang sama. Huruf besar dan kecil dianggap sama, dan rumus dianggap sebagai nilai yang dihasilkannya.
5. Tekan **OK**. Kotak dialog memberi tahu berapa baris yang dihapus dan berapa yang tersisa.

Baris di bawah baris yang dihapus naik ke atas di dalam sel yang dipilih, beserta format dan rumusnya. Sel di bawah pilihan tidak bergeser. Baris kosong yang tersisa di bagian bawah pilihan dikosongkan. Hapus Duplikat adalah satu langkah yang dapat diurungkan.

## Goal Seek

Gunakan ini ketika Anda mengetahui jawaban yang diinginkan tetapi tidak mengetahui angka yang menghasilkannya. Misalnya, sel pembayaran berisi rumus yang memakai suku bunga, dan Anda ingin pembayarannya 500. Goal Seek mencoba angka pada sel suku bunga hingga pembayarannya 500.

1. Pilih **Goal Seek** pada tab **Data**.
2. Pada **Sel dengan rumus**, ketik sel yang berisi rumus, misalnya B3. Awalnya sel yang Anda pilih.
3. Pada **Nilai yang Anda inginkan**, ketik jawaban yang ingin dihasilkan rumus.
4. Pada **Sel yang diubah**, ketik sel yang boleh diubah Goal Seek, misalnya B1. Sel ini harus berisi angka, bukan rumus.
5. Tekan **OK**.

Jika Goal Seek menemukan angka, angka itu tetap berada di sel yang diubah, dan bilah status menyatakan angkanya. Jika tidak menemukan, tidak ada yang berubah dan Anda diberi tahu. Cobalah angka awal yang berbeda pada sel yang diubah, dan periksa bahwa rumus benar-benar memakai sel tersebut. Goal Seek adalah satu langkah yang dapat diurungkan.

## Pengelola Skenario

Skenario adalah kumpulan nilai bernama untuk beberapa sel. Gunakan skenario untuk mencoba "apa jika" pada satu lembar. Misalnya, simpan skenario "Penjualan rendah" dan skenario "Penjualan tinggi" untuk sel masukan yang sama, lalu tampilkan yang ingin Anda lihat.

1. Pilih **Pengelola Skenario** pada tab **Data**.
2. Tekan **Tambah**. Ketik nama, misalnya Penjualan tinggi. Pada **Sel yang diubah**, ketik sel yang diatur skenario, misalnya B1:B3 atau D5. Skenario dapat mengubah hingga 32 sel. Sel yang Anda pilih sudah diisikan untuk Anda. Setiap sel awalnya berisi nilai yang dimilikinya sekarang. Ketik nilai yang harus dipakai skenario ini pada tabel, lalu tekan **OK**.
3. Pilih skenario dalam daftar dan tekan **Tampilkan** untuk memasukkan nilainya ke sel. Mengklik dua kali nama juga melakukan hal yang sama. Tekan **Edit** untuk mengubah namanya, komentarnya, atau nilainya. Tekan **Hapus** untuk menghapusnya.

Tampilkan, Tambah, Edit, dan Hapus masing-masing adalah satu langkah yang dapat diurungkan.

**Ringkasan** menempatkan semua skenario berdampingan pada lembar baru bernama Ringkasan Skenario. Perintah ini meminta sel hasil, yaitu sel yang ingin Anda bandingkan, misalnya total pada B10. Untuk setiap skenario, ringkasan menampilkan nilai sel yang diubah dan hasil sel hasil, yang dihitung dengan nilai skenario tersebut. Kolom bernama Nilai Saat Ini menampilkan keadaan sel saat ringkasan dibuat. Sel Anda sendiri dibiarkan seperti semula.

Skenario disimpan dalam buku kerja. Buku kerja yang dibuat program lain tetap menyimpan skenarionya, dan Anda dapat melihatnya di sini. Salinan yang disimpan sebagai berkas .xls versi lama atau sebagai .ods tidak memuat skenario tersebut, dan saat menyimpan, program akan memberi tahu Anda.

## Subtotal

Gunakan ini untuk menempatkan baris total di bawah setiap kelompok baris yang memiliki entri sama pada satu kolom, dan total keseluruhan di bagian bawah. Daftar memerlukan baris header, dan sebaiknya diurutkan menurut kolom yang dikelompokkan, agar entri yang sama berdampingan.

1. Pilih satu sel dalam daftar.
2. Pilih **Subtotal** pada grup **Garis Besar** di tab **Data**.
3. Pada **Pada setiap perubahan di**, pilih kolom yang mengelompokkan baris, misalnya Wilayah.
4. Pada **Gunakan fungsi**, pilih apa yang akan dihitung: **Jumlah**, **Hitung**, **Rata-rata**, **Maks**, **Min**, dan lainnya.
5. Pada **Tambahkan subtotal ke**, centang setiap kolom yang akan dijumlahkan.
6. Biarkan **Ganti subtotal saat ini** tercentang untuk mengganti subtotal yang sudah ada. Tekan **OK**.

Setiap baris total berisi tulisan "Total Timur" (atau entri yang menjadi dasarnya) dan memuat rumus SUBTOTAL, sehingga baris total lain tidak ikut dihitung. Baris setiap kelompok dikelompokkan, sehingga Anda dapat memakai tombol 1, 2, dan 3 di samping baris untuk hanya menampilkan total. Untuk menghapus total, buka **Subtotal** lagi dan tekan **Hapus Semua**. Menambah atau menghapus subtotal adalah satu langkah yang dapat diurungkan.

## Filter Lanjutan

Tombol Filter memilih baris berdasarkan satu kolom pada satu waktu. **Lanjutan** (pada grup **Urutkan & Filter**) memungkinkan Anda menulis kondisi pada lembar, sehingga kondisinya dapat sepanjang yang Anda perlukan.

1. Di sebelah daftar, salin judul kolom yang ingin diuji ke dalam satu baris sel kosong.
2. Di bawah setiap judul, tulis sebuah kondisi. Kondisi pada **baris yang sama** harus semuanya benar (ini adalah "dan"). Kondisi pada **baris yang berbeda** merupakan alternatif: sebuah baris daftar cukup memenuhi salah satu baris tersebut (ini adalah "atau").
3. Pilih **Lanjutan**. Pastikan **Rentang daftar** berisi daftar beserta baris headernya, lalu ketik sel judul dan kondisi Anda pada **Rentang kondisi**.
4. Pilih **Sembunyikan baris lain, dalam daftar**, atau **Salin baris ke tempat lain** dan ketik sel pertama tempat tujuan, misalnya H1. Centang **Abaikan baris yang menduplikasi baris sebelumnya** untuk menyalin hanya satu dari setiap baris.
5. Tekan **OK**.

Cara menulis kondisi:

- Sebuah kata, misalnya **An**, memilih teks yang diawali dengan kata tersebut. Huruf besar dan kecil dianggap sama. **?** mewakili satu karakter apa pun dan ***** mewakili deretan karakter apa pun.
- Sebuah angka memilih sel yang sama dengannya.
- **=Ann** memilih teks yang persis Ann. **=** saja memilih sel kosong.
- **>**, **>=**, **<**, **<=**, dan **<>** (tidak sama dengan) di depan angka atau kata membandingkan dengannya, misalnya **>25**. **<>** saja memilih sel yang tidak kosong.

Baris kondisi yang kosong memilih semua baris. Untuk menampilkan kembali baris yang tersembunyi, pilih **Hapus**. Salinan dibuat seketika dan tidak berubah ketika daftar berubah. Filter Lanjutan adalah satu langkah yang dapat diurungkan.

## Validasi Data

Gunakan ini untuk menentukan apa yang boleh diketik pada beberapa sel, sehingga entri yang salah dihentikan atau dipertanyakan.

1. Pilih sel-selnya.
2. Pilih **Validasi Data** pada grup **Alat Data** di tab **Data**.
3. Pada tab **Pengaturan**, pilih yang akan **Diizinkan**: **Nilai apa pun** (tanpa aturan), **Bilangan bulat**, **Desimal**, **Daftar**, **Tanggal**, **Waktu**, **Panjang teks**, atau **Kustom**.
4. Untuk angka, tanggal, waktu, atau panjang teks, pilih **Data** (**antara**, **tidak di antara**, **sama dengan**, dan seterusnya) lalu ketik batasnya. Batas dapat berupa nilai atau rumus yang diawali dengan =.
5. Untuk **Daftar**, ketik pilihan dengan koma di antaranya, misalnya Merah,Hijau,Biru, atau ketik rentang sel setelah =, misalnya =$A$1:$A$5. Sel kemudian menampilkan panah berisi pilihan tersebut. Matikan **Daftar turun dalam sel** untuk menyembunyikan panah.
6. Untuk **Kustom**, ketik rumus yang diawali dengan =. Entri diizinkan ketika rumus benar.
7. **Abaikan kosong** membiarkan sel kosong lolos.

Pada tab **Pesan Masukan**, centang kotaknya dan tulis judul serta pesan. Pesan itu tampil di samping sel setiap kali sel dipilih.

Pada tab **Peringatan Galat**, pilih apa yang terjadi pada entri yang tidak diizinkan. **Hentikan** menolaknya. **Peringatan** menanyakan apakah entri tetap disimpan. **Informasi** memberi tahu pengguna dan tetap menyimpannya. Tulis judul dan pesan untuk peringatan tersebut.

**Lingkari Data Tidak Valid** (di samping **Validasi Data**) menempatkan lingkaran oval merah di sekeliling setiap sel pada lembar yang nilainya tidak diizinkan oleh aturannya, misalnya angka yang sudah ada sebelum aturan dibuat, atau hasil rumus. Sebuah pesan memberi tahu Anda jika semua sel sudah benar. Sel yang Anda perbaiki kehilangan lingkarannya. **Hapus Lingkaran Validasi** menghapus semua lingkaran tersebut.

**Hapus Semua** menghapus aturan dari sel yang dipilih. Aturan baru menggantikan aturan apa pun yang sebelumnya dimiliki sel yang dipilih; sel lain tetap memiliki aturannya. Mengubah aturan adalah satu langkah yang dapat diurungkan, dan aturan disimpan dalam berkas.

## Konsolidasi

Gunakan ini untuk menggabungkan rentang yang mirip, seperti tabel yang sama dari beberapa lembar, menjadi satu tabel.

1. Pilih **Konsolidasi** pada grup **Alat Data** di tab **Data**.
2. Pilih **Fungsi** yang akan digabungkan, misalnya **Jumlah** atau **Rata-rata**.
3. Ketik rentang pada kotak dan tekan **Tambah**. Gunakan **Sheet2!A1:C10** untuk rentang pada lembar lain. Tambahkan setiap rentang yang ingin digabungkan. Pilih rentang dalam daftar dan tekan **Hapus** untuk mengeluarkannya.
4. Jika rentang memiliki nama pada baris pertamanya, centang **Baris teratas berisi nama kolom**. Jika memiliki nama pada kolom pertamanya, centang **Kolom kiri berisi nama baris**.
5. Pada **Letakkan hasil di**, ketik sel pertama untuk hasilnya.
6. Centang **Buat tautan ke data sumber** jika setiap hasil harus berupa rumus yang membaca sel sumbernya, sehingga ikut berubah ketika sel sumber berubah. Tanpa centang ini, hasilnya berupa angka biasa.
7. Tekan **OK**.

Jika tidak ada nama yang dicentang, sel pada posisi yang sama di setiap rentang digabungkan (sel pertama dari setiap rentang, lalu sel kedua, dan seterusnya). Jika nama dicentang, sel dengan nama yang sama digabungkan, di mana pun posisinya dalam setiap rentang, dan nama yang hanya dimiliki sebagian rentang tetap mendapatkan baris atau kolom. Konsolidasi adalah satu langkah yang dapat diurungkan.
