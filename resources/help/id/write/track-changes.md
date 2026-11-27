# Lacak perubahan, lalu terima atau tolak

<!-- keywords: bandingkan, bandingkan dokumen, dua versi, apa yang berubah, lacak perubahan, perubahan yang dilacak, revisi, markup, terima, tolak, terima semua, tolak semua, tampilkan untuk tinjauan, markup sederhana, semua markup, tanpa markup, asli, tampilkan markup, balon, panel peninjauan, bilah perubahan, penyisipan, penghapusan, ctrl+shift+e, penulis, peninjau, nama pengguna, suntingan dari orang lain -->

**Lacak Perubahan** mencatat setiap suntingan yang Anda buat, sehingga Anda atau orang lain dapat melihat apa yang berubah dan memutuskan apakah akan menyimpannya. Gunakan fitur ini saat Anda menyunting dokumen milik orang lain, atau dokumen yang akan ditinjau seseorang setelah Anda.

## Mengaktifkan dan menonaktifkan Lacak Perubahan

Klik **Tinjau > Lacak Perubahan**, atau tekan **Ctrl+Shift+E**. Bilah status di bagian bawah jendela menunjukkan apakah fitur ini aktif. Klik tulisan tersebut untuk mengubah keadaannya. Pengaturan ini disimpan di dalam dokumen, sehingga tetap aktif saat dokumen kembali dari orang lain.

Saat fitur ini aktif:

- **Teks yang Anda ketik** ditandai sebagai penyisipan.
- **Teks yang Anda hapus** tidak benar-benar hilang. Teks itu tetap ada di dalam kalimat, dicoret, sehingga dapat dikembalikan. Pengecualian hanya untuk teks yang Anda ketik sendiri selama Lacak Perubahan aktif: menghapusnya langsung menghilangkan teks tersebut, karena tidak ada orang lain yang pernah melihatnya.
- **Pemisah paragraf yang Anda buat** (Enter) ditandai sebagai disisipkan, dan yang Anda hapus (Backspace di awal paragraf) ditandai sebagai dihapus.
- **Perubahan tampilan** (tebal, font, ukuran, warna, perataan, indentasi, atau gaya) dicatat beserta tampilan teks sebelumnya.

Setiap perubahan disimpan bersama nama orang yang membuatnya dan waktunya. Nama diambil dari **Berkas > Opsi**, di bawah **Nama pengguna**. Ubah di sana jika namanya salah.

Setiap suntingan merupakan satu langkah, sehingga **Ctrl+Z** membatalkannya seperti suntingan lain.

## Tampilan perubahan

Secara bawaan, perubahan setiap orang memiliki warna sendiri. Orang pertama yang perubahannya ditampilkan dokumen mendapat warna pertama, orang berikutnya warna kedua, dan seterusnya. Seseorang mempertahankan warnanya selama dokumen masih terbuka. Teks yang disisipkan digarisbawahi dengan warna orang tersebut, dan teks yang dihapus dicoret. Teks yang dipindahkan berwarna hijau: dicoret ganda di tempat asalnya, dan digarisbawahi ganda di tempat tujuannya. Teks yang format-nya diubah ditebalkan. Sebuah bilah di margin kiri menandai setiap baris yang memiliki perubahan.

Anda dapat mengubah semua ini. Klik **Tinjau > Pelacakan > Opsi Pelacakan** untuk membuka kotak **Opsi Lacak Perubahan**. Di bawah **Markup**, setiap jenis perubahan memiliki **Tampilan** dan **Warna**:

- **Penyisipan**: garis bawah, garis bawah ganda, coret, tebal, miring, hanya warna, atau (tidak ada).
- **Penghapusan**: coret, coret ganda, tersembunyi, caret, atau hanya warna. Dengan **Tersembunyi**, kata yang dihapus tidak ditampilkan pada baris. Dengan **Caret**, kata tersebut juga tidak ditampilkan, dan tanda ^ kecil menunjukkan letaknya. Teks yang dipindahkan juga tidak ditampilkan jika Anda memilih **Balon** untuk penghapusan, tetapi tidak jika pilihan lain.
- **Dipindahkan dari** dan **Dipindahkan ke**: tampilan dan warna untuk teks yang dipindahkan. Pilihan ini tersedia selama **Lacak pemindahan** dicentang. Jika Anda menghapus centangnya, teks yang dipindahkan ditampilkan sebagai penghapusan dan penyisipan biasa.
- **Pemformatan**: tebal, hanya warna, atau (tidak ada). Jika Anda menghapus centang **Lacak pemformatan**, perubahan format tidak ditandai dalam teks.
- **Bilah perubahan**: bingkai kiri, bingkai kanan, bingkai luar (sisi kanan halaman ganjil dan sisi kiri halaman genap), atau (tidak ada), beserta warna bilahnya.

