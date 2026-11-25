# Surat massal, amplop, dan label
<!-- keywords: surat massal, gabung surat, cetak surat massal, mailings, surat massal, penerima, daftar alamat, surat formulir, label, amplop, bidang gabungan, blok alamat, baris salam, aturan, jika maka lainnya, lewati rekaman, rekaman berikutnya, pratinjau hasil, selesaikan dan gabungkan, direktori, pesan email, csv, xlsx, ods, vcard, gabung email, daftar penerima, kode batang, cetak massal -->

**Surat massal** membuat banyak salinan satu surat, masing-masing dengan rincian orang yang berbeda. Anda menulis surat sekali saja. Anda menyimpan rincian orang-orang tersebut dalam sebuah **daftar**. JeffWrite memasukkan nama dan alamat orang pertama ke salinan pertama, nama dan alamat orang kedua ke salinan kedua, dan seterusnya.

Alatnya ada di tab **Surat Massal**. Untuk panduan langkah demi langkah, klik **Surat Massal > Mulai Surat Massal > Pemandu Cetak Surat Massal Langkah demi Langkah**.

## Pilih jenis yang akan dibuat

Klik **Surat Massal > Mulai Surat Massal**, lalu pilih salah satu:

- **Surat**: satu salinan halaman untuk setiap orang. Setiap salinan dimulai di halaman baru.
- **Pesan Email**: satu pesan untuk setiap orang. Lihat "Kirim pesan email" di bawah.
- **Amplop** dan **Label**: halaman yang dibuat untuknya. Lihat bagian di bawah.
- **Direktori**: satu daftar untuk semua orang, tanpa pemisah halaman di antara setiap orang. Gunakan untuk daftar telepon atau katalog.
- **Hapus Jenis Gabungan** membuat dokumen kembali menjadi dokumen biasa.

JeffWrite menyimpan jenis gabungan dan nama daftar di dalam berkas saat Anda menyimpannya. Ketika Anda membuka berkas itu lagi, gabungan tersebut masih ada.

## Pilih penerima

**Penerima** adalah orang yang menerima salinan. Klik **Surat Massal > Pilih Penerima**:

- **Ketik Daftar Baru**: ketik orang-orang di jendela yang terbuka. Klik **Entri Baru** untuk setiap orang. JeffWrite kemudian menawarkan untuk menyimpan daftar sebagai berkas .csv.
- **Gunakan Daftar yang Ada**: pilih berkas yang sudah berupa daftar. Bisa berupa berkas .csv atau berkas teks dengan tab (satu baris untuk setiap orang, dan baris pertama berisi nama kolom), lembar kerja (.xlsx, .xls, atau .ods, dari JeffCrunch atau program lain), atau berkas kontak (.vcf).
- **Pilih dari Kontak**: pilih berkas kontak (.vcf). Sebagian besar buku alamat dan ponsel dapat menyimpan kontaknya sebagai satu berkas seperti itu.

Lalu klik **Surat Massal > Edit Daftar Penerima** untuk memeriksa daftar:

- Hapus centang pada sebuah baris untuk mengecualikan orang tersebut.
- Klik judul kolom untuk mengurutkan.
- Ketik di **Saring penerima** untuk hanya menampilkan baris yang memuat kata Anda.
- **Bedakan penerima berdasarkan** memilih kolom yang membedakan satu orang dari yang lain, seperti alamat email. Ketika Anda membuka dokumen lagi dan daftar dibaca, orang yang Anda hapus centangnya tetap dikecualikan, meskipun urutan daftar berubah atau ada orang baru. Dengan **Tempatkan dalam daftar**, mereka diingat berdasarkan nomor baris dalam berkas daftar, meskipun Anda mengurutkan atau menyaring baris di jendela ini. Jika daftar telah berubah sehingga hal ini tidak lagi sesuai, JeffWrite memberi tahu Anda dan menyertakan semua orang yang tidak dapat dicocokkan. Dokumen menyimpan bentuk terkode dari setiap alamat, bukan alamat itu sendiri, tetapi orang yang memiliki dokumen dan menebak sebuah alamat tetap dapat menguji tebakannya. **Periksa Dokumen** (Berkas > Info) menghapus catatan tentang siapa saja yang dikecualikan.
- **Temukan Duplikat** menghapus centang pada data yang berulang.
- **Validasi Alamat** memberi warna pada baris yang tidak memiliki alamat atau yang kode posnya tampak keliru.

