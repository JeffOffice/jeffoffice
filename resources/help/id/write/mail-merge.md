# Surat massal, amplop, dan label
<!-- keywords: surat massal, gabung, cetak surat massal, mailings, penerima, daftar alamat, surat formulir, label, amplop, bidang gabungan, blok alamat, baris salam, aturan, jika maka lainnya, lewati rekaman, rekaman berikutnya, pratinjau hasil, selesai dan gabungkan, direktori, pesan email, csv, xlsx, ods, vcard -->

**Surat massal** membuat banyak salinan dari satu surat, dengan detail orang yang berbeda di setiap salinan. Anda menulis surat sekali saja. Anda menyimpan detail orang-orang dalam sebuah **daftar**. JeffWrite memasukkan nama dan alamat orang pertama ke salinan pertama, orang kedua ke salinan kedua, dan seterusnya.

Alatnya ada di tab **Mailings**. Untuk panduan langkah demi langkah, klik **Mailings > Start Mail Merge > Step-by-Step Mail Merge Wizard**.

## Pilih yang akan Anda buat

Klik **Mailings > Start Mail Merge** lalu pilih salah satu:

- **Letters**: satu salinan halaman untuk setiap orang. Setiap salinan dimulai di halaman baru.
- **Email Messages**: satu pesan untuk setiap orang. Lihat "Kirim pesan email" di bawah.
- **Envelopes** dan **Labels**: halaman yang dibuat untuk keperluan itu. Lihat bagian di bawah.
- **Directory**: satu daftar untuk semua orang, tanpa pemisah halaman di antara orang-orang tersebut. Gunakan untuk daftar telepon atau katalog.
- **Clear Merge Type** membuat dokumen kembali menjadi dokumen biasa.

JeffWrite menyimpan jenis penggabungan dan nama daftar di dalam berkas saat Anda menyimpannya. Ketika Anda membuka berkas itu lagi, penggabungannya masih ada.

## Pilih penerima

**Penerima** adalah orang yang menerima salinan. Klik **Mailings > Select Recipients**:

- **Type a New List**: ketik orang-orangnya di jendela yang terbuka. Klik **New Entry** untuk setiap orang. JeffWrite kemudian menawarkan untuk menyimpan daftar sebagai berkas .csv.
- **Use an Existing List**: pilih berkas yang sudah berupa daftar. Bisa berupa berkas .csv atau berkas teks dengan tab (satu baris untuk setiap orang, dan baris pertama berisi nama kolom), lembar kerja (.xlsx, .xls, atau .ods, dari JeffCrunch atau program lain), atau berkas kontak (.vcf).
- **Select from Contacts**: pilih berkas kontak (.vcf). Sebagian besar buku alamat dan ponsel dapat menyimpan kontaknya sebagai berkas seperti itu.

Lalu klik **Mailings > Edit Recipient List** untuk memeriksa daftar:

- Hapus centang pada baris untuk mengecualikan orang tersebut.
- Klik judul kolom untuk mengurutkan.
- Ketik di **Filter recipients** untuk menampilkan hanya baris yang memuat kata-kata Anda.
- **Find Duplicates** menghapus centang pada data yang berulang.
- **Validate Addresses** memberi warna pada baris yang tidak punya alamat atau kode pos yang tampaknya salah.

## Masukkan detail ke dalam surat

**Bidang gabungan** adalah tempat penampung. Bidang ini menampilkan nama kolom di antara tanda kurung siku ganda, misalnya «First Name». Di setiap salinan, bidang ini digantikan dengan detail orang tersebut.

Klik tempat detail akan muncul, lalu gunakan **Mailings > Write & Insert Fields**:

