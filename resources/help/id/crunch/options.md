# Mengubah pengaturan JeffCrunch

<!-- keywords: opsi rumus, pengaturan perhitungan, perubahan maksimum, iterasi, pemulihan, pulihkan pekerjaan yang belum disimpan, pemulihan otomatis, pemeriksaan, pengeditan, isi otomatis, pegangan isi, seret dan lepas, tempat desimal, opsi, pengaturan, preferensi, statistik penggunaan, nomor instalasi, privasi, pemberitahuan privasi, JP_NO_TELEMETRY, hapus statistik, pembaruan, tentang, edit di sel, warna garis kisi, zoom default, lebar standar, privasi, informasi pribadi, sesuaikan keyboard, sesuaikan pita, sesuaikan bilah alat cepat, bilah alat cepat, tambahkan perintah ke bilah alat cepat, sembunyikan tab, sembunyikan grup, tambahkan tab, tambahkan grup, ganti nama tab, atur ulang pita, ubah pintasan keyboard, tetapkan ulang tombol, atur ulang tombol -->

Pengaturan Anda ada di jendela **Opsi JeffCrunch**. Klik **File > Opsi** untuk membukanya. Klik **OK** untuk menyimpan perubahan, atau **Batal** untuk membuangnya.

## Umum

**Tampilkan Bilah Alat Mini saat Memilih** aktif secara bawaan. Opsi ini menentukan apakah bilah format kecil muncul di dekat teks yang dipilih dan di bagian atas menu pintasan sel (lihat [Mengetik di sel](edit-cells)). Pilihan ini dipakai bersama oleh aplikasi JeffOffice.

## Pengeditan

Halaman **Pengeditan** mengubah cara mengetik di sel bekerja.

- **Setelah menekan Enter, pindahkan pilihan** menentukan ke mana pilihan berpindah saat Anda menyelesaikan entri: ke bawah, kanan, atas, kiri, atau tidak ke mana-mana.
- **Tawarkan untuk melengkapi teks dari sel di atas dan di bawah** disebut Isi Otomatis. Saat Anda mengetik beberapa huruf pertama sebuah teks, dan hanya satu teks dalam kolom itu yang diawali huruf tersebut, JeffCrunch menampilkan sisanya, dalam keadaan dipilih. Tekan **Enter** untuk menerimanya. Lanjutkan mengetik untuk mengabaikannya, atau tekan **Backspace** untuk menghapusnya. Isi Otomatis hanya melihat blok sel di sebelah sel tempat Anda mengetik, dan hanya teks, bukan angka.
- **Gunakan pegangan isi, dan seret sel untuk memindahkan atau menyalin** mengaktifkan kotak kecil di sudut pilihan dan penyeretan batas pilihan. Nonaktifkan jika Anda sering memindahkan sel secara tidak sengaja. Salin dan Tempel tetap berfungsi.
- **Tampilkan nol di sel yang bernilai nol (lembar ini)** mengaktifkan atau menonaktifkan tampilan nol untuk lembar yang sedang Anda lihat. Saat nonaktif, sel yang berisi 0 tampak kosong, tetapi tetap menyimpan 0 dan dihitung sebagai 0. Pilihan ini disimpan bersama lembar.
- **Tambahkan titik desimal pada bilangan bulat yang diketik** disebut tempat desimal otomatis. Atur **Tempat** ke 2, maka mengetik 1234 menghasilkan 12,34. Atur ke -2, maka mengetik 5 menghasilkan 500. Angka yang Anda ketik dengan titik desimal dimasukkan apa adanya.
- **Izinkan pengeditan langsung di sel** aktif secara bawaan. Jika Anda menonaktifkannya, entri yang diketik atau F2 akan masuk ke bilah rumus, dan sel hanya menampilkan hasilnya saat Anda selesai mengetik.

## Opsi rumus

Halaman **Rumus** mengatur cara buku kerja yang terbuka menghitung rumusnya: **Perhitungan** (otomatis, otomatis kecuali tabel data, atau manual), apakah rumus dihitung sebelum buku kerja disimpan saat perhitungan manual, apakah rumus yang membaca dirinya sendiri dihitung dengan cara berputar, jumlah putaran maksimum, dan perubahan yang menghentikan putaran. Pengaturan ini milik buku kerja, bukan milik Anda: disimpan dalam berkasnya dan tidak diingat untuk buku kerja lain, dan pilihannya nonaktif saat tidak ada buku kerja yang terbuka. Perubahan ditampilkan dengan semua digitnya, dan Anda dapat mengetiknya sebagai angka, misalnya 0,0000001 atau 1e-7. Hanya pengaturan yang Anda ubah yang dicatat: **OK** tanpa perubahan tidak menambah langkah Urungkan dan tidak mengubah buku kerja. Satu perubahan adalah satu langkah Urungkan. Berkas buku kerja yang lebih baru dan berkas .xls yang lebih lama menyimpan semua pengaturan. Salinan .ods menyimpan pengaturan iterasi, tetapi tidak memiliki tempat untuk perhitungan manual, sehingga salinan itu menghitung rumus saat nilainya berubah, dan laporan sebelum salinan disimpan menyebutkannya. Pengaturan yang sama juga ada pada tab **Rumus**.