Setiap kotak **Warna** menawarkan **Menurut penulis** (warna masing-masing orang) atau satu warna untuk semua orang. Pilihan ini milik Anda. Pengaturan disimpan di komputer ini, bukan di dalam dokumen, berlaku untuk setiap dokumen dan setiap jendela yang terbuka, dan setiap jendela berubah segera setelah Anda klik **OK**. Bilah digambar dengan cara yang sama di layar, pada hasil cetak, dan di PDF setiap kali markup ditampilkan.

## Memilih yang Anda lihat: Tampilkan untuk Tinjauan

Klik **Tinjau > Tampilkan untuk Tinjauan** dan pilih salah satu:

- **Semua Markup** menampilkan setiap perubahan dalam teks, dengan bilah di margin.
- **Markup Sederhana** menampilkan dokumen seperti tampilannya jika semua perubahan diterima, dengan bilah di margin pada setiap baris yang memiliki perubahan.
- **Tanpa Markup** menampilkan dokumen seperti tampilannya jika semua perubahan diterima, tanpa tanda lain.
- **Asli** menampilkan dokumen seperti sebelum ada perubahan apa pun: teks yang dihapus kembali muncul, dan teks yang disisipkan hilang.

Pilihan ini hanya mengubah apa yang Anda lihat. Pilihan ini tidak mengubah dokumen dan tidak disimpan bersamanya. Dokumen yang memiliki perubahan akan terbuka dengan **Semua Markup**.

**Tinjau > Tampilkan Markup** memilih jenis perubahan yang ditampilkan: **Penyisipan dan Penghapusan**, **Pemformatan**, dan **Komentar**. Di bawahnya, pilih cara teks yang dihapus ditampilkan. **Tampilkan Revisi dalam Baris** mempertahankan teks yang dihapus di dalam kalimat, dicoret. **Tampilkan Revisi dalam Balon** mengeluarkan teks yang dihapus dari baris dan menempatkannya dalam balon di margin. **Tampilkan Hanya Komentar dan Pemformatan dalam Balon** mempertahankan penyisipan dan penghapusan di dalam teks.

## Menerima atau menolak perubahan

Menerima perubahan berarti menyimpannya secara permanen. Menolak perubahan berarti membatalkannya.

- **Tinjau > Terima** menerima perubahan tempat kursor berada (atau setiap perubahan dalam teks yang Anda pilih), lalu pindah ke perubahan berikutnya. **Tinjau > Tolak** melakukan hal yang sama, tetapi membatalkan perubahan.
- Klik panah di bawah **Terima** atau **Tolak** untuk pilihan lain: **Terima Semua Perubahan yang Ditampilkan**, **Terima Semua Perubahan**, dan **Terima Semua Perubahan dan Hentikan Pelacakan**; serta **Tolak Semua Perubahan yang Ditampilkan** dan **Tolak Semua Perubahan**. "Yang ditampilkan" berarti perubahan dari jenis yang diaktifkan oleh **Tampilkan Markup** (dan, di panel **Peninjauan**, peninjau yang Anda pilih).
- **Sebelumnya** dan **Berikutnya** pada grup **Perubahan** berpindah dari satu perubahan ke perubahan lain tanpa mengubah apa pun. Jika kursor berada di awal teks, header, atau catatan, **Berikutnya** menuju perubahan yang dimulai tepat di situ.

Setiap perintah tersebut adalah satu langkah, sehingga **Ctrl+Z** mengembalikan dokumen persis seperti semula.