- **Insert Merge Field** menampilkan kolom-kolom daftar Anda. Pilih salah satu.
  - **Barcode from Field**, dalam daftar yang sama, memasukkan kode batang yang nilainya berasal dari sebuah kolom, misalnya nomor suku cadang atau ISBN 13 digit sebuah buku. Pilih kolomnya, lalu pilih jenisnya dan lihat di jendela **Insert Barcode** (apa pun yang Anda ketik di sana hanya contoh). Jendela ini hanya menawarkan apa yang dapat dimuat sebuah bidang: jenis, ukuran, tinggi batang, digit, dan tingkat QR. Jendela ini tidak menyertakan harga buku, add-on, karakter cek, dan latar belakang bening, yang tidak dapat dinyatakan oleh bidang; ISBN buku adalah EAN-13. Di dokumen, bidang menampilkan nama kolom di antara tanda kutip sudut (guillemet). Saat penggabungan, salinan setiap orang mendapat kode batang dengan nilainya sendiri, digambar sebagai gambar di dalam baris, sehingga ikut tercetak dan tersimpan ke PDF seperti gambar lainnya. Nilainya hanya data: tanda kutip atau garis miring terbalik di dalamnya tidak mengubah apa pun. Jika nilai kosong, tidak ada kode batang. Jika sebuah kode tidak dapat memuat nilai (huruf dalam EAN-13, atau lebih dari 500 karakter dalam Code 128), salinan itu menampilkan pesan singkat di dalam tanda kurung siku sebagai gantinya, dan salinan lain tidak terpengaruh. Satu penggabungan membuat paling banyak 2.000 kode batang, termasuk setiap nilai yang tidak dapat dimuat; gabungkan lebih sedikit orang jika muncul pesan bahwa jumlahnya terlalu banyak. Dalam `.docx` yang disimpan, bidang ini berupa bidang DISPLAYBARCODE dengan kolom di dalamnya, sehingga tetap tersimpan saat berkas disimpan dan dibuka kembali. Bidang dengan nilai yang diketik langsung, atau berjenis yang tidak digambar JeffWrite (UPC-E, misalnya), dibiarkan apa adanya. Penggabungan email memasukkan nilai sebagai teks.
- **Address Block** memasukkan seluruh alamat surat: nama, perusahaan, jalan, dan kota. Pilih baris yang Anda inginkan. Baris yang kosong dihilangkan. Anda juga dapat menyebutkan negara yang akan dihilangkan, misalnya negara Anda sendiri. **Preview** menampilkan alamat orang pertama.
- **Greeting Line** memasukkan baris seperti "Dear Dr. Fenwick,". Anda memilih kata-katanya. Pilihan pertama, "Dear Dr. Fenwick,", menggunakan gelar orang tersebut. Jika seseorang tidak memiliki gelar, yang dipakai adalah nama depan dan nama belakangnya, seperti "Dear Nora Pell,". Jika seseorang tidak memiliki nama, salam menggunakan kalimat lain, seperti "Dear Sir or Madam,", yang dapat Anda ubah.
- **Match Fields** memberi tahu blok alamat dan baris salam kolom mana yang memuat setiap bagian, jika nama kolom Anda tidak biasa.

## Aturan

**Aturan** membuat salinan berbeda untuk sebagian orang. Klik **Rules** lalu pilih salah satu:

- **If…Then…Else**: menampilkan satu teks ketika sebuah kolom berisi nilai, dan teks lain ketika tidak. Misalnya, jika State adalah "CA", tulis "Welcome home".
- **Skip Record If**: melewati seseorang ketika sebuah kolom berisi nilai, misalnya ketika nama belakang kosong.
- **Next Record** dan **Next Record If**: menggunakan detail orang berikutnya di tempat yang sama. Label menggunakan ini untuk menempatkan orang yang berbeda di setiap label.
- **Merge Record #** memasukkan nomor baris orang tersebut dalam daftar. **Merge Sequence #** menghitung salinan yang sudah dibuat.
- **Ask** dan **Fill-in**: menanyakan sesuatu kepada Anda ketika penggabungan selesai. Ask menyimpan jawaban di bawah sebuah nama. Fill-in memasukkan jawaban ke dalam teks.
- **Set Bookmark**: memberi sebuah nama suatu nilai. Referensi silang ke nama tersebut menampilkan nilainya.

## Pratinjau hasil

Klik **Mailings > Preview Results**. Sebuah panel terbuka dan menampilkan halaman untuk satu orang. Gunakan **First Record**, **Previous Record**, **Next Record**, dan **Last Record** untuk berpindah antarorang, atau ketik nomor. **Find Recipient** melompat ke orang berikutnya yang memiliki kata-kata Anda dalam salah satu detailnya.

**Highlight Merge Fields** memberi warna abu-abu pada bidang gabungan di halaman Anda agar Anda dapat melihat letaknya. Bidang lain, seperti nomor halaman, tidak diberi warna. Pewarnaan ini tampil di layar dan tidak pernah tercetak.

Dokumen Anda sendiri tidak berubah. **Check for Errors** memberi tahu Anda jika daftar tidak memiliki kolom untuk sebuah bidang, atau jika tidak ada orang yang disertakan.

## Selesaikan penggabungan

Klik **Mailings > Finish & Merge**:

- **Edit Individual Documents** membuat dokumen baru berisi salinan setiap orang. Anda dapat mengubahnya sebelum mencetak.
- **Print Documents** membuat dokumen itu dan membuka halaman **Print**.
- **Save as PDF** menyimpan semua salinan dalam satu berkas PDF.
- **Send Email Messages** membuat satu berkas pesan .eml untuk setiap orang, di dalam folder yang Anda pilih. Pilih kolom yang berisi alamat email, dan ketik subjek. JeffWrite tidak mengirim pesan-pesan tersebut. Buka berkasnya di program email Anda.

Orang yang Anda hapus centangnya dalam daftar tidak disertakan.

Berkas pesan hanya dibuat untuk orang yang alamatnya berisi satu alamat email. Jika sebuah alamat berisi dua alamat (dengan koma atau titik koma di antaranya), memiliki nama di dalamnya, atau tidak memiliki domain, orang tersebut tidak mendapat pesan, dan JeffWrite memberi tahu jumlahnya. Subjek yang panjang disimpan dalam beberapa bagian yang akan disusun kembali oleh program email.

Penggabungan menampilkan jendela kemajuan, dan **Cancel** menghentikannya. Jika sebuah aturan mengajukan pertanyaan (Ask atau Fill-in), Anda ditanya setiap pertanyaan yang berbeda satu kali, hingga 20 pertanyaan. Setelah itu, sisanya menggunakan jawaban standarnya. Jika Anda membatalkan sebuah pertanyaan, seluruh penggabungan berhenti. Penggabungan berhenti, dan memberi tahu Anda, jika akan menghasilkan lebih dari 10.000 halaman, sehingga dokumen yang sangat panjang dengan daftar yang sangat panjang tidak memakan waktu berjam-jam. Gabungkan sebagian daftar setiap kali sebagai gantinya.

## Amplop

Klik **Mailings > Envelopes**. Ketik alamat tujuan dan alamat pengirim Anda, lalu pilih ukuran amplop dari daftar. Ukuran ditampilkan menurut lebar dan tingginya. Klik **Add to Document** untuk menempatkan amplop pada halamannya sendiri di awal dokumen Anda, atau **Print** untuk mencetaknya. Halaman selebar sisi panjang amplop, yang merupakan cara kebanyakan printer menerima amplop.

Untuk membuat amplop untuk penggabungan, klik **Start Mail Merge > Envelopes**. Setiap amplop memiliki blok alamat.

## Label

Klik **Mailings > Labels**. Ketik alamat dan pilih lembar dari daftar. Lembar diberi nama menurut jumlah label yang dimuatnya dan ukuran setiap label. Jika lembar Anda tidak ada dalam daftar, pilih **Your own sheet** dan ketik ukurannya: berapa label ke samping dan ke bawah, seberapa besar sebuah label, ruang di atas dan di samping label pertama, serta jarak antarlabel.

- **Full page of the same label** memasukkan alamat yang sama ke setiap label pada lembar.
- **Single label** memasukkan alamat ke satu label, sehingga Anda dapat memakai sisa lembar yang sudah sebagian terpakai. Sebutkan baris dan kolomnya.

Klik **New Document** untuk membuat dokumen yang dapat Anda lihat dan ubah, atau **Print** untuk mencetaknya.

Untuk membuat label untuk penggabungan, klik **Start Mail Merge > Labels**. Label pertama memiliki blok alamat, dan setiap label lainnya dimulai dengan aturan **Next Record**. Jika Anda mengubah label pertama, klik **Update Labels** untuk menyalin perubahan ke semua label lainnya.

## Isi berkas

Jenis penggabungan, nama dan lokasi daftar, subjek, dan kolom email disimpan di berkas .docx Anda. Orang mana saja yang Anda hapus centangnya, serta pilihan Match Fields Anda, tidak disimpan. Keduanya akan diminta lagi setelah Anda membuka berkas. Daftar dibaca ulang dari berkasnya setiap kali. Jika berkas tidak berada di lokasi semula, JeffWrite mencari di sebelah dokumen Anda, lalu meminta Anda memilih berkasnya.

Sebuah daftar dapat memiliki hingga 1.000 kolom dan 4.000.000 sel secara keseluruhan, dan penggabungan menggunakan 100.000 orang pertama. Jika berkas memiliki lebih banyak, JeffWrite menggunakan yang muat dan memberi tahu Anda. Baris yang kosong dihilangkan dari daftar.

## Terkait

- [Menggunakan bidang](fields)
- [Mencetak](print)
- [Membuat dan mengubah tabel](tables)