## Opsi penyimpanan

**Simpan informasi pemulihan otomatis setiap** menentukan seberapa sering JeffCrunch menyimpan salinan buku kerja yang memiliki perubahan belum disimpan, dari 1 hingga 120 menit. Salinan berupa berkas .xlsx dalam folder yang disediakan JeffCrunch untuk keperluan ini, dibuat di latar belakang sehingga Anda tetap bisa mengetik. Salinan dihapus saat Anda menyimpan atau menutup buku kerja. Jika JeffCrunch tertutup tiba-tiba, JeffCrunch menawarkan salinan tersebut saat dijalankan berikutnya. Anda juga dapat mengklik **File > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Buku kerja yang dilindungi kata sandi tidak disalin, karena salinannya tidak akan terlindungi.

## Opsi pemeriksaan

Halaman **Pemeriksaan** dipakai bersama oleh aplikasi JeffOffice. Halaman ini menentukan apakah ejaan diperiksa saat Anda mengetik di sel (nonaktifkan **Periksa ejaan saat mengetik** untuk menghilangkan garis merah bergelombang di setiap jendela yang terbuka dan aplikasi lainnya; **Ejaan** pada tab Tinjau tetap berfungsi), apakah kata dengan huruf kapital dan kata yang mengandung angka diabaikan saat ejaan diperiksa, apakah Koreksi Otomatis memperbaiki kata saat Anda mengetik, apakah tanda kutip lurus diubah menjadi tanda kutip melengkung, apakah tanda hubung, pecahan, dan ordinal diubah, dan apakah alamat web menjadi tautan. Tab **Simbol Matematika** pada Opsi Koreksi Otomatis diperuntukkan bagi JeffWrite, JeffDeck, dan JeffPub: JeffCrunch tidak mengubah garis miring terbalik dan sebuah nama menjadi simbol di dalam sel. Lihat [Ejaan](notes-links) dan [Mengetik di sel](edit-cells).

## Opsi tampilan

- **Warna garis kisi** adalah warna garis di antara sel pada layar Anda. Ini hanya pilihan untuk layar Anda, tidak disimpan dalam buku kerja.
- **Zoom buku kerja baru** menentukan seberapa besar lembar buku kerja baru tampil, dari 10% hingga 400%.
- **Lebar kolom standar (lembar ini)** adalah lebar setiap kolom pada lembar ini yang tidak memiliki lebar sendiri. Ini sama dengan **Lebar Standar** pada tab Beranda.
- **Tampilkan tab lembar** dan **Tampilkan bilah gulir** menyembunyikan atau menampilkan baris tab lembar di bawah sel serta bilah gulir di kanan dan bawah. Keduanya hanya pilihan untuk layar Anda, dan berlaku untuk setiap buku kerja yang Anda buka.
- **Tampilkan catatan sel sebagai** memilih tampilan sel yang memiliki catatan: sebuah tanda kecil pada sel dan catatannya saat penunjuk diarahkan ke sel tersebut, hanya tanda, atau tidak ada apa pun. Catatan yang Anda pilih untuk ditampilkan terus-menerus (**Tampilkan Catatan**) tetap ada di lembar dalam semua keadaan.

## Sesuaikan

Tab **Sesuaikan** memiliki tiga halaman, yaitu **Pita**, **Bilah Alat Cepat**, dan **Keyboard**. Halaman **Keyboard** adalah tempat Anda memilih tombol pintas untuk perintah JeffCrunch. Lihat "Pilih tombol pintas Anda sendiri" pada topik keyboard.

### Menyesuaikan pita

Halaman **Pita** mencantumkan tab pita, masing-masing dengan grupnya.

