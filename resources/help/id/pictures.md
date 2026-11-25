# Menambahkan dan menyesuaikan gambar
<!-- keywords: efek gambar, bayangan, pantulan, cahaya, tepi lembut, bevel, rotasi 3-D, atur ulang gambar dan ukuran, foto, citra, sisipkan gambar, potong, klip seni, halaman PDF, jpg, png, svg, ubah warna, teks alternatif, keterangan, placeholder, pengelola grafis, gambar daring, tautkan ke berkas, gambar tertaut, sisipkan dan tautkan, sematkan, gambar hilang, tautkan ulang -->

Gambar berada di dalam sebuah bingkai. Anda dapat memotong gambar, serta memindahkan atau mengubah ukuran bingkainya.

## Menyisipkan gambar dari berkas

1. Klik **Sisipkan > Ilustrasi > Gambar**.
2. Di jendela **Sisipkan Gambar**, pilih sebuah berkas. JeffPub membuka berkas PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO, dan PDF.
3. Pilih **Sisipkan**, **Tautkan ke Berkas**, atau **Sisipkan dan Tautkan** di kotak **Sisipkan sebagai**, yang berada bersama daftar berkas dan dapat dicapai dengan tombol Tab. **Sisipkan** dipilih setiap kali jendela dibuka. Lihat "Menautkan gambar ke berkasnya" di bawah.
4. Klik **Buka**.

Gambar akan muncul di tengah halaman. Anda juga dapat menyeret berkas gambar ke halaman; cara ini selalu menggunakan **Sisipkan**.

JeffPub membaca berkas gambar hingga 256 MB. Berkas yang lebih besar ditolak, begitu pula berkas kosong atau sesuatu yang bukan berkas biasa di disk, seperti perangkat atau pipa. Pesannya menyatakan bahwa JeffPub tidak dapat membacanya sebagai gambar.

Jika Anda memilih beberapa gambar sekaligus, gambar-gambar itu akan tersusun sebagai miniatur di area coret-coret, yaitu ruang abu-abu di samping halaman. Seret gambar yang Anda inginkan ke halaman.

Jika sebuah PDF memiliki lebih dari satu halaman, jendela **Sisipkan Halaman PDF** menampilkan setiap halaman. Pilih satu dan klik **OK**.

## Menautkan gambar ke berkasnya

**Sisipkan** menaruh salinan gambar di dalam publikasi. Salinan itu ikut ke mana pun publikasi pergi, tetapi publikasi dengan banyak foto berukuran besar juga akan ikut besar. Tautan membuat foto tetap berada di berkasnya sendiri.

- **Sisipkan** menyalin gambar ke dalam publikasi. Ini pilihan yang paling umum.
- **Tautkan ke Berkas** hanya menyimpan lokasi berkas dan pratinjau kecil, tidak lebih dari 512 piksel pada sisi terpanjangnya. Gambar di halaman, serta dalam cetakan, PDF, atau e-book, diambil dari berkas dengan kualitas penuh. Jika berkas dipindahkan atau dihapus, halaman akan menampilkan pratinjau kecil.
- **Sisipkan dan Tautkan** melakukan keduanya. Publikasi menyimpan gambar utuh dan tautannya. Saat Anda membuka publikasi dan berkasnya telah berubah, JeffPub membaca ulang berkas dan mengganti salinannya.

JeffPub mengingat lokasi lengkap berkas, dan juga letaknya relatif terhadap publikasi, misalnya "folder Foto di sebelahnya". Jika Anda memindahkan publikasi dan gambarnya bersama-sama, lokasi kedua yang berlaku, sehingga gambar ditemukan kembali. Biarkan folder tetap seperti semula.

JeffPub membuka berkas gambar tertaut secara otomatis hanya jika berkas itu berada di folder publikasi atau di folder di dalamnya. Gambar yang ditautkan ke berkas di tempat lain menampilkan pratinjau tersimpannya, buram, dan ditandai **Belum diperbarui**, sampai Anda memilihnya dan mengklik **Perbarui Tautan** di Pengelola Grafis. Ini mencegah publikasi yang dikirim seseorang kepada Anda mengakses komputer Anda. Agar gambar dimuat sendiri, simpan gambar di folder publikasi.

