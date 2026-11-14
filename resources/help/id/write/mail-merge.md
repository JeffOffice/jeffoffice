# Surat massal, amplop, dan label
<!-- keywords: surat massal, cetak surat massal, gabung, penerima, daftar alamat, surat formulir, label, amplop, bidang gabungan, blok alamat, baris salam, aturan, jika maka lainnya, lewati rekaman, rekaman berikutnya, pratinjau hasil, selesai dan gabungkan, direktori, pesan email, csv, xlsx, ods, vcard, kode batang, mailing, surat massal -->

Sebuah **surat massal** membuat banyak salinan dari satu surat, masing-masing dengan rincian orang yang berbeda di dalamnya. Anda menulis surat itu sekali. Anda menyimpan rincian orang-orang tersebut dalam sebuah **daftar**. JeffWrite memasukkan nama dan alamat orang pertama ke salinan pertama, nama dan alamat orang kedua ke salinan kedua, dan seterusnya.

Alat-alatnya terdapat pada tab **Surat Massal**. Untuk langkah demi langkah yang terpandu, klik **Surat Massal > Mulai Surat Massal > Panduan Cetak Surat Massal Langkah demi Langkah**.

## Pilih yang akan dibuat

Klik **Surat Massal > Mulai Surat Massal**, lalu pilih salah satu:

- **Surat**: satu salinan halaman untuk setiap orang. Setiap salinan dimulai di halaman baru.
- **Pesan Email**: satu pesan untuk setiap orang. Lihat "Mengirim pesan email" di bawah.
- **Amplop** dan **Label**: halaman yang dibuat untuk keperluan tersebut. Lihat bagian di bawah.
- **Direktori**: satu daftar berisi semua orang, tanpa pemisah halaman di antara setiap orang. Gunakan untuk daftar telepon atau katalog.
- **Hapus Jenis Gabungan** membuat dokumen kembali menjadi dokumen biasa.

JeffWrite menyimpan jenis gabungan dan nama daftar di dalam berkas saat Anda menyimpannya. Ketika Anda membuka berkas itu lagi, gabungan tersebut masih ada.

## Pilih penerima

**Penerima** adalah orang yang menerima salinan. Klik **Surat Massal > Pilih Penerima**:

- **Ketik Daftar Baru**: ketik orang-orang di jendela yang terbuka. Klik **Entri Baru** untuk setiap orang. JeffWrite kemudian menawarkan untuk menyimpan daftar sebagai berkas .csv.
- **Gunakan Daftar yang Ada**: pilih berkas yang sudah berupa daftar. Bisa berupa berkas .csv atau berkas teks dengan tab (satu baris untuk setiap orang, dan baris pertama berisi nama kolom), lembar kerja (.xlsx, .xls, atau .ods, dari JeffCrunch atau program lain), atau berkas kontak (.vcf).
- **Pilih dari Kontak**: pilih berkas kontak (.vcf). Sebagian besar buku alamat dan ponsel dapat menyimpan kontak mereka sebagai satu berkas tersebut.

Kemudian klik **Surat Massal > Edit Daftar Penerima** untuk memeriksa daftar:

- Hapus centang pada baris untuk mengeluarkan orang tersebut.
- Klik judul kolom untuk mengurutkan.
- Ketik di **Saring penerima** untuk hanya menampilkan baris yang memuat kata Anda.
- **Temukan Duplikat** menghapus centang pada data yang berulang.
- **Validasi Alamat** memberi warna pada baris yang tidak memiliki alamat atau memiliki kode pos yang tampak keliru.

## Masukkan rincian ke dalam surat

**Bidang gabungan** adalah tempat penampung. Bidang ini menampilkan nama kolom di antara tanda kurung sudut ganda, seperti «Nama Depan». Pada setiap salinan, bidang ini digantikan dengan rincian milik orang tersebut.