- **Sembunyikan atau tampilkan tab atau grup.** Hapus centang kotak di sebelahnya untuk menyembunyikannya, dan centang kembali untuk menampilkannya. Grup yang disembunyikan tidak memakan ruang, dan perintahnya tetap bisa dijalankan dengan tombol pintas. **Beranda** tidak memiliki kotak, karena selalu ditampilkan, begitu pula tombol **File**. Tab yang hanya muncul saat sesuatu dipilih, misalnya gambar atau tabel, berada di akhir daftar dan tidak memiliki kotak.
- **Buat tab atau grup Anda sendiri.** Ketik nama di kotak **Nama**, lalu klik **Tab Baru**. Untuk membuat grup, pilih sebuah tab dan klik **Grup Baru**. Grup milik Anda dapat ditempatkan di tab mana pun, termasuk tab yang hanya muncul saat sesuatu dipilih.
- **Tambahkan perintah ke grup Anda.** Pilih grup Anda, cari perintah dalam daftar **Perintah** (ketik di kotak **Cari perintah** untuk mempersempit daftar), lalu klik **Tambahkan Perintah**. Perintah apa pun yang memiliki nama dapat dimasukkan. Sebuah perintah hanya dapat berada dalam satu grup sekali.
- **Ganti nama.** Pilih tab, grup, atau perintah milik Anda, ketik nama baru, lalu klik **Ganti Nama**. Perintah yang diganti namanya tetap melakukan hal yang sama, dan nama baru hanya tampil pada tombolnya. Untuk mengembalikan nama asli perintah, ganti namanya dengan nama kosong.
- **Atur ulang urutan.** Pilih sebuah tab dan klik **Pindah ke Atas** atau **Pindah ke Bawah**. Anda dapat memindahkan tab bawaan pita maupun tab milik Anda, tetapi tidak tab yang hanya muncul saat sesuatu dipilih. Grup dan perintah milik Anda dipindahkan dengan cara yang sama di dalam tab atau grupnya. Grup bawaan pita tetap pada urutannya.
- **Hapus.** Pilih tab, grup, atau perintah milik Anda, lalu klik **Hapus**.

**Atur Ulang Satu** mengembalikan tab yang dipilih: tab itu ditampilkan, grupnya ditampilkan, dan grup yang Anda tambahkan ke tab itu dihapus. Tab milik Anda akan dihapus. **Atur Ulang Semua** mengembalikan seluruh pita, termasuk urutan tabnya.

Tidak ada perubahan sampai Anda mengklik **OK**. Setelah itu, setiap jendela JeffCrunch yang terbuka menampilkan pita Anda, begitu pula setiap jendela yang Anda buka kemudian. JeffCrunch hanya menyimpan yang berbeda dari pita bawaan, sehingga tab atau grup yang ditambahkan versi berikutnya akan muncul dengan sendirinya. Tab dan grup Anda mendapat KeyTips (huruf yang ditampilkan saat Anda menekan **Alt**) yang tidak bertabrakan dengan yang lain. Anda dapat memiliki 20 tab milik sendiri, 30 grup milik sendiri pada satu tab, dan 60 perintah dalam satu grup.

### Menyesuaikan bilah alat cepat

Halaman **Bilah Alat Cepat** untuk bilah alat kecil di pojok kiri atas jendela. Daftar kiri menampilkan apa yang ada di bilah alat saat ini, dan daftar **Perintah** di kanan memuat setiap perintah yang memiliki nama (ketik di kotak **Cari perintah** untuk mempersempitnya, sama seperti pada halaman **Pita**).

- **Tambahkan perintah.** Pilih perintah dalam daftar **Perintah**, lalu klik **Tambahkan Perintah**. Perintah itu ditempatkan di akhir bilah alat. Perintah yang membuka menu akan tetap membuka menunya dari bilah alat. Sebuah perintah hanya dapat berada di bilah alat sekali.
- **Klik kanan.** Anda juga dapat mengklik kanan perintah mana pun pada pita dan memilih **Tambahkan ke Bilah Alat Cepat**, atau mengklik kanan tombol pada bilah alat dan memilih **Hapus dari Bilah Alat Cepat**. Menu yang sama memiliki **Sesuaikan Bilah Alat Cepat** dan **Sesuaikan Pita**, yang membuka halaman ini.
- **Pindahkan ke bawah pita.** Centang **Tampilkan Bilah Alat Cepat di bawah pita** untuk menempatkan bilah alat dalam satu baris di bawah pita, bukan di pojok kiri atas. Kedua menu klik kanan juga memiliki **Tampilkan Bilah Alat Cepat di Bawah Pita**. JeffCrunch mengingat pilihan ini.
- **Tambahkan pemisah.** Klik **Tambahkan Pemisah** untuk menambahkan garis tipis di akhir, lalu pindahkan ke posisi yang Anda inginkan.
- **Atur ulang urutan.** Pilih perintah atau pemisah di daftar kiri, lalu klik **Pindah ke Atas** atau **Pindah ke Bawah**.
- **Hapus.** Pilih item tersebut, lalu klik **Hapus**.

**Atur Ulang Satu** mengembalikan perintah yang dipilih: ke tempat JeffCrunch biasanya menaruhnya, atau keluar dari bilah alat jika JeffCrunch biasanya tidak menaruhnya di sana. **Atur Ulang Semua** mengembalikan bilah alat seperti yang disediakan JeffCrunch. Atur Ulang Semua pada halaman ini tidak mengubah pita Anda, dan Atur Ulang Semua pada halaman **Pita** tidak mengubah bilah alat Anda.