Menerima penghapusan akan menghilangkan teks secara permanen. Menolak penyisipan juga menghilangkan teks. Menerima penyisipan mempertahankan teks dan menghapus tanda bahwa teks itu baru. Menolak penghapusan mempertahankan teks dan menghapus coretannya. Menolak perubahan tampilan mengembalikan tampilan yang dimiliki teks sebelumnya. Teks yang dipindahkan merupakan sepasang perubahan, satu di tempat asalnya dan satu di tempat tujuannya, yang dihubungkan oleh nama pemindahan. Jika pasangan itu ditandai dengan nama yang sama, seperti yang disimpan program lain, menerima atau menolak salah satu dari keduanya akan memengaruhi keduanya, sehingga teks tersebut benar-benar dipindahkan atau dikembalikan. Nama yang dipakai lebih dari sekali, atau hanya memiliki satu bagian, tidak menghubungkan apa pun: setiap bagian menjadi perubahan tersendiri.

## Panel Peninjauan

Klik **Tinjau > Panel Peninjauan** untuk membuka daftar perubahan dalam dokumen: jenisnya, siapa yang membuatnya, kapan, dan katanya. Klik salah satu untuk menampilkannya dalam teks. Gunakan **Terima**, **Tolak**, **Sebelumnya**, dan **Berikutnya** di bawah daftar, atau pilih satu peninjau dari kotak di atasnya untuk hanya melihat perubahan orang tersebut.

## Perubahan dalam header, footer, catatan, dan tabel

Perubahan yang dibuat orang lain di header, footer, catatan kaki, atau catatan akhir ditampilkan dan didaftarkan seperti perubahan dalam teks, termasuk header atau footer yang tidak digunakan halaman mana pun (misalnya yang untuk halaman genap ketika dokumen tidak memakai halaman ganjil dan genap yang berbeda): **Berikutnya** melewatinya, dan **Terima** serta **Tolak** melakukan perubahan yang terakhir Anda tuju di sana. **Berikutnya**, **Sebelumnya**, **Terima**, **Tolak**, dan panel **Peninjauan** membawa Anda ke header, footer, atau catatan yang memiliki perubahan, dengan urutan: teks dahulu, lalu catatan kaki, lalu header dan footer. **Terima Semua** dan **Tolak Semua** mencakup semuanya.

Perubahan pada tabel juga didaftarkan, baik dalam teks maupun di header dan footer: baris atau sel yang disisipkan atau dihapus, sel yang digabungkan, serta perubahan tampilan tabel, baris, atau sel. Sebuah bilah di margin menandai baris-baris dari baris atau sel tersebut. Menerima baris yang dihapus, atau menolak baris yang disisipkan, akan menghapus baris beserta isinya. Baris yang setiap selnya memiliki perubahan itu menjadi perubahan milik baris tersebut. Menolak perubahan tampilan mengembalikan tampilan yang dimiliki tabel, baris, atau sel sebelumnya, dan menolak penggabungan akan memisahkan sel kembali. Jika menghapus perubahan akan merusak tabel, perubahan itu dipertahankan dan pesan menyebutkan berapa banyak yang dipertahankan. Hal ini terjadi pada sel yang digabungkan dengan sel lain ke bawah: sel itu tetap dan kehilangan tandanya, dan pada perubahan kisi yang tidak dapat diikuti oleh baris, yang tetap sebagai perubahan yang dapat Anda terima. Perubahan di dalam tabel pada catatan kaki dipertahankan seperti aslinya: tidak didaftarkan, dan **Terima** serta **Tolak** tidak memengaruhinya.

Pemindahan adalah satu baris dalam daftar, dan satu perubahan dalam jumlah yang diberikan **Terima Semua** dan **Tolak Semua**: pasangannya dan tanda paragraf yang menyertainya ikut diselesaikan. Perubahan di dalam kotak teks didaftarkan satu per satu, setelah perubahan dalam teks dan sebelum perubahan dalam catatan kaki. **Berikutnya** dan **Sebelumnya** membuka kotak untuk diketik, dengan perubahan tersebut terpilih, dan **Terima** serta **Tolak** menyelesaikan perubahan itu, masing-masing sebagai satu langkah. Memindahkan kursor di dalam kotak yang terbuka membuat **Berikutnya** dan **Sebelumnya** melanjutkan dari posisi kursor. Kotak yang diselesaikan satu per satu disimpan tanpa salinan yang dibaca program lama, seperti saat Anda mengetik di dalam kotak. Hanya 200 kotak pertama yang memuat perubahan didaftarkan dengan cara ini. Kotak teks yang menjadi bagian dari kelompok, atau yang teksnya berlanjut ke kotak lain, kotak di dalam kotak lain, kotak di header atau catatan, serta kotak setelah 200 pertama tidak dapat dibuka dengan cara ini: panel **Peninjauan** menghitung perubahannya dalam satu baris di bawah daftar, dan hanya **Terima Semua** dan **Tolak Semua** yang dapat menanganinya.