**Pengelola Grafis** menunjukkan apakah setiap gambar berstatus **Tersemat**, **Tertaut**, **Hilang**, **Diubah**, atau **Belum diperbarui**, dan memiliki tombol untuk memperbarui tautan, mengarahkannya ke berkas lain, atau menjadikannya salinan. Lihat [Pengelola Grafis](graphics-manager).

Saat Anda menyimpan publikasi sebagai berkas `.pub`, membuat berkas Pack and Go, atau mengirimnya lewat email, JeffPub memasukkan setiap gambar tertaut ke dalam berkas secara utuh, sehingga berkas tidak bergantung pada berkas yang tertinggal. Jika berkas tertaut hilang saat itu, atau gambarnya **Belum diperbarui**, pratinjau kecilnya yang dimasukkan. Berkas `.pub` yang Anda buka tidak pernah memiliki tautan: gambarnya masuk sebagai salinan.

## Menambahkan placeholder gambar

Placeholder adalah bingkai kosong yang menyediakan tempat untuk gambar. Klik **Sisipkan > Ilustrasi > Placeholder Gambar**, lalu klik dua kali bingkai dan pilih sebuah berkas. Gambar akan mengisi bingkai, dan bagian tepi yang menjorok disembunyikan. Placeholder kosong tidak tercetak.

## Mencari gambar gratis secara daring

Klik **Sisipkan > Ilustrasi > Gambar Daring**. Panel **Gambar Daring** mencari di dua pustaka gambar gratis yang berlisensi terbuka: Openverse, yang mencakup banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Pilih sebuah pustaka, ketik yang Anda cari, misalnya "lumbung merah", lalu tekan Enter.
2. Klik sebuah gambar untuk melihat siapa pembuatnya dan lisensinya. Lisensi adalah seperangkat aturan penggunaan gambar. Kebanyakan meminta Anda mencantumkan nama pembuatnya.
3. Klik **Sisipkan**. JeffPub menaruh gambar di halaman dan menggunakan judulnya sebagai teks alternatif. Dengan **Tambahkan kredit di bawah gambar** dicentang, baris seperti "Lumbung merah" oleh Pat Lee, CC BY 2.0 ditaruh di bawahnya.

Kata pencarian Anda dikirim ke pustaka yang Anda pilih. Pastikan lisensi setiap gambar sesuai dengan cara Anda akan menggunakannya.

Satu pencarian menampilkan hingga 24 gambar. JeffPub hanya menggunakan alamat web aman (https) dari sebuah pustaka, dan membuang gambar yang alamatnya tidak demikian. Dari Wikimedia Commons, gambar yang lebarnya lebih dari 1.920 piksel diambil sebagai salinan selebar itu, bukan unggahan asli. Unduhan lebih dari 50 MB, atau yang terlalu lama, dihentikan, dan panel akan menyatakan bahwa gambar tidak dapat diunduh atau dimuat.

## Memotong gambar

Memotong berarti menyembunyikan tepi gambar.

1. Klik gambar, lalu klik **Format Gambar > Potong > Potong**.
2. Seret pegangan untuk menyembunyikan sebagian gambar. Seret gambar untuk menggesernya di dalam bingkai.
3. Klik **Potong** lagi, atau tekan **Esc**.

Juga di grup **Potong**: **Rasio Aspek** memotong bingkai menjadi bentuk seperti persegi, 4 lebar kali 3 tinggi, atau 16 kali 9, dengan berpusat di tengahnya (gambar tetap di tempatnya pada halaman). **Pas** menampilkan seluruh gambar di dalam bingkainya, **Isi** mengisi bingkai, **Potong ke Bentuk** memangkas gambar menjadi bentuk seperti oval, dan **Hapus Potongan** menampilkan kembali seluruh gambar.

## Mengubah tampilan gambar

Pada tab **Format Gambar**, gunakan grup **Sesuaikan**:

- **Hapus Latar Belakang**: menghilangkan latar belakang gambar, misalnya dinding polos di belakang seseorang atau bagian putih di sekitar logo. Klik gambar, lalu klik **Hapus Latar Belakang**. JeffPub mencari warna di sepanjang tepi gambar dan memberi warna merah muda pada bagian yang tampak sebagai latar belakang. Jika terlalu banyak atau terlalu sedikit yang dihapus, klik **Tandai Area untuk Dipertahankan** atau **Tandai Area untuk Dihapus** pada tab **Penghapusan Latar Belakang**, lalu seret di atas gambar; jika dua tanda bertemu, tanda yang Anda buat terakhir yang berlaku. **Simpan Perubahan** (atau Enter) mengakhiri proses dan menyimpan hasilnya, dan **Buang Semua Perubahan** (atau Esc) mengembalikan semuanya. Seluruh sesi ini menjadi satu langkah untuk diurungkan. Gambar tidak pernah terpotong: klik **Hapus Latar Belakang** lagi untuk melanjutkan penandaan, atau **Atur Ulang Gambar** untuk mengembalikan seluruh gambar. Gambar yang sangat besar menampilkan kotak dengan **Batal** selama JeffPub bekerja. Gambar berupa gambar garis dan bentuk (SVG) serta gambar animasi tidak dapat dihapus latar belakangnya. Berkas .jpub menyimpan penghapusan itu sehingga dapat diedit lagi. **Atur Ulang Gambar** membatalkan penghapusan yang dibuat di sini; **Ganti Gambar** tidak tersedia selama penandaan. Berpindah ke halaman lain, menghapus gambar, atau menyimpan akan mengakhiri Hapus Latar Belakang dan menyimpan apa yang sudah Anda tandai. Berkas .pub, PDF, halaman web, dan e-book hanya memuat gambar yang sudah jadi, dan Periksa Kompatibilitas menyatakannya untuk .pub. Gambar jadi disimpan pada ukuran tampilannya, sehingga bisa tampak lembut jika program lain memperbesarnya, dan JeffPub membuatnya ulang dari aslinya setiap kali Anda menyimpan di sini.
- **Koreksi**: pilih pasangan kecerahan dan kontras, dari minus 40 hingga plus 40 persen, atau **Pertajam** dan **Lembutkan** (masing-masing 25% dan 50% ke dua arah). **Format Objek…**, di bagian bawah menu, memiliki penggeser.
- **Ubah Warna** juga memiliki lima pilihan **Suhu**: negatif lebih dingin dan lebih biru, positif lebih hangat dan lebih merah.
- **Efek Artistik**: menampilkan contoh kecil gambar Anda dalam setiap efek (**Kabur**, **Sketsa Pensil Skala Abu-abu**, **Cahaya Tepi**, dan **Mosaik**). Pilih satu untuk menggunakannya, atau **Tanpa Efek Artistik** untuk menghapusnya. Setiap pilihan adalah satu langkah untuk diurungkan, dan **Atur Ulang Gambar** menghapusnya. Gambar aslinya tidak pernah berubah. Dalam berkas .pub, gambar jadi yang disimpan, dan **Periksa Kompatibilitas** menyatakan bahwa efek tidak dapat diubah di sana. Efek tidak menghapus gambar asli dari berkas: **Kabur** dan **Mosaik** tidak menyembunyikan apa pun dari orang yang menerima berkas, karena aslinya masih ada di dalamnya. Untuk menghapus aslinya, pilih **Berkas**, lalu **Info**, lalu **Periksa Publikasi**, dan klik **Hapus Asli** di samping **Gambar asli disimpan di bawah efek gambar**. Gambar kemudian hanya menyimpan tampilan jadinya, dan **Atur Ulang Gambar** tidak lagi dapat mengembalikan aslinya. Daftar yang sama memiliki **Bagian gambar yang dipotong**: gambar yang dipotong tetap menyimpan bagian yang Anda potong dari pandangan. **Hapus Bagian yang Dipotong** memangkas setiap gambar hanya sampai bagian yang tampak, sehingga halaman terlihat persis sama seperti sebelumnya (gambar dengan efek lebih dulu dijadikan gambar jadi, dan gambar yang digambar lebih kecil daripada ukuran penyimpanannya dibiarkan apa adanya, dan daftar akan menyatakannya). Selama efek masih dihitung, gambar **Kabur** atau **Mosaik** tampil sebagai bidang warna rata dengan warna rata-ratanya, tidak pernah sebagai gambar yang disembunyikannya, dan pertunjukan slaid, pencetakan, serta penyimpanan menunggu sampai selesai.
- **Ubah Warna**: jadikan gambar Skala abu-abu, Sepia, Pudar, Hitam dan Putih, atau rona aksen. **Atur Warna Transparan** membuat warna di sudut kiri atas gambar menjadi transparan.
- **Transparansi…**: memudarkan seluruh gambar.
- **Atur Ulang Gambar**: menghapus setiap perubahan pada daftar ini, serta garis tepi, bentuk, dan efek gambar. Pemotongan juga ikut dihapus, sehingga bingkai menampilkan seluruh gambar. Klik panah di sebelahnya untuk **Atur Ulang Gambar & Ukuran**, yang juga mengembalikan ukuran gambar seperti di berkasnya.
- **Kompres Gambar…**: memperkecil berkas dengan menurunkan detail menjadi 300, 220, atau 96 ppi (piksel per inci). Secara bawaan, semua gambar diubah dan area yang dipotong dihapus.
- **Ubah ke Bentuk**: mengubah gambar SVG (gambar yang dibuat dari garis dan bentuk) menjadi bentuk yang dapat Anda sunting, dipotong di tempat gambar itu terpotong. Teks dan gambar di dalamnya tidak disertakan.