Tidak ada perubahan sampai Anda mengklik **OK**. Setelah itu, setiap jendela JeffCrunch yang terbuka menampilkan bilah alat Anda, begitu pula setiap jendela yang Anda buka kemudian. Bilah alat dapat memuat hingga 100 perintah dan pemisah. Jika tombolnya lebih banyak daripada yang muat di sepertiga lebar jendela, sisanya menunggu di balik tombol **»** di akhir bilah alat, sehingga judul jendela tidak pernah terdorong keluar layar. JeffCrunch tetap menyimpan sebuah perintah dalam daftar Anda ketika versi berikutnya tidak lagi memilikinya, dan menampilkannya lagi jika perintah itu kembali. Tombol pada bilah alat mendapat KeyTips, yaitu angka 1 sampai 9 lalu dua huruf, sesuai urutannya pada bilah alat. Bilah alat selalu berada di pojok kiri atas, di samping tombol **File**.

## Opsi privasi

- **Hapus informasi pribadi dari berkas ini saat menyimpan** menghapus pengarang, orang yang terakhir menyimpan berkas, perusahaan, manajer, serta nama dan waktu pada catatan dan komentar, dari berkas yang ditulis (buku kerja, salinan, dan buku kerja yang dikirim lewat email). Isi berkas tetap sama. Properti kustom berkas dibiarkan apa adanya, dan dokumen yang sedang Anda buka tidak berubah. Pilihan ini sama di setiap aplikasi JeffOffice: mengubahnya di satu aplikasi juga mengubahnya di semua aplikasi. Untuk melakukannya sekali saja, gunakan **Periksa Buku Kerja** di [Info](info). Kotak yang sama ada pada halaman **Info**.

## Statistik penggunaan

**Kirim statistik penggunaan anonim** memungkinkan Anda membagikan fakta tentang cara JeffCrunch digunakan. Anonim berarti fakta tersebut tidak mengungkap siapa Anda. Saat opsi ini aktif, JeffCrunch mengirim fakta ini sekali sehari, dan pada hari Anda memperbarui: bahwa ini JeffCrunch, versinya, sistem operasi dan bahasa Anda, berapa kali JeffCrunch dijalankan, seberapa sering setiap perintah dipakai, dan jenis berkas yang Anda buka atau simpan. Jika JeffCrunch pernah tertutup tiba-tiba, laporan kerusakan juga dikirim saat JeffCrunch dijalankan berikutnya. JeffCrunch tidak pernah mengirim berkas Anda, nama berkas, atau isinya, dan tidak pula apa yang Anda ketik. Jika opsi ini nonaktif, laporan kerusakan juga tidak dikirim.

Pilihan ini berlaku untuk semua aplikasi JeffOffice: mengubahnya di sini berarti mengubahnya di setiap aplikasi. JeffCrunch menanyakannya pertama kali dijalankan, dalam jendela bernama **Bantu tingkatkan JeffCrunch**.

- **Pemberitahuan privasi** di samping kotak centang membuka pemberitahuan privasi kami di jeffoffice.com/privacy. Di sana dijelaskan apa yang dikirim, ke mana dikirim, dan berapa lama kami menyimpannya. Kami menyimpan statistik selama 24 bulan.
- **Nomor instalasi** ditampilkan di bawah kotak centang. Ini adalah angka acak yang dibuat untuk salinan JeffOffice Anda, sama di setiap aplikasi JeffOffice. Angka ini tidak mengungkap apa pun tentang Anda. Klik **Salin** untuk menyalinnya. Jika statistik nonaktif, atau belum ada angka yang dibuat, tertulis **tidak ada**.
- Untuk meminta penghapusan statistik yang sudah kami simpan, kirim nomor instalasi Anda ke info@jeffoffice.com **sebelum** Anda menonaktifkan statistik. Menonaktifkan statistik akan menghapus nomor itu dari komputer Anda, dan tanpanya kami tidak dapat mengetahui catatan mana yang milik Anda.
- Untuk menonaktifkan statistik tanpa membuka JeffCrunch, atur variabel lingkungan `JP_NO_TELEMETRY` ke nilai apa pun sebelum Anda menjalankannya. Dengan begitu tidak ada yang dihitung atau dikirim, dan JeffCrunch tidak menanyakan pertanyaan saat pertama kali dijalankan.

## Keamanan

Tab **Keamanan** menentukan apa yang dilakukan JeffCrunch terhadap hal-hal yang dapat ditunjuk oleh sebuah berkas: berkas tertaut, film dan suara tertaut, serta berkas dari internet. Makro disimpan dalam berkas dan tidak pernah dijalankan, dan tautan yang akan menjalankan program selalu ditolak. Lihat [Keamanan](security).

## Terkait

- [Selamat datang di JeffCrunch](start)