Perubahan pada penomoran paragraf didaftarkan sebagai **Penomoran** atau **Penomoran diubah**. Jika perubahan tampilan paragraf mencatat daftar dan tingkat yang dimiliki paragraf, menolaknya akan mengembalikan paragraf ke daftar itu pada tingkat tersebut (atau mengeluarkannya dari daftar yang tidak dimilikinya). Perubahan yang hanya menyimpan penomoran lama sebagai label (seperti yang dilakukan beberapa program) tidak menyebutkan daftar untuk dikembalikan: menerimanya mempertahankan penomoran, dan menolaknya menghapus penomoran hanya jika labelnya kosong; jika tidak, perubahan tetap ada dan dihitung sebagai dipertahankan. Menolak penomoran yang ditambahkan mengembalikan paragraf ke apa yang diberikan gayanya (gaya judul yang memiliki penomoran tetap memilikinya), dan tidak ada penomoran jika gayanya tidak memilikinya. Jika daftar yang akan dikembalikan oleh perubahan sudah tidak ada di dalam dokumen, tidak ada yang berubah pada paragraf itu, dan perubahan dihitung sebagai dipertahankan.

Perubahan pada bagian (ukuran halaman, orientasi, margin, kolom, penomoran halaman, atau jenis pemisah) didaftarkan sebagai **Bagian berformat**, pada paragraf yang mengakhiri bagian, atau di akhir teks untuk bagian terakhir. Menerimanya mempertahankan bagian seperti adanya dan menghapus catatannya, dan menolaknya mengembalikan properti yang dicatat, lalu halaman langsung ditata ulang. Header dan footer yang dirujuk bagian tidak berubah. Jika properti lama dalam catatan tidak dapat dibaca, menolak akan membiarkan bagian dan perubahan seperti semula, dan dihitung sebagai dipertahankan; tidak ada halaman yang diganti dengan nilai bawaan. Jika **Terima Semua** atau **Tolak Semua** juga menghapus atau mengembalikan pemisah bagian, perubahan setiap bagian diselesaikan lebih dahulu, sehingga tidak ada yang tertinggal.

Perubahan yang tidak dapat diselesaikan tanpa merusak dokumen (tabel yang akan rusak, daftar atau bagian yang keadaan lamanya tidak dapat dikembalikan, atau kotak teks yang terlalu besar untuk ditulis kembali) dibiarkan seperti tertulis, dan pesan setelah **Terima Semua** atau **Tolak Semua** menyebutkan berapa banyak yang dipertahankan.

## Perubahan yang dicatat dan yang tidak

Perubahan berikut dicatat saat Lacak Perubahan aktif: mengetik, menghapus, dan menempel teks; Enter; mengganti kata; tampilan teks dan paragraf (font, ukuran, warna, perataan, spasi, bingkai, dan gaya); **Ubah Huruf**; simbol dan karakter khusus; pemisah halaman dan kolom; tanda paragraf pemisah bagian; dan, dalam tabel, baris yang Anda sisipkan (dengan **Sisipkan di Atas** atau **Sisipkan di Bawah**, atau dengan Tab di sel terakhir), baris yang Anda hapus, tabel yang Anda hapus, dan tabel yang Anda sisipkan. Baris yang disisipkan atau dihapus didaftarkan di panel **Peninjauan** sebagai satu perubahan, beserta katanya, dan **Terima** serta **Tolak** mempertahankannya atau menghapusnya. Menghapus baris yang memuat tabel juga menandai baris-baris tabel tersebut. Baris yang disisipkan orang lain lalu Anda hapus memuat kedua perubahan, dan **Terima** maupun **Tolak** akan menghapusnya. Baris yang Anda sisipkan sendiri akan hilang secara permanen jika Anda menghapusnya.

