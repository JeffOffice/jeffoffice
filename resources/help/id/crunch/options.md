# Mengubah pengaturan JeffCrunch
<!-- keywords: opsi rumus, pengaturan perhitungan, perubahan maksimum, iterasi, pemulihan, pulihkan pekerjaan yang belum disimpan, pemulihan otomatis, pemeriksaan, pengeditan, isi otomatis, pegangan isi, seret dan lepas, tempat desimal, opsi, pengaturan, preferensi, statistik penggunaan, nomor instalasi, privasi, pemberitahuan privasi, JP_NO_TELEMETRY, hapus statistik, pembaruan, tentang, edit di dalam sel, warna garis kisi, zoom bawaan, lebar standar, privasi, informasi pribadi, sesuaikan pintasan keyboard, sesuaikan pita, sesuaikan bilah alat cepat, bilah alat cepat, tambahkan perintah ke bilah alat cepat, sembunyikan tab, sembunyikan grup, tambahkan tab, tambahkan grup, ganti nama tab, atur ulang pita, ubah tombol pintas, tetapkan ulang tombol, atur ulang tombol -->

Pengaturan Anda ada di jendela **Opsi JeffCrunch**. Klik **File > Opsi** untuk membukanya. Klik **OK** untuk menyimpan perubahan, atau **Batal** untuk membuangnya.

## Umum

**Tampilkan Bilah Alat Mini saat Memilih** aktif secara bawaan. Opsi ini menentukan apakah bilah format kecil muncul di dekat teks yang dipilih dan di bagian atas menu pintasan sel (lihat [Mengetik di sel](edit-cells)). Pilihan ini dipakai bersama oleh aplikasi JeffOffice.

## Pengeditan

Halaman **Pengeditan** mengubah cara mengetik di dalam sel.

- **Setelah menekan Enter, pindahkan pilihan** menentukan ke mana pilihan berpindah saat Anda selesai mengisi: ke bawah, kanan, atas, kiri, atau tidak pindah.
- **Tawarkan untuk melengkapi teks dari sel di atas dan di bawah** disebut Isi Otomatis. Saat Anda mengetik beberapa huruf awal sebuah teks, dan hanya satu teks dalam kolom itu yang diawali huruf tersebut, JeffCrunch menampilkan sisa teks itu dalam keadaan terpilih. Tekan **Enter** untuk menerimanya. Lanjutkan mengetik untuk mengabaikannya, atau tekan **Backspace** untuk menghapusnya. Isi Otomatis hanya melihat blok sel di sebelah sel yang sedang Anda ketik, dan hanya teks, bukan angka.
- **Gunakan pegangan isi, dan seret sel untuk memindahkan atau menyalin** mengaktifkan kotak kecil di sudut pilihan dan penyeretan tepi pilihan. Nonaktifkan jika Anda sering memindahkan sel secara tidak sengaja. Salin dan Tempel tetap berfungsi.
- **Tampilkan nol di sel yang bernilai nol (lembar ini)** mengaktifkan atau menonaktifkan tampilan nol untuk lembar yang sedang Anda lihat. Saat dinonaktifkan, sel yang berisi 0 tampak kosong, tetapi tetap menyimpan 0 dan dijumlahkan sebagai 0. Pilihan ini disimpan bersama lembar.
- **Tambahkan titik desimal pada angka bulat yang diketik** disebut tempat desimal otomatis. Atur **Tempat** ke 2, maka mengetik 1234 akan dimasukkan sebagai 12,34. Atur ke -2, maka mengetik 5 akan dimasukkan sebagai 500. Angka yang Anda ketik dengan titik desimal dimasukkan apa adanya.

- **Izinkan pengeditan langsung di dalam sel** aktif secara bawaan. Jika Anda menonaktifkannya, entri yang diketik atau F2 akan diarahkan ke bilah rumus, dan sel hanya menampilkan hasilnya saat Anda selesai mengetik.

## Opsi rumus