## Gaya gambar, bingkai, dan efek

Gunakan grup **Gaya Gambar** pada tab **Format Gambar**:

- **Galeri** menampilkan sekitar dua puluh gaya siap pakai: bingkai, bayangan, pantulan, tepi lembut, bentuk membulat dan oval, putaran dalam ruang, cahaya, dan bevel. Bevel adalah tepi miring yang membuat gambar tampak timbul. Setiap kotak menampilkan warna Anda. Klik salah satu untuk memberi gambar tampilan itu. Sebuah gaya mengatur bentuk, bingkai, dan efek sekaligus, dan menggantikan yang sudah dimiliki gambar. Pemotongan, ukuran, dan warna tetap.
- **Garis Tepi Gambar** mengatur warna, **Bobot** (seberapa tebal garisnya), dan **Garis Putus-putus** (garis padat, titik-titik, atau putus-putus). **Tanpa Garis Tepi** menghapus garis tepi.
- **Efek Gambar** membuka **Bayangan**, **Pantulan**, **Cahaya**, **Tepi Lembut**, **Bevel**, dan **Rotasi 3-D**. Setiap menu memiliki pilihan siap pakai. Pilih salah satu untuk menambahkannya. Efek lain tetap ada. Pilih entri pertama, seperti **Tanpa Bayangan**, untuk menghapus salah satu efek. **Opsi Efek Lainnya…** membuka **Format Objek**.

Setiap langkah tersebut dapat diurungkan satu kali.

## Keterangan, teks alternatif, dan menukar gambar

- **Format Gambar > Gaya Gambar > Keterangan** menambahkan kotak teks di bawah gambar.
- Teks alternatif adalah deskripsi singkat untuk orang yang tidak dapat melihat gambar, seperti pengguna pembaca layar. Klik kanan gambar, pilih **Format Objek…**, buka tab **Teks Alternatif**, lalu ketik di **Teks alternatif**.
- **Format Gambar > Sisipkan > Ganti Gambar** memasukkan berkas lain ke bingkai yang sama. Bingkai mempertahankan ukuran, tempat, dan tampilannya: bingkai, bentuk, efek, dan perubahan warna. Jika gambar baru memiliki bentuk yang sama dengan yang lama, pemotongan juga tetap. Jika bentuknya berbeda, gambar mengisi bingkai.
- Untuk menukar dua gambar, seret satu ke yang lain. Saat tips menampilkan **Lepaskan untuk menukar gambar**, lepaskan tombol mouse: gambar bertukar bingkai, dan bingkai yang Anda seret kembali ke tempat semula. Atau tahan Shift, klik kedua gambar, lalu klik **Tukar**. Tahan Ctrl saat menyeret untuk menyalin gambar. Salinan yang Anda lepas di atas gambar lain tetap berada di tempat Anda meletakkannya, dan tidak ada yang bertukar.

## Melihat semua gambar

Klik **Tampilan > Tampilkan > Pengelola Grafis** untuk menampilkan setiap gambar dalam panel **Pengelola Grafis**. Pilih satu, lalu klik **Buka** untuk menuju ke gambar itu, **Simpan Sebagai…** untuk menyimpan salinan, atau **Ganti…** untuk memilih berkas lain. Untuk gambar tertaut, panel ini juga memiliki **Perbarui Tautan**, **Ubah Tautan…**, dan **Sematkan Gambar**. Lihat [Pengelola Grafis](graphics-manager).

## Terkait

- [Pengelola Grafis](graphics-manager)
- [Menggambar bentuk dan garis](shapes)
- [Memindahkan, merapikan, mengelompokkan, dan mengatur lapisan objek](arrange)
- [Tab Format Gambar](tab-picture-format)