Klik tempat rincian akan dimasukkan, lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Sisipkan Bidang Gabungan** menampilkan kolom dari daftar Anda. Pilih salah satu.
  - **Kode Batang dari Bidang**, dalam daftar yang sama, memasukkan kode batang yang nilainya berasal dari sebuah kolom, misalnya nomor suku cadang atau ISBN 13 digit sebuah buku. Pilih kolomnya, lalu pilih jenisnya dan lihat di jendela **Sisipkan Kode Batang** (apa pun yang Anda ketik di sana hanyalah contoh). Jendela ini hanya menawarkan yang dapat dimuat oleh sebuah bidang: jenis, ukuran, tinggi batang, digit, dan tingkat QR. Jendela ini tidak menyediakan harga buku, add-on, karakter cek, dan latar belakang bersih, yang tidak dapat dinyatakan oleh sebuah bidang; ISBN buku adalah EAN-13. Di dalam dokumen, bidang digambar sebagai kode batang, berupa contoh dari jenis tersebut hingga Anda menggabungkan; untuk mengubah jenis atau tampilannya, klik dua kali bidang itu, atau klik kanan lalu pilih **Edit Kode Batang**. Saat digabungkan, salinan setiap orang mendapat kode batang dengan nilainya sendiri, digambar sebagai gambar dalam baris, sehingga ikut tercetak dan tersimpan ke PDF seperti gambar lainnya. Nilai hanyalah data: tanda kutip atau garis miring terbalik di dalamnya tidak mengubah apa pun. Jika nilai kosong, tidak ada kode batang. Jika kode tidak dapat memuat nilai (huruf dalam EAN-13, atau lebih dari 500 karakter dalam Code 128), salinan tersebut menampilkan pesan singkat di dalam tanda kurung siku sebagai gantinya, dan salinan lain tidak terpengaruh. Satu penggabungan membuat paling banyak 2.000 kode batang, termasuk setiap nilai yang tidak dapat dimuat; gabungkan lebih sedikit orang jika ada pesan bahwa terlalu banyak. Dalam `.docx` yang disimpan, bidang berupa bidang DISPLAYBARCODE dengan kolom di dalamnya, sehingga tetap tersimpan saat berkas disimpan dan dibuka kembali. Bidang dengan nilai yang diketik langsung, atau berjenis yang tidak digambar JeffWrite (UPC-E, misalnya), dibiarkan apa adanya. Penggabungan email memasukkan nilainya sebagai teks.
- **Blok Alamat** memasukkan satu alamat surat lengkap: nama, perusahaan, jalan, dan kota. Pilih baris yang Anda inginkan. Baris yang tidak berisi apa pun dibiarkan keluar. Anda juga dapat menyebutkan negara yang akan dihilangkan, misalnya negara Anda sendiri. **Pratinjau** menampilkan alamat orang pertama.
- **Baris Salam** memasukkan baris seperti "Yth. Dr. Fenwick,". Anda memilih katanya. Pilihan pertama, "Yth. Dr. Fenwick,", menggunakan gelar orang tersebut. Jika seseorang tidak memiliki gelar, yang digunakan adalah nama depan dan nama belakang, seperti "Yth. Nora Pell,". Jika seseorang tidak memiliki nama, salam akan berbunyi lain, misalnya "Yth. Bapak/Ibu,", yang dapat Anda ubah.
- **Cocokkan Bidang** memberi tahu blok alamat dan baris salam kolom mana yang memuat setiap bagian, jika nama kolom Anda tidak lazim.

## Aturan

**Aturan** membuat salinan berbeda untuk sebagian orang. Klik **Aturan** lalu pilih salah satu:

- **Jika…Maka…Lainnya…**: tampilkan satu teks ketika sebuah kolom berisi nilai, dan teks lain ketika tidak. Misalnya, jika Provinsi adalah "CA", tulis "Selamat datang di rumah".
- **Lewati Rekaman Jika**: keluarkan seseorang ketika sebuah kolom berisi nilai, misalnya ketika nama belakang kosong.
- **Rekaman Berikutnya** dan **Rekaman Berikutnya Jika**: gunakan rincian orang berikutnya di tempat yang sama. Label menggunakan ini untuk menempatkan orang yang berbeda di setiap label.
- **Nomor Rekaman Gabungan** memasukkan nomor baris orang tersebut dalam daftar. **Nomor Urutan Gabungan** menghitung salinan yang sudah dibuat.
- **Tanya** dan **Isian**: menanyakan sesuatu kepada Anda ketika Anda menyelesaikan penggabungan. Tanya menyimpan jawaban di bawah sebuah nama. Isian memasukkan jawaban ke dalam teks.
- **Atur Penanda**: memberi nama sebuah nilai. Referensi silang ke nama itu menampilkan nilainya.

## Pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil**. Sebuah panel terbuka dan menampilkan halaman untuk satu orang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk berpindah di antara orang-orang tersebut, atau ketik sebuah nomor. **Temukan Penerima** melompat ke orang berikutnya yang memiliki kata Anda dalam salah satu rincian.

**Sorot Bidang Gabungan** memberi warna abu-abu pada bidang gabungan di halaman Anda agar Anda dapat melihat letaknya. Bidang lain, seperti nomor halaman, tidak diberi warna. Sorotan tampak di layar dan tidak pernah tercetak.

Dokumen Anda sendiri tidak berubah. **Periksa Kesalahan** memberi tahu Anda ketika daftar tidak memiliki kolom untuk sebuah bidang, atau ketika tidak ada orang yang disertakan.

## Selesaikan penggabungan

Klik **Surat Massal > Selesai & Gabungkan**:

- **Edit Dokumen Individual** membuat dokumen baru berisi salinan setiap orang. Anda dapat mengubahnya sebelum mencetak.
- **Cetak Dokumen** membuat dokumen tersebut dan membuka halaman **Cetak**.
- **Simpan sebagai PDF** menyimpan setiap salinan dalam satu berkas PDF.
- **Kirim Pesan Email** membuat satu berkas pesan .eml untuk setiap orang, di folder yang Anda pilih. Pilih kolom yang memuat alamat email, dan ketik subjek. JeffWrite tidak mengirimkannya. Buka berkas-berkas itu di program email Anda.

Orang yang Anda hapus centangnya dalam daftar tidak disertakan.

Berkas pesan hanya dibuat untuk orang yang alamatnya berupa satu alamat email. Jika sebuah alamat memuat dua (dengan koma atau titik koma di antaranya), memiliki nama di dalamnya, atau tidak memiliki domain, orang tersebut tidak mendapat pesan, dan JeffWrite memberi tahu jumlahnya. Subjek yang panjang disimpan dalam beberapa bagian yang disatukan kembali oleh program email.

Penggabungan menampilkan jendela kemajuan, dan **Batal** menghentikannya. Jika sebuah aturan mengajukan pertanyaan (Tanya atau Isian), Anda diminta menjawab setiap pertanyaan berbeda sekali, hingga 20 pertanyaan. Setelah itu, sisanya menggunakan jawaban standarnya. Jika Anda membatalkan sebuah pertanyaan, seluruh penggabungan berhenti. Penggabungan berhenti dan memberi tahu Anda jika akan menghasilkan lebih dari 10.000 halaman, sehingga dokumen yang sangat panjang dengan daftar yang sangat panjang tidak memakan waktu berjam-jam. Gabungkan sebagian daftar sekaligus sebagai gantinya.

## Amplop

Klik **Surat Massal > Amplop**. Ketik alamat tujuan dan alamat pengirim Anda, lalu pilih ukuran amplop dari daftar. Ukuran diberi nama menurut lebar dan tinggi. Klik **Tambahkan ke Dokumen** untuk meletakkan amplop di halaman tersendiri di awal dokumen Anda, atau **Cetak** untuk langsung mencetaknya. Halaman selebar sisi panjang amplop, yang merupakan cara kebanyakan printer menerima amplop.

Untuk membuat amplop untuk sebuah penggabungan, klik **Mulai Surat Massal > Amplop**. Setiap amplop memiliki blok alamat.

## Label

Klik **Surat Massal > Label**. Ketik alamat dan pilih lembar dari daftar. Sebuah lembar dinamai menurut jumlah label di dalamnya dan ukuran setiap label. Jika lembar Anda tidak ada dalam daftar, pilih **Lembar sendiri** dan ketik ukurannya: berapa label ke samping dan ke bawah, seberapa besar satu label, jarak di atas dan di samping label pertama, serta seberapa jauh jarak antarlabel.

- **Satu halaman penuh dengan label yang sama** memasukkan alamat yang sama ke setiap label pada lembar.
- **Satu label** memasukkan alamat pada satu label, sehingga Anda dapat memakai lembar yang sudah terpakai sebagian. Sebutkan baris dan kolomnya.

Klik **Dokumen Baru** untuk membuat dokumen yang dapat Anda lihat dan ubah, atau **Cetak** untuk mencetaknya.

Untuk membuat label bagi sebuah penggabungan, klik **Mulai Surat Massal > Label**. Label pertama memiliki blok alamat, dan setiap label lainnya dimulai dengan aturan **Rekaman Berikutnya**. Jika Anda mengubah label pertama, klik **Perbarui Label** untuk menyalin perubahan itu ke semua label lainnya.

## Isi berkas

Jenis gabungan, nama dan lokasi daftar, subjek, dan kolom email disimpan dalam berkas .docx Anda. Orang mana yang Anda hapus centangnya, dan pilihan Cocokkan Bidang Anda, tidak disimpan. Keduanya diminta lagi setelah Anda membuka berkas. Daftar dibaca ulang dari berkasnya setiap kali. Jika berkas tidak berada di tempat semula, JeffWrite mencari di sebelah dokumen Anda, lalu meminta Anda memilihnya.

Sebuah daftar dapat memiliki hingga 1.000 kolom dan 4.000.000 sel secara keseluruhan, dan penggabungan menggunakan 100.000 orang pertama. Jika berkas memuat lebih banyak, JeffWrite menggunakan yang muat dan memberi tahu Anda. Baris yang kosong dihilangkan dari daftar.

## Terkait

- [Gunakan bidang](fields)
- [Cetak](print)
- [Buat dan ubah tabel](tables)
