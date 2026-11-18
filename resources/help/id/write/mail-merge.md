# Penggabungan surat, amplop, dan label
<!-- keywords: penggabungan surat, cetak surat massal, gabung surat, surat massal, mail merge, merge, mailings, penerima, daftar alamat, surat formulir, label, amplop, bidang gabungan, blok alamat, baris salam, aturan, jika maka lainnya, lewati rekaman, rekaman berikutnya, pratinjau hasil, selesai dan gabungkan, direktori, pesan email, csv, xlsx, ods, vcard -->

**Penggabungan surat** membuat banyak salinan dari satu surat, masing-masing dengan rincian orang yang berbeda. Anda menulis surat sekali saja. Anda menyimpan rincian orang-orang tersebut dalam sebuah **daftar**. JeffWrite memasukkan nama dan alamat orang pertama ke salinan pertama, nama dan alamat orang kedua ke salinan kedua, dan seterusnya.

Alatnya terdapat pada tab **Surat Massal**. Untuk langkah yang terpandu, klik **Surat Massal > Mulai Cetak Surat Massal > Panduan Cetak Surat Massal Langkah demi Langkah**.

## Pilih yang akan Anda buat

Klik **Surat Massal > Mulai Cetak Surat Massal**, lalu pilih salah satu:

- **Surat**: satu salinan halaman untuk setiap orang. Setiap salinan dimulai di halaman baru.
- **Pesan Email**: satu pesan untuk setiap orang. Lihat "Mengirim pesan email" di bawah.
- **Amplop** dan **Label**: halaman yang dibuat khusus untuk keduanya. Lihat bagian di bawah.
- **Direktori**: satu daftar berisi semua orang, tanpa pemisah halaman di antara orang-orang tersebut. Gunakan untuk daftar telepon atau katalog.
- **Hapus Jenis Gabungan** membuat dokumen kembali menjadi dokumen biasa.

JeffWrite menyimpan jenis penggabungan dan nama daftar di dalam berkas saat Anda menyimpannya. Ketika berkas dibuka kembali, penggabungan tetap ada.

## Pilih penerima Anda

**Penerima** adalah orang yang mendapat salinan. Klik **Surat Massal > Pilih Penerima**:

- **Ketik Daftar Baru**: ketik orang-orang di jendela yang terbuka. Klik **Entri Baru** untuk setiap orang. JeffWrite kemudian menawarkan untuk menyimpan daftar sebagai berkas .csv.
- **Gunakan Daftar yang Ada**: pilih berkas yang sudah berupa daftar. Dapat berupa berkas .csv atau berkas teks dengan tab (satu baris untuk setiap orang, dan baris pertama berisi nama kolom), lembar kerja (.xlsx, .xls, atau .ods, dari JeffCrunch atau program lain), atau berkas kontak (.vcf).
- **Pilih dari Kontak**: pilih berkas kontak (.vcf). Sebagian besar buku alamat dan ponsel dapat menyimpan kontaknya sebagai berkas tersebut.

Kemudian klik **Surat Massal > Edit Daftar Penerima** untuk memeriksa daftar:

- Hapus centang pada baris untuk mengeluarkan orang tersebut.
- Klik judul kolom untuk mengurutkan.
- Ketik di **Saring penerima** untuk menampilkan hanya baris yang memuat kata-kata Anda.
- **Temukan Duplikat** menghapus centang pada data yang berulang.
- **Validasi Alamat** memberi warna pada baris yang tidak memiliki alamat atau kode pos yang tampak keliru.

## Masukkan rincian ke dalam surat

**Bidang gabungan** adalah tempat penampung. Bidang ini menampilkan nama kolom dalam tanda kurung sudut ganda, misalnya «Nama Depan». Pada setiap salinan, bidang tersebut digantikan dengan rincian orang tersebut.

