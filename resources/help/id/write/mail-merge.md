# Surat massal, amplop, dan label
<!-- keywords: surat massal, gabung, cetak surat massal, penerima, daftar alamat, surat formulir, label, amplop, bidang gabungan, blok alamat, baris salam, aturan, jika maka lainnya, lewati rekaman, rekaman berikutnya, pratinjau hasil, selesai dan gabungkan, direktori, pesan email, csv, xlsx, ods, vcard -->

**Surat massal** membuat banyak salinan dari satu surat, dengan rincian orang yang berbeda di setiap salinan. Anda menulis surat sekali saja. Anda menyimpan rincian orang-orang tersebut dalam sebuah **daftar**. JeffWrite memasukkan nama dan alamat orang pertama ke salinan pertama, nama dan alamat orang kedua ke salinan kedua, dan seterusnya.

Alatnya terdapat pada tab **Surat Massal**. Untuk langkah demi langkah yang terpandu, klik **Surat Massal > Mulai Surat Massal > Panduan Cetak Surat Massal Langkah demi Langkah**.

## Pilih yang akan dibuat

Klik **Surat Massal > Mulai Surat Massal**, lalu pilih salah satu:

- **Surat**: satu salinan halaman untuk setiap orang. Setiap salinan dimulai di halaman baru.
- **Pesan Email**: satu pesan untuk setiap orang. Lihat "Kirim pesan email" di bawah.
- **Amplop** dan **Label**: halaman yang dibuat khusus untuk keduanya. Lihat bagian di bawah.
- **Direktori**: satu daftar berisi semua orang, tanpa pemisah halaman di antara setiap orang. Gunakan untuk daftar telepon atau katalog.
- **Hapus Jenis Gabungan** membuat dokumen kembali menjadi dokumen biasa.

JeffWrite menyimpan jenis gabungan dan nama daftar di dalam berkas saat Anda menyimpannya. Ketika Anda membuka berkas itu lagi, gabungan tersebut masih ada.

## Pilih penerima

**Penerima** adalah orang yang menerima salinan. Klik **Surat Massal > Pilih Penerima**:

- **Ketik Daftar Baru**: ketik orang-orang di jendela yang terbuka. Klik **Entri Baru** untuk setiap orang. JeffWrite kemudian menawarkan untuk menyimpan daftar sebagai berkas .csv.
- **Gunakan Daftar yang Ada**: pilih berkas yang sudah berupa daftar. Bisa berupa berkas .csv atau berkas teks dengan tab (satu baris untuk setiap orang, dan baris pertama berisi nama kolom), lembar kerja (.xlsx, .xls, atau .ods, dari JeffCrunch atau program lain), atau berkas kontak (.vcf).
- **Pilih dari Kontak**: pilih berkas kontak (.vcf). Sebagian besar buku alamat dan ponsel dapat menyimpan kontak mereka sebagai satu berkas.

Kemudian klik **Surat Massal > Edit Daftar Penerima** untuk memeriksa daftar:

- Hapus centang pada baris untuk mengeluarkan orang tersebut.
- Klik judul kolom untuk mengurutkan.
- Ketik di **Saring penerima** untuk hanya menampilkan baris yang memuat kata Anda.
- **Temukan Duplikat** menghapus centang pada baris yang berulang.
- **Validasi Alamat** memberi warna pada baris yang tidak memiliki alamat atau memiliki kode pos yang tampak keliru.

## Masukkan rincian ke dalam surat

**Bidang gabungan** adalah tempat penampung. Bidang ini menampilkan nama kolom dalam tanda kurung sudut ganda, misalnya «Nama Depan». Di setiap salinan, bidang tersebut diganti dengan rincian milik orang yang bersangkutan.