## Masukkan rincian ke dalam surat

**Bidang gabungan** adalah tempat penampung. Bidang ini menampilkan nama sebuah kolom dalam tanda kurung sudut ganda, misalnya «Nama Depan». Di setiap salinan, bidang tersebut digantikan oleh rincian orang itu.

Klik tempat rincian akan muncul, lalu gunakan **Surat Massal > Tulis & Sisipkan Bidang**:

- **Sisipkan Bidang Gabungan** menampilkan kolom daftar Anda. Pilih salah satu.
  - **Kode Batang dari Bidang**, dalam daftar yang sama, memasukkan kode batang yang nilainya berasal dari sebuah kolom, misalnya nomor suku cadang atau ISBN 13 digit sebuah buku. Pilih kolomnya, lalu pilih jenisnya dan lihat jendela **Sisipkan Kode Batang** (apa pun yang Anda ketik di sana hanya contoh). Jendela ini hanya menawarkan apa yang dapat dimuat sebuah bidang: jenis, ukuran, tinggi batang, digit, dan tingkat QR. Jendela ini tidak menyertakan harga buku, add-on, karakter cek, dan latar belakang bening, yang tidak dapat dinyatakan oleh sebuah bidang; ISBN buku adalah EAN-13. Di dokumen, bidang ini digambar sebagai kode batang, berupa contoh jenisnya sampai Anda menggabungkan. Untuk mengubah jenis atau tampilannya, klik dua kali, atau klik kanan lalu pilih **Edit Kode Batang**. Saat digabungkan, salinan setiap orang mendapat kode batang dengan nilainya sendiri, digambar sebagai gambar dalam baris, sehingga ikut tercetak dan tersimpan ke PDF seperti gambar lainnya. Nilainya hanya data: tanda kutip atau garis miring terbalik di dalamnya tidak mengubah apa pun. Jika nilai kosong, tidak ada kode batang. Jika sebuah kode tidak dapat memuat nilainya (misalnya huruf dalam EAN-13, atau lebih dari 500 karakter dalam Code 128), salinan tersebut menampilkan pesan singkat dalam tanda kurung siku sebagai gantinya, dan salinan lainnya tidak terpengaruh. Satu penggabungan membuat paling banyak 2.000 kode batang, termasuk setiap nilai yang tidak dapat dimuat; gabungkan lebih sedikit orang jika muncul pesan bahwa terlalu banyak. Dalam berkas `.docx` yang tersimpan, bidang ini berupa bidang DISPLAYBARCODE dengan kolom di dalamnya, sehingga tetap tersimpan saat Anda menyimpan dan membuka berkas lagi. Bidang dengan nilai yang diketik langsung, atau berjenis yang tidak digambar JeffWrite (misalnya UPC-E), dibiarkan apa adanya. Gabungan email memasukkan nilainya sebagai teks.
- **Blok Alamat** memasukkan seluruh alamat surat: nama, perusahaan, jalan, dan kota. Pilih baris yang Anda inginkan. Baris yang kosong dilewati. Anda juga dapat menyebutkan negara yang akan dilewati, misalnya negara Anda sendiri. **Pratinjau** menampilkan alamat orang pertama.
- **Baris Salam** memasukkan baris seperti "Yth. Dr. Fenwick,". Anda memilih kata-katanya. Pilihan pertama, "Yth. Dr. Fenwick,", menggunakan gelar orang tersebut. Jika seseorang tidak memiliki gelar, yang dipakai adalah nama depan dan nama belakang, seperti "Yth. Nora Pell,". Jika seseorang tidak memiliki nama, salamnya memakai kalimat lain, misalnya "Yth. Bapak/Ibu,", yang dapat Anda ubah.
- **Cocokkan Bidang** memberi tahu blok alamat dan baris salam kolom mana yang berisi setiap bagian, jika nama kolom Anda tidak biasa.

## Aturan

