# Lacak perubahan, dan terima atau tolak perubahan
<!-- keywords: bandingkan, bandingkan dokumen, dua versi, apa yang berubah, lacak perubahan, perubahan yang dilacak, revisi, markup, terima, tolak, terima semua, tolak semua, tampilkan untuk tinjauan, markup sederhana, semua markup, tanpa markup, asli, tampilkan markup, balon, panel tinjauan, bilah perubahan, sisipan, penghapusan, ctrl+shift+e, penulis, peninjau, nama pengguna, suntingan dari orang lain -->

**Lacak Perubahan** menyimpan catatan setiap suntingan yang Anda buat, sehingga Anda (atau orang lain) dapat melihat apa yang berubah dan memutuskan apakah akan mempertahankannya. Gunakan fitur ini saat Anda menyunting dokumen milik orang lain, atau dokumen yang akan ditinjau seseorang setelah Anda.

## Mengaktifkan dan menonaktifkan Lacak Perubahan

Klik **Tinjau > Lacak Perubahan**, atau tekan **Ctrl+Shift+E**. Bilah status di bagian bawah jendela menunjukkan apakah fitur ini aktif. Klik pada tulisan tersebut untuk mengubah keadaannya. Pengaturan ini disimpan di dalam dokumen, sehingga tetap aktif saat dokumen kembali dari orang lain.

Saat fitur ini aktif:

- **Apa yang Anda ketik** ditandai sebagai sisipan.
- **Apa yang Anda hapus** tidak benar-benar hilang. Teks tersebut tetap ada, dicoret, sehingga dapat dikembalikan. Teks yang Anda ketik sendiri saat pelacakan aktif menjadi pengecualian: menghapusnya langsung menghilangkannya, karena tidak ada orang lain yang pernah melihatnya.
- **Pemisah paragraf yang Anda buat** (Enter) ditandai sebagai disisipkan, dan yang Anda hapus (Backspace di awal paragraf) ditandai sebagai dihapus.
- **Perubahan tampilan** (tebal, font, ukuran, warna, perataan, indentasi, atau gaya) dicatat beserta tampilan teks sebelumnya.

Setiap perubahan disimpan bersama nama pembuatnya dan waktunya. Nama tersebut adalah yang ada di **Berkas > Opsi**, pada bagian **Nama pengguna**. Ubah di sana jika namanya salah.

Setiap suntingan merupakan satu langkah, sehingga **Ctrl+Z** membatalkannya seperti suntingan lain.

## Tampilan perubahan

Secara bawaan, setiap orang memiliki warna sendiri untuk perubahannya. Orang pertama yang perubahannya ditampilkan dokumen mendapat warna pertama, orang berikutnya warna kedua, dan seterusnya. Seseorang mempertahankan warnanya selama dokumen masih terbuka. Teks sisipan digarisbawahi dengan warna orang tersebut, dan teks yang dihapus dicoret. Teks yang dipindahkan berwarna hijau: dicoret ganda di tempat asalnya, dan digarisbawahi ganda di tempat tujuannya. Teks yang format-nya diubah dicetak tebal. Sebuah bilah di margin kiri menandai setiap baris yang memiliki perubahan.

Anda dapat mengubah semua ini. Klik **Tinjau > Pelacakan > Opsi Pelacakan** untuk membuka kotak **Opsi Lacak Perubahan**. Di bawah **Markup**, setiap jenis perubahan memiliki **Tampilan** dan **Warna**:

- **Sisipan**: garis bawah, garis bawah ganda, coret, tebal, miring, hanya warna, atau (tidak ada).
- **Penghapusan**: coret, coret ganda, tersembunyi, caret, atau hanya warna. Dengan **Tersembunyi**, kata yang dihapus tidak ditampilkan dalam baris. Dengan **Caret**, kata tersebut juga tidak ditampilkan, dan tanda ^ kecil menunjukkan tempat semula. Teks yang dipindahkan juga disembunyikan jika Anda memilih **Balon** untuk penghapusan, tetapi tidak dalam kondisi lain.
- **Dipindahkan dari** dan **Dipindahkan ke**: tampilan dan warna untuk teks yang dipindahkan. Pilihan ini tersedia selama **Lacak pemindahan** dicentang. Jika Anda menghapus centangnya, teks yang dipindahkan ditampilkan sebagai penghapusan dan sisipan biasa.
- **Pemformatan**: tebal, hanya warna, atau (tidak ada). Jika Anda menghapus centang **Lacak pemformatan**, perubahan format tidak ditandai dalam teks.
- **Bilah perubahan**: bingkai kiri, bingkai kanan, bingkai luar (sisi kanan pada halaman ganjil dan sisi kiri pada halaman genap), atau (tidak ada), serta warna bilah.

Setiap kotak **Warna** menawarkan **Menurut penulis** (warna masing-masing orang) atau satu warna untuk setiap orang. Pilihan ini milik Anda sendiri. Pengaturan disimpan di komputer ini, bukan di dalam dokumen, berlaku untuk setiap dokumen dan setiap jendela yang terbuka, dan setiap jendela langsung berubah begitu Anda mengklik **OK**. Bilah digambar dengan cara yang sama di layar, pada hasil cetak, dan dalam PDF setiap kali markup ditampilkan.

## Pilih yang Anda lihat: Tampilkan untuk Tinjauan

Klik **Tinjau > Tampilkan untuk Tinjauan**, lalu pilih salah satu:

- **Semua Markup** menampilkan setiap perubahan dalam teks, dengan bilah di margin.
- **Markup Sederhana** menampilkan dokumen seperti tampilannya jika semua perubahan diterima, dengan bilah di margin pada setiap baris yang memiliki perubahan.
- **Tanpa Markup** menampilkan dokumen seperti tampilannya jika semua perubahan diterima, tanpa tambahan apa pun.
- **Asli** menampilkan dokumen seperti sebelum ada perubahan: teks yang dihapus kembali muncul, dan sisipan hilang.

Pilihan ini hanya mengubah tampilan. Dokumen tidak berubah, dan pilihan ini tidak disimpan bersama dokumen. Dokumen yang memiliki perubahan akan dibuka dengan **Semua Markup**.

**Tinjau > Tampilkan Markup** menentukan jenis perubahan yang ditampilkan: **Sisipan dan Penghapusan**, **Pemformatan**, dan **Komentar**. Di bawahnya, pilih cara menampilkan teks yang dihapus. **Tampilkan Semua Revisi dalam Baris** menyimpan teks tersebut dalam baris, dicoret. **Tampilkan Revisi dalam Balon** mengeluarkan teks yang dihapus dari baris dan menaruhnya dalam balon di margin. **Tampilkan Hanya Komentar dan Pemformatan dalam Balon** menyimpan sisipan dan penghapusan dalam teks.

## Menerima atau menolak perubahan

Menerima perubahan berarti mempertahankannya secara permanen. Menolak perubahan berarti membatalkannya.

- **Tinjau > Terima** menerima perubahan tempat kursor berada (atau setiap perubahan dalam teks yang Anda pilih), lalu pindah ke perubahan berikutnya. **Tinjau > Tolak** melakukan hal yang sama, tetapi membatalkan perubahan.
- Klik panah di bawah **Terima** atau **Tolak** untuk pilihan lainnya: **Terima Semua Perubahan yang Ditampilkan**, **Terima Semua Perubahan**, dan **Terima Semua Perubahan dan Hentikan Pelacakan**; serta **Tolak Semua Perubahan yang Ditampilkan** dan **Tolak Semua Perubahan**. "Yang ditampilkan" berarti perubahan dari jenis yang diaktifkan melalui **Tampilkan Markup** (dan, di panel **Tinjauan**, dari peninjau yang Anda pilih).
- **Sebelumnya** dan **Berikutnya** pada grup **Perubahan** berpindah dari satu perubahan ke perubahan berikutnya tanpa mengubah apa pun.

Setiap pilihan ini merupakan satu langkah, sehingga **Ctrl+Z** mengembalikan dokumen persis seperti semula.