Halaman **Rumus** mengatur cara buku kerja yang terbuka menghitung rumusnya: **Perhitungan** (otomatis, otomatis kecuali tabel data, atau manual), apakah rumus dihitung sebelum buku kerja disimpan saat perhitungan dilakukan secara manual, apakah rumus yang membaca dirinya sendiri dihitung dengan cara berputar, jumlah putaran maksimum, dan perubahan yang menghentikan putaran. Pengaturan ini menjadi milik buku kerja, bukan milik Anda: disimpan dalam berkasnya dan tidak diingat untuk buku kerja lain, dan pilihannya nonaktif saat tidak ada buku kerja yang terbuka. Perubahan ditampilkan dengan semua digitnya, dan Anda dapat mengetiknya sebagai angka, misalnya 0,0000001 atau 1e-7. Hanya pengaturan yang Anda ubah yang ditulis: **OK** tanpa perubahan tidak menambah langkah Urungkan dan membiarkan buku kerja seperti semula. Satu perubahan adalah satu langkah Urungkan. Berkas buku kerja yang lebih baru dan berkas .xls yang lebih lama menyimpan semua pengaturan. Salinan .ods menyimpan pengaturan iterasi, tetapi tidak memiliki tempat untuk perhitungan manual, sehingga salinan itu menghitung rumus setiap kali nilainya berubah, dan laporan sebelum salinan disimpan menyebutkan hal ini. Pengaturan yang sama juga ada pada tab **Rumus**.

## Opsi penyimpanan

**Simpan informasi pemulihan otomatis setiap** menentukan seberapa sering JeffCrunch menyimpan salinan buku kerja yang memiliki perubahan belum disimpan, dari 1 hingga 120 menit. Salinan berupa berkas .xlsx di dalam folder yang disediakan JeffCrunch untuk keperluan ini, dibuat di latar belakang sehingga Anda tetap dapat mengetik. Salinan dihapus saat Anda menyimpan atau menutup buku kerja. Jika JeffCrunch tertutup secara tiba-tiba, JeffCrunch akan menawarkan salinan tersebut saat dijalankan berikutnya. Anda juga dapat klik **File > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Buku kerja yang dilindungi kata sandi tidak disalin, karena salinannya tidak akan terlindungi.

## Opsi pemeriksaan

Halaman **Pemeriksaan** dipakai bersama oleh aplikasi JeffOffice. Halaman ini menentukan apakah ejaan diperiksa saat Anda mengetik di sel (nonaktifkan **Periksa ejaan saat mengetik** untuk menghilangkan garis bergelombang merah di setiap jendela yang terbuka dan aplikasi lainnya; **Ejaan** pada tab Tinjau tetap berfungsi), apakah kata yang seluruhnya huruf kapital dan kata yang mengandung angka diabaikan saat ejaan diperiksa, apakah Koreksi Otomatis memperbaiki kata saat Anda mengetik, apakah tanda kutip lurus diubah menjadi tanda kutip melengkung, apakah tanda hubung, pecahan, dan ordinal diubah, dan apakah alamat web menjadi tautan. Lihat [Ejaan](notes-links) dan [Mengetik di sel](edit-cells).

## Opsi tampilan

- **Warna garis kisi** adalah warna garis di antara sel pada layar Anda. Pilihan ini hanya untuk layar Anda dan tidak disimpan dalam buku kerja.
- **Zoom buku kerja baru** menentukan seberapa besar lembar pada buku kerja baru tampak, dari 10% hingga 400%.
- **Lebar kolom standar (lembar ini)** adalah lebar setiap kolom pada lembar ini yang tidak memiliki lebar sendiri. Ini sama dengan **Lebar Standar** pada tab Beranda.

## Sesuaikan

Tab **Sesuaikan** memiliki tiga halaman, **Pita**, **Bilah Alat Cepat**, dan **Pintasan Keyboard**. Halaman **Pintasan Keyboard** adalah tempat Anda memilih tombol pintasan untuk perintah JeffCrunch. Lihat "Pilih tombol pintasan sendiri" pada topik keyboard.

### Sesuaikan pita

Halaman **Pita** menampilkan tab pita beserta grupnya.

- **Sembunyikan atau tampilkan tab atau grup.** Hapus centang kotak di sebelahnya untuk menyembunyikannya, dan centang kembali untuk menampilkannya. Grup yang disembunyikan tidak memakan ruang, dan perintahnya tetap dapat dijalankan dengan tombol pintasannya. **Beranda** tidak memiliki kotak, karena selalu tampil, dan begitu juga tombol **File**. Tab yang hanya muncul saat sesuatu dipilih, seperti gambar atau tabel, berada di akhir daftar dan tidak memiliki kotak.
- **Buat tab atau grup sendiri.** Ketik nama di kotak **Nama** lalu klik **Tab Baru**. Untuk membuat grup, pilih tab lalu klik **Grup Baru**. Grup milik Anda dapat ditempatkan pada tab mana pun, termasuk tab yang hanya muncul saat sesuatu dipilih.
- **Masukkan perintah ke grup Anda.** Pilih grup Anda, cari perintah di daftar **Perintah** (ketik di kotak **Cari perintah** untuk mempersempit daftar), lalu klik **Tambahkan Perintah**. Perintah apa pun yang memiliki nama dapat dimasukkan. Sebuah perintah hanya dapat berada satu kali dalam sebuah grup.
- **Ganti nama.** Pilih tab, grup, atau perintah milik Anda, ketik nama baru, lalu klik **Ganti Nama**. Perintah yang diganti namanya tetap melakukan hal yang sama, dan nama baru hanya menjadi tulisan pada tombolnya. Untuk mengembalikan nama asli perintah, ganti namanya dengan nama kosong.
- **Atur ulang urutan.** Pilih tab lalu klik **Pindah ke Atas** atau **Pindah ke Bawah**. Anda dapat memindahkan tab bawaan pita maupun tab milik Anda, tetapi tidak tab yang hanya muncul saat sesuatu dipilih. Grup dan perintah milik Anda berpindah dengan cara yang sama di dalam tab atau grupnya. Grup bawaan pita tetap pada urutannya.
- **Hapus.** Pilih tab, grup, atau perintah milik Anda lalu klik **Hapus**.