**Aturan** membuat salinan berbeda untuk sebagian orang. Klik **Aturan** lalu pilih salah satu:

- **Jika…Maka…Lainnya…**: tampilkan satu teks saat sebuah kolom berisi nilai, dan teks lain saat tidak berisi. Misalnya, jika Provinsi adalah "CA", tulis "Selamat datang di rumah".
- **Lewati Rekaman Jika**: lewati seseorang jika sebuah kolom memenuhi uji yang Anda tetapkan, misalnya saat nama belakang kosong.
- **Rekaman Berikutnya** dan **Rekaman Berikutnya Jika**: gunakan rincian orang berikutnya di tempat yang sama. Label memakai ini untuk menempatkan orang yang berbeda di setiap label.
- **Nomor Rekaman Gabungan** memasukkan nomor baris orang tersebut dalam daftar. **Nomor Urutan Gabungan** menghitung salinan yang sudah dibuat.
- **Tanya** dan **Isian**: menanyakan sesuatu kepada Anda saat Anda menyelesaikan penggabungan. Tanya menyimpan jawaban di bawah sebuah nama. Isian memasukkan jawaban ke dalam teks.
- **Atur Penanda**: memberi nama sebuah nilai. Referensi silang ke nama itu menampilkan nilainya.

## Pratinjau hasil

Klik **Surat Massal > Pratinjau Hasil**. Sebuah panel terbuka yang menampilkan halaman untuk satu orang. Gunakan **Rekaman Pertama**, **Rekaman Sebelumnya**, **Rekaman Berikutnya**, dan **Rekaman Terakhir** untuk berpindah di antara orang-orang tersebut, atau ketik sebuah nomor. **Cari Penerima** melompat ke orang berikutnya yang memiliki kata Anda dalam salah satu rinciannya.

**Sorot Bidang Gabungan** memberi warna abu-abu pada bidang gabungan di halaman Anda sehingga Anda dapat melihat letaknya. Bidang lain, seperti nomor halaman, tidak diberi warna. Sorotan tampil di layar dan tidak pernah tercetak.

Dokumen Anda sendiri tidak berubah. **Periksa Kesalahan** memberi tahu Anda ketika daftar tidak memiliki kolom untuk sebuah bidang, atau ketika tidak ada orang yang disertakan.

## Selesaikan gabungan

Klik **Surat Massal > Selesaikan & Gabungkan**:

- **Edit Dokumen Individu** membuat dokumen baru berisi salinan setiap orang. Anda dapat mengubahnya sebelum mencetak.
- **Cetak Dokumen** membuat dokumen tersebut lalu membuka halaman **Cetak**.
- **Simpan sebagai PDF** menyimpan setiap salinan dalam satu berkas PDF.
- **Kirim Pesan Email** membuat satu berkas pesan .eml untuk setiap orang, dalam folder yang Anda pilih. Pilih kolom yang berisi alamat email, lalu ketik subjek. JeffWrite tidak mengirim pesan tersebut. Buka berkas-berkasnya di program email Anda.
- **Simpan Masing-masing sebagai Berkas** membuat satu berkas untuk setiap orang, dalam folder yang Anda pilih, sebagai berkas .docx atau PDF. Beri nama berkas dengan kolom dalam kurung kurawal, misalnya {Nama Belakang}, {Nama Depan}; nama yang dipakai dua kali mendapat nomor. Sebuah kotak menampilkan nama depan dan jumlah berkas yang akan dibuat. Anda diminta konfirmasi sebelum lebih dari 200 berkas, dan sekali jika berkas dengan nama yang sama sudah ada di folder. Klik **Batal** untuk berhenti di antara berkas: berkas yang sudah dibuat tetap ada; menutup jendela juga memberikan hasil yang sama. Berkas yang merupakan tautan ke berkas lain digantikan oleh berkas baru, dan berkas yang dituju tautan itu tidak disentuh. Berkas-berkas hanya memuat surat untuk satu orang: tidak memuat daftar, lokasinya, atau siapa saja yang dikecualikan. Satu penggabungan membuat paling banyak 100.000 berkas.

Orang yang Anda hapus centangnya dalam daftar akan dikecualikan.