Menerima penghapusan akan menghilangkan teks secara permanen. Menolak sisipan juga menghilangkan teks. Menerima sisipan mempertahankan teks dan menghapus tanda bahwa teks itu baru. Menolak penghapusan mempertahankan teks dan menghapus coretannya. Menolak perubahan tampilan mengembalikan tampilan semula teks. Teks yang dipindahkan merupakan sepasang perubahan, satu di tempat asal dan satu di tempat tujuan. Jika pasangan itu ditandai dengan nama yang sama, seperti yang disimpan program lain, menerima atau menolak salah satu bagiannya juga berlaku pada bagian lainnya, sehingga teks tersebut dipindahkan secara permanen atau dikembalikan.

## Panel Tinjauan

Klik **Tinjau > Panel Tinjauan** untuk membuka daftar perubahan dalam dokumen: jenisnya, siapa pembuatnya, kapan, dan kata-katanya. Klik salah satu untuk menampilkannya dalam teks. Gunakan **Terima**, **Tolak**, **Sebelumnya**, dan **Berikutnya** di bawah daftar, atau pilih satu peninjau dari kotak di atasnya untuk hanya melihat perubahan orang tersebut.

## Yang belum dilacak

Perubahan pada tabel (baris, sel, penggabungan), penomoran dan daftar, bagian dan tata letak halaman, serta teks dalam header, footer, dan catatan dilakukan seperti biasa, tanpa catatan. Perubahan yang dicatat program lain di tempat-tempat tersebut tetap dipertahankan dan disimpan seperti semula.

## Membandingkan dua versi

Bandingkan mencari perbedaan antara dua versi dokumen dan menampilkannya sebagai perubahan yang dilacak.

1. Buka versi yang lebih lama. Ini adalah **asli**.
2. Klik **Bandingkan** pada grup **Bandingkan** di tab **Tinjau**.
3. Klik **Telusuri** dan pilih versi yang lebih baru. Ini adalah versi **revisi**.
4. Ketikkan nama yang akan dicantumkan pada perubahan. Pilih opsi lain yang diinginkan, misalnya mengabaikan huruf kapital. **Temukan teks yang dipindahkan** dan **Temukan perubahan pemformatan** aktif kecuali Anda menonaktifkannya.
5. Klik **OK**.

Jendela baru akan terbuka dengan teks asli dan perubahan yang mengubahnya menjadi teks revisi. Kedua berkas Anda tidak berubah. Gunakan **Terima** dan **Tolak** seperti pada perubahan yang dilacak lainnya. Jika Anda menerima semua perubahan, Anda mendapatkan teks revisi. Jika Anda menolak semua perubahan, Anda mendapatkan teks asli.

Bandingkan hanya memeriksa teks pada badan dokumen. Tabel, header, footer, dan catatan kaki tidak diperiksa.

Paragraf yang terdiri atas 12 kata atau lebih dan dipotong dari satu tempat lalu ditempelkan di tempat lain ditampilkan sebagai **pemindahan**: satu catatan di tempat asal, dan satu catatan di tempat tujuan. Menerima atau menolak salah satunya berlaku pada keduanya. Paragraf tersebut harus sama, atau sekurang-kurangnya 9 dari 10 kata sama dan sebagian besar dalam urutan yang sama; jika tidak, paragraf ditampilkan sebagai penghapusan dan sisipan. Kata-kata yang berbeda di dalam paragraf yang dipindahkan ditampilkan sebagai sisipan dan penghapusan di dalam pemindahan tersebut.

Teks yang sama di kedua versi tetapi tampilannya berbeda ditampilkan sebagai **perubahan pemformatan**. Bandingkan memeriksa tebal, miring, garis bawah, ukuran, font, warna, sorotan, serta gaya, perataan, indentasi, dan spasi paragraf. Menolak perubahan akan mengembalikan tampilan lama.

Jika kedua dokumen memiliki terlalu banyak perubahan, Bandingkan akan memberi tahu Anda. Setelah itu, sebagian paragraf ditampilkan sebagai diganti seluruhnya, dan pencarian pemindahan atau perubahan pemformatan mungkin tidak dilakukan pada semua paragraf.