Klik tempat rincian akan dimasukkan, lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Sisipkan Bidang Gabungan** menampilkan kolom daftar Anda. Pilih salah satu.
  - **Kode Batang dari Bidang**, dalam daftar yang sama, memasukkan kode batang yang nilainya berasal dari sebuah kolom, misalnya nomor suku cadang atau ISBN 13 digit sebuah buku. Pilih kolomnya, lalu pilih jenisnya dan lihat jendela **Sisipkan Kode Batang** (apa pun yang Anda ketik di sana hanya contoh). Jendela ini hanya menawarkan apa yang dapat dimuat sebuah bidang: jenis, ukuran, tinggi batang, digit, dan tingkat QR. Jendela ini tidak menyertakan harga buku, add-on, karakter cek, dan latar belakang bening, yang tidak dapat dinyatakan oleh sebuah bidang; ISBN buku adalah EAN-13. Di dokumen, bidang ditampilkan sebagai kode batang, berupa contoh jenisnya sampai Anda melakukan penggabungan. Untuk mengubah jenis atau tampilannya, klik dua kali bidang itu, atau klik kanan dan pilih **Edit Kode Batang**. Saat penggabungan, salinan setiap orang mendapat kode batang sesuai nilainya sendiri, digambar sebagai gambar dalam baris, sehingga ikut tercetak dan tersimpan ke PDF seperti gambar lainnya. Nilai hanya berupa data: tanda kutip atau garis miring terbalik di dalamnya tidak mengubah apa pun. Jika nilai kosong, tidak ada kode batang. Jika sebuah kode tidak dapat memuat nilai (huruf dalam EAN-13, atau lebih dari 500 karakter dalam Code 128), salinan tersebut menampilkan pesan singkat dalam tanda kurung siku sebagai gantinya, dan salinan lain tidak terpengaruh. Satu penggabungan membuat paling banyak 2.000 kode batang, termasuk setiap nilai yang tidak dapat dimuat; gabungkan lebih sedikit orang jika muncul pesan bahwa terlalu banyak. Dalam `.docx` yang disimpan, bidang ini berupa bidang DISPLAYBARCODE dengan kolom di dalamnya, sehingga tetap tersimpan saat berkas disimpan dan dibuka kembali. Bidang dengan nilai yang diketik langsung, atau jenis yang tidak digambar JeffWrite (UPC-E, misalnya), dibiarkan apa adanya. Penggabungan email memasukkan nilai sebagai teks.
- **Blok Alamat** memasukkan seluruh alamat surat: nama, perusahaan, jalan, dan kota. Pilih baris yang Anda inginkan. Baris yang kosong akan dihilangkan. Anda juga dapat menyebutkan negara yang akan dihilangkan, misalnya negara Anda sendiri. **Pratinjau** menampilkan alamat orang pertama.
- **Baris Salam** memasukkan baris seperti "Yth. Dr. Fenwick,". Anda yang memilih kata-katanya. Pilihan pertama, "Yth. Dr. Fenwick,", menggunakan gelar orang tersebut. Jika seseorang tidak memiliki gelar, salam menggunakan nama depan dan nama belakang sebagai gantinya, seperti "Yth. Nora Pell,". Jika seseorang tidak memiliki nama, salam akan berbunyi lain, misalnya "Yth. Bapak/Ibu,", yang dapat Anda ubah.
- **Cocokkan Bidang** memberi tahu blok alamat dan baris salam kolom mana yang berisi setiap bagian, jika nama kolom Anda tidak biasa.

## Aturan

**Aturan** membuat sebuah salinan berbeda untuk sebagian orang. Klik **Aturan** dan pilih salah satu:

- **Jika…Maka…Lainnya**: menampilkan satu teks ketika sebuah kolom memiliki nilai, dan teks lain ketika tidak. Misalnya, jika Provinsi adalah "CA", tulis "Selamat datang kembali".
- **Lewati Rekaman Jika**: mengeluarkan seseorang dari penggabungan ketika sebuah kolom memenuhi uji yang Anda atur, misalnya ketika nama belakang kosong.
- **Rekaman Berikutnya** dan **Rekaman Berikutnya Jika**: memakai rincian orang berikutnya di tempat yang sama. Label menggunakan ini untuk menempatkan orang yang berbeda di setiap label.
- **Nomor Rekaman Gabungan** memasukkan nomor baris orang tersebut dalam daftar. **Nomor Urutan Gabungan** menghitung jumlah salinan yang sudah dibuat.
- **Tanya** dan **Isian**: menanyakan sesuatu kepada Anda ketika penggabungan selesai. Tanya menyimpan jawaban di bawah sebuah nama. Isian memasukkan jawaban ke dalam teks.
- **Atur Penanda**: memberi nama sebuah nilai. Rujukan silang ke nama tersebut menampilkan nilainya.

## Pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil**. Sebuah panel terbuka yang menampilkan halaman untuk satu orang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk berpindah antarorang, atau ketik sebuah nomor. **Cari Penerima** melompat ke orang berikutnya yang memiliki kata Anda dalam salah satu rincian.

**Sorot Bidang Gabungan** memberi warna abu-abu pada bidang gabungan di halaman Anda agar Anda dapat melihat letaknya. Bidang lain, seperti nomor halaman, tidak diberi warna. Penyorotan tampak di layar dan tidak pernah tercetak.

Dokumen Anda sendiri tidak berubah. **Periksa Kesalahan** memberi tahu Anda jika daftar tidak memiliki kolom untuk sebuah bidang, atau jika tidak ada orang yang disertakan.

## Selesaikan penggabungan

Klik **Surat Massal > Selesai & Gabungkan**:

- **Edit Dokumen Individual** membuat dokumen baru berisi salinan setiap orang. Anda dapat mengubahnya sebelum mencetak.
- **Cetak Dokumen** membuat dokumen tersebut dan membuka halaman **Cetak**.
- **Simpan sebagai PDF** menyimpan setiap salinan dalam satu berkas PDF.
- **Kirim Pesan Email** membuat satu berkas pesan .eml untuk setiap orang, di folder yang Anda pilih. Pilih kolom yang berisi alamat email, lalu ketik subjek. JeffWrite tidak mengirim pesan tersebut. Buka berkas-berkas itu di program email Anda.

Orang yang Anda hapus centangnya dalam daftar tidak akan disertakan.

Berkas pesan hanya dibuat untuk orang yang alamatnya berupa satu alamat email. Jika sebuah alamat memuat dua alamat (dengan koma atau titik koma di antaranya), memiliki nama di dalamnya, atau tidak memiliki domain, orang tersebut tidak mendapat pesan, dan JeffWrite memberi tahu Anda berapa banyak yang terlewat. Subjek yang panjang disimpan dalam beberapa bagian yang akan disatukan kembali oleh program email.

Penggabungan menampilkan jendela kemajuan, dan **Batal** menghentikannya. Jika sebuah aturan mengajukan pertanyaan (Tanya atau Isian), Anda ditanya setiap pertanyaan berbeda sekali saja, hingga 20 pertanyaan. Setelah itu, sisanya memakai jawaban bawaan. Jika Anda membatalkan sebuah pertanyaan, seluruh penggabungan berhenti. Penggabungan akan berhenti dan memberi tahu Anda jika akan menghasilkan lebih dari 10.000 halaman, sehingga dokumen yang sangat panjang dengan daftar yang sangat panjang tidak memakan waktu berjam-jam. Gabungkan sebagian daftar sekaligus.

## Amplop

Klik **Surat Massal > Amplop**. Ketik alamat tujuan dan alamat pengirim Anda, lalu pilih ukuran amplop dari daftar. Ukuran diurutkan menurut lebar dan tinggi. Klik **Tambahkan ke Dokumen** untuk meletakkan amplop di halamannya sendiri di awal dokumen Anda, atau **Cetak** untuk mencetaknya. Halaman selebar sisi panjang amplop, yang merupakan cara paling umum printer menerima amplop.

Untuk membuat amplop bagi penggabungan, klik **Mulai Cetak Surat Massal > Amplop**. Setiap amplop memiliki blok alamat.

## Label

Klik **Surat Massal > Label**. Ketik alamat, lalu pilih lembar dari daftar. Lembar dinamai menurut jumlah label di dalamnya dan ukuran setiap label. Jika lembar Anda tidak ada dalam daftar, pilih **Lembar sendiri** dan ketik ukurannya: berapa label ke samping dan ke bawah, seberapa besar setiap label, ruang di atas dan di samping label pertama, dan seberapa jauh jarak antarlabel.

- **Satu halaman penuh label yang sama** menempatkan alamat yang sama pada setiap label di lembar.
- **Label tunggal** menempatkan alamat pada satu label, sehingga Anda dapat memanfaatkan lembar yang sudah sebagian terpakai. Sebutkan baris dan kolomnya.

Klik **Dokumen Baru** untuk membuat dokumen yang dapat Anda lihat dan ubah, atau **Cetak** untuk mencetaknya.

Untuk membuat label bagi penggabungan, klik **Mulai Cetak Surat Massal > Label**. Label pertama memiliki blok alamat, dan setiap label lainnya dimulai dengan aturan **Rekaman Berikutnya**. Jika Anda mengubah label pertama, klik **Perbarui Label** untuk menyalin perubahan ke semua label lainnya.

## Isi berkas

Jenis penggabungan, nama dan lokasi daftar, subjek, dan kolom email disimpan dalam berkas .docx Anda. Orang mana yang Anda hapus centangnya, dan pilihan Cocokkan Bidang Anda, tidak disimpan. Keduanya akan ditanyakan kembali setelah berkas dibuka. Daftar dibaca ulang dari berkasnya setiap kali. Jika berkas tidak berada di tempat semula, JeffWrite mencari di sebelah dokumen Anda, lalu meminta Anda memilihnya.

Sebuah daftar dapat memiliki hingga 1.000 kolom dan 4.000.000 sel secara keseluruhan, dan penggabungan menggunakan 100.000 orang pertama. Jika berkas memiliki lebih banyak, JeffWrite menggunakan yang muat dan memberi tahu Anda. Baris yang tidak berisi apa pun dihilangkan dari daftar.

## Terkait

- [Gunakan bidang](fields)
- [Cetak](print)
- [Buat dan ubah tabel](tables)