**Atur Ulang Satu** mengembalikan tab yang dipilih: tab itu ditampilkan, grupnya ditampilkan, dan grup yang Anda tambahkan ke dalamnya dihapus. Tab milik Anda akan dihapus. **Atur Ulang Semua** mengembalikan seluruh pita, termasuk urutan tab.

Tidak ada yang berubah sampai Anda klik **OK**. Setelah itu, setiap jendela JeffCrunch yang terbuka menampilkan pita Anda, dan begitu pula setiap jendela yang Anda buka nanti. JeffCrunch hanya menyimpan bagian yang berbeda dari pita bawaan, sehingga tab atau grup yang ditambahkan versi berikutnya akan muncul dengan sendirinya. Tab dan grup Anda mendapatkan KeyTips (huruf yang muncul saat Anda menekan **Alt**) yang tidak bentrok dengan yang lain. Anda dapat memiliki 20 tab milik sendiri, 30 grup milik sendiri pada satu tab, dan 60 perintah dalam satu grup.

### Sesuaikan bilah alat cepat

Halaman **Bilah Alat Cepat** untuk bilah alat kecil di pojok kiri atas jendela. Daftar kiri menampilkan apa yang ada di bilah alat saat ini, dan daftar **Perintah** di sebelah kanan memuat setiap perintah yang memiliki nama (ketik di kotak **Cari perintah** untuk mempersempit daftar, sama seperti pada halaman **Pita**).

- **Tambahkan perintah.** Pilih perintah di daftar **Perintah** lalu klik **Tambahkan Perintah**. Perintah akan ditambahkan di akhir bilah alat. Perintah yang membuka menu akan mempertahankan menunya di bilah alat. Sebuah perintah hanya dapat berada satu kali di bilah alat.
- **Klik kanan.** Anda juga dapat klik kanan perintah mana pun di pita lalu pilih **Tambahkan ke Bilah Alat Cepat**, atau klik kanan tombol di bilah alat lalu pilih **Hapus dari Bilah Alat Cepat**. Menu yang sama memiliki **Sesuaikan Bilah Alat Cepat** dan **Sesuaikan Pita**, yang membuka halaman ini.
- **Pindahkan ke bawah pita.** Centang **Tampilkan Bilah Alat Cepat di bawah pita** untuk meletakkan bilah alat dalam satu baris di bawah pita, bukan di pojok kiri atas. Kedua menu klik kanan juga memiliki **Tampilkan Bilah Alat Cepat di Bawah Pita**. JeffCrunch mengingat pilihan ini.
- **Tambahkan pemisah.** Klik **Tambahkan Pemisah** untuk menambahkan garis tipis di akhir, lalu pindahkan ke posisi yang Anda inginkan.
- **Atur ulang urutan.** Pilih perintah atau pemisah di daftar kiri lalu klik **Pindah ke Atas** atau **Pindah ke Bawah**.
- **Hapus.** Pilih lalu klik **Hapus**.

**Atur Ulang Satu** mengembalikan perintah yang dipilih: ke tempat biasanya JeffCrunch menaruhnya, atau dikeluarkan dari bilah alat jika JeffCrunch biasanya tidak menaruhnya di sana. **Atur Ulang Semua** mengembalikan bilah alat seperti yang disediakan JeffCrunch. Atur Ulang Semua pada halaman ini tidak mengubah pita Anda, dan Atur Ulang Semua pada halaman **Pita** tidak mengubah bilah alat Anda.