Berkas pesan hanya dibuat untuk orang yang alamatnya berupa satu alamat email. Jika alamat memuat dua (dengan koma atau titik koma di antaranya), memiliki nama di dalamnya, atau tidak memiliki domain, orang tersebut tidak mendapat pesan, dan JeffWrite memberi tahu jumlahnya. Subjek yang panjang disimpan dalam beberapa bagian yang akan disatukan kembali oleh program email.

Penggabungan menampilkan jendela kemajuan, dan **Batal** menghentikannya. Jika sebuah aturan mengajukan pertanyaan (Tanya atau Isian), Anda akan ditanya setiap pertanyaan yang berbeda satu kali, hingga 20 pertanyaan. Setelah itu, sisanya menggunakan jawaban bawaan. Jika Anda membatalkan sebuah pertanyaan, seluruh penggabungan berhenti. Penggabungan akan berhenti dan memberi tahu Anda jika akan menghasilkan lebih dari 10.000 halaman, sehingga dokumen yang sangat panjang dengan daftar yang sangat panjang tidak memakan waktu berjam-jam. Gabungkan sebagian daftar sekali waktu sebagai gantinya.

## Amplop

Klik **Surat Massal > Amplop**. Ketik alamat tujuan dan alamat pengirim Anda, lalu pilih ukuran amplop dari daftar. Ukuran diurutkan berdasarkan lebar dan tingginya. Klik **Tambahkan ke Dokumen** untuk meletakkan amplop di halaman tersendiri di awal dokumen Anda, atau **Cetak** untuk mencetaknya. Halaman akan selebar sisi panjang amplop, yang merupakan cara paling umum printer menerima amplop.

Untuk membuat amplop untuk sebuah penggabungan, klik **Mulai Surat Massal > Amplop**. Setiap amplop memiliki blok alamat.

## Label

Klik **Surat Massal > Label**. Ketik alamat lalu pilih lembar dari daftar. Lembar dinamai menurut jumlah label di dalamnya dan ukuran setiap label. Jika lembar Anda tidak ada dalam daftar, pilih **Lembar sendiri** dan ketik ukurannya: berapa label ke samping dan ke bawah, seberapa besar sebuah label, jarak di atas dan di samping label pertama, serta seberapa jauh jarak antarlabel.

- **Satu halaman penuh label yang sama** menempatkan alamat yang sama pada setiap label di lembar.
- **Satu label** menempatkan alamat pada satu label, sehingga Anda dapat memakai lembar yang sudah terpakai sebagian. Sebutkan baris dan kolom yang dimaksud.

Klik **Dokumen Baru** untuk membuat dokumen yang dapat Anda lihat dan ubah, atau **Cetak** untuk mencetaknya.

Untuk membuat label untuk sebuah gabungan, klik **Mulai Surat Massal > Label**. Label pertama memiliki blok alamat, dan setiap label lainnya dimulai dengan aturan **Rekaman Berikutnya**. Jika Anda mengubah label pertama, klik **Perbarui Label** untuk menyalin perubahan ke semua label lainnya.

## Isi berkas

Jenis gabungan, nama dan lokasi daftar, subjek, dan kolom email disimpan dalam berkas .docx Anda. Orang mana saja yang Anda hapus centangnya juga disimpan, sebagai kode singkat dari kolom yang Anda pilih, bukan sebagai nama atau alamat mereka (hanya berkas .docx yang menyimpannya; Periksa Kompatibilitas memberi tahu hal ini untuk jenis lainnya). Pilihan Anda di Cocokkan Bidang juga disimpan. Daftar dibaca ulang dari berkasnya setiap kali. Jika berkas tidak berada di lokasinya, JeffWrite mencari di sebelah dokumen Anda, lalu meminta Anda memilihnya.

Sebuah daftar dapat memiliki hingga 1.000 kolom dan 4.000.000 sel secara keseluruhan, dan penggabungan menggunakan 100.000 orang pertama. Jika berkas memiliki lebih banyak, JeffWrite menggunakan apa yang muat dan memberi tahu Anda. Baris yang kosong dalam daftar akan dilewati.

## Terkait

- [Gunakan bidang](fields)
- [Cetak](print)
- [Buat dan ubah tabel](tables)