Perubahan berikut dilakukan seperti biasa, tanpa tanda, dan bilah status menampilkan "This change was not recorded as a tracked change." setiap kali: butir, penomoran, dan perintah daftar lainnya; **Tambah Indentasi** dan **Kurangi Indentasi**; pengaturan halaman, kolom, margin, dan nomor baris; header dan footer, serta mengetik, menghapus, dan mengganti kata di header, footer, catatan kaki, atau catatan akhir; menyisipkan catatan kaki, catatan akhir, daftar isi, indeks, bidang, tautan, atau kontrol konten; gambar, bagan, persamaan, kotak teks, dan bentuk yang digambar; kolom, sel, penggabungan, pemisahan, dan tampilan tabel; menghapus baris yang merupakan bagian sel yang digabung ke bawah melalui beberapa baris; mengurutkan paragraf atau baris tabel; serta memindahkan tabel atau bagian. Saat Anda mengetik di header atau catatan, pesan muncul sekali, bukan pada setiap tombol. **Urungkan** membatalkan salah satu perubahan ini sebagai satu langkah.

Beberapa perubahan sama sekali tidak dilakukan saat Lacak Perubahan aktif, karena tanpa tanda perubahan tersebut akan merusak perubahan yang sudah dicatat: di dalam tabel yang memiliki baris yang disisipkan atau dihapus, **Hapus Kolom**, **Hapus Sel**, **Gabungkan Sel**, **Pisahkan Sel**, **Ubah ke Teks**, dan menempel di atas sel. Bilah status menyebutkannya. Terima atau tolak perubahan dalam tabel itu terlebih dahulu, atau nonaktifkan Lacak Perubahan. Dalam dokumen yang mengunci Lacak Perubahan dalam keadaan aktif, sehingga Anda tidak dapat menerima atau menolak, perubahan tersebut tetap dilakukan, dan bilah status menyebutkan bahwa perubahan itu tidak dicatat.

Perubahan yang dicatat program lain pada penomoran dan bagian didaftarkan, dan dapat diterima atau ditolak seperti dijelaskan di atas.

## Membandingkan dua versi

**Bandingkan** mencari perbedaan antara dua versi dokumen dan menampilkannya sebagai perubahan yang dilacak.

1. Buka versi yang lebih lama. Itu adalah **asli**.
2. Klik **Bandingkan** pada grup **Bandingkan** di tab **Tinjau**.
3. Klik **Telusuri** dan pilih versi yang lebih baru. Itu adalah versi **revisi**.
4. Ketik nama yang akan digunakan untuk menandai perubahan. Pilih opsi lain yang Anda inginkan, misalnya mengabaikan huruf kapital. **Temukan teks yang dipindahkan** dan **Temukan perubahan pemformatan** aktif kecuali Anda menonaktifkannya.
5. Klik **OK**.

Jendela baru akan terbuka dengan teks asli dan perubahan yang mengubahnya menjadi teks revisi. Kedua berkas Anda tidak berubah. Gunakan **Terima** dan **Tolak** seperti pada perubahan yang dilacak lainnya. Jika Anda menerima semua perubahan, Anda mendapatkan teks revisi. Jika Anda menolak semua perubahan, Anda mendapatkan teks asli.

Bandingkan hanya memeriksa teks pada badan dokumen. Bandingkan tidak memeriksa tabel, header, footer, atau catatan kaki.

Paragraf berisi 12 kata atau lebih yang dipotong dari satu tempat dan ditempelkan di tempat lain ditampilkan sebagai **pemindahan**: satu catatan di tempat teks itu berasal, dan satu catatan di tempat teks itu tiba. Menerima atau menolak salah satu dari keduanya akan memengaruhi keduanya. Paragraf harus sama, atau setidaknya 9 dari 10 kata sama dan sebagian besar dalam urutan yang sama; jika tidak, paragraf ditampilkan sebagai penghapusan dan penyisipan. Kata yang berbeda di dalam paragraf yang dipindahkan ditampilkan sebagai penyisipan dan penghapusan di dalam pemindahan.

Teks yang sama di kedua versi tetapi tampilannya berbeda ditampilkan sebagai **perubahan pemformatan**. Bandingkan memeriksa tebal, miring, garis bawah, ukuran, font, warna, sorotan, serta gaya, perataan, indentasi, dan spasi paragraf. Menolak perubahan ini mengembalikan tampilan lama.

Jika kedua dokumen memiliki terlalu banyak perubahan, Bandingkan akan memberi tahu Anda. Bandingkan kemudian menampilkan sebagian paragraf sebagai diganti seluruhnya, dan mungkin tidak mencari pemindahan atau perubahan pemformatan di semua paragraf.