Klik tempat rincian akan dimasukkan, lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Sisipkan Bidang Gabungan** menampilkan kolom dari daftar Anda. Pilih salah satu.
  - **Kode Batang dari Bidang**, dalam daftar yang sama, memasukkan kode batang yang nilainya berasal dari sebuah kolom, misalnya nomor suku cadang atau ISBN 13 digit sebuah buku. Pilih kolom tersebut, lalu pilih jenisnya dan lihat di jendela **Sisipkan Kode Batang** (apa pun yang Anda ketik di sana hanya contoh). Jendela ini hanya menawarkan hal-hal yang dapat dimuat sebuah bidang: jenis, ukuran, tinggi batang, digit, dan tingkat QR. Jendela ini tidak menyediakan harga buku, add-on, karakter cek, dan latar belakang bersih, yang tidak dapat dinyatakan oleh sebuah bidang; ISBN buku adalah EAN-13. Di dalam dokumen, bidang ditampilkan sebagai kode batang, berupa contoh dari jenisnya sampai Anda menggabungkan. Untuk mengubah jenis atau tampilannya, klik dua kali bidang tersebut, atau klik kanan dan pilih **Edit Kode Batang**. Saat digabungkan, salinan setiap orang mendapat kode batang dengan nilainya sendiri, digambar sebagai gambar dalam baris, sehingga ikut tercetak dan tersimpan ke PDF seperti gambar lainnya. Nilai hanya berupa data: tanda kutip atau garis miring terbalik di dalamnya tidak mengubah apa pun. Jika nilai kosong, tidak ada kode batang. Jika kode tidak dapat memuat nilai (huruf pada EAN-13, atau lebih dari 500 karakter pada Code 128), salinan tersebut menampilkan pesan singkat dalam tanda kurung siku di tempatnya, dan salinan lain tidak terpengaruh. Satu penggabungan membuat paling banyak 2.000 kode batang, termasuk setiap nilai yang tidak dapat dimuat; gabungkan lebih sedikit orang jika program menyatakan terlalu banyak. Dalam `.docx` yang disimpan, bidang ini berupa bidang DISPLAYBARCODE dengan nama kolom di dalamnya, sehingga tetap tersimpan saat Anda menyimpan dan membuka berkas lagi. Bidang yang berisi nilai ketikan, atau berjenis yang tidak digambar JeffWrite (misalnya UPC-E), dibiarkan apa adanya. Gabungan email memasukkan nilai sebagai teks.
- **Blok Alamat** memasukkan alamat surat lengkap: nama, perusahaan, jalan, dan kota. Pilih baris yang Anda inginkan. Baris yang kosong dihilangkan. Anda juga dapat menyebutkan negara yang akan dihilangkan, misalnya negara Anda sendiri. **Pratinjau** menampilkan alamat orang pertama.
- **Baris Salam** memasukkan baris seperti "Yth. Dr. Fenwick,". Anda memilih kata-katanya. Pilihan pertama, "Yth. Dr. Fenwick,", menggunakan gelar orang tersebut. Jika seseorang tidak memiliki gelar, yang dipakai adalah nama depan dan nama belakang, seperti "Yth. Nora Pell,". Jika seseorang tidak memiliki nama, salam menggunakan kalimat lain, misalnya "Yth. Bapak/Ibu,", yang dapat Anda ubah.
- **Cocokkan Bidang** memberi tahu blok alamat dan baris salam kolom mana yang berisi setiap bagian, jika nama kolom Anda tidak biasa.

## Aturan

**Aturan** membuat salinan berbeda untuk sebagian orang. Klik **Aturan** dan pilih salah satu:

- **Jika…Maka…Lainnya**: tampilkan satu teks ketika sebuah kolom berisi nilai, dan teks lain ketika tidak. Misalnya, jika Provinsi adalah "CA", tulis "Selamat datang di rumah".
- **Lewati Rekaman Jika**: keluarkan seseorang ketika sebuah kolom berisi nilai, misalnya ketika nama belakang kosong.
- **Rekaman Berikutnya** dan **Rekaman Berikutnya Jika**: gunakan rincian orang berikutnya di tempat yang sama. Label memakai ini untuk memasukkan orang yang berbeda di setiap label.
- **Nomor Rekaman Gabungan** memasukkan nomor baris orang tersebut dalam daftar. **Nomor Urutan Gabungan** menghitung jumlah salinan yang sudah dibuat.
- **Tanya** dan **Isian**: menanyakan sesuatu kepada Anda saat Anda menyelesaikan penggabungan. Tanya menyimpan jawabannya di bawah sebuah nama. Isian memasukkan jawaban ke dalam teks.
- **Atur Penanda**: memberi nama sebuah nilai. Referensi silang ke nama tersebut menampilkan nilainya.

## Pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil**. Sebuah panel terbuka dan menampilkan halaman untuk satu orang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk berpindah di antara orang-orang tersebut, atau ketik nomor. **Temukan Penerima** melompat ke orang berikutnya yang memiliki kata Anda dalam salah satu rinciannya.

**Sorot Bidang Gabungan** memberi warna abu-abu pada bidang gabungan di halaman Anda agar Anda dapat melihat letaknya. Bidang lain, seperti nomor halaman, tidak diberi warna. Penyorotan hanya tampak di layar dan tidak pernah tercetak.

Dokumen Anda sendiri tidak berubah. **Periksa Kesalahan** memberi tahu Anda jika daftar tidak memiliki kolom untuk sebuah bidang, atau jika tidak ada orang yang disertakan.

## Selesaikan penggabungan

Klik **Surat Massal > Selesai & Gabungkan**:

- **Edit Dokumen Individual** membuat dokumen baru berisi salinan setiap orang. Anda dapat mengubahnya sebelum mencetak.
- **Cetak Dokumen** membuat dokumen tersebut dan membuka halaman **Cetak**.
- **Simpan sebagai PDF** menyimpan semua salinan dalam satu berkas PDF.
- **Kirim Pesan Email** membuat satu berkas pesan .eml untuk setiap orang, di dalam folder yang Anda pilih. Pilih kolom yang berisi alamat email, lalu ketik subjek. JeffWrite tidak mengirim pesan tersebut. Buka berkas-berkas itu di program email Anda.

Orang yang dihapus centangnya dalam daftar tidak disertakan.

Berkas pesan hanya dibuat untuk orang yang alamatnya berisi satu alamat email. Jika sebuah alamat memuat dua (dipisahkan koma atau titik koma), memiliki nama di dalamnya, atau tidak memiliki domain, orang tersebut tidak mendapat pesan, dan JeffWrite memberi tahu jumlahnya. Subjek yang panjang disimpan dalam beberapa bagian yang akan disatukan kembali oleh program email.

Penggabungan menampilkan jendela kemajuan, dan **Batal** menghentikannya. Jika sebuah aturan mengajukan pertanyaan (Tanya atau Isian), Anda ditanya setiap pertanyaan berbeda sekali, hingga 20 pertanyaan. Setelah itu, sisanya menggunakan jawaban standarnya. Jika Anda membatalkan sebuah pertanyaan, seluruh penggabungan berhenti. Penggabungan akan berhenti dan memberi tahu Anda jika hasilnya lebih dari 10.000 halaman, sehingga dokumen yang sangat panjang dengan daftar yang sangat panjang tidak memakan waktu berjam-jam. Gabungkan sebagian daftar setiap kali.

## Amplop

Klik **Surat Massal > Amplop**. Ketik alamat tujuan dan alamat pengirim Anda, lalu pilih ukuran amplop dari daftar. Ukuran diurutkan menurut lebar dan tingginya. Klik **Tambahkan ke Dokumen** untuk menempatkan amplop di halaman tersendiri di awal dokumen Anda, atau **Cetak** untuk mencetaknya. Halaman selebar sisi panjang amplop, yang merupakan cara paling umum printer menerima amplop.

Untuk membuat amplop bagi sebuah penggabungan, klik **Mulai Surat Massal > Amplop**. Setiap amplop memiliki blok alamat.

## Label

Klik **Surat Massal > Label**. Ketik alamat dan pilih lembar dari daftar. Lembar diberi nama menurut jumlah label di dalamnya dan ukuran setiap label. Jika lembar Anda tidak ada dalam daftar, pilih **Lembar sendiri** dan ketik ukurannya: berapa label ke samping dan ke bawah, seberapa besar sebuah label, jarak di atas dan di samping label pertama, serta jarak antarlabel.

- **Satu halaman penuh label yang sama** memasukkan alamat yang sama pada setiap label di lembar.
- **Satu label** memasukkan alamat pada satu label saja, sehingga Anda dapat memakai lembar yang sudah sebagian terpakai. Tentukan baris dan kolom yang dimaksud.

Klik **Dokumen Baru** untuk membuat dokumen yang dapat Anda lihat dan ubah, atau **Cetak** untuk mencetaknya.

Untuk membuat label bagi sebuah penggabungan, klik **Mulai Surat Massal > Label**. Label pertama memiliki blok alamat, dan setiap label lain dimulai dengan aturan **Rekaman Berikutnya**. Jika Anda mengubah label pertama, klik **Perbarui Label** untuk menyalin perubahan ke semua label lainnya.

## Isi berkas

Jenis gabungan, nama dan lokasi daftar, subjek, dan kolom email disimpan dalam berkas .docx Anda. Orang mana yang Anda hapus centangnya, serta pilihan Cocokkan Bidang Anda, tidak disimpan. Pilihan tersebut akan diminta lagi setelah Anda membuka berkas. Daftar dibaca ulang dari berkasnya setiap kali. Jika berkas tidak berada di tempat semula, JeffWrite mencarinya di sebelah dokumen Anda, lalu meminta Anda memilihnya.

Sebuah daftar dapat memiliki hingga 1.000 kolom dan 4.000.000 sel secara keseluruhan, dan penggabungan menggunakan 100.000 orang pertama. Jika berkas memuat lebih dari itu, JeffWrite memakai bagian yang muat dan memberi tahu Anda. Baris yang kosong seluruhnya dihilangkan dari daftar.

## Terkait

- [Gunakan bidang](fields)
- [Cetak](print)
- [Buat dan ubah tabel](tables)