Tidak ada yang berubah sampai Anda klik **OK**. Setelah itu, setiap jendela JeffCrunch yang terbuka menampilkan bilah alat Anda, dan begitu pula setiap jendela yang Anda buka nanti. Bilah alat dapat memuat 100 perintah dan pemisah. Jika tombolnya lebih banyak daripada yang muat di sepertiga lebar jendela, sisanya menunggu di balik tombol **»** di ujung bilah alat, sehingga judul jendela tidak pernah terdorong keluar layar. JeffCrunch menyimpan perintah dalam daftar Anda ketika versi berikutnya tidak lagi memilikinya, dan menampilkannya lagi jika perintah itu kembali. Tombol pada bilah alat mendapatkan KeyTips, yaitu angka 1 hingga 9 lalu dua huruf, sesuai urutannya di bilah alat. Bilah alat selalu berada di pojok kiri atas, di sebelah tombol **File**.

## Opsi privasi

- **Hapus informasi pribadi dari berkas ini saat menyimpan** menghapus nama pengarang, orang yang terakhir menyimpan berkas, perusahaan, manajer, serta nama dan waktu pada catatan dan komentar, dari berkas yang ditulis (buku kerja, salinan, dan buku kerja yang dikirim lewat email). Isi berkas tetap seperti semula. Properti kustom berkas dibiarkan apa adanya, dan dokumen yang sedang Anda buka tidak berubah. Pilihan ini sama di setiap aplikasi JeffOffice: ubah di salah satu, dan akan berubah juga di semuanya. Untuk melakukannya sekali saja, gunakan **Periksa Buku Kerja** di [Info](info). Kotak yang sama juga ada pada halaman **Info**.

## Statistik penggunaan

**Kirim statistik penggunaan anonim** memungkinkan Anda membagikan fakta tentang cara JeffCrunch digunakan. Anonim berarti fakta tersebut tidak memberi tahu siapa Anda. Saat opsi ini aktif, JeffCrunch mengirim fakta-fakta ini sekali sehari, dan pada hari Anda memperbarui: bahwa ini JeffCrunch, versinya, sistem operasi dan bahasa Anda, berapa kali JeffCrunch dijalankan, seberapa sering setiap perintah dipakai, dan jenis berkas yang Anda buka atau simpan. Jika JeffCrunch pernah tertutup secara tiba-tiba, JeffCrunch juga mengirim laporan kerusakan saat dijalankan berikutnya. JeffCrunch tidak pernah mengirim berkas Anda, nama berkasnya, atau isinya, dan tidak pula apa pun yang Anda ketik. Jika opsi ini nonaktif, laporan kerusakan juga tidak dikirim.

Pilihan ini berlaku untuk semua aplikasi JeffOffice: ubah di sini dan akan berubah di setiap aplikasi. JeffCrunch menanyakannya pertama kali dijalankan, di jendela bernama **Bantu tingkatkan JeffCrunch**.

- **Pemberitahuan privasi** di samping kotak centang membuka pemberitahuan privasi kami di jeffoffice.com/privacy. Di sana dijelaskan apa yang dikirim, ke mana dikirim, dan berapa lama kami menyimpannya. Kami menyimpan statistik selama 24 bulan.
- **Nomor instalasi** ditampilkan di bawah kotak centang. Ini adalah angka acak yang dibuat untuk salinan JeffOffice Anda, sama di setiap aplikasi JeffOffice. Angka ini tidak memberi tahu apa pun tentang Anda. Klik **Salin** untuk menyalinnya. Jika statistik nonaktif, atau belum ada angka yang dibuat, tertulis **tidak ada**.
- Untuk meminta penghapusan statistik yang sudah kami simpan, kirim nomor instalasi Anda ke info@jeffoffice.com **sebelum** Anda menonaktifkan statistik. Menonaktifkan statistik akan menghapus nomor tersebut dari komputer Anda, dan tanpa nomor itu kami tidak dapat mengetahui catatan mana yang milik Anda.
- Untuk menonaktifkan statistik tanpa membuka JeffCrunch, atur variabel lingkungan `JP_NO_TELEMETRY` ke nilai apa pun sebelum Anda menjalankannya. Dengan begitu, tidak ada yang dihitung atau dikirim, dan JeffCrunch tidak menanyakan pertanyaan saat pertama kali dijalankan.

## Keamanan

Tab **Keamanan** menentukan apa yang dilakukan JeffCrunch terhadap hal-hal yang dapat ditunjuk oleh sebuah berkas: berkas tertaut, film dan suara tertaut, serta berkas dari internet. Makro disimpan dalam berkas dan tidak pernah dijalankan, dan tautan yang akan menjalankan program selalu ditolak. Lihat [Keamanan](security).

## Terkait

- [Selamat datang di JeffCrunch](start)
